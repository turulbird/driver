/*
 *   cxa2161r.c - audio/video switch driver
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
 *   Implementation for Homecast HS8100/9000/9100 series
 *
 *   HS8100/9000/9100 connections to the CXA2161R are as follows:
 *
 *   TV SCART CVBS in     (20) : NC
 *   TV SCART CVBS out    (19) : VOUT4    CXA2161R pin 43
 *   TV SCART audioinL     (6) : NC
 *   TV SCART audioinR     (2) : NC
 *   TV SCART audiooutL    (3) : LTV      CXA2161R pin 25
 *   TV SCART audiooutR    (1) : RTV      CXA2161R pin 26
 *   TV SCART R           (15) : VOUT_3   CXA2161R pin 45
 *   TV SCART G           (11) : VOUT_2   CXA2161R pin 46
 *   TV SCART B            (7) : VOUT_1   CXA2161R pin 47
 *   TV SCART Fast blank  (16) : TV_FBLK  CXA2161R pin 9 
 *   TV SCART status       (8) : FNC_TV   CXA2161R pin 11
 *
 *   VCR SCART CVBS in    (20) : VIN_10   CXA2161R pin 4
 *   VCR SCART CVBS out   (19) : VOUT_6   CXA2161R pin 39
 *   VCR SCART audioinL    (6) : LIN_2    CXA2161R pin 19
 *   VCR SCART audioinR    (2) : RIN_2    CXA2161R pin 20
 *   VCR SCART audiooutL   (3) : LOUT1    CXA2161R pin 27
 *   VCR SCART audiooutR   (1) : ROUT1    CXA2161R pin 28
 *   VCR SCART R          (15) : VIN_7    CXA2161R pin 5
 *   VCR SCART G          (11) : VIN_4    CXA2161R pin 6
 *   VCR SCART B           (7) : VIN_2    CXA2161R pin 7
 *   VCR SCART Fast blank (16) : FBLK_IN2 CXA2161R pin 12
 *   VCR SCART status      (8) : FNC_VCR  CXA2161R pin 13

 *   CINCH CVBS                : VOUT_7   CXA2161R pin 38
 *   CINCH Audio L             : PHONO_L  CXA2161R pin 30
 *   CINCH Audio R             : PHONO_R  CXA2161R pin 31
 *
 *   VOUT_5, VIN_5, LIN_4, RIN_4 & MONO are NC
 *
 *   The CXA2161R is therefore used as follows:
 *
 *   Video switch 1 is used for:
 *   1. Selecting TV RGB outputs and can be switched to
 *      B: VIN_1 (SoC B) or VIN_2 (VCR B) or mute to VOUT_1 (TV B)
 *      G: VIN_3 (SoC G) or VIN_4 (VCR G) or mute to VOUT_2 (TV B)
 *      R: VIN_5 (SoC R) or VIN_6 (SoC C) or VIN_7 (VCR R) or VIN_13 (AUX C) or mute to VOUT_3 (TV R)
 *
 *   2. Selecting CINCH CVBS and can be switched to:
 *      VIN_8  (SoC CVBS)
 *      VIN_9  (SoC Y?)
 *      VIN_10 (VCR CVBS in)
 *      VIN_11 (?)
 *      VIN_12 (NC)
 *      VIN_3  (?)
 *      Mute
 *
 *   Video switch 2 is used for:
 *   1. VCR CVBS and can be switched to:
 *      VIN_8  (SoC CVBS)
 *      VIN_9  (SoC Y?)
 *      VIN_10 (VCR CVBS in)
 *      VIN_11 (TV CVBS)
 *      VIN_12 (NC)
 *      VIN_3  (?)
 *      Mute
 *   2. VOUT_5 (which is not connected in the HS8100)
 *
 *   The FBLK switch is used for switching the state for TV SCART pin 16
 *   (fast blanking) and can be switched to:
 *   +3.5V
 *   0 V
 *   FBLK_IN1 (NC)
 *   FBLK_IN2 (VCR FBLK)
 *
 *   Audio switch 1 is used for switching audio intended for the TV and
 *   controls both the TV SCART and CINCH audio connections.
 *   Its primary function is to select the auto source which can be:
 *   LIN_1/RIN_1 (SoC audio)
 *   LIN_2/RIN_2 (VCR audio in)
 *   LIN_3/RIN_3 (NC)
 *   LIN_4/RIN_4 (NC)
 *   Mute
 *   
 *   Both the SCART and CINCH audio outputs can be connected to the source
 *   selector directly, or through the volume control in the CXA2161R
 *   by using the bypass settings.
 *
 *   In addition the TV SCART audio outputs can be set to stereo, L+R,
 *   left only or right only.
 *
 *   Audio switch 2 is used for switching audio intended for VCR SCART audio
 *   outputs and controls the source of the outputs. It can be set to:
 *   LIN_1/RIN_1 (SoC audio)
 *   LIN_2/RIN_2 (VCR audio in)
 *   LIN_3/RIN_3 (NC)
 *   LIN_4/RIN_4 (NC)
 *   Mute
 *   
 *   In addition the VCR SCART audio outputs can be set to stereo, L+R,
 *   left only or right only.
 *      
 */

//#include <linux/version.h>
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
#include "cxa2161r.h"
#include "tools.h"

#define CXA2161R_MAX_REGS 7

static int debug = AVS_DEBUG;

static unsigned char regs[CXA2161R_MAX_REGS + 1]; /* range 0x01 to 0x07 */

static unsigned char backup_regs[CXA2161R_MAX_REGS + 1];
/* hold old values for standby */
static unsigned char t_stnby=0;


#define CXA2161R_DATA_SIZE sizeof(regs)

