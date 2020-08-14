/*****************************************************************************
 *
 * proton.c
 *
 * (c) 2010 Spider-Team
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
 *****************************************************************************
 *
 * Spider-Box HL101 frontpanel driver.
 *
 * Devices:
 *	- /dev/vfd (vfd ioctls and read/write function)
 *	- /dev/rc  (reading of key events)
 *
 *****************************************************************************
 *
 * The front panel is constructed around a Princeton PT6311-LQ display driver
 * IC of which the generic switch inputs and the LED outputs are not used.
 *
 * The driver IC is controlled using three PIO driven lines ('bit-banged').
 *
 * The VFD display used is very elaborate as it features:
 * - Clock area, Four 7-segment digits with a colon but without periods;
 * - Txt area, Eight 15 segment digits, without periods;
 * - Icon, 45 in total (not counting the colon in the clock area).
 *
 * As the controller is to be regarded as non-intelligent and has no time
 * capabilities, the receiver cannot be woken up from deep standby by it. As
 * As a result, the HL101 cannot make timed recordings from standby in the
 * normal Enigma2/Neutrino configuration.
 *
 *****************************************************************************
 *
 * Changes
 *
 * Date     By              Description
 * --------------------------------------------------------------------------
 * 201????? Spider Team?    Initial version.
 * 20200731 Audioniek       Default GMT offset changed to plus one hour.
 *
 *****************************************************************************/

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/termbits.h>
#include <linux/kthread.h>
#include <linux/version.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/poll.h>
#include <linux/workqueue.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17)
#include <linux/stm/pio.h>
#else
#include <linux/stpio.h>
#endif

#include "proton.h"
#include "utf.h"

#include <linux/device.h>  /* class_creatre */
#include <linux/cdev.h>    /* cdev_init */

static int gmt_offset = 3600;

static char *gmt = "+3600";

static tFrontPanelOpen FrontPanelOpen [LASTMINOR];

/* I make this static, I think if there are more
 * then 100 commands to transmit something went
 * wrong ... point of no return
 */
struct transmit_s transmit[cMaxTransQueue];

static int transmitCount = 0;

static wait_queue_head_t   wq;

struct receive_s receive[cMaxReceiveQueue];

static int receiveCount = 0;

/* waiting retry counter, to stop waiting on ack */
static int waitAckCounter = 0;

static int timeoutOccured = 0;
static int dataReady = 0;

struct semaphore write_sem;
struct semaphore rx_int_sem;  /* unused until the irq works */
struct semaphore transmit_sem;
struct semaphore receive_sem;
struct semaphore key_mutex;

static struct semaphore display_sem;

// Place to store previous data
static struct saved_data_s lastdata;

/* last received ioctl command. we dont queue answers
 * from "getter" requests to the fp. they are protected
 * by a semaphore and the threads goes to sleep until
 * the answer has been received or a timeout occurs.
 */
int writePosition = 0;
int readPosition = 0;
unsigned char receivedData[BUFFERSIZE];

unsigned char str[64];
static SegAddrVal_T VfdSegAddr[15];

struct rw_semaphore vfd_rws;
/**************************************************
 *
 * Character table, upper case letters
 * (for 15-segment main text field)
 *
 */
unsigned char ASCII[48][2] =
{  //                   Char offs ASCII
	{0xF1, 0x38},	// A 0x00 0x41
	{0x74, 0x72},	// B 0x01 0x42
	{0x01, 0x68},	// C 0x02 0x43
	{0x54, 0x72},	// D 0x03 0x44
	{0xE1, 0x68},	// E 0x04 0x45
	{0xE1, 0x28},	// F 0x05 0x46
	{0x71, 0x68},	// G 0x06 0x47
	{0xF1, 0x18},	// H 0x07 0x48
	{0x44, 0x62},	// I 0x08 0x49
	{0x45, 0x22},	// J 0x09 0x4A
	{0xC3, 0x0C},	// K 0x0A 0x4B
	{0x01, 0x48},	// L 0x0B 0x4D
	{0x51, 0x1D},	// M 0x0C 0x4E
	{0x53, 0x19},	// N 0x0D 0x4F
	{0x11, 0x78},	// O 0x0E 0x4F
	{0xE1, 0x38},	// P 0x0F 0x50
	{0x13, 0x78},	// Q 0x10 0x51
	{0xE3, 0x38},	// R 0x11 0x52
	{0xF0, 0x68},	// S 0x12 0x53
	{0x44, 0x22},	// T 0x13 0x54
	{0x11, 0x58},	// U 0x14 0x55
	{0x49, 0x0C},	// V 0x15 0x56
	{0x5B, 0x18},	// W 0x16 0x57
	{0x4A, 0x05},	// X 0x17 0x58
	{0x44, 0x05},	// Y 0x18 0x59
	{0x48, 0x64},	// Z 0x19 0x5A
	{0x01, 0x68},   // [ 0x1A 0x5B
	{0x42, 0x01},   // \ 0x1B 0x5C 
	{0x10, 0x70},   // ] 0x1C 0x5D
	{0x43, 0x09},   //   0x1D 0x5E (back quote)
	{0x00, 0x40},   // _ 0x1E 0x5F

#define OFFSET_ASCII1 0x2a - 0x1f // -> 0x0b
	{0xEE, 0x07},   // * 0x1F 0x2a
	{0xE4, 0x02},   // + 0x20 0x2b
	{0x04, 0x00},   // , 0x21 0x2c
	{0xE0, 0x00},   // - 0x22 0x2d
	{0x04, 0x00},   // . 0x23 0x2e
	{0x48, 0x04},   // / 0x24 0x2f
	{0x11, 0x78},	// 0 0x25 0x30
	{0x44, 0x02},	// 1 0x26 0x31
	{0xE1, 0x70},	// 2 0x27 0x32
	{0xF0, 0x70},	// 3 0x28 0x33
	{0xF0, 0x18},	// 4 0x29 0x34
	{0xF0, 0x68},	// 5 0x2A 0x35
	{0xF1, 0x68},	// 6 0x2B 0x36
	{0x10, 0x30},	// 7 0x2C 0x37
	{0xF1, 0x78},	// 8 0x2D 0x38
	{0xF0, 0x78},	// 9 0x2E 0x39
#define OFFSET_ASCII2 0x2f
	{0x00, 0x00}    //   0x2F, End of table, code for a space
};

