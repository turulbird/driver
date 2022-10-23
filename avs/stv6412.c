/*
 *   stv6412.c - audio/video switch driver (dbox-II-project)
 *
 *   Homepage: http://www.tuxbox.org
 *
 *   Copyright (C) 2000-2002 Gillem gillem@berlios.de
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Revision 1.26.
 *
 *   The code has been adapted to TF7700.
 *
 */

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

#include "avs_core.h"
#include "stv6412.h"

/* ---------------------------------------------------------------------- */

static int debug = AVS_DEBUG;

/*
 * stv6412 data struct
 *
 */
#if 1 
//defined(__LITTLE_ENDIAN__)
typedef struct s_stv6412_data
{
	/* Data 0 */
	unsigned char svm       : 1;
	unsigned char t_vol_c   : 5;
	unsigned char t_vol_x   : 1;
	unsigned char t_stereo  : 1;
	/* Data 1 */
	unsigned char tc_asc    : 3;  // TV SCART & CINC audio source selection
	unsigned char c_ag      : 1;  // cinch audio gain
	unsigned char v_asc     : 2;  // VCR SCART audio selection
	unsigned char res1      : 1;
	unsigned char v_stereo  : 1;  // VCR audio mode
	/* Data 2 */
	unsigned char t_vsc     : 3;  // TV SCART video source
	unsigned char t_cm      : 1;  // TV SCART chroma mute
	unsigned char v_vsc     : 3;  // VCR SCART video source
	unsigned char v_cm      : 1;  // VCR SCART chroma mute
	/* Data 3 */
	unsigned char fblk      : 2;  // TV SCART fast blanking value
	unsigned char rgb_vsc   : 2;  // TV SCART RGB video source
	unsigned char rgb_gain  : 3;  // TV SCART RGB gain
	unsigned char rgb_tri   : 1;  // TV SCART FB and RGB tri-state control
	/* Data 4 */
	unsigned char t_rcos    : 1;  // selects Red of Chroma on R output
	unsigned char r_ac      : 1;  // Addr control bit 0, selects CVBS or Y/C to RF output
	unsigned char r_tfc     : 1;  // Addr control bit 1, selects chroma filter on/off
	unsigned char v_cgc     : 1;  // Cgate output control
	unsigned char v_coc     : 1;  // C VCR control
	unsigned char res2      : 1;  // bit is don't care
	unsigned char slb       : 1;  // slow blanking source: normal or VCR
	unsigned char it_enable : 1;
	/* Data 5 */
	unsigned char e_rcsc    : 1;  // Clamping of SoC video
	unsigned char v_rcsc    : 1;  // Clamping of VCR video
	unsigned char e_aig     : 2;  // SoC audio input level
	unsigned char t_sb      : 2;  // TV SCART status
	unsigned char v_sb      : 2;  // VCR SCART status
	/* Data 6 */
	unsigned char e_in      : 1;
	unsigned char v_in      : 1;
	unsigned char t_in      : 1;
	unsigned char a_in      : 1;
	unsigned char v_out     : 1;
	unsigned char c_out     : 1;
	unsigned char t_out     : 1;
	unsigned char r_out     : 1;
} s_stv6412_data;
#else
typedef struct s_stv6412_data
{
 /* Data 0 */
	unsigned char t_stereo  : 1;
	unsigned char t_vol_x   : 1;
	unsigned char t_vol_c   : 5;
	unsigned char svm       : 1;
 /* Data 1 */
	unsigned char v_stereo  : 1;
	unsigned char res1      : 1;
	unsigned char v_asc     : 2;
	unsigned char c_ag      : 1;
	unsigned char tc_asc    : 3;
 /* Data 2 */
	unsigned char v_cm      : 1;
	unsigned char v_vsc     : 3;
	unsigned char t_cm      : 1;
	unsigned char t_vsc     : 3;
 /* Data 3 */
	unsigned char rgb_tri   : 1;
	unsigned char rgb_gain  : 3;
	unsigned char rgb_vsc   : 2;
	unsigned char fblk      : 2;
 /* Data 4 */
	unsigned char it_enable : 1;
	unsigned char slb       : 1;
	unsigned char res2      : 1;
	unsigned char v_coc     : 1;
	unsigned char v_cgc     : 1;
	unsigned char r_tfc     : 1;
	unsigned char r_ac      : 1;
	unsigned char t_rcos    : 1;
 /* Data 5 */
	unsigned char v_sb      : 2;
	unsigned char t_sb      : 2;
	unsigned char e_aig     : 2;
	unsigned char v_rcsc    : 1;
	unsigned char e_rcsc    : 1;
 /* Data 6 */
	unsigned char r_out     : 1;
	unsigned char t_out     : 1;
	unsigned char c_out     : 1;
	unsigned char v_out     : 1;
	unsigned char a_in      : 1;
	unsigned char t_in      : 1;
	unsigned char v_in      : 1;
	unsigned char e_in      : 1;
} s_stv6412_data;
#endif

