/*
 * hchs8100_fp_procfs.c
 *
 * (c) 20?? Gustav Gans
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
 * procfs for Homecast HS8100 series front panel driver.
 *
 *
 ****************************************************************************
 *
 * Changes
 *
 * Date     By              Description
 * --------------------------------------------------------------------------
 * 20220908 Audioniek       Initial version based on cuberevo_micom_procfs.c.
 * 
 ****************************************************************************/

#include <asm/io.h>
#include <linux/proc_fs.h>      /* proc fs */
#include <asm/uaccess.h>        /* copy_from_user */
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17)
#include <linux/stm/pio.h>
#else
#include <linux/stpio.h>
#endif

#include "hchs8100_fp.h"

/*
 *  /proc/------
 *             |
 *             +--- progress (rw)           Progress of E2 startup in %
 *
 *  /proc/stb/fan/
 *             |
 *             +--- fan_ctrl (rw)           Control of fan speed (deprecated, use /proc/stb/fp/fan_pwm)
 *
 *  /proc/stb/fp/
 *             |
 *             +--- fan (rw)                Control of fan (on/off)
 *             +--- fan_pwm (rw)            Control of fan speed
 *             +--- oled_brightness (w)     Direct control of display brightness
 *             +--- rtc (rw)                RTC time (UTC, seconds since Unix epoch)
 *             +--- rtc_offset (rw)         RTC offset in seconds from UTC
 *             +--- wakeup_time (rw)        Next wakeup time (absolute, UTC, seconds since Unix epoch)
 *             +--- was_timer_wakeup (r)    Reports timer wakup or not
 *             +--- symbol_circle (rw)      Control of spinner icon
 *             +--- text (w)                Direct writing of display text
 *
 *  /proc/stb/info/
 *             |
 *             +--- OEM (r)                 Name of OEM
 *             +--- brand (r)               Brand name
 *             +--- model_name (r)          Model name
 *
 *  /proc/stb/lcd/
 *             |
 *             +--- symbol_circle (rw)      Control of spinner
 */

/* from e2procfs */
extern int install_e2_procs(char *name, read_proc_t *read_proc, write_proc_t *write_proc, void *data);
extern int remove_e2_procs(char *name, read_proc_t *read_proc, write_proc_t *write_proc);

/* from other modules */
extern int pt6302_write_dcram(unsigned char addr, unsigned char *data, unsigned char len);
extern void pt6302_set_brightness(int level);
extern void clear_display(void);
extern void calcGetRTCTime(time_t theTime, char *destString);
extern time_t calcSetRTCTime(char *time);
extern int hchs8100_SetWakeUpTime(char *wtime);
extern void hchs8100_GetWakeUpTime(char *time);

/* Globals */
static int progress = 0;
static int symbol_circle = 0;
static int fan_onoff = 1;
static int fan_speed = 0;
extern struct stpio_pin *fan_pin;
extern tSpinnerState spinner_state;

static int progress_write(struct file *file, const char __user *buf, unsigned long count, void *data)
{
	char* page;
	char* myString;
	ssize_t ret = -ENOMEM;

	page = (char*)__get_free_page(GFP_KERNEL);

	if (page)
	{
		ret = -EFAULT;
		if (copy_from_user(page, buf, count))
		{
			goto out;
		}
		myString = (char*) kmalloc(count + 1, GFP_KERNEL);
		strncpy(myString, page, count);
		myString[count - 1] = '\0';

		sscanf(myString, "%d", &progress);
		kfree(myString);

		/* always return count to avoid endless loop */
		ret = count;
	}
out:
	free_page((unsigned long)page);
	return ret;
}

static int progress_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	if (NULL != page)
	{
		len = sprintf(page,"%d", progress);
	}
	return len;
}	