/* CXA2161R I2C data structure (write)

Data1
 Bit 0  : TV audio mute
 Bit 1-5: Volume control
 Bit 6-7: Rin/Lin Gain control

Data2
 Bit 0  : Phono bypass
 Bit 1-2: TV audio select
 Bit 3-5: TV mono switch
 Bit 6  : TV volume bypass
 Bit 7  : Mono switch

Data3
 Bit 0  : Overlay enable
 Bit 1-2: VCR audio select
 Bit 3-5: VCR mono switch
 Bit 6  : Output limit
 Bit 7  : TV audio mute

Data4
 Bit 0-1: Fast blank
 Bit 2  : FNC direction
 Bit 3  : FNC follow
 Bit 4-5: FNC level
 Bit 6  : Logic level
 Bit 7  : TV input mode

Data5
 Bit 0-2: TV video switch
          Blue Vout2  Green Vout2 Red Vout2 CVBS/Y Vout4
 Bit 3-4: RGB gain
          00: 0dB (power on default)
          01: +1dB
          10: +2dB
          11: +3dB
 Bit 5-7: VCR switch
               Chroma Vout5         CVBS Vout6         Comment
          000  Encode chroma Vin5   Encode CVBS Vin8   Digital Encoder
 Y/C

Data6
 Bit 0-1: Mixer control
 Bit 2  : Vin3 clamp
 Bit 3  : Vin7 clamp
 Bit 4  : Vin5 clamp
 Bit 5-6: Sync selection
 Bit 7  : VCR input mute

Data7
 Bit 0  : Enable Vout1
 Bit 1  : Enable Vout2
 Bit 2  : Enable Vout3
 Bit 3  : Enable Vout4
 Bit 4  : Enable Vout5
 Bit 5  : Enable Vout6
 Bit 6  : Vout5 0V
 Bit 7  : Zero Cross Detect

CXA2161R I2C data structure (read)

Data 1
 Bit 0-1: FNC_VCR
 Bit 2  : Sync detect
 Bit 3  : Not used
 Bit 4  : Power on detect
 Bit 5  : Zero cross status
 Bit 6-7: Not used
*/

/*
 * cxa2161r data struct
 *
 */
typedef struct s_cxa2161r_data
{
	// write
	/* Data 1 */
	unsigned char tv_audio_mute    : 1;
	unsigned char volume_control   : 5;
	unsigned char rin1_lin1_gain   : 2;
	/* Data 2 */
	unsigned char phono_bypass     : 1;
	unsigned char tv_audio_select  : 2;
	unsigned char tv_mono_switch   : 3;
	unsigned char tv_vol_bypass    : 1;
	unsigned char mono_switch      : 1;
	/* Data 3 */
	unsigned char overlay_enable   : 1;
	unsigned char vcr_audio_select : 2;
	unsigned char vcr_mono_switch  : 3;
	unsigned char output_limit     : 1;
	unsigned char tv_audio_mute2   : 1;
	/* Data 4 */
	unsigned char fast_blank       : 2;
	unsigned char fnc_direction    : 1;
	unsigned char fnc_follow       : 1;
	unsigned char fnc_level        : 2;
	unsigned char logic_level      : 1;
	unsigned char tv_input_mute    : 1;
	/* Data 5 */
	unsigned char tv_video_switch  : 3;
	unsigned char rgb_gain         : 2;
	unsigned char vcr_video_switch : 3;
	/* Data 6 */
	unsigned char mixer_control    : 2;
	unsigned char vin3_clamp       : 1;
	unsigned char vin7_clamp       : 1;
	unsigned char vin5_clamp       : 1;
	unsigned char sync_sel         : 2;
	unsigned char vcr_input_mute   : 1;
	/* Data 7 */
	unsigned char enable_vout1     : 1;
	unsigned char enable_vout2     : 1;
	unsigned char enable_vout3     : 1;
	unsigned char enable_vout4     : 1;
	unsigned char enable_vout5     : 1;
	unsigned char enable_vout6     : 1;
	unsigned char vout5_0v         : 1;
	unsigned char zcd              : 1;

	// read
	/* Data1 */
	unsigned fnc_vcr               : 2;
	unsigned sync_detect           : 1;
	unsigned not_used1             : 1;
	unsigned pod                   : 1;
	unsigned zcd_status            : 1;
	unsigned not_used2             : 1;
	unsigned not_used3             : 1;
} s_cxa2161r_data;
 
#define CXA2161R_DATA_SIZE1 sizeof(s_cxa2161r_data)
static struct s_cxa2161r_data cxa2161r_data;
static struct s_cxa2161r_data tmpcxa2161r_data;
static int cxa2161r_s_old_src;

#define cReg0  0x01
#define cReg1  0x02
#define cReg2  0x03
#define cReg3  0x04
#define cReg4  0x05
#define cReg5  0x06
#define cReg6  0x06

/* hold old values for mute / unmute */
static unsigned char audio_value; /* audio switch control */

#if 0
int cxa2161r_set2(struct i2c_client *client)
{
	int i;
	
	regs[0] = 0x00; //bit staly START
	regs[1] = 0x1E; //0x00h > 0x06 
	regs[2] = 0x00; //0x01h > 0x02 
	regs[3] = 0x51; //0x02h > 0x52
	regs[4] = 0x55; //0x03h > 0x03 04
	regs[5] = 0x00; //0x04h > 0x04
	regs[6] = 0x00; //bit staly END

	dprintk(50, "[CXA2161R] %s > data size is %d\n", __func__, CXA2161R_DATA_SIZE);

	dprintk(20, "[CXA2161R] init regs = { ");
	for (i = 0; i <= CXA2161R_MAX_REGS; i++)
	{
		if (paramDebug > 19)
		{
			printk("0x%02x ", regs[i]);
		}
	}
	if (paramDebug > 19)
	{
		printk(" }\n");
	}
	if (CXA2161R_DATA_SIZE != i2c_master_send(client, regs, CXA2161R_DATA_SIZE))
	{
		dprintk(1, "[CXA2161R] %s: error sending data\n", __func__);
		return -EFAULT;
	}
	dprintk(50, "[CXA2161R] %s <\n", __func__);
	return 0;
}
#endif

