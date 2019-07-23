/*
 * adb_box_pt6302.c
 *
 * (c) 20?? ??
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
 * Front panel driver for nBOX ITI-5800S(X), BSLA and BZZB models;
 * VFD driver for PT6302.
 *
 * Devices:
 *  - /dev/vfd (vfd ioctls and read/write function)
 *  - /dev/rc  (reading of key events)
 *
 *
 ****************************************************************************************
 *
 * Changes
 *
 * Date     By              Description
 * --------------------------------------------------------------------------------------
 * 20130929 B4team?         Initial version
 * 20160523 Audioniek       procfs added.
 *
 ****************************************************************************************/

#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17)
#include <linux/stm/pio.h>
#else
#include <linux/stpio.h>
#endif

#include "adb_box_fp.h"  // Global stuff
#include "adb_box_pt6302.h"  // local stuff
#include "adb_box_table.h"  // character table


static char adb_box_scp_access_char(struct scp_driver *scp, int dout)
{
	uint8_t din   = 0;
	int     outen = (dout < 0) ? 0 : 1, i;

	for (i = 0; i < 8; i++)
	{
		if (outen)
		{
			stpio_set_pin(scp->sda, (dout & 1) == 1);
		}
		stpio_set_pin(scp->scl, 0);
		udelay(delay);
		stpio_set_pin(scp->scl, 1);
		udelay(delay);
		din  = (din >> 1) | (stpio_get_pin(scp->sda) > 0 ? 0x80 : 0);
		dout = dout >> 1;
	}
	return din;
};

static inline void adb_box_scp_write_char(struct scp_driver *scp, char data)  // used
{
	stpio_set_pin(scp->scs, 0);
	adb_box_scp_access_char(scp, data);
	stpio_set_pin(scp->scs, 1);
};

static inline char adb_box_scp_read_char(struct scp_driver *scp)
{
	stpio_set_pin(scp->scs, 0);
	return adb_box_scp_access_char(scp, -1);
	stpio_set_pin(scp->scs, 1);
};

static void adb_box_scp_write_data(struct scp_driver *scp, char *data, int len)  // used
{
	int i;

	stpio_set_pin(scp->scs, 0);

	for (i = 0; i < len; i++)
	{
		adb_box_scp_access_char(scp, data[i]);
	}
	stpio_set_pin(scp->scs, 1);
};

static int adb_box_scp_read_data(struct scp_driver *scp, char *data, int len)
{
	int i;

	stpio_set_pin(scp->scs, 0);

	for (i = 0; i < len; i++)
	{
		data[i] = adb_box_scp_access_char(scp, -1);
	}
	stpio_set_pin(scp->scs, 1);
	return len;
};

//
// pt6302
//

typedef union
{
	struct
	{
		uint8_t addr: 4, cmd: 4;
	} dcram;
	struct
	{
		uint8_t addr: 3, reserved: 1, cmd: 4;
	} cgram;
	struct
	{
		uint8_t addr: 4, cmd: 4;
	} adram;
	struct
	{
		uint8_t port1: 1, port2: 1, reserved: 2, cmd: 4;
	} port;
	struct
	{
		uint8_t duty: 3, reserved: 1, cmd: 4;
	} duty;
	struct
	{
		uint8_t digits: 3, reserved: 1, cmd: 4;
	} digits;
	struct
	{
		uint8_t onoff: 2, reserved: 2, cmd: 4;
	} lights;
	uint8_t all;
} pt6302_command_t;

#if 0
struct pt6302_driver
{
	struct scp_driver *scp;
};
#endif

#define pt6302_write_data(scp, data, len) adb_box_scp_write_data(scp, data, len)
#define pt6302_write_char(scp, data)      adb_box_scp_write_char(scp, data)

void pt6302_free(struct pt6302_driver *ptd);

struct pt6302_driver *pt6302_init(struct scp_driver *scp)
{
	struct pt6302_driver *ptd = NULL;

