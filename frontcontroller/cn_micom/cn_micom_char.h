/****************************************************************************
 *
 * cn_micom_char.h
 *
 * (c) 2009 Dagobert@teamducktales
 * (c) 2010 Schischu & konfetti: Add irq handling
 * (c) 2021 Audioniek: enlarged charactersets
 *
 * Character definitions for CreNova front panel driver.
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
 * This header file defines the segment patterns for both the LED and VFD
 * models, based on the ASCII input value, which is the table index.
 *
 ****************************************************************************
 *
 * Changes
 *
 * Date     By              Description
 * --------------------------------------------------------------------------
 * 20090306 W.Y.Yi          Original Crenova Micom version
 * 20230309 Audioniek       VFD characterset now full ASCII with option to
 *                          replace lower case letters with upper case due to
 *                          the fact that the centre vertical segment can only
 *                          be controlled as a pair.
 * 20230418 Audioniek       Changed VFD characters '1' and 'm'.
 *
 ****************************************************************************/
#ifndef __CN_MICOM_CHAR_H__
#define __CN_MICOM_CHAR_H__


/* Uncomment next line to enable lower case letters on VFD */
#define VFD_LOWER_CASE                1

/****************************************************************************

  7-segment LED display (Opticum HD 9600 mini, Atemio AM 520HD, Sogno HD800 V3)
  Base model  : MINILINE
  Front Micom : Version 4.xx.xx

        a
    ---------
   |         |
   |         |
 f |         | b
   |         |
   |    g    |
    ---------
   |         |
   |         |
 e |         | c
   |         |
   |         |
    ---------
        d
                          7 6 5 4  3 2 1 0 
 Segment to bit mapping: (x|g|f|e  d|c|b|a)
 Bit 7 is aways zero.

*****************************************************************************/

// Note: font is the same as on Fortis LED models
#define _7SEG_NULL      "\x00"
#define _7SEG_ASC20     "\x00"  // space
#define _7SEG_ASC21     "\x06"  // '!'
#define _7SEG_ASC22     "\x22"  // '"'
#define _7SEG_ASC23     "\x5c"  // '#'
#define _7SEG_ASC24     "\x6d"  // '$'
#define _7SEG_ASC25     "\x52"  // '%'
#define _7SEG_ASC26     "\x7d"  // '&'
#define _7SEG_ASC27     "\x02"  // '
#define _7SEG_ASC28     "\x39"  // '('
#define _7SEG_ASC29     "\x0f"  // ')'
#define _7SEG_ASC2A     "\x76"  // '*'
#define _7SEG_ASC2B     "\x46"  // '+'
#define _7SEG_ASC2C     "\x0c"  // ','
#define _7SEG_ASC2D     "\x40"  // '-'
#define _7SEG_ASC2E     "\x08"  // '.'
#define _7SEG_ASC2F     "\x52"  // '/'
#define _7SEG_NUM_0     "\x3F"  // '0'
#define _7SEG_NUM_1     "\x06"  // '1'
#define _7SEG_NUM_2     "\x5B"  // '2'
#define _7SEG_NUM_3     "\x4F"  // '3
#define _7SEG_NUM_4     "\x66"  // '4'
#define _7SEG_NUM_5     "\x6D"  // '5'
#define _7SEG_NUM_6     "\x7D"  // '6'
#define _7SEG_NUM_7     "\x07"  // '7'
#define _7SEG_NUM_8     "\x7F"  // '8'
#define _7SEG_NUM_9     "\x6F"  // '9'
#define _7SEG_ASC3A     "\x10"  // ':'
#define _7SEG_ASC3B     "\x0c"  // ';'
#define _7SEG_ASC3C     "\x58"  // '<'
#define _7SEG_ASC3D     "\x48"  // '='
#define _7SEG_ASC3E     "\x7c"  // '>'
#define _7SEG_ASC3F     "\x07"  // '?'
#define _7SEG_ASC40     "\x7B"  // '@'
#define _7SEG_CH_A      "\x77"
#define _7SEG_CH_B      "\x7C"
#define _7SEG_CH_C      "\x39"
#define _7SEG_CH_D      "\x5E"
#define _7SEG_CH_E      "\x79"
#define _7SEG_CH_F      "\x71"
#define _7SEG_CH_G      "\x3D"
#define _7SEG_CH_H      "\x76"
#define _7SEG_CH_I      "\x06"
#define _7SEG_CH_J      "\x1E"
#define _7SEG_CH_K      "\x7a"
#define _7SEG_CH_L      "\x38"
#define _7SEG_CH_M      "\x55"
#define _7SEG_CH_N      "\x37"
#define _7SEG_CH_O      "\x3F"
#define _7SEG_CH_P      "\x73"
#define _7SEG_CH_Q      "\x67"
#define _7SEG_CH_R      "\x77"
#define _7SEG_CH_S      "\x6D"
#define _7SEG_CH_T      "\x78"
#define _7SEG_CH_U      "\x3E"
#define _7SEG_CH_V      "\x3E"
#define _7SEG_CH_W      "\x7E"
#define _7SEG_CH_X      "\x76"
#define _7SEG_CH_Y      "\x67"
#define _7SEG_CH_Z      "\x5B"
#define _7SEG_ASC5B     "\x39"  // '['
#define _7SEG_ASC5C     "\x64"  // '\'
#define _7SEG_ASC5D     "\x0F"  // ']'
#define _7SEG_ASC5E     "\x23"  // '^'
#define _7SEG_ASC5F     "\x08"  // '_'
#define _7SEG_ASC60     "\x20"  // '`'
#define _7SEG_CH_a      "\x5F"
#define _7SEG_CH_b      "\x7c"
#define _7SEG_CH_c      "\x58"
#define _7SEG_CH_d      "\x5E"
#define _7SEG_CH_e      "\x7B"
#define _7SEG_CH_f      "\x71"
#define _7SEG_CH_g      "\x6F"
#define _7SEG_CH_h      "\x74"
#define _7SEG_CH_i      "\x04"
#define _7SEG_CH_j      "\x0E"
#define _7SEG_CH_k      "\x78"
#define _7SEG_CH_l      "\x38"
#define _7SEG_CH_m      "\x55"
#define _7SEG_CH_n      "\x54"
#define _7SEG_CH_o      "\x5C"
#define _7SEG_CH_p      "\x73"
#define _7SEG_CH_q      "\x67"
#define _7SEG_CH_r      "\x50"
#define _7SEG_CH_s      "\x6D"
#define _7SEG_CH_t      "\x78"
#define _7SEG_CH_u      "\x1C"
#define _7SEG_CH_v      "\x1C"
#define _7SEG_CH_w      "\x1C"
#define _7SEG_CH_x      "\x76"
#define _7SEG_CH_y      "\x6E"
#define _7SEG_CH_z      "\x5B"
#define _7SEG_ASC7B     "\x39"  // '{'
#define _7SEG_ASC7C     "\x06"  // '|'
#define _7SEG_ASC7D     "\x0F"  // '}'
#define _7SEG_ASC7E     "\x40"  // '~'
#define _7SEG_ASC7F     "\xFF"  // DEL