#define STV6412_DATA_SIZE sizeof(s_stv6412_data)


/* hold old values for mute/unmute */
unsigned char tc_asc;
unsigned char v_asc;

/* hold old values for standby */
unsigned char t_stnby = 0;

/* ---------------------------------------------------------------------- */

static struct s_stv6412_data stv6412_data;
static struct s_stv6412_data tmpstv6412_data;
static int stv6412_s_old_src;

/***************************************************
 *
 * Send all current register values to the STV6412.
 *
/***************************************************/
#if defined(FS9000) \
 || defined(HL101) \
 || defined(VIP1_V1) \
 || defined(IPBOX9900)
//Trick: hack ;)
int stv6412_set(struct i2c_client *client)
{
	char buffer[11];

	printk("[AVS] [STV6412] set!\n");

	buffer[0]  = 0x00;
	buffer[1]  = 0x40;
	buffer[2]  = 0x09;
	buffer[3]  = 0x11;
	buffer[4]  = 0x84;
	buffer[5]  = 0x84;
	buffer[6]  = 0x25;
	buffer[7]  = 0x08;
	buffer[8]  = 0x21;
	buffer[9]  = 0xc0;
	buffer[10] = 0x00;
	i2c_master_send(client, buffer, 10); //not 11?
	return 0;
}
#else
int stv6412_set(struct i2c_client *client)
{
	char buffer[STV6412_DATA_SIZE + 1];

	buffer[0] = 0;

	memcpy(buffer + 1, &stv6412_data, STV6412_DATA_SIZE);

	if ((STV6412_DATA_SIZE + 1) != i2c_master_send(client, buffer, STV6412_DATA_SIZE + 1))
	{
		return -EFAULT;
	}
	return 0;
}
#endif

/***************************************************
 *
 * Set volume.
 *
 ***************************************************/
int stv6412_set_volume(struct i2c_client *client, int vol)
{
	int c = 0;

	c = vol;

#if !defined(ADB_BOX)
	if (c == 63)
	{
		c = 62;
	}
	if (c == 0)
	{
		c = 1;
	}
	if ((c > 63) || (c < 0))
	{
		return -EINVAL;
	}
	c /= 2;
	stv6412_data.t_vol_c = c;
#endif
	return stv6412_set(client);
}

/***************************************************
 *
 * Set mute state.
 *
 * sw = output
 *      0 = VCR SCART
 *      1 = TV SCART RGB
 *      2 = TV SCART CVBS or Y/C
 *
 * type: 0 = off, 1 on
 *
/***************************************************/
inline int stv6412_set_mute(struct i2c_client *client, int type)
{
	if ((type < AVS_UNMUTE) || (type > AVS_MUTE))
	{
		return -EINVAL;
	}

	if (type == AVS_MUTE) 
	{
		if (tc_asc == 0xff)
		{
			/* save old values */
			tc_asc = stv6412_data.tc_asc;
			v_asc  = stv6412_data.v_asc;

			/* set mute */
			stv6412_data.tc_asc = 0;  // tv & cinch mute
			stv6412_data.v_asc  = 0;  // vcr mute
		}
	}
	else /* unmute with old values */
	{
		if (tc_asc != 0xff)
		{
			stv6412_data.tc_asc = tc_asc;
			stv6412_data.v_asc  = v_asc;

			tc_asc = 0xff;
			v_asc  = 0xff;
		}
	}
	return stv6412_set(client);
}