	dprintk(10, "%s (scp = %p)", __func__, scp);

	if (scp == NULL)
	{
		dprintk(1, "Failed to access scp driver; abort.");
		return NULL;
	}
	ptd = (struct pt6302_driver *)kzalloc(sizeof(struct pt6302_driver), GFP_KERNEL);
	if (ptd == NULL)
	{
		dprintk(1, "Unable to allocate pt6302 driver struct; abort.");
		goto pt6302_init_fail;
	}
	ptd->scp = scp;
	return ptd;

pt6302_init_fail:
	pt6302_free(ptd);
	return NULL;
}

// display on VFD
/******************************************************
 *
 * pt6302_write_dcram
 *
 * Displays a string on the VFD display.
 *
 */
int pt6302_write_dcram(struct pt6302_driver *ptd, unsigned char addr, unsigned char *data, unsigned char len)
{
	pt6302_command_t cmd;
	uint8_t          *wdata;

	int              i = 0;
	int              j = 0;
	int              char_index = 0x00;
	char             led_txt[16];

	dprintk(1, "Text [%s] (len =%d)\n", data, len); // display what comes on screen

	if ((data[0] == 'l') && (data[1] == '3') && (data[2] == 'd')) // if string starts with l3d
	{
		goto led_control;  // set LEDs
	}
	wdata = kmalloc(1 + len_vfd, GFP_KERNEL);

	if (wdata == NULL)
	{
		dprintk(1,"Unable to allocate write buffer of %d bytes.\n", len_vfd + 1);
		return -ENOMEM;
	}

	memset(wdata, ' ', len_vfd + 1); // fill buffer with spaces
	cmd.dcram.cmd  = PT6302_COMMAND_DCRAM_WRITE;
	cmd.dcram.addr = (addr & 0xf);
	wdata[0] = cmd.all;

	len = (len <= len_vfd) ? len : len_vfd;

	// clean LED
	for (i = 0; i < 16; i++)
	{
		led_txt[i] = 0x20;  //space
	}

	// skipping characters outside STANDARD table
	for (i = 0; i < len; i++)
	{
		if (data[i] < 0x80)
		{
			wdata[len_vfd - j] = pt6302_adb_box_rom_table[data[i]];  // select VFD display
			j++;
		}
		else
		{
			dprintk(1, "[%x]\n", data[i]); // display the characters > 0x7f on VFD and LED

			char_index = 0x00;

			if ((data[i] == 0xc4) && (data[i + 1] == 0x85))
			{
				char_index = 0x61;  //0xc4,0x85 ą
			}
			if ((data[i] == 0xc4) && (data[i + 1] == 0x87))
			{
				char_index = 0x63;  //0xc4,0x87 ć
			}
			if ((data[i] == 0xc4) && (data[i + 1] == 0x99))
			{
				char_index = 0x65;  //0xc4,0x99 ę
			}
			if ((data[i] == 0xc5) && (data[i + 1] == 0x82))
			{
				char_index = 0x6c;  //0xc5,0x82 ł
			}
			if ((data[i] == 0xc5) && (data[i + 1] == 0x84))
			{
				char_index = 0x6e;  //0xc3,0xb3 ń
			}
			if ((data[i] == 0x87) && (data[i + 1] == 0xc5))
			{
				char_index = 0x73;  //0x87,0xc5 ś
			}
			if ((data[i] == 0xc3) && (data[i + 1] == 0xb3))
			{
				char_index = 0x6f;  //0xc3,0xb3 ó
			}
			if ((data[i] == 0xc5) && (data[i + 1] == 0xbc))
			{
				char_index = 0x7a;  //0xc5,0xbc ż
			}
			if ((data[i] == 0xc5) && (data[i + 1] == 0xba))
			{
				char_index = 0x7a;  //0xc5,0xba ź
			}
			if (char_index != 0x00)
			{
				wdata[len_vfd - j] = pt6302_adb_box_rom_table[char_index];  // select the display
				j++;
			}
		}
	}
	pt6302_write_data(ptd->scp, wdata, len_vfd + 1);

// support LED
	for (i = 0; i < 16; i++)
	{
		led_txt[i] = wdata[len_vfd - i];
	}
	pt6958_display(led_txt);
	return 0;

led_control:  // Set the LEDs
	dprintk(1, "led: 0x%x stan: 0x%x", data[4], data[6]);

	if ((data[4] == '1') && (data[6] == '0'))
	{
		pt6958_led_control(PT6958_CMD_ADDR_LED1, 0);  // set LED1 off
	}
	if ((data[4] == '1') && (data[6] == '1'))
	{
		pt6958_led_control(PT6958_CMD_ADDR_LED1, 1);  // set LED1 on (red)
	}
	if ((data[4] == '1') && (data[6] == '2'))
	{
		pt6958_led_control(PT6958_CMD_ADDR_LED1, 2);  // set LED1 on (green)
	}
	if ((data[4] == '1') && (data[6] == '3'))
	{
		pt6958_led_control(PT6958_CMD_ADDR_LED1, 3);  // set LED1 on (yellow)
	}

	if ((data[4] == '2') && (data[6] == '0'))
	{
		pt6958_led_control(PT6958_CMD_ADDR_LED2, 0);  // set LED2 off
	}
	if ((data[4] == '2') && (data[6] == '1'))
	{
		pt6958_led_control(PT6958_CMD_ADDR_LED2, 1);  // set LED2 on
	}

	if ((data[4] == '3') && (data[6] == '0'))
	{
		pt6958_led_control(PT6958_CMD_ADDR_LED3, 0);  // set LED3 off
	}
	if ((data[4] == '3') && (data[6] == '1'))
	{
		pt6958_led_control(PT6958_CMD_ADDR_LED3, 1);  // set LED3 on
	}

	if ((data[4] == '4') && (data[6] == '0'))
	{
		pt6958_led_control(PT6958_CMD_ADDR_LED4, 0);  // set LED4 off
	}
	if ((data[4] == '4') && (data[6] == '1'))
	{
		pt6958_led_control(PT6958_CMD_ADDR_LED4, 1);  // set LED4 on
	}
	return 0;

}

