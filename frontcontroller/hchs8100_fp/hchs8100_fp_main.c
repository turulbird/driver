/****************************************************************************************
 *
 * hchs8100_fp_main.c
 *
 * (c) 2019-2023 Audioniek
 *
 * Some ground work has been done by corev in the past in the form of
 * a VFD driver for the HS5101 models which share the same front panel
 * board.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *
 * Front panel driver for Homecast HS8100 series;
 * Button, LEDs, Fan, RTC and VFD driver, main part.
 *
 * The HS8100 features a 12 character dot matrix VFD display with
 * 34 icons. The display and icons are driven from a PT6302-007
 * display controller.
 *
 * In addition the front panel features seven push buttons.
 *
 * On the main board a battery backed up Dallas DS1307 RTC is present,
 * the driver supports this as a true RTC.
 *
 *
 * Devices:
 *  - /dev/vfd (vfd ioctls and read/write function)
 *  - /dev/rtc0
 *
 ****************************************************************************************
 *
 * Changes
 *
 * Date     By              Description
 * --------------------------------------------------------------------------------------
 * 20220918 Audioniek       Start of work.
 * 20221229 Audioniek       Initial release version.
 *
 ****************************************************************************************/
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/input.h>
#include <linux/version.h>
#include <linux/workqueue.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17)
#include <linux/stm/pio.h>
#else
#include <linux/stpio.h>
#endif
#include <linux/timer.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/reboot.h>
#include <linux/i2c.h>
#include <linux/rtc.h>
#include <linux/bcd.h>
#include <linux/platform_device.h>

#include "hchs8100_fp.h"

// Global variables
struct stpio_pin *button_data_in;   // HC164 A & B inputs
struct stpio_pin *button_clk;       // HC164 clock
struct stpio_pin *button_key_data;  // button data

static struct fp_driver fp;

// module parameter defaults
short paramDebug = 0;
int waitTime = 1000;
static char *gmt = "+0000";
char *gmt_offset = "3600";
int rtc_offset;

// fan
struct stpio_pin *fan_onoff_pio;
struct stpio_pin *fan_speed_pio;

// VFD write thread
tVFDState vfd_text_state;

// Standby
tStandbyState standby_state;
//int front_power_key = 0;
int rc_power_key = 0;

// Timer
tTimerState timer_state;

// Spinner
tSpinnerState spinner_state;

