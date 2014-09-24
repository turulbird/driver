/*
 *   lnb_core.c - lnb power control driver
 */

#include "lnb_core.h"

int paramDebug;
#define TAGDEBUG "[LNB] "

#define dprintk(level, x...) do { \
	if ((paramDebug) && (paramDebug > level)) printk(TAGDEBUG x); \
} while (0)

enum
{
	A8293,
	LNB24,
	LNB_PIO,
};

static const struct i2c_device_id lnb_id[] = {
	{ "a8293", A8293 },
	{ "lnb24", LNB24 },
	{ "pio", LNB_PIO },
	{ }
};
MODULE_DEVICE_TABLE(i2c, lnb_id);

static int devType;
static char *type = "a8293";
short debug = 0;

/*
 * Addresses to scan
 */
static unsigned short normal_i2c[] = {
	0x08, /* A8293 */
	I2C_CLIENT_END
};
I2C_CLIENT_INSMOD;

static struct i2c_driver lnb_i2c_driver;
static struct i2c_client *lnb_client = NULL;

/*
 * i2c probe
 */

static int lnb_newprobe(struct i2c_client *client, const struct i2c_device_id *id)
{
	if (lnb_client)
	{
		dprintk(10, "failure, client already registered\n");
		return -ENODEV;
	}

	printk("[LNB]: chip found @ 0x%x\n", client->addr);

	switch(devType)
	{
		case A8293:
		{
			a8293_init(client);
			break;
		}
		case LNB24:
		{
			lnb24_init(client);
			break;
		}
		case LNB_PIO:
		{
			lnb_pio_init();
			break;
		}
		default:
		{
			return -ENODEV;
		}
	}
	lnb_client = client;
	return 0;
}

static int lnb_remove(struct i2c_client *client)
{
	lnb_client = NULL;
	dprintk(10, "remove\n");
	return 0;
}

/*
 * devfs fops
 */

static int lnb_command_ioctl(struct i2c_client *client, unsigned int cmd, void *arg)
{
	int err = 0;

	dprintk(10, "%s (%x)\n", __func__, cmd);

	if (!client && (devType != LNB_PIO))
	{
		return -1;
	}

	switch(devType)
	{
		case A8293:
		{
			err = a8293_command(client, cmd, arg);
		   	break;
		}
		case LNB24:
		{
			err = lnb24_command(client, cmd, arg);
			break;
		}
		case LNB_PIO:
		{
			err = lnb_pio_command(cmd, arg);
			break;
		}
	}
	return err;
}

int lnb_command_kernel(unsigned int cmd, void *arg)
{
	int err = 0;
	struct i2c_client *client = lnb_client;

	dprintk(10, "[LNB]: %s (%x)\n", __func__, cmd);

	if (!client && (devType != LNB_PIO))
	{
		return -1;
	}

	switch(devType)
	{
		case A8293:
		{
			err = a8293_command_kernel(client, cmd, arg);
			break;
		}
		case LNB24:
		{
			err = lnb24_command_kernel(client, cmd, arg);
			break;
		}
		case LNB_PIO:
		{
			err = lnb_pio_command_kernel(cmd, arg);
			break;
		}
	}
	return err;
}

EXPORT_SYMBOL(lnb_command_kernel); // export to use in other drivers

static int lnb_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	dprintk(5, "IOCTL\n");
	return lnb_command_ioctl(lnb_client, cmd, (void *) arg);
}

static int lnb_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int lnb_close (struct inode *inode, struct file *filp)
{
	return 0;
}

static struct file_operations lnb_fops = {
	.owner 		= THIS_MODULE,
	.ioctl 		= lnb_ioctl,
	.open  		= lnb_open,
	.release  	= lnb_close
};

static int lnb_detect(struct i2c_client *client, int kind, struct i2c_board_info *info)
{
	const char *name = "";
	if (kind == 0)
	{
		name = "a8293";
		kind = A8293;
	}

	if (kind < 0)
	{
		/* detection and identification */
		if (!type || !strcmp("a8293", type))
		{
			kind = A8293;
		}
		else if (!strcmp("lnb24", type))
		{
			kind = LNB24;
		}
		else if(!strcmp("pio", type))
		{
			kind = LNB_PIO;
		}
		else
		{
			return -ENODEV;
		}
	}
	switch (kind)
	{
		case A8293:
		{
			name = "a8293";
			break;
		}
		case LNB24:
		{
			name = "lnb24";
			break;
		}
		case LNB_PIO:
		{
			name = "lnp-pio";
			break;
		}
		default:
		{
			return -ENODEV;
		}
	}
	devType = kind;
	strlcpy(info->type, name, I2C_NAME_SIZE);
	return 0;
}

/*
 * i2c
 */

static struct i2c_driver lnb_i2c_driver = {
	.class = I2C_CLASS_TV_DIGITAL,
	.driver = {
		.owner = THIS_MODULE,
		.name  = "lnb_driver", /* 2.6.30 requires name without spaces */
	},
	.probe        = lnb_newprobe,
	.detect       = lnb_detect,
	.remove       = lnb_remove,
	.address_data = &addr_data,
	.command      = lnb_command_ioctl
};

/*
 * module init/exit
 */

int __init lnb_init(void)
{
	int res, err;
	const char *name;

	struct i2c_board_info info;
	err = lnb_detect(NULL, -1, &info);
	name = info.type;

	if (err)
	{
		printk("[LNB]: Error detecting LNB type!\n");
		return err;
	}

	dprintk(10, "LNB power control handling, type: %s\n", name);

	if (devType == LNB_PIO)
	{
		if ((res = lnb_pio_init()))
		{
			printk("[LNB] init lnb pio failed\n");
			return res;
		}
	}
	else if ((res = i2c_add_driver(&lnb_i2c_driver)))
	{
		printk("[LNB] i2c add driver failed\n");
		return res;
	
		if (!lnb_client)
		{
			printk(KERN_ERR "[LNB] no client found\n");
			i2c_del_driver(&lnb_i2c_driver);
			return -EIO;
		}
	}

	if (register_chrdev(LNB_MAJOR,"LNB",&lnb_fops)<0)
	{
		printk(KERN_ERR "[LNB] unable to register device\n");
		if (devType == LNB_PIO)
		{
			lnb_pio_exit();
		}
		else
		{
			i2c_del_driver(&lnb_i2c_driver);
		}
		return -EIO;
	}
	return 0;
}

void __exit lnb_exit(void)
{
	unregister_chrdev(LNB_MAJOR,"LNB");
	i2c_del_driver(&lnb_i2c_driver);
}

module_init(lnb_init);
module_exit(lnb_exit);

module_param(type,charp,0);
MODULE_PARM_DESC(type, "device type (a8293, lnb24, pio)");

module_param(paramDebug, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(paramDebug, "Debug Output 0=disabled >0=enabled(debuglevel)");

MODULE_AUTHOR("Spider-Team");
MODULE_DESCRIPTION("Multiplatform LNB power control driver");
MODULE_LICENSE("GPL");

