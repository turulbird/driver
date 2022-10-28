#ifndef CXA2161R_123
#define CXA2161R_123

/*
 *   CXA2161R.h - audio/video switch driver
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
 */

/* I2C data structure (write)

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
 Bit 0-2: TV vdeo switch
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
 Bit 0  :Enable Vout1
 Bit 1  :Enable Vout2
 Bit 2  :Enable Vout3
 Bit 3  :Enable Vout4
 Bit 4  :Enable Vout5
 Bit 5  :Enable Vout6
 Bit 6  :Vout5 0V
 Bit 7  :Zero Cross Detect
*/
// Write data7 definitions

/* I2C data structure (read)
Data 1
 Bit 0-1: FNC_VCR
 Bit 2  : Sync detect
 Bit 3  : Not used
 Bit 4  : Power on dtect
 Bit 5  : Zero cross status
 Bit 6-7: Not used
 */
// Read data definitions
#define FNC_VCR           0x03
#define SYNC_DETECT       0x04
#define CXA2161R_POD      0x10
#define ZERO_CROSS_STATUS 0x20

int cxa2161r_init(struct i2c_client *client);
int cxa2161r_command(struct i2c_client *client, unsigned int cmd, void *arg);
int cxa2161r_command_kernel(struct i2c_client *client, unsigned int cmd, void *arg);
int cxa2161r_set_volume(struct i2c_client *client, int vol);
int cxa2161r_get_volume(void);
int cxa2161r_get_status(struct i2c_client *client);
#endif