// RTC
#define RTC_NAME "hchs8100-rtc"
static struct platform_device *rtc_pdev;
char *weekday_name[8] = { "Illegal", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

struct ds1307_state *rtc_state = NULL;

/******************************************************
 *
 * Fan speed code
 *
 */

/******************************************************
 *
 * Set fan speed.
 *
 */
int hchs8100_set_fan(int on, int speed)
{
	int res = 0;

//	dprintk(100, "%s > on = %d speed = %d\n", __func__, on, speed);

	stpio_set_pin(fan_onoff_pio, on);
	stpio_set_pin(fan_speed_pio, speed);
//	dprintk(100, "%s <\n", __func__);
	return res;
}

void fan_free_pio_pins(void)
{
	if (fan_onoff_pio != NULL)
	{
		stpio_free_pin(fan_onoff_pio);
	}
	if (fan_speed_pio != NULL)
	{
		stpio_free_pin(fan_speed_pio);
	}
}

static int __init init_fan_module(void)
{
	dprintk(50, "Initializing fan driver\n");

	fan_onoff_pio = stpio_request_pin(FAN_ON_PORT, FAN_ON_PIN, "fan_onoff", STPIO_OUT);
	if (fan_onoff_pio == NULL)
	{
		dprintk(1, "%s: Request for Fan on/off PIO failed; abort\n", __func__);
		goto fan_init_fail;
	}
	fan_speed_pio = stpio_request_pin(FAN_SPEED_PORT, FAN_SPEED_PIN, "fan_speed", STPIO_OUT);
	if (fan_speed_pio == NULL)
	{
		dprintk(1, "%s: Request for Fan speed PIO failed; abort\n", __func__);
		goto fan_init_fail;
	}
	stpio_set_pin(fan_onoff_pio, 1);
	stpio_set_pin(fan_speed_pio, 0);  // low speed
	return 0;

fan_init_fail:
	fan_free_pio_pins();
	dprintk(1, "%s < Error initializing fan PIO pins\n", __func__);
	return -EIO;
}

static void __exit cleanup_fan_module(void)
{
	fan_free_pio_pins();
}
// end of fan driver code

/*********************************************************************
 *
 * Front panel button driver.
 *
 * Based on work by corev and spark key driver.
 *
 */
#define BUTTON_POLL_MSLEEP 50  // ms
#define BUTTON_POLL_DELAY  10  // us
static char                    *button_driver_name = "Homecast HS8100 series front panel button driver";
static struct input_dev        *button_dev;
static int                     key_polling = 1;
static struct workqueue_struct *button_wq;

/******************************************************
 *
 * Free the front panel button PIO pins
 *
 */
void button_free_pio_pins(void)
{
//	dprintk(150, "%s >\n", __func__);
	if (button_data_in)
	{
		stpio_set_pin(button_data_in, 1);
	}
	if (button_key_data)
	{
		stpio_free_pin(button_key_data);
	}
	if (button_clk)
	{
		stpio_free_pin(button_clk);
	}
	if (button_data_in)
	{
		stpio_free_pin(button_data_in);
	}
//	dprintk(150, "%s < button PIO pins freed\n", __func__);
};

/******************************************************
 *
 * Initialize the front panel button PIOs.
 *
 */
static unsigned char button_init_pio_pins(void)
{
	dprintk(20, "Initialize button PIO pins\n");

//	button_free_pio_pins();
//	HC164 Data
	button_data_in = stpio_request_pin(HC164_DIN_PORT, HC164_DIN_PIN, "HC164_Din", STPIO_OUT);
	if (button_data_in == NULL)
	{
		dprintk(1, "%s: Request for HC164 Data input failed; abort\n", __func__);
		goto pio_init_fail;
	}
//	HC164 Clock
	button_clk = stpio_request_pin(HC164_CLK_PORT, HC164_CLK_PIN, "HC164_CLK", STPIO_OUT);
	if (button_clk == NULL)
	{
		dprintk(1, "%s: Request for HC164 CLK failed; abort\n", __func__);
		goto pio_init_fail;
	}
//	Button data in
	button_key_data = stpio_request_pin(KEY_DATA_PORT, KEY_DATA_PIN, "Key_Data", STPIO_IN);
	if (button_key_data == NULL)
	{
		dprintk(1, "%s: Request for Button data failed; abort\n", __func__);
		goto pio_init_fail;
	}
//	set all involved PIO pins
	stpio_set_pin(button_data_in, 1);
	stpio_set_pin(button_clk, 1);
	return 0;

pio_init_fail:
	button_free_pio_pins();
	dprintk(1, "%s < Error initializing button PIO pins\n", __func__);
	return -EIO;
}

/******************************************************
 *
 * Get front panel key data.
 *
 */
int get_fp_keyData(void)
{
	int value;
	int old_value;
	int i;

	// HC164: clock in a single zero bit
	stpio_set_pin(button_data_in, 0);  // PIO1,5
	stpio_set_pin(button_clk, 0);
	udelay(BUTTON_POLL_DELAY);
	stpio_set_pin(button_clk, 1);
	udelay(BUTTON_POLL_DELAY);
	// next, clock 8 one's in during the scan, to get a walking zero
	stpio_set_pin(button_data_in, 1);

	value = 0;  // value for no key pressed
	for (i = 0; i < 7; i++)
	{
		if (stpio_get_pin(button_key_data) == 0)  // if key data is zero
		{
			value |= 0x80;  // flag this key down
		}
		value >>= 1;
		// toggle HC164 clock input
		stpio_set_pin(button_clk, 0);
		udelay(BUTTON_POLL_DELAY);
		stpio_set_pin(button_clk, 1);
		udelay(BUTTON_POLL_DELAY);
	}
	// show key response on frontpanel
	if (old_value != value)
	{
		i = pt6302_set_icon(ICON_CIRCLE_0, 2);  // 2: invert icon state
	}
	old_value = value;
	// if standby thread running, only test for power key
	if (standby_state.state == 1 || standby_state.state == 2)
	{
		if (value == HCHS8100_BUTTON_POWER)
		{
//			dprintk(50, "%s Front panel power pressed during standby\n", __func__);
//			front_power_key = 1;
			return HCHS8100_BUTTON_POWER;
		}
		return HCHS8100_BUTTON_NONE;
	}
	else
	{
		switch (value)
		{
			case HCHS8100_BUTTON_POWER:
			{
				return KEY_POWER;
			}
			case HCHS8100_BUTTON_LEFT:
			{
				return KEY_LEFT;
			}
			case HCHS8100_BUTTON_RIGHT:
			{
				return KEY_RIGHT;
			}
			case HCHS8100_BUTTON_DOWN:
			{
				return KEY_DOWN;
			}
			case HCHS8100_BUTTON_UP:
			{
				return KEY_UP;
			}
			case HCHS8100_BUTTON_MENU:
			{
				return KEY_MENU;
			}
			case HCHS8100_BUTTON_EXIT:
			{
				return KEY_OK;
			}
			default:
			{
				return HCHS8100_BUTTON_NONE;
			}
		}
	}
	return HCHS8100_BUTTON_NONE;  // flag no key
}

/******************************************************
 *
 * Key polling.
 *
 */
static void button_polling(struct work_struct *work)
{
	int ret = 0;
	int btn_pressed = 0;
	int report_key = 0;

	dprintk(50, "%s >\n", __func__);

	while (key_polling == 1)
	{
		int button_value;

		msleep(BUTTON_POLL_MSLEEP);
		button_value = get_fp_keyData();
		if (button_value != HCHS8100_BUTTON_NONE)
		{
			if (btn_pressed == 1)
			{
				if (report_key == button_value)
				{
					continue;
				}
				input_report_key(button_dev, report_key, 0);
				input_sync(button_dev);
			}
			report_key = button_value;
			btn_pressed = 1;

			switch (button_value)
			{
				case KEY_LEFT:
				case KEY_RIGHT:
				case KEY_UP:
				case KEY_DOWN:
				case KEY_MENU:
				case KEY_OK:
				case KEY_POWER:
				{
					input_report_key(button_dev, button_value, 1);
					input_sync(button_dev);
					break;
				}
				default:
				{
					dprintk(1, "%s: Unknown button_value %02X\n", __func__, button_value);
				}
			}
		}
		else if (btn_pressed)
		{
			btn_pressed = 0;

			input_report_key(button_dev, report_key, 0);
			input_sync(button_dev);
		}
	}
	key_polling = 2;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 17)
static DECLARE_WORK(button_obj, button_polling);
#else
static DECLARE_WORK(button_obj, button_polling, NULL);
#endif

static int button_input_open(struct input_dev *dev)
{
	button_wq = create_workqueue("button");
	if (queue_work(button_wq, &button_obj))
	{
		dprintk(20, "[BTN] Queue_work successful.\n");
		return 0;
	}
	dprintk(1, "[BTN] Queue_work not successful, exiting.\n");
	return 1;
}

static void button_input_close(struct input_dev *dev)
{
	key_polling = 0;
	while (key_polling != 2)
	{
		msleep(1);
	}
	key_polling = 1;

	if (button_wq)
	{
		destroy_workqueue(button_wq);
		dprintk(20, "[BTN] Workqueue destroyed.\n");
	}
}

int button_dev_init(void)
{
	int error;

	dprintk(20, "[BTN] Allocating and registering button device\n");
	if (button_init_pio_pins() != 0)
	{
		return -EIO;
	}

	button_dev = input_allocate_device();

	if (!button_dev)
	{
		return -ENOMEM;
	}
	button_dev->name  = button_driver_name;
	button_dev->open  = button_input_open;
	button_dev->close = button_input_close;

	set_bit(EV_KEY,    button_dev->evbit);
	set_bit(KEY_UP,    button_dev->keybit);
	set_bit(KEY_DOWN,  button_dev->keybit);
	set_bit(KEY_LEFT,  button_dev->keybit);
	set_bit(KEY_RIGHT, button_dev->keybit);
	set_bit(KEY_POWER, button_dev->keybit);
	set_bit(KEY_MENU,  button_dev->keybit);
	set_bit(KEY_EXIT,  button_dev->keybit);
	error = input_register_device(button_dev);
	if (error)
	{
		input_free_device(button_dev);
	}
	return error;
}

void button_dev_exit(void)
{
	dprintk(20, "[BTN] Unregistering button device.\n");
	input_unregister_device(button_dev);
	button_free_pio_pins();
}

/*********************************************************************
 *
 * Real Time Clock driver.
 *
 * The main board has a battery backed up Dallas DS1307 compatible
 * RTC device mounted on it.
 *
 */

/******************************************************
 *
 * Code to access the DS1307 RTC.
 *
 * The RTC is located on I2C address 0x68 on I2C bus 2.
 *
 * The RTC is always operated in 24h mode; the mode is
 * forcibly set to 24h each time the time is written.
 *
 * RTC time is in UTC.
 *
 */

/******************************************************
 *
 * Read one DS1307 register.
 *
 */
int ds1307_readreg(unsigned char reg)
{
	int ret;
	unsigned char b0[] = { reg };
	unsigned char b1[] = { 0 };
	struct i2c_msg msg[] =
	{
		{ .addr = rtc_state->rtc_i2c_address, .flags = 0,        .buf = b0, .len = 1 },
		{ .addr = rtc_state->rtc_i2c_address, .flags = I2C_M_RD, .buf = b1, .len = 1 }
	};

	ret = i2c_transfer(rtc_state->rtc_i2c, msg, 2);

	if (ret != 2)
	{
		dprintk(1, "%s: reg = 0x%x (error = %d)\n", __func__, reg, ret);
		return ret;
	}
	return b1[0];
}

/******************************************************
 *
 * Write one DS1307 register.
 *
 */
int ds1307_writereg(unsigned char reg, unsigned char value)
{
	unsigned char buf[] = { reg, value };
	struct i2c_msg msg =
	{ .addr = rtc_state->rtc_i2c_address, .flags = 0, .buf = buf, .len = 2 };
	int ret;

	ret = i2c_transfer(rtc_state->rtc_i2c, &msg, 1);
	if (ret != 1)
	{
		dprintk(1, "%s: writereg error(ret == %i, reg == 0x%02x, value == 0x%02x)\n", __func__, ret, reg, value);
		return -EREMOTEIO;
	}
	return 0;
}

/******************************************************
 *
 * Read all 8 DS1307 RTC clock registers into state.
 *
 */
void ds1307_readregs(void)
{
	int i;
	char *pm_string = { 0 };

	rtc_state->rtc_reg[DS1307_REG_SECS]    = ds1307_readreg(DS1307_REG_SECS);
	rtc_state->rtc_reg[DS1307_REG_MINS]    = ds1307_readreg(DS1307_REG_MINS);
	rtc_state->rtc_reg[DS1307_REG_HOURS]   = ds1307_readreg(DS1307_REG_HOURS);
	rtc_state->rtc_reg[DS1307_REG_WDAY]    = ds1307_readreg(DS1307_REG_WDAY);
	rtc_state->rtc_reg[DS1307_REG_MDAY]    = ds1307_readreg(DS1307_REG_MDAY);
	rtc_state->rtc_reg[DS1307_REG_MONTH]   = ds1307_readreg(DS1307_REG_MONTH);
	rtc_state->rtc_reg[DS1307_REG_YEAR]    = ds1307_readreg(DS1307_REG_YEAR);
	rtc_state->rtc_reg[DS1307_REG_CONTROL] = ds1307_readreg(DS1307_REG_CONTROL);

	rtc_state->rtc_seconds = bcd2bin(rtc_state->rtc_reg[DS1307_REG_SECS] & 0x7f);
	rtc_state->rtc_minutes = bcd2bin(rtc_state->rtc_reg[DS1307_REG_MINS] & 0x7f);
	if (bcd2bin(rtc_state->rtc_reg[DS1307_REG_HOURS] & DS1307_BIT_AMPM))
	{
		// AM/PM mode
		rtc_state->rtc_hours    = bcd2bin(rtc_state->rtc_reg[DS1307_REG_HOURS] & 0x1f);
		rtc_state->rtc_ampmmode = DS1307_BIT_AMPM;
		rtc_state->rtc_pm       = bcd2bin(rtc_state->rtc_reg[DS1307_REG_HOURS] & DS1307_BIT_PM);
	}
	else
	{
		// 24h mode
		rtc_state->rtc_hours    = bcd2bin(rtc_state->rtc_reg[DS1307_REG_HOURS] & 0x3f);
		rtc_state->rtc_ampmmode = 0;
		rtc_state->rtc_pm       = 0;
	}
	rtc_state->rtc_weekday = bcd2bin(rtc_state->rtc_reg[DS1307_REG_WDAY] & 0x07);  // Note: Sunday = 1
	rtc_state->rtc_day     = bcd2bin(rtc_state->rtc_reg[DS1307_REG_MDAY] & 0x3f);
	rtc_state->rtc_month   = bcd2bin(rtc_state->rtc_reg[DS1307_REG_MONTH] & 0x1f);
	rtc_state->rtc_year    = bcd2bin(rtc_state->rtc_reg[DS1307_REG_YEAR]);

#if 0
	if (rtc_state->rtc_ampmmode)
	{
		pm_string = (rtc_state->rtc_pm ? " PM" : " AM");
	}
	else
	{
		pm_string = "";
	}
	dprintk(50, "%s: DS1307 Date: %s %02d-%02d-(20)%02d Time: %02d:%02d:%02d%s\n", __func__,
		weekday_name[rtc_state->rtc_weekday], rtc_state->rtc_day, rtc_state->rtc_month, rtc_state->rtc_year,
		rtc_state->rtc_hours, rtc_state->rtc_minutes, rtc_state->rtc_seconds, pm_string);
#endif
}

/******************************************************
 *
 * Write state to all 8 DS1307 RTC registers.
 *
 */
int ds1307_writeregs(void)
{
	int ret = 0;

	// seconds must be written last to avoid roll over issues
	ret |= ds1307_writereg(DS1307_REG_CONTROL, rtc_state->rtc_reg[DS1307_REG_CONTROL]);
	ret |= ds1307_writereg(DS1307_REG_YEAR,    rtc_state->rtc_reg[DS1307_REG_YEAR]);
	ret |= ds1307_writereg(DS1307_REG_MONTH,   rtc_state->rtc_reg[DS1307_REG_MONTH]);
	ret |= ds1307_writereg(DS1307_REG_MDAY,    rtc_state->rtc_reg[DS1307_REG_MDAY]);
	ret |= ds1307_writereg(DS1307_REG_WDAY,    rtc_state->rtc_reg[DS1307_REG_WDAY]);
	ret |= ds1307_writereg(DS1307_REG_HOURS,   rtc_state->rtc_reg[DS1307_REG_HOURS]);
	ret |= ds1307_writereg(DS1307_REG_MINS,    rtc_state->rtc_reg[DS1307_REG_MINS]);
	ret |= ds1307_writereg(DS1307_REG_SECS,    rtc_state->rtc_reg[DS1307_REG_SECS]);
	return ret;
}

/******************************************************
 *
 * Read the time from the DS1307.
 *
 * Returns time as seconds since epoch.
 *
 */
time_t read_rtc_time(void)
{
	time_t uTime = 0;
	int i;

	ds1307_readregs();

	// convert the rtc state to uTime
	uTime  = rtc_state->rtc_seconds;
	uTime += rtc_state->rtc_minutes * MINUTE_SECS;
	uTime += rtc_state->rtc_hours * HOUR_SECS;
	if (rtc_state->rtc_pm & rtc_state->rtc_hours >= 12)
	{
		uTime += 12 * HOUR_SECS;
	}
	uTime += (rtc_state->rtc_day - 1) * DAY_SECS;  // - 1: days count from 1, do not include current day
	// handle months
	for (i = 0; i < rtc_state->rtc_month - 1; i++)  // - 1: months count from 1, do not include current month
	{
		uTime += (_ytab[(LEAPYEAR(rtc_state->rtc_year + 2000))][i]) * DAY_SECS;
	}
	// handle RTC years
	for (i = 0; i < rtc_state->rtc_year; i++)  // years count from 0, do not include current year
	{
		uTime += YEARSIZE(i + 2000) * DAY_SECS;
	}
	// Note: linux epoch is 01-01-1970 midnight, RTC epoch is 01-01-2000 midnight, so:
	// account for epoch difference
	for (i = 1970; i < 2000; i++)
	{
		uTime += YEARSIZE(i) * DAY_SECS;
	}
	return uTime;
}

/******************************************************
 *
 * Write seconds since linux epoch to the DS1307.
 *
 * NOTE: Always sets RTC to 24 hour mode.
 *
 */
int write_rtc_time(time_t uTime)
{
	int dayclock;
	int dayno;
	int year;
	int month;
	int weekday;
	int i;
	int ret = 0;

	dayclock = uTime % DAY_SECS;  // number of seconds in the current day
	dayno = (uTime / DAY_SECS);  // number of whole days since 01-01-1970
	weekday = ((dayno + 4) % 7) + 1;  // 01-01-1970 was a thursday (4), Sunday = 1

	// account for the difference between Linux and RTC epochs
	for (i = 0; i < 30; i++)
	{
		dayno -= YEARSIZE(i + 1970);
	}
	// dayno is now the number of whole days since 01-01-2000
	// calculate years to send to DS1307
	year = 2000;
	while (dayno >= YEARSIZE(year))
	{
		dayno -= YEARSIZE(year);
		year++;
	}
	year -= 2000; // remove century (DS1307 has not got one)
	// dayno is now the number of whole days in the current year
	month = 0;
	while (dayno >= _ytab[LEAPYEAR(year)][month])
	{
		dayno -= _ytab[LEAPYEAR(year)][month];
		month++;
	}
	month++;  // months count from 1
	dayno++;  // days count from 1

	rtc_state->rtc_reg[DS1307_REG_YEAR]  = bin2bcd(year);
 	rtc_state->rtc_reg[DS1307_REG_MONTH] = bin2bcd(month);
 	rtc_state->rtc_reg[DS1307_REG_MDAY]  = bin2bcd(dayno);
	rtc_state->rtc_reg[DS1307_REG_WDAY]  = bin2bcd(weekday);
 	rtc_state->rtc_reg[DS1307_REG_HOURS] = bin2bcd(dayclock / HOUR_SECS) & ~DS1307_BIT_AMPM;
 	rtc_state->rtc_reg[DS1307_REG_MINS]  = bin2bcd((dayclock % HOUR_SECS) / MINUTE_SECS);
	rtc_state->rtc_reg[DS1307_REG_SECS]  = bin2bcd(dayclock % MINUTE_SECS);
	ret |= ds1307_writeregs();
	return ret;
}
 	
/**************************************
 *
 * Read wakeup time into state.
 *
 */
int ds1307_getalarmstate(void)
{
	rtc_state->rtc_wakeup_seconds = bcd2bin(ds1307_readreg(DS1307_REG_WAKEUP_SECS));
	rtc_state->rtc_wakeup_minutes = bcd2bin(ds1307_readreg(DS1307_REG_WAKEUP_MINS));
	rtc_state->rtc_wakeup_hours   = bcd2bin(ds1307_readreg(DS1307_REG_WAKEUP_HOURS));
	rtc_state->rtc_wakeup_day     = bcd2bin(ds1307_readreg(DS1307_REG_WAKEUP_DAY));
	rtc_state->rtc_wakeup_month   = bcd2bin(ds1307_readreg(DS1307_REG_WAKEUP_MONTH));
	rtc_state->rtc_wakeup_year    = bcd2bin(ds1307_readreg(DS1307_REG_WAKEUP_YEAR));
	return 0;
}

/**************************************
 *
 * Read wakeup time from DS1307 RAM.
 *
 */
int ds1307_getalarmtime(unsigned char *year, unsigned char *month, unsigned char *day, unsigned char *hours, unsigned char *minutes, unsigned char *seconds)
{
	int ret = 0;

	ret |= ds1307_getalarmstate();
	*seconds = rtc_state->rtc_wakeup_seconds;
	*minutes = rtc_state->rtc_wakeup_minutes;
	*hours   = rtc_state->rtc_wakeup_hours;
	*day     = rtc_state->rtc_wakeup_day;
	*month   = rtc_state->rtc_wakeup_month;
	*year    = rtc_state->rtc_wakeup_year;
	return ret;
}

/**************************************
 *
 * Write wakeup time in DS1307 RAM.
 *
 */
int ds1307_setalarm(unsigned char year, unsigned char month, unsigned char day, unsigned char hours, unsigned char minutes, unsigned char seconds)
{
	int ret = 0;

	rtc_state->rtc_wakeup_seconds = seconds;
	rtc_state->rtc_wakeup_minutes = minutes;
	rtc_state->rtc_wakeup_hours   = hours;
	rtc_state->rtc_wakeup_day     = day;
	rtc_state->rtc_wakeup_month   = month;
	rtc_state->rtc_wakeup_year    = year;
	ret |= ds1307_writereg(DS1307_REG_WAKEUP_SECS,  bin2bcd(seconds));
	ret |= ds1307_writereg(DS1307_REG_WAKEUP_MINS,  bin2bcd(minutes));
	ret |= ds1307_writereg(DS1307_REG_WAKEUP_HOURS, bin2bcd(hours));
	ret |= ds1307_writereg(DS1307_REG_WAKEUP_DAY,   bin2bcd(day));
	ret |= ds1307_writereg(DS1307_REG_WAKEUP_MONTH, bin2bcd(month));
	ret |= ds1307_writereg(DS1307_REG_WAKEUP_YEAR,  bin2bcd(year));
	return ret;
}

/**************************************
 *
 * Read deep standby flag from DS1307
 * RAM.
 *
 */
int ds1307_get_deepstandby(int *dstby_flag)
{
	if (rtc_is_invalid())
	{
		*dstby_flag = WAKEUP_INVALID;
	}
	else
	{
		*dstby_flag  = ds1307_readreg(DS1307_REG_STANDBY_H) << 8;
		*dstby_flag |= ds1307_readreg(DS1307_REG_STANDBY_L) & 0xff;
	}
	rtc_state->rtc_deep_standby = *dstby_flag;
	return 0;
}

/**************************************
 *
 * Write deep standby flag in DS1307
 * RAM.
 *
 */
int ds1307_set_deepstandby(int dstby_flag)
{
	int ret = 0;

	ret |= ds1307_writereg(DS1307_REG_STANDBY_L, (unsigned char)(dstby_flag & 0xff));
	ret |= ds1307_writereg(DS1307_REG_STANDBY_H, (unsigned char)((dstby_flag >> 8) & 0xff));
	rtc_state->rtc_deep_standby = dstby_flag;
	return ret;
}

/**************************************
 *
 * Determine if DS1307 data is valid
 *
 */
int rtc_is_invalid(void)
{
	int ret = 0;
	int flag;

	if (ds1307_readreg(DS1307_REG_SECS) & DS1307_BIT_CH)
	{
		dprintk(1, "Error: RTC Clock stopped, assume battery empty.\n");
		return 1;
	}
	ds1307_readregs();  // get registers in state
	flag  = ds1307_readreg(DS1307_REG_STANDBY_H) << 8;
	flag |= ds1307_readreg(DS1307_REG_STANDBY_L) & 0xff;
	ret |= ds1307_getalarmstate();

	// Check if all bits that should read back as 0 actually are zero
	if ((rtc_state->rtc_reg[DS1307_REG_MINS]    & 0x80)
	||  (rtc_state->rtc_reg[DS1307_REG_HOURS]   & 0x80)
	||  (rtc_state->rtc_reg[DS1307_REG_WDAY]    & 0xf8)
	||  (rtc_state->rtc_reg[DS1307_REG_MDAY]    & 0xc0)
	||  (rtc_state->rtc_reg[DS1307_REG_MONTH]   & 0xe0)
	||  (rtc_state->rtc_reg[DS1307_REG_CONTROL] & 0x6c)
	// Check if clock holds valid data
	||  (rtc_state->rtc_seconds < 0 || rtc_state->rtc_seconds > 59)
	||  (rtc_state->rtc_minutes < 0 || rtc_state->rtc_minutes > 59)
	|| ((rtc_state->rtc_reg[DS1307_REG_HOURS] & DS1307_BIT_AMPM) && rtc_state->rtc_hours < 1)
	|| ((rtc_state->rtc_reg[DS1307_REG_HOURS] & DS1307_BIT_AMPM) && rtc_state->rtc_hours > 12)
	|| ((rtc_state->rtc_reg[DS1307_REG_HOURS] & DS1307_BIT_AMPM  == 0) && rtc_state->rtc_hours < 0)
	|| ((rtc_state->rtc_reg[DS1307_REG_HOURS] & DS1307_BIT_AMPM  == 0) && rtc_state->rtc_hours > 23)
	||  (rtc_state->rtc_weekday < 1 || rtc_state->rtc_weekday > 7)  // Sunday = 1
	||  (rtc_state->rtc_day < 1     || rtc_state->rtc_day > 31)
	||  (rtc_state->rtc_month < 1   || rtc_state->rtc_month > 12)
	||  (rtc_state->rtc_year < 0    || rtc_state->rtc_year > 99)
	// Check if a valid alarm time set
	||  (flag != WAKEUP_NORMAL && flag != WAKEUP_DEEPSTDBY)
	|| ((flag == WAKEUP_DEEPSTDBY) && (rtc_state->rtc_wakeup_seconds < 0 || rtc_state->rtc_wakeup_seconds > 59))
	|| ((flag == WAKEUP_DEEPSTDBY) && (rtc_state->rtc_wakeup_minutes < 0 || rtc_state->rtc_wakeup_minutes > 59))
	|| ((flag == WAKEUP_DEEPSTDBY) && (rtc_state->rtc_wakeup_hours < 0 || rtc_state->rtc_wakeup_hours > 23))
	|| ((flag == WAKEUP_DEEPSTDBY) && (rtc_state->rtc_wakeup_day < 1 || rtc_state->rtc_wakeup_day > 31))
	|| ((flag == WAKEUP_DEEPSTDBY) && (rtc_state->rtc_wakeup_month < 1 || rtc_state->rtc_wakeup_month > 12))
	|| ((flag == WAKEUP_DEEPSTDBY) && (rtc_state->rtc_wakeup_year < 0 || rtc_state->rtc_wakeup_year > 99))
	)
	{
		dprintk(1, "Error: RTC data corrupt.\n");
		return 1;
	}
	return 0;
}


/**************************************
 *
 * Probe the DS1307.
 *
 */
int probe_ds1307(void)
{
	int ret = 0;
	char *pm_string = { 0 };
	int flag;
	unsigned char wakeup_year;
	unsigned char wakeup_month;
	unsigned char wakeup_day;
	unsigned char wakeup_hours;
	unsigned char wakeup_minutes;
	unsigned char wakeup_seconds;

	rtc_state = kzalloc(sizeof(struct ds1307_state), GFP_KERNEL);
	if (rtc_state == NULL)
	{
		goto error1;
	}
	rtc_state->rtc_i2c_address = RTC_I2CADDR;
	rtc_state->rtc_i2c_bus = RTC_I2CBUS;
	rtc_state->rtc_i2c = i2c_get_adapter(rtc_state->rtc_i2c_bus);
	if (rtc_state->rtc_i2c == NULL)
	{
		dprintk(1, "%s Error: Failed to allocate I2C bus %d for DS1307 RTC.\n", __func__, rtc_state->rtc_i2c_bus);
		return -1;
	}
	/* check if the RTC is present */
	ret = ds1307_readreg(DS1307_REG_CONTROL);
	if (ret & ~(DS1307_BIT_OUT + DS1307_BIT_SQWE + DS1307_BIT_RS1 + DS1307_BIT_RS0))
	{
		dprintk(1, "Invalid probe, probably not a DS1307 device.\n");
		goto error2;
	}
	dprintk(50, "DS1307 compatible RTC detected at address 0x%02x on I2C bus %d.\n", rtc_state->rtc_i2c_address, rtc_state->rtc_i2c_bus);

	// check if clock holds valid data (also fills rtc_state)
	if (rtc_is_invalid())
	{
		dprintk(1, "Clock data corrupt or time/date data invalid, resetting RTC.\n");
		ret = 1;
		goto error1;
	}

	// Set RTC to 24h mode if in AM/PM mode
	if (rtc_state->rtc_reg[DS1307_REG_HOURS] & DS1307_BIT_AMPM)
	{
		dprintk(50, "CAUTION: RTC set to 24h mode (hours corrected)\n");
		if (rtc_state->rtc_reg[DS1307_REG_HOURS] & DS1307_BIT_PM)
		{
			rtc_state->rtc_reg[DS1307_REG_HOURS] += 0x12;
			if (rtc_state->rtc_reg[DS1307_REG_HOURS] & 0x1f == 0x24)
			{
				rtc_state->rtc_reg[DS1307_REG_HOURS] = 0x00;
			}
		}
		rtc_state->rtc_reg[DS1307_REG_HOURS] &= ~DS1307_BIT_AMPM;  // force 24h mode
		ds1307_writereg(DS1307_REG_HOURS, rtc_state->rtc_reg[DS1307_REG_HOURS]);
	}
	dprintk(50, "DS1307 Date: %s %02d-%02d-20%02d Time: %02d:%02d:%02d (UTC)\n",
		weekday_name[rtc_state->rtc_weekday], rtc_state->rtc_day, rtc_state->rtc_month, rtc_state->rtc_year,
		rtc_state->rtc_hours, rtc_state->rtc_minutes, rtc_state->rtc_seconds);
	dprintk(50, "DS1307 Wake up date: %02d-%02d-20%02d Wake up time: %02d:%02d:%02d\n",
		rtc_state->rtc_wakeup_day, rtc_state->rtc_wakeup_month, rtc_state->rtc_wakeup_year,
		rtc_state->rtc_wakeup_hours, rtc_state->rtc_wakeup_minutes, rtc_state->rtc_wakeup_seconds);
	ret |= ds1307_get_deepstandby(&flag);
	if (flag != WAKEUP_DEEPSTDBY)
	{
		dprintk(50, "No wake up flag set.\n");
	}
	return 0;

error1:
	/* Reset clock to:
	   24h mode
	   Time: midnight
	   Date 01-01-(20)00  // is a Saturday -> Weekday 7
	   Output 0
	   Square wave output off (f = 32.768kHz)
	   Oscillator enabled
	   Wakeup time is 01-01-(20)00, 00:00:00
	   Deep standby flag = 0 
	 */
#if 0
	// oscillator off?  turn it on
	if ((ds1307_readreg(DS1307_REG_SECS)) & DS1307_BIT_CH)
	{
		dprintk(1, "Clock stopped, restarting oscillator.\n");
		ret |= ds1307_writereg(DS1307_REG_SECS, ret &= ~DS1307_BIT_CH);
	}
#endif
	dprintk(10, "CAUTION: Initializing RTC to 00:00:00 Sat 01-01-2022 (24h mode)\n");
	rtc_state->rtc_reg[DS1307_REG_SECS]    = (0 & ~DS1307_BIT_CH); // 0 seconds, CH bit is zero -> start oscillator
	rtc_state->rtc_reg[DS1307_REG_MINS]    = 0;  // 0 minutes
	rtc_state->rtc_reg[DS1307_REG_HOURS]   = (0 & ~DS1307_BIT_AMPM); // 0 hours, force 24h mode
	rtc_state->rtc_reg[DS1307_REG_WDAY]    = 7;  // Saturday
	rtc_state->rtc_reg[DS1307_REG_MDAY]    = 1;  // Day
	rtc_state->rtc_reg[DS1307_REG_MONTH]   = 1;  // Month
	rtc_state->rtc_reg[DS1307_REG_YEAR]    = 22; // Year
	rtc_state->rtc_reg[DS1307_REG_CONTROL] = 0;  // OUT, SQWE, RS1 and RS0 bits zero
	rtc_state->rtc_reg[DS1307_REG_SECS]    = (0 & ~DS1307_BIT_CH); // 0 seconds, CH bit is zero -> start oscillator
	ret |= ds1307_writeregs();
	// set wake up time to 00:00:00 01-01-2000
	rtc_state->rtc_wakeup_seconds = 0;
	rtc_state->rtc_wakeup_minutes = 0;
	rtc_state->rtc_wakeup_hours   = 0;
	rtc_state->rtc_wakeup_day     = 1;
	rtc_state->rtc_wakeup_month   = 1;
	rtc_state->rtc_wakeup_year    = 0;
	ret |= ds1307_setalarm(rtc_state->rtc_wakeup_year, rtc_state->rtc_wakeup_month, rtc_state->rtc_wakeup_day, rtc_state->rtc_wakeup_hours, rtc_state->rtc_wakeup_minutes, rtc_state->rtc_wakeup_seconds);
	ret |= ds1307_set_deepstandby(WAKEUP_NORMAL);
	return ret;

error2:
	return -1;
}

/*********************************************************************
 *
 * Real Time Clock driver.
 *
 * The main board has a battery backed up Dallas DS1307 compatible
 * RTC device.
 *
 */
static int hchs8100_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	time_t uTime = 0;  // seconds since Linux epoch

	dprintk(150, "%s >\n", __func__);
	uTime = read_rtc_time();
	rtc_time_to_tm(uTime, tm);
	dprintk(150, "%s < %d\n", __func__, uTime);
	return 0;
}

