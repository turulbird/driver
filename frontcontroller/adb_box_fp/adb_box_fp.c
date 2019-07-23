/*
 * adb_box_fp.c
 *
 * (c) 20?? B4Team?
 * (c) 2019 Audioniek
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
 * Front panel driver for nBOX ITI-5800S(X), BSKA, BSLA, BXXB and BZZB models;
 * generic part.
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

#include "adb_box_fp.h"
#include "pio.h"

/* <-----led-----> */
//#include "adb_box_pt6958.h"

/* <-----vfd-----> */
//#include "adb_box_pt6302.h"

/* <-----buttons-----> */
#include <linux/input.h>
#include <linux/version.h>
#include <linux/workqueue.h>


#define BUTTON_POLL_MSLEEP 1
#define BUTTON_POLL_DELAY  10

/* <-----button assignment-----> */
/*KEY  CODE1  CODE2
  POW    2      0
  OK     4      0
  UP    20      0
  DOWN   0      2
  LEFT   0      4
  RIGHT 80      0
  EPG   40      0
  REC    0      8
  RES    8      0
*/

// Globals (button routines)
static int                     button_polling = 1;
unsigned char                  key_group1 = 0, key_group2 = 0;
static unsigned int            key_button = 0;

int SCP_PORT = 0;

struct __vfd_scp *vfd_scp_ctrl = NULL;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
void button_poll(void)
#else
void button_poll(struct work_struct *ignored)
#endif
{
	while (button_polling == 1)
	{
		msleep(BUTTON_POLL_MSLEEP);

		if (FP_GETKEY)
		{
			pt6958_write(PT6958_CMD_READ_KEY, NULL, 0);

			udelay(1);

			key_group1 = pt6958_read_key() & 0xfe;
			key_group2 = pt6958_read_key() & 0xfe;

			if (paramDebug)
			{
				dprintk(1, "Key group [%X:%X]\n", key_group1, key_group2);
			}
			if ((key_group1 == 0x02) && (key_group2 == 0x00))
			{
				key_button = KEY_POWER;  //POW
			}
			if ((key_group1 == 0x04) && (key_group2 == 0x00))
			{
				key_button = KEY_OK;  //OK
			}
			if ((key_group1 == 0x20) && (key_group2 == 0x00))
			{
				key_button = KEY_UP;  //UP
			}
			if ((key_group1 == 0x00) && (key_group2 == 0x02))
			{
				key_button = KEY_DOWN;  //DOWN
			}
			if ((key_group1 == 0x00) && (key_group2 == 0x04))
			{
				key_button = KEY_LEFT;  //LEFT
			}
			if ((key_group1 == 0x80) && (key_group2 == 0x00))
			{
				key_button = KEY_RIGHT;  //RIGHT
			}
			if ((key_group1 == 0x40) && (key_group2 == 0x00))
			{
				if (rec)  // if bsla version
				{
					key_button = KEY_EPG;  //EPG
				}
				else  // bska version
				{
					key_button = KEY_MENU;  //MENU
				}
			}
			if ((key_group1 == 0x00) && (key_group2 == 0x08))
			{
				if (rec)  // if bsla version
				{
					key_button = KEY_MENU;  //REC
				}
				else  // bska version
				{
					key_button = KEY_EPG;  //EPG
				}
			}
			if ((key_group1 == 0x08) && (key_group2 == 0x00))
			{
				key_button = KEY_HOME;  //RES
			}
			if ((key_group1 == 0x0A) && (key_group2 == 0x00))  //RES+OK
			{
				dprintk(1, "[fp_key] reboot...\n");
				msleep(250);
				button_reset = stpio_request_pin(3, 2, "btn_reset", STPIO_OUT);
			}
			if ((key_group1 == 0x08) && (key_group2 == 0x2))
			{
				key_button = KEY_COMPOSE;  //RES+DOWN
			}
			if (key_button > 0)
			{
				dprintk(1, "fp_key: %d\n", key_button);
				input_report_key(button_dev, key_button, 1);
				input_sync(button_dev);
				msleep(250);

				input_report_key(button_dev, key_button, 0);
				input_sync(button_dev);

				// clean up variables
				key_group1 = 0;
				key_group2 = 0;
				key_button = 0;
			}
		}
	}
}

