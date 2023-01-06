/*****************************************************************************
 *
 * hchs8100_fp.h
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
 * Header file for button, fan, RTC and VFD driver.
 *
 * Devices:
 *  - /dev/vfd (vfd ioctls and read/write function)
 *  - /dev/rtc0
 *
 *****************************************************************************/
#if !defined HCHS8100_FP_H
#define HCHS8100_FP_H

extern short paramDebug;
#define TAGDEBUG "[hchs8100_fp] "

#ifndef dprintk
#define dprintk(level, x...) do \
{ \
	if ((paramDebug) && (paramDebug >= level)) printk(TAGDEBUG x); \
} while (0)
#endif

/****************************************************************************************/

/***************************************************************
 *
 * Icons for VFD
 *
 */ 
enum  // icon numbers and their names
{
	ICON_MIN,         // 00
	ICON_CIRCLE_0,    // 01
	ICON_CIRCLE_1,    // 02
	ICON_CIRCLE_2,    // 03
	ICON_CIRCLE_3,    // 04
	ICON_CIRCLE_4,    // 05
	ICON_CIRCLE_5,    // 06
	ICON_CIRCLE_6,    // 07
	ICON_CIRCLE_7,    // 08
	ICON_CIRCLE_8,    // 09
	ICON_CIRCLE_I1,   // 10
	ICON_CIRCLE_I2,   // 11
	ICON_CIRCLE_I3,   // 12
	ICON_CIRCLE_I4,   // 13
	ICON_CIRCLE_I5,   // 14
	ICON_CIRCLE_I6,   // 15
	ICON_CIRCLE_I7,   // 16
	ICON_CIRCLE_I8,   // 17
	ICON_REC,         // 18
	ICON_TV,          // 19
	ICON_RADIO,       // 20
	ICON_DOLBY,       // 21
	ICON_POWER,       // 22
	ICON_TIMER,       // 23
	ICON_HDD_FRAME,   // 24
	ICON_HDD_1,       // 25
	ICON_HDD_2,       // 26
	ICON_HDD_3,       // 27
	ICON_HDD_4,       // 28
	ICON_HDD_5,       // 29
	ICON_HDD_6,       // 30
	ICON_HDD_7,       // 31
	ICON_HDD_8,       // 32
	ICON_HDD_9,       // 33
	ICON_HDD_A,       // 34
	ICON_MAX,         // 35
	ICON_SPINNER      // 36
};

#define VFD_MAJOR                  147
#define VFD_DISP_SIZE              12  // width of text area
#define ICON_WIDTH                 2   // width in characters of icon area

struct fp_driver
{
	struct semaphore sem;
	int              opencount;
};

// Thread stuff
#define THREAD_STATUS_RUNNING      0
#define THREAD_STATUS_STOPPED      1
#define THREAD_STATUS_INIT         2
#define THREAD_STATUS_HALTED       3  // (semaphore down)

// IOCTL definitions
#define VFDDISPLAYCHARS            0xc0425a00
#define VFDBRIGHTNESS              0xc0425a03
#define VFDDISPLAYWRITEONOFF       0xc0425a05
#define VFDDRIVERINIT              0xc0425a08
#define VFDICONDISPLAYONOFF        0xc0425a0a

#define VFDSETPOWERONTIME          0xc0425af6
#define VFDSETFAN                  0xc0425af8
#define VFDGETWAKEUPMODE           0xc0425af9
#define VFDGETTIME                 0xc0425afa
#define VFDSETTIME                 0xc0425afb
#define VFDSTANDBY                 0xc0425afc
#define VFDSETMODE                 0xc0425aff
#define VFDGETWAKEUPTIME           0xc0425b03
#define VFDSETREMOTEKEY            0xc0425b05

// Generic STPIO stuff
#define PIO_PORT_SIZE              0x1000
#define PIO_BASE                   0xb8020000
#define STPIO_SET_OFFSET           0x04
#define STPIO_CLEAR_OFFSET         0x08
#define STPIO_POUT_OFFSET          0x00