int cxa2161r_set(struct i2c_client *client)
{
	char buffer[CXA2161R_DATA_SIZE1 + 1];

	buffer[0] = 0;

	memcpy(buffer + 1, &cxa2161r_data, CXA2161R_DATA_SIZE1);

	if ((CXA2161R_DATA_SIZE1 + 1) != i2c_master_send(client, buffer, CXA2161R_DATA_SIZE1 + 1))
	{
		return -EFAULT;
	}
	return 0;
}

/***************************************************
 *
 * Set volume.
 *
 ***************************************************/
int cxa2161r_set_volume(struct i2c_client *client, int vol)
{
	int c = 0;

	dprintk(50, "[CXA2161R] %s >\n", __func__);
	c = vol;

	if (c == 63)
	{
		c = 62;
	}
//	if (c == 0)
//	{
//		c = 1;
//	}
	if ((c > 63) || (c < 0))
	{
		return -EINVAL;
	}
	c /= 2;
	cxa2161r_data.volume_control = c;
	dprintk(50, "[CXA2161R] %s <\n", __func__);
	return cxa2161r_set(client);
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
inline int cxa2161r_set_mute(struct i2c_client *client, int sw, int type)
{
	dprintk(50, "[CXA2161R] %s >\n", __func__);

	if ((type < AVS_UNMUTE) || (type > AVS_MUTE))
	{
		return -EINVAL;
	}

	switch (sw)  // get output type
	{
		case 0:
		{
			cxa2161r_data.vcr_input_mute = type;
			break;
		}
		case 1:
		case 2:
		{
			cxa2161r_data.tv_audio_mute  = type;
			cxa2161r_data.tv_audio_mute2 = type;
			cxa2161r_data.tv_input_mute  = type;
			break;
		}
		default:
		{
			return -EINVAL;
		}
	}
	dprintk(50, "[CXA2161R] %s <\n", __func__);
	return cxa2161r_set(client);
}

/***************************************************
 *
 * Set video signal type format.
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
inline int cxa2161r_set_vsw(struct i2c_client *client, int sw, int type)
{
	dprintk(20, "[CXA2161R] %s: Set VSW: %d %d\n",__func__, sw, type);

	if (type < 0 || type > 4)
	{
		return -EINVAL;
	}

	switch (sw)  // get output to set: VCR CVBS, TV RGB, TV CVBS
	{
		case 0:	 // VCR video output selection
		{
			switch (type)
			{
				case 0:  // video muted
				{
					cxa2161r_data.vcr_video_switch = 7;  // muted
					break;
				}
				case 1:  // SoC CVBS video
				{
					cxa2161r_data.vcr_video_switch = 1;  // SoC (Vin8)
					break;
				}
				case 2:  // SoC Y/C video
				{
					cxa2161r_data.vcr_video_switch = 0;  // SoC (Vin9)
					break;
				}
				case 3:  // TV SCART CVBS out
				{
					cxa2161r_data.vcr_video_switch = 3;  // TV SCART (Vin11)
					break;
				}
	        }
			break;
		}
		case 1:	 // RGB selection
		{
			if (type < 0 || type > 2)
			{
				return -EINVAL;
			}
			switch (type)
			{
				case 0:  // RGB muted
				{
					cxa2161r_data.tv_video_switch = 7;  // muted
					break;
				}
				case 1:  // SoC RGB
				{
					cxa2161r_data.tv_video_switch = 0;  // SoC RGB
					break;
				}
				case 2:  // VCR RGB?
				{
					cxa2161r_data.tv_video_switch = 2;  // VCR RGB
					break;
				}
			}
			break;
		}
		case 2: // TV video CVBS output selection
		{
			switch (type)
			{
				case 0:  // CVBS muted
				{
					cxa2161r_data.tv_video_switch = 7;  // muted
					break;
				}
				case 1:  // SoC CVBS
				{
					cxa2161r_data.tv_video_switch = 0;  // SoC CVBS (Vin8)
					break;
				}
				case 2:  // SoC Y/C
				{
					cxa2161r_data.tv_video_switch = 1;  // SoC Y/C (Vin9)
					break;
				}
				case 3:  // VCR CVBS
				{
					cxa2161r_data.tv_video_switch = 2;  // VCR CVBS (Vin10)
					break;
				}
			}
			break;
		}
		default:
		{
			return -EINVAL;
		}
	}
	dprintk(50, "[CXA2161R] %s <\n", __func__);
	return cxa2161r_set(client);
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
inline int cxa2161r_set_asw(struct i2c_client *client, int sw, int type)
{
	dprintk(50, "[CXA2161R] %s >\n", __func__);

	switch (sw)
	{
		case 0:  // VCR SCART
		{
			cxa2161r_data.vcr_input_mute = 0;  // enable VCR audio output

			switch (type)
			{
				case 0:  // VCR SCART audio muted
				{
					cxa2161r_data.vcr_input_mute = 1;  // disable VCR audio output
					break;
				}
				case 1:  // VCR SCART audio: SoC audio
				{
					cxa2161r_data.vcr_audio_select = 0;  // Lin/Rin1 (Soc)
					break;
				}
				case 2:  // VCR SCART audio: TV SCART in (NC)
				case 4:  // VCR SCART audio: TV?
				{
					dprintk(20, "[CXA2161R] %s Note: Lin/Rin3 are not connected (so no output)\n", __func__);
					cxa2161r_data.vcr_audio_select = 2;  // Lin/Rin3 (TV/OVERLAY)
					break;
				}
				case 3:  // VCR SCART audio: AUX (NC)
				{
					dprintk(20, "[CXA2161R] %s Note: Lin/Rin4 are not connected (so no output)\n", __func__);
					cxa2161r_data.vcr_audio_select = 3;  // Lin/Rin4 (AUX)
					break;
				}
			}
			break;
		}
		if (type == 1)
		{
			cxa2161r_data.fast_blank = 3;  // set RGB mode
		}
		else
		{
			cxa2161r_data.fast_blank = 0;  // set CVBS mode
		}
		case 1:  // TV SCART RGB
		case 2:  // TV SCART CVBS
		{
			switch (type)
			{
				case 0:  // TV SCART audio muted
				{
					cxa2161r_data.vcr_input_mute = 1;  // disable VCR audio output
					break;
				}
				case 1:  // TV SCART audio: SoC audio
				{
					cxa2161r_data.tv_audio_select = 0;  // Lin/Rin1 (Soc)
					break;
				}
				case 2:  // TV SCART audio: VCR SCART in
				{
					cxa2161r_data.tv_audio_select = 1;  // Lin/Rin2 (VCR)
					break;
				}
				case 4:  // TV SCART audio: TV (NC)
				{
					dprintk(20, "[CXA2161R] %s Note: Lin/Rin3 are not connected (so no output)\n", __func__);
					cxa2161r_data.tv_audio_select = 2;  // Lin/Rin3 (TV/OVERLAY)
					break;
				}
				case 3:  // VCR SCART audio: AUX (NC)
				{
					dprintk(20, "[CXA2161R] %s Note: Lin/Rin4 are not connected (so no output)\n", __func__);
					cxa2161r_data.tv_audio_select = 3;  // Lin/Rin4 (AUX)
					break;
				}
			}
			break;
		}
		default:
		{
			return -EINVAL;
		}
	}
	dprintk(50, "[CXA2161R] %s <\n", __func__);
	return cxa2161r_set(client);
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
inline int cxa2161r_set_t_sb(struct i2c_client *client, int type)
{
/* fixme: on cxa2161r we have another range
 * think on this ->see register description below
 */
	dprintk(50, "[CXA2161R] %s >\n", __func__);

	if (type < 0 || type > 3)
	{
		return -EINVAL;
	}
	switch (type)
	{
		case 0:
		case 2:  // internal TV
		{
			cxa2161r_data.fnc_level = 0;
			break;
		}
		case 1:  // 16:9
		{
			cxa2161r_data.fnc_level = 1;
			break;
		}
		case 3:  // 4:3
		{
			cxa2161r_data.fnc_level = 3;
			break;
		}
		default:
		{
			return -EINVAL;
		}
	}
	cxa2161r_data.fnc_direction = 0;  // set level
	cxa2161r_data.fnc_follow = 0;  // VCR status to input
	dprintk(50, "[CXA2161R] %s <\n", __func__);
	return cxa2161r_set(client);
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
inline int cxa2161r_set_wss(struct i2c_client *client, int vol)
{
	dprintk(50, "[CXA2161R] %s >\n", __func__);

	if (vol == SAA_WSS_43F)
	{
		cxa2161r_data.fnc_level = 3;
	}
	else if (vol == SAA_WSS_169F)
	{
		cxa2161r_data.fnc_level = 1;
	}
	else if (vol == SAA_WSS_OFF)
	{
		cxa2161r_data.fnc_level = 0;
	}
	else
	{
		return  -EINVAL;
	}
	cxa2161r_data.fnc_direction = 0;  // set level
	cxa2161r_data.fnc_follow = 0;  // VCR status to input
	dprintk(50, "[CXA2161R] %s <\n", __func__);
	return cxa2161r_set(client);
}

/***************************************************
 *
 * Set TV SCART fast blanking pin 16.
 *
 * type = status
 *        0 = low level (CVBS mode)
 *        1 = high level (RGB mode)
 *        2 = from SoC (not supported)
 *        3 = from VCR SCART fast blanking pin
 *
 * Note: does not actually set WSS but sets status
 *       pin of TV SCART to matching aspect ratio.
 *
/***************************************************/
inline int cxa2161r_set_fblk(struct i2c_client *client, int type)
{
	dprintk(50, "[CXA2161R] %s >\n", __func__);

	if (type < 0 || type > 3)
	{
		return -EINVAL;
	}
	switch (type)
	{
		case 0:
		{
			cxa2161r_data.fast_blank = 0;
			break;
		}
		case 1:
		{
			cxa2161r_data.fast_blank = 3;
			break;
		}
		case 3:
		{
			cxa2161r_data.fast_blank = 2;
			break;
		}
		default:
		{
			dprintk(1, "[CXA2161R] %s: Fast blank type %d is not supported by the CXA2161R\n", type);
			return -EINVAL;
		}
	}
	dprintk(50, "[CXA2161R] %s <\n", __func__);
	return cxa2161r_set(client);
}

/***************************************************
 *
 * Get AVS status.
 *
 * Merely reads the status byte and returns it
 * without further processing.
 *
/***************************************************/
int cxa2161r_get_status(struct i2c_client *client)
{
	unsigned char byte;
	char *fnc_vcr_names[4] = { "Internal", "16:9 external", "Not defined", "4:3 external" };

	dprintk(50, "[CXA2161R] %s >\n", __func__);

	byte = 0;

	if (1 != i2c_master_recv(client, &byte, 1))
	{
		return -1;
	}
	switch (byte & FNC_VCR)
	{
		case 0:
		case 1:
		case 3:
		{
			dprintk(20, "[CXA2161R] %s VCR status pin   : %s\n", __func__, fnc_vcr_names[byte & FNC_VCR]);
		}
		default:
		{
			dprintk(20, "[CXA2161R] %s VCR status pin   : not determinable\n", __func__);
		}		
	}
	dprintk(20, "[CXA2161R] %s Sync detect      : %s\n", __func__, (byte & SYNC_DETECT ? "Yes" : "No"));
	dprintk(20, "[CXA2161R] %s Power On detect  : %s\n", __func__, (byte & CXA2161R_POD ? "Yes" : "No"));
	dprintk(20, "[CXA2161R] %s Zero Croos status: %s detected\n", __func__, (byte & ZERO_CROSS_STATUS ? "" : "not"));
	dprintk(50, "[CXA2161R] %s <\n", __func__);
	return byte;
}

/***************************************************
 *
 * Get volume.
 *
 *
/***************************************************/
int cxa2161r_get_volume(void)
{
	int c;

	dprintk(50, "[CXA2161R] %s >\n", __func__);
	c = cxa2161r_data.volume_control;
	if (c)
	{
		c *= 2;  // times 2
	}
	dprintk(50, "[CXA2161R] %s <\n", __func__);
	return c;
}

/***************************************************
 *
 * Get mute status. Returns zero if both TV SCART
 * and VCR SCART are not muted.
 *
/***************************************************/
inline int cxa2161r_get_mute(void)
{
	dprintk(50, "[CXA2161R] %s <>\n", __func__);
	return !(audio_value == 0xff);
}

/***************************************************
 *
 * Get fast blanking status.
 *
/***************************************************/
inline int cxa2161r_get_fblk(void)
{
	unsigned char c;

	dprintk(50, "[CXA2161R] %s >\n", __func__);
	c = cxa2161r_data.fast_blank & 0x03;
	switch (c)
	{
		case 0:
		{
			return 0;
		}
		case 3:
		{
			return 1;
		}
		case 1:
		{
			dprintk(20, "[CXA2161R] %s Fast blanking level is equal to Fast blank input 1 (cannot be read from CXA2161R\n", __func__);
			break;
		}
		case 2:
		{
			dprintk(20, "[CXA2161R] %s Fast blanking level is equal to VCR fast blank input (cannot be read from CXA2161R\n", __func__);
			break;
		}
	}
	return -1;
}

/***************************************************
 *
 * Get TV SCART status.
 *
/***************************************************/
inline int cxa2161r_get_t_sb(void)
{
	int c;

	dprintk(50, "[CXA2161R] %s <>\n", __func__);
	switch (cxa2161r_data.fnc_level)
	{
		case 0:
		case 2:
		default:
		{
			return 1;
		}
		case 1:  // 16:9
		{
			return 2;
		}
		case 3:
		{
			return 3;
		}
	}
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
 * return values:
 * 0: muted
 * 1: CVBS from SoC
 * 2: Y/C from SoC
 * 3: CVBS + Chroma from VCR (TV SCART) or TV (VCR SCART)
 * 4: CVBS from AUX
 *
/***************************************************/
inline int cxa2161r_get_vsw(int sw)
{
	dprintk(50, "[CXA2161R] %s >\n", __func__);

	switch (sw)
	{
		case 0:  // VCR SCART
		{
			switch (cxa2161r_data.vcr_video_switch)
			{
				case 7:  // muted
				{
					return 0;
				}
				case 1:  // CVBS from SoC
				default:
				{
					return 1;
				}
				case 0: // Y/C from SoC
				{
					return 2;
				}
				case 3:  // CVBS from TV
				{
					return 3;
				}
				case 5:  // CVBS from AUX
				{
					return 4; 
				}
			}
		}
		case 1:  // TV SCART RGB
		case 2:
		default:
		{
			switch (cxa2161r_data.tv_video_switch)
			{
				case 7:  // muted
				{
					return 0;
				}
				case 0:  // CVBS from SoC
				default:
				{
					return 1;
				}
				case 1: // Y/C from SoC
				{
					return 2;
				}
				case 3:  // CVBS from TV
				{
					return 3;
				}
				case 5:  // CVBS from AUX
				{
					return 4; 
				}
			}
		}
	}
	dprintk(50, "[CXA2161R] %s <\n", __func__);
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
 * Returns:
 * 0: muted
 * 1: Audio from SoC
 * 2: Audio from opposite SCART
 * 3: Audio from AUX
 * 4: Audio from TV SCART audio input (TV SCART only)
 *
/***************************************************/
inline int cxa2161r_get_asw(int sw)
{
	dprintk(50, "[CXA2161R] %s >\n", __func__);

	switch (sw)
	{
		case 0:  // VCR
		{
			// muted ? yes: return tmp values
			if (cxa2161r_data.vcr_input_mute == 1)
			{
				return 0;
			}
			else
			{
				switch (cxa2161r_data.vcr_audio_select)
				{
					case 0: // audio from SoC
					{
						return 1;
					}
					case 2: // audio from TV SCART
					{
						dprintk(20, "CAUTION: Lin/Rin3 are not connected, so no output\n");
						return 2;
					}
					case 3: // audio from AUX
					{
						dprintk(20, "CAUTION: Lin/Rin4 are not connected, so no output\n");
						return 3;
					}
				}
			}
		}
		case 1:  // TV RGB
		case 2:  // TV CVBS
		{
			if (cxa2161r_data.tv_audio_mute == 1 || cxa2161r_data.tv_audio_mute2 == 1)
			{
				return 0;
			}
			else
			{
				switch (cxa2161r_data.tv_audio_select)
				{
					case 0: // audio from SoC (Lin/Rin1)
					{
						return 1;
					}
					case 1: // audio from VCR SCART (Lin/Rin2)
					default:
					{
						return 2;
					}
					case 3: // audio from AUX (Lin/Rin4)
					{
						dprintk(20, "CAUTION: Lin/Rin4 are not connected, so no output\n");
						return 3;
					}
					case 2:  // audio from TV SCART input (Lin/Rin3)
					{
						dprintk(20, "CAUTION: Lin/Rin3 are not connected, so no output\n");
						return 4;
					}
				}
			}
		}
		default:
		{
			return -EINVAL;
		}
	}
	dprintk(50, "[CXA2161R] %s <\n", __func__);
	return -EINVAL;
}

/***************************************************
 *
 * Set encoder mode.
 *
 * NOT IMPLEMENTED
 *
/***************************************************/
int cxa2161r_set_encoder(struct i2c_client *client, int vol)
{
	return 0;
}
 
/***************************************************
 *
 * Set TV SCART output video format.
 *
 * val = format
 *       0 (SAA_MODE_RGB) = RGB mode
 *       1 (SAA_MODE_FBAS) = CVBS mode
 *       2 (SAA_MODE_SVIDEO) = Y/C
 *       3 (SAA_MODE_COMPONENT) = component
 *
/***************************************************/
int cxa2161r_set_mode(struct i2c_client *client, int val)
{
	dprintk(20, "[CXA2161R] SAAIOSMODE command : 0x%04x\n", val);

	if (val == SAA_MODE_RGB)  // set TV SCART pin 16 to RGB mode
	{
		if (cxa2161r_data.tv_video_switch == 6)  // if in AUX mode
		{
			cxa2161r_s_old_src = 0;
		}
		else
		{
			cxa2161r_data.tv_video_switch = 0;
		}
		cxa2161r_data.fast_blank = 3;
	}
	else if (val == SAA_MODE_FBAS)  // set TV SCART pin 16 to CVBS mode
	{
		if (cxa2161r_data.tv_video_switch == 6)  // if in AUX mode
		{
			cxa2161r_s_old_src = 0;
		}
		else
		{
			cxa2161r_data.tv_video_switch = 0;
		}
		cxa2161r_data.fast_blank = 0;
	}
#if 0
	else if (val == SAA_MODE_SVIDEO)  // set TV SCART pin 16 to Y/C mode
	{
		if (cxa2161r_data.tv_video_switch == 6)  // if in AUX mode
		{
			cxa2161r_s_old_src = 2;
		}
		else
		{
//			set_bits(regs, cReg2, 2, 5, 2);
//			set_bits(regs, cReg3, 0, 5, 2);  /* fast blanking */
		}
	}
	else if (val == SAA_MODE_COMPONENT)  // set TV SCART pin 16 to Component mode
	{
		if (cxa2161r_data.tv_video_switch == 6)  // if in AUX mode
		{
			cxa2161r_s_old_src = 2;
		}
		else
		{
//			set_bits(regs, cReg2, 2, 5, 2);
//			set_bits(regs, cReg3, 0, 5, 2);  /* fast blanking */
		}
	}
#endif
	else
	{
		return  -EINVAL;
	}
	dprintk(50, "[CXA2161R] %s <\n", __func__);
	return cxa2161r_set(client);
}
 
/***************************************************
 *
 * Set TV SCART sources.
 *
 * src = source
 *       0 both video and audio come from SoC
 *       1 both video and audio come from VCR SCART
 *
/***************************************************/
int cxa2161r_src_sel(struct i2c_client *client, int src)
{  // TODO
	dprintk(50, "[CXA2161R] %s >\n", __func__);

	if (src == SAA_SRC_ENC)
	{
		cxa2161r_data.tv_video_switch = 0;  // TV SCART RGB & CVBS from SoC
		cxa2161r_data.tv_audio_select = 0;  // TV SCART audio from SoC
		cxa2161r_data.fnc_follow = 0;  // TV SCART pin 8 = FNC_LEVEL, VCR SCART pin 8 is input
		cxa2161r_data.fnc_direction = 0;
	}
	else if (src == SAA_SRC_SCART)
	{
		cxa2161r_data.tv_video_switch = 2;  // TV SCART RGB & CVBS from VCR SCART
		cxa2161r_data.tv_audio_select = 1;  // TV SCART audio from VCR SCART
		cxa2161r_data.fnc_follow = 1;  // TV SCART pin 8 = follow VCR SCART pin 8, VCR SCART pin 8 is input
		cxa2161r_data.fnc_direction = 0;
  	}
  	else
	{
		return  -EINVAL;
	}
	dprintk(50, "[CXA2161R] %s <\n", __func__);
	return cxa2161r_set(client);
}

/***************************************************
 *
 * Set standby mode.
 *
 * type: 1 = standby, 0 = active
 *
/***************************************************/
inline int cxa2161r_standby(struct i2c_client *client, int type)
{
	dprintk(50, "[CXA2161R] %s >\n", __func__);

	if ((type < 0) || ( type > 1))
	{
		return -EINVAL;
	}
	if (type == 1)
	{
		if (t_stnby == 0)
		{
			tmpcxa2161r_data = cxa2161r_data;  // save current AVS data
			// Disable all outputs
			cxa2161r_data.enable_vout1    = 0;  // TV SCART B
			cxa2161r_data.enable_vout2    = 0;  // TV SCART G
			cxa2161r_data.enable_vout3    = 0;  // TV SCART R
			cxa2161r_data.enable_vout4    = 0;  // TV SCART CVBS out
//			cxa2161r_data.enable_vout5    = 0;  // NC
			cxa2161r_data.enable_vout6    = 0;  // VCR SCART CVBS out
			cxa2161r_data.vcr_mono_switch = 7;  // all audio utput disabled
			t_stnby                       = 1;  // flag standby mode set
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
			cxa2161r_data = tmpcxa2161r_data;  // get old data
			t_stnby = 0;  // flag standby off
		}
		else
		{
			return -EINVAL;
		}
	}
	dprintk(50, "[CXA2161R] %s <\n", __func__);
	return cxa2161r_set(client);
}

/***************************************************
 *
 * Execute command.
 *
/***************************************************/
int cxa2161r_command(struct i2c_client *client, unsigned int cmd, void *arg)
{
	int val = 0;
	unsigned char scartPin8Table[3] = { 0, 1, 3 };
	unsigned char scartPin8Table_reverse[4] = { 0, 0, 1, 3 };

	dprintk(50, "[CXA2161R] %s cmd = 0x%04x\n", __func__, cmd);
	
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
				return cxa2161r_set_vsw(client, 0, val);
			}
			case AVSIOSVSW2:
			{
				return cxa2161r_set_vsw(client, 1, val);
			}
			case AVSIOSVSW3:
			{
				return cxa2161r_set_vsw(client, 2, val);
			}
			/* set audio */
			case AVSIOSASW1:
			{
				return cxa2161r_set_asw(client, 0, val);
			}
			case AVSIOSASW2:
			{
				return cxa2161r_set_asw(client, 1, val);
			}
			case AVSIOSASW3:
			{
				return cxa2161r_set_asw(client, 2, val);
			}
			/* set vol & mute */
			case AVSIOSVOL:
			{
				return cxa2161r_set_volume(client, val);
			}
			case AVSIOSMUTE:
			{
				return cxa2161r_set_mute(client, 2, val);
			}
			/* set video fast blanking */
			case AVSIOSFBLK:
			{
				return cxa2161r_set_fblk(client, val);
			}
			/* set slow blanking (tv) */
			case AVSIOSSCARTPIN8:
			{
				return cxa2161r_set_t_sb(client, scartPin8Table[val]);
			}
			case AVSIOSFNC:
			{
				return cxa2161r_set_t_sb(client, val);
			}
			case AVSIOSTANDBY:
			{
				return cxa2161r_standby(client, val);
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
				val = cxa2161r_get_vsw(0);
				break;
			}
			case AVSIOGVSW2:
			{
				val = cxa2161r_get_vsw(1);
				break;
			}
			case AVSIOGVSW3:
			{
				val = cxa2161r_get_vsw(2);
				break;
			}
			/* get audio */
			case AVSIOGASW1:
			{
				val = cxa2161r_get_asw(0);
				break;
			}
			case AVSIOGASW2:
			{
				val = cxa2161r_get_asw(1);
				break;
			}
			case AVSIOGASW3:
			{
				val = cxa2161r_get_asw(2);
				break;
			}
			/* get vol & mute */
			case AVSIOGVOL:
			{
				val = cxa2161r_get_volume();
				break;
			}
			case AVSIOGMUTE:
			{
				val = cxa2161r_get_mute();
				break;
			}
			/* get video fast blanking */
			case AVSIOGFBLK:
			{
				val = cxa2161r_get_fblk();
				break;
			}
			case AVSIOGSCARTPIN8:
			{
				val = scartPin8Table_reverse[cxa2161r_get_t_sb()];
				break;
			}
			/* get slow blanking (tv) */
			case AVSIOGFNC:
			{
				val = cxa2161r_get_t_sb();
				break;
			}
			/* get status */
			case AVSIOGSTATUS:
			{
				// TODO: error handling
				val = cxa2161r_get_status(client);
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
		/* an SAA command */
		if (copy_from_user(&val, arg, sizeof(val)))
		{
			return -EFAULT;
		}

		switch (cmd)
		{
			case SAAIOSMODE:
			{
				return cxa2161r_set_mode(client, val);
			}
			case SAAIOSENC:
			{
				return cxa2161r_set_encoder(client, val);
			}
			case SAAIOSWSS:
			{
				return cxa2161r_set_wss(client, val);
			}
			case SAAIOSSRCSEL:
			{
				return cxa2161r_src_sel(client, val);
			}
			default:
			{
				dprintk(1, "[CXA2161R] %s: SAA command 0x%04x not supported\n", __func__, cmd);
				return -EINVAL;
			}
		}
	}
	dprintk(50, "[CXA2161R] %s <\n", __func__);
	return 0;
}

/***************************************************
 *
 * Execute command.
 *
/***************************************************/
int cxa2161r_command_kernel(struct i2c_client *client, unsigned int cmd, void *arg)
{
	int val = 0;
	unsigned char scartPin8Table[3] = { 0, 1, 3 };
	unsigned char scartPin8Table_reverse[4] = { 0, 0, 1, 3 };
	
	dprintk(50, "[CXA2161R] %s >\n", __func__);

	if (cmd & AVSIOSET)
	{
		val = (int) arg;

      	dprintk(20, "[CXA2161R] %s: AVSIOSET command 0x%04x\n", __func__, cmd);

		switch (cmd)
		{
			/* set video */
			case AVSIOSVSW1:
			{
				return cxa2161r_set_vsw(client, 0, val);
			}
			case AVSIOSVSW2:
			{
				return cxa2161r_set_vsw(client, 1, val);
			}
			case AVSIOSVSW3:
			{
				return cxa2161r_set_vsw(client, 2, val);
			}
			/* set audio */
			case AVSIOSASW1:
			{
				return cxa2161r_set_asw(client, 0, val);
			}
			case AVSIOSASW2:
			{
				return cxa2161r_set_asw(client, 1, val);
			}
			case AVSIOSASW3:
			{
				return cxa2161r_set_asw(client, 2, val);
			}
			/* set vol & mute */
			case AVSIOSVOL:
			{
				return cxa2161r_set_volume(client, val);
			}
			case AVSIOSMUTE:
			{
				return cxa2161r_set_mute(client, 2, val);
			}
			/* set video fast blanking */
			case AVSIOSFBLK:
			{
				return cxa2161r_set_fblk(client, val);
			}
			/* set slow blanking (tv) */
			/* no direct manipulation possible, use set_t_sb instead */
			case AVSIOSSCARTPIN8:
			{
				return cxa2161r_set_t_sb(client, scartPin8Table[val]);
			}
			case AVSIOSFNC:
			{
				return cxa2161r_set_t_sb(client, val);
			}
			case AVSIOSTANDBY:
			{
				return cxa2161r_standby(client, val);
			}
			default:
			{
				dprintk(1, "[CXA2161R] %s: AVSIOSET command 0x%04x not supported\n", __func__, cmd);
				return -EINVAL;
			}
		}
	}
	else if (cmd & AVSIOGET)
	{
      	dprintk(20, "[CXA2161R] %s: AVSIOGET command 0x%04x\n", __func__, cmd);

		switch (cmd)
		{
			/* get video */
			case AVSIOGVSW1:
			{
				val = cxa2161r_get_vsw(0);
				break;
			}
			case AVSIOGVSW2:
			{
				val = cxa2161r_get_vsw(1);
				break;
			}
			case AVSIOGVSW3:
			{
				val = cxa2161r_get_vsw(2);
				break;
			}
			/* get audio */
			case AVSIOGASW1:
			{
				val = cxa2161r_get_asw(0);
				break;
			}
			case AVSIOGASW2:
			{
				val = cxa2161r_get_asw(1);
				break;
			}
			case AVSIOGASW3:
			{
				val = cxa2161r_get_asw(2);
				break;
			}
			/* get vol & mute */
			case AVSIOGVOL:
			{
				val = cxa2161r_get_volume();
				break;
			}
			case AVSIOGMUTE:
			{
				val = cxa2161r_get_mute();
				break;
			}
			/* get video fast blanking */
			case AVSIOGFBLK:
			{
				val = cxa2161r_get_fblk();
				break;
			}
			case AVSIOGSCARTPIN8:
			{
				val = scartPin8Table_reverse[cxa2161r_get_t_sb()];
				break;
			}
			/* get slow blanking (tv) */
			case AVSIOGFNC:
			{
				val = cxa2161r_get_t_sb();
				break;
			}
			/* get status */
			case AVSIOGSTATUS:
			{
				// TODO: error handling
				val = cxa2161r_get_status(client);
				break;
			}
			default:
			{
				dprintk(1, "[CXA2161R] %s: AVSIOGET command 0x%04x not supported\n", __func__, cmd);
				return -EINVAL;
			}
		}
		arg = (void*)val;
		dprintk(50, "[CXA2161R] %s <\n", __func__);
		return 0;
	}
	else
	{
		dprintk(20, "[CXA2161R] %s: SAA command 0x%04x\n", __func__, cmd);
		val = (int) arg;

		switch (cmd)
		{
			case SAAIOSMODE:
			{
				return cxa2161r_set_mode(client, val);
			}
			case SAAIOSENC:
			{
				return cxa2161r_set_encoder(client, val);
			}
			case SAAIOSWSS:
			{
				return cxa2161r_set_wss(client, val);
			}
			case SAAIOSSRCSEL:
			{
				return cxa2161r_src_sel(client,val);
			}
			default:
			{
				dprintk(1, "[CXA2161R] %s: SAA command 0x%04x not supported\n", __func__, cmd);
				return -EINVAL;
			}
		}
	}
	dprintk(50, "[CXA2161R] %s <\n", __func__);
	return 0;
}

int cxa2161r_init(struct i2c_client *client)
{
	memset((void*)&cxa2161r_data, 0, CXA2161R_DATA_SIZE1);

	dprintk(50, "[CXA2161R] %s >\n", __func__);

	// Set AVS to:
	// Simultanious output on both TV and VCR SCARTs and CINCH video of SoC Video
	// Status on pin 8 is 16:9 mode
	// RGB output on TV SCART active
	// volume control at 0dB
	// All audio outputs on and in stereo
	// (Un)muted on zero crossing

	// get default settings
	// Data 1
	cxa2161r_data.tv_audio_mute    = 0;  // Unmute audio on all outputs on next zero cross
	cxa2161r_data.volume_control   = 3;  // Volume is at 0 dB
	cxa2161r_data.rin1_lin1_gain   = 0;  // SoC audio gain is -6dB
	// Data 2
	cxa2161r_data.phono_bypass     = 0;  // CINCH audio outputs have volume control
	cxa2161r_data.tv_audio_select  = 0;  // TV SCART audio outputs: SoC audio
	cxa2161r_data.tv_mono_switch   = 0;  // TV SCART audio outputs: stereo
	cxa2161r_data.tv_vol_bypass    = 0;  // TV SCART audio outputs have volume control
//	cxa2161r_data.mono_switch      = 0;  // Mono output (NC) connected to TV SCART L+R
	// Data 3
	cxa2161r_data.overlay_enable   = 0;  // overlay off
	cxa2161r_data.vcr_audio_select = 0;  // VCR SCART audio outputs: SoC audio
	cxa2161r_data.vcr_mono_switch  = 0;  // VCR SCART audio outputs: stereo
	cxa2161r_data.output_limit     = 1;  // audio volume is not limited
	cxa2161r_data.tv_audio_mute2   = 0;  // Unmute audio on all outputs on next zero cross
	// Data 4
	cxa2161r_data.fast_blank       = 0;  // CVBS output display
	cxa2161r_data.fnc_direction    = 0; 
	cxa2161r_data.fnc_follow       = 0;  // TV SCART pin 8 is at fnc_level
	cxa2161r_data.fnc_level        = 1;  // SCART input selected, 16:9 mode
	cxa2161r_data.logic_level      = 1;  // Logic output is open collector
	cxa2161r_data.tv_input_mute    = 1;  // mute state of TV audio selector is muted
	// Data 5
	cxa2161r_data.tv_video_switch  = 0;  // TV SCART video: CVBS + RGB from SoC
	cxa2161r_data.rgb_gain         = 0;  // RGB gain is 0dB
	cxa2161r_data.vcr_video_switch = 0;  // VCR SCART video: CVBS from SoC
	// Data 6
	cxa2161r_data.mixer_control    = 3;  // CINCH video from SoC CVBS
	cxa2161r_data.vin3_clamp       = 0;  // Green on VIN_3
	cxa2161r_data.vin7_clamp       = 1;  // Red on VIN_7
//	cxa2161r_data.vin5_clamp       = 0;  // Red in VIN_5 (don't care, VIN5 is NC)
	cxa2161r_data.sync_sel         = 0;  // RGB sync source is SoC CVBS
	cxa2161r_data.vcr_input_mute   = 1;  // mute state of VCR audio selector is muted
	// Data 7
	cxa2161r_data.enable_vout1     = 1;  // TV SCART B output on
	cxa2161r_data.enable_vout2     = 1;  // TV SCART G output on
	cxa2161r_data.enable_vout3     = 1;  // TV SCART R output on
	cxa2161r_data.enable_vout4     = 1;  // TV SCART CVBS output on
//	cxa2161r_data.enable_vout5     = 0;  // VOUT_5 is NC
	cxa2161r_data.enable_vout6     = 1;  // VCR SCART CVBS output on
	cxa2161r_data.vout5_0v         = 1;  // use CXA2161R pin 15 as input
	cxa2161r_data.zcd              = 1;  // (un)mute on zero crossing on
	dprintk(50, "[CXA2161R] %s <\n", __func__);
    return cxa2161r_set(client);
}
// vim:ts=4