/*****************************************************
 *
 * Frontpanel button driver
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
static DECLARE_WORK(button_obj, button_poll, NULL);
#else
static DECLARE_WORK(button_obj, button_poll);
#endif

static int button_input_open(struct input_dev *dev)
{
	button_wq = create_workqueue("button");
	if (queue_work(button_wq, &button_obj))
	{
		dprintk(10, "queue_work successful...\n");
	}
	else
	{
		dprintk(1, "queue_work not successful, exiting...\n");
		return 1;
	}
	return 0;
}

static void button_input_close(struct input_dev *dev)
{
	button_polling = 0;
	msleep(55);
	button_polling = 1;

	if (button_wq)
	{
		destroy_workqueue(button_wq);
		dprintk(10, "workqueue destroyed\n");
	}
}

static unsigned char pt6958_ram[PT6958_RAM_SIZE];

int button_dev_init(void)
{
	int error;

	dprintk(0, "Allocating and button PIO pins...\n");

// PT6958 DOUT
	dprintk(20, "PT6958 DOUT pin: request STPIO %d, %d (%s, STPIO_IN\n", BUTTON_DO_PORT, BUTTON_DO_PIN, "btn_do");
	button_do = stpio_request_pin(BUTTON_DO_PORT, BUTTON_DO_PIN, "btn_do", STPIO_IN);  //[ IN  (Hi-Z) ]
	if (button_do == NULL)
	{
		dprintk(1, "Request STPIO btn_do failed; abort.\n");
		return -EIO;
	}
// PT6958 STB
	dprintk(20, "PT6958 STB pin: request STPIO %d, %d (%s, STPIO_OUT)\n", BUTTON_DS_PORT, BUTTON_DS_PIN, "btn_stb");
	button_ds = stpio_request_pin(BUTTON_DS_PORT, BUTTON_DS_PIN, "btn_stb", STPIO_OUT);  //[ OUT (push-pull) ]
	if (button_ds == NULL)
	{
		dprintk(1, "Request STPIO btn_stb failed; abort.\n");
		return -EIO;
	}

//reset ( does not exist on PT6958, but does on PT6302!)
#if 0
	dprintk(20, "PT6302 RST pin: request STPIO %d ,%d (%s, STPIO_OUT\n", BUTTON_RESET_PORT, BUTTON_RESET_PIN, "btn_reset");
	button_reset = stpio_request_pin(BUTTON_RESET_PORT, BUTTON_RESET_PIN, "btn_reset", STPIO_OUT);
	if (button_reset == NULL)
	{
		dprintk(1, "Request STPIO reset failed. abort.\n");
		return -EIO;
	}
#endif
	printk(" done.\n");

	dprintk(0, "Allocating and registering button device...");

	button_dev = input_allocate_device();
	if (!button_dev)
	{
		return -ENOMEM;
	}
	button_dev->name  =  button_driver_name;
	button_dev->open  =  button_input_open;
	button_dev->close =  button_input_close;

	set_bit(EV_KEY,    	 button_dev->evbit);
	set_bit(KEY_UP,    	 button_dev->keybit);
	set_bit(KEY_DOWN,  	 button_dev->keybit);
	set_bit(KEY_LEFT,  	 button_dev->keybit);
	set_bit(KEY_RIGHT, 	 button_dev->keybit);
	set_bit(KEY_POWER, 	 button_dev->keybit);
	set_bit(KEY_MENU,  	 button_dev->keybit);
	set_bit(KEY_EXIT,  	 button_dev->keybit);
	set_bit(KEY_EPG,   	 button_dev->keybit);
	set_bit(KEY_HOME, 	 button_dev->keybit);
	set_bit(KEY_OK,      button_dev->keybit);
	set_bit(KEY_RESTART, button_dev->keybit);
	set_bit(KEY_COMPOSE, button_dev->keybit);

	error = input_register_device(button_dev);
	if (error)
	{
		input_free_device(button_dev);
		dprintk(1, "Error registering button device!\n");
		return error;
	}

	memset(pt6958_ram, 0, PT6958_RAM_SIZE);

	pt6958_write(PT6958_CMD_WRITE_INC, NULL, 0);

	pt6958_write(PT6958_CMD_ADDR_SET, pt6958_ram, PT6958_RAM_SIZE);
	pt6958_write(PT6958_CMD_DISPLAY_ON, NULL, 0);
	printk(" done.\n");
	return 0;
}

void button_dev_exit(void)
{
	dprintk(0, "Unregistering button device");
	input_unregister_device(button_dev);
	if (button_ds)
	{
		stpio_free_pin(button_ds);
	}
	if (button_do)
	{
		stpio_free_pin(button_do);
	}
	if (button_reset)
	{
		stpio_free_pin(button_reset);
	}
	printk(" done.\n");
}

/*****************************************************
 *
 * Frontpanel VFD driver
 */