static int tm2time(struct rtc_time *tm)
{
	return mktime(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
}

static int hchs8100_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	int res = 0;

	time_t uTime = tm2time(tm);
	dprintk(150, "%s > uTime %d\n", __func__, uTime);
	write_rtc_time(uTime);
//	YWPANEL_FP_ControlTimer(true);
	dprintk(150, "%s < res: %d\n", __func__, res);
	return res;
}

static int hchs8100_rtc_read_alarm(struct device *dev, struct rtc_wkalrm *al)
{  /// TODO
	u32 a_time = 0;

	dprintk(150, "%s >\n", __func__);
//	a_time = YWPANEL_FP_GetPowerOnTime();
//	if (al->enabled)
//	{
//		rtc_time_to_tm(a_time, &al->time);
//	}
	dprintk(150, "%s < Enabled: %d RTC alarm time: %d time: %d\n", __func__, al->enabled, (int)&a_time, a_time);
	return 0;
}

static int hchs8100_rtc_set_alarm(struct device *dev, struct rtc_wkalrm *al)
{  // TODO
	u32 a_time = 0;

	dprintk(150, "%s >\n", __func__);
//	if (al->enabled)
//	{
//		a_time = tm2time(&al->time);
//	}
//	YWPANEL_FP_SetPowerOnTime(a_time);
	dprintk(150, "%s < Enabled: %d time: %d\n", __func__, al->enabled, a_time);
	return 0;
}

