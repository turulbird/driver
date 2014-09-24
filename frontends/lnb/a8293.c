/*
 *   a8293.c -
 *
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/types.h>
#include <linux/i2c.h>

#include <asm/io.h>

#include "lnb_core.h"

extern int _12v_isON; //defined in e2_proc ->I will implement a better mechanism later

extern short paramDebug;
#define TAGDEBUG "[A8293] "
#define dprintk(level, x...) do { \
	if ((paramDebug) && (paramDebug > level)) printk(TAGDEBUG x); \
} while (0)

unsigned char a8293_read(struct i2c_client *client)
{
	unsigned char byte;

	dprintk(10,"%s >\n", __func__);

	byte = 0;

	if (1 != i2c_master_recv(client,&byte,1))
	{
		return -1;
	}

	dprintk(10,"%s <\n", __func__);

	return byte;
}

int a8293_write(struct i2c_client *client, unsigned char reg)
{
	dprintk(10, "0x%02x %s >\n", reg, __func__);

	if ( 1 != i2c_master_send(client, &reg, 1))
	{
		printk("[A8293] %s: error sending data\n", __func__);
		return -EFAULT;
	}

	dprintk(10, "%s ok <\n", __func__);

	return 0;
}

int a8293_command_kernel(struct i2c_client *client, unsigned int cmd, void *arg)
{
	unsigned char reg = 0x10;

	dprintk(10,"%s (%x)\n", __func__, cmd);

	if (cmd != LNB_VOLTAGE_OFF)
	{
		reg |= (1<<5);
	}

	switch (cmd)
	{
		case LNB_VOLTAGE_OFF:
		{
			dprintk(10, "Switch LNB voltage off\n");

			if (_12v_isON == 0)
			{
				return a8293_write(client, reg);
			}
			else
			{
				return 0;
			}
		}
		case LNB_VOLTAGE_VER:
		{
			dprintk(10, "Set LNB voltage vertical (13V)\n");
			reg |= 0x04;
			return a8293_write(client, reg);
		}
		case LNB_VOLTAGE_HOR:
		{
			dprintk(10, "Set LNB voltage horizontal (18V)\n");
			reg |= 0x0B;
			return a8293_write(client, reg);
		}
	}
	return 0;
}

 
int a8293_command(struct i2c_client *client, unsigned int cmd, void *arg)
{
	return a8293_command_kernel(client, cmd, NULL);
}

int a8293_init(struct i2c_client *client)
{
	unsigned char reg;
	int res;

	/* this read is required, otherwise
	 * LNB power is not supplied!
	 */	
	reg = a8293_read(client);

	dprintk(10, "%s -> 0x%02X\n", __func__, reg);

	res = a8293_write(client, 0x82);

	if (res == 0)
	{
		/* setup pio6 */
		u32 reg = ctrl_inl(0xfd026030);

		dprintk(20, "%s: reg = 0x%08x\n", __func__, reg);

	reg |= 0x00000001;

	dprintk(20, "%s: reg = 0x%08x\n", __func__, reg);

	ctrl_outl(reg, 0xfd026030);

       reg = ctrl_inl(0xfd026000);

	dprintk(20, "%s: reg = 0x%08x\n", __func__, reg);

	reg &= ~(0x0000001);

	dprintk(20, "%s: reg = 0x%08x\n", __func__, reg);

	ctrl_outl(reg, 0xfd026000);
	}
	return res;
}