static void adb_box_scp_free(struct scp_driver *scp)
{
	if (scp == NULL)
	{
		return;
	}
	if (scp->scs)
	{
		stpio_set_pin(scp->scs, 1);
	}
	if (scp->sda)
	{
		stpio_free_pin(scp->sda);
	}
	if (scp->scl)
	{
		stpio_free_pin(scp->scl);
	}
	if (scp->scs)
	{
		stpio_free_pin(scp->scs);
	}
	if (scp)
	{
		kfree(scp);
	}
	scp = NULL;
	dprintk(10, "Removed front panel PT6302 & PT6958 driver.\n");
};

static const char stpio_vfd_scs[] = "vfd_scs";
static const char stpio_vfd_scl[] = "vfd_sck";
static const char stpio_vfd_sda[] = "vfd_sda";

static struct scp_driver *adb_box_scp_init(void)
{
	struct scp_driver *scp = NULL;

	dprintk(0, "Initialize scp driver.\n");

	scp = (struct scp_driver *)kzalloc(sizeof(struct scp_driver), GFP_KERNEL);
	if (scp == NULL)
	{
		dprintk(1, "Unable to allocate scp driver struct; abort.\n");
		goto adb_box_scp_init_fail;
	}

//PT6302
// CS
	dprintk(10, "PT6302 CS pin: request STPIO %d, %d (%s, STPIO_OUT).\n", ADB_BOX_VFD_PIO_PORT_SCS, ADB_BOX_VFD_PIO_PIN_SCS, stpio_vfd_scs);
	scp->scs = stpio_request_pin(ADB_BOX_VFD_PIO_PORT_SCS, ADB_BOX_VFD_PIO_PIN_SCS, stpio_vfd_scs, STPIO_OUT);

	if (scp->scs == NULL)
	{
		dprintk(1, "Request CS STPIO failed; abort\n");
		goto adb_box_scp_init_fail;
	}

//wspolne (??)
// CLK
	dprintk(10, "PT6302 CLK pin: request STPIO %d, %d (%s, STPIO_OUT).\n", ADB_BOX_VFD_PIO_PORT_SCL, ADB_BOX_VFD_PIO_PIN_SCL, stpio_vfd_scl);
	scp->scl = stpio_request_pin(ADB_BOX_VFD_PIO_PORT_SCL, ADB_BOX_VFD_PIO_PIN_SCL, stpio_vfd_scl, STPIO_OUT);

	if (scp->scl == NULL)
	{
		dprintk(1, "Request scl STPIO failed; abort.\n");
		goto adb_box_scp_init_fail;
	}

// DIN/DOUT
	dprintk(10, "PT6302 DIN pin: request STPIO %d, %d (%s, STPIO_BIDIR).\n", ADB_BOX_VFD_PIO_PORT_SDA, ADB_BOX_VFD_PIO_PIN_SDA, stpio_vfd_sda);
	scp->sda = stpio_request_pin(ADB_BOX_VFD_PIO_PORT_SDA, ADB_BOX_VFD_PIO_PIN_SDA, stpio_vfd_sda, STPIO_BIDIR);

