/***************************************************************************
 *
 * 74t1.c  Enigma2 compatible driver for XXX74T1 I2C controlled RF modulator
 *
 * Version for Shenzen Tena Electronics TNF-0170S722-2 modulator as used
 * in Fulan Spark7162 models
 *
 * (c) 2021 Audioniek
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
 ***************************************************************************
 *
 * Changes
 *
 * Date     By              Description
 * -------------------------------------------------------------------------
 * 20210326 Audioniek       Initial version, based on Shenzen Tena
 *                          Electronics datasheet for TNF-0170S722-2.
 *
 */

#include "rfmod_core.h"

/***************************************************************************
 *
 * Note: the term "frequency" in this driver is meant to mean the frequency
 * of the video carrier generated by the XXX74T1.
 * Throughout the driver this value is in kHz.
 *
 */

#if defined TAGDEBUG
#undef TAGDEBUG
#endif
#define TAGDEBUG "[74t1] "

enum  // enumerate the sound carrier frequencies
{
	AC4_5_MHZ,
	AC5_5_MHZ,
	AC6_0_MHZ,
	AC6_5_MHZ
};

unsigned char *audiosubcarrier_name[4] =
{
	"4.5",
	"5.5",
	"6.0",
	"6.5"
};

// global variables
unsigned char byte[4];  // values for the four bytes to write to the XXX74T1
int base_video_carrier;

/*******************************************
 *
 * I2C routines
 *
 */
unsigned char xxx_74t1_read(struct i2c_client *client)
{
	unsigned char byte;

//	dprintk(100, "%s >\n", __func__);

	byte = 0;

	if (i2c_master_recv(client, &byte, 1) != 1)
	{
		dprintk(1, "%s: Error reading data from address 0x%02x\n", client->addr);
		return -1;
	}
//	dprintk(100, "%s < OK, data = 0x%02x\n", __func__, byte);
	return byte;
}

// This function writes two bytes to the XXX74T1
int xxx_74t1_write_2bytes(struct i2c_client *client, unsigned char byte1, unsigned char byte2)
{
	int err;
	unsigned char reg[2] = { byte1, byte2 };  // the two bytes to write

//	dprintk(100, "%s >\n", __func__);

#if 0
	if (paramDebug > 70)
	{
		if (reg[0] & 0x80)
		{
			dprintk(1, "Write: C1=0x%02x, C2=0x%02x to address 0x%02x\n", reg[0], reg[1], client->addr);
		}
		else
		{
			dprintk(1, "Write: PB1=0x%02x, PB2=0x%02x to address 0x%02x\n", reg[0], reg[1], client->addr);
		}
	}
#endif
	if ((err = i2c_master_send(client, reg, sizeof(reg))) != sizeof(reg))
	{
		dprintk(1, "%s: Error writing XXX74T1: err = %i\n", __func__, err);
		return -EREMOTEIO;
	}
	return 0;
}

/******************************************************************
 *
 *  Data read format of the XXX74T1
 *
 *  byte1:   1  |  1  |  0  |  0  |  1  |  0  |  1  |  1    Address byte
 *  byte2:   X  |  X  |  X  |  X  |  X  |  Y2 |  Y1 | OOR   Status byte
 *
 *  byte1 = address
 *  byte2:
 *	OOR = Out Of Range flag (1 = OOR)
 *	Y1 = Error data, valid only if OOR = 1: 0 = frequency to set too low, 1 = too high
 *  Y2 = VCO in use, 0 = High, 1 = Low
 *	X  = Don't care (ignore)
 *
 */

