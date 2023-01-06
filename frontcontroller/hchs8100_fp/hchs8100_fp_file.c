/*****************************************************************************
 *
 * hchs8100_fp_file.c
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
 * Button, fan, RTC and VFD driver, file part.
 *
 * The front panel has a 12 character dot matrix VFD with 34 icons
 * driven by a PIO controlled PT6302-007.
 *
 * The button interface is a simple circuit around a 74HC164 shift
 * register that is also controlled by two PIO pins; a third pin reads
 * the key data.
 *
 * The driver also supports the Dallas DS1307 compatible battery backed
 * up real time clock on the main board masquerading as the front panel
 * clock. In addition eight RTC RAM locations are used to create a
 * simulated deep standby mechanism including reporting of the correct
 * wake up reason.
 *
 * The remote control is not handled in this driver and LiRC is used to
 * process it.
 *
 * All RTC clock times in this driver are in UTC. As the DS1307 RTC does not
 * feature a century, this driver assumes the RTC century to be always 20,
 * resulting in an RTC epoch of 00:00:00 01-01-2000.
 *
 * Devices:
 *  - /dev/vfd (vfd ioctls and read/write function)
 *  - /dev/rtc0 (real time clock)
 *
 *****************************************************************************
 *
 * Changes
 *
 * Date     By              Description
 * --------------------------------------------------------------------------------------
 * 20220908 Audioniek       Initial version.
 * 20220910 Audioniek       Apart from ICON_POWER and ICON timer: icons working.
 * 20220913 Audioniek       Fan control operating as designed.
 * 20220921 Audioniek       VFDSETTIME, VFDGETIME and RTC routines working.
 * 20220923 Audioniek       VFDSETPOWERONTIME and VFDGETWAKEUPTIME working.
 * 20220927 Audioniek       ICON_POWER and ICON_TIMER work.
 * 20220928 Audioniek       VFDGETWAKEMODE added.
 * 20220928 Audioniek       Fix bug in pt6302_write_adram.
 * 20221005 Audioniek       Determination of wake up mode moved to startup, works now.
 * 20221013 Audioniek       Several small bugs fixed in RTC time handling.
 * 20221107 Audioniek       Add toggle icon state (on = 2).
 * 20221229 Audioniek       Initial release version.
 *
 ****************************************************************************************/
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/delay.h>
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
#include <linux/time.h>
#include <linux/reboot.h>
#include <linux/i2c.h>
#include <linux/syscalls.h>
#include <linux/reboot.h>
#include <linux/kthread.h>

#include "pt6302_utf.h"
#include "hchs8100_fp.h"

spinlock_t mr_lock = SPIN_LOCK_UNLOCKED;

static struct fp_driver fp;

struct saved_data_s lastdata;

// For PT6302
struct stpio_pin *pt6302_din;
struct stpio_pin *pt6302_clk;
struct stpio_pin *pt6302_cs;

typedef union
{
	struct
	{
		uint8_t addr: 4, cmd: 4;  // bits 0-3 = addr, bits 4-7 = cmd
	} dcram;  // PT6302_COMMAND_DCRAM_WRITE command

	struct
	{
		uint8_t addr: 3, reserved: 1, cmd: 4;  // bits 0-2 = addr, bit 3 = *, bits 4-7 = cmd
	} cgram;  // PT6302_COMMAND_CGRAM_WRITE command

	struct
	{
		uint8_t addr: 4, cmd: 4;  // bits 0-3 = addr, bits 4-7 = cmd
	} adram;  // PT6302_COMMAND_ADRAM_WRITE command

	struct
	{
		uint8_t port1: 1, port2: 1, reserved: 2, cmd: 4;  // bits 0 = port 1, bit 1 = port 2, bit 2-3 = *, bits 4-7 = cmd
	} port;  // PT6302_COMMAND_SET_PORTS command

	struct
	{
		uint8_t duty: 3, reserved: 1, cmd: 4;
	} duty;  // PT6302_COMMAND_SET_DUTY command

	struct
	{
		uint8_t digits: 3, reserved: 1, cmd: 4;
	} digits;  // PT6302_COMMAND_SET_DIGITS command

	struct
	{
		uint8_t onoff: 2, reserved: 2, cmd: 4;
	} light;  // PT6302_COMMAND_SET_LIGHT command

	uint8_t all;
} pt6302_command_t;

int wakeup_mode = WAKEUP_UNKNOWN;
char *wakeupmode_name[4] = { "Unknown", "Power on", "From deep standby", "Timer" };
char wakeup_time[5];

// standby thread
extern struct ds1307_state *rtc_state;
extern tStandbyState standby_state;
extern int rc_power_key;

// timer thread
extern tTimerState timer_state;

// VFD write thread
extern tVFDState vfd_text_state;
static struct vfd_ioctl_data last_vfd_draw_data;

// spinner thread
extern tSpinnerState spinner_state;

/***************************************************************
 *
 * Icons for PT6302 VFD
 *
 * The icons apart from ICON_POWER and ICON_TIMER are
 * connected to the PT6302 on character positions 12 and 13 and
 * each is the equivalent of one pixel in the nominal character
 * matrix of 5 column by 7 rows.
 *
 * These icons can be controlled by displaying one of the CGRAM
 * characters (0x00 through 0x07) on these positions and changing
 * the corresponding bit positions in CGRAM.
 *
 * A character on the VFD display is simply a pixel matrix
 * consisting of five columns of seven pixels high.
 * Column 1 is the leftmost.
 *
 * Format of pixel data:
 *
 * pixeldata1: column 1
 *  ...
 * pixeldata5: column 5
 *
 * Each bit in the pixel data represents one pixel in the column,
 * with the LSbit being the top pixel and bit 6 the bottom one.
 * The pixel will be on when the corresponding bit is one.
 * The resulting map is:
 *
 *  Column -->    0     1     2     3     4
 *              -----------------------------
 *  Bitmask     0x01  0x01  0x01  0x01  0x01
 *     |        0x02  0x02  0x02  0x02  0x02
 *     |        0x04  0x04  0x04  0x04  0x04
 *     V        0x08  0x08  0x08  0x08  0x08
 *              0x10  0x10  0x10  0x10  0x10
 *              0x20  0x20  0x20  0x20  0x20
 *              0x40  0x40  0x40  0x40  0x40
 *
 * The driver uses the CGRAM controlled characters 0x01 and 0x02
 * on the display positions 12 and 13 to control the icons by
 * changing the bit patterns in CGRAM. A single icon is therefore
 * characterized by its display position (12 or 13), its pixel column
 * number in the character used on position 12 or 13, and the pixel
 * row of that character. The latter translates to a bit mask used
 * to (re)set the bit in the CGRAM data.
 *
 * To display an icon, its corresponding bit pattern is written
 * in the correct position in CGRAM.
 * Switching an icon off is achieved by rewriting its bit in
 * the CGRAM to zero.
 *
 * The icons ICON_POWER and ICON_TIMER are controlled differently
 * as these are connected to AD outputs instead of segment outputs
 * of the PT6302. They are both located on grid 13 and controlled
 * by writing the appropriate data in ADRAM. In the table below,
 * if the column number is 0xff, the icon is ADRAM controlled.
 *
 */