static int symbol_circle_write(struct file *file, const char __user *buf, unsigned long count, void *data)
{
	char* page;
	ssize_t ret = -ENOMEM;
	char* myString;
	int i = 0;

	page = (char*)__get_free_page(GFP_KERNEL);

	if (page)
	{
		ret = -EFAULT;
		if (copy_from_user(page, buf, count))
		{
			goto out;
		}
		myString = (char*) kmalloc(count + 1, GFP_KERNEL);
		strncpy(myString, page, count);
		myString[count - 1] = '\0';

		sscanf(myString, "%d", &symbol_circle);
		kfree(myString);

		if (symbol_circle != 0)
		{  // spinner on
			if (symbol_circle > 255)
			{
				symbol_circle = 255;  // set maximum value
			}
			if (spinner_state.state == 0)  // if spinner not active
			{
				if (symbol_circle == 1)  // handle special value 1
				{
					spinner_state.period = 250;  // set standard speed
				}
				else
				{
					spinner_state.period = symbol_circle * 10;  // set user specified speed
				}
				spinner_state.state = 1;
				lastdata.icon_state[ICON_SPINNER] = 1;
				up(&spinner_state.sem);
			}
		}
		else
		{  // spinner off
			if (spinner_state.state != 0)
			{
				spinner_state.state = 0;
				lastdata.icon_state[ICON_SPINNER] = 0;
//				dprintk(50, "%s Stop spinner thread\n", __func__);
				i = 0;
				do
				{
					msleep(250);
					i++;
				}
				while (spinner_state.status != THREAD_STATUS_HALTED && i < 20);  //time out of 20 seconds
//				dprintk(50, "%s Spinner thread stopped\n", __func__);
			}
		}
	}
	else
	{
		symbol_circle = 0;
	}
	/* always return count to avoid endless loop */
	ret = count;
	goto out;

out:
	free_page((unsigned long)page);
	return ret;
}

static int symbol_circle_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	if (NULL != page)
	{
		len = sprintf(page,"%d", symbol_circle);
	}
	return len;
}

static int text_write(struct file *file, const char __user *buf, unsigned long count, void *data)
{
	char *page;
	int ret = -ENOMEM;

	page = (char *)__get_free_page(GFP_KERNEL);

	if (page)
	{
		ret = -EFAULT;
		if (copy_from_user(page, buf, count) == 0)
		{
			ret = pt6302_write_dcram(0, page, count);
			if (ret >= 0)
			{
				ret = count;
			}
		}
		free_page((unsigned long)page);
	}
	return ret;
}

static int read_rtc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	time_t uTime;

	uTime = read_rtc_time();

	if (NULL != page)
	{
		len = sprintf(page,"%u\n", (int)(uTime - rtc_offset));  // return UTC
	}
	return len;
}

static int write_rtc(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	char *page = NULL;
	ssize_t ret = -ENOMEM;
	int test = -1;
	time_t argument = 0;
	char *myString = kmalloc(count + 1, GFP_KERNEL);

	dprintk(50, "%s >\n", __func__);
	page = (char *)__get_free_page(GFP_KERNEL);

	if (page)
	{
		ret = -EFAULT;

		if (copy_from_user(page, buffer, count))
		{
			goto out;
		}
		strncpy(myString, page, count);
		myString[count] = '\0';

		test = sscanf(myString, "%u", (unsigned int *)&argument);

		if (test > 0)
		{
			ret = write_rtc_time(argument + rtc_offset);
		}
		/* always return count to avoid endless loop */
		ret = count;
	}
out:
	free_page((unsigned long)page);
	kfree(myString);
	return ret;
}

static int read_rtc_offset(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	if (NULL != page)
	{
		len = sprintf(page, "%d\n", rtc_offset);
	}
	return len;
}

static int write_rtc_offset(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	char *page = NULL;
	ssize_t ret = -ENOMEM;
	int test = -1;
	char *myString = kmalloc(count + 1, GFP_KERNEL);

	page = (char *)__get_free_page(GFP_KERNEL);

	if (page)
	{
		ret = -EFAULT;
		if (copy_from_user(page, buffer, count))
		{
			goto out;
		}
		strncpy(myString, page, count);
		myString[count] = '\0';

		test = sscanf (myString, "%d", &rtc_offset);

		/* always return count to avoid endless loop */
		ret = count;
	}
out:
	free_page((unsigned long)page);
	kfree(myString);
	return ret;
}