/***************************************************
 *
 * Set video output format.
 *
 * sw = output
 *      0 = VCR SCART
 *      1 = TV SCART RGB
 *      2 = TV SCART CVBS or Y/C
 *
 * type = video format
 *        0 = muted (no output)
 *        1 = SoC CVBS
 *        2 = SoC Y/C
 *        3 = CVBS (of opposite SCART)
 *
/***************************************************/
inline int stv6412_set_vsw( struct i2c_client *client, int sw, int type)
{
	printk("[STV6412] Set VSW: %d %d\n", sw, type);

	if (type < 0 || type > 4)
	{
		return -EINVAL;
	}

	switch (sw)  // get output to set: VCR CVBS, TV RGB, TV CVBS
	{
		case 0:	 // vcr
		{
			stv6412_data.v_vsc = type;  // 
			break;
		}
		case 1:	 // rgb
		{
			if (type < 0 || type > 2)
			{
				return -EINVAL;
			}
			stv6412_data.rgb_vsc = type;  // 0 = TV RGB, 1 VCR RGB
			break;
		}
		case 2:  // tv
		{
			stv6412_data.t_vsc = type;  // 0 = no output, 1 = SoC CVBS, 2 = SoC Y/C, 3 = VCR Y/CVBS & VCR C
			break;
		}
		default:
		{
			return -EINVAL;
		}
	}
	return stv6412_set(client);
}

/***************************************************
 *
 * Set audio output source.
 *
 * sw = output
 *      0 = VCR SCART
 *      1 = TV SCART RGB
 *      2 = TV SCART CVBS or Y/C
 *
 * type = audio format
 *        0 = muted (no output)
 *        1 = SoC audio
 *        2 = audio of opposite SCART
 *        3 = AUX
 *        4 = TV ?
 *
/***************************************************/
inline int stv6412_set_asw( struct i2c_client *client, int sw, int type )
{
	switch(sw)
	{
		case 0:  // VCR SCART
		{
			if (type <= 0 || type > 3)
			{
				return -EINVAL;
			}

			/* if muted ? yes: save in temp */
			if ( v_asc == 0xff )
			{
				stv6412_data.v_asc = type;  
			}
			else
			{
				v_asc = type;  // keep for unmute
			}
			break;
		}
		case 1:  // TV SCART RGB
		case 2:  // TV SCART CVBS
		{
			if (type <= 0 || type > 4)
			{
				return -EINVAL;
			}

			/* if muted ? yes: save in temp */
			if ( tc_asc == 0xff )
			{
				stv6412_data.tc_asc = type;
			}
			else
			{
				tc_asc = type;
			}
			break;
		}
		default:
		{
			return -EINVAL;
		}
	}
	return stv6412_set(client);
}

/***************************************************
 *
 * Set TV SCART status pin 8.
 *
 * type = status
 *        0 = pin floats (TV does not select SCART input)
 *        1 = pin < 2V (TV does not select SCART input)
 *        2 = Select SCART input, 16:9 mode
 *        3 = Select SCART input, 4:3 mode
 *
/***************************************************/
inline int stv6412_set_t_sb(struct i2c_client *client, int type)
{
	if (type < 0 || type > 3)
	{
		return -EINVAL;
	}
	stv6412_data.t_sb = type;
	return stv6412_set(client);
}

/***************************************************
 *
 * Set WSS on TV SCART.
 *
 * type = aspect ratio
 *        0 = 4:3
 *        1 = 16:9
 *        2 = off
 *
 * Note: does not actually set WSS but sets status
 *       pin of TV SCART to matching aspect ratio.
 *
/***************************************************/
inline int stv6412_set_wss(struct i2c_client *client, int vol)
{
	if (vol == SAA_WSS_43F)
	{
		stv6412_data.t_sb = 3;
	}
	else if (vol == SAA_WSS_169F)
	{
		stv6412_data.t_sb = 2;
	}
	else if (vol == SAA_WSS_OFF)
	{
		stv6412_data.t_sb = 0; //1;
	}
	else
	{
		return  -EINVAL;
	}
	return stv6412_set(client);
}