struct iconToInternal vfdIcons[] =
{
	/*- Name ---------- icon# -------- pos column mask ---*/
	{ "ICON_MIN",       ICON_MIN,       00, 0x00, 0x00 },  // 00
	{ "ICON_CIRCLE_0",  ICON_CIRCLE_0,  12, 0x03, 0x08 },  // 01
	{ "ICON_CIRCLE_1",  ICON_CIRCLE_1,  12, 0x01, 0x04 },  // 02
	{ "ICON_CIRCLE_2",  ICON_CIRCLE_2,  12, 0x00, 0x08 },  // 03
	{ "ICON_CIRCLE_3",  ICON_CIRCLE_3,  12, 0x01, 0x10 },  // 04
	{ "ICON_CIRCLE_4",  ICON_CIRCLE_4,  12, 0x00, 0x20 },  // 05
	{ "ICON_CIRCLE_5",  ICON_CIRCLE_5,  12, 0x04, 0x10 },  // 06
	{ "ICON_CIRCLE_6",  ICON_CIRCLE_6,  12, 0x00, 0x10 },  // 07
	{ "ICON_CIRCLE_7",  ICON_CIRCLE_7,  12, 0x04, 0x04 },  // 08
	{ "ICON_CIRCLE_8",  ICON_CIRCLE_8,  12, 0x00, 0x04 },  // 09
	{ "ICON_CIRCLE_I1", ICON_CIRCLE_I1, 12, 0x03, 0x04 },  // 10
	{ "ICON_CIRCLE_I2", ICON_CIRCLE_I2, 12, 0x02, 0x08 },  // 11
	{ "ICON_CIRCLE_I3", ICON_CIRCLE_I3, 12, 0x03, 0x10 },  // 12
	{ "ICON_CIRCLE_I4", ICON_CIRCLE_I4, 12, 0x02, 0x20 },  // 13
	{ "ICON_CIRCLE_I5", ICON_CIRCLE_I5, 12, 0x01, 0x20 },  // 14
	{ "ICON_CIRCLE_I6", ICON_CIRCLE_I6, 12, 0x02, 0x10 },  // 15
	{ "ICON_CIRCLE_I7", ICON_CIRCLE_I7, 12, 0x01, 0x08 },  // 16
	{ "ICON_CIRCLE_I8", ICON_CIRCLE_I8, 12, 0x02, 0x04 },  // 17
	{ "ICON_REC",       ICON_REC,       12, 0x04, 0x08 },  // 18
	{ "ICON_TV",        ICON_TV,        13, 0x02, 0x01 },  // 19
	{ "ICON_RADIO",     ICON_RADIO,     13, 0x01, 0x01 },  // 10
	{ "ICON_DOLBY",     ICON_DOLBY,     13, 0x00, 0x01 },  // 21
	{ "ICON_POWER",     ICON_POWER,     13, 0xff, 0x01 },  // 22 controlled through ADRAM
	{ "ICON_TIMER",     ICON_TIMER,     13, 0xff, 0x02 },  // 23 controlled through ADRAM
	{ "ICON_HDD_FRAME", ICON_HDD_FRAME, 13, 0x02, 0x40 },  // 24
	{ "ICON_HDD_1",     ICON_HDD_1,     13, 0x01, 0x40 },  // 25
	{ "ICON_HDD_2",     ICON_HDD_2,     13, 0x00, 0x40 },  // 26
	{ "ICON_HDD_3",     ICON_HDD_3,     13, 0x04, 0x20 },  // 27
	{ "ICON_HDD_4",     ICON_HDD_4,     13, 0x03, 0x20 },  // 28
	{ "ICON_HDD_5",     ICON_HDD_5,     13, 0x02, 0x20 },  // 29
	{ "ICON_HDD_6",     ICON_HDD_6,     13, 0x01, 0x20 },  // 30
	{ "ICON_HDD_7",     ICON_HDD_7,     13, 0x00, 0x20 },  // 31
	{ "ICON_HDD_8",     ICON_HDD_8,     13, 0x04, 0x10 },  // 32
	{ "ICON_HDD_9",     ICON_HDD_9,     13, 0x03, 0x10 },  // 33
	{ "ICON_HDD_A",     ICON_HDD_A,     13, 0x02, 0x10 },  // 34
};
// End of icon definitions


/******************************************************
 *
 * Routines to communicate with the PT6302
 *
 */

/******************************************************
 *
 * Free the PT6302 PIO pins
 *
 */
void pt6302_free_pio_pins(void)
{
	if (pt6302_cs)
	{
		stpio_set_pin(pt6302_cs, 1);  // deselect PT6302
	}
	if (pt6302_din)
	{
		stpio_free_pin(pt6302_din);
	}
	if (pt6302_clk)
	{
		stpio_free_pin(pt6302_clk);
	}
	if (pt6302_cs)
	{
		stpio_free_pin(pt6302_cs);
	}
};

/******************************************************
 *
 * Initialize the front panel PIO pins
 *
 */
static unsigned char fp_init_pio_pins(void)
{

//	PT6302 Chip select
	pt6302_cs = stpio_request_pin(PT6302_CS_PORT, PT6302_CS_PIN, "PT6302_CS", STPIO_OUT);
	if (pt6302_cs == NULL)
	{
		dprintk(1, "%s: Request for PT6302 CS STPIO failed; abort\n", __func__);
		goto pt_init_fail;
	}
//	PT6302 Clock
	pt6302_clk = stpio_request_pin(PT6302_CLK_PORT, PT6302_CLK_PIN, "PT6302_CLK", STPIO_OUT);
	if (pt6302_clk == NULL)
	{
		dprintk(1, "%s: Request for PT6302 CLK STPIO failed; abort\n", __func__);
		goto pt_init_fail;
	}
//	PT6302 Data in
	pt6302_din = stpio_request_pin(PT6302_DIN_PORT, PT6302_DIN_PIN, "PT6302_DIN", STPIO_OUT);
	if (pt6302_din == NULL)
	{
		dprintk(1, "%s: Request for PT6302 Din STPIO failed; abort\n", __func__);
		goto pt_init_fail;
	}
//	set all involved PIO pins high
	stpio_set_pin(pt6302_cs, 1);
	stpio_set_pin(pt6302_clk, 1);
	stpio_set_pin(pt6302_din, 1);
	return 1;

pt_init_fail:
	pt6302_free_pio_pins();
	return 0;
};

/****************************************************
 *
 * Write one byte to the PT6302
 *
 */
static void pt6302_send_byte(unsigned char value)
{
	int i;

	stpio_set_pin(pt6302_cs, 0);  // set chip select
	for (i = 0; i < 8; i++)  // 8 bits in a byte, LSB first
	{
		stpio_set_pin(pt6302_din, value & 0x01);  // write bit
		stpio_set_pin(pt6302_clk, 0);  // toggle
		udelay(VFD_Delay);
		stpio_set_pin(pt6302_clk, 1);  // clock pin
		udelay(VFD_Delay);
		value >>= 1;  // get next bit
	}
	stpio_set_pin(pt6302_cs, 1);  // deselect
	udelay(VFD_Delay * 2);
}

/****************************************************
 *
 * Write len bytes to the PT6302
 *
 */
static void pt6302_WriteData(uint8_t *data, unsigned int len)
{
	int i, j;

	stpio_set_pin(pt6302_cs, 0);  // select PT6302

	dprintk(150, "%s data:", __func__);
	for (i = 0; i < len; i++)
	{
		if (paramDebug > 149)
		{
			printk(" 0x%02x", data[i]);
		}
		for (j = 0; j < 8; j++)  // 8 bits in a byte, LSB first
		{
			stpio_set_pin(pt6302_din, data[i] & 0x01);  // write bit
			stpio_set_pin(pt6302_clk, 0);  // toggle
			udelay(VFD_Delay);
			stpio_set_pin(pt6302_clk, 1);  // clock pin
			udelay(VFD_Delay);
			data[i] >>= 1;  // get next bit
		}
		if (paramDebug > 149)
		{
			printk("\n");
		}
	}
	stpio_set_pin(pt6302_cs, 1);  // and deselect PT6302
	udelay(VFD_Delay * 2);  // CS hold time
	if (paramDebug > 150)
	{
		printk(")\n");
	}
}

/******************************************************
 *
 * PT6302 VFD functions
 *
 */

/******************************************************
 *
 * Write PT6302 DCRAM (display text)
 *
 * The leftmost character on the VFD display is
 * digit position or PT6302 character address zero.
 *
 * addr is used as the direct PT6302 character address;
 * data[0] is written at address addr.
 *
 */
int pt6302_write_dcram(unsigned char addr, unsigned char *data, unsigned char len)
{
	int              i;
	uint8_t          wdata[17]; // = { 0 };
	pt6302_command_t cmd;

	if (len == 0)  // if length is zero,
	{
		return 0; // do nothing
	}
	if (addr > VFD_DISP_SIZE + ICON_WIDTH)
	{
		dprintk(1, "%s Error: addr too large (max. %d)\n", __func__, VFD_DISP_SIZE + ICON_WIDTH - 1);
		return 0;  // do nothing
	}
	if (len + addr > VFD_DISP_SIZE + ICON_WIDTH)
	{
		dprintk(1, "%s: len (%d) too large, adjusting to %d\n", __func__, len, VFD_DISP_SIZE  + ICON_WIDTH - addr);
		len = VFD_DISP_SIZE + ICON_WIDTH - addr;
	}
	spin_lock(&mr_lock);

	// assemble command
	cmd.dcram.cmd  = PT6302_COMMAND_DCRAM_WRITE;
	cmd.dcram.addr = (addr & 0x0f);  // get address (= character position)

	if (addr < VFD_DISP_SIZE)  // assume text display
	{
		memset(wdata, 0x20, VFD_DISP_SIZE);  // fill text field with spaces
	}
	wdata[0] = (uint8_t)cmd.all;

	for (i = 0; i < len; i++)
	{
		wdata[i + 1] = data[i];  // copy text/data to display
	}
	pt6302_WriteData(wdata, 1 + len);

	spin_unlock(&mr_lock);
	return 0;
}

/****************************************************
 *
 * Write PT6302 ADRAM (set underline data)
 *
 * In the display, AD2 (bit 1) is supposed to be
 * connected to underline segments.
 * addr = starting digit number (0 = rightmost)
 *
 * NOTE: Display tube does not have underline
 * segments but two icons are controlled through
 * ADRAM.
 *
 */
int pt6302_write_adram(unsigned char addr, unsigned char *data, unsigned char len)
{
	pt6302_command_t cmd;
	uint8_t          wdata[20] = { 0x0 };
	int              i = 0;

	dprintk(150, "%s > addr = 0x%02x, len = %d\n", __func__, (int)addr, (int)len);

	spin_lock(&mr_lock);

	cmd.adram.cmd  = PT6302_COMMAND_ADRAM_WRITE;
	cmd.adram.addr = (addr & 0xf);

	wdata[0] = (uint8_t)cmd.all;
	dprintk(150, "%s: wdata = 0x%02x", __func__, wdata[0]);
	if (len > VFD_DISP_SIZE + ICON_WIDTH - addr)  // 12 + 2 - 13 = 1
	{
		len = VFD_DISP_SIZE + ICON_WIDTH - addr;
		dprintk(1, "%s: len too large, corrected to 0x%02x", __func__, len);
	}
#if 0
	for (i = 0; i < (int)len; i++)
	{
		wdata[i + 1] = data[i] & 0x03;
		if (paramDebug > 149)
		{
			printk(" 0x%02x", wdata[i + 1]);
		}
	}
	if (paramDebug > 149)
	{
		printk("\n");
	}
#endif
	pt6302_WriteData(wdata, len + 1);
	spin_unlock(&mr_lock);
	return 0;
}