	if (scp->sda == NULL)
	{
		dprintk(1, "Request sda STPIO failed; abort.\n");
		goto adb_box_scp_init_fail;
	}
	return scp;

adb_box_scp_init_fail:
	adb_box_scp_free(scp);
	return 0;
}

//----------------------------------------------------------------------------
// TODO: move to header file
struct vfd_driver
{
	struct scp_driver    *scp;
	struct pt6302_driver *ctrl;
	struct semaphore      sem;
	int                   opencount;
};

static struct vfd_driver vfd;

#define VFDIOC_DCRAMWRITE        0xc0425a00
#define VFDIOC_BRIGHTNESS        0xc0425a03
#define VFDIOC_DISPLAYWRITEONOFF 0xc0425a05
#define VFDIOC_DRIVERINIT        0xc0425a08
#define VFDIOC_ICONDISPLAYONOFF  0xc0425a0a

#define VFDGETVERSION            0xc0425af7
#define VFDLEDBRIGHTNESS         0xc0425af8
#define VFDGETWAKEUPMODE         0xc0425af9
#define VFDGETTIME               0xc0425afa
#define VFDSETTIME               0xc0425afb
#define VFDSTANDBY               0xc0425afc
#define VFDREBOOT                0xc0425afd

#define VFDSETLED                0xc0425afe
#define VFDSETMODE               0xc0425aff

#define VFD_LED_IR	             0x00000023
#define VFD_LED_POW	             0x00000024
#define VFD_LED_REC	             0x0000001e
#define VFD_LED_HD	             0x00000011
#define VFD_LED_LOCK	         0x00000013

/*****************************************************
 *
 * Frontpanel VFD IOCTL
 */
static int vfd_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	struct vfd_ioctl_data vfddata;

	switch (cmd)
	{
		case VFDIOC_DCRAMWRITE:
		case VFDIOC_BRIGHTNESS:
		case VFDIOC_DISPLAYWRITEONOFF:
		case VFDIOC_ICONDISPLAYONOFF:
		{
			copy_from_user(&vfddata, (void *)arg, sizeof(struct vfd_ioctl_data));
			break;
		}
	}
	switch (cmd)
	{
		case VFDIOC_DCRAMWRITE:
		{
//			copy_from_user(&vfddata, (void *)arg, sizeof(struct vfd_ioctl_data));
			return pt6302_write_dcram(vfd.ctrl, vfddata.address, vfddata.data, vfddata.length);
			break;
		}
		case VFDIOC_BRIGHTNESS:
		{
//			copy_from_user(&vfddata, (void *)arg, sizeof(struct vfd_ioctl_data));
			pt6302_set_brightness(vfd.ctrl, vfddata.address);
			break;
		}
		case VFDIOC_DISPLAYWRITEONOFF:
		{
//			copy_from_user(&vfddata, (void *)arg, sizeof(struct vfd_ioctl_data));
			pt6302_set_lights(vfd.ctrl, vfddata.address);
			break;
		}
		case VFDIOC_ICONDISPLAYONOFF:
		{
//			copy_from_user(&vfddata, (void *)arg, sizeof(struct vfd_ioctl_data));
			//pt_6958_set_icon(vfddata.address, vfddata.data, vfddata.length); // ikonki i inne pierdoly
			//pt_6958_set_icon(data ); // ikonki i inne pierdoly

			//dprintk(10, "[fp_led] VFDICONDISPLAYONOFF %x:%x", vfddata.data[0], vfddata.data[4]);

			if (vfddata.data[0] != 0x00000023)
			{
				dprintk(10,"VFDICONDISPLAYONOFF %x:%x\n", vfddata.data[0], vfddata.data[4]);
			}
			switch (vfddata.data[0])
			{
				// VFD driver of enigma2 issues codes during the start
				// phase, map them to the CD segments to indicate progress
				case VFD_LED_IR:
				{
					if (vfddata.data[4])
					{
						pt6958_led_control(PT6958_CMD_ADDR_LED1, led_POW | 0x01);
					}
					else
					{
						pt6958_led_control(PT6958_CMD_ADDR_LED1, led_POW);
					}
					break;
				}
				case VFD_LED_POW:
				{
					if (vfddata.data[4])
					{
						led_POW = 0x02;
					}
					else
					{
						led_POW = 0x00;
					}
					pt6958_led_control(PT6958_CMD_ADDR_LED1, led_POW);
					break;
				}
				case VFD_LED_REC:
				{
					printk("\n[fp_led] HDLED:%x\n", vfddata.data[4]);
					if (vfddata.data[4])
					{
						pt6958_led_control(PT6958_CMD_ADDR_LED2, 1); //nagrywanie
					}
					else
					{
						pt6958_led_control(PT6958_CMD_ADDR_LED2, 0);
					}
					break;
				}
#if 0
				case VFD_ICON_PAUSE:
				{
					if (vfddata.data[4])
					{
						pt6958_led_control(PT6958_CMD_ADDR_LED1, 3); //pause
					}
					else
					{
						pt6958_led_control(PT6958_CMD_ADDR_LED1, 2);
					}
					break;
				}
#endif
				case VFD_LED_LOCK:
				{
					if (vfddata.data[4])
					{
						pt6958_led_control(PT6958_CMD_ADDR_LED3, 1); //kanal kodowany
					}
					else
					{
						pt6958_led_control(PT6958_CMD_ADDR_LED3, 0);
					}
					break;
				}
				case VFD_LED_HD:
				{
					if (vfddata.data[4])
					{
						pt6958_led_control(PT6958_CMD_ADDR_LED4, 1); //kanal HD
					}
					else
					{
						pt6958_led_control(PT6958_CMD_ADDR_LED4, 0);
					}
					break;
				}
				default:
				{
					return 0;
				}
			}
			break;
		}
		case VFDIOC_DRIVERINIT:
		{
			pt6302_setup(vfd.ctrl);
			break;
		}
		default:
		{
			dprintk(1, "unknown IOCTL %08x\n", cmd);
			break;
		}
	}
	return 0;
}