#define STPIO_SET_PIN(PIO_ADDR, PIN, V) \
	writel(1 << PIN, PIO_ADDR + STPIO_POUT_OFFSET + ((V) ? STPIO_SET_OFFSET : STPIO_CLEAR_OFFSET))
#define PIO_PORT(n) \
	(((n) * PIO_PORT_SIZE) + PIO_BASE)

// PT6302 PIO pin definitions
#define PT6302_CLK_PORT            1
#define PT6302_CLK_PIN             1

#define PT6302_DIN_PORT            1
#define PT6302_DIN_PIN             2

#define PT6302_CS_PORT             1
#define PT6302_CS_PIN              0

// fan
#define FAN_ON_PORT                5
#define FAN_ON_PIN                 4

#define FAN_SPEED_PORT             5
#define FAN_SPEED_PIN              3

/***************************************************************
 *
 * Definitions for the PT6302 VFD controller
 *
 */ 
// Commands to the PT6302
#define DISP_CTLCMD                0x80
// Options for DATA_CTLCMD
#define DISPLAY_ON                 0x08

#define PT6302_COMMAND_DCRAM_WRITE 1
#define PT6302_COMMAND_CGRAM_WRITE 2
#define PT6302_COMMAND_ADRAM_WRITE 3
#define PT6302_COMMAND_SET_PORTS   4

#define PT6302_COMMAND_SET_DUTY    5
// options for PT6302_COMMAND_SET_DUTY
#define PT6302_DUTY_MIN            0
#define PT6302_DUTY_MAX            7

#define PT6302_COMMAND_SET_DIGITS  6
// options for PT6302_COMMAND_SET_DIGITS
#define PT6302_DIGITS_MIN          9
#define PT6302_DIGITS_MAX         16
#define PT6302_DIGITS_OFFSET       8

#define PT6302_COMMAND_SET_LIGHT   7
// options for PT6302_COMMAND_SET_LIGHT
#define PT6302_LIGHT_NORMAL        0
#define PT6302_LIGHT_OFF           1
#define PT6302_LIGHT_ON            3

#define PT6302_COMMAND_TESTMODE    8  // do not use

#define VFD_Delay                  8  // us, tDOFF for PT6302

/***************************************************************
 *
 * Definitions for the DS1307 RTC
 *
 */ 
#define RTC_I2CBUS                 2
#define RTC_I2CADDR                0x68
// Register definitions
#define DS1307_REG_SECS            0x00  // 00-59
#define DS1307_BIT_CH              0x80  // in REG_SECS, write as 0 to start oscillator
#define DS1307_REG_MINS            0x01  // 00-59
#define DS1307_REG_HOURS           0x02  // 00-23, or 1-12 in AM/PM mode
#define DS1307_BIT_AMPM            0x40  // in REG_HOUR, 0 = 24h mode, 1 = AM/PM mode
#define DS1307_BIT_PM              0x20  // in REG_HOUR, indicates PM in AM/PM mode, else hours tens bit 1,
#define DS1307_REG_WDAY            0x03  // 01-07
#define DS1307_REG_MDAY            0x04  // 01-31
#define DS1307_REG_MONTH           0x05  // 01-12
#define DS1307_REG_YEAR            0x06  // 00-99
#define DS1307_REG_CONTROL         0x07
#define DS1307_BIT_OUT             0x80  // state of OUT pin when square wave is disabled
#define DS1307_BIT_SQWE            0x10  // square wave output is on when 1
#define DS1307_BIT_RS1             0x02  // determines square wave output frequency:
#define DS1307_BIT_RS0             0x01  // 00 = 1 Hz, 01 = 4096 Hz, 10 = 8192 Hz, 11 = 32768 Hz
#define DS1307_RAM_SIZE            0x40 - DS1307_REG_CONTROL - 1
// DS1307 RAM locations used by this driver
#define DS1307_REG_WAKEUP_SECS     0x08  // 00-59
#define DS1307_REG_WAKEUP_MINS     0x09  // 00-59
#define DS1307_REG_WAKEUP_HOURS    0x0a  // 00-23
#define DS1307_REG_WAKEUP_DAY      0x0b  // 01-31
#define DS1307_REG_WAKEUP_MONTH    0x0c  // 01-12
#define DS1307_REG_WAKEUP_YEAR     0x0d  // 00-99
#define DS1307_REG_STANDBY_L       0x0e  // WAKEUP_DEEPSTDBY & 0xff if set
#define DS1307_REG_STANDBY_H       0x0f  // WAKEUP_DEEPSTDBY >> 8 if set
// DS1307 flags for rtc_deep_standby
#define WAKEUP_NORMAL              0
#define WAKEUP_DEEPSTDBY           0xaa55
#define WAKEUP_INVALID             0x5a5a