/****************************************************************************

  14-segment VFD display (Opticum HD 9600 Prima, Xsarius Combo HD)
  Base model  : FLEXLINE, ECOLINE
  Front Micom : Version 3.xx.xx

          0
     -----------
   |\     |     /|
   | \    |8   / |
  5|  \7  |   /9 | 1
   |   \  |  /   |
   | 6  \ | /  10|
     ---- * ----    Centre point is bit 13
   |    / | \    |
   | 12/  |  \11 |
  4|  /   |   \  | 2
   | /    |8   \ |
   |/     |     \|
     -----------
          3

 Data format : (7|6|5|4  3|2|1|0) (x|x|13|12  11|10|9|8)
 Bits marked with x should always be zero.

*****************************************************************************/

#define _14SEG_NULL         "\x00\x00"
#define _14SEG_ASC20        "\x00\x00"
#define _14SEG_ASC21        "\x00\x21"  // '!'
#define _14SEG_ASC22        "\x22\x00"  // '"'
#define _14SEG_ASC23        "\x40\x25"  // '#'
#define _14SEG_ASC24        "\x6d\x25"  // '$'
#define _14SEG_ASC25        "\x24\x32"  // '%'
#define _14SEG_ASC26        "\x79\x28"  // '&'
#define _14SEG_ASC27        "\x00\x02"  // '
#define _14SEG_ASC28        "\x00\x2a"  // '('
#define _14SEG_ASC29        "\x80\x30"  // ')'
#define _14SEG_ASC2A        "\xc0\x3e"  // '*'
#define _14SEG_ASC2B        "\x40\x25"  // '+'
#define _14SEG_ASC2C        "\x40\x10"  // ','
#define _14SEG_ASC2D        "\x40\x24"  // '-'
#define _14SEG_ASC2E        "\x04\x00"  // '.'
#define _14SEG_ASC2F        "\x00\x32"  // '/'
#define _14SEG_NUM_0        "\x3F\x20"  // '0'
#define _14SEG_NUM_1        "\x06\x02"  // '1'
#define _14SEG_NUM_2        "\x5B\x24"  // '2'
#define _14SEG_NUM_3        "\x4F\x24"  // '3
#define _14SEG_NUM_4        "\x66\x24"  // '4'
#define _14SEG_NUM_5        "\x69\x28"  // '5'
#define _14SEG_NUM_6        "\x7D\x24"  // '6'
#define _14SEG_NUM_7        "\x07\x00"  // '7'
#define _14SEG_NUM_8        "\x7F\x24"  // '8'
#define _14SEG_NUM_9        "\x6F\x24"  // '9'
#define _14SEG_ASC3A        "\x00\x01"  // ':'
#define _14SEG_ASC3B        "\x00\x01"  // ';'
#define _14SEG_ASC3C        "\x00\x0a"  // '<'
#define _14SEG_ASC3D        "\x44\x24"  // '='
#define _14SEG_ASC3E        "\x80\x10"  // '>'
#define _14SEG_ASC3F        "\x03\x34"  // '?'
#define _14SEG_ASC40        "\x7b\x24"  // '@'
#define _14SEG_CH_A         "\x77\x24"
#define _14SEG_CH_B         "\x0F\x25"
#define _14SEG_CH_C         "\x39\x00"
#define _14SEG_CH_D         "\x0F\x21"
#define _14SEG_CH_E         "\x79\x24"
#define _14SEG_CH_F         "\x71\x24"
#define _14SEG_CH_G         "\x3D\x24"
#define _14SEG_CH_H         "\x76\x24"
#define _14SEG_CH_I         "\x09\x21"
#define _14SEG_CH_J         "\x0E\x00"
#define _14SEG_CH_K         "\x70\x2A"
#define _14SEG_CH_L         "\x38\x00"
#define _14SEG_CH_M         "\xB6\x22"
#define _14SEG_CH_N         "\xB6\x28"
#define _14SEG_CH_O         "\x3F\x00"
#define _14SEG_CH_P         "\x73\x24"
#define _14SEG_CH_Q         "\x3F\x08"
#define _14SEG_CH_R         "\x73\x2C"
#define _14SEG_CH_S         "\x6D\x24"
#define _14SEG_CH_T         "\x01\x21"
#define _14SEG_CH_U         "\x3E\x00"
#define _14SEG_CH_V         "\x30\x32"
#define _14SEG_CH_W         "\x36\x38"
#define _14SEG_CH_X         "\x80\x3A"
#define _14SEG_CH_Y         "\x6E\x24"
#define _14SEG_CH_Z         "\x09\x32"
#define _14SEG_ASC5B        "\x39\x00"  // '['
#define _14SEG_ASC5C        "\x80\x28"  // '\'
#define _14SEG_ASC5D        "\x0f\x00"  // ']'
#define _14SEG_ASC5E        "\x00\x38"  // '^'
#define _14SEG_ASC5F        "\x08\x00"  // '_'
#define _14SEG_ASC60        "\x80\x00"  // '`'
#if defined(VFD_LOWER_CASE)
#define _14SEG_CH_a         "\x0c\x34"  // a
#define _14SEG_CH_b         "\x78\x28"  // b
#define _14SEG_CH_c         "\x58\x24"  // c
#define _14SEG_CH_d         "\x0e\x34"  // d
#define _14SEG_CH_e         "\x79\x22"  // e
#define _14SEG_CH_f         "\x71\x20"  // f
#define _14SEG_CH_g         "\x8f\x24"  // g
#define _14SEG_CH_h         "\x74\x24"  // h
#define _14SEG_CH_i         "\x00\x21"  // i
#define _14SEG_CH_j         "\x1e\x00"  // j
#define _14SEG_CH_k         "\x00\x2d"  // k
#define _14SEG_CH_l         "\x00\x21"  // l
#define _14SEG_CH_m         "\xB6\x22"  // m
#define _14SEG_CH_n         "\x54\x24"  // n
#define _14SEG_CH_o         "\x5c\x24"  // o
#define _14SEG_CH_p         "\x71\x22"  // p
#define _14SEG_CH_q         "\x87\x24"  // q
#define _14SEG_CH_r         "\x50\x24"  // r
#define _14SEG_CH_s         "\x6d\x24"  // s
#define _14SEG_CH_t         "\x78\x20"  // t
#define _14SEG_CH_u         "\x1c\x00"  // u
#define _14SEG_CH_v         "\x10\x10"  // v
#define _14SEG_CH_w         "\x1c\x01"  // w
#define _14SEG_CH_x         "\x80\x3a"  // x
#define _14SEG_CH_y         "\x8e\x24"  // y
#define _14SEG_CH_z         "\x09\x32"  // z
#else  // same as upper case
#define _14SEG_CH_a         _14SEG_CH_A
#define _14SEG_CH_b         _14SEG_CH_B
#define _14SEG_CH_c         _14SEG_CH_C
#define _14SEG_CH_d         _14SEG_CH_D
#define _14SEG_CH_e         _14SEG_CH_E
#define _14SEG_CH_f         _14SEG_CH_F
#define _14SEG_CH_g         _14SEG_CH_G
#define _14SEG_CH_h         _14SEG_CH_H
#define _14SEG_CH_i         _14SEG_CH_I
#define _14SEG_CH_j         _14SEG_CH_J
#define _14SEG_CH_k         _14SEG_CH_K
#define _14SEG_CH_l         _14SEG_CH_L
#define _14SEG_CH_m         _14SEG_CH_M
#define _14SEG_CH_n         _14SEG_CH_N
#define _14SEG_CH_o         _14SEG_CH_O
#define _14SEG_CH_p         _14SEG_CH_P
#define _14SEG_CH_q         _14SEG_CH_Q
#define _14SEG_CH_r         _14SEG_CH_R
#define _14SEG_CH_s         _14SEG_CH_S
#define _14SEG_CH_t         _14SEG_CH_T
#define _14SEG_CH_u         _14SEG_CH_U
#define _14SEG_CH_v         _14SEG_CH_V
#define _14SEG_CH_w         _14SEG_CH_W
#define _14SEG_CH_x         _14SEG_CH_X
#define _14SEG_CH_y         _14SEG_CH_Y
#define _14SEG_CH_z         _14SEG_CH_Z
#endif
#define _14SEG_ASC7B        "\x00\x2a"  // '{'
#define _14SEG_ASC7C        "\x00\x01"  // '|'
#define _14SEG_ASC7D        "\x80\x30"  // '}'
#define _14SEG_ASC7E        "\x40\x24"  // '~'
#define _14SEG_ASC7F        "\xff\x3f"  // DEL