/**************************************************
 *
 * Character table, digits
 * (for 15-segment main text field)
 *
 */
unsigned char NumLib[10][2] =
{
	{0x77, 0x77},  // {01110111, 01110111},
	{0x24, 0x22},  // {00100100, 00010010},
	{0x6B, 0x6D},  // {01101011, 01101101},
	{0x6D, 0x6B},  // {01101101, 01101011},
	{0x3C, 0x3A},  // {00111100, 00111010},
	{0x5D, 0x5B},  // {01011101, 01011011},
	{0x5F, 0x5F},  // {01011111, 01011111},
	{0x64, 0x62},  // {01100100, 01100010},
	{0x7F, 0x7F},  // {01111111, 01111111},
	{0x7D, 0x7B}   // {01111101, 01111011},
};

/**************************************************
 *
 * Bit-bang routines to drive PT6311
 *
 */
// Set Pio data pin direction
static int PROTONfp_Set_PIO_Mode(PIO_Mode_T Mode_PIO)
{
	int ret = 0;

	if (Mode_PIO == PIO_Out)
	{
		stpio_configure_pin(cfg.data, STPIO_OUT);
	}
	else if (Mode_PIO == PIO_In)
	{
		stpio_configure_pin(cfg.data, STPIO_IN);
	}
	return ret;
}

// Read one byte from PT6311 (key data)
unsigned char PROTONfp_RD(void)
{
	int i;
	unsigned char val = 0, data = 0;

	down_read(&vfd_rws);

	PROTONfp_Set_PIO_Mode(PIO_In);  // switch data pin to input
	for (i = 0; i < 8; i++)
	{
		val >>= 1;
		VFD_CLK_CLR();
		udelay(1);
		data = stpio_get_pin(cfg.data);
		VFD_CLK_SET();
		if (data)
		{
			val |= 0x80;
		}
		VFD_CLK_SET();
		udelay(1);
	}
	udelay(1);
	PROTONfp_Set_PIO_Mode(PIO_Out);    // switch data pin to output
	up_read(&vfd_rws);
	return val;
}

// Write one byte to PT6311
static int VFD_WR(unsigned char data)
{
	int i;

	down_write(&vfd_rws);

	for (i = 0; i < 8; i++)
	{
		VFD_CLK_CLR();
		if (data & 0x01)
		{
			VFD_DATA_SET();
		}
		else
		{
			VFD_DATA_CLR();
		}
		VFD_CLK_SET();
		data >>= 1;
	}
	up_write(&vfd_rws);
	return 0;
}

// Initialize PT6311 segment addressing
void VFD_Seg_Addr_Init(void)
{
	unsigned char i, addr = 0xC0;//adress flag

	for (i = 0; i < 13; i++)  // grid counter
	{
		VfdSegAddr[i + 1].CurrValue1 = 0;
		VfdSegAddr[i + 1].CurrValue2 = 0;
		VfdSegAddr[i + 1].Segaddr1 = addr;
		VfdSegAddr[i + 1].Segaddr2 = addr + 1;
		addr += 3;
	}
}

static int VFD_Seg_Dig_Seg(unsigned char dignum, SegNum_T segnum, unsigned char val)
{
	unsigned char addr = 0;

	if (segnum < 0 && segnum > 1)
	{
		dprintk(1, "bad parameter!\n");
		return -1;
	}
	VFD_CS_CLR();
	if (segnum == SEGNUM1)
	{
		addr = VfdSegAddr[dignum].Segaddr1;
		VfdSegAddr[dignum].CurrValue1 = val;
	}
	else if (segnum == SEGNUM2)
	{
		addr = VfdSegAddr[dignum].Segaddr2;
		VfdSegAddr[dignum].CurrValue2 = val;
	}
	VFD_WR(addr);
	udelay(10);
	VFD_WR(val);
	VFD_CS_SET();
	return  0;
}

// Set PT6311 mode to read key or write RAM
static int VFD_Set_Mode(VFDMode_T mode)
{
	unsigned char data = 0;

	if (mode == VFDWRITEMODE)
	{
		data = PT6311_DATASET + PT6311_FIXEDADR;  // 0x44 -> 0100 0100 -> Dataset
		VFD_CS_CLR();
		VFD_WR(data);
		VFD_CS_SET();
	}
	else if (mode == VFDREADMODE)
	{
		data = PT6311_DATASET + PT6311_FIXEDADR + PT6311_READKEY;  // 0x46 = 0100 01100 
		VFD_WR(data);
		udelay(5);
	}
	return 0;
}