/***************************************************
 *
 * Set TV SCART fast blanking pin 16.
 *
 * type = status
 *        0 = low level
 *        1 = high level (RGB mode)
 *        2 = from SoC
 *        3 = from VCR SCART fast blanking pin
 *
 * Note: does not actually set WSS but sets status
 *       pin of TV SCART to matching aspect ratio.
 *
/***************************************************/
inline int stv6412_set_fblk(struct i2c_client *client, int type)
{
	if (type < 0 || type > 3)
	{
		return -EINVAL;
	}
	stv6412_data.fblk = type;
	return stv6412_set(client);
}

/***************************************************
 *
 * Get AVS status.
 *
 * Merely reads the status byte and returns it
 * without further processing.
 *
/***************************************************/
int stv6412_get_status(struct i2c_client *client)
{
	unsigned char byte;

	byte = 0;

	if (1 != i2c_master_recv(client, &byte, 1))
	{
		return -1;
	}
	return byte;
}

/***************************************************
 *
 * Get volume.
 *
 *
/***************************************************/
int stv6412_get_volume(void)
{
	int c;

	c = stv6412_data.t_vol_c;  // get current volume value

	if (c)
	{
		c *= 2; // times 2
	}
	return c;
}

/***************************************************
 *
 * Get mute status. Returns zero if both TV SCART
 * and VCR SCART are not muted.
 *
/***************************************************/
inline int stv6412_get_mute(void)
{
	return !((tc_asc == 0xff) && (v_asc == 0xff));
}

/***************************************************
 *
 * Get fast blanking status.
 *
/***************************************************/
inline int stv6412_get_fblk(void)
{
	return stv6412_data.fblk;
}

/***************************************************
 *
 * Get TV SCART status.
 *
/***************************************************/
inline int stv6412_get_t_sb(void)
{
	return stv6412_data.t_sb;
}

/***************************************************
 *
 * Get video source.
 *
 * sw = output
 *      0 = VCR SCART CVBS
 *      1 = TV SCART RGB
 *      2 = TV SCART CVBS
 *
/***************************************************/
inline int stv6412_get_vsw(int sw)
{
	switch (sw)
	{
		case 0:
		{
			return stv6412_data.v_vsc;
			break;
		}
		case 1:
		{
			return stv6412_data.rgb_vsc;
			break;
		}
		case 2:
		{
			return stv6412_data.t_vsc;
			break;
		}
		default:
		{
			return -EINVAL;
		}
	}
	return -EINVAL;
}

/***************************************************
 *
 * Get audio source.
 *
 * sw = output
 *      0 = VCR SCART CVBS
 *      1 = TV SCART RGB
 *      2 = TV SCART CVBS
 *
/***************************************************/
inline int stv6412_get_asw( int sw )
{
	switch (sw)
	{
		case 0:  // VCR
		{
			// muted ? yes: return tmp values
			if (v_asc == 0xff)
			{
				return stv6412_data.v_asc;
			}
			else
			{
				return v_asc;
			}
		}
		case 1:  // TV RGB
		case 2:  // TV CVBS
		{
			if (tc_asc == 0xff)
			{
				return stv6412_data.tc_asc;
			}
			else
			{
				return tc_asc;
			}
			break;
		}
		default:
		{
			return -EINVAL;
		}
	}
	return -EINVAL;
}

/***************************************************
 *
 * Set encoder mode.
 *
 * NOT IMPLEMENTED
 *
/***************************************************/
//NOT IMPLEMENTED
int stv6412_set_encoder(struct i2c_client *client, int vol)
{
	return 0;
}