// brightness control
#define PT6302_DUTY_MIN  2
#define PT6302_DUTY_MAX  7

/******************************************************
 *
 * pt6302_set_brightness
 *
 * Sets brightness of the VFD display.
 *
 */
void pt6302_set_brightness(struct pt6302_driver *ptd, int level)
{
	pt6302_command_t cmd;

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

	pt6302_write_char(ptd->scp, cmd.all);
}

// setting the size of char_index
#define PT6302_DIGITS_MIN    9
#define PT6302_DIGITS_MAX    16
#define PT6302_DIGITS_OFFSET 8

/******************************************************
 *
 * pt6302_set_digits
 *
 * Sets width of the VFD display.
 *
 */
static void pt6302_set_digits(struct pt6302_driver *ptd, int num)
{
	pt6302_command_t cmd;

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

	pt6302_write_char(ptd->scp, cmd.all);
}

/******************************************************
 *
 * pt6302_set_lights
 *
 * Sets VFD display to normal, all segments off or all
 * segments on.
 *
 */
void pt6302_set_lights(struct pt6302_driver *ptd, int onoff)
{
	pt6302_command_t cmd;

	if (onoff < PT6302_LIGHTS_NORMAL || onoff > PT6302_LIGHTS_ON)
	{
		onoff = PT6302_LIGHTS_ON; // if illegal input, set all segments on?
	}
	cmd.lights.cmd   = PT6302_COMMAND_SET_LIGHTS;
	cmd.lights.onoff = onoff;

	pt6302_write_char(ptd->scp, cmd.all);

	// jasnosc didek i lfd
//	pt6958_write(PT6958_CMD_DISPLAY_ON, NULL, 0);
}

/******************************************************
 *
 * pt6302_set_ports
 *
 * Sets the binary output pins of the PT6302 to the
 * states specified.
 *
 */