static const struct rtc_class_ops hchs8100_rtc_ops =
{
	.read_time  = hchs8100_rtc_read_time,
	.set_time   = hchs8100_rtc_set_time,
	.read_alarm = hchs8100_rtc_read_alarm,
	.set_alarm  = hchs8100_rtc_set_alarm
};

static int __devinit hchs8100_rtc_probe(struct platform_device *pdev)
{
	struct rtc_device *rtc;
	int ret;

//	dprintk(150, "%s >\n", __func__);
	dprintk(50, "Homecast DS1307 main board real time clock\n");
	// probe the DS1307
	ret = probe_ds1307();
	if (ret)
	{
		return -1;
	}
	/* I have no idea where the pdev comes from, but it needs the can_wakeup = 1
	 * otherwise we don't get the wakealarm sysfs attribute... :-) */
	pdev->dev.power.can_wakeup = 0;
	rtc = rtc_device_register("hchs8100_rtc", &pdev->dev, &hchs8100_rtc_ops, THIS_MODULE);
	if (IS_ERR(rtc))
	{
		return PTR_ERR(rtc);
	}
	platform_set_drvdata(pdev, rtc);
	dprintk(150, "%s < %p\n", __func__, rtc);
	return 0;
}

static int __devexit hchs8100_rtc_remove(struct platform_device *pdev)
{
	struct rtc_device *rtc = platform_get_drvdata(pdev);
	dprintk(150, "%s %p >\n", __func__, rtc);
	rtc_device_unregister(rtc);
	platform_set_drvdata(pdev, NULL);
	dprintk(150, "%s <\n", __func__);
	return 0;
}