/***************************************************
 *
 * Set TV SCART output video format.
 *
 * vol = format
 *       0 (SAA_MODE_RGB) = RGB mode
 *       1 (SAA_MODE_FBAS) = CVBS mode
 *       2 (SAA_MODE_SVIDEO) = Y/C
 *       3 (SAA_MODE_COMPONENT) = component
 *
/***************************************************/
int stv6412_set_mode(struct i2c_client *client, int vol)
{
	dprintk("[STV6412] SAAIOSMODE command : %d\n", vol);
	if (vol == SAA_MODE_RGB)
	{
		if (stv6412_data.t_vsc == 4)  // if in AUX mode
		{
			stv6412_s_old_src = 1;  // save value for SoC video
		}
		else
		{
			stv6412_data.t_vsc = 1;  // else set SoC CVBS video as source for TV SCART CVBS
		}
		stv6412_data.fblk = 1;  // set fast blanking to high (RGB mode)
	}
	else if (vol == SAA_MODE_FBAS)
	{
		if (stv6412_data.t_vsc == 4)  // if in AUX mode
		{
			stv6412_s_old_src = 1;  // save value for SoC video
		}
		else
		{
			stv6412_data.t_vsc = 1;  // else set SoC CVBS video as source for TV SCART CVBS
		}
		stv6412_data.fblk = 0;  // set fast blanking to low (RGB mode off)
	}
	else if (vol == SAA_MODE_SVIDEO)
	{
		if (stv6412_data.t_vsc == 4) // scart selected
		{
			stv6412_s_old_src = 2;  // save value for SoC video
		}
		else
		{
			stv6412_data.t_vsc = 2;  // else set SoC Y/C video as source for TV SCART CVBS
		}
		stv6412_data.fblk = 0;  // set fast blanking to low (RGB mode off)
	}
	else
	{
		return  -EINVAL;
	}
	return stv6412_set(client);
}

/***************************************************
 *
 * Set TV SCART sources.
 *
 * src = format
 *       0 both video and audio come from SoC
 *       1 both video and audio come from VCR SCART
 *
/***************************************************/
int stv6412_src_sel(struct i2c_client *client, int src)
{
	if (src == SAA_SRC_ENC)
	{
		stv6412_data.t_vsc = stv6412_s_old_src;
		stv6412_data.v_vsc = stv6412_s_old_src;
		stv6412_data.tc_asc = 1;  // TV SCART audio source is SoC audio
		stv6412_data.v_asc = 1;  // VCR SCART audio source is SoC audio
	}
	else if (src == SAA_SRC_SCART)
	{
		stv6412_s_old_src = stv6412_data.t_vsc;
#if !defined(ADB_BOX)
		stv6412_data.t_vsc = 4;  // set TV SCART AUX mode
		stv6412_data.v_vsc = 0;  // mute VCR video CVBS output
		stv6412_data.tc_asc = 2;  // TV SCART audio source is VCR SCART audio
		stv6412_data.v_asc = 0;  // VCR SCART audio source is muted
#endif
  	}
  	else
	{
		return  -EINVAL;
	}
	return stv6412_set(client);
}

/***************************************************
 *
 * Set standby mode.
 *
 * type: 1 = standby, 0 = active
 *
/***************************************************/
inline int stv6412_standby(struct i2c_client *client, int type)
{
 
	if ((type < 0) || (type > 1))
	{
		return -EINVAL;
	}
 
	if (type == 1) 
	{
		if (t_stnby == 0)
		{
			tmpstv6412_data = stv6412_data;  // save current AVS data
			// Full Stop mode
			/* Data 6 */
			stv6412_data.e_in = 1;  // SoC inputs off
			stv6412_data.v_in = 1;  // VCR inputs off
			stv6412_data.t_in = 1;  // TV inputs off
			stv6412_data.a_in = 1;  // AUX inputs off
			stv6412_data.v_out = 1;  // TV SCART outputs off
			stv6412_data.c_out = 1;  // CINCH outputs off
			stv6412_data.t_out = 1;  // TV SCART inputs off
			stv6412_data.r_out = 1;  // RF modulator outputs off
			t_stnby = 1;  // flag standby mode set
		}
		else
		{
			return -EINVAL;		
		}
	}
	else
	{
		if (t_stnby == 1)  // if in standby mode
		{
			stv6412_data = tmpstv6412_data;  // get old data
			t_stnby = 0;  // flag standby off
		}
		else
		{
			return -EINVAL;
		}
	}
	return stv6412_set(client);
}