static int VFD_Show_Content(void)
{
	down_interruptible(&display_sem); //maybe no need. need more testing

	VFD_CS_CLR();
	VFD_WR(PT6311_DISP_CTL + PT6311_DISP_ON + PT6311_DISPBRMAX);  // 0x8f -> 1000 1111 
	VFD_CS_SET();

	up(&display_sem);

	udelay(20);
	return 0;
}

static int VFD_Show_Content_Off(void)
{
	VFD_WR(PT6311_DISP_CTL+ PT6311_DISPBRMAX); // 0x87 -> 10000111 -> display off
	return 0;
}

void VFD_Clear_All(void)
{
	int i;
	for (i = 0; i < 13; i++)
	{
		VFD_Seg_Dig_Seg(i + 1, SEGNUM1, 0x00);
		VFD_Show_Content();
		VfdSegAddr[i + 1].CurrValue1 = 0x00;
		VFD_Seg_Dig_Seg(i + 1, SEGNUM2, 0x00);
		VFD_Show_Content();
		VfdSegAddr[i + 1].CurrValue2 = 0;
	}
}

// Show one character on specified position (text section)
void VFD_Draw_ASCII_Char(char c, unsigned char position)
{
	if (position < 1 || position > 8)
	{
		dprintk(1, "Character position error! %d\n", position);
		return;
	}
	if (c >= 'A' && c <= 0x5F)  // if upper case character
	{
		c = c - 'A';  // convert to table offset
	}
	else if (c >= 'a' && c <= 'z')
	{
		c = c - 'a';  // convert table offset (upper case)
	}
	else if (c >= '*' && c <= '9')  // if digit
	{
		c = c - OFFSET_ASCII1; // convert to digit offset
	}
	else
	{
		dprintk(1, "Unknown character!\n");
		return;
	}
	VFD_Seg_Dig_Seg(position, SEGNUM1, ASCII[(unsigned char)c][0]);
	VFD_Seg_Dig_Seg(position, SEGNUM2, ASCII[(unsigned char)c][1]);

	VFD_Show_Content();
}

// Digit in clock section on specified position
void VFD_Draw_Num(unsigned char c, unsigned char position)
{
	int dignum;

	if (position < 1 || position > 4)
	{
		dprintk(2, "Numeric position error! %d\n", position);
		return;
	}
	if (c > 9)
	{
		dprintk(1, "Illegal value!\n");
		return;
	}
	dignum = 10 - position / 3;
	if (position % 2 == 1)
	{
		if (NumLib[c][1] & 0x01)
		{
			VFD_Seg_Dig_Seg(dignum, SEGNUM1, VfdSegAddr[dignum].CurrValue1 | 0x80);
		}
		else
		{
			VFD_Seg_Dig_Seg(dignum, SEGNUM1, VfdSegAddr[dignum].CurrValue1 & 0x7F);
		}
		VfdSegAddr[dignum].CurrValue2 = VfdSegAddr[dignum].CurrValue2 & 0x40;//sz
		VFD_Seg_Dig_Seg(dignum, SEGNUM2, (NumLib[c][1] >> 1) | VfdSegAddr[dignum].CurrValue2);
	}
	else if (position % 2 == 0)
	{
		if ((NumLib[c][0] & 0x01))
		{
			VFD_Seg_Dig_Seg(dignum, SEGNUM2, VfdSegAddr[dignum].CurrValue2 | 0x40);// SZ  08-05-30
		}
		else
		{
			VFD_Seg_Dig_Seg(dignum, SEGNUM2, VfdSegAddr[dignum].CurrValue2 & 0x3F);
		}
		VfdSegAddr[dignum].CurrValue1 = VfdSegAddr[dignum].CurrValue1 & 0x80;
		VFD_Seg_Dig_Seg(dignum, SEGNUM1, (NumLib[c][0] >> 1) | VfdSegAddr[dignum].CurrValue1);
	}
}

// Write time in clock section
static int VFD_Show_Time(int hh, int mm)
{
	int res = 0;

	if ((hh > 23) && (mm > 59))
	{
		dprintk(1, "%s Bad value!\n", __func__);
		return res;
	}
	VFD_Draw_Num((hh / 10), 1);
	VFD_Draw_Num((hh % 10), 2);
	VFD_Draw_Num((mm / 10), 3);
	VFD_Draw_Num((mm % 10), 4);
	VFD_Show_Content();
	return 0;
}