/******************************************************************
 *
 *  Data write format of the XXX74T1
 *
 *  byte1:   1  |  1   |  0  |  0   |  1   |  0  |  1  |  0    Address byte (ADR)
 *  byte2:   1  |  0   |  SO |  0   |  PS  |  X3 |  X2 |  0    Control byte 1 (C1)
 *  byte3:  PWC | OSC  | ATT | SFD1 | SFD0 |  0  |  X5 |  X4   Control byte 2 (C2)
 *  byte4:   0  | TPEN | N11 | N10  |  N9  |  N8 |  N7 |  N6   Ports byte 1 (PB1)
 *  byte5:   N5 |  N4  |  N3 |  N2  |  N1  |  N0 |  X1 |  X0   Ports byte 2 (PB2)
 *
 * Meaning of individual bits:
 * SO    = Sound Oscillator on/off; 0=on, PowerOn value=0 
 * PS    = Picture sound ratio: 0: 12dB 1: 16dB (recommended), PowerOn value=0
 * PWC   = Peak White Clip (0 = on), PowerOn value=0
 * OSC   = Carrier Oscillator enable: 1=on (PowerOn value=1)
 * ATT   = Modulator output attenuator (0=normal, 1: attenuator on), PowerOn value=0
 * SFD10 = Sound subcarrier frequency selection
 *         00 = 4.5 MHz
 *         01 = 5.5 MHz (PowerOn value)
 *         10 = 6.0 MHz
 *         11 = 6.5 MHz
 * TPEN  = Test pattern enable: 1=on, 0=noral operation, PowerOn value=0
 * Nx    = PLL Carrier divider bits, PowerOn value dependent on chip revision
 *         F = N / 4 MHz
 * X210  = Set Carrier frequency divider (may be used to generate VHF frequencies)
 *         000 = divider = 1 (normal operation for UHF, also PowerOn value)
 *         001 = divider = 2
 *         010 = divider = 4
 *         011 = divider = 8
 *         100 = divider = 16
 *         Other values are illegal
 *         Note: the driver always set these bits to zero (UHF operation)
 *
 * X543  = Test modes, write as zero for normal operation, PowerOn value=000
 *
 * Software standby is achieved when SO=1, OSC=0 and ATT=1 at the same time
 *
 * Allowable write sequences are:
 * ADR -> C1 -> C2 -> PB1 -> PB2 (not used by driver)
 * ADR -> PB1 -> PB2 -> C1 -> C2 (not used by driver)
 * ADR -> C1 -> C2
 * ADR -> PB1 -> PB2
 *
 * For further details, see datasheet.
 *
 */

// Bitmask definitions
#define BIT74T1_SO   0x20  // byte[0] (C1)
#define BIT74T1_PS   0x08  // byte[0] (C1)
#define BIT74T1_X5   0x04  // byte[1] (C2)
#define BIT74T1_X4   0x02  // byte[1] (C2)
#define BIT74T1_X3   0x04  // byte[0] (C1)
#define BIT74T1_X2   0x02  // byte[0] (C1)
#define BIT74T1_X1   0x02  // byte[2] (PB1)
#define BIT74T1_X0   0x01  // byte[2] (PB1)
#define BIT74T1_PWC  0x80  // byte[1] (C2)
#define BIT74T1_OSC  0x40  // byte[1] (C2)
#define BIT74T1_ATT  0x20  // byte[1] (C2)
#define BIT74T1_SFD1 0x10  // byte[1] (C2)
#define BIT74T1_SFD0 0x08  // byte[1] (C2)
#define BIT74T1_TPEN 0x40  // byte[2] (PB1)

/************************************************************
 *
 * Set video carrier frequency
 *
 * Input: divisor factor N
 *
 * Note: sets X2..0 to zero (UHF operation)
 *
 */
int xxx_74t1_set_video_carrier_freq(struct i2c_client *client, int N)
{
	int res = 0;
	unsigned char status;

//	dprintk(100, "%s > Set video carrier frequency to %d.%03d MHz\n", __func__, ((N * 1000) / 4) / 1000, ((N * 1000) / 4) % 1000);
	byte[2] &= BIT74T1_TPEN;  // clear MSbit and N bits, preserve TPEN
	byte[2] |= (N >> 6);   // add N11..N6
	byte[3] = 0;  // clear X1, X0, N bits
	byte[3] |= ((N & 0x3f) << 2);   // add N5..0
	res = xxx_74t1_write_2bytes(client, byte[2], byte[3]);
	msleep(300);
	byte[0] &= ~(BIT74T1_X3 + BIT74T1_X2);  // clear X3 and X2
	res |= xxx_74t1_write_2bytes(client, byte[0], byte[1]);
	status = xxx_74t1_read(client);
	if (status & 0x01)  // get OOR bit
	{
		dprintk(1, "Error: VCO out of range; frequency too %s\n", (status & 0x02 ? "low" : "high"));
	}
	return res;
}

/************************************************************
 *
 * Convert channel to frequency in kHz
 *
 * Notes:
 * 1. Valid channel numbers are 21 - 69;
 * 2. Channel 21 has a carrier of 471.25 MHz (CCIR PAL-G standard);
 * 3. Channel spacing is 8 MHz (CCIR PAL-G standard).
 *
 * Returns: frequency in kHz
 */
int conv_chan_2_freq(int channel)
{
	int freq = 0;

	base_video_carrier = ((channel - 21) * 8000) + 471250;  // kHz, channel 21 is 471.25 MHz, spacing is 8 MHz (CCIR PAL-G)
	dprintk(20, "Channel: %d -> frequency: %d.%03d\n", channel, base_video_carrier / 1000, base_video_carrier % 1000);
	return base_video_carrier;
}