/***************************************************
 *
 * Execute command.
 *
/***************************************************/
int stv6412_command(struct i2c_client *client, unsigned int cmd, void *arg)
{
	int val=0;

	unsigned char scartPin8Table[3] = { 0, 2, 3 };
	unsigned char scartPin8Table_reverse[4] = { 0, 0, 1, 2 };

	printk("[STV6412]: command\n");
	
	if (cmd & AVSIOSET)
	{
		if (copy_from_user(&val, arg, sizeof(val)))
		{
			return -EFAULT;
		}

		switch (cmd)
		{
			/* set video */
			case AVSIOSVSW1:
			{
				return stv6412_set_vsw(client, 0, val);
			}
			case AVSIOSVSW2:
			{
				return stv6412_set_vsw(client, 1, val);
			}
			case AVSIOSVSW3:
			{
				return stv6412_set_vsw(client, 2, val);
			}
			/* set audio */
			case AVSIOSASW1:
			{
				return stv6412_set_asw(client, 0, val);
			}
			case AVSIOSASW2:
			{
				return stv6412_set_asw(client, 1, val);
			}
			case AVSIOSASW3:
			{
				return stv6412_set_asw(client, 2, val);
			}
			/* set vol & mute */
			case AVSIOSVOL:
			{
				return stv6412_set_volume(client, val);
			}
			case AVSIOSMUTE:
			{
				return stv6412_set_mute(client, val);
			}
			/* set video fast blanking */
			case AVSIOSFBLK:
			{
#if defined(FS9000) \
 || defined(HL101) \
 || defined(VIP1_V1) \
 || defined(IPBOX9900) \
 || defined(IPBOX99)
				printk("[STV6412] does not support AVSIOSFBLK yet!\n");
				return -1;
#else
				return stv6412_set_fblk(client,val);
#endif
			}
#if 1
			/* set slow blanking (tv) */
			/* no direct manipulation allowed, use set_wss instead */
			case AVSIOSSCARTPIN8:
			{
				return stv6412_set_t_sb(client, scartPin8Table[val]);
			}
			case AVSIOSFNC:
			{
				return stv6412_set_t_sb(client, val);
			}
#endif
			case AVSIOSTANDBY:
			{
				return stv6412_standby(client, val);
			}
			default:
			{
				return -EINVAL;
			}
		}
	}
	else if (cmd & AVSIOGET)
	{
		switch (cmd)
		{
			/* get video */
			case AVSIOGVSW1:
			{
				val = stv6412_get_vsw(0);
				break;
			}
			case AVSIOGVSW2:
			{
				val = stv6412_get_vsw(1);
				break;
			}
			case AVSIOGVSW3:
			{
				val = stv6412_get_vsw(2);
				break;
			}
			/* get audio */
			case AVSIOGASW1:
			{
				val = stv6412_get_asw(0);
				break;
			}
			case AVSIOGASW2:
			{
				val = stv6412_get_asw(1);
				break;
			}
			case AVSIOGASW3:
			{
				val = stv6412_get_asw(2);
				break;
			}
			/* get vol & mute */
			case AVSIOGVOL:
			{
				val = stv6412_get_volume();
				break;
			}
			case AVSIOGMUTE:
			{
				val = stv6412_get_mute();
				break;
			}
			/* get video fast blanking */
			case AVSIOGFBLK:
			{
#if defined(FS9000) \
 || defined(HL101) \
 || defined(VIP1_V1) \
 || defined(IPBOX9900) \
 || defined(IPBOX99)
				printk("[STV6412] does not support AVSIOSFBLK yet!\n");
				break;
#else
				val = stv6412_get_fblk();
				break;
#endif
			}
			case AVSIOGSCARTPIN8:
			{
				val = scartPin8Table_reverse[stv6412_get_t_sb()];
				break;
			}
			/* get slow blanking (tv) */
			case AVSIOGFNC:
			{
				val = stv6412_get_t_sb();
				break;
			}
			/* get status */
			case AVSIOGSTATUS:
			{
				// TODO: error handling
				val = stv6412_get_status(client);
				break;
			}
			default:
			{
				return -EINVAL;
			}
		}
		return put_user(val, (int*)arg);
	}
	else
	{
		printk("[STV6412]: SAA command\n");

		/* an SAA command */
		if (copy_from_user(&val, arg, sizeof(val)))
		{
			return -EFAULT;
		}

		switch (cmd)
		{
			case SAAIOSMODE:
			{
		   		 return stv6412_set_mode(client, val);
			}
	 	        case SAAIOSENC:
			{
				 return stv6412_set_encoder(client, val);
			}
			case SAAIOSWSS:
			{
				return stv6412_set_wss(client, val);
			}
			case SAAIOSSRCSEL:
			{
				return stv6412_src_sel(client, val);
			}
			default:
			{
				dprintk("[STV6412]: SAA command %d not supported\n", cmd);
				return -EINVAL;
			}
		}
	}
	return 0;
}