static int VFD_Show_Ico(LogNum_t log_num, int log_stat)
{
	int dig_num = 0, seg_num = 0;
	SegNum_T seg_part = 0;
	u8  seg_offset = 0;
	u8  addr = 0, val = 0;

	if (log_num >= LogNum_Max)
	{
		dprintk(1, "%s Bad value!\n", __func__);
		return -1;
	}
	dig_num = log_num / 16;
	seg_num = log_num % 16;
	seg_part = seg_num / 9;

	VFD_CS_CLR();
	if (seg_part == SEGNUM1)
	{
		seg_offset = 0x01 << ((seg_num % 9) - 1);
		addr = VfdSegAddr[dig_num].Segaddr1;
		if (log_stat == LOG_ON)
		{
			VfdSegAddr[dig_num].CurrValue1 |= seg_offset;
		}
		if (log_stat == LOG_OFF)
		{
			VfdSegAddr[dig_num].CurrValue1 &= (0xFF - seg_offset);
		}
		val = VfdSegAddr[dig_num].CurrValue1 ;
	}
	else if (seg_part == SEGNUM2)
	{
		seg_offset = 0x01 << ((seg_num % 8) - 1);
		addr = VfdSegAddr[dig_num].Segaddr2;
		if (log_stat == LOG_ON)
		{
			VfdSegAddr[dig_num].CurrValue2 |= seg_offset;
		}
		if (log_stat == LOG_OFF)
		{
			VfdSegAddr[dig_num].CurrValue2 &= (0xFF - seg_offset);
		}
		val = VfdSegAddr[dig_num].CurrValue2 ;
	}
	VFD_WR(addr);
	udelay(5);
	VFD_WR(val);
	VFD_CS_SET();
	VFD_Show_Content();

	return 0;
}

static struct task_struct *thread;
static int thread_stop  = 1;

void clear_display(void)
{
	int j;
	for (j = 0; j < 8; j++)
	{
		VFD_Seg_Dig_Seg(j + 1, SEGNUM1, 0x00);
		VFD_Seg_Dig_Seg(j + 1, SEGNUM2, 0x00);
	}
	VFD_Show_Content();
}

void draw_thread(void *arg)
{
	struct vfd_ioctl_data *data;
	struct vfd_ioctl_data draw_data;
	int count = 0;
	int pos = 0;
	int k = 0;
	int j = 0;
	unsigned char c0;
	unsigned char c1;
	unsigned char temp;
	unsigned char draw_buf[64][2];

	data = (struct vfd_ioctl_data *)arg;

	draw_data.length = data->length;
	memcpy(draw_data.data, data->data, data->length);

	thread_stop = 0;

	clear_display();

	while (pos < draw_data.length)
	{
		if (kthread_should_stop())
		{
			thread_stop = 1;
			return;
		}
		c0 = c1 = temp = 0;

		if (draw_data.data[pos] == 32)
		{
			k++;
			if (k == 3)
			{
				count = count - 2;
				break;
			}
		}
		else
		{
			k = 0;
		}
		if (draw_data.data[pos] < 0x80)
		{
			temp = draw_data.data[pos];

			if (temp == '\n' || temp == 0)  // skip NULLs and LFs
			{
				break;
			}
			else if (temp == '(' || temp == ')')  // replace parenthesis by spaces
			{
				temp = ' ';
			}
			else if (temp >= 'A' && temp <= '_')
			{
				temp = temp - 'A';
			}
			else if (temp >= 'a' && temp <= 'z')
			{
				temp = temp - 'a';
			}
			else if (temp >= '*' && temp <= '9')
				temp = temp - OFFSET_ASCII1;
			else if (temp == ' ')
			{
				temp = OFFSET_ASCII2;
			}			if (temp < OFFSET_ASCII2 + 1)  // if contained in table
			{
				c0 = ASCII[temp][0];  // get
				c1 = ASCII[temp][1];  // segment data
			}
		}
		// handle possible UTF-8
		else if (draw_data.data[pos] < 0xE0)
		{
			pos++;
			switch (draw_data.data[pos - 1])
			{
				case 0xc2:
				{
					c0 = UTF_C2[draw_data.data[pos] & 0x3f][0];
					c1 = UTF_C2[draw_data.data[pos] & 0x3f][1];
					break;
				}
				case 0xc3:
				{
					c0 = UTF_C3[draw_data.data[pos] & 0x3f][0];
					c1 = UTF_C3[draw_data.data[pos] & 0x3f][1];
					break;
				}
				case 0xc4:
				{
					c0 = UTF_C4[draw_data.data[pos] & 0x3f][0];
					c1 = UTF_C4[draw_data.data[pos] & 0x3f][1];
					break;
				}
				case 0xc5:
				{
					c0 = UTF_C5[draw_data.data[pos] & 0x3f][0];
					c1 = UTF_C5[draw_data.data[pos] & 0x3f][1];
					break;
				}
				case 0xd0:
				{
					c0 = UTF_D0[draw_data.data[pos] & 0x3f][0];
					c1 = UTF_D0[draw_data.data[pos] & 0x3f][1];
					break;
				}
				case 0xd1:
				{
					c0 = UTF_D1[draw_data.data[pos] & 0x3f][0];
					c1 = UTF_D1[draw_data.data[pos] & 0x3f][1];
					break;
				}
			}
		}
		else
		{
			if (draw_data.data[pos] < 0xF0)
			{
				pos += 2;
			}
			else if (draw_data.data[pos] < 0xF8)
			{
				pos += 3;
			}
			else if (draw_data.data[pos] < 0xFC)
			{
				pos += 4;
			}
			else
			{
				pos += 5;
			}
		}
		// end of UTF-8 handling
		draw_buf[count][0] = c0;  // set
		draw_buf[count][1] = c1;  // segment data
		count++;
		pos++;
	}

#ifdef ENABLE_SCROLL // j00zek: disabled by default, interferes with GUI's
	if (count > 8)
	{
		pos  = 0;
		while (pos < count)
		{
			if (kthread_should_stop())
			{
				thread_stop = 1;
				return;
			}
			k = 8;
			if (count - pos < 8)
			{
				k = count - pos;
			}
			clear_display();

			for (j = 0; j < k; j++)
			{
				VFD_Seg_Dig_Seg(j + 1, SEGNUM1, draw_buf[pos + j][0]);
				VFD_Seg_Dig_Seg(j + 1, SEGNUM2, draw_buf[pos + j][1]);
			}
			VFD_Show_Content();
			msleep(200);
			pos++;
		}
	}
#endif
	if (count > 0)
	{
		k = 8;
		if (count < 8)
		{
			k = count;
		}
		if (kthread_should_stop())
		{
			thread_stop = 1;
			return;
		}
		clear_display();

		for (j = 0; j < k; j++)
		{
			VFD_Seg_Dig_Seg(j + 1, SEGNUM1, draw_buf[j][0]);
			VFD_Seg_Dig_Seg(j + 1, SEGNUM2, draw_buf[j][1]);
		}
		VFD_Show_Content();
	}

	if (count == 0)
	{
		clear_display();
	}
	thread_stop = 1;
}