static int wakeup_time_write(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	char *page = NULL;
	ssize_t ret = -ENOMEM;
	time_t wakeup_time = 0;
	int test = -1;
	char *myString = kmalloc(count + 1, GFP_KERNEL);
	char wtime[5];

	memset(wtime, 0, sizeof(wtime));
	page = (char *)__get_free_page(GFP_KERNEL);

	if (page)
	{
		ret = -EFAULT;

		if (copy_from_user(page, buffer, count))
		{
			goto out;
		}
		strncpy(myString, page, count);
		myString[count] = '\0';

		test = sscanf(myString, "%u", (unsigned int *)&wakeup_time);

		if (0 < test)
		{
			calcGetRTCTime(wakeup_time, wtime);  // set FP wake up time (local)
			ret = hchs8100_SetWakeUpTime(wtime);
		}
		/* always return count to avoid endless loop */
		ret = count;
	}
out:
	free_page((unsigned long)page);
	kfree(myString);
//	dprintk(10, "%s <\n", __func__);
	return ret;
}

static int wakeup_time_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char wtime[5];  // MJD hms
	int	w_time;

	if (NULL != page)
	{
		
		hchs8100_GetWakeUpTime(wtime);  // get wakeup time from FP clock (UTC)
		w_time = calcSetRTCTime(wtime);

		len = sprintf(page, "%u\n", w_time);
	}
	return len;
}

static int was_timer_wakeup_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	int res = 0;
	int wdata;

	dprintk(50, "%s >\n", __func__);
	if (NULL != page)
	{
		if (wakeup_mode == WAKEUP_TIMER) // if timer wakeup
		{
			wdata = 1;
		}
		else
		{
			wdata = 0;
		}
	}
	else
	{
			wdata = -1;
	}
	len = sprintf(page,"%d\n", wdata);
	return len;
}

static int oled_brightness_write(struct file *file, const char __user *buf, unsigned long count, void *data)
{
	char *page;
	long level;
	int  ret = -ENOMEM;

	page = (char *)__get_free_page(GFP_KERNEL);

	if (page)
	{
		ret = -EFAULT;

		if (copy_from_user(page, buf, count) == 0)
		{
			page[count - 1] = '\0';

			level = simple_strtol(page, NULL, 10);
			pt6302_set_brightness((int)level);
			ret = count;
		}
		free_page((unsigned long)page);
	}
	return ret;
}

static int fan_write(struct file *file, const char __user *buf, unsigned long count, void *data)
{
	char *page;
	int ret = -ENOMEM;
	unsigned int value;

	page = (char *)__get_free_page(GFP_KERNEL);

	if (page)
	{
		ret = -EFAULT;
		if (copy_from_user(page, buf, count) == 0)
		{
			page[count - 1] = '\0';
			fan_onoff = (unsigned int)simple_strtol(page, NULL, 10);

			if (fan_onoff != 0)
			{
				fan_onoff = 1;
			}
			ret = hchs8100_set_fan(fan_onoff, fan_speed);
			ret = count;
		}
		free_page((unsigned long)page);
	}
	return ret;
}

static int fan_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	if (NULL != page)
	{
		len = sprintf(page,"%d", fan_onoff);
	}
	return len;
}

static int fan_ctrl_write(struct file *file, const char __user *buf, unsigned long count, void *data)
{
	char *page;
	int ret = -ENOMEM;
	unsigned int value;

	page = (char *)__get_free_page(GFP_KERNEL);

	if (page)
	{
		ret = -EFAULT;
		if (copy_from_user(page, buf, count) == 0)
		{
			page[count - 1] = '\0';
			fan_onoff = (unsigned int)simple_strtol(page, NULL, 10);

			if (fan_onoff != 0)
			{
				fan_onoff = 1;
			}
			ret = hchs8100_set_fan(fan_onoff, fan_speed);
			ret = count;
		}
		free_page((unsigned long)page);
	}
	return ret;
}