/***************************************************
 *
 * Execute command.
 *
/***************************************************/
int stv6412_command_kernel(struct i2c_client *client, unsigned int cmd, void *arg)
{
   int val = 0;

	unsigned char scartPin8Table[3] = { 0, 2, 3 };
	unsigned char scartPin8Table_reverse[4] = { 0, 0, 1, 2 };

	dprintk("[STV6412]: command_kernel(%u)\n", cmd);
	
	if (cmd & AVSIOSET)
	{
		val = (int) arg;

      		dprintk("[STV6412]: AVSIOSET command\n");

		switch (cmd)
		{
			/* set video */
			case AVSIOSVSW1:
			{
				return stv6412_set_vsw(client, 0, val);
			}
			case AVSIOSVSW2:
			{
				return stv6412_set_vsw(client, 1, val);
			}
			case AVSIOSVSW3:
			{
				return stv6412_set_vsw(client, 2, val);
			}
			/* set audio */
			case AVSIOSASW1:
			{
				return stv6412_set_asw(client, 0, val);
			}
			case AVSIOSASW2:
			{
				return stv6412_set_asw(client, 1, val);
			}
			case AVSIOSASW3:
			{
				return stv6412_set_asw(client, 2, val);
			}
			/* set vol & mute */
			case AVSIOSVOL:
			{
				return stv6412_set_volume(client, val);
			}
			case AVSIOSMUTE:
			{
				return stv6412_set_mute(client, val);
			}
			/* set video fast blanking */
			case AVSIOSFBLK:
			{
				return stv6412_set_fblk(client, val);
			}
#if 1
			/* set slow blanking (tv) */
			/* no direct manipulation allowed, use set_wss instead */
			case AVSIOSSCARTPIN8:
			{
				return stv6412_set_t_sb(client, scartPin8Table[val]);
			}
			case AVSIOSFNC:
			{
				return stv6412_set_t_sb(client, val);
			}
#endif
			case AVSIOSTANDBY:
			{
				return stv6412_standby(client, val);
			}
			default:
			{
				return -EINVAL;
			}
		}
	}
	else if (cmd & AVSIOGET)
	{
		dprintk("[STV6412]: AVSIOGET command\n");

		switch (cmd)
		{
			/* get video */
			case AVSIOGVSW1:
			{
				val = stv6412_get_vsw(0);
				break;
			}
			case AVSIOGVSW2:
			{
				val = stv6412_get_vsw(1);
				break;
			}
			case AVSIOGVSW3:
			{
				val = stv6412_get_vsw(2);
				break;
			}
			/* get audio */
			case AVSIOGASW1:
			{
				val = stv6412_get_asw(0);
				break;
			}
			case AVSIOGASW2:
			{
				val = stv6412_get_asw(1);
				break;
			}
			case AVSIOGASW3:
			{
				val = stv6412_get_asw(2);
				break;
			}
			/* get vol & mute */
			case AVSIOGVOL:
			{
				val = stv6412_get_volume();
				break;
			}
			case AVSIOGMUTE:
			{
				val = stv6412_get_mute();
				break;
			}
			/* get video fast blanking */
			case AVSIOGFBLK:
			{
				val = stv6412_get_fblk();
				break;
			}
			case AVSIOGSCARTPIN8:
			{
				val = scartPin8Table_reverse[stv6412_get_t_sb()];
				break;
			}
			/* get slow blanking (tv) */
			case AVSIOGFNC:
			{
				val = stv6412_get_t_sb();
				break;
			}
			/* get status */
			case AVSIOGSTATUS:
			{
				// TODO: error handling
				val = stv6412_get_status(client);
				break;
			}
			default:
			{
				return -EINVAL;
			}
		}
		*((int*)arg) = (int)val;
		return 0;
	}
	else
	{
		printk("[STV6412]: SAA command\n");
		val = (int) arg;

		switch (cmd)
		{
			case SAAIOSMODE:
			{
		   		 return stv6412_set_mode(client, val);
			}
	 	        case SAAIOSENC:
			{
				 return stv6412_set_encoder(client, val);
			}
			case SAAIOSWSS:
			{
				return stv6412_set_wss(client, val);
			}
			case SAAIOSSRCSEL:
			{
				return stv6412_src_sel(client, val);
			}
			default:
			{
				dprintk("[STV6412]: SAA command %d not supported\n", cmd);
				return -EINVAL;
			}
		}
	}
	return 0;
}