/*****************************************************
 *
 * Write to /dev/vfd (display text)
 */
static ssize_t vfd_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
	unsigned char *kbuf;
	size_t        wlen;

	dprintk(10, "write: len = %d (%d), off = %d.\n", len, ADB_BOX_VFD_MAX_CHARS, (int)*off);

	if (len == 0)
	{
		return len;
	}
	kbuf = kmalloc(len, GFP_KERNEL);
	if (kbuf == NULL)
	{
		dprintk(1, "Unable to allocate kernel write buffer\n");
		return -ENOMEM;
	}
	copy_from_user(kbuf, buf, len);

	wlen = len;
	if (kbuf[len - 1] == '\n')  // remove trailing new line
	{
		kbuf[len - 1] = '\0';
		wlen--;
	}
// TODO: add scrolling?
	if (wlen > ADB_BOX_VFD_MAX_CHARS)  //display ADB_BOX_VFD_MAX_CHARS maximum
	{
		wlen = ADB_BOX_VFD_MAX_CHARS;
	}
	dprintk(10, "write : len = %d, wlen = %d, kbuf = '%s'.\n", len, wlen, kbuf);

	pt6302_write_dcram(vfd.ctrl, 0, kbuf, wlen);

#if 0
	if (wlen <= ADB_BOX_VFD_MAX_CHARS)
	{
		pt6302_write_dcram(vfd.ctrl, 0, kbuf, wlen);
	}
	else
	{
		pos = 0;
		pt6302_write_dcram(vfd.ctrl, 0, kbuf + pos, ADB_BOX_VFD_MAX_CHARS);
	}
#endif
	return len;
}

/*****************************************************
 *
 * Read from /dev/vfd
 */
static ssize_t vfd_read(struct file *filp, char *buf, size_t len, loff_t *off)
{
	// TODO: return current display string
	return len;
}

static int vfd_open(struct inode *inode, struct file *file)
{
	if (down_interruptible(&(vfd.sem)))
	{
		dprintk(10, "interrupted while waiting for semaphore\n");
		return -ERESTARTSYS;
	}
	if (vfd.opencount > 0)
	{
		dprintk(10, "device already opened\n");
		up(&(vfd.sem));
		return -EUSERS;
	}
	vfd.opencount++;
	up(&(vfd.sem));

	//pt6302_set_lights( vfd.ctrl, PT6302_LIGHTS_ON );
	//udelay(1000);
	pt6302_set_lights(vfd.ctrl, PT6302_LIGHTS_NORMAL);
	return 0;
}