#if defined(_BUYER_SATCOM_RUSSIA) \
 || defined(_BUYER_SOGNO_GERMANY)
#define _14SEG_RUS_a        "\x77\x24"
#define _14SEG_RUS_b        "\x7d\x24"
#define _14SEG_RUS_v        "\x79\x2A"
#define _14SEG_RUS_g        "\x31\x00"
#define _14SEG_RUS_d        "\x0F\x21"
#define _14SEG_RUS_ye       "\x79\x24"
#define _14SEG_RUS_yo       "\x79\x24"
#define _14SEG_RUS_zh       "\xC0\x3F"
#define _14SEG_RUS_z        "\x09\x2A"
#define _14SEG_RUS_i        "\x36\x32"
#define _14SEG_RUS_ekra     "\x36\x32"
#define _14SEG_RUS_k        "\x70\x2A"
#define _14SEG_RUS_el       "\x06\x32"
#define _14SEG_RUS_m        "\xB6\x22"
#define _14SEG_RUS_n        "\x76\x24"
#define _14SEG_RUS_o        "\x3F\x00"
#define _14SEG_RUS_p        "\x37\x00"
#define _14SEG_RUS_r        "\x73\x24"
#define _14SEG_RUS_s        "\x39\x00"
#define _14SEG_RUS_t        "\x01\x21"
#define _14SEG_RUS_u        "\x6E\x24"
#define _14SEG_RUS_f        "\x3F\x21"
#define _14SEG_RUS_kh       "\x80\x3A"
#define _14SEG_RUS_ts       "\x38\x21"
#define _14SEG_RUS_ch       "\x66\x24"
#define _14SEG_RUS_sh       "\x3E\x21"
#define _14SEG_RUS_shch     "\x3E\x21"
#define _14SEG_RUS_tvyor    "\x0D\x25"
#define _14SEG_RUS_y        "\x7E\x21"
#define _14SEG_RUS_myah     "\x0C\x25"
#define _14SEG_RUS_eobeo    "\x0F\x24"
#define _14SEG_RUS_yu       "\x76\x2A"
#define _14SEG_RUS_ya       "\x27\x34"
#endif

#define _14SEG_SPECIAL_1    "\x10\x00"
#define _14SEG_SPECIAL_2    "\x20\x00"
#define _14SEG_SPECIAL_3    "\x01\x00"
#define _14SEG_SPECIAL_4    "\x02\x00"
#define _14SEG_SPECIAL_5    "\x04\x00"
#define _14SEG_SPECIAL_6    "\x08\x00"
#define _14SEG_SPECIAL_7    "\x40\x24"

#endif  // __CN_MICOM_CHAR_H__
// vim:ts=4