int run_draw_thread(struct vfd_ioctl_data *draw_data)
{
	if (!thread_stop && thread)
	{
		kthread_stop(thread);
	}
	//wait thread stop
	while (!thread_stop)
	{
		msleep(5);
	}
	msleep(10);

	thread_stop = 2;
	thread = kthread_run(draw_thread, draw_data, "draw_thread");

	//wait thread run
	while (thread_stop == 2)
	{
		msleep(5);
	}
	return 0;
}

static int VFD_Show_Time_Off(void)
{
	int ret = 0;

	ret = VFD_Seg_Dig_Seg(9, SEGNUM1, 0x00);
	ret = VFD_Seg_Dig_Seg(9, SEGNUM2, 0x00);
	ret = VFD_Seg_Dig_Seg(10, SEGNUM1, 0x00);
	ret = VFD_Seg_Dig_Seg(10, SEGNUM2, 0x00);
	return ret;
}

// Read n bytes key data from PT6311
unsigned char PROTONfp_Scan_Keyboard(unsigned char read_num)
{
	unsigned char key_val[read_num] ;
	unsigned char i = 0, ret;

	VFD_CS_CLR();
	ret = VFD_Set_Mode(VFDREADMODE);  // Set read from to PT6311 data
	if (ret)
	{
		dprintk(1, "%s DEVICE BUSY!\n", __func__);
		return -1;
	}

	for (i = 0; i < read_num; i++)
	{
		key_val[i] = PROTONfp_RD();
	}
	VFD_CS_SET();

	ret = VFD_Set_Mode(VFDWRITEMODE);  // SET write to PT6311 RAM
	if (ret)
	{
		dprintk(2, "%s DEVICE BUSY!\n", __func__);
		return -1;
	}
	return key_val[5];
}

static int PROTONfp_Get_Key_Value(void)
{
	unsigned char byte = 0;
	int key_val = INVALID_KEY;

	byte = PROTONfp_Scan_Keyboard(6);

	switch (byte)
	{
		case 0x01:
		{
			key_val = EXIT_KEY;
			break;
		}
		case 0x02:
		{
			key_val = KEY_LEFT;
			break;
		}
		case 0x04:
		{
			key_val = KEY_UP;
			break;
		}
		case 0x08:
		{
			key_val = KEY_OK;
			break;
		}
		case 0x10:
		{
			key_val = KEY_RIGHT;
			break;
		}
		case 0x20:
		{
			key_val = KEY_DOWN;
			break;
		}
		case 0x40:
		{
			key_val = KEY_POWER;
			break;
		}
		case 0x80:
		{
			key_val = KEY_MENU;
			break;
		}
		default :
		{
			key_val = INVALID_KEY;
			break;
		}
	}

	return key_val;
}

// Set time to show in clock part
int protonSetTime(char *time)
{
	char buffer[8];
	int  res = 0;

	dprintk(150, "%s >\n", __func__);

	dprintk(50, "%s time: %02d:%02d\n", __func__, time[2], time[3]);
	memset(buffer, 0, 8);
	memcpy(buffer, time, 5);
	VFD_Show_Time(time[2], time[3]);
	dprintk(5, "%s <\n", __func__);
	return res;
}

int vfd_init_func(void)
{
	dprintk(150, "%s >\n", __func__);
	dprintk(0, "Spider HL101 VFD module initializing\n");

	cfg.data_pin[0] = 3;
	cfg.data_pin[1] = 2;
	cfg.clk_pin[0] = 3;
	cfg.clk_pin[1] = 4;
	cfg.cs_pin[0] = 3;
	cfg.cs_pin[1] = 5;

	cfg.cs  = stpio_request_pin(cfg.cs_pin[0], cfg.cs_pin[1], "VFD CS", STPIO_OUT);
	cfg.clk = stpio_request_pin(cfg.clk_pin[0], cfg.clk_pin[1], "VFD CLK", STPIO_OUT);
	cfg.data = stpio_request_pin(cfg.data_pin[0], cfg.data_pin[1], "VFD DATA", STPIO_OUT);

	if (!cfg.cs || !cfg.data || !cfg.clk)
	{
		printk("vfd_init_func:  PIO errror!\n");
		return -1;
	}
	init_rwsem(&vfd_rws);

	VFD_CS_CLR();
	VFD_WR(PT6311_DMODESET + PT6311_13DIGIT);  // 0x0c = 0000 1100 -> 13 digit mode
	VFD_CS_SET();

	VFD_Set_Mode(VFDWRITEMODE);  // Set write to PT6311 RAM
	VFD_Seg_Addr_Init();
	VFD_Clear_All();
	VFD_Show_Content();
	return 0;
}