/************************************************************
 *
 * Calculate divider N from frequency in kHz
 *
 * Returns: N
 *
 * Note: frequency must be between 471250 and 855250 kHz.
 *
 * Datasheet specifies the following formula:
 *
 * N = (F / 8) * (128 / 4) with F in MHz (note: assumes 4 MHz Xtal)
 *
 * which reduces to: N = F * 4 (F in MHz)
 *
 */
int xxx_74t1_calc_N_from_freq(int freq)
{
	int N;

	if (freq < 471250 || freq > 855250)
	{
		dprintk(1, "Warning: invalid frequency %d.%03d MHz\n", freq / 1000, freq % 1000);
		return -1;
	}
	N = ((freq << 2) / 1000);
//	dprintk(10, "Video carrier frequency: %d.%03d -> N = %d\n", freq / 1000, freq % 1000, N);
	return N;
}

/************************************************************
 *
 * Calculate divider N from channel number
 *
 * Notes:
 * 1. Valid channel numbers are 21 - 69;
 * 2. Channel 21 has a carrier of 471.25 MHz (CCIR PAL-G standard);
 * 3. Channel spacing is 8 MHz (CCIR PAL-G standard).
 *
 */
int xxx_74t1_calc_N_from_channel(int channel)
{
	int freq;  // frequency in kHz
	int N;

	if (channel < 21)
	{
		dprintk(1, "Warning: channel# too low, setting channel 21\n");
		channel = 21;
	}
	if (channel > 69)
	{
		dprintk(1, "Warning: channel# too high, setting channel 69\n");
		channel = 69;
	}
	freq = conv_chan_2_freq(channel);
	N = xxx_74t1_calc_N_from_freq(freq);
	if (N < 0)
	{
		dprintk(1, "Error: illegal divisor N, set channel 36\n");
		N = 2365;
	}
	return N;
}

/************************************************************
 *
 * Set audio carrier bits
 *
 * Input: enum audio carrier frequency
 *
 */
void set_audiocarrier_bits(int audiocarrier)
{
	byte[1] &= ~(BIT74T1_SFD1 + BIT74T1_SFD0);  // clear sound carrier bits
	byte[1] |= (audiocarrier & 0x03) << 3;
}

/************************************************************
 *
 * IOCTL routines
 *
 */
int xxx_74t1_set_channel(struct i2c_client *client, int channel)
{
	int N;

	N = xxx_74t1_calc_N_from_channel(channel);
	return xxx_74t1_set_video_carrier_freq(client, N);
}

int xxx_74t1_set_testmode(struct i2c_client *client, int testmode)
{
	if (testmode)
	{
		// TPEN=1
		byte[2] |=  BIT74T1_TPEN;  // TPEN=1  // test pattern on
	}
	else
	{
		// TPEN=0
		byte[2] &= ~BIT74T1_TPEN;  // TPEN=0  // test pattern off
	}
	return xxx_74t1_write_2bytes(client, byte[2], byte[3]);
}

int xxx_74t1_set_audioenable(struct i2c_client *client, int enable)
{
	if (enable)
	{
		// SO=0
		byte[0] &= ~BIT74T1_SO;  // SO=0  // sound oscillator off
	}
	else
	{
		// SO=1
		byte[0] |=  BIT74T1_SO;  // SO=1  // sound oscillator on
	}
	return xxx_74t1_write_2bytes(client, byte[0], byte[1]);
}

int xxx_74t1_set_audiocarrier(struct i2c_client *client, int audiocarrier)
{
	int res;

//	dprintk(100, "%s > Set audio subcarrier to %d\n", __func__, audiosubcarrier_name[audiocarrier]);
	set_audiocarrier_bits(audiocarrier);
	return xxx_74t1_write_2bytes(client, byte[0], byte[1]);
}

int xxx_74t1_finetune(struct i2c_client *client, int finetune)
{
	int N, NewN;

//	dprintk(100, "%s > Fine tune %d (%d kHz)\n", __func__, finetune, finetune * 250);
	
	// get current base carrier frequency and calculate N
	N = xxx_74t1_calc_N_from_freq(base_video_carrier);
	if (N < 0)
	{
		dprintk(1, "Error: illegal divisor N, set channel 36\n");
		N = xxx_74t1_calc_N_from_channel(36);
	}
	NewN = N + finetune;
	return xxx_74t1_set_video_carrier_freq(client, NewN);
}