static struct platform_driver hchs8100_rtc_driver =
{
	.probe = hchs8100_rtc_probe,
	.remove = __devexit_p(hchs8100_rtc_remove),
	.driver =
	{
		.name	= RTC_NAME,
		.owner	= THIS_MODULE
	}
};

// code of the threads
/*********************************************************************
 *
 * Timer thread code.
 *
 * The timer thread runs once every 29.5 seconds during simulated deep
 * standby. It will monitor the alarm status of the RTC.
 *
 * The thread will run until either:
 * - Requested to halt by the standby thread because the user
 *   has pressed a power button;
 * - The wakeup time minus WAKEUP_TIME is reached.
 *  
 * Code inspired by aotom_spark text thread code (thnx Martii)
 *
 */
int timer_thread(void *arg)
{
	int ret = 0;
	unsigned char wakeup_year;
	unsigned char wakeup_month;
	unsigned char wakeup_day;
	unsigned char wakeup_hours;
	unsigned char wakeup_minutes;
	unsigned char wakeup_seconds;
	unsigned char diff_day;
	unsigned char diff_hours;
	unsigned char diff_minutes;
	unsigned char diff_seconds;
	int i;
	time_t wakeup_t;
	time_t rtc_t;
	time_t diff_t;
	char *timeString;
	struct tm *diff_time;

	dprintk(150, "%s >\n", __func__);

	timer_state.status = THREAD_STATUS_INIT;

	timer_state.status = THREAD_STATUS_RUNNING;

	while (!kthread_should_stop())
	{
		if (!down_interruptible(&timer_state.sem))
		{
			if (kthread_should_stop())
			{
				goto stop;
			}
			while (!down_trylock(&timer_state.sem));
			{
				dprintk(50, "%s: Start timer thread, period = %d ms\n", __func__, timer_state.period);

				ret |= ds1307_getalarmtime(&wakeup_year, &wakeup_month, &wakeup_day, &wakeup_hours, &wakeup_minutes, &wakeup_seconds);
				// get wakeup time as a time_t
				wakeup_t  = 0;  // seconds
				wakeup_t += wakeup_minutes * MINUTE_SECS;
				wakeup_t += wakeup_hours * HOUR_SECS;
				wakeup_t += wakeup_day * DAY_SECS;
				for (i = 1; i < wakeup_month; i++)
				{
					wakeup_t += (_ytab[LEAPYEAR(wakeup_year + 2000)][i]) * DAY_SECS;
				}
				for (i = 1970; i < (wakeup_year + 2000); i++)  // handle epoch difference
				{
					wakeup_t += (YEARSIZE(i) * DAY_SECS);
				}
				wakeup_t -= WAKEUP_TIME;  // allow WAKEUP_TIME seconds to power up
				dprintk(50, "%s: Supplied wakeup date/time is %02d-%02d-20%02d %02d:%02d\n", __func__, wakeup_day, wakeup_month, wakeup_year, wakeup_hours, wakeup_minutes);
				diff_time = gmtime(wakeup_t);
				dprintk(50, "%s: Wake up date/time corrected with power up time is %02d-%02d-%04d %02d:%02d\n", __func__,
						diff_time->tm_mday, diff_time->tm_mon + 1, diff_time->tm_year + 1900, diff_time->tm_hour, diff_time->tm_min);

				while (timer_state.state
				&&     !kthread_should_stop())
				{
					msleep(timer_state.period);

					ds1307_readregs();  // get rtc_state
					// get RTC time as a time_t
					rtc_t  = rtc_state->rtc_seconds;  // = 0; seconds
					rtc_t += rtc_state->rtc_minutes * MINUTE_SECS;
					rtc_t += rtc_state->rtc_hours * HOUR_SECS;
					rtc_t += rtc_state->rtc_day * DAY_SECS;
					for (i = 1; i < rtc_state->rtc_month; i++)
					{
						rtc_t += (_ytab[LEAPYEAR(rtc_state->rtc_year + 2000)][i]) * DAY_SECS;
					}
					for (i = 1970; i < (rtc_state->rtc_year + 2000); i++)
					{
						rtc_t += YEARSIZE(i) * DAY_SECS;
					}
					diff_t = wakeup_t - rtc_t;

					if (diff_t < 0)
					{
						dprintk(10, "%s: Wake up time reached.\n", __func__);
						timer_state.state = 0;
						break;
					}
					else
					{
						if (paramDebug > 49)
						{
							i = 0;  // flag: year and/or month printed
							dprintk(1, "Deep standby: wake up in ");
							diff_time = gmtime(diff_t);

							if ((diff_time->tm_year - 70) > 0)
							{
								printk("%d year(s), "), diff_time->tm_year - 70;
								i = 1;
							}
							if (diff_time->tm_mon == 1)
							{
								printk("%d month, ", diff_time->tm_mon);
							}
							else if ((diff_time->tm_mon == 0 && i == 1)  // year printed
							||       (diff_time->tm_mon > 1))
							{
								printk("%d months, ", diff_time->tm_mon);
								i = 1;
							}
							if (diff_time->tm_mday == 1)
							{
								printk("%d day, ", diff_time->tm_mday);
							}
							else if ((diff_time->tm_mday == 0 && i == 1)  // year/month printed
							||       (diff_time->tm_mday > 1))
							{
								printk("%d days", diff_time->tm_mday);
								i = 1;
							}
							if (diff_time->tm_hour == 1)
							{
								printk("%d hour and ", diff_time->tm_hour);
							}
							else if (diff_time->tm_hour > 1 || i == 1)
							{
								printk("%d hours and ", diff_time->tm_hour);
							}
							if (diff_time->tm_min > 1)
							{
								printk("%d minutes%s\n", diff_time->tm_min, diff_time->tm_sec > 29 ? " and about 30 seconds." : ".");
							}
							else
							{
								printk("Less than a minute.\n");
							}
						}
					}
					// stop this thread when no standby thread is running
					if (standby_state.status == THREAD_STATUS_STOPPED
					||  standby_state.status == THREAD_STATUS_HALTED)
					{
						dprintk(50, "%s: Standby thread halted.\n", __func__);
						break;
					}						
				}
				timer_state.status = THREAD_STATUS_HALTED;
			}
			dprintk(50, "%s: Timer thread halted.\n", __func__);
		}
	}

stop:
	timer_state.status = THREAD_STATUS_STOPPED;
	dprintk(150, "%s: stopped\n", __func__);
	return ret;
}
// end of timer thread code