#define WAKEUP_TIME                120  // seconds, receiver will wake up this period before the actual wake up time

// wake up mode
enum  // define wake up mode numbers
{
	WAKEUP_UNKNOWN,      // 00
	WAKEUP_POWERON,      // 01
	WAKEUP_DEEPSTANDBY,  // 02
	WAKEUP_TIMER         // 03
};

struct ds1307_state
{
	struct i2c_adapter *rtc_i2c;
	unsigned char      rtc_i2c_address;
	unsigned char      rtc_i2c_bus;
	unsigned char      rtc_reg[8];  // actual register values (mostly BCD)
	unsigned char      rtc_hours;  // binary values
	unsigned char      rtc_minutes;
	unsigned char      rtc_seconds;
	unsigned char      rtc_year;
	unsigned char      rtc_month;
	unsigned char      rtc_day;
	unsigned char      rtc_weekday;
	unsigned char      rtc_pm;  // AM/PM bit
	unsigned char      rtc_ampmmode;  // 12/24 hour mode
	unsigned char      rtc_wakeup_hours;  // binary values, but stored in RTC as BCD
	unsigned char      rtc_wakeup_minutes;
	unsigned char      rtc_wakeup_seconds;
	unsigned char      rtc_wakeup_year;
	unsigned char      rtc_wakeup_month;
	unsigned char      rtc_wakeup_day;
	int                rtc_deep_standby;
};