int xxx_74t1_set_standby(struct i2c_client *client, int standby)
{
	int res;

	if (standby)
	{
		// OSC=0, SO=1, ATT=1
		byte[1] &= ~BIT74T1_OSC;  // OSC=0 -> UHF oscillator off
		byte[0] |=  BIT74T1_SO;   // SO=1  -> sound oscillator off
		byte[1] |=  BIT74T1_ATT;  // ATT=1 -> set software standby
		return xxx_74t1_write_2bytes(client, byte[0], byte[1]);
	}
	else
	{
		// OSC=1, SO=0, ATT=0
		byte[1] |=  BIT74T1_OSC;  // OSC=1  ->UHF oscillator on
		byte[0] &= ~BIT74T1_SO;   // SO=0  -> sound oscillator on
		byte[1] &= ~BIT74T1_ATT;  // ATT=0 -> normal operation
		res = xxx_74t1_write_2bytes(client, byte[0], byte[1]);
		// data sheet states to reprogram the video carrier
		// frequency after leaving standby
		res |= xxx_74t1_write_2bytes(client, byte[2], byte[3]);
		msleep(300);  // datasheet says wait 263 ms minimum for oscillator to lock and stabilize
		return res;
	}
}

int xxx_74t1_ioctl_kernel(struct i2c_client *client, unsigned int cmd, void *arg)
{
	int i;
	int val;

	if (copy_from_user(&val, arg, sizeof(val)))
	{
		return -EFAULT;
	}
	dprintk(100, "%s > (IOCTL 0x%02x, arg=%d)\n", __func__, cmd, val);

	switch (cmd)
	{
		case IOCTL_SET_CHANNEL:
		{
			dprintk(20, "Set video carrier to channel %d\n", val);
			return xxx_74t1_set_channel(client, val);
		}
		case IOCTL_SET_TESTMODE:
		{
			dprintk(20, "Set test mode %s\n", (val ? "on" : "off"));
			return xxx_74t1_set_testmode(client, val);
		}
		case IOCTL_SET_SOUNDENABLE:
		{
			dprintk(20, "%s sound\n", (val ? "Enable" : "Disable"));
			return xxx_74t1_set_audioenable(client, val);
		}
		case IOCTL_SET_SOUNDSUBCARRIER:
		{
			dprintk(20, "Set sound subcarrier to %s MHz\n", audiosubcarrier_name[val]);
			return xxx_74t1_set_audiocarrier(client, val);
		}
		case IOCTL_SET_FINETUNE:
		{
			dprintk(20, "Fine tune %d kHz\n", val * 250);
			return xxx_74t1_finetune(client, val);
		}
		case IOCTL_SET_STANDBY:
		{
			dprintk(20, "Switch modulator %s\n", (val ? "off" : "on"));
			return xxx_74t1_set_standby(client, val);
		}
		default:
		{
			dprintk(1, "Error: invalid IOCTL 0x%08x\n", cmd);
		}
	}
	arg = (void *)val;
	return 0;
}

int xxx_74t1_ioctl(struct i2c_client *client, unsigned int cmd, void *arg)
{
	return xxx_74t1_ioctl_kernel(client, cmd, arg);
}

/***************************************************************************
 *
 * Initialize the XXX74T1
 *
 * Initial settings are:
 * - Video carrier channel 21 (471.25MHz, CCIR PAL-G);
 * - Audio carrier 5.5MHz (CCIR PAL-G);
 * - Power mode: software standby;
 * - Picture to sound ratio 16dB;
 * - Peak White Clip off;
 */
int xxx_74t1_init(struct i2c_client *client)
{
	int res;
	int N;

//	dprintk(100, "%s >\n", __func__);

	memset(byte, 0, sizeof(byte));  // initialize data array
	
	// Prepare byte C1 (byte[0])
	// Note because of previous memset, only the 1 bits need to be set
	byte[0] |=  0x80;        // set MSbit
	byte[0] |=  BIT74T1_SO;  // SO=1 -> sound oscillator off (needed for standby)
	byte[0] |=  BIT74T1_PS;  // PS=1 -> picture to sound ratio is 16dB (recommended value)
	byte[0] &= ~BIT74T1_X3;  // X3=0
	byte[0] &= ~BIT74T1_X2;  // X2=0
	
	// Prepare byte C2 (byte[1])
	byte[1] |=  BIT74T1_PWC;  // PWC=1 -> Peak White Clip off
	byte[1] |=  BIT74T1_ATT;  // ATT=1 -> Modulator output attenuated (needed for standby)
	set_audiocarrier_bits(AC5_5_MHZ);

	// set C1 and C2 control bytes
	res = xxx_74t1_write_2bytes(client, byte[0], byte[1]);

	// Set initial channel
	N = xxx_74t1_calc_N_from_channel(36);
	res = xxx_74t1_set_video_carrier_freq(client, N);
	return res;
}
// vim:ts=4