static void VFD_CLR(void)
{
	VFD_CS_CLR();
	VFD_WR(PT6311_DMODESET + PT6311_13DIGIT);
	VFD_CS_SET();

	VFD_Set_Mode(VFDWRITEMODE);  // set write to PT6311 RAM
	VFD_Seg_Addr_Init();
	VFD_Clear_All();
	VFD_Show_Content();
}

int protonSetIcon(int which, int on)
{
	int  res = 0;

	dprintk(5, "%s > %d, %d\n", __func__, which, on);
	if (which < 1 || which > 45)
	{
		printk("VFD/PROTON icon number out of range %d\n", which);
		return -EINVAL;
	}
	which -= 1;
	res = VFD_Show_Ico(((which / 15) + 11) * 16 + (which % 15) + 1, on);
	dprintk(10, "%s <\n", __func__);
	return res;
}
EXPORT_SYMBOL(protonSetIcon);

static ssize_t PROTONdev_write(struct file *filp, const unsigned char *buff, size_t len, loff_t *off)
{
	char *kernel_buf;
	int minor, vLoop, res = 0;

	struct vfd_ioctl_data data;

	dprintk(5, "%s > (len %d, offs %d)\n", __func__, len, (int) *off);

	minor = -1;
	for (vLoop = 0; vLoop < LASTMINOR; vLoop++)
	{
		if (FrontPanelOpen[vLoop].fp == filp)
		{
			minor = vLoop;
		}
	}

	if (minor == -1)
	{
		printk("Error Bad Minor\n");
		return -1; //FIXME
	}

	dprintk(1, "minor = %d\n", minor);

	if (minor == FRONTPANEL_MINOR_RC)
		return -EOPNOTSUPP;

	kernel_buf = kmalloc(len, GFP_KERNEL);

	if (kernel_buf == NULL)
	{
		printk("%s return no mem<\n", __func__);
		return -ENOMEM;
	}
	copy_from_user(kernel_buf, buff, len);

	if (down_interruptible(&write_sem))
		return -ERESTARTSYS;

	data.length = len;
	if (kernel_buf[len - 1] == '\n')
	{
		kernel_buf[len - 1] = 0;
		data.length--;
	}

	if (len < 0)
	{
		res = -1;
		dprintk(2, "empty string\n");
	}
	else
	{
		memcpy(data.data, kernel_buf, len);
		res = run_draw_thread(&data);
	}

	kfree(kernel_buf);

	up(&write_sem);

	dprintk(10, "%s < res %d len %d\n", __func__, res, len);

	if (res < 0)
		return res;
	else
		return len;
}

static ssize_t PROTONdev_read(struct file *filp, unsigned char __user *buff, size_t len, loff_t *off)
{
	int minor, vLoop;

	dprintk(5, "%s > (len %d, offs %d)\n", __func__, len, (int) *off);

	minor = -1;
	for (vLoop = 0; vLoop < LASTMINOR; vLoop++)
	{
		if (FrontPanelOpen[vLoop].fp == filp)
		{
			minor = vLoop;
		}
	}

	if (minor == -1)
	{
		printk("Error Bad Minor\n");
		return -EUSERS;
	}

	dprintk(1, "minor = %d\n", minor);

	if (minor == FRONTPANEL_MINOR_RC)
	{

		while (receiveCount == 0)
		{
			if (wait_event_interruptible(wq, receiveCount > 0))
				return -ERESTARTSYS;
		}

		/* 0. claim semaphore */
		down_interruptible(&receive_sem);

		/* 1. copy data to user */
		copy_to_user(buff, receive[0].buffer, receive[0].len);

		/* 2. copy all entries to start and decreas receiveCount */
		receiveCount--;
		memmove(&receive[0], &receive[1], 99 * sizeof(struct receive_s));

		/* 3. free semaphore */
		up(&receive_sem);

		return 8;
	}

	/* copy the current display string to the user */
	if (down_interruptible(&FrontPanelOpen[minor].sem))
	{
		printk("%s return erestartsys<\n", __func__);
		return -ERESTARTSYS;
	}

	if (FrontPanelOpen[minor].read == lastdata.length)
	{
		FrontPanelOpen[minor].read = 0;

		up(&FrontPanelOpen[minor].sem);
		printk("%s return 0<\n", __func__);
		return 0;
	}

	if (len > lastdata.length)
		len = lastdata.length;

	/* fixme: needs revision because of utf8! */
	if (len > 16)
		len = 16;

	FrontPanelOpen[minor].read = len;
	copy_to_user(buff, lastdata.data, len);

	up(&FrontPanelOpen[minor].sem);

	dprintk(10, "%s < (len %d)\n", __func__, len);
	return len;
}