/****************************************************
 *
 * Write PT6302 CGRAM (set symbol pixel data)
 *
 * addr = symbol number (0..7)
 * data = symbol data (5 bytes for each column,
 *        column 0 (leftmost) first, MSbit ignored)
 * len  = number of symbols to define pixels
 *        of (maximum is 8 - addr)
 *
 * Note: this routine is universal for the PT6302;
 *       in the HS8100 series however, only two
 *       of the display positions actually contain
 *       icons.
 */
int pt6302_write_cgram(unsigned char addr, unsigned char *data, unsigned char len)
{
	pt6302_command_t cmd;
	uint8_t          wdata[41];
	uint8_t          i = 0, j = 0;

	if (len == 0)
	{
		return 0;
	}
	spin_lock(&mr_lock);

	cmd.cgram.cmd  = PT6302_COMMAND_CGRAM_WRITE;
	cmd.cgram.addr = (addr & 0x7);
	wdata[0] = (uint8_t)cmd.all;

	if (len > 8 - addr)
	{
		len = 8 - addr;
	}
	for (i = 0; i < len; i++)
	{
		for (j = 0; j < 5; j++)
		{
			wdata[i * 5 + j + 1] = data[i * 5 + j];
		}
	}
	dprintk(150, "%s: wdata =", __func__);
	for (i = 0; i< len * 5 + 1; i++)
	{
		if (paramDebug > 149)
		{
			printk(" %02x", wdata[i]);
		}
	}
	if (paramDebug > 149)
	{
		printk("\n");
	}
	pt6302_WriteData(wdata, len * 5 + 1);
	spin_unlock(&mr_lock);
	return 0;
}
// end of PT6302 routines

/******************************************************
 *
 * Convert UTF8 formatted text to display text for
 * PT6302.
 *
 * Returns corrected length.
 *
 */
int pt6302_utf8conv(unsigned char *text, unsigned char len)
{
	int i;
	int wlen;
	unsigned char kbuf[64];
	unsigned char *UTF_Char_Table = NULL;

	wlen = 0;  // input index
	i = 0;  // output index
	memset(kbuf, 0x20, sizeof(kbuf));  // fill output buffer with spaces
	text[len] = 0x00;  // terminate text

	while ((i < len) && (wlen < VFD_DISP_SIZE))
	{
		if (text[i] == 0x00)
		{
			break;  // stop processing
		}
		else if (text[i] >= 0x20 && text[i] < 0x80)  // if normal ASCII, but not a control character
		{
			kbuf[wlen] = text[i];
			wlen++;
		}
		else if (text[i] < 0xe0)
		{
			switch (text[i])
			{
				case 0xc2:
				{
					if (text[i + 1] < 0xa0)
					{
						i++; // skip non-printing character
					}
					else
					{
						UTF_Char_Table = PT6302_UTF_C2;
					}
					break;
				}
				case 0xc3:
				{
					UTF_Char_Table = PT6302_UTF_C3;
					break;
				}
				case 0xc4:
				{
					UTF_Char_Table = PT6302_UTF_C4;
					break;
				}
				case 0xc5:
				{
					UTF_Char_Table = PT6302_UTF_C5;
					break;
				}
#if 0  // Cyrillic currently not supported
				case 0xd0:
				{
					UTF_Char_Table = PT6302_UTF_D0;
					break;
				}
				case 0xd1:
				{
					UTF_Char_Table = PT6302_UTF_D1;
					break;
				}
#endif
				default:
				{
					UTF_Char_Table = NULL;
				}
			}
			i++;  // skip UTF-8 lead in
			if (UTF_Char_Table)
			{
				kbuf[wlen] = UTF_Char_Table[text[i] & 0x3f];
				wlen++;
			}
		}
		else
		{
			if (text[i] < 0xF0)
			{
				i += 2;  // skip 2 bytes
			}
			else if (text[i] < 0xF8)
			{
				i += 3;  // skip 3 bytes
			}
			else if (text[i] < 0xFC)
			{
				i += 4;  // skip 4 bytes
			}
			else
			{
				i += 5;  // skip 5 bytes
			}
		}
		i++;
	}
	memset(text, 0x00, sizeof(kbuf) + 1);
	for (i = 0; i < wlen; i++)
	{
		text[i] = kbuf[i];
	}
	return wlen;
}

/******************************************************
 *
 * Write text on VFD display.
 *
 * Always writes VFD_DISP_SIZE characters, padded with
 * spaces if needed.
 *
 */
int pt6302_write_text(unsigned char offset, unsigned char *text, unsigned char len, int utf8_flag)
{
	int ret = -1;
	int i;
	int wlen;
	unsigned char kbuf[VFD_DISP_SIZE];

	if (len == 0)  // if length is zero,
	{
		dprintk(1, "%s < (len =  0)\n", __func__);
		return 0;  // do nothing
	}

	// remove possible trailing LF
	if (text[len - 1] == 0x0a)
	{
		len--;
	}
	text[len] = 0x00;  // terminate text

	dprintk(50, "%s: [%s] len = %d\n", __func__, text, len);
	/* handle UTF-8 chars */
	if (utf8_flag)
	{
		len = pt6302_utf8conv(text, len);
	}
	memset(kbuf, 0x20, sizeof(kbuf));  // fill output buffer with spaces
	wlen = (len > VFD_DISP_SIZE ? VFD_DISP_SIZE : len);
	memcpy(kbuf, text, wlen);  // copy text

#if 0  // set to 1 if you want the display text centered (carried over, not tested yet)
	dprintk(10, "%s Center text\n", __func__);
	/* Center text */
	j = 0;

	if (wlen < VFD_DISP_SIZE - 1)  // handle leading spaces
	{
		for (i = 0; i < ((VFD_DISP_SIZE - wlen) / 2); i++)
		{
			wdata[i + 1] = 0x20;
			j++;
		}
		wlen += j;
	}
	for (i = 0; i < wlen; i++)  // handle text
	{
		wdata[i + 1 + j] = kbuf[(wlen - 1) - i];
	}

	if (wlen < VFD_DISP_SIZE)  // handle trailing spaces
	{
		for (i = j + wlen; i < VFD_DISP_SIZE; i++)
		{
			wdata[i + 1] = 0x20;
		}
	}
	memcpy(kbuf, wdata, VFD_DISP_SIZE);
#endif
	/* save last string written to fp */
	memcpy(&lastdata.vfddata, kbuf, wlen);
	lastdata.length = wlen;
	ret = pt6302_write_dcram(0, kbuf, VFD_DISP_SIZE);
	return ret;
}

/******************************************************
 *
 * Set brightness of VFD display
 *
 */
void pt6302_set_brightness(int level)
{
	pt6302_command_t cmd;

	spin_lock(&mr_lock);

	if (level < PT6302_DUTY_MIN)
	{
		level = PT6302_DUTY_MIN;
	}
	if (level > PT6302_DUTY_MAX)
	{
		level = PT6302_DUTY_MAX;
	}
	cmd.duty.cmd  = PT6302_COMMAND_SET_DUTY;
	cmd.duty.duty = level;

	pt6302_send_byte(cmd.all);
	spin_unlock(&mr_lock);
}

/*********************************************************
 *
 * Set number of characters on VFD display.
 *
 * Note: sets num + 8 as width, but num = 0 sets width 16
 * 
 */
static void pt6302_set_digits(int num)
{
	pt6302_command_t cmd;

	spin_lock(&mr_lock);

	if (num < PT6302_DIGITS_MIN)
	{
		num = PT6302_DIGITS_MIN;
	}
	if (num > PT6302_DIGITS_MAX)
	{
		num = PT6302_DIGITS_MAX;
	}
	num = (num == PT6302_DIGITS_MAX) ? 0 : (num - PT6302_DIGITS_OFFSET);

	cmd.digits.cmd    = PT6302_COMMAND_SET_DIGITS;
	cmd.digits.digits = num;

	pt6302_send_byte(cmd.all);
	spin_unlock(&mr_lock);
}

/******************************************************
 *
 * Set VFD display on, off or all segments lit.
 *
 */
static void pt6302_set_light(int onoff)
{
	pt6302_command_t cmd;

	spin_lock(&mr_lock);

	if (onoff < PT6302_LIGHT_NORMAL || onoff > PT6302_LIGHT_ON)
	{
		onoff = PT6302_LIGHT_NORMAL;
	}
	cmd.light.cmd   = PT6302_COMMAND_SET_LIGHT;
	cmd.light.onoff = onoff;
	pt6302_send_byte(cmd.all);
	spin_unlock(&mr_lock);
}

/******************************************************
 *
 * Set binary output pins of PT6302.
 *
 * The digital outputs of the PT6302 are not connected
 * in the HS8100. This routine is merely here for
 * reasons of completeness. The pins are initialized
 * by the driver to a zero state.
 *
 */