// generic time stuff
#define LEAPYEAR(year) (!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define YEARSIZE(year) (LEAPYEAR(year) ? 366 : 365)
static const int _ytab[2][12] =
{
	{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
	{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

#define DAY_SECS                   86400
#define HOUR_SECS                  3600
#define MINUTE_SECS                60

// Front button interface PIO pin definitions
#define HC164_DIN_PORT             1
#define HC164_DIN_PIN              5

#define HC164_CLK_PORT             1
#define HC164_CLK_PIN              6

#define KEY_DATA_PORT              1
#define KEY_DATA_PIN               7

extern struct stpio_pin *button_data_in;   // HC164 A & B inputs
extern struct stpio_pin *button_clk;       // HC164 clock
extern struct stpio_pin *button_key_data;  // button data

// Front button key masks
#define HCHS8100_BUTTON_UP           0x01
#define HCHS8100_BUTTON_DOWN         0x02
#define HCHS8100_BUTTON_LEFT         0x04
#define HCHS8100_BUTTON_RIGHT        0x08
#define HCHS8100_BUTTON_POWER        0x10
#define HCHS8100_BUTTON_MENU         0x20
#define HCHS8100_BUTTON_EXIT         0x40
#define HCHS8100_BUTTON_NONE         0x80

/****************************************************************************************/

struct set_brightness_s
{
	int level;
};

struct set_icon_s
{
	int icon_nr;
	int on;
};

struct set_light_s
{
	int onoff;
};

struct set_fan_s
{
	int speed;
};

/* time must be given as follows:
 * time[0] & time[1] = MJD
 * time[2] = hour
 * time[3] = min
 * time[4] = sec
 */
struct set_standby_s
{
	char time[5];
};

struct set_time_s
{
	char time[5];
};

struct set_remote_s
{
	char key;
};

/* This will set the mode temporarily (for one ioctl)
 * to the desired mode. Currently the "normal" mode
 * is the compatible vfd mode
 */
struct set_mode_s
{
	int compat; /* 0 = compatibility mode to vfd driver; 1 = homecast mode */
};

struct hchs8100_fp_ioctl_data
{
	union
	{
		struct set_icon_s icon;
		struct set_brightness_s brightness;
		struct set_light_s light;
		struct set_mode_s mode;
		struct set_standby_s standby;
		struct set_time_s time;
		struct set_fan_s fan;
		struct set_remote_s remote_key;
	} u;
};

struct vfd_ioctl_data
{
	unsigned char address;
	unsigned char data[64];
	unsigned char length;
};

// struct to store current values
struct saved_data_s
{
	int length;
	char vfddata[VFD_DISP_SIZE];
	int icon_state[ICON_MAX];  // the state of each icon
	unsigned char icon_data[ICON_WIDTH * 5];  // holds the CGRAM bit patterns for icon display
	unsigned char adram;  // holds the ADRAM data for icon ICON_POWER and ICON_TIMER
	int saved_icon_state[ICON_MAX];  // the state of each icon during standby
	unsigned char saved_icon_data[ICON_WIDTH * 5];  // holds the CGRAM bit patterns for icon display during standby
};

// structs for the threads
typedef struct
{
	int state;
	int period;
	int status;
	struct task_struct *task;
	struct semaphore sem;
} tStandbyState;

typedef struct
{
	int state;
	int period;
	int status;
	struct task_struct *task;
	struct semaphore sem;
} tTimerState;

typedef struct
{
	int state;
	int period;
	int status;
	struct task_struct *task;
	struct semaphore sem;
} tVFDState;

typedef struct
{
	int state;
	int period;
	int status;
	struct task_struct *task;
	struct semaphore sem;
} tSpinnerState;

// structure of icon data table
struct iconToInternal
{
	char *name;
	u16 icon;
	int pos;
	int column;
	int mask;
};

// symbols shared between modules
extern int hchs8100_fp_init_func(void);
extern void pt6302_free_pio_pins(void);
extern int pt6302_write_dcram(unsigned char addr, unsigned char *data, unsigned char len);
extern int pt6302_write_cgram(unsigned char addr, unsigned char *data, unsigned char len);
extern int pt6302_write_text(unsigned char offset, unsigned char *text, unsigned char len, int utf8_flag);
extern int pt6302_set_icon(int icon_nr, int on);
extern int hchs8100_set_fan(int on, int speed);
extern time_t read_rtc_time(void);
extern void ds1307_readregs(void);
extern int ds1307_readreg(u8 reg);
extern int ds1307_writereg(unsigned char reg, unsigned char value);
extern int ds1307_getalarmtime(unsigned char *year, unsigned char *month, unsigned char *day, unsigned char *hours, unsigned char *minutes, unsigned char *seconds);
extern int ds1307_getalarmstate(void);
extern int ds1307_setalarm(unsigned char year, unsigned char month, unsigned char day, unsigned char hours, unsigned char minutes, unsigned char seconds);
extern int rtc_is_invalid(void);
extern int ds1307_get_deepstandby(int *dstby_flag);
extern int ds1307_set_deepstandby(int dstby_flag);
extern int write_rtc_time(time_t uTime);
extern struct tm *gmtime(register const time_t time);
extern struct file_operations vfd_fops;
extern struct saved_data_s lastdata;
extern int standby_thread(void *arg);
extern int timer_thread(void *arg);
extern int rtc_offset;
extern int rc_power_key;
extern int wakeup_mode;

#endif  // HCHS8100_FP_H
// vim:ts=4