void pt6302_set_ports(struct pt6302_driver *ptd, int port1, int port2)  // used
{
	pt6302_command_t cmd;

	cmd.port.cmd   = PT6302_COMMAND_SET_PORTS;
	cmd.port.port1 = (port1) ? 1 : 0;
	cmd.port.port2 = (port2) ? 1 : 0;

	pt6302_write_char(ptd->scp, cmd.all);
}

#if 0  // not used anywhere
static uint8_t PT6302_CLEAR_DATA[] =
{
	0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20
};
#endif

/******************************************************
 *
 * pt6302_setup
 *
 * Initializes the VFD display.
 *
 */
void pt6302_setup(struct pt6302_driver *pfd)
{
	int i;

	dprintk(1, "Initialize PT6302 VFD controller.");

	pt6302_set_ports(pfd, 1, 0);

	//pt6302_set_digits( pfd, PT6302_DIGITS_MAX );
	pt6302_set_digits(pfd, len_vfd);
	pt6302_set_brightness(pfd, PT6302_DUTY_MIN);
	pt6302_set_lights(pfd, PT6302_LIGHTS_NORMAL);

	dprintk(1, "Setup of PT6302 completed.");

#if 0
	if (rec)
	{
		pt6302_write_dcram(pfd, 0x0, "       []       ", 16);  // welcome
		pt6958_display("-  -");
		for (i = 0; i < 50; i++)
		{
			udelay(2500);
		}
		pt6302_write_dcram(pfd, 0x0, "      [  ]      ", 16);  // welcome
		for (i = 0; i < 50; i++)
		{
			udelay(2500);
		}
		pt6302_write_dcram(pfd, 0x0, "     [ ** ]     ", 16);  // welcome
		pt6958_display(" -- ");
		for (i = 0; i < 50; i++)
		{
			udelay(2500);
		}
		pt6302_write_dcram(pfd, 0x0, "    [ *  * ]    ", 16);  // welcome
		for (i = 0; i < 50; i++)
		{
			udelay(2500);
		}
		pt6302_write_dcram(pfd, 0x0, "   [ *    * ]   ", 16);  // welcome
		pt6958_display(" bm ");
		for (i = 0; i < 50; i++)
		{
			udelay(1500);
		}
		pt6302_write_dcram(pfd, 0x0, "  [ *  Bm  * ]  ", 16);  // welcome
		for (i = 0; i < 50; i++)
		{
			udelay(2500);
		}
		pt6302_write_dcram(pfd, 0x0, " [ *  B4am  * ] ", 16);  // welcome
		pt6958_display("b4tm");
		for (i = 0; i < 50; i++)
		{
			udelay(2500);
		}
		pt6302_write_dcram(pfd, 0x0, "[ *  B4Team  * ]", 16);  // welcome
		pt6958_display("b4tm");
		for (i = 0; i < 150; i++)
		{
			udelay(4000);
		}
		pt6302_write_dcram(pfd, 0x0, "[ *= B4Team =* ]", 16);  // welcome
		pt6958_display("05.03");
	}
	else
	{
		pt6958_display(led_txt);
	}
#endif
	for (i = 0; i < 150; i++)
	{
		udelay(5000);
	}
	pt6302_write_dcram(pfd, 0x0, "[              ]", 16);  // welcome
	pt6958_display("    ");  // blank LED display
//	pt6958_led_control(PT6958_CMD_ADDR_LED1, 2 );
}


void pt6302_free(struct pt6302_driver *ptd)  // used
{
	if (ptd == NULL)
	{
		return;
	}
	pt6302_set_lights(ptd, PT6302_LIGHTS_OFF);
	pt6302_set_brightness(ptd, PT6302_DUTY_MAX);
	pt6302_set_digits(ptd, PT6302_DIGITS_MIN);
	pt6302_set_ports(ptd, 0, 0);

	ptd->scp = NULL;
}
// vim:ts=4