static void pt6302_set_port(int port1, int port2)
{
	pt6302_command_t cmd;

	spin_lock(&mr_lock);

	cmd.port.cmd   = PT6302_COMMAND_SET_PORTS;
	cmd.port.port1 = (port1) ? 1 : 0;
	cmd.port.port2 = (port2) ? 1 : 0;

	pt6302_send_byte(cmd.all);
	spin_unlock(&mr_lock);
}

/****************************************
 *
 * (Re)set one icon on VFD
 *
 * The icons ICON_POWER and ICON_TIMER
 * are controlled by writing to ADRAM,
 * all others by writing to CGRAM. The
 * column data is set to 0xff if ADRAM is
 * to be used.
 * The CGRAM controlled characters 0x01
 * and 0x02 are used to create the icon
 * display on positions 12 and 13
 * respectively.
 *
 * NOTE: Contrary to most front panel
 *       drivers the value 2 is also
 *       valid for on: it inverts the
 *       current icon state.
 *
 */
int pt6302_set_icon(int icon_nr, int on)
{
	int i, j;
	int ret = 0;
	unsigned char cgram_data[ICON_WIDTH * 5];
	int pos;
	int column;
	int offset;
	unsigned char buf[4];
	int icon_power;
	int icon_timer;

	if (icon_nr < ICON_MIN + 1 || icon_nr > ICON_MAX - 1)
	{
		dprintk(1, "%s Illegal icon number %d, range is %d-%d\n", __func__, icon_nr, ICON_MIN + 1, ICON_MAX - 1);
		dprintk(150, "%s < (-1)\n", __func__);
		return -1;
	}
	if (on < 0 || on > 2)
	{
		dprintk(1, "%s Invalid value %d for state, range is 0-2\n", __func__, on);
		dprintk(150, "%s < (-1)\n", __func__);
		return -1;
	}
	// test if icon is already in requested state
	if (lastdata.icon_state[icon_nr] == on)
	{
		return 0;  // do nothing
	}
	dprintk(50, "Setting icon %d (%s) to %s\n", icon_nr, vfdIcons[icon_nr].name, (on ? "on" : "off"));

	// get icon data
	pos = vfdIcons[icon_nr].pos;
	column = vfdIcons[icon_nr].column;

	// Handle ICON_POWER & ICON_TIMER
	if (column == 0xff)  // icon is ADRAM controlled
	{
		if (on == 1)
		{
			buf[0] = lastdata.adram |= vfdIcons[icon_nr].mask;  // switch bit on
		}
		else if (on == 0)
		{
			buf[0] = lastdata.adram &= ~vfdIcons[icon_nr].mask;  // switch bit off
		}
		else
		{
			buf[0] = lastdata.adram ^= ~vfdIcons[icon_nr].mask;  // toggle bit
		}
		ret |= pt6302_write_adram((unsigned char)pos, buf, 1);
	}
	else  // other icons: control through CGRAM
	{
		// get current CGRAM data
		memcpy(cgram_data, lastdata.icon_data, sizeof(lastdata.icon_data));
	
		// calculate byte offset into CGRAM data
		offset = ((pos - VFD_DISP_SIZE) * 5) + column;

		if (on == 1)
		{
			cgram_data[offset] |= vfdIcons[icon_nr].mask;  // switch bit on
		}
		else if (on == 0)
		{
			cgram_data[offset] &= ~vfdIcons[icon_nr].mask;  // switch bit off
		}
		else
		{
			cgram_data[offset] ^= ~vfdIcons[icon_nr].mask;  // toggle bit
		}
		ret |= pt6302_write_cgram(1, cgram_data, ICON_WIDTH);  // set CGRAM (start with character 1)
		// make sure the icon display characters are in DCRAM
		buf[0] = 0x01;
		buf[1] = 0x02;
		ret |= pt6302_write_dcram(VFD_DISP_SIZE, buf, ICON_WIDTH);  // rewrite icon positions

		// save current CGRAM data
		memcpy(lastdata.icon_data, cgram_data, sizeof(lastdata.icon_data));
	}
	// save current icon state
	lastdata.icon_state[icon_nr] = on;
	return ret;
}

/**************************************************
 *
 * Time subroutines
 *
 * What follows below is a set of subroutines
 * that deal with the fact that the driver is
 * confronted with four different representations
 * of a clock time:
 * - number of seconds since linux or RTC epoch;
 * - format MJD H Mi S;
 * - format (C) Y Mo D H Mi S;
 * - a struct *tm.
 *
 * Not all possible conversions are provided, only
 * those needed by this driver are.
 *
 */

/**************************************************
 *
 * mjd2ymd: convert MJD to a date without century.
 *
 * NOTE: the century is assumed to be 20, matching
 *       the RTC epoch of 01-01-2000; the routine
 *       therefore returns a year between 0 and 38.
 *
 */
static void mjd2ymd(int MJD, unsigned char *year, unsigned char *month, unsigned char *day)
{  // converts MJD to year, month, day
	int i;
	int y;
	int m;

//	dprintk(150, "%s > MJD to convert: %d\n", MJD);

	// correct epoch difference (MJD: Nov 17th 1858, RTC: newyear 2000, both midnight)
	MJD -= 51544;

	y = 2000;
	while (MJD >= YEARSIZE(y))
	{
		MJD -= YEARSIZE(y);
		y++;
	}		
	m = 0;
	while (MJD >= _ytab[LEAPYEAR(y)][m])
	{
		MJD -= _ytab[LEAPYEAR(y)][m];
		m++;
	}
	m++;  // months count from 1
	MJD++;  // days count from 1

	*year  = (unsigned char)y - 2000;
	*month = (unsigned char)m;
	*day   = (unsigned char)MJD;
}

/**************************************************
 *
 * gmtime: convert seconds since linux epoch to
 *         a struct tm.
 *
 */
struct tm *gmtime(register const time_t time)
{ // seconds since linux epoch -> struct tm
	static struct tm fptime;
	register unsigned long dayclock, dayno;
	int year = 70;

	dayclock = (unsigned long)time % DAY_SECS;
	dayno = (unsigned long)time / DAY_SECS;

	fptime.tm_sec = dayclock % MINUTE_SECS;
	fptime.tm_min = (dayclock % HOUR_SECS) / MINUTE_SECS;
	fptime.tm_hour = dayclock / HOUR_SECS;
	fptime.tm_wday = (dayno + 4) % 7;  // day 0 was a thursday

	while (dayno >= YEARSIZE(year))
	{
		dayno -= YEARSIZE(year);
		year++;
	}
	fptime.tm_year = year;
	fptime.tm_yday = dayno;
	fptime.tm_mon = 0;
	while (dayno >= _ytab[LEAPYEAR(year)][fptime.tm_mon])
	{
		dayno -= _ytab[LEAPYEAR(year)][fptime.tm_mon];
		fptime.tm_mon++;
	}
	fptime.tm_mday = dayno;
//	fptime.tm_isdst = -1;

	return &fptime;
}

/**************************************************
 *
 * getMJD: read a struct tm and return MJD.
 *
 */
int getMJD(struct tm *theTime)
{  // struct tm (date) -> mjd
	int mjd = 0;
	int year;

	year = theTime->tm_year - 1;  // do not count current year
	while (year >= 70)
	{
		mjd += 365;
		if (LEAPYEAR(year))
		{
			mjd++;
		}
		year--;
	}
	mjd += theTime->tm_yday;
	mjd += 40587;  // mjd starts on midnight 17-11-1858 which is 40587 days before unix epoch
	return mjd;
}

/**************************************************
 *
 * calcSetRTCTime: convert MJD HMS to seconds since
 *                 linux epoch.
 *
 */
time_t calcSetRTCTime(char *time)
{  // mjd hh:mm:ss -> seconds since linux epoch
	unsigned int    mjd;
	unsigned long   epoch;
	unsigned int    hour;
	unsigned int    min;
	unsigned int    sec;

	mjd     = ((time[0] & 0xff) << 8) + (time[1] & 0xff);
	epoch   = ((mjd - 40587) * DAY_SECS);
	hour    = time[2] & 0xff;
	min     = time[3] & 0xff;
	sec     = time[4] & 0xff;

	epoch += (hour * HOUR_SECS + min * MINUTE_SECS + sec);
	return epoch;
}

/**************************************************
 *
 * calcGetRTCTime: convert seconds since linux
 *                 epoch to MJD HMS.
 *
 */
void calcGetRTCTime(time_t theTime, char *destString)
{  // seconds since epoch -> mjd h:m:s
	struct tm *now_tm;
	int mjd;

	now_tm = gmtime(theTime);
	mjd = getMJD(now_tm);
	destString[0] = (mjd >> 8) & 0xff;
	destString[1] = (mjd & 0xff);
	destString[2] = now_tm->tm_hour;
	destString[3] = now_tm->tm_min;
	destString[4] = now_tm->tm_sec;
}

/**************************************************
 *
 * datetime2time_t: convert a time/date to seconds
 *                  since linux epoch (01-01-1970).
 *
 */