static int fan_ctrl_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	if (NULL != page)
	{
		len = sprintf(page,"%d", fan_onoff);
	}
	return len;
}

int fan_speed_write(struct file *file, const char __user *buf, unsigned long count, void *data)
{
	char *page;
	char *myString;
	int  ret = -ENOMEM;
	unsigned int value;
	
	page = (char *)__get_free_page(GFP_KERNEL);

	if (page) 
	{
		ret = -EFAULT;
		if (copy_from_user(page, buf, count) == 0)
		{
			page[count - 1] = '\0';
			ret = hchs8100_set_fan(fan_onoff, (simple_strtol(page, NULL, 10) < 128 ? 0 : 1));
			ret = count;
		}
		free_page((unsigned long)page);
	}
	return ret;
}

int fan_speed_read(char *page, char **start, off_t off, int count, int *eof, void *data_unused)
{
	int      len = 0;
	
	dprintk(10, "%s >\n", __func__);

	if (NULL != page)
	{
		len = sprintf(page, "%d\n", fan_speed);
	}
	return len;
}

static int oem_name_read(char *page, char **start, off_t off, int count, int *eof, void *data_unused)
{
	int len = 0;

	if (NULL != page)
	{
		len = sprintf(page, "Homecast\n");
	}
	return len;
}

static int brand_name_read(char *page, char **start, off_t off, int count, int *eof, void *data_unused)
{
	int len = 0;
	
	if (NULL != page)
	{
		len = sprintf(page, "%s\n", "Homecast");
	}
	return len;
}

static int model_name_read(char *page, char **start, off_t off, int count, int *eof, void *data_unused)
{
	int len = 0;

	if (NULL != page)
	{
		len = sprintf(page, "%s\n", "HS8100 CI series");
	}
	dprintk(50, "%s < %d\n", __func__, len);
	return len;
}

/*
static int null_write(struct file *file, const char __user *buf, unsigned long count, void *data)
{
	return count;
}
*/

struct fp_procs
{
	char *name;
	read_proc_t *read_proc;
	write_proc_t *write_proc;
} fp_procs[] =
{
	{ "progress", progress_read, progress_write },
	{ "stb/fan/fan_ctrl", fan_ctrl_read, fan_ctrl_write },
	{ "stb/fp/fan", fan_read, fan_write },
	{ "stb/fp/fan_pwm", fan_speed_read, fan_speed_write },
	{ "stb/fp/rtc", read_rtc, write_rtc },
	{ "stb/fp/rtc_offset", read_rtc_offset, write_rtc_offset },
	{ "stb/fp/oled_brightness", NULL, oled_brightness_write },
	{ "stb/fp/text", NULL, text_write },
	{ "stb/lcd/symbol_circle", symbol_circle_read, symbol_circle_write },
	{ "stb/fp/wakeup_time", wakeup_time_read, wakeup_time_write },
	{ "stb/fp/was_timer_wakeup", was_timer_wakeup_read, NULL },
	{ "stb/info/OEM", oem_name_read, NULL },
	{ "stb/info/brand", brand_name_read, NULL },
	{ "stb/info/model_name", model_name_read, NULL },
};

void create_proc_fp(void)
{
	int i;

	for (i = 0; i < sizeof(fp_procs) / sizeof(fp_procs[0]); i++)
	{
		install_e2_procs(fp_procs[i].name, fp_procs[i].read_proc, fp_procs[i].write_proc, NULL);
	}
}

void remove_proc_fp(void)
{
	int i;

	for (i = sizeof(fp_procs) / sizeof(fp_procs[0]) - 1; i >= 0; i--)
	{
		remove_e2_procs(fp_procs[i].name, fp_procs[i].read_proc, fp_procs[i].write_proc);
	}
}
// vim:ts=4