/***************************************************
 *
 * Initialize the STV6412.
 *
/***************************************************/
int stv6412_init(struct i2c_client *client)
{
	memset((void*)&stv6412_data,0,STV6412_DATA_SIZE);

#if 0
	stv6412_data.svm = 0;
	stv6412_data.t_vol_c = 0;
	stv6412_data.t_vol_x = 0;
	stv6412_data.t_stereo = 0;

	stv6412_data.c_ag = 0;
	stv6412_data.tc_asc = 1;
	stv6412_data.v_asc  = 1;
	stv6412_data.v_stereo = 0;

	stv6412_data.t_vsc = 1;
	stv6412_data.rgb_vsc = 1;
	stv6412_data.t_cm = 0;
	stv6412_data.rgb_gain = 0;
	stv6412_data.rgb_tri = 1;

	stv6412_data.v_cm = 0;
	stv6412_data.v_vsc = 1;
	stv6412_data.rgb_gain = 0;
	stv6412_data.rgb_tri = 1;
	stv6412_data.fblk = 0;

	stv6412_data.slb = 0;
	stv6412_data.t_sb = 3;

	stv6412_data.a_in  = 1;
	stv6412_data.v_sb = 0;
#else
	/* Data 0 */
	stv6412_data.t_vol_c = 0;  // TV volume: 0 dB
	 /* Data 1 */
	stv6412_data.v_asc   = 1;  // VCR audio: Encoder L/R
	stv6412_data.tc_asc  = 1;  // TV / Cinch audio: Encoder L/R
	/* Data 2 */
	stv6412_data.v_vsc   = 1;  // VCR CVBS: Encoder CVBS
	stv6412_data.t_vsc   = 1;  // TV CVBS: Encoder CVBS
	/* Data 3 */
	stv6412_data.rgb_tri = 1; // TV SCART: RGB and FB outputs on
	stv6412_data.rgb_vsc = 1; // TV SCART: RGB output from Encoder
	stv6412_data.fblk    = 2; // Fast blanking from SoC
	/* Data 4 */
	stv6412_data.t_rcos  = 1;  // Chroma on TV R (wrong!, should be 0 for RGB output)
	stv6412_data.res2    = 1;  // ?? bit is don't care
	/* Data 5 */
	stv6412_data.t_sb    = 3;  // TV slow blanking is 4:3 (why not 16:9?) 
	stv6412_data.v_sb    = 0;  // VCR slow blanking is input
	/* Data 6 */
	stv6412_data.a_in    = 0; // AUX inputs active
#endif
	stv6412_s_old_src = 1;

	/* save mute/unmute values */
	tc_asc = 0xff;
	v_asc  = 0xff;
	return stv6412_set(client);
}
// vim:ts=4