time_t datetime2time_t(unsigned char year, unsigned char month, unsigned char day, unsigned char hours, unsigned char minutes, unsigned char seconds)
{
	int ret = 0;
	int temp_year = 0;
	int temp_month = 0;
	int temp_yday = 0;
	char time[5];
	time_t theTime;

	// calculate the number of the day in the current year
	temp_month = (int)month - 1;  // do not count current month
	temp_year = (int)year - 1;  // do not count current year

	while (temp_year > 0)  // add whole previous years since 2001
	{
		temp_yday += YEARSIZE(temp_year);
		temp_year--;
	}
	temp_yday += YEARSIZE(2000); // add the year 2000

	while (temp_month > 0)  // add whole months in this year
	{
		temp_yday += _ytab[LEAPYEAR(year)][temp_month - 1];  // array index starts at zero
		temp_month--;
	}
	temp_yday += (int)day - 1;  // add this month, but not the current day
	temp_yday += 51544; // epoch of MJD is 17-11-1858, not 01-01-2000

	time[0] = (temp_yday) >> 8;
	time[1] = (temp_yday) & 0xff;
	time[2] = hours;
	time[3] = minutes;
	time[4] = seconds;
	theTime = calcSetRTCTime(time);
	return theTime;
}
// end of time conversion routines

// IOCTL time routines
/**************************************************
 *
 * hchs8100_SetTime: sets RTC clock time.
 *
 */
int hchs8100_SetTime(char *time)
{
	int uTime;
	int ret = 0;

	uTime = calcSetRTCTime(time);
	ret = write_rtc_time(uTime);
	return ret;
}

/****************************************
 *
 * hchs8100_GetTime: read RTC clock time.
 *
 */
int hchs8100_GetTime(char *time)
{
	int uTime; // seconds since epoch
	int ret = 0;
	int i;

	uTime = read_rtc_time();  // read RTC

	// convert time_t to MJD HMS
	calcGetRTCTime(uTime, time);
	dprintk(20, "RTC Time = %d (MJD) %02d:%02d:%02d (UTC)\n", ((time[0] & 0xff) << 8) + (time[1] & 0xff),
		time[2], time[3], time[4]);
	return ret;
}

/******************************************************
 *
 * hchs8100_SetWakeUpTime: sets RTC clock wake up time.
 *
 * CAUTION: The DS1307 RTC does not have an alarm
 * function; the time handled here is merely stored in
 * its battery backed up RAM.
 *
 */
int hchs8100_SetWakeUpTime(char *wtime)
{
	int ret = 0, i;
	int mjd;
	unsigned char year;
	unsigned char month;
	unsigned char day;
	unsigned char rtc_year;
	unsigned char rtc_month;
	unsigned char rtc_day;
	unsigned char rtc_hour;
	unsigned char rtc_min;

	dprintk(50, "%s Wake up time to set: MJD = %d, %02d:%02d:%02d\n", __func__, (((wtime[0] & 0xff) << 8) + (wtime[1] & 0xff)) & 0xffff, wtime[2], wtime[3], wtime[4]);
	// Check if wake up time is legal (that is on or after midnight 01-01-2000)
	if ((((wtime[0] & 0xff) << 8) + (wtime[1] & 0xff) & 0xffff) < 51544)  // 01-01-2000
	{
		wtime[0] = 51544 >> 8;  // if illegal wake up time
		wtime[1] = 51544 & 0xff;  // set 01-01-2000, midnight
		wtime[2] = 0;
		wtime[3] = 0;
		wtime[4] = 0;
		dprintk(1, "CAUTION: illegal wake up time, defaulting to 01-01-2000, 00:00:00\n");
	}
	// store wtime for retrieval through VFDGETPOWERONTIME
	// without conversions
	for (i = 0; i < 3; i++)
	{
		wakeup_time[i] = wtime[i];
	}
	wakeup_time[4] = 0;  //  seconds are always zero in this case

	// Convert the MJD to a date
	mjd = ((wtime[0] & 0xff) << 8) + (wtime[1] & 0xff); 
	mjd2ymd(mjd, &year, &month, &day);
	dprintk(50, "%s: Wake up time to store: %02d-%02d-20%02d %02d:%02d:%02d\n", __func__, day, month, year, wtime[2], wtime[3], wtime[4]); 
	// store in DS1307 RAM
	ret = ds1307_setalarm(year, month, day, wtime[2], wtime[3], wtime[4]);
	return ret;
}

/******************************************************
 *
 * hchs8100_GetWakeUpTime: read stored wakeup time.
 *
 */
void hchs8100_GetWakeUpTime(char *time)
{
	int i;

	for (i = 0; i < 3; i++)
	{
		time[i] = wakeup_time[i];
	}
	time[4] = 0;
}

/******************************************************
 *
 * hchs8100_GetWakeUpMode: get wake up reason.
 *
 * The actual wake up reason is determined at the
 * start of the driver and stored.
 *
 * This routine merely returns the stored value.
 *
 */
int hchs8100_GetWakeUpMode(int *wakeup_reason)
{

	*wakeup_reason = wakeup_mode;
	return 0;
}

/******************************************************
 *
 * hchs8100_SetStandby: sets wake up time and then 
 *                      switches receiver to simulated
 *                      standby.
 *
 * Description of simulated standby:
 *
 * Although the HS8100 series can switch some of its
 * supply power voltages off through PIO 3.2, this
 * shuts down too many parts of the receiver. The
 * model therefore does not have a true deep standby
 * mode for power saving purposes.
 * A mechanism has been devised to simulate deep
 * standby behaviour, although this barely saves any
 * energy.
 *
 * Upon entering this routine the following is done:
 * - The current system time is stored in the DS1307
 *   RTC, synchronizing it.
 * - If a power up time at least five minutes in the
 *   future is specified, it is stored as such in the
 *   DS1307 RTC.
 * - A wait loop is started that shows the time and
 *   date without seconds and year on the front panel
 *   about twice each second (in order to show a
 *   blinking colon).
 * - The standby icon is switched on.
 * - If a valid wake up time at least five minutes in
 *   the future is stored in the RTC, the timer icon
 *   will be on.
 * - In the background a thread is started that
 *   performs these actions once every 100
 *   milliseconds:
 *   - Check if either the front panel power key or
 *     the remote control power key have been pressed.
 *     If so, all threads and the wait loop will be
 *     terminated and control is returned to the
 *     system.
 *     TODO: As long as the thread runs, all other input
 *     through the front panel key or remote control
 *     keys is inhibited.
 * - If a valid wake up time at least five minutes in
 *   the future is stored in the RTC, a second thread
 *   is started running every 29.5 seconds. This thread
 *   will evaluate if the current RTC time has reached
 *   the point in time WAKEUP_TIME seconds before the
 *   stored wakeup time. When this happens, the 100 ms
 *   key scan thread will be stopped and the receiver
 *   will reboot.
 *   This thread will run until the wake up time minus
 *   WAKEUP_TIME seconds is reached or when a power key
 *   is pressed.
 *
 * Note: if the intention is to merely start simulated
 *       deep standby, specify a wake up time in the
 *       far future or in the past. fp_control uses
 *       the first possiblility.
 *
 * TODO: Fix a mysterious bug as this code shows the date
 *       of yesterday on the front panel display before
 *       noon during simulated deep standby. It cannot be
 *       the RTC, linux or MJD epochs, as these all start
 *       at midnight, only JD has an epoch starting at
 *       noon.
 *
 */