/*********************************************************************
 *
 * Standby thread code.
 *
 * The standby thread runs almost continuously once simulated deep
 * standby mode has been entered. It will monitor pressing a power
 * key and update the front panel display about twice a second.
 *
 * This thread will halt when the user presses the power button on
 * either the front panel or on the remote control.
 * In case a timer thread is also running, this thread will also
 * halt when the timer thread halts; that is when the wake up time
 * is reached.
 * 
 * Code inspired by aotom_spark text thread code (thnx Martii)
 *
 */
int standby_thread(void *arg)
{
	int ret = 0;
	int seconds_flag;
	char disp_string[VFD_DISP_SIZE];
	char colon_string[2];
	int i, j;

	dprintk(150, "%s > (period = %d ms)\n", __func__, standby_state.period);

	standby_state.status = THREAD_STATUS_INIT;

	standby_state.status = THREAD_STATUS_RUNNING;

	seconds_flag = 1000 / standby_state.period;  // calculate occurences per second
	if (seconds_flag < 2)
	{
		seconds_flag = 2;
	}

	while (!kthread_should_stop())
	{
		if (!down_interruptible(&standby_state.sem))
		{
			if (kthread_should_stop())
			{
				goto stop;
			}
			j = 0;
			while (!down_trylock(&standby_state.sem));
			{
				dprintk(50, "%s: Start standby thread, period = %d ms\n", __func__, standby_state.period);

				while (standby_state.state
				&&     !kthread_should_stop())
				{
					if (standby_state.state == 2
					&&  (timer_state.status == THREAD_STATUS_STOPPED
					  || timer_state.status == THREAD_STATUS_HALTED))
					{
						dprintk(50, "%s: Timer triggered.\n", __func__);
						standby_state.state = 0;
						break;
					}
					ret = get_fp_keyData();  // scan front panel keyboard
					if (ret == HCHS8100_BUTTON_POWER || rc_power_key == 1)
					{
						dprintk(50, "%s: Power key pressed.\n", __func__);
						standby_state.state = 0;
						break;
					}
					msleep(standby_state.period);
				}
				standby_state.status = THREAD_STATUS_HALTED;

				// power key pressed or timer triggered
				rc_power_key = 0;
				ret |= pt6302_set_icon(ICON_POWER, 0);
				ret |= pt6302_set_icon(ICON_TIMER, 0);
				ret |= pt6302_write_text(0, "[ Wake up ]", sizeof("[ Wake up ]"), 0);
				msleep(1000);
				// restart
				kernel_restart(NULL);
			}
			dprintk(50, "%s: Standby thread halted.\n", __func__);
		}
	}

stop:
	standby_state.status = THREAD_STATUS_STOPPED;
	standby_state.task = 0;
	dprintk(150, "%s: stopped\n", __func__);
	return ret;
}
// end of standby thread code