int PROTONdev_open(struct inode *inode, struct file *filp)
{
	int minor;

	dprintk(5, "%s >\n", __func__);

	minor = MINOR(inode->i_rdev);

	dprintk(1, "open minor %d\n", minor);

	if (FrontPanelOpen[minor].fp != NULL)
	{
		printk("EUSER\n");
		return -EUSERS;
	}
	FrontPanelOpen[minor].fp = filp;
	FrontPanelOpen[minor].read = 0;

	dprintk(5, "%s <\n", __func__);
	return 0;
}

int PROTONdev_close(struct inode *inode, struct file *filp)
{
	int minor;

	dprintk(5, "%s >\n", __func__);

	minor = MINOR(inode->i_rdev);

	dprintk(1, "close minor %d\n", minor);

	if (FrontPanelOpen[minor].fp == NULL)
	{
		printk("EUSER\n");
		return -EUSERS;
	}
	FrontPanelOpen[minor].fp = NULL;
	FrontPanelOpen[minor].read = 0;

	dprintk(5, "%s <\n", __func__);
	return 0;
}

static int PROTONdev_ioctl(struct inode *Inode, struct file *File, unsigned int cmd, unsigned long arg)
{
	static int mode = 0;
	struct proton_ioctl_data *proton = (struct proton_ioctl_data *)arg;
	int res = 0;

	dprintk(5, "%s > 0x%.8x\n", __func__, cmd);

	if (down_interruptible(&write_sem))
		return -ERESTARTSYS;

	switch (cmd)
	{
		case VFDSETMODE:
			mode = proton->u.mode.compat;
			break;
		case VFDSETLED:
			break;
		case VFDBRIGHTNESS:
			break;
		case VFDICONDISPLAYONOFF:
		{
			//struct vfd_ioctl_data *data = (struct vfd_ioctl_data *) arg;
//		  res = protonSetIcon(proton->u.icon.icon_nr, proton->u.icon.on);
		}

		mode = 0;
		case VFDSTANDBY:
			break;
		case VFDSETTIME:
			//struct set_time_s *data2 = (struct set_time_s *) arg;
#ifdef ENABLE_CLOCK_SECTION
			res = protonSetTime((char *)arg);
#endif
			break;
		case VFDGETTIME:
			break;
		case VFDGETWAKEUPMODE:
			break;
		case VFDDISPLAYCHARS:
			if (mode == 0)
			{
				struct vfd_ioctl_data *data = (struct vfd_ioctl_data *) arg;
				if (data->length < 0)
				{
					res = -1;
					dprintk(2, "empty string\n");
				}
				else
					res = run_draw_thread(data);
			}
			else
			{
				//not supported
			}

			mode = 0;

			break;
		case VFDDISPLAYWRITEONOFF:
			if (proton->u.mode.compat) // 1 = show, 0 = off
				VFD_Show_Content();
			else
				VFD_Show_Content_Off();
			break;
		case VFDDISPLAYCLR:
			if (!thread_stop)
				kthread_stop(thread);

			//wait thread stop
			while (!thread_stop)
			{
				msleep(5);
			}
			VFD_CLR();
			break;
		default:
			dprintk(1, "VFD/Proton: unknown IOCTL 0x%x\n", cmd);

			mode = 0;
			break;
	}

	up(&write_sem);

	dprintk(5, "%s <\n", __func__);
	return res;
}

static unsigned int PROTONdev_poll(struct file *filp, poll_table *wait)
{
	unsigned int mask = 0;

	poll_wait(filp, &wq, wait);

	if (receiveCount > 0)
	{
		mask = POLLIN | POLLRDNORM;
	}

	return mask;
}

static struct file_operations vfd_fops =
{
	.owner = THIS_MODULE,
	.ioctl = PROTONdev_ioctl,
	.write = (void *) PROTONdev_write,
	.read  = (void *) PROTONdev_read,
	.poll  = (void *) PROTONdev_poll,
	.open  = PROTONdev_open,
	.release  = PROTONdev_close
};

/*----- Button driver -------*/

static char *button_driver_name = "Spider HL101 frontpanel buttons";
static struct input_dev *button_dev;
static int button_value = -1;
static int bad_polling = 1;
static struct workqueue_struct *fpwq;