int hchs8100_SetStandby(char *wtime)
{
	int ret = 0;
	int i = 0, j;
	int wakeup_int;
	unsigned char year;
	unsigned char month;
	unsigned char day;
	unsigned char wakeup_year;
	unsigned char wakeup_month;
	unsigned char wakeup_day;
	unsigned char wakeup_hours;
	unsigned char wakeup_minutes;
	unsigned char wakeup_seconds;
	unsigned char temp_hours;
	unsigned char temp_minutes;
	int wakeup_time_valid = 0;
	char disp_string[VFD_DISP_SIZE];
	char temp_string[5];  // temporary hold of current time
	struct timeval tv;
	char *month_name[13] = { "???", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	time_t rtc_time_t;

	dprintk(100, "%s >\n", __func__);
	ret = hchs8100_SetWakeUpTime(wtime);  // set power on time

	dprintk(50, "%s Synchronise RTC time.\n", __func__);
	do_gettimeofday(&tv);  // system time
	ret |= write_rtc_time(tv.tv_sec);  // synchronise RTC

	// check for no wake up time (19 January 2038 03:14:07 UTC -> MJD = 65442, or before RTC epoch)
	// note that mjd reaches a bit further than the linux 32 bit time limit
	if (((((wtime[0] & 0xff) << 8) + (wtime[1] & 0xff) & 0xffff) > 65441) || (((wtime[0] & 0xff) << 8) + (wtime[1] & 0xff)) & 0xffff < 51544)
	{
		dprintk(50, "%s No usable wake up time set.\n", __func__);
		goto standby;
	}

	// determine if a valid wake time/date is set
	ret = ds1307_getalarmtime(&wakeup_year, &wakeup_month, &wakeup_day, &wakeup_hours, &wakeup_minutes, &wakeup_seconds);
	// test validity
	if ((wakeup_year < 0 || wakeup_year > 99)
	||  (wakeup_month < 1 || wakeup_month > 12)
	||  (wakeup_day < 0 || wakeup_day > 31)
	||  (wakeup_hours < 0 || wakeup_hours > 23)
	||  (wakeup_minutes < 0 || wakeup_minutes > 59)
	||  (wakeup_seconds < 0 || wakeup_seconds > 59))
	{
		// set a valid wake up time in the past
		ret |= ds1307_setalarm(22, 1, 1, 0, 0, 0);  // midnight on 01-01-2022
		goto standby;
	}
	// test if wake up time is at least WAKEUP_TIME seconds in the future
	wakeup_int = calcSetRTCTime(wtime);  // get wake up time as time_t
#if 0
	calcGetRTCTime(wakeup_int - WAKEUP_TIME, temp_string);
	dprintk(50, "%s temp_string: 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\n", __func__, (int)temp_string[0] & 0xff, (int)temp_string[1] & 0xff, (int)temp_string[2] & 0xff, (int)temp_string[3] & 0xff, (int)temp_string[4] & 0xff);
	i = ((temp_string[0] << 8) + temp_string[1]) & 0xffff;
	dprintk(50, "%s Wakeup time MJD is %d\n", __func__, i);
	mjd2ymd(i, &w_year, &w_month, &w_day);	
	dprintk(50, "%s Desired wakeup time is %02d-%02d-20%02d %02d:%02d\n", __func__, (int)w_day, (int)w_month, (int)w_year, (int)disp_string[2], (int)disp_string[3]);
	dprintk(50, "%s Current RTC time is    %02d-%02d-20%02d %02d:%02d\n", __func__, (int)rtc_state->rtc_day, (int)rtc_state->rtc_month, (int)rtc_state->rtc_year, (int)rtc_state->rtc_hours, (int)rtc_state->rtc_minutes);
#endif	
	if ((wakeup_int - tv.tv_sec) > WAKEUP_TIME)
	{
		// wake up time is at least WAKEUP_TIME seconds in the future, add timer thread
//		dprintk(50, "%s Wakeup time is at least %d seconds in the future (%d seconds ahead).\n", __func__, wakeup_int - tv.tv_sec);
		wakeup_time_valid = 1;
	}

standby:
	for (i = 1; i < ICON_MAX; i++)
	{
		ret |= 	pt6302_set_icon(i, 0);  // all icons off
	}
	ret |= pt6302_write_dcram(0x00, "            ", VFD_DISP_SIZE);  // clear display (1st 12 characters)
	ret |= pt6302_set_icon(ICON_POWER, 1);
	ret |= ds1307_set_deepstandby(WAKEUP_DEEPSTDBY);  // set deep standby flag
	rc_power_key = 0;

	// start standby thread
	standby_state.period = 100;  // test every 100ms for power keys
	standby_state.state = 1;
	up(&standby_state.sem);

	if (wakeup_time_valid)
	{
		dprintk(50, "%s Add timer thread\n", __func__);
		ret |= pt6302_set_icon(ICON_TIMER, 1);
		// start timer thread
		timer_state.period = 29500;  // test every 29.5s for wakeup time
		timer_state.state = 1;
		standby_state.state = 2;  // flag timer thread also running
		up(&timer_state.sem);
	}
	// dirty: fake /dev/fd closes
//	fp.opencount = 0;
	j = 0;
	while (standby_state.status != THREAD_STATUS_STOPPED && standby_state.status != THREAD_STATUS_HALTED)
	{  // TODO: date is 1 day off before noon(?)
		// update display
		rtc_time_t = read_rtc_time();  // this time is in UTC
		rtc_time_t += rtc_offset;  // convert to local time
		calcGetRTCTime(rtc_time_t, temp_string);
		i = (((temp_string[0] & 0xff) << 8) + (temp_string[1] & 0xff)) & 0xffff;
		mjd2ymd(i, &year, &month, &day);
		i = sprintf(disp_string, "%02d %s %02d%s%02d", day, month_name[month],
			temp_string[2], (j ? ":" : " "), temp_string[3]);
		ret |= pt6302_write_dcram(0x00, (unsigned char *)disp_string, i);
		j++;
		if (j == 2)
		{
			j = 0;
		}
		msleep(500);
	}
	// power key pressed or timer triggered
	ret |= pt6302_set_icon(ICON_POWER, 0);
	ret |= pt6302_set_icon(ICON_TIMER, 0);
	ret |= pt6302_write_text(0, "[ Wake up ]", sizeof("[ Wake up ]"), 0);
	msleep(1000);
	if (standby_state.state == 1)
	{
		standby_state.state = 0;
		dprintk(50, "%s Halt standby thread\n", __func__);
		i = 0;
		do
		{
			msleep(standby_state.period / 20);
			i++;
		}
		while (standby_state.status != THREAD_STATUS_HALTED && i < 22);

		if (i == 22)
		{
			dprintk(1, "%s Time out halting standby thread!\n", __func__);
		}
		dprintk(50, "%s Standby thread halted\n", __func__);
	}
	rc_power_key = 0;

	// restart
	kernel_restart(NULL);
	return ret;  // never reached; keep compiler happy
}

/******************************************************
 *
 * Setup VFD display
 * 12 text characters, normal display, maximum
 * brightness.
 *
 */
int pt6302_setup(void)
{
	unsigned char buf[40];  // init for ADRAM & CGRAM
	int ret = 0;
	int i, j, k;

	dprintk(50, "Set display width to %d (text) + %d (icons)\n", VFD_DISP_SIZE, ICON_WIDTH);
	pt6302_set_digits(VFD_DISP_SIZE + ICON_WIDTH);
	dprintk(50, "Set display brightness to %d\n", PT6302_DUTY_MAX);
	pt6302_set_brightness(PT6302_DUTY_MAX);  // maximum brightness
	dprintk(50, "Switch PT6302 digital outputs off\n");
	pt6302_set_port(0, 0);  // digital output pins off
	dprintk(50, "Switch display on\n");
	pt6302_set_light(PT6302_LIGHT_NORMAL);  // normal operation
#if 1
	ret |= pt6302_write_dcram(0x00, "            ", VFD_DISP_SIZE);  // clear display (1st 12 characters)
#else
//	dprintk(50, "Show signon text\n");
	ret |= pt6302_write_text(0, "HS8100 VFD", 10, 0);
#endif
	// Initialize icon positions
	buf[0] = 0x01;
	buf[1] = 0x02;
	ret |= pt6302_write_dcram(VFD_DISP_SIZE, buf, ICON_WIDTH);  // initialize icon positions
	// Clear cursor data (including ICON_POWER and ICON_TIMER)
	dprintk(50, "Clearing ADRAM\n");
	memset(buf, 0, sizeof(buf));  // clear all bits
	pt6302_write_adram(0x00, buf, VFD_DISP_SIZE + ICON_WIDTH);  // clear ADRAM (underline data)
	dprintk(50, "Clearing CGRAM\n");
	memset(buf, 0x00, sizeof(buf));  // clear all bits (includes remaining icons)
	ret |= pt6302_write_cgram(0, buf, 8);  // clear CGRAM (start with character 0, 8 patterns)
	ret |= pt6302_write_dcram(0x00, "            ", VFD_DISP_SIZE);  // clear display (1st 12 characters)
	return ret;
}

/***************************************************
 *
 * Initialize the driver
 *
 */
int hchs8100_fp_init_func(void)
{
	int i = 0;
	int ret = 0;
	time_t theTime;
	time_t wakeupTime;

	dprintk(150, "%s >\n", __func__);
	dprintk(10, "Homecast HS8100 series front panel driver initializing\n");

	if (fp_init_pio_pins() == 0)
	{
		dprintk(1, "Unable to initialize PIO pins; abort\n");
		goto fp_init_fail;
	}
	ret |= pt6302_setup();
	sema_init(&(fp.sem), 1);
	fp.opencount = 0;

	// clear icons
	for (i = ICON_MIN + 1; i < ICON_MAX; i++)
	{
		ret |= pt6302_set_icon(i, 0);
	}

	// determine wake up reason
	i  = ds1307_readreg(DS1307_REG_STANDBY_H) << 8;
	i |= ds1307_readreg(DS1307_REG_STANDBY_L) & 0xff;
	if (rtc_is_invalid()  // note: fills rtc_state
	||  i == WAKEUP_INVALID)
	{
		wakeup_mode = WAKEUP_UNKNOWN;  // report we do not know
		// set default wake up time of 01-01-2000 midnight (RTC epoch)
		wakeup_time[0] = 51544 >> 8;
		wakeup_time[1] = 51544 & 0xff;
		wakeup_time[2] = 0;
		wakeup_time[3] = 0;
		wakeup_time[4] = 0;
	}
	else if (i == WAKEUP_DEEPSTDBY)
	{
		// from deep standby, evaluate wake up time, if any
		wakeup_mode = WAKEUP_DEEPSTANDBY;
		ret |= ds1307_getalarmstate();  // get alarm time in rtc_state
		wakeupTime = datetime2time_t(rtc_state->rtc_wakeup_year, rtc_state->rtc_wakeup_month, rtc_state->rtc_wakeup_day,
			rtc_state->rtc_wakeup_hours, rtc_state->rtc_wakeup_minutes, rtc_state->rtc_wakeup_seconds);
		calcGetRTCTime(wakeupTime, wakeup_time);

		ds1307_readregs();  // get RTC time in state
		theTime = datetime2time_t(rtc_state->rtc_year, rtc_state->rtc_month, rtc_state->rtc_day,
			rtc_state->rtc_hours, rtc_state->rtc_minutes, rtc_state->rtc_seconds);
#if 0
		dprintk(50, "Timer is %d seconds %s RTC time\n", abs(theTime - wakeupTime), (theTime - wakeupTime > 0 ? "behind" : "ahead of"));
#endif
		// if wake up time is less than 5 minutes in the future but not more, assume timer wake up
		if ((wakeupTime - theTime) > 0)  // if wake up time in the future
		{
			if ((wakeupTime - theTime) < WAKEUP_TIME)  // but less than WAKEUP_TIME
			{
//				dprintk(50, "Wake up time less than %d seconds in the future -> Wake up from timer.\n", WAKEUP_TIME);
				wakeup_mode = WAKEUP_TIMER;
			}
		}
		// clear deep stand by flag
		ret |= ds1307_set_deepstandby(WAKEUP_NORMAL);
	}
	else
	{
		wakeup_mode = WAKEUP_POWERON;  // must be normal power on
		ret |= ds1307_set_deepstandby(WAKEUP_NORMAL);
	}
	dprintk(50, "Wake up reason: %s (%d)\n", wakeupmode_name[wakeup_mode], wakeup_mode);
	return ret;

fp_init_fail:
	pt6302_free_pio_pins();
	dprintk(1, "Homecast HS8100 series front panel driver initialization failed\n");
	return -1;
}

/******************************************************
 *
 * Text display thread code.
 *
 * All text display via /dev/vfd is done in this
 * thread that also handles scrolling.
 * 
 * Code largely taken from aotom_spark (thnx Martii)
 *
 */
void clear_display(void)
{
	char bBuf[VFD_DISP_SIZE];
	int ret = 0;

	memset(bBuf, ' ', sizeof(bBuf));
	ret |= pt6302_write_text(0, bBuf, VFD_DISP_SIZE, 0);
}

static int vfd_text_thread(void *arg)
{
	struct vfd_ioctl_data *data = (struct vfd_ioctl_data *)arg;
	unsigned char buf[sizeof(data->data) + 2 * VFD_DISP_SIZE];
	int disp_size;
	int len = data->length;
	int off = 0;
	int ret = 0;
	int i;

	data->data[len] = 0;  // terminate string
	len = pt6302_utf8conv(data->data, data->length);
	memset(buf, 0x20, sizeof(buf));

	if (len > VFD_DISP_SIZE)
	{
		off = 3;
		memcpy(buf + off, data->data, len);
		len += off;
		buf[len + VFD_DISP_SIZE] = 0;
	}
	else
	{
		memcpy(buf, data->data, len);
		buf[len + VFD_DISP_SIZE] = 0;
	}
	vfd_text_state.status = THREAD_STATUS_RUNNING;

	if (len > VFD_DISP_SIZE + 1)
	{
		unsigned char *b = buf;
		int pos;

		for (pos = 0; pos < len; pos++)
		{
			int i;

			if (kthread_should_stop())
			{
				vfd_text_state.status = THREAD_STATUS_STOPPED;
				return 0;
			}

			ret |= pt6302_write_text(0, b, VFD_DISP_SIZE, 0);

			// check for stop request
			for (i = 0; i < 6; i++)  // note: this loop also constitutes the scroll delay
			{
				if (kthread_should_stop())
				{
					vfd_text_state.status = THREAD_STATUS_STOPPED;
					return 0;
				}
				msleep(25);
			}
			// advance to next character
			b++;
		}
	}
	if (len > 0)  // final display, also no scroll
	{
		ret |= pt6302_write_text(0, buf + off, VFD_DISP_SIZE, 0);
	}
	else
	{
		clear_display();
	}
	vfd_text_state.status = THREAD_STATUS_STOPPED;
	return 0;
}

static int run_text_thread(struct vfd_ioctl_data *draw_data)
{
	if (down_interruptible(&vfd_text_state.sem))
	{
		return -ERESTARTSYS;
	}

	// return if there is already a draw task running for the same VFD text
	if (vfd_text_state.status != THREAD_STATUS_STOPPED
	&&  vfd_text_state.task
	&&  last_vfd_draw_data.length == draw_data->length
	&&  !memcmp(&last_vfd_draw_data.data, draw_data->data, draw_data->length))
	{
		up(&vfd_text_state.sem);
		return 0;
	}
	memcpy(&last_vfd_draw_data, draw_data, sizeof(struct vfd_ioctl_data));

	// stop existing thread, if any
	if (vfd_text_state.status != THREAD_STATUS_STOPPED
	&&  vfd_text_state.task)
	{
		kthread_stop(vfd_text_state.task);
		while (vfd_text_state.status != THREAD_STATUS_STOPPED)
		{
			msleep(1);
		}
	}
	vfd_text_state.status = THREAD_STATUS_INIT;
	vfd_text_state.task = kthread_run(vfd_text_thread, draw_data, "vfd_text_thread");

	// wait until thread has copied the argument
	while (vfd_text_state.status == THREAD_STATUS_INIT)
	{
		msleep(1);
	}
	up(&vfd_text_state.sem);
	return 0;
}

int hchs8100_WriteText(char *buf, size_t len)
{
	int res = 0;
	struct vfd_ioctl_data data;

	if (len > sizeof(data.data))  // do not display more than 64 characters
	{
		data.length = sizeof(data.data);
	}
	else
	{
		data.length = len;
	}

	while ((data.length > 0) && (buf[data.length - 1 ] == '\n'))
	{
		data.length--;
	}
	buf[data.length ] = 0;  // terminate string

	if (data.length > sizeof(data.data))
	{
		len = data.length = sizeof(data.data);
	}
	dprintk(150, "%s [%s] (len %d)\n", __func__, buf, len);
	memcpy(data.data, buf, data.length);
	res = run_text_thread(&data);
	return res;
}

/***************************************************
 *
 * Code for writing to /dev/vfd
 *
 * If text to display is longer than the display
 * width, it is scolled once. Maximum text length
 * is 64 characters.
 *
 */
static ssize_t vfd_write(struct file *filp, const char *buff, size_t len, loff_t *off)
{
	char *kernel_buf;
	int res = 0;

	struct vfd_ioctl_data data;

//	dprintk(150, "%s > (len %d, offs %d)\n", __func__, len, (int) *off);

	kernel_buf = kmalloc(len + 1, GFP_KERNEL);

	memset(kernel_buf, 0, len + 1);
	memset(&data, 0, sizeof(struct vfd_ioctl_data));

	if (kernel_buf == NULL)
	{
		dprintk(1, "%s returns no memory <\n", __func__);
		return -ENOMEM;
	}
	copy_from_user(kernel_buf, buff, len);

	hchs8100_WriteText(kernel_buf, len);

	kfree(kernel_buf);

//	dprintk(150, "%s < res %d len %d\n", __func__, res, len);

	if (res < 0)
	{
		return res;
	}
	return len;
}

/*******************************************
 *
 * Code for reading from /dev/vfd
 *
 */
static ssize_t vfd_read(struct file *filp, char *buf, size_t len, loff_t *off)
{
	if (len > lastdata.length)
	{
		len = lastdata.length;
	}

	if (len > VFD_DISP_SIZE)
	{
		len = VFD_DISP_SIZE;
	}
	copy_to_user(buf, lastdata.vfddata, len);
	return len;
}

static int vfd_open(struct inode *inode, struct file *file)
{
	if (down_interruptible(&(fp.sem)))
	{
		dprintk(1, "Interrupted while waiting for semaphore\n");
		return -ERESTARTSYS;
	}
	if (fp.opencount > 0)
	{
		dprintk(1, "%s: Device already opened\n", __func__);
		up(&(fp.sem));
		return -EUSERS;
	}
	fp.opencount++;
	up(&(fp.sem));
	return 0;
}

static int vfd_close(struct inode *inode, struct file *file)
{
	fp.opencount = 0;
	return 0;
}

/****************************************
 *
 * IOCTL handling.
 *
 */
static int vfd_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	static int mode = 0;
	struct hchs8100_fp_ioctl_data *hchs8100_fp = (struct hchs8100_fp_ioctl_data *)arg;  // used in mode other than 0
	struct vfd_ioctl_data vfddata;  // = (struct vfd_ioctl_data *)arg;  // used in mode 0
	int i;
	int level;
	int ret = 0;
	int icon_nr;
	int on;
	unsigned char clear[1] = { 0x00 };

//	dprintk(150, "%s >\n", __func__);

	switch (cmd)
	{
		case VFDSETMODE:
		case VFDDISPLAYCHARS:
		case VFDBRIGHTNESS:
		case VFDDISPLAYWRITEONOFF:
		case VFDICONDISPLAYONOFF:
		case VFDSETFAN:
		case VFDSETREMOTEKEY:
		{
			copy_from_user(&vfddata, (void *)arg, sizeof(struct vfd_ioctl_data));
		}
	}

	switch (cmd)
	{
		case VFDSETMODE:
		{
			mode = hchs8100_fp->u.mode.compat;
			break;
		}
		case VFDBRIGHTNESS:
		{
			level = (mode == 0 ? vfddata.address : hchs8100_fp->u.brightness.level);
			if (level < 0)
			{
				level = 0;
			}
			else if (level > 7)
			{
				level = 7;
			}
			pt6302_set_brightness(level);
			mode = 0;  // always return to mode 0
			break;
		}
		case VFDDRIVERINIT:
		{
			ret = hchs8100_fp_init_func();
			mode = 0;  // always return to mode 0
			break;
		}
		case VFDICONDISPLAYONOFF:
		{
			icon_nr = (mode == 0 ? vfddata.data[0] : hchs8100_fp->u.icon.icon_nr);
			on = (mode == 0 ? vfddata.data[4] : hchs8100_fp->u.icon.on);

			if (icon_nr < ICON_MIN + 1 || icon_nr > ICON_SPINNER)
			{
				dprintk(1, "Error: illegal icon number %d (mode %d)\n", icon_nr, mode);
				goto icon_exit;
			}

			// Part one: translate E2 icon numbers to own icon numbers (vfd mode only)
			if (mode == 0)  // vfd mode
			{
				switch (icon_nr)
				{
					case 0x17:  // dolby
					{
						icon_nr = ICON_DOLBY;
						break;
					}
					case 0x1e:  // record
					{
						icon_nr = ICON_REC;
						break;
					}
					default:
					{
						break;
					}
				}
			}
			// Part two: decide whether one icon, all or spinner
			if (on > 2 || on < 0)
			{
				dprintk(1, "%s: Illegal state %d for icon %d (defaulting to off)\n", __func__, on, icon_nr);
				on = 0;
			}
			switch (icon_nr)
			{
				case ICON_SPINNER:
				{
					ret = 0;
					if (on)
					{
						lastdata.icon_state[ICON_SPINNER] = 1;
						spinner_state.state = 1;
						if (on == 1)  // handle default speed
						{
							on = 25;  // full cycle takes 16 * 250 ms = 4 sec
						}
						spinner_state.period = on * 10;
						up(&spinner_state.sem);
					}
					else
					{
						if (spinner_state.state == 1)
						{
							spinner_state.state = 0;
//							dprintk(50, "%s Stop spinner\n", __func__);
							i = 0;
							do
							{
								msleep(250);
								i++;
							}
							while (spinner_state.status != THREAD_STATUS_HALTED && i < 20);
							if (i == 20)
							{
								dprintk(1, "%s Time out stopping spinner thread!\n", __func__);
							}
							dprintk(50, "%s Spinner stopped.\n", __func__);
						}
						lastdata.icon_state[ICON_SPINNER] = 0;  // flag spinner off
					}
					break;
				}  // ICON_SPINNER
				case ICON_MAX:
				{
					if (on > 2 || on < 0)
					{
						dprintk(1, "%s: Illegal state for icon %d\n", __func__, ICON_MAX);
					}
					lastdata.icon_state[ICON_MAX] = on;
					for (i = ICON_MIN + 1; i < ICON_MAX; i++)
					{
						ret |= pt6302_set_icon(i, on);
					}
					break;
				}
				default:  // (re)set a single icon
				{
					if (on > 2 || on < 0)
					{
						dprintk(1, "%s: Illegal state for icon %d\n", __func__, icon_nr);
					}
					ret |= pt6302_set_icon(icon_nr, on);
					break;
				}
			}

icon_exit:
			mode = 0;  // always return to mode 0
			break;
		}
		case VFDDISPLAYCHARS:
		{
			if (mode == 0)
			{
				unsigned char out[VFD_DISP_SIZE + 1];
				int wlen;

				memset(out, 0x20, sizeof(out));  // fill output buffer with spaces
				wlen = (vfddata.length > VFD_DISP_SIZE ? VFD_DISP_SIZE : vfddata.length);
				memcpy(out, vfddata.data, wlen);
				ret |= pt6302_write_dcram(0, out, VFD_DISP_SIZE);
			}
			mode = 0;  // always return to mode 0
			break;
		}
		case VFDDISPLAYWRITEONOFF:
		{
			pt6302_set_light(hchs8100_fp->u.light.onoff == 0 ? PT6302_LIGHT_OFF : PT6302_LIGHT_NORMAL);
			mode = 0;  // always return to mode 0
			break;
		}
		case VFDSETFAN:
		{
			if (hchs8100_fp->u.fan.speed > 255)
			{
				hchs8100_fp->u.fan.speed = 255;
			}
			if (hchs8100_fp->u.fan.speed < 0)
			{
				hchs8100_fp->u.fan.speed = 0;
			}
			dprintk(10, "Set fan speed to: %d\n", hchs8100_fp->u.fan.speed);
			
			if (hchs8100_fp->u.fan.speed == 0)
			{
					ret = hchs8100_set_fan(0, 0);  // switch fan off
			}
			else if (hchs8100_fp->u.fan.speed < 128)
			{
				ret = hchs8100_set_fan(1, 0);  // set low fan speed
			}
			else
			{
				ret = hchs8100_set_fan(1, 1);  // set high fan speed
			}
			mode = 0;  // always return to mode 0
			break;
		}
		case VFDSTANDBY:
		{
//			clear_display();
			dprintk(10, "Set simulated standby mode, wake up time: (MJD= %d) - %02d:%02d:%02d (UTC)\n", (hchs8100_fp->u.standby.time[0] & 0xff) * 256 + (hchs8100_fp->u.standby.time[1] & 0xff),
				hchs8100_fp->u.standby.time[2], hchs8100_fp->u.standby.time[3], hchs8100_fp->u.standby.time[4]);
			ret = hchs8100_SetStandby(hchs8100_fp->u.standby.time);
			mode = 0;  // always return to mode 0
			break;
		}
		case VFDSETPOWERONTIME:
		{
			dprintk(10, "Set wake up time: (MJD= %d) - %02d:%02d:%02d (UTC)\n", ((hchs8100_fp->u.standby.time[0] << 8) + (hchs8100_fp->u.standby.time[1])) & 0xffff,
				hchs8100_fp->u.standby.time[2], hchs8100_fp->u.standby.time[3], hchs8100_fp->u.standby.time[4]);
			ret = hchs8100_SetWakeUpTime(hchs8100_fp->u.standby.time);
			mode = 0;  // always return to mode 0
			break;
		}
		case VFDGETWAKEUPTIME:
		{
			dprintk(10, "Wake up time: (MJD= %d) - %02d:%02d:%02d (UTC)\n", ((wakeup_time[0] & 0xff) << 8) + (wakeup_time[1] & 0xff),
				wakeup_time[2], wakeup_time[3], wakeup_time[4]);
			copy_to_user((void *)arg, &wakeup_time, 5);
			ret = 0;
			mode = 0;  // always return to mode 0
			break;
		}
		case VFDGETWAKEUPMODE:
		{
			int wakeup_mode = WAKEUP_UNKNOWN;

			ret = hchs8100_GetWakeUpMode(&wakeup_mode);
			dprintk(10, "Wake up mode = %d (%s)\n", wakeup_mode, wakeupmode_name[wakeup_mode]);
			ret |= copy_to_user((void *)arg, &wakeup_mode, 1);
			mode = 0;
			break;
		}
		case VFDGETTIME:
		{
			char time[5];

			ret = hchs8100_GetTime(time);
			dprintk(10, "Get RTC time: (MJD=) %d - %02d:%02d:%02d (UTC)\n", ((time[0] & 0xff) << 8) + (time[1] & 0xff),
				time[2], time[3], time[4]);
			copy_to_user((void *)arg, &time, 5);
			mode = 0;  // always return to mode 0
			break;
		}
		case VFDSETTIME:  // mode 1 only
		{
			if (hchs8100_fp->u.time.time != 0)
			{
				dprintk(10, "Set RTC time to (MJD=) %d - %02d:%02d:%02d (UTC)\n", ((hchs8100_fp->u.time.time[0] & 0xff) << 8) + (hchs8100_fp->u.time.time[1] & 0xff),
					hchs8100_fp->u.time.time[2], hchs8100_fp->u.time.time[3], hchs8100_fp->u.time.time[4]);
				ret = hchs8100_SetTime(hchs8100_fp->u.time.time);
			}
			mode = 0;  // always return to mode 0
			break;
		}
		case VFDSETREMOTEKEY:
		{
			if (vfddata.data[0] == KEY_POWER)
			{
				rc_power_key = 1;
				dprintk(10, "Remote control power key pressed\n");
			}
			mode = 0;  // always return to mode 0
			break;
		}
		case 0x5305:
//		case 0x5401:
		{
			mode = 0;  // always return to mode 0
			break;
		}
		default:
		{
			dprintk(1, "%s: Unknown IOCTL %08x\n", __func__, cmd);
			mode = 0;
			break;
		}
	}
//	dprintk(150, "%s <\n", __func__);
	return ret;
}

struct file_operations vfd_fops =
{
	.owner   = THIS_MODULE,
	.ioctl   = vfd_ioctl,
	.write   = vfd_write,
	.read    = vfd_read,
	.open    = vfd_open,
	.release = vfd_close
};
// vim:ts=4