/*********************************************************************
 *
 * Spinner thread code.
 *
 */

/*********************************************************************
 *
 * spinner_thread: Thread to display the spinner on VFD.
 *
 * Taken from nuvoton driver.
 *
 */
static int spinner_thread(void *arg)
{
	int i;
	int ret = 0;
	unsigned char icon_char[1];

	if (spinner_state.status == THREAD_STATUS_RUNNING)
	{
		return 0;
	}
	dprintk(150, "%s: starting\n", __func__);
	spinner_state.status = THREAD_STATUS_INIT;

	dprintk(150, "%s: started\n", __func__);
	spinner_state.status = THREAD_STATUS_RUNNING;

	while (!kthread_should_stop())
	{
		if (!down_interruptible(&spinner_state.sem))
		{
			if (kthread_should_stop())
			{
				goto stop;
			}
			ret |= pt6302_set_icon(ICON_CIRCLE_0, 1);  // switch inner circle on
			while (!down_trylock(&spinner_state.sem));
			{
				dprintk(50, "%s: Start spinner, period = %d ms\n", __func__, spinner_state.period);
				while (spinner_state.state
				&&     !kthread_should_stop())
				{
					switch (i) // display a two rotating circles in 32 states
					{
						case 0:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_I1, 1);  // switch inner circle segment 1 on
							break;
						}
						case 1:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_I2, 1);  // switch inner circle segment 2 on
							break;
						}
						case 2:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_I3, 1);  // switch inner circle segment 3 on
							break;
						}
						case 3:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_I4, 1);  // switch inner circle segment 4 on
							break;
						}
						case 4:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_I5, 1);  // switch inner circle segment 5 on
							break;
						}
						case 5:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_I6, 1);  // switch inner circle segment 6 on
							break;
						}
						case 6:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_I7, 1);  // switch inner circle segment 7 on
							break;
						}
						case 7:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_I8, 1);  // switch inner circle segment 8 on
							break;
						}
						case 8:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_1, 1);  // switch outer circle segment 1 on
							break;
						}
						case 9:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_2, 1);  // switch outer circle segment 2 on
							break;
						}
						case 10:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_3, 1);  // switch outer circle segment 3 on
							break;
						}
						case 11:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_4, 1);  // switch outer circle segment 4 on
							break;
						}
						case 12:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_5, 1);  // switch outer circle segment 5 on
							break;
						}
						case 13:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_6, 1);  // switch outer circle segment 6 on
							break;
						}
						case 14:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_7, 1);  // switch outer circle segment 7 on
							break;
						}
						case 15:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_8, 1);  // switch outer circle segment 8 on
							break;
						}
						case 16:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_1, 0);  // switch outer circle segment 1 off
							break;
						}
						case 17:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_2, 0);  // switch outer circle segment 2 off
							break;
						}
						case 18:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_3, 0);  // switch outer circle segment 3 off
							break;
						}
						case 19:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_4, 0);  // switch outer circle segment 4 off
							break;
						}
						case 20:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_5, 0);  // switch outer circle segment 5 off
							break;
						}
						case 21:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_6, 0);  // switch outer circle segment 6 off
							break;
						}
						case 22:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_7, 0);  // switch outer circle segment 7 off
							break;
						}
						case 23:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_8, 0);  // switch outer circle segment 8 off
							break;
						}
						case 24:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_I1, 0);  // switch inner circle segment 1 off
							break;
						}
						case 25:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_I2, 0);  // switch inner circle segment 2 off
							break;
						}
						case 26:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_I3, 0);  // switch inner circle segment 3 off
							break;
						}
						case 27:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_I4, 0);  // switch inner circle segment 4 off
							break;
						}
						case 28:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_I5, 0);  // switch inner circle segment 5 off
							break;
						}
						case 29:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_I6, 0);  // switch inner circle segment 6 off
							break;
						}
						case 30:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_I7, 0);  // switch inner circle segment 7 off
							break;
						}
						case 31:
						{
							ret |= pt6302_set_icon(ICON_CIRCLE_I8, 0);  // switch inner circle segment 8 off
							break;
						}
					}
					i++;
					i %= 32;
					msleep(spinner_state.period);
				}
				// stop spinner
				ret |= pt6302_set_icon(ICON_CIRCLE_1, 0);   // switch outer circle segment 1 off
				ret |= pt6302_set_icon(ICON_CIRCLE_2, 0);   // switch outer circle segment 2 off
				ret |= pt6302_set_icon(ICON_CIRCLE_3, 0);   // switch outer circle segment 3 off
				ret |= pt6302_set_icon(ICON_CIRCLE_4, 0);   // switch outer circle segment 4 off
				ret |= pt6302_set_icon(ICON_CIRCLE_5, 0);   // switch outer circle segment 5 off
				ret |= pt6302_set_icon(ICON_CIRCLE_6, 0);   // switch outer circle segment 6 off
				ret |= pt6302_set_icon(ICON_CIRCLE_7, 0);   // switch outer circle segment 7 off
				ret |= pt6302_set_icon(ICON_CIRCLE_8, 0);   // switch outer circle segment 8 off
				ret |= pt6302_set_icon(ICON_CIRCLE_I1, 0);  // switch inner circle segment 1 off
				ret |= pt6302_set_icon(ICON_CIRCLE_I2, 0);  // switch inner circle segment 2 off
				ret |= pt6302_set_icon(ICON_CIRCLE_I3, 0);  // switch inner circle segment 3 off
				ret |= pt6302_set_icon(ICON_CIRCLE_I4, 0);  // switch inner circle segment 4 off
				ret |= pt6302_set_icon(ICON_CIRCLE_I5, 0);  // switch inner circle segment 5 off
				ret |= pt6302_set_icon(ICON_CIRCLE_I6, 0);  // switch inner circle segment 6 off
				ret |= pt6302_set_icon(ICON_CIRCLE_I7, 0);  // switch inner circle segment 7 off
				ret |= pt6302_set_icon(ICON_CIRCLE_I8, 0);  // switch inner circle segment 8 off
				ret |= pt6302_set_icon(ICON_CIRCLE_0, 0);   // switch inner circle off
				spinner_state.status = THREAD_STATUS_HALTED;
			}
			dprintk(50, "%s: Spinner off\n", __func__);
		}
	}