static int vfd_close(struct inode *inode, struct file *file)
{
	//  pt6302_set_lights( vfd.ctrl, PT6302_LIGHTS_OFF );
	vfd.opencount = 0;
	return 0;
}

static struct file_operations vfd_fops =
{
	.owner   = THIS_MODULE,
	.ioctl   = vfd_ioctl,
	.write   = vfd_write,
	.read    = vfd_read,
	.open    = vfd_open,
	.release = vfd_close
};

//static void __exit vfd_module_exit(void)
static void vfd_module_exit(void)
{
	/* <-----buttons-----> */
	dprintk(0, "Button driver unloading ...");
	button_dev_exit();
	printk(" done.\n");

	/* <-----vfd-----> */
	dprintk(0, "PT6302 driver unloading.\n");
	unregister_chrdev(VFD_MAJOR, "vfd");
	if (vfd.ctrl)
	{
		pt6302_free(vfd.ctrl);
	}
	if (vfd.scp)
	{
		adb_box_scp_free(vfd.scp);
	}
	vfd.ctrl = NULL;
	vfd.scp  = NULL;
	printk(" done.\n");
}

/******************************************************
 *
 * vfd_module_init
 *
 * Initialize the driver.
 *
 */
static int __init vfd_module_init(void)
{
	/* <-----vfd-----> */
	dprintk(0, "Front panel PT6302 & PT6958 driver modified by B4Team & Audioniek\n");
	dprintk(0, "Driver init...");

#if 0
	if (rec != 1)  // give rec a default
	{
		rec = 0;
	}
#endif
	vfd.scp  = NULL;
	vfd.ctrl = NULL;

	dprintk(10, "Probe for scp driver.\n");
	vfd.scp = adb_box_scp_init();
	if (vfd.scp == NULL)
	{
		dprintk(1, "Unable to init scp driver; abort.");
		goto vfd_init_fail;
	}
	dprintk(10, "Probe for scp driver successful.\n");
	dprintk(10, "Probe for ctrl driver.\n");
	vfd.ctrl = pt6302_init(vfd.scp);
	if (vfd.ctrl == NULL)
	{
		dprintk(1, "Unable to init ctrl driver; abort.");
		goto vfd_init_fail;
	}
	dprintk(10, "Probe for ctrl driver successful.\n");
	dprintk(10, "Register character device %d.\n", VFD_MAJOR);
	if (register_chrdev(VFD_MAJOR, "vfd", &vfd_fops))
	{
		dprintk(1, "Registering major %d failed.\n", VFD_MAJOR);
		goto vfd_init_fail;
	}
	sema_init(&(vfd.sem), 1);
	vfd.opencount = 0;
	dprintk(10, "Registering character device %d successful.\n", VFD_MAJOR);

	pt6302_setup(vfd.ctrl);

	/* <-----buttons-----> */
	dprintk(0, "Initializing button driver...");
	if (button_dev_init() != 0)
	{
		dprintk(1, "Initializing button driver failed.\n");
		goto vfd_init_fail;
	}
	printk(" done.\n");

	pt6958_led_control(PT6958_CMD_ADDR_LED1, 2);
	return 0;

vfd_init_fail:
	vfd_module_exit();
	return -EIO;
}

module_init(vfd_module_init);
module_exit(vfd_module_exit);

module_param(paramDebug, int, 0644);
MODULE_PARM_DESC(debug, "paramDebug (default 0)");

module_param(delay, int, 0644);
MODULE_PARM_DESC(delay, "scp delay (default 5");

module_param(rec, int, 0644);
MODULE_PARM_DESC(rec, "adb_box rec (default 0)");

MODULE_DESCRIPTION("Front panel PT6302 & PT6958 driver");
MODULE_AUTHOR("B4Team & Audioniek");
MODULE_LICENSE("GPL");
// vim:ts=4