void button_bad_polling(void)
{
	int btn_pressed = 0;
	int report_key = 0;

	while (bad_polling == 1)
	{
		msleep(50);
		button_value = PROTONfp_Get_Key_Value();
		if (button_value != INVALID_KEY)
		{
			dprintk(5, "got button: %X\n", button_value);
			VFD_Show_Ico(DOT2, LOG_ON);
			if (1 == btn_pressed)
			{
				if (report_key != button_value)
				{
					input_report_key(button_dev, report_key, 0);
					input_sync(button_dev);
				}
				else
				{
					continue;
				}
			}
			report_key = button_value;
			btn_pressed = 1;
			switch (button_value)
			{
				case KEY_LEFT:
				{
					input_report_key(button_dev, KEY_LEFT, 1);
					input_sync(button_dev);
					break;
				}
				case KEY_RIGHT:
				{
					input_report_key(button_dev, KEY_RIGHT, 1);
					input_sync(button_dev);
					break;
				}
				case KEY_UP:
				{
					input_report_key(button_dev, KEY_UP, 1);
					input_sync(button_dev);
					break;
				}
				case KEY_DOWN:
				{
					input_report_key(button_dev, KEY_DOWN, 1);
					input_sync(button_dev);
					break;
				}
				case KEY_OK:
				{
					input_report_key(button_dev, KEY_OK, 1);
					input_sync(button_dev);
					break;
				}
				case KEY_MENU:
				{
					input_report_key(button_dev, KEY_MENU, 1);
					input_sync(button_dev);
					break;
				}
				case KEY_EXIT:
				{
					input_report_key(button_dev, KEY_EXIT, 1);
					input_sync(button_dev);
					break;
				}
				case KEY_POWER:
				{
					input_report_key(button_dev, KEY_POWER, 1);
					input_sync(button_dev);
					break;
				}
				default:
					dprintk(5, "[BTN] unknown button_value?\n");
			}
		}
		else
		{
			if (btn_pressed)
			{
				btn_pressed = 0;
				msleep(50);
				VFD_Show_Ico(DOT2, LOG_OFF);
				input_report_key(button_dev, report_key, 0);
				input_sync(button_dev);
			}
		}
	}
}
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17)
static DECLARE_WORK(button_obj, button_bad_polling);
#else
static DECLARE_WORK(button_obj, button_bad_polling, NULL);
#endif
static int button_input_open(struct input_dev *dev)
{
	fpwq = create_workqueue("button");
	if (queue_work(fpwq, &button_obj))
	{
		dprintk(5, "[BTN] queue_work successful ...\n");
	}
	else
	{
		dprintk(5, "[BTN] queue_work not successful, exiting ...\n");
		return 1;
	}

	return 0;
}

static void button_input_close(struct input_dev *dev)
{
	bad_polling = 0;
	msleep(55);
	bad_polling = 1;

	if (fpwq)
	{
		destroy_workqueue(fpwq);
		dprintk(5, "[BTN] workqueue destroyed\n");
	}
}

int button_dev_init(void)
{
	int error;

	dprintk(5, "[BTN] allocating and registering button device\n");

	button_dev = input_allocate_device();
	if (!button_dev)
		return -ENOMEM;

	button_dev->name = button_driver_name;
	button_dev->open = button_input_open;
	button_dev->close = button_input_close;


	set_bit(EV_KEY		, button_dev->evbit);
	set_bit(KEY_UP		, button_dev->keybit);
	set_bit(KEY_DOWN	, button_dev->keybit);
	set_bit(KEY_LEFT	, button_dev->keybit);
	set_bit(KEY_RIGHT	, button_dev->keybit);
	set_bit(KEY_POWER	, button_dev->keybit);
	set_bit(KEY_MENU	, button_dev->keybit);
	set_bit(KEY_OK		, button_dev->keybit);
	set_bit(KEY_EXIT	, button_dev->keybit);

	error = input_register_device(button_dev);
	if (error)
	{
		input_free_device(button_dev);
		return error;
	}

	return 0;
}

void button_dev_exit(void)
{
	dprintk(5, "[BTN] unregistering button device\n");
	input_unregister_device(button_dev);
}

static struct cdev   vfd_cdev;
static struct class *vfd_class = 0;

static int __init proton_init_module(void)
{
	int i;

	dprintk(5, "%s >\n", __func__);

	sema_init(&display_sem, 1);

	if (vfd_init_func())
	{
		printk("unable to init module\n");
		return -1;
	}

	vfd_class = class_create(THIS_MODULE, "proton");
	device_create(vfd_class, NULL, MKDEV(VFD_MAJOR, 0), NULL, "vfd");
	cdev_init(&vfd_cdev, &vfd_fops);
	cdev_add(&vfd_cdev, MKDEV(VFD_MAJOR, 0), 1);

	if (button_dev_init() != 0)
		return -1;

	if (register_chrdev(VFD_MAJOR, "VFD", &vfd_fops))
		printk("unable to get major %d for VFD\n", VFD_MAJOR);

	sema_init(&write_sem, 1);
	sema_init(&key_mutex, 1);

	for (i = 0; i < LASTMINOR; i++)
		sema_init(&FrontPanelOpen[i].sem, 1);


	dprintk(5, "%s <\n", __func__);

	return 0;
}

static void __exit proton_cleanup_module(void)
{

	if (cfg.data != NULL)
		stpio_free_pin(cfg.data);
	if (cfg.clk != NULL)
		stpio_free_pin(cfg.clk);
	if (cfg.cs != NULL)
		stpio_free_pin(cfg.cs);

	dprintk(5, "[BTN] unloading ...\n");
	button_dev_exit();

	//kthread_stop(time_thread);

	cdev_del(&vfd_cdev);
	unregister_chrdev(VFD_MAJOR, "VFD");
	device_destroy(vfd_class, MKDEV(VFD_MAJOR, 0));
	class_destroy(vfd_class);
	printk("HL101 FrontPanel module unloading\n");
}


module_init(proton_init_module);
module_exit(proton_cleanup_module);

module_param(gmt, charp, 0);
MODULE_PARM_DESC(gmt, "GMT offset (default +3600");
module_param(paramDebug, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(paramDebug, "Debug Output 0=disabled >0=enabled(debuglevel)");

MODULE_DESCRIPTION("VFD module for Spider HL101");
MODULE_AUTHOR("Spider-Team");
MODULE_LICENSE("GPL");
// vim:ts=4