stop:
	spinner_state.status = THREAD_STATUS_STOPPED;
	spinner_state.task = 0;
	dprintk(150, "%s: stopped\n", __func__);
	return ret;
}
// end of spinner thread code

/*********************************************************************
 *
 * Module functions.
 *
 */
extern void create_proc_fp(void);
extern void remove_proc_fp(void);

//static void __exit vfd_module_exit(void)
static void fp_module_exit(void)
{
	printk(TAGDEBUG"Homecast HS8100 series front processor module unloading\n");

	remove_proc_fp();

	if ((vfd_text_state.status != THREAD_STATUS_STOPPED) && vfd_text_state.task)
	{
		dprintk(50, "Stopping VFD text write thread\n");
		kthread_stop(vfd_text_state.task);
	}

	if ((standby_state.status != THREAD_STATUS_STOPPED) && standby_state.task)
	{
		dprintk(50, "Stopping standby thread\n");
		kthread_stop(standby_state.task);
	}

	if ((timer_state.status != THREAD_STATUS_STOPPED) && timer_state.task)
	{
		dprintk(50, "Stopping timer thread\n");
		kthread_stop(timer_state.task);
	}

	if (!(spinner_state.status == THREAD_STATUS_STOPPED) && spinner_state.task)
	{
		dprintk(50, "Stopping spinner thread\n");
		kthread_stop(spinner_state.task);
	}

	while (vfd_text_state.status != THREAD_STATUS_STOPPED
	&&     standby_state.status  != THREAD_STATUS_STOPPED
	&&     timer_state.status    != THREAD_STATUS_STOPPED
	&&     spinner_state.status  != THREAD_STATUS_STOPPED)
	{
		msleep(1);
	}
	dprintk(50, "All threads stopped\n");

	// fan driver
	cleanup_fan_module();

	dprintk(50, "Unregister VFD character device %d\n", VFD_MAJOR);
	unregister_chrdev(VFD_MAJOR, "vfd");
	pt6302_free_pio_pins();
	dprintk(20, "Unregister front panel button device\n");
	button_dev_exit();
	dprintk(20, "Unregister RTC driver\n");
	platform_driver_unregister(&hchs8100_rtc_driver);
	platform_set_drvdata(rtc_pdev, NULL);
	platform_device_unregister(rtc_pdev);

	printk("HS8100 front panel module unloaded.\n");
}

static int __init fp_module_init(void)
{
	int error = 0;

	printk("HS8100 front panel driver\n");
	msleep(waitTime);

	// RTC device (note: must be done first, otherwise hchs8100_fp_init_func will fail)
	dprintk(50, "Register RTC driver\n");
	error |= platform_driver_register(&hchs8100_rtc_driver);
	if (error)
	{
		dprintk(1, "%s platform_driver_register failed: %d\n", __func__, error);
	}
	else
	{
		rtc_pdev = platform_device_register_simple(RTC_NAME, -1, NULL, 0);
	}

	if (IS_ERR(rtc_pdev))
	{
		dprintk(1, "%s platform_device_register_simple failed: %ld\n", __func__, PTR_ERR(rtc_pdev));
	}

	if (hchs8100_fp_init_func() != 0)
	{
		goto fp_module_init_fail;
	}
	// front panel button driver
	dprintk(50, "Register button device\n");
	if (button_dev_init() != 0)
	{
		return -1;
	}
	// vfd driver
	dprintk(50, "Register VFD character device %d\n", VFD_MAJOR);
	if (register_chrdev(VFD_MAJOR, "vfd", &vfd_fops))
	{
		dprintk(1, "%s: Unable to get major %d for vfd\n", __func__, VFD_MAJOR);
		goto fp_module_init_fail;
	}
	// fan driver
	init_fan_module();

	// initialize fan
	error |= hchs8100_set_fan(1, 0);  // low speed

	dprintk(50, "Initialize VFD write thread\n");
	sema_init(&vfd_text_state.sem, 1);  // initialize VFD text write semaphore

	dprintk(50, "Initialize standby thread\n");
	standby_state.state = 0;
	standby_state.period = 100;  // test every 100ms for power keys
	standby_state.status = THREAD_STATUS_STOPPED;
	sema_init(&standby_state.sem, 0);
	standby_state.task = kthread_run(standby_thread, (void *)ICON_SPINNER, "standby_thread");

	dprintk(50, "Initialize timer thread\n");
	timer_state.state = 0;
	timer_state.period = 59000;  // test every 29.5s for wakeup time
	timer_state.status = THREAD_STATUS_STOPPED;
	sema_init(&timer_state.sem, 0);
	timer_state.task = kthread_run(timer_thread, (void *)ICON_SPINNER, "timer_thread");

	dprintk(50, "Initialize spinner thread\n");
	spinner_state.period = 0;
	spinner_state.status = THREAD_STATUS_STOPPED;
	sema_init(&spinner_state.sem, 0);
	spinner_state.task = kthread_run(spinner_thread, (void *)ICON_SPINNER, "spinner_thread");

	memset(lastdata.icon_data, 0, sizeof(lastdata.icon_data));
	memset(lastdata.saved_icon_data, 0, sizeof(lastdata.saved_icon_data));

	create_proc_fp();
	dprintk(20, "Homecast HS8100 series front panel driver initialization successful\n");
	dprintk(150, "%s <\n", __func__);
	return 0;

fp_module_init_fail:
	dprintk(20, "Homecast HS8100 series front panel driver initialization failed\n");
	fp_module_exit();
	return -EIO;
}

module_init(fp_module_init);
module_exit(fp_module_exit);

module_param(paramDebug, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(paramDebug, "Debug Output 0=disabled (default), >1=enabled (level_");

module_param(waitTime, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(waitTime, "Wait before init in ms (default=1000)");

module_param(gmt_offset, charp, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(gmt_offset, "GMT offset (default 3600");

MODULE_DESCRIPTION("Homecast HS8100 series front panel driver");
MODULE_AUTHOR("Audioniek");
MODULE_LICENSE("GPL");
// vim:ts=4
