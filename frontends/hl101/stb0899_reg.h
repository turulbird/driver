/*
 * STB0899 Multistandard Frontend driver
 * Copyright (C) Manu Abraham (abraham.manu@gmail.com)
 *
 * Copyright (C) ST Microelectronics
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __STB0899_REG_H
#define __STB0899_REG_H

/* S1 */
#define STB0899_DEV_ID                     0xf000
#define STB0899_CHIP_ID                    (0x0f << 4)
#define STB0899_OFFST_CHIP_ID              4
#define STB0899_WIDTH_CHIP_ID              4
#define STB0899_CHIP_REL                   (0x0f << 0)
#define STB0899_OFFST_CHIP_REL             0
#define STB0899_WIDTH_CHIP_REL             4

#define STB0899_DEMOD                      0xf40e
#define STB0899_MODECOEFF                  (0x01 << 0)
#define STB0899_OFFST_MODECOEFF            0
#define STB0899_WIDTH_MODECOEFF            1

#define STB0899_RCOMPC                     0xf410
#define STB0899_AGC1CN                     0xf412
#define STB0899_AGC1REF                    0xf413
#define STB0899_RTC                        0xf417
#define STB0899_TMGCFG                     0xf418
#define STB0899_AGC2REF                    0xf419
#define STB0899_TLSR                       0xf41a

#define STB0899_CFD                        0xf41b
#define STB0899_CFD_ON                     (0x01 << 7)
#define STB0899_OFFST_CFD_ON               7
#define STB0899_WIDTH_CFD_ON               1

#define STB0899_ACLC                       0xf41c

#define STB0899_BCLC                       0xf41d
#define STB0899_OFFST_ALGO                 6
#define STB0899_WIDTH_ALGO_QPSK2           2
#define STB0899_ALGO_QPSK2                 (2 << 6)
#define STB0899_ALGO_QPSK1                 (1 << 6)
#define STB0899_ALGO_BPSK                  (0 << 6)
#define STB0899_OFFST_BETA                 0
#define STB0899_WIDTH_BETA                 6

#define STB0899_EQON                       0xf41e
#define STB0899_LDT                        0xf41f
#define STB0899_LDT2                       0xf420
#define STB0899_EQUALREF                   0xf425
#define STB0899_TMGRAMP                    0xf426
#define STB0899_TMGTHD                     0xf427
#define STB0899_IDCCOMP                    0xf428
#define STB0899_QDCCOMP                    0xf429
#define STB0899_POWERI                     0xf42a
#define STB0899_POWERQ                     0xf42b
#define STB0899_RCOMP                      0xf42c

#define STB0899_AGCIQIN                    0xf42e
#define STB0899_AGCIQVALUE                 (0xff << 0)
#define STB0899_OFFST_AGCIQVALUE           0
#define STB0899_WIDTH_AGCIQVALUE           8

#define STB0899_AGC2I1                     0xf436
#define STB0899_AGC2I2                     0xf437

#define STB0899_TLIR                       0xf438
#define STB0899_TLIR_TMG_LOCK_IND          (0xff << 0)
#define STB0899_OFFST_TLIR_TMG_LOCK_IND    0
#define STB0899_WIDTH_TLIR_TMG_LOCK_IND    8

#define STB0899_RTF                        0xf439
#define STB0899_RTF_TIMING_LOOP_FREQ       (0xff << 0)
#define STB0899_OFFST_RTF_TIMING_LOOP_FREQ 0
#define STB0899_WIDTH_RTF_TIMING_LOOP_FREQ 8

#define STB0899_DSTATUS                    0xf43a
#define STB0899_CARRIER_FOUND              (0x01 << 7)
#define STB0899_OFFST_CARRIER_FOUND        7
#define STB0899_WIDTH_CARRIER_FOUND        1
#define STB0899_TMG_LOCK                   (0x01 << 6)
#define STB0899_OFFST_TMG_LOCK             6
#define STB0899_WIDTH_TMG_LOCK             1
#define STB0899_DEMOD_LOCK                 (0x01 << 5)
#define STB0899_OFFST_DEMOD_LOCK           5
#define STB0899_WIDTH_DEMOD_LOCK           1
#define STB0899_TMG_AUTO                   (0x01 << 4)
#define STB0899_OFFST_TMG_AUTO             4
#define STB0899_WIDTH_TMG_AUTO             1
#define STB0899_END_MAIN                   (0x01 << 3)
#define STB0899_OFFST_END_MAIN             3
#define STB0899_WIDTH_END_MAIN             1

#define STB0899_LDI                        0xf43b
#define STB0899_OFFST_LDI                  0
#define STB0899_WIDTH_LDI                  8

#define STB0899_CFRM                       0xf43e
#define STB0899_OFFST_CFRM                 0
#define STB0899_WIDTH_CFRM                 8

#define STB0899_CFRL                       0xf43f
#define STB0899_OFFST_CFRL                 0
#define STB0899_WIDTH_CFRL                 8

#define STB0899_NIRM                       0xf440
#define STB0899_OFFST_NIRM                 0
#define STB0899_WIDTH_NIRM                 8

#define STB0899_NIRL                       0xf441
#define STB0899_OFFST_NIRL                 0
#define STB0899_WIDTH_NIRL                 8

#define STB0899_ISYMB                      0xf444
#define STB0899_QSYMB                      0xf445

#define STB0899_SFRH                       0xf446
#define STB0899_OFFST_SFRH                 0
#define STB0899_WIDTH_SFRH                 8

#define STB0899_SFRM                       0xf447
#define STB0899_OFFST_SFRM                 0
#define STB0899_WIDTH_SFRM                 8

#define STB0899_SFRL                       0xf448
#define STB0899_OFFST_SFRL                 4
#define STB0899_WIDTH_SFRL                 4

#define STB0899_SFRUPH                     0xf44c
#define STB0899_SFRUPM                     0xf44d
#define STB0899_SFRUPL                     0xf44e

#define STB0899_EQUAI1                     0xf4e0
#define STB0899_EQUAQ1                     0xf4e1
#define STB0899_EQUAI2                     0xf4e2
#define STB0899_EQUAQ2                     0xf4e3
#define STB0899_EQUAI3                     0xf4e4
#define STB0899_EQUAQ3                     0xf4e5
#define STB0899_EQUAI4                     0xf4e6
#define STB0899_EQUAQ4                     0xf4e7
#define STB0899_EQUAI5                     0xf4e8
#define STB0899_EQUAQ5                     0xf4e9

#define STB0899_DSTATUS2                   0xf50c
#define STB0899_DS2_TMG_AUTOSRCH           (0x01 << 7)
#define STB8999_OFFST_DS2_TMG_AUTOSRCH     7
#define STB0899_WIDTH_DS2_TMG_AUTOSRCH     1
#define STB0899_DS2_END_MAINLOOP           (0x01 << 6)
#define STB0899_OFFST_DS2_END_MAINLOOP     6
#define STB0899_WIDTH_DS2_END_MAINLOOP     1
#define STB0899_DS2_CFSYNC                 (0x01 << 5)
#define STB0899_OFFST_DS2_CFSYNC           5
#define STB0899_WIDTH_DS2_CFSYNC           1
#define STB0899_DS2_TMGLOCK                (0x01 << 4)
#define STB0899_OFFST_DS2_TMGLOCK          4
#define STB0899_WIDTH_DS2_TMGLOCK          1
#define STB0899_DS2_DEMODWAIT              (0x01 << 3)
#define STB0899_OFFST_DS2_DEMODWAIT        3
#define STB0899_WIDTH_DS2_DEMODWAIT        1
#define STB0899_DS2_FECON                  (0x01 << 1)
#define STB0899_OFFST_DS2_FECON            1
#define STB0899_WIDTH_DS2_FECON            1

/* S1 FEC */
#define STB0899_VSTATUS                    0xf50d
#define STB0899_VSTATUS_VITERBI_ON         (0x01 << 7)
#define STB0899_OFFST_VSTATUS_VITERBI_ON   7
#define STB0899_WIDTH_VSTATUS_VITERBI_ON   1
#define STB0899_VSTATUS_END_LOOPVIT        (0x01 << 6)
#define STB0899_OFFST_VSTATUS_END_LOOPVIT  6
#define STB0899_WIDTH_VSTATUS_END_LOOPVIT  1
#define STB0899_VSTATUS_PRFVIT             (0x01 << 4)
#define STB0899_OFFST_VSTATUS_PRFVIT       4
#define STB0899_WIDTH_VSTATUS_PRFVIT       1
#define STB0899_VSTATUS_LOCKEDVIT          (0x01 << 3)
#define STB0899_OFFST_VSTATUS_LOCKEDVIT    3
#define STB0899_WIDTH_VSTATUS_LOCKEDVIT    1

#define STB0899_VERROR                     0xf50f

#define STB0899_IQSWAP                     0xf523
#define STB0899_SYM                        (0x01 << 3)
#define STB0899_OFFST_SYM                  3
#define STB0899_WIDTH_SYM                  1

#define STB0899_FECAUTO1                   0xf530
#define STB0899_DSSSRCH                    (0x01 << 3)
#define STB0899_OFFST_DSSSRCH              3
#define STB0899_WIDTH_DSSSRCH              1
#define STB0899_SYMSRCH                    (0x01 << 2)
#define STB0899_OFFST_SYMSRCH              2
#define STB0899_WIDTH_SYMSRCH              1
#define STB0899_QPSKSRCH                   (0x01 << 1)
#define STB0899_OFFST_QPSKSRCH             1
#define STB0899_WIDTH_QPSKSRCH             1
#define STB0899_BPSKSRCH                   (0x01 << 0)
#define STB0899_OFFST_BPSKSRCH             0
#define STB0899_WIDTH_BPSKSRCH             1

#define STB0899_FECM                       0xf533
#define STB0899_FECM_NOT_DVB               (0x01 << 7)
#define STB0899_OFFST_FECM_NOT_DVB         7
#define STB0899_WIDTH_FECM_NOT_DVB         1
#define STB0899_FECM_RSVD1                 (0x07 << 4)
#define STB0899_OFFST_FECM_RSVD1           4
#define STB0899_WIDTH_FECM_RSVD1           3
#define STB0899_FECM_VITERBI_ON            (0x01 << 3)
#define STB0899_OFFST_FECM_VITERBI_ON      3
#define STB0899_WIDTH_FECM_VITERBI_ON      1
#define STB0899_FECM_RSVD0                 (0x01 << 2)
#define STB0899_OFFST_FECM_RSVD0           2
#define STB0899_WIDTH_FECM_RSVD0           1
#define STB0899_FECM_SYNCDIS               (0x01 << 1)
#define STB0899_OFFST_FECM_SYNCDIS         1
#define STB0899_WIDTH_FECM_SYNCDIS         1
#define STB0899_FECM_SYMI                  (0x01 << 0)
#define STB0899_OFFST_FECM_SYMI            0
#define STB0899_WIDTH_FECM_SYMI            1

#define STB0899_VTH12                      0xf534
#define STB0899_VTH23                      0xf535
#define STB0899_VTH34                      0xf536
#define STB0899_VTH56                      0xf537
#define STB0899_VTH67                      0xf538
#define STB0899_VTH78                      0xf539

#define STB0899_PRVIT                      0xf53c
#define STB0899_PR_7_8                     (0x01 << 5)
#define STB0899_OFFST_PR_7_8               5
#define STB0899_WIDTH_PR_7_8               1
#define STB0899_PR_6_7                     (0x01 << 4)
#define STB0899_OFFST_PR_6_7               4
#define STB0899_WIDTH_PR_6_7               1
#define STB0899_PR_5_6                     (0x01 << 3)
#define STB0899_OFFST_PR_5_6               3
#define STB0899_WIDTH_PR_5_6               1
#define STB0899_PR_3_4                     (0x01 << 2)
#define STB0899_OFFST_PR_3_4               2
#define STB0899_WIDTH_PR_3_4               1
#define STB0899_PR_2_3                     (0x01 << 1)
#define STB0899_OFFST_PR_2_3               1
#define STB0899_WIDTH_PR_2_3               1
#define STB0899_PR_1_2                     (0x01 << 0)
#define STB0899_OFFST_PR_1_2               0
#define STB0899_WIDTH_PR_1_2               1

#define STB0899_VITSYNC                    0xf53d
#define STB0899_AM                         (0x01 << 7)
#define STB0899_OFFST_AM                   7
#define STB0899_WIDTH_AM                   1
#define STB0899_FREEZE                     (0x01 << 6)
#define STB0899_OFFST_FREEZE               6
#define STB0899_WIDTH_FREEZE               1
#define STB0899_SN_65536                   (0x03 << 4)
#define STB0899_OFFST_SN_65536             4
#define STB0899_WIDTH_SN_65536             2
#define STB0899_SN_16384                   (0x01 << 5)
#define STB0899_OFFST_SN_16384             5
#define STB0899_WIDTH_SN_16384             1
#define STB0899_SN_4096                    (0x01 << 4)
#define STB0899_OFFST_SN_4096              4
#define STB0899_WIDTH_SN_4096              1
#define STB0899_SN_1024                    (0x00 << 4)
#define STB0899_OFFST_SN_1024              4
#define STB0899_WIDTH_SN_1024              0
#define STB0899_TO_128                     (0x03 << 2)
#define STB0899_OFFST_TO_128               2
#define STB0899_WIDTH_TO_128               2
#define STB0899_TO_64                      (0x01 << 3)
#define STB0899_OFFST_TO_64                3
#define STB0899_WIDTH_TO_64                1
#define STB0899_TO_32                      (0x01 << 2)
#define STB0899_OFFST_TO_32                2
#define STB0899_WIDTH_TO_32                1
#define STB0899_TO_16                      (0x00 << 2)
#define STB0899_OFFST_TO_16                2
#define STB0899_WIDTH_TO_16                0
#define STB0899_HYST_128                   (0x03 << 1)
#define STB0899_OFFST_HYST_128             1
#define STB0899_WIDTH_HYST_128             2
#define STB0899_HYST_64                    (0x01 << 1)
#define STB0899_OFFST_HYST_64              1
#define STB0899_WIDTH_HYST_64              1
#define STB0899_HYST_32                    (0x01 << 0)
#define STB0899_OFFST_HYST_32              0
#define STB0899_WIDTH_HYST_32              1
#define STB0899_HYST_16                    (0x00 << 0)
#define STB0899_OFFST_HYST_16              0
#define STB0899_WIDTH_HYST_16              0

#define STB0899_RSULC                      0xf548
#define STB0899_ULDIL_ON                   (0x01 << 7)
#define STB0899_OFFST_ULDIL_ON             7
#define STB0899_WIDTH_ULDIL_ON             1
#define STB0899_ULAUTO_ON                  (0x01 << 6)
#define STB0899_OFFST_ULAUTO_ON            6
#define STB0899_WIDTH_ULAUTO_ON            1
#define STB0899_ULRS_ON                    (0x01 << 5)
#define STB0899_OFFST_ULRS_ON              5
#define STB0899_WIDTH_ULRS_ON              1
#define STB0899_ULDESCRAM_ON               (0x01 << 4)
#define STB0899_OFFST_ULDESCRAM_ON         4
#define STB0899_WIDTH_ULDESCRAM_ON         1
#define STB0899_UL_DISABLE                 (0x01 << 2)
#define STB0899_OFFST_UL_DISABLE           2
#define STB0899_WIDTH_UL_DISABLE           1
#define STB0899_NOFTHRESHOLD               (0x01 << 0)
#define STB0899_OFFST_NOFTHRESHOLD         0
#define STB0899_WIDTH_NOFTHRESHOLD         1

#define STB0899_RSLLC                      0xf54a
#define STB0899_DEMAPVIT                   0xf583
#define STB0899_DEMAPVIT_RSVD              (0x01 << 7)
#define STB0899_OFFST_DEMAPVIT_RSVD        7
#define STB0899_WIDTH_DEMAPVIT_RSVD        1
#define STB0899_DEMAPVIT_KDIVIDER          (0x7f << 0)
#define STB0899_OFFST_DEMAPVIT_KDIVIDER    0
#define STB0899_WIDTH_DEMAPVIT_KDIVIDER    7

#define STB0899_PLPARM                     0xf58c
#define STB0899_VITMAPPING                 (0x07 << 5)
#define STB0899_OFFST_VITMAPPING           5
#define STB0899_WIDTH_VITMAPPING           3
#define STB0899_VITMAPPING_BPSK            (0x01 << 5)
#define STB0899_OFFST_VITMAPPING_BPSK      5
#define STB0899_WIDTH_VITMAPPING_BPSK      1
#define STB0899_VITMAPPING_QPSK            (0x00 << 5)
#define STB0899_OFFST_VITMAPPING_QPSK      5
#define STB0899_WIDTH_VITMAPPING_QPSK      0
#define STB0899_VITCURPUN                  (0x1f << 0)
#define STB0899_OFFST_VITCURPUN            0
#define STB0899_WIDTH_VITCURPUN            5
#define STB0899_VITCURPUN_1_2              (0x0d << 0)
#define STB0899_VITCURPUN_2_3              (0x12 << 0)
#define STB0899_VITCURPUN_3_4              (0x15 << 0)
#define STB0899_VITCURPUN_5_6              (0x18 << 0)
#define STB0899_VITCURPUN_6_7              (0x19 << 0)
#define STB0899_VITCURPUN_7_8              (0x1a << 0)

/* S2 DEMOD */
#define STB0899_OFF0_DMD_STATUS            0xf300
#define STB0899_BASE_DMD_STATUS            0x00000000
#define STB0899_IF_AGC_LOCK                (0x01 << 8)
#define STB0899_OFFST_IF_AGC_LOCK          0
#define STB0899_WIDTH_IF_AGC_LOCK          1

#define STB0899_OFF0_CRL_FREQ              0xf304
#define STB0899_BASE_CRL_FREQ              0x00000000
#define STB0899_CARR_FREQ                  (0x3fffffff << 0)
#define STB0899_OFFST_CARR_FREQ            0
#define STB0899_WIDTH_CARR_FREQ            30

#define STB0899_OFF0_BTR_FREQ              0xf308
#define STB0899_BASE_BTR_FREQ              0x00000000
#define STB0899_BTR_FREQ                   (0xfffffff << 0)
#define STB0899_OFFST_BTR_FREQ             0
#define STB0899_WIDTH_BTR_FREQ             28

#define STB0899_OFF0_IF_AGC_GAIN           0xf30c
#define STB0899_BASE_IF_AGC_GAIN           0x00000000
#define STB0899_IF_AGC_GAIN                (0x3fff < 0)
#define STB0899_OFFST_IF_AGC_GAIN          0
#define STB0899_WIDTH_IF_AGC_GAIN          14

#define STB0899_OFF0_BB_AGC_GAIN           0xf310
#define STB0899_BASE_BB_AGC_GAIN           0x00000000
#define STB0899_BB_AGC_GAIN                (0x3fff < 0)
#define STB0899_OFFST_BB_AGC_GAIN          0
#define STB0899_WIDTH_BB_AGC_GAIN          14

#define STB0899_OFF0_DC_OFFSET             0xf314
#define STB0899_BASE_DC_OFFSET             0x00000000
#define STB0899_I                          (0xff < 8)
#define STB0899_OFFST_I                    8
#define STB0899_WIDTH_I                    8
#define STB0899_Q                          (0xff < 0)
#define STB0899_OFFST_Q                    8
#define STB0899_WIDTH_Q                    8

#define STB0899_OFF0_DMD_CNTRL             0xf31c
#define STB0899_BASE_DMD_CNTRL             0x00000000
#define STB0899_ADC0_PINS1IN               (0x01 << 6)
#define STB0899_OFFST_ADC0_PINS1IN         6
#define STB0899_WIDTH_ADC0_PINS1IN         1
#define STB0899_IN2COMP1_OFFBIN0           (0x01 << 3)
#define STB0899_OFFST_IN2COMP1_OFFBIN0     3
#define STB0899_WIDTH_IN2COMP1_OFFBIN0     1
#define STB0899_DC_COMP                    (0x01 << 2)
#define STB0899_OFFST_DC_COMP              2
#define STB0899_WIDTH_DC_COMP              1
#define STB0899_MODMODE                    (0x03 << 0)
#define STB0899_OFFST_MODMODE              0
#define STB0899_WIDTH_MODMODE              2

#define STB0899_OFF0_IF_AGC_CNTRL          0xf320
#define STB0899_BASE_IF_AGC_CNTRL          0x00000000
#define STB0899_IF_GAIN_INIT               (0x3fff << 13)
#define STB0899_OFFST_IF_GAIN_INIT         13
#define STB0899_WIDTH_IF_GAIN_INIT         14
#define STB0899_IF_GAIN_SENSE              (0x01 << 12)
#define STB0899_OFFST_IF_GAIN_SENSE        12
#define STB0899_WIDTH_IF_GAIN_SENSE        1
#define STB0899_IF_LOOP_GAIN               (0x0f << 8)
#define STB0899_OFFST_IF_LOOP_GAIN         8
#define STB0899_WIDTH_IF_LOOP_GAIN         4
#define STB0899_IF_LD_GAIN_INIT            (0x01 << 7)
#define STB0899_OFFST_IF_LD_GAIN_INIT      7
#define STB0899_WIDTH_IF_LD_GAIN_INIT      1
#define STB0899_IF_AGC_REF                 (0x7f << 0)
#define STB0899_OFFST_IF_AGC_REF           0
#define STB0899_WIDTH_IF_AGC_REF           7

#define STB0899_OFF0_BB_AGC_CNTRL          0xf324
#define STB0899_BASE_BB_AGC_CNTRL          0x00000000
#define STB0899_BB_GAIN_INIT               (0x3fff << 12)
#define STB0899_OFFST_BB_GAIN_INIT         12
#define STB0899_WIDTH_BB_GAIN_INIT         14
#define STB0899_BB_LOOP_GAIN               (0x0f << 8)
#define STB0899_OFFST_BB_LOOP_GAIN         8
#define STB0899_WIDTH_BB_LOOP_GAIN         4
#define STB0899_BB_LD_GAIN_INIT            (0x01 << 7)
#define STB0899_OFFST_BB_LD_GAIN_INIT      7
#define STB0899_WIDTH_BB_LD_GAIN_INIT      1
#define STB0899_BB_AGC_REF                 (0x7f << 0)
#define STB0899_OFFST_BB_AGC_REF           0
#define STB0899_WIDTH_BB_AGC_REF           7

#define STB0899_OFF0_CRL_CNTRL             0xf328
#define STB0899_BASE_CRL_CNTRL             0x00000000
#define STB0899_CRL_LOCK_CLEAR             (0x01 << 5)
#define STB0899_OFFST_CRL_LOCK_CLEAR       5
#define STB0899_WIDTH_CRL_LOCK_CLEAR       1
#define STB0899_CRL_SWPR_CLEAR             (0x01 << 4)
#define STB0899_OFFST_CRL_SWPR_CLEAR       4
#define STB0899_WIDTH_CRL_SWPR_CLEAR       1
#define STB0899_CRL_SWP_ENA                (0x01 << 3)
#define STB0899_OFFST_CRL_SWP_ENA          3
#define STB0899_WIDTH_CRL_SWP_ENA          1
#define STB0899_CRL_DET_SEL                (0x01 << 2)
#define STB0899_OFFST_CRL_DET_SEL          2
#define STB0899_WIDTH_CRL_DET_SEL          1
#define STB0899_CRL_SENSE                  (0x01 << 1)
#define STB0899_OFFST_CRL_SENSE            1
#define STB0899_WIDTH_CRL_SENSE            1
#define STB0899_CRL_PHSERR_CLEAR           (0x01 << 0)
#define STB0899_OFFST_CRL_PHSERR_CLEAR     0
#define STB0899_WIDTH_CRL_PHSERR_CLEAR     1

#define STB0899_OFF0_CRL_PHS_INIT          0xf32c
#define STB0899_BASE_CRL_PHS_INIT          0x00000000
#define STB0899_CRL_PHS_INIT_31            (0x1 << 30)
#define STB0899_OFFST_CRL_PHS_INIT_31      30
#define STB0899_WIDTH_CRL_PHS_INIT_31      1
#define STB0899_CRL_LD_INIT_PHASE          (0x1 << 24)
#define STB0899_OFFST_CRL_LD_INIT_PHASE    24
#define STB0899_WIDTH_CRL_LD_INIT_PHASE    1
#define STB0899_CRL_INIT_PHASE             (0xffffff << 0)
#define STB0899_OFFST_CRL_INIT_PHASE       0
#define STB0899_WIDTH_CRL_INIT_PHASE       24

#define STB0899_OFF0_CRL_FREQ_INIT         0xf330
#define STB0899_BASE_CRL_FREQ_INIT         0x00000000
#define STB0899_CRL_FREQ_INIT_31           (0x1 << 30)
#define STB0899_OFFST_CRL_FREQ_INIT_31     30
#define STB0899_WIDTH_CRL_FREQ_INIT_31     1
#define STB0899_CRL_LD_FREQ_INIT           (0x1 << 24)
#define STB0899_OFFST_CRL_LD_FREQ_INIT     24
#define STB0899_WIDTH_CRL_LD_FREQ_INIT     1
#define STB0899_CRL_FREQ_INIT              (0xffffff << 0)
#define STB0899_OFFST_CRL_FREQ_INIT        0
#define STB0899_WIDTH_CRL_FREQ_INIT        24

#define STB0899_OFF0_CRL_LOOP_GAIN         0xf334
#define STB0899_BASE_CRL_LOOP_GAIN         0x00000000
#define STB0899_KCRL2_RSHFT                (0xf << 16)
#define STB0899_OFFST_KCRL2_RSHFT          16
#define STB0899_WIDTH_KCRL2_RSHFT          4
#define STB0899_KCRL1                      (0xf << 12)
#define STB0899_OFFST_KCRL1                12
#define STB0899_WIDTH_KCRL1                4
#define STB0899_KCRL1_RSHFT                (0xf << 8)
#define STB0899_OFFST_KCRL1_RSHFT          8
#define STB0899_WIDTH_KCRL1_RSHFT          4
#define STB0899_KCRL0                      (0xf << 4)
#define STB0899_OFFST_KCRL0                4
#define STB0899_WIDTH_KCRL0                4
#define STB0899_KCRL0_RSHFT                (0xf << 0)
#define STB0899_OFFST_KCRL0_RSHFT          0
#define STB0899_WIDTH_KCRL0_RSHFT          4

#define STB0899_OFF0_CRL_NOM_FREQ          0xf338
#define STB0899_BASE_CRL_NOM_FREQ          0x00000000
#define STB0899_CRL_NOM_FREQ               (0x3fffffff << 0)
#define STB0899_OFFST_CRL_NOM_FREQ         0
#define STB0899_WIDTH_CRL_NOM_FREQ         30

#define STB0899_OFF0_CRL_SWP_RATE          0xf33c
#define STB0899_BASE_CRL_SWP_RATE          0x00000000
#define STB0899_CRL_SWP_RATE               (0x3fffffff << 0)
#define STB0899_OFFST_CRL_SWP_RATE         0
#define STB0899_WIDTH_CRL_SWP_RATE         30

#define STB0899_OFF0_CRL_MAX_SWP           0xf340
#define STB0899_BASE_CRL_MAX_SWP           0x00000000
#define STB0899_CRL_MAX_SWP                (0x3fffffff << 0)
#define STB0899_OFFST_CRL_MAX_SWP          0
#define STB0899_WIDTH_CRL_MAX_SWP          30

#define STB0899_OFF0_CRL_LK_CNTRL          0xf344
#define STB0899_BASE_CRL_LK_CNTRL          0x00000000

#define STB0899_OFF0_DECIM_CNTRL           0xf348
#define STB0899_BASE_DECIM_CNTRL           0x00000000
#define STB0899_BAND_LIMIT_B               (0x01 << 5)
#define STB0899_OFFST_BAND_LIMIT_B         5
#define STB0899_WIDTH_BAND_LIMIT_B         1
#define STB0899_WIN_SEL                    (0x03 << 3)
#define STB0899_OFFST_WIN_SEL              3
#define STB0899_WIDTH_WIN_SEL              2
#define STB0899_DECIM_RATE                 (0x07 << 0)
#define STB0899_OFFST_DECIM_RATE           0
#define STB0899_WIDTH_DECIM_RATE           3

#define STB0899_OFF0_BTR_CNTRL             0xf34c
#define STB0899_BASE_BTR_CNTRL             0x00000000
#define STB0899_BTR_FREQ_CORR              (0x7ff << 4)
#define STB0899_OFFST_BTR_FREQ_CORR        4
#define STB0899_WIDTH_BTR_FREQ_CORR        11
#define STB0899_BTR_CLR_LOCK               (0x01 << 3)
#define STB0899_OFFST_BTR_CLR_LOCK         3
#define STB0899_WIDTH_BTR_CLR_LOCK         1
#define STB0899_BTR_SENSE                  (0x01 << 2)
#define STB0899_OFFST_BTR_SENSE            2
#define STB0899_WIDTH_BTR_SENSE            1
#define STB0899_BTR_ERR_ENA                (0x01 << 1)
#define STB0899_OFFST_BTR_ERR_ENA          1
#define STB0899_WIDTH_BTR_ERR_ENA          1
#define STB0899_INTRP_PHS_SENSE            (0x01 << 0)
#define STB0899_OFFST_INTRP_PHS_SENSE      0
#define STB0899_WIDTH_INTRP_PHS_SENSE      1

#define STB0899_OFF0_BTR_LOOP_GAIN         0xf350
#define STB0899_BASE_BTR_LOOP_GAIN         0x00000000
#define STB0899_KBTR2_RSHFT                (0x0f << 16)
#define STB0899_OFFST_KBTR2_RSHFT          16
#define STB0899_WIDTH_KBTR2_RSHFT          4
#define STB0899_KBTR1                      (0x0f << 12)
#define STB0899_OFFST_KBTR1                12
#define STB0899_WIDTH_KBTR1                4
#define STB0899_KBTR1_RSHFT                (0x0f << 8)
#define STB0899_OFFST_KBTR1_RSHFT          8
#define STB0899_WIDTH_KBTR1_RSHFT          4
#define STB0899_KBTR0                      (0x0f << 4)
#define STB0899_OFFST_KBTR0                4
#define STB0899_WIDTH_KBTR0                4
#define STB0899_KBTR0_RSHFT                (0x0f << 0)
#define STB0899_OFFST_KBTR0_RSHFT          0
#define STB0899_WIDTH_KBTR0_RSHFT          4

#define STB0899_OFF0_BTR_PHS_INIT          0xf354
#define STB0899_BASE_BTR_PHS_INIT          0x00000000
#define STB0899_BTR_LD_PHASE_INIT          (0x01 << 28)
#define STB0899_OFFST_BTR_LD_PHASE_INIT    28
#define STB0899_WIDTH_BTR_LD_PHASE_INIT    1
#define STB0899_BTR_INIT_PHASE             (0xfffffff << 0)
#define STB0899_OFFST_BTR_INIT_PHASE       0
#define STB0899_WIDTH_BTR_INIT_PHASE       28

#define STB0899_OFF0_BTR_FREQ_INIT         0xf358
#define STB0899_BASE_BTR_FREQ_INIT         0x00000000
#define STB0899_BTR_LD_FREQ_INIT           (1 << 28)
#define STB0899_OFFST_BTR_LD_FREQ_INIT     28
#define STB0899_WIDTH_BTR_LD_FREQ_INIT     1
#define STB0899_BTR_FREQ_INIT              (0xfffffff << 0)
#define STB0899_OFFST_BTR_FREQ_INIT        0
#define STB0899_WIDTH_BTR_FREQ_INIT        28

#define STB0899_OFF0_BTR_NOM_FREQ          0xf35c
#define STB0899_BASE_BTR_NOM_FREQ          0x00000000
#define STB0899_BTR_NOM_FREQ               (0xfffffff << 0)
#define STB0899_OFFST_BTR_NOM_FREQ         0
#define STB0899_WIDTH_BTR_NOM_FREQ         28

#define STB0899_OFF0_BTR_LK_CNTRL          0xf360
#define STB0899_BASE_BTR_LK_CNTRL          0x00000000
#define STB0899_BTR_MIN_ENERGY             (0x0f << 24)
#define STB0899_OFFST_BTR_MIN_ENERGY       24
#define STB0899_WIDTH_BTR_MIN_ENERGY       4
#define STB0899_BTR_LOCK_TH_LO             (0xff << 16)
#define STB0899_OFFST_BTR_LOCK_TH_LO       16
#define STB0899_WIDTH_BTR_LOCK_TH_LO       8
#define STB0899_BTR_LOCK_TH_HI             (0xff << 8)
#define STB0899_OFFST_BTR_LOCK_TH_HI       8
#define STB0899_WIDTH_BTR_LOCK_TH_HI       8
#define STB0899_BTR_LOCK_GAIN              (0x03 << 6)
#define STB0899_OFFST_BTR_LOCK_GAIN        6
#define STB0899_WIDTH_BTR_LOCK_GAIN        2
#define STB0899_BTR_LOCK_LEAK              (0x3f << 0)
#define STB0899_OFFST_BTR_LOCK_LEAK        0
#define STB0899_WIDTH_BTR_LOCK_LEAK        6

#define STB0899_OFF0_DECN_CNTRL            0xf364
#define STB0899_BASE_DECN_CNTRL            0x00000000

#define STB0899_OFF0_TP_CNTRL              0xf368
#define STB0899_BASE_TP_CNTRL              0x00000000

#define STB0899_OFF0_TP_BUF_STATUS         0xf36c
#define STB0899_BASE_TP_BUF_STATUS         0x00000000
#define STB0899_TP_BUFFER_FULL             (1 << 0)

#define STB0899_OFF0_DC_ESTIM              0xf37c
#define STB0899_BASE_DC_ESTIM              0x0000
#define STB0899_I_DC_ESTIMATE              (0xff << 8)
#define STB0899_OFFST_I_DC_ESTIMATE        8
#define STB0899_WIDTH_I_DC_ESTIMATE        8
#define STB0899_Q_DC_ESTIMATE              (0xff << 0)
#define STB0899_OFFST_Q_DC_ESTIMATE        0
#define STB0899_WIDTH_Q_DC_ESTIMATE        8

#define STB0899_OFF0_FLL_CNTRL             0xf310
#define STB0899_BASE_FLL_CNTRL             0x00000020
#define STB0899_CRL_FLL_ACC                (0x01 << 4)
#define STB0899_OFFST_CRL_FLL_ACC          4
#define STB0899_WIDTH_CRL_FLL_ACC          1
#define STB0899_FLL_AVG_PERIOD             (0x0f << 0)
#define STB0899_OFFST_FLL_AVG_PERIOD       0
#define STB0899_WIDTH_FLL_AVG_PERIOD       4

#define STB0899_OFF0_FLL_FREQ_WD           0xf314
#define STB0899_BASE_FLL_FREQ_WD           0x00000020
#define STB0899_FLL_FREQ_WD                (0xffffffff << 0)
#define STB0899_OFFST_FLL_FREQ_WD          0
#define STB0899_WIDTH_FLL_FREQ_WD          32

#define STB0899_OFF0_ANTI_ALIAS_SEL        0xf358
#define STB0899_BASE_ANTI_ALIAS_SEL        0x00000020
#define STB0899_ANTI_ALIAS_SELB            (0x03 << 0)
#define STB0899_OFFST_ANTI_ALIAS_SELB      0
#define STB0899_WIDTH_ANTI_ALIAS_SELB      2

#define STB0899_OFF0_RRC_ALPHA             0xf35c
#define STB0899_BASE_RRC_ALPHA             0x00000020
#define STB0899_RRC_ALPHA                  (0x03 << 0)
#define STB0899_OFFST_RRC_ALPHA            0
#define STB0899_WIDTH_RRC_ALPHA            2

#define STB0899_OFF0_DC_ADAPT_LSHFT        0xf360
#define STB0899_BASE_DC_ADAPT_LSHFT        0x00000020
#define STB0899_DC_ADAPT_LSHFT             (0x077 << 0)
#define STB0899_OFFST_DC_ADAPT_LSHFT       0
#define STB0899_WIDTH_DC_ADAPT_LSHFT       3

#define STB0899_OFF0_IMB_OFFSET            0xf364
#define STB0899_BASE_IMB_OFFSET            0x00000020
#define STB0899_PHS_IMB_COMP               (0xff << 8)
#define STB0899_OFFST_PHS_IMB_COMP         8
#define STB0899_WIDTH_PHS_IMB_COMP         8
#define STB0899_AMPL_IMB_COMP              (0xff << 0)
#define STB0899_OFFST_AMPL_IMB_COMP        0
#define STB0899_WIDTH_AMPL_IMB_COMP        8

#define STB0899_OFF0_IMB_ESTIMATE          0xf368
#define STB0899_BASE_IMB_ESTIMATE          0x00000020
#define STB0899_PHS_IMB_ESTIMATE           (0xff << 8)
#define STB0899_OFFST_PHS_IMB_ESTIMATE     8
#define STB0899_WIDTH_PHS_IMB_ESTIMATE     8
#define STB0899_AMPL_IMB_ESTIMATE          (0xff << 0)
#define STB0899_OFFST_AMPL_IMB_ESTIMATE    0
#define STB0899_WIDTH_AMPL_IMB_ESTIMATE    8

#define STB0899_OFF0_IMB_CNTRL             0xf36c
#define STB0899_BASE_IMB_CNTRL             0x00000020
#define STB0899_PHS_ADAPT_LSHFT            (0x07 << 4)
#define STB0899_OFFST_PHS_ADAPT_LSHFT      4
#define STB0899_WIDTH_PHS_ADAPT_LSHFT      3
#define STB0899_AMPL_ADAPT_LSHFT           (0x07 << 1)
#define STB0899_OFFST_AMPL_ADAPT_LSHFT     1
#define STB0899_WIDTH_AMPL_ADAPT_LSHFT     3
#define STB0899_IMB_COMP                   (0x01 << 0)
#define STB0899_OFFST_IMB_COMP             0
#define STB0899_WIDTH_IMB_COMP             1

#define STB0899_OFF0_IF_AGC_CNTRL2         0xf374
#define STB0899_BASE_IF_AGC_CNTRL2         0x00000020
#define STB0899_IF_AGC_LOCK_TH             (0xff << 11)
#define STB0899_OFFST_IF_AGC_LOCK_TH       11
#define STB0899_WIDTH_IF_AGC_LOCK_TH       8
#define STB0899_IF_AGC_SD_DIV              (0xff << 3)
#define STB0899_OFFST_IF_AGC_SD_DIV        3
#define STB0899_WIDTH_IF_AGC_SD_DIV        8
#define STB0899_IF_AGC_DUMP_PER            (0x07 << 0)
#define STB0899_OFFST_IF_AGC_DUMP_PER      0
#define STB0899_WIDTH_IF_AGC_DUMP_PER      3

#define STB0899_OFF0_DMD_CNTRL2            0xf378
#define STB0899_BASE_DMD_CNTRL2            0x00000020
#define STB0899_SPECTRUM_INVERT            (0x01 << 2)
#define STB0899_OFFST_SPECTRUM_INVERT      2
#define STB0899_WIDTH_SPECTRUM_INVERT      1
#define STB0899_AGC_MODE                   (0x01 << 1)
#define STB0899_OFFST_AGC_MODE             1
#define STB0899_WIDTH_AGC_MODE             1
#define STB0899_CRL_FREQ_ADJ               (0x01 << 0)
#define STB0899_OFFST_CRL_FREQ_ADJ         0
#define STB0899_WIDTH_CRL_FREQ_ADJ         1

#define STB0899_OFF0_TP_BUFFER             0xf300
#define STB0899_BASE_TP_BUFFER             0x00000040
#define STB0899_TP_BUFFER_IN               (0xffff << 0)
#define STB0899_OFFST_TP_BUFFER_IN         0
#define STB0899_WIDTH_TP_BUFFER_IN         16

#define STB0899_OFF0_TP_BUFFER1            0xf304
#define STB0899_BASE_TP_BUFFER1            0x00000040
#define STB0899_OFF0_TP_BUFFER2            0xf308
#define STB0899_BASE_TP_BUFFER2            0x00000040
#define STB0899_OFF0_TP_BUFFER3            0xf30c
#define STB0899_BASE_TP_BUFFER3            0x00000040
#define STB0899_OFF0_TP_BUFFER4            0xf310
#define STB0899_BASE_TP_BUFFER4            0x00000040
#define STB0899_OFF0_TP_BUFFER5            0xf314
#define STB0899_BASE_TP_BUFFER5            0x00000040
#define STB0899_OFF0_TP_BUFFER6            0xf318
#define STB0899_BASE_TP_BUFFER6            0x00000040
#define STB0899_OFF0_TP_BUFFER7            0xf31c
#define STB0899_BASE_TP_BUFFER7            0x00000040
#define STB0899_OFF0_TP_BUFFER8            0xf320
#define STB0899_BASE_TP_BUFFER8            0x00000040
#define STB0899_OFF0_TP_BUFFER9            0xf324
#define STB0899_BASE_TP_BUFFER9            0x00000040
#define STB0899_OFF0_TP_BUFFER10           0xf328
#define STB0899_BASE_TP_BUFFER10           0x00000040
#define STB0899_OFF0_TP_BUFFER11           0xf32c
#define STB0899_BASE_TP_BUFFER11           0x00000040
#define STB0899_OFF0_TP_BUFFER12           0xf330
#define STB0899_BASE_TP_BUFFER12           0x00000040
#define STB0899_OFF0_TP_BUFFER13           0xf334
#define STB0899_BASE_TP_BUFFER13           0x00000040
#define STB0899_OFF0_TP_BUFFER14           0xf338
#define STB0899_BASE_TP_BUFFER14           0x00000040
#define STB0899_OFF0_TP_BUFFER15           0xf33c
#define STB0899_BASE_TP_BUFFER15           0x00000040
#define STB0899_OFF0_TP_BUFFER16           0xf340
#define STB0899_BASE_TP_BUFFER16           0x00000040
#define STB0899_OFF0_TP_BUFFER17           0xf344
#define STB0899_BASE_TP_BUFFER17           0x00000040
#define STB0899_OFF0_TP_BUFFER18           0xf348
#define STB0899_BASE_TP_BUFFER18           0x00000040
#define STB0899_OFF0_TP_BUFFER19           0xf34c
#define STB0899_BASE_TP_BUFFER19           0x00000040
#define STB0899_OFF0_TP_BUFFER20           0xf350
#define STB0899_BASE_TP_BUFFER20           0x00000040
#define STB0899_OFF0_TP_BUFFER21           0xf354
#define STB0899_BASE_TP_BUFFER21           0x00000040
#define STB0899_OFF0_TP_BUFFER22           0xf358
#define STB0899_BASE_TP_BUFFER22           0x00000040
#define STB0899_OFF0_TP_BUFFER23           0xf35c
#define STB0899_BASE_TP_BUFFER23           0x00000040
#define STB0899_OFF0_TP_BUFFER24           0xf360
#define STB0899_BASE_TP_BUFFER24           0x00000040
#define STB0899_OFF0_TP_BUFFER25           0xf364
#define STB0899_BASE_TP_BUFFER25           0x00000040
#define STB0899_OFF0_TP_BUFFER26           0xf368
#define STB0899_BASE_TP_BUFFER26           0x00000040
#define STB0899_OFF0_TP_BUFFER27           0xf36c
#define STB0899_BASE_TP_BUFFER27           0x00000040
#define STB0899_OFF0_TP_BUFFER28           0xf370
#define STB0899_BASE_TP_BUFFER28           0x00000040
#define STB0899_OFF0_TP_BUFFER29           0xf374
#define STB0899_BASE_TP_BUFFER29           0x00000040
#define STB0899_OFF0_TP_BUFFER30           0xf378
#define STB0899_BASE_TP_BUFFER30           0x00000040
#define STB0899_OFF0_TP_BUFFER31           0xf37c
#define STB0899_BASE_TP_BUFFER31           0x00000040
#define STB0899_OFF0_TP_BUFFER32           0xf300
#define STB0899_BASE_TP_BUFFER32           0x00000060
#define STB0899_OFF0_TP_BUFFER33           0xf304
#define STB0899_BASE_TP_BUFFER33           0x00000060
#define STB0899_OFF0_TP_BUFFER34           0xf308
#define STB0899_BASE_TP_BUFFER34           0x00000060
#define STB0899_OFF0_TP_BUFFER35           0xf30c
#define STB0899_BASE_TP_BUFFER35           0x00000060
#define STB0899_OFF0_TP_BUFFER36           0xf310
#define STB0899_BASE_TP_BUFFER36           0x00000060
#define STB0899_OFF0_TP_BUFFER37           0xf314
#define STB0899_BASE_TP_BUFFER37           0x00000060
#define STB0899_OFF0_TP_BUFFER38           0xf318
#define STB0899_BASE_TP_BUFFER38           0x00000060
#define STB0899_OFF0_TP_BUFFER39           0xf31c
#define STB0899_BASE_TP_BUFFER39           0x00000060
#define STB0899_OFF0_TP_BUFFER40           0xf320
#define STB0899_BASE_TP_BUFFER40           0x00000060
#define STB0899_OFF0_TP_BUFFER41           0xf324
#define STB0899_BASE_TP_BUFFER41           0x00000060
#define STB0899_OFF0_TP_BUFFER42           0xf328
#define STB0899_BASE_TP_BUFFER42           0x00000060
#define STB0899_OFF0_TP_BUFFER43           0xf32c
#define STB0899_BASE_TP_BUFFER43           0x00000060
#define STB0899_OFF0_TP_BUFFER44           0xf330
#define STB0899_BASE_TP_BUFFER44           0x00000060
#define STB0899_OFF0_TP_BUFFER45           0xf334
#define STB0899_BASE_TP_BUFFER45           0x00000060
#define STB0899_OFF0_TP_BUFFER46           0xf338
#define STB0899_BASE_TP_BUFFER46           0x00000060
#define STB0899_OFF0_TP_BUFFER47           0xf33c
#define STB0899_BASE_TP_BUFFER47           0x00000060
#define STB0899_OFF0_TP_BUFFER48           0xf340
#define STB0899_BASE_TP_BUFFER48           0x00000060
#define STB0899_OFF0_TP_BUFFER49           0xf344
#define STB0899_BASE_TP_BUFFER49           0x00000060
#define STB0899_OFF0_TP_BUFFER50           0xf348
#define STB0899_BASE_TP_BUFFER50           0x00000060
#define STB0899_OFF0_TP_BUFFER51           0xf34c
#define STB0899_BASE_TP_BUFFER51           0x00000060
#define STB0899_OFF0_TP_BUFFER52           0xf350
#define STB0899_BASE_TP_BUFFER52           0x00000060
#define STB0899_OFF0_TP_BUFFER53           0xf354
#define STB0899_BASE_TP_BUFFER53           0x00000060
#define STB0899_OFF0_TP_BUFFER54           0xf358
#define STB0899_BASE_TP_BUFFER54           0x00000060
#define STB0899_OFF0_TP_BUFFER55           0xf35c
#define STB0899_BASE_TP_BUFFER55           0x00000060
#define STB0899_OFF0_TP_BUFFER56           0xf360
#define STB0899_BASE_TP_BUFFER56           0x00000060
#define STB0899_OFF0_TP_BUFFER57           0xf364
#define STB0899_BASE_TP_BUFFER57           0x00000060
#define STB0899_OFF0_TP_BUFFER58           0xf368
#define STB0899_BASE_TP_BUFFER58           0x00000060
#define STB0899_OFF0_TP_BUFFER59           0xf36c
#define STB0899_BASE_TP_BUFFER59           0x00000060
#define STB0899_OFF0_TP_BUFFER60           0xf370
#define STB0899_BASE_TP_BUFFER60           0x00000060
#define STB0899_OFF0_TP_BUFFER61           0xf374
#define STB0899_BASE_TP_BUFFER61           0x00000060
#define STB0899_OFF0_TP_BUFFER62           0xf378
#define STB0899_BASE_TP_BUFFER62           0x00000060
#define STB0899_OFF0_TP_BUFFER63           0xf37c
#define STB0899_BASE_TP_BUFFER63           0x00000060

#define STB0899_OFF0_RESET_CNTRL           0xf300
#define STB0899_BASE_RESET_CNTRL           0x00000400
#define STB0899_DVBS2_RESET                (0x01 << 0)
#define STB0899_OFFST_DVBS2_RESET          0
#define STB0899_WIDTH_DVBS2_RESET          1

#define STB0899_OFF0_ACM_ENABLE            0xf304
#define STB0899_BASE_ACM_ENABLE            0x00000400
#define STB0899_ACM_ENABLE                 1

#define STB0899_OFF0_DESCR_CNTRL           0xf30c
#define STB0899_BASE_DESCR_CNTRL           0x00000400
#define STB0899_OFFST_DESCR_CNTRL          0
#define STB0899_WIDTH_DESCR_CNTRL          16

#define STB0899_OFF0_UWP_CNTRL1            0xf320
#define STB0899_BASE_UWP_CNTRL1            0x00000400
#define STB0899_UWP_TH_SOF                 (0x7fff << 11)
#define STB0899_OFFST_UWP_TH_SOF           11
#define STB0899_WIDTH_UWP_TH_SOF           15
#define STB0899_UWP_ESN0_QUANT             (0xff << 3)
#define STB0899_OFFST_UWP_ESN0_QUANT       3
#define STB0899_WIDTH_UWP_ESN0_QUANT       8
#define STB0899_UWP_ESN0_AVE               (0x03 << 1)
#define STB0899_OFFST_UWP_ESN0_AVE         1
#define STB0899_WIDTH_UWP_ESN0_AVE         2
#define STB0899_UWP_START                  (0x01 << 0)
#define STB0899_OFFST_UWP_START            0
#define STB0899_WIDTH_UWP_START            1

#define STB0899_OFF0_UWP_CNTRL2            0xf324
#define STB0899_BASE_UWP_CNTRL2            0x00000400
#define STB0899_UWP_MISS_TH                (0xff << 16)
#define STB0899_OFFST_UWP_MISS_TH          16
#define STB0899_WIDTH_UWP_MISS_TH          8
#define STB0899_FE_FINE_TRK                (0xff << 8)
#define STB0899_OFFST_FE_FINE_TRK          8
#define STB0899_WIDTH_FE_FINE_TRK          8
#define STB0899_FE_COARSE_TRK              (0xff << 0)
#define STB0899_OFFST_FE_COARSE_TRK        0
#define STB0899_WIDTH_FE_COARSE_TRK        8

#define STB0899_OFF0_UWP_STAT1             0xf328
#define STB0899_BASE_UWP_STAT1             0x00000400
#define STB0899_UWP_STATE                  (0x03ff << 15)
#define STB0899_OFFST_UWP_STATE            15
#define STB0899_WIDTH_UWP_STATE            10
#define STB0899_UW_MAX_PEAK                (0x7fff << 0)
#define STB0899_OFFST_UW_MAX_PEAK          0
#define STB0899_WIDTH_UW_MAX_PEAK          15

#define STB0899_OFF0_UWP_STAT2             0xf32c
#define STB0899_BASE_UWP_STAT2             0x00000400
#define STB0899_ESNO_EST                   (0x07ffff << 7)
#define STB0899_OFFST_ESN0_EST             7
#define STB0899_WIDTH_ESN0_EST             19
#define STB0899_UWP_DECODE_MOD             (0x7f << 0)
#define STB0899_OFFST_UWP_DECODE_MOD       0
#define STB0899_WIDTH_UWP_DECODE_MOD       7

#define STB0899_OFF0_DMD_CORE_ID           0xf334
#define STB0899_BASE_DMD_CORE_ID           0x00000400
#define STB0899_CORE_ID                    (0xffffffff << 0)
#define STB0899_OFFST_CORE_ID              0
#define STB0899_WIDTH_CORE_ID              32

#define STB0899_OFF0_DMD_VERSION_ID        0xf33c
#define STB0899_BASE_DMD_VERSION_ID        0x00000400
#define STB0899_VERSION_ID                 (0xff << 0)
#define STB0899_OFFST_VERSION_ID           0
#define STB0899_WIDTH_VERSION_ID           8

#define STB0899_OFF0_DMD_STAT2             0xf340
#define STB0899_BASE_DMD_STAT2             0x00000400
#define STB0899_CSM_LOCK                   (0x01 << 1)
#define STB0899_OFFST_CSM_LOCK             1
#define STB0899_WIDTH_CSM_LOCK             1
#define STB0899_UWP_LOCK                   (0x01 << 0)
#define STB0899_OFFST_UWP_LOCK             0
#define STB0899_WIDTH_UWP_LOCK             1

#define STB0899_OFF0_FREQ_ADJ_SCALE        0xf344
#define STB0899_BASE_FREQ_ADJ_SCALE        0x00000400
#define STB0899_FREQ_ADJ_SCALE             (0x0fff << 0)
#define STB0899_OFFST_FREQ_ADJ_SCALE       0
#define STB0899_WIDTH_FREQ_ADJ_SCALE       12

#define STB0899_OFF0_UWP_CNTRL3            0xf34c
#define STB0899_BASE_UWP_CNTRL3            0x00000400
#define STB0899_UWP_TH_TRACK               (0x7fff << 15)
#define STB0899_OFFST_UWP_TH_TRACK         15
#define STB0899_WIDTH_UWP_TH_TRACK         15
#define STB0899_UWP_TH_ACQ                 (0x7fff << 0)
#define STB0899_OFFST_UWP_TH_ACQ           0
#define STB0899_WIDTH_UWP_TH_ACQ           15

#define STB0899_OFF0_SYM_CLK_SEL           0xf350
#define STB0899_BASE_SYM_CLK_SEL           0x00000400
#define STB0899_SYM_CLK_SEL                (0x03 << 0)
#define STB0899_OFFST_SYM_CLK_SEL          0
#define STB0899_WIDTH_SYM_CLK_SEL          2

#define STB0899_OFF0_SOF_SRCH_TO           0xf354
#define STB0899_BASE_SOF_SRCH_TO           0x00000400
#define STB0899_SOF_SEARCH_TIMEOUT         (0x3fffff << 0)
#define STB0899_OFFST_SOF_SEARCH_TIMEOUT   0
#define STB0899_WIDTH_SOF_SEARCH_TIMEOUT   22

#define STB0899_OFF0_ACQ_CNTRL1            0xf358
#define STB0899_BASE_ACQ_CNTRL1            0x00000400
#define STB0899_FE_FINE_ACQ                (0xff << 8)
#define STB0899_OFFST_FE_FINE_ACQ          8
#define STB0899_WIDTH_FE_FINE_ACQ          8
#define STB0899_FE_COARSE_ACQ              (0xff << 0)
#define STB0899_OFFST_FE_COARSE_ACQ        0
#define STB0899_WIDTH_FE_COARSE_ACQ        8

#define STB0899_OFF0_ACQ_CNTRL2            0xf35c
#define STB0899_BASE_ACQ_CNTRL2            0x00000400
#define STB0899_ZIGZAG                     (0x01 << 25)
#define STB0899_OFFST_ZIGZAG               25
#define STB0899_WIDTH_ZIGZAG               1
#define STB0899_NUM_STEPS                  (0xff << 17)
#define STB0899_OFFST_NUM_STEPS            17
#define STB0899_WIDTH_NUM_STEPS            8
#define STB0899_FREQ_STEPSIZE              (0x1ffff << 0)
#define STB0899_OFFST_FREQ_STEPSIZE        0
#define STB0899_WIDTH_FREQ_STEPSIZE        17

#define STB0899_OFF0_ACQ_CNTRL3            0xf360
#define STB0899_BASE_ACQ_CNTRL3            0x00000400
#define STB0899_THRESHOLD_SCL              (0x3f << 23)
#define STB0899_OFFST_THRESHOLD_SCL        23
#define STB0899_WIDTH_THRESHOLD_SCL        6
#define STB0899_UWP_TH_SRCH                (0x7fff << 8)
#define STB0899_OFFST_UWP_TH_SRCH          8
#define STB0899_WIDTH_UWP_TH_SRCH          15
#define STB0899_AUTO_REACQUIRE             (0x01 << 7)
#define STB0899_OFFST_AUTO_REACQUIRE       7
#define STB0899_WIDTH_AUTO_REACQUIRE       1
#define STB0899_TRACK_LOCK_SEL             (0x01 << 6)
#define STB0899_OFFST_TRACK_LOCK_SEL       6
#define STB0899_WIDTH_TRACK_LOCK_SEL       1
#define STB0899_ACQ_SEARCH_MODE            (0x03 << 4)
#define STB0899_OFFST_ACQ_SEARCH_MODE      4
#define STB0899_WIDTH_ACQ_SEARCH_MODE      2
#define STB0899_CONFIRM_FRAMES             (0x0f << 0)
#define STB0899_OFFST_CONFIRM_FRAMES       0
#define STB0899_WIDTH_CONFIRM_FRAMES       4

#define STB0899_OFF0_FE_SETTLE             0xf364
#define STB0899_BASE_FE_SETTLE             0x00000400
#define STB0899_SETTLING_TIME              (0x3fffff << 0)
#define STB0899_OFFST_SETTLING_TIME        0
#define STB0899_WIDTH_SETTLING_TIME        22

#define STB0899_OFF0_AC_DWELL              0xf368
#define STB0899_BASE_AC_DWELL              0x00000400
#define STB0899_DWELL_TIME                 (0x3fffff << 0)
#define STB0899_OFFST_DWELL_TIME           0
#define STB0899_WIDTH_DWELL_TIME           22

#define STB0899_OFF0_ACQUIRE_TRIG          0xf36c
#define STB0899_BASE_ACQUIRE_TRIG          0x00000400
#define STB0899_ACQUIRE                    (0x01 << 0)
#define STB0899_OFFST_ACQUIRE              0
#define STB0899_WIDTH_ACQUIRE              1

#define STB0899_OFF0_LOCK_LOST             0xf370
#define STB0899_BASE_LOCK_LOST             0x00000400
#define STB0899_LOCK_LOST                  (0x01 << 0)
#define STB0899_OFFST_LOCK_LOST            0
#define STB0899_WIDTH_LOCK_LOST            1

#define STB0899_OFF0_ACQ_STAT1             0xf374
#define STB0899_BASE_ACQ_STAT1             0x00000400
#define STB0899_STEP_FREQ                  (0x1fffff << 11)
#define STB0899_OFFST_STEP_FREQ            11
#define STB0899_WIDTH_STEP_FREQ            21
#define STB0899_ACQ_STATE                  (0x07 << 8)
#define STB0899_OFFST_ACQ_STATE            8
#define STB0899_WIDTH_ACQ_STATE            3
#define STB0899_UW_DETECT_COUNT            (0xff << 0)
#define STB0899_OFFST_UW_DETECT_COUNT      0
#define STB0899_WIDTH_UW_DETECT_COUNT      8

#define STB0899_OFF0_ACQ_TIMEOUT           0xf378
#define STB0899_BASE_ACQ_TIMEOUT           0x00000400
#define STB0899_ACQ_TIMEOUT                (0x3fffff << 0)
#define STB0899_OFFST_ACQ_TIMEOUT          0
#define STB0899_WIDTH_ACQ_TIMEOUT          22

#define STB0899_OFF0_ACQ_TIME              0xf37c
#define STB0899_BASE_ACQ_TIME              0x00000400
#define STB0899_ACQ_TIME_SYM               (0xffffff << 0)
#define STB0899_OFFST_ACQ_TIME_SYM         0
#define STB0899_WIDTH_ACQ_TIME_SYM         24

#define STB0899_OFF0_FINAL_AGC_CNTRL       0xf308
#define STB0899_BASE_FINAL_AGC_CNTRL       0x00000440
#define STB0899_FINAL_GAIN_INIT            (0x3fff << 12)
#define STB0899_OFFST_FINAL_GAIN_INIT      12
#define STB0899_WIDTH_FINAL_GAIN_INIT      14
#define STB0899_FINAL_LOOP_GAIN            (0x0f << 8)
#define STB0899_OFFST_FINAL_LOOP_GAIN      8
#define STB0899_WIDTH_FINAL_LOOP_GAIN      4
#define STB0899_FINAL_LD_GAIN_INIT         (0x01 << 7)
#define STB0899_OFFST_FINAL_LD_GAIN_INIT   7
#define STB0899_WIDTH_FINAL_LD_GAIN_INIT   1
#define STB0899_FINAL_AGC_REF              (0x7f << 0)
#define STB0899_OFFST_FINAL_AGC_REF        0
#define STB0899_WIDTH_FINAL_AGC_REF        7

#define STB0899_OFF0_FINAL_AGC_GAIN        0xf30c
#define STB0899_BASE_FINAL_AGC_GAIN        0x00000440
#define STB0899_FINAL_AGC_GAIN             (0x3fff << 0)
#define STB0899_OFFST_FINAL_AGC_GAIN       0
#define STB0899_WIDTH_FINAL_AGC_GAIN       14

#define STB0899_OFF0_EQUALIZER_INIT        0xf310
#define STB0899_BASE_EQUALIZER_INIT        0x00000440
#define STB0899_EQ_SRST                    (0x01 << 1)
#define STB0899_OFFST_EQ_SRST              1
#define STB0899_WIDTH_EQ_SRST              1
#define STB0899_EQ_INIT                    (0x01 << 0)
#define STB0899_OFFST_EQ_INIT              0
#define STB0899_WIDTH_EQ_INIT              1

#define STB0899_OFF0_EQ_CNTRL              0xf314
#define STB0899_BASE_EQ_CNTRL              0x00000440
#define STB0899_EQ_ADAPT_MODE              (0x01 << 18)
#define STB0899_OFFST_EQ_ADAPT_MODE        18
#define STB0899_WIDTH_EQ_ADAPT_MODE        1
#define STB0899_EQ_DELAY                   (0x0f << 14)
#define STB0899_OFFST_EQ_DELAY             14
#define STB0899_WIDTH_EQ_DELAY             4
#define STB0899_EQ_QUANT_LEVEL             (0xff << 6)
#define STB0899_OFFST_EQ_QUANT_LEVEL       6
#define STB0899_WIDTH_EQ_QUANT_LEVEL       8
#define STB0899_EQ_DISABLE_UPDATE          (0x01 << 5)
#define STB0899_OFFST_EQ_DISABLE_UPDATE    5
#define STB0899_WIDTH_EQ_DISABLE_UPDATE    1
#define STB0899_EQ_BYPASS                  (0x01 << 4)
#define STB0899_OFFST_EQ_BYPASS            4
#define STB0899_WIDTH_EQ_BYPASS            1
#define STB0899_EQ_SHIFT                   (0x0f << 0)
#define STB0899_OFFST_EQ_SHIFT             0
#define STB0899_WIDTH_EQ_SHIFT             4

#define STB0899_OFF0_EQ_I_INIT_COEFF_0     0xf320
#define STB0899_OFF1_EQ_I_INIT_COEFF_1     0xf324
#define STB0899_OFF2_EQ_I_INIT_COEFF_2     0xf328
#define STB0899_OFF3_EQ_I_INIT_COEFF_3     0xf32c
#define STB0899_OFF4_EQ_I_INIT_COEFF_4     0xf330
#define STB0899_OFF5_EQ_I_INIT_COEFF_5     0xf334
#define STB0899_OFF6_EQ_I_INIT_COEFF_6     0xf338
#define STB0899_OFF7_EQ_I_INIT_COEFF_7     0xf33c
#define STB0899_OFF8_EQ_I_INIT_COEFF_8     0xf340
#define STB0899_OFF9_EQ_I_INIT_COEFF_9     0xf344
#define STB0899_OFFa_EQ_I_INIT_COEFF_10    0xf348
#define STB0899_BASE_EQ_I_INIT_COEFF_N     0x00000440
#define STB0899_EQ_I_INIT_COEFF_N          (0x0fff << 0)
#define STB0899_OFFST_EQ_I_INIT_COEFF_N    0
#define STB0899_WIDTH_EQ_I_INIT_COEFF_N    12

#define STB0899_OFF0_EQ_Q_INIT_COEFF_0     0xf350
#define STB0899_OFF1_EQ_Q_INIT_COEFF_1     0xf354
#define STB0899_OFF2_EQ_Q_INIT_COEFF_2     0xf358
#define STB0899_OFF3_EQ_Q_INIT_COEFF_3     0xf35c
#define STB0899_OFF4_EQ_Q_INIT_COEFF_4     0xf360
#define STB0899_OFF5_EQ_Q_INIT_COEFF_5     0xf364
#define STB0899_OFF6_EQ_Q_INIT_COEFF_6     0xf368
#define STB0899_OFF7_EQ_Q_INIT_COEFF_7     0xf36c
#define STB0899_OFF8_EQ_Q_INIT_COEFF_8     0xf370
#define STB0899_OFF9_EQ_Q_INIT_COEFF_9     0xf374
#define STB0899_OFFa_EQ_Q_INIT_COEFF_10    0xf378
#define STB0899_BASE_EQ_Q_INIT_COEFF_N     0x00000440
#define STB0899_EQ_Q_INIT_COEFF_N          (0x0fff << 0)
#define STB0899_OFFST_EQ_Q_INIT_COEFF_N    0
#define STB0899_WIDTH_EQ_Q_INIT_COEFF_N    12

#define STB0899_OFF0_EQ_I_OUT_COEFF_0      0xf300
#define STB0899_OFF1_EQ_I_OUT_COEFF_1      0xf304
#define STB0899_OFF2_EQ_I_OUT_COEFF_2      0xf308
#define STB0899_OFF3_EQ_I_OUT_COEFF_3      0xf30c
#define STB0899_OFF4_EQ_I_OUT_COEFF_4      0xf310
#define STB0899_OFF5_EQ_I_OUT_COEFF_5      0xf314
#define STB0899_OFF6_EQ_I_OUT_COEFF_6      0xf318
#define STB0899_OFF7_EQ_I_OUT_COEFF_7      0xf31c
#define STB0899_OFF8_EQ_I_OUT_COEFF_8      0xf320
#define STB0899_OFF9_EQ_I_OUT_COEFF_9      0xf324
#define STB0899_OFFa_EQ_I_OUT_COEFF_10     0xf328
#define STB0899_BASE_EQ_I_OUT_COEFF_N      0x00000460
#define STB0899_EQ_I_OUT_COEFF_N           (0x0fff << 0)
#define STB0899_OFFST_EQ_I_OUT_COEFF_N     0
#define STB0899_WIDTH_EQ_I_OUT_COEFF_N     12

#define STB0899_OFF0_EQ_Q_OUT_COEFF_0      0xf330
#define STB0899_OFF1_EQ_Q_OUT_COEFF_1      0xf334
#define STB0899_OFF2_EQ_Q_OUT_COEFF_2      0xf338
#define STB0899_OFF3_EQ_Q_OUT_COEFF_3      0xf33c
#define STB0899_OFF4_EQ_Q_OUT_COEFF_4      0xf340
#define STB0899_OFF5_EQ_Q_OUT_COEFF_5      0xf344
#define STB0899_OFF6_EQ_Q_OUT_COEFF_6      0xf348
#define STB0899_OFF7_EQ_Q_OUT_COEFF_7      0xf34c
#define STB0899_OFF8_EQ_Q_OUT_COEFF_8      0xf350
#define STB0899_OFF9_EQ_Q_OUT_COEFF_9      0xf354
#define STB0899_OFFa_EQ_Q_OUT_COEFF_10     0xf358
#define STB0899_BASE_EQ_Q_OUT_COEFF_N      0x00000460
#define STB0899_EQ_Q_OUT_COEFF_N           (0x0fff << 0)
#define STB0899_OFFST_EQ_Q_OUT_COEFF_N     0
#define STB0899_WIDTH_EQ_Q_OUT_COEFF_N     12

/*        S2 FEC        */
#define STB0899_OFF0_BLOCK_LNGTH           0xfa04
#define STB0899_BASE_BLOCK_LNGTH           0x00000000
#define STB0899_BLOCK_LENGTH               (0xff << 0)
#define STB0899_OFFST_BLOCK_LENGTH         0
#define STB0899_WIDTH_BLOCK_LENGTH         8

#define STB0899_OFF0_ROW_STR               0xfa08
#define STB0899_BASE_ROW_STR               0x00000000
#define STB0899_ROW_STRIDE                 (0xff << 0)
#define STB0899_OFFST_ROW_STRIDE           0
#define STB0899_WIDTH_ROW_STRIDE           8

#define STB0899_OFF0_MAX_ITER              0xfa0c
#define STB0899_BASE_MAX_ITER              0x00000000
#define STB0899_MAX_ITERATIONS             (0xff << 0)
#define STB0899_OFFST_MAX_ITERATIONS       0
#define STB0899_WIDTH_MAX_ITERATIONS       8

#define STB0899_OFF0_BN_END_ADDR           0xfa10
#define STB0899_BASE_BN_END_ADDR           0x00000000
#define STB0899_BN_END_ADDR                (0x0fff << 0)
#define STB0899_OFFST_BN_END_ADDR          0
#define STB0899_WIDTH_BN_END_ADDR          12

#define STB0899_OFF0_CN_END_ADDR           0xfa14
#define STB0899_BASE_CN_END_ADDR           0x00000000
#define STB0899_CN_END_ADDR                (0x0fff << 0)
#define STB0899_OFFST_CN_END_ADDR          0
#define STB0899_WIDTH_CN_END_ADDR          12

#define STB0899_OFF0_INFO_LENGTH           0xfa1c
#define STB0899_BASE_INFO_LENGTH           0x00000000
#define STB0899_INFO_LENGTH                (0xff << 0)
#define STB0899_OFFST_INFO_LENGTH          0
#define STB0899_WIDTH_INFO_LENGTH          8

#define STB0899_OFF0_BOT_ADDR              0xfa20
#define STB0899_BASE_BOT_ADDR              0x00000000
#define STB0899_BOTTOM_BASE_ADDR           (0x03ff << 0)
#define STB0899_OFFST_BOTTOM_BASE_ADDR     0
#define STB0899_WIDTH_BOTTOM_BASE_ADDR     10

#define STB0899_OFF0_BCH_BLK_LN            0xfa24
#define STB0899_BASE_BCH_BLK_LN            0x00000000
#define STB0899_BCH_BLOCK_LENGTH           (0xffff << 0)
#define STB0899_OFFST_BCH_BLOCK_LENGTH     0
#define STB0899_WIDTH_BCH_BLOCK_LENGTH     16

#define STB0899_OFF0_BCH_T                 0xfa28
#define STB0899_BASE_BCH_T                 0x00000000
#define STB0899_BCH_T                      (0x0f << 0)
#define STB0899_OFFST_BCH_T                0
#define STB0899_WIDTH_BCH_T                4

#define STB0899_OFF0_CNFG_MODE             0xfa00
#define STB0899_BASE_CNFG_MODE             0x00000800
#define STB0899_MODCOD                     (0x1f << 2)
#define STB0899_OFFST_MODCOD               2
#define STB0899_WIDTH_MODCOD               5
#define STB0899_MODCOD_SEL                 (0x01 << 1)
#define STB0899_OFFST_MODCOD_SEL           1
#define STB0899_WIDTH_MODCOD_SEL           1
#define STB0899_CONFIG_MODE                (0x01 << 0)
#define STB0899_OFFST_CONFIG_MODE          0
#define STB0899_WIDTH_CONFIG_MODE          1

#define STB0899_OFF0_LDPC_STAT             0xfa04
#define STB0899_BASE_LDPC_STAT             0x00000800
#define STB0899_ITERATION                  (0xff << 3)
#define STB0899_OFFST_ITERATION            3
#define STB0899_WIDTH_ITERATION            8
#define STB0899_LDPC_DEC_STATE             (0x07 << 0)
#define STB0899_OFFST_LDPC_DEC_STATE       0
#define STB0899_WIDTH_LDPC_DEC_STATE       3

#define STB0899_OFF0_ITER_SCALE            0xfa08
#define STB0899_BASE_ITER_SCALE            0x00000800
#define STB0899_ITERATION_SCALE            (0xff << 0)
#define STB0899_OFFST_ITERATION_SCALE      0
#define STB0899_WIDTH_ITERATION_SCALE      8

#define STB0899_OFF0_INPUT_MODE            0xfa0c
#define STB0899_BASE_INPUT_MODE            0x00000800
#define STB0899_SD_BLOCK1_STREAM0          (0x01 << 0)
#define STB0899_OFFST_SD_BLOCK1_STREAM0    0
#define STB0899_WIDTH_SD_BLOCK1_STREAM0    1

#define STB0899_OFF0_LDPCDECRST            0xfa10
#define STB0899_BASE_LDPCDECRST            0x00000800
#define STB0899_LDPC_DEC_RST               (0x01 << 0)
#define STB0899_OFFST_LDPC_DEC_RST         0
#define STB0899_WIDTH_LDPC_DEC_RST         1

#define STB0899_OFF0_CLK_PER_BYTE_RW       0xfa14
#define STB0899_BASE_CLK_PER_BYTE_RW       0x00000800
#define STB0899_CLKS_PER_BYTE              (0x0f << 0)
#define STB0899_OFFST_CLKS_PER_BYTE        0
#define STB0899_WIDTH_CLKS_PER_BYTE        5

#define STB0899_OFF0_BCH_ERRORS            0xfa18
#define STB0899_BASE_BCH_ERRORS            0x00000800
#define STB0899_BCH_ERRORS                 (0x0f << 0)
#define STB0899_OFFST_BCH_ERRORS           0
#define STB0899_WIDTH_BCH_ERRORS           4

#define STB0899_OFF0_LDPC_ERRORS           0xfa1c
#define STB0899_BASE_LDPC_ERRORS           0x00000800
#define STB0899_LDPC_ERRORS                (0xffff << 0)
#define STB0899_OFFST_LDPC_ERRORS          0
#define STB0899_WIDTH_LDPC_ERRORS          16

#define STB0899_OFF0_BCH_MODE              0xfa20
#define STB0899_BASE_BCH_MODE              0x00000800
#define STB0899_BCH_CORRECT_N              (0x01 << 1)
#define STB0899_OFFST_BCH_CORRECT_N        1
#define STB0899_WIDTH_BCH_CORRECT_N        1
#define STB0899_FULL_BYPASS                (0x01 << 0)
#define STB0899_OFFST_FULL_BYPASS          0
#define STB0899_WIDTH_FULL_BYPASS          1

#define STB0899_OFF0_ERR_ACC_PER           0xfa24
#define STB0899_BASE_ERR_ACC_PER           0x00000800
#define STB0899_BCH_ERR_ACC_PERIOD         (0x0f << 0)
#define STB0899_OFFST_BCH_ERR_ACC_PERIOD   0
#define STB0899_WIDTH_BCH_ERR_ACC_PERIOD   4

#define STB0899_OFF0_BCH_ERR_ACC           0xfa28
#define STB0899_BASE_BCH_ERR_ACC           0x00000800
#define STB0899_BCH_ERR_ACCUM              (0xff << 0)
#define STB0899_OFFST_BCH_ERR_ACCUM        0
#define STB0899_WIDTH_BCH_ERR_ACCUM        8

#define STB0899_OFF0_FEC_CORE_ID_REG       0xfa2c
#define STB0899_BASE_FEC_CORE_ID_REG       0x00000800
#define STB0899_FEC_CORE_ID                (0xffffffff << 0)
#define STB0899_OFFST_FEC_CORE_ID          0
#define STB0899_WIDTH_FEC_CORE_ID          32

#define STB0899_OFF0_FEC_VER_ID_REG        0xfa34
#define STB0899_BASE_FEC_VER_ID_REG        0x00000800
#define STB0899_FEC_VER_ID                 (0xff << 0)
#define STB0899_OFFST_FEC_VER_ID           0
#define STB0899_WIDTH_FEC_VER_ID           8

#define STB0899_OFF0_FEC_TP_SEL            0xfa38
#define STB0899_BASE_FEC_TP_SEL            0x00000800

#define STB0899_OFF0_CSM_CNTRL1            0xf310
#define STB0899_BASE_CSM_CNTRL1            0x00000400
#define STB0899_CSM_FORCE_FREQLOCK         (0x01 << 19)
#define STB0899_OFFST_CSM_FORCE_FREQLOCK   19
#define STB0899_WIDTH_CSM_FORCE_FREQLOCK   1
#define STB0899_CSM_FREQ_LOCKSTATE         (0x01 << 18)
#define STB0899_OFFST_CSM_FREQ_LOCKSTATE   18
#define STB0899_WIDTH_CSM_FREQ_LOCKSTATE   1
#define STB0899_CSM_AUTO_PARAM             (0x01 << 17)
#define STB0899_OFFST_CSM_AUTO_PARAM       17
#define STB0899_WIDTH_CSM_AUTO_PARAM       1
#define STB0899_FE_LOOP_SHIFT              (0x07 << 14)
#define STB0899_OFFST_FE_LOOP_SHIFT        14
#define STB0899_WIDTH_FE_LOOP_SHIFT        3
#define STB0899_CSM_AGC_SHIFT              (0x07 << 11)
#define STB0899_OFFST_CSM_AGC_SHIFT        11
#define STB0899_WIDTH_CSM_AGC_SHIFT        3
#define STB0899_CSM_AGC_GAIN               (0x1ff << 2)
#define STB0899_OFFST_CSM_AGC_GAIN         2
#define STB0899_WIDTH_CSM_AGC_GAIN         9
#define STB0899_CSM_TWO_PASS               (0x01 << 1)
#define STB0899_OFFST_CSM_TWO_PASS         1
#define STB0899_WIDTH_CSM_TWO_PASS         1
#define STB0899_CSM_DVT_TABLE              (0x01 << 0)
#define STB0899_OFFST_CSM_DVT_TABLE        0
#define STB0899_WIDTH_CSM_DVT_TABLE        1

#define STB0899_OFF0_CSM_CNTRL2            0xf314
#define STB0899_BASE_CSM_CNTRL2            0x00000400
#define STB0899_CSM_GAMMA_RHO_ACQ          (0x1ff << 9)
#define STB0899_OFFST_CSM_GAMMA_RHOACQ     9
#define STB0899_WIDTH_CSM_GAMMA_RHOACQ     9
#define STB0899_CSM_GAMMA_ACQ              (0x1ff << 0)
#define STB0899_OFFST_CSM_GAMMA_ACQ        0
#define STB0899_WIDTH_CSM_GAMMA_ACQ        9

#define STB0899_OFF0_CSM_CNTRL3            0xf318
#define STB0899_BASE_CSM_CNTRL3            0x00000400
#define STB0899_CSM_GAMMA_RHO_TRACK        (0x1ff << 9)
#define STB0899_OFFST_CSM_GAMMA_RHOTRACK   9
#define STB0899_WIDTH_CSM_GAMMA_RHOTRACK   9
#define STB0899_CSM_GAMMA_TRACK            (0x1ff << 0)
#define STB0899_OFFST_CSM_GAMMA_TRACK      0
#define STB0899_WIDTH_CSM_GAMMA_TRACK      9

#define STB0899_OFF0_CSM_CNTRL4            0xf31c
#define STB0899_BASE_CSM_CNTRL4            0x00000400
#define STB0899_CSM_PHASEDIFF_THRESH       (0x0f << 8)
#define STB0899_OFFST_CSM_PHASEDIFF_THRESH 8
#define STB0899_WIDTH_CSM_PHASEDIFF_THRESH 4
#define STB0899_CSM_LOCKCOUNT_THRESH       (0xff << 0)
#define STB0899_OFFST_CSM_LOCKCOUNT_THRESH 0
#define STB0899_WIDTH_CSM_LOCKCOUNT_THRESH 8

/* Check on chapter 8 page 42 */
#define STB0899_ERRCTRL1                   0xf574
#define STB0899_ERRCTRL2                   0xf575
#define STB0899_ERRCTRL3                   0xf576
#define STB0899_ERR_SRC_S1                 (0x1f << 3)
#define STB0899_OFFST_ERR_SRC_S1           3
#define STB0899_WIDTH_ERR_SRC_S1           5
#define STB0899_ERR_SRC_S2                 (0x0f << 0)
#define STB0899_OFFST_ERR_SRC_S2           0
#define STB0899_WIDTH_ERR_SRC_S2           4
#define STB0899_NOE                        (0x07 << 0)
#define STB0899_OFFST_NOE                  0
#define STB0899_WIDTH_NOE                  3

#define STB0899_ECNT1M                     0xf524
#define STB0899_ECNT1L                     0xf525
#define STB0899_ECNT2M                     0xf526
#define STB0899_ECNT2L                     0xf527
#define STB0899_ECNT3M                     0xf528
#define STB0899_ECNT3L                     0xf529

#define STB0899_DMONMSK1                   0xf57b
#define STB0899_DMONMSK1_WAIT_1STEP        (1 << 7)
#define STB0899_DMONMSK1_FREE_14           (1 << 6)
#define STB0899_DMONMSK1_AVRGVIT_CALC      (1 << 5)
#define STB0899_DMONMSK1_FREE_12           (1 << 4)
#define STB0899_DMONMSK1_FREE_11           (1 << 3)
#define STB0899_DMONMSK1_B0DIV_CALC        (1 << 2)
#define STB0899_DMONMSK1_KDIVB1_CALC       (1 << 1)
#define STB0899_DMONMSK1_KDIVB2_CALC       (1 << 0)

#define STB0899_DMONMSK0                   0xf57c
#define STB0899_DMONMSK0_SMOTTH_CALC       (1 << 7)
#define STB0899_DMONMSK0_FREE_6            (1 << 6)
#define STB0899_DMONMSK0_SIGPOWER_CALC     (1 << 5)
#define STB0899_DMONMSK0_QSEUIL_CALC       (1 << 4)
#define STB0899_DMONMSK0_FREE_3            (1 << 3)
#define STB0899_DMONMSK0_FREE_2            (1 << 2)
#define STB0899_DMONMSK0_KVDIVB1_CALC      (1 << 1)
#define STB0899_DMONMSK0_KVDIVB2_CALC      (1 << 0)

#define STB0899_TSULC                      0xf549
#define STB0899_ULNOSYNCBYTES              (0x01 << 7)
#define STB0899_OFFST_ULNOSYNCBYTES        7
#define STB0899_WIDTH_ULNOSYNCBYTES        1
#define STB0899_ULPARITY_ON                (0x01 << 6)
#define STB0899_OFFST_ULPARITY_ON          6
#define STB0899_WIDTH_ULPARITY_ON          1
#define STB0899_ULSYNCOUTRS                (0x01 << 5)
#define STB0899_OFFST_ULSYNCOUTRS          5
#define STB0899_WIDTH_ULSYNCOUTRS          1
#define STB0899_ULDSS_PACKETS              (0x01 << 0)
#define STB0899_OFFST_ULDSS_PACKETS        0
#define STB0899_WIDTH_ULDSS_PACKETS        1

#define STB0899_TSLPL                      0xf54b
#define STB0899_LLDVBS2_MODE               (0x01 << 4)
#define STB0899_OFFST_LLDVBS2_MODE         4
#define STB0899_WIDTH_LLDVBS2_MODE         1
#define STB0899_LLISSYI_ON                 (0x01 << 3)
#define STB0899_OFFST_LLISSYI_ON           3
#define STB0899_WIDTH_LLISSYI_ON           1
#define STB0899_LLNPD_ON                   (0x01 << 2)
#define STB0899_OFFST_LLNPD_ON             2
#define STB0899_WIDTH_LLNPD_ON             1
#define STB0899_LLCRC8_ON                  (0x01 << 1)
#define STB0899_OFFST_LLCRC8_ON            1
#define STB0899_WIDTH_LLCRC8_ON            1

#define STB0899_TSCFGH                     0xf54c
#define STB0899_OUTRS_PS                   (0x01 << 6)
#define STB0899_OFFST_OUTRS_PS             6
#define STB0899_WIDTH_OUTRS_PS             1
#define STB0899_SYNCBYTE                   (0x01 << 5)
#define STB0899_OFFST_SYNCBYTE             5
#define STB0899_WIDTH_SYNCBYTE             1
#define STB0899_PFBIT                      (0x01 << 4)
#define STB0899_OFFST_PFBIT                4
#define STB0899_WIDTH_PFBIT                1
#define STB0899_ERR_BIT                    (0x01 << 3)
#define STB0899_OFFST_ERR_BIT              3
#define STB0899_WIDTH_ERR_BIT              1
#define STB0899_MPEG                       (0x01 << 2)
#define STB0899_OFFST_MPEG                 2
#define STB0899_WIDTH_MPEG                 1
#define STB0899_CLK_POL                    (0x01 << 1)
#define STB0899_OFFST_CLK_POL              1
#define STB0899_WIDTH_CLK_POL              1
#define STB0899_FORCE0                     (0x01 << 0)
#define STB0899_OFFST_FORCE0               0
#define STB0899_WIDTH_FORCE0               1

#define STB0899_TSCFGM                     0xf54d
#define STB0899_LLPRIORITY                 (0x01 << 3)
#define STB0899_OFFST_LLPRIORIY            3
#define STB0899_WIDTH_LLPRIORITY           1
#define STB0899_EN188                      (0x01 << 2)
#define STB0899_OFFST_EN188                2
#define STB0899_WIDTH_EN188                1

#define STB0899_TSCFGL                     0xf54e
#define STB0899_DEL_ERRPCK                 (0x01 << 7)
#define STB0899_OFFST_DEL_ERRPCK           7
#define STB0899_WIDTH_DEL_ERRPCK           1
#define STB0899_ERRFLAGSTD                 (0x01 << 5)
#define STB0899_OFFST_ERRFLAGSTD           5
#define STB0899_WIDTH_ERRFLAGSTD           1
#define STB0899_MPEGERR                    (0x01 << 4)
#define STB0899_OFFST_MPEGERR              4
#define STB0899_WIDTH_MPEGERR              1
#define STB0899_BCH_CHK                    (0x01 << 3)
#define STB0899_OFFST_BCH_CHK              5
#define STB0899_WIDTH_BCH_CHK              1
#define STB0899_CRC8CHK                    (0x01 << 2)
#define STB0899_OFFST_CRC8CHK              2
#define STB0899_WIDTH_CRC8CHK              1
#define STB0899_SPEC_INFO                  (0x01 << 1)
#define STB0899_OFFST_SPEC_INFO            1
#define STB0899_WIDTH_SPEC_INFO            1
#define STB0899_LOW_PRIO_CLK               (0x01 << 0)
#define STB0899_OFFST_LOW_PRIO_CLK         0
#define STB0899_WIDTH_LOW_PRIO_CLK         1
#define STB0899_ERROR_NORM                 (0x00 << 0)
#define STB0899_OFFST_ERROR_NORM           0
#define STB0899_WIDTH_ERROR_NORM           0

#define STB0899_TSOUT                      0xf54f
#define STB0899_RSSYNCDEL                  0xf550
#define STB0899_TSINHDELH                  0xf551
#define STB0899_TSINHDELM                  0xf552
#define STB0899_TSINHDELL                  0xf553
#define STB0899_TSLLSTKM                   0xf55a
#define STB0899_TSLLSTKL                   0xf55b
#define STB0899_TSULSTKM                   0xf55c
#define STB0899_TSULSTKL                   0xf55d
#define STB0899_TSSTATUS                   0xf561

#define STB0899_PDELCTRL                   0xf600
#define STB0899_INVERT_RES                 (0x01 << 7)
#define STB0899_OFFST_INVERT_RES           7
#define STB0899_WIDTH_INVERT_RES           1
#define STB0899_FORCE_ACCEPTED             (0x01 << 6)
#define STB0899_OFFST_FORCE_ACCEPTED       6
#define STB0899_WIDTH_FORCE_ACCEPTED       1
#define STB0899_FILTER_EN                  (0x01 << 5)
#define STB0899_OFFST_FILTER_EN            5
#define STB0899_WIDTH_FILTER_EN            1
#define STB0899_LOCKFALL_THRESH            (0x01 << 4)
#define STB0899_OFFST_LOCKFALL_THRESH      4
#define STB0899_WIDTH_LOCKFALL_THRESH      1
#define STB0899_HYST_EN                    (0x01 << 3)
#define STB0899_OFFST_HYST_EN              3
#define STB0899_WIDTH_HYST_EN              1
#define STB0899_HYST_SWRST                 (0x01 << 2)
#define STB0899_OFFST_HYST_SWRST           2
#define STB0899_WIDTH_HYST_SWRST           1
#define STB0899_ALGO_EN                    (0x01 << 1)
#define STB0899_OFFST_ALGO_EN              1
#define STB0899_WIDTH_ALGO_EN              1
#define STB0899_ALGO_SWRST                 (0x01 << 0)
#define STB0899_OFFST_ALGO_SWRST           0
#define STB0899_WIDTH_ALGO_SWRST           1

#define STB0899_PDELCTRL2                  0xf601
#define STB0899_BBHCTRL1                   0xf602
#define STB0899_BBHCTRL2                   0xf603
#define STB0899_HYSTTHRESH                 0xf604

#define STB0899_MATCSTM                    0xf605
#define STB0899_MATCSTL                    0xf606
#define STB0899_UPLCSTM                    0xf607
#define STB0899_UPLCSTL                    0xf608
#define STB0899_DFLCSTM                    0xf609
#define STB0899_DFLCSTL                    0xf60a
#define STB0899_SYNCCST                    0xf60b
#define STB0899_SYNCDCSTM                  0xf60c
#define STB0899_SYNCDCSTL                  0xf60d
#define STB0899_ISI_ENTRY                  0xf60e
#define STB0899_ISI_BIT_EN                 0xf60f
#define STB0899_MATSTRM                    0xf610
#define STB0899_MATSTRL                    0xf611
#define STB0899_UPLSTRM                    0xf612
#define STB0899_UPLSTRL                    0xf613
#define STB0899_DFLSTRM                    0xf614
#define STB0899_DFLSTRL                    0xf615
#define STB0899_SYNCSTR                    0xf616
#define STB0899_SYNCDSTRM                  0xf617
#define STB0899_SYNCDSTRL                  0xf618

#define STB0899_CFGPDELSTATUS1             0xf619
#define STB0899_BADDFL                     (0x01 << 6)
#define STB0899_OFFST_BADDFL               6
#define STB0899_WIDTH_BADDFL               1
#define STB0899_CONTINUOUS_STREAM          (0x01 << 5)
#define STB0899_OFFST_CONTINUOUS_STREAM    5
#define STB0899_WIDTH_CONTINUOUS_STREAM    1
#define STB0899_ACCEPTED_STREAM            (0x01 << 4)
#define STB0899_OFFST_ACCEPTED_STREAM      4
#define STB0899_WIDTH_ACCEPTED_STREAM      1
#define STB0899_BCH_ERRFLAG                (0x01 << 3)
#define STB0899_OFFST_BCH_ERRFLAG          3
#define STB0899_WIDTH_BCH_ERRFLAG          1
#define STB0899_CRCRES                     (0x01 << 2)
#define STB0899_OFFST_CRCRES               2
#define STB0899_WIDTH_CRCRES               1
#define STB0899_CFGPDELSTATUS_LOCK         (0x01 << 1)
#define STB0899_OFFST_CFGPDELSTATUS_LOCK   1
#define STB0899_WIDTH_CFGPDELSTATUS_LOCK   1
#define STB0899_1STLOCK                    (0x01 << 0)
#define STB0899_OFFST_1STLOCK              0
#define STB0899_WIDTH_1STLOCK              1

#define STB0899_CFGPDELSTATUS2             0xf61a
#define STB0899_BBFERRORM                  0xf61b
#define STB0899_BBFERRORL                  0xf61c
#define STB0899_UPKTERRORM                 0xf61d
#define STB0899_UPKTERRORL                 0xf61e

#define STB0899_TSTCK                      0xff10

#define STB0899_TSTRES                     0xff11
#define STB0899_FRESLDPC                   (0x01 << 7)
#define STB0899_OFFST_FRESLDPC             7
#define STB0899_WIDTH_FRESLDPC             1
#define STB0899_FRESRS                     (0x01 << 6)
#define STB0899_OFFST_FRESRS               6
#define STB0899_WIDTH_FRESRS               1
#define STB0899_FRESVIT                    (0x01 << 5)
#define STB0899_OFFST_FRESVIT              5
#define STB0899_WIDTH_FRESVIT              1
#define STB0899_FRESMAS1_2                 (0x01 << 4)
#define STB0899_OFFST_FRESMAS1_2           4
#define STB0899_WIDTH_FRESMAS1_2           1
#define STB0899_FRESACS                    (0x01 << 3)
#define STB0899_OFFST_FRESACS              3
#define STB0899_WIDTH_FRESACS              1
#define STB0899_FRESSYM                    (0x01 << 2)
#define STB0899_OFFST_FRESSYM              2
#define STB0899_WIDTH_FRESSYM              1
#define STB0899_FRESMAS                    (0x01 << 1)
#define STB0899_OFFST_FRESMAS              1
#define STB0899_WIDTH_FRESMAS              1
#define STB0899_FRESINT                    (0x01 << 0)
#define STB0899_OFFST_FRESINIT             0
#define STB0899_WIDTH_FRESINIT             1

#define STB0899_TSTOUT                     0xff12
#define STB0899_EN_SIGNATURE               (0x01 << 7)
#define STB0899_OFFST_EN_SIGNATURE         7
#define STB0899_WIDTH_EN_SIGNATURE         1
#define STB0899_BCLK_CLK                   (0x01 << 6)
#define STB0899_OFFST_BCLK_CLK             6
#define STB0899_WIDTH_BCLK_CLK             1
#define STB0899_SGNL_OUT                   (0x01 << 5)
#define STB0899_OFFST_SGNL_OUT             5
#define STB0899_WIDTH_SGNL_OUT             1
#define STB0899_TS                         (0x01 << 4)
#define STB0899_OFFST_TS                   4
#define STB0899_WIDTH_TS                   1
#define STB0899_CTEST                      (0x01 << 0)
#define STB0899_OFFST_CTEST                0
#define STB0899_WIDTH_CTEST                1

#define STB0899_TSTIN                      0xff13
#define STB0899_TEST_IN                    (0x01 << 7)
#define STB0899_OFFST_TEST_IN              7
#define STB0899_WIDTH_TEST_IN              1
#define STB0899_EN_ADC                     (0x01 << 6)
#define STB0899_OFFST_EN_ADC               6
#define STB0899_WIDTH_ENADC                1
#define STB0899_SGN_ADC                    (0x01 << 5)
#define STB0899_OFFST_SGN_ADC              5
#define STB0899_WIDTH_SGN_ADC              1
#define STB0899_BCLK_IN                    (0x01 << 4)
#define STB0899_OFFST_BCLK_IN              4
#define STB0899_WIDTH_BCLK_IN              1
#define STB0899_JETONIN_MODE               (0x01 << 3)
#define STB0899_OFFST_JETONIN_MODE         3
#define STB0899_WIDTH_JETONIN_MODE         1
#define STB0899_BCLK_VALUE                 (0x01 << 2)
#define STB0899_OFFST_BCLK_VALUE           2
#define STB0899_WIDTH_BCLK_VALUE           1
#define STB0899_SGNRST_T12                 (0x01 << 1)
#define STB0899_OFFST_SGNRST_T12           1
#define STB0899_WIDTH_SGNRST_T12           1
#define STB0899_LOWSP_ENAX                 (0x01 << 0)
#define STB0899_OFFST_LOWSP_ENAX           0
#define STB0899_WIDTH_LOWSP_ENAX           1

#define STB0899_TSTSYS                     0xff14
#define STB0899_TSTCHIP                    0xff15
#define STB0899_TSTFREE                    0xff16
#define STB0899_TSTI2C                     0xff17
#define STB0899_BITSPEEDM                  0xff1c
#define STB0899_BITSPEEDL                  0xff1d
#define STB0899_TBUSBIT                    0xff1e
#define STB0899_TSTDIS                     0xff24
#define STB0899_TSTDISRX                   0xff25
#define STB0899_TSTJETON                   0xff28
#define STB0899_TSTDCADJ                   0xff40
#define STB0899_TSTAGC1                    0xff41
#define STB0899_TSTAGC1N                   0xff42
#define STB0899_TSTPOLYPH                  0xff48
#define STB0899_TSTR                       0xff49
#define STB0899_TSTAGC2                    0xff4a
#define STB0899_TSTCTL1                    0xff4b
#define STB0899_TSTCTL2                    0xff4c
#define STB0899_TSTCTL3                    0xff4d
#define STB0899_TSTDEMAP                   0xff50
#define STB0899_TSTDEMAP2                  0xff51
#define STB0899_TSTDEMMON                  0xff52
#define STB0899_TSTRATE                    0xff53
#define STB0899_TSTSELOUT                  0xff54
#define STB0899_TSYNC                      0xff55
#define STB0899_TSTERR                     0xff56
#define STB0899_TSTRAM1                    0xff58
#define STB0899_TSTVSELOUT                 0xff59
#define STB0899_TSTFORCEIN                 0xff5a
#define STB0899_TSTRS1                     0xff5c
#define STB0899_TSTRS2                     0xff5d
#define STB0899_TSTRS3                     0xff53

#define STB0899_INTBUFSTATUS               0xf200
#define STB0899_INTBUFCTRL                 0xf201
#define STB0899_PCKLENUL                   0xf55e
#define STB0899_PCKLENLL                   0xf55f
#define STB0899_RSPCKLEN                   0xf560

/* 2 registers */
#define STB0899_SYNCDCST                   0xf60c

/*        DiSEqC        */
#define STB0899_DISCNTRL1                  0xf0a0
#define STB0899_TIMOFF                     (0x01 << 7)
#define STB0899_OFFST_TIMOFF               7
#define STB0899_WIDTH_TIMOFF               1
#define STB0899_DISEQCRESET                (0x01 << 6)
#define STB0899_OFFST_DISEQCRESET          6
#define STB0899_WIDTH_DISEQCRESET          1
#define STB0899_TIMCMD                     (0x03 << 4)
#define STB0899_OFFST_TIMCMD               4
#define STB0899_WIDTH_TIMCMD               2
#define STB0899_DISPRECHARGE               (0x01 << 2)
#define STB0899_OFFST_DISPRECHARGE         2
#define STB0899_WIDTH_DISPRECHARGE         1
#define STB0899_DISEQCMODE                 (0x03 << 0)
#define STB0899_OFFST_DISEQCMODE           0
#define STB0899_WIDTH_DISEQCMODE           2

#define STB0899_DISCNTRL2                  0xf0a1
#define STB0899_RECEIVER_ON                (0x01 << 7)
#define STB0899_OFFST_RECEIVER_ON          7
#define STB0899_WIDTH_RECEIVER_ON          1
#define STB0899_IGNO_SHORT_22K             (0x01 << 6)
#define STB0899_OFFST_IGNO_SHORT_22K       6
#define STB0899_WIDTH_IGNO_SHORT_22K       1
#define STB0899_ONECHIP_TRX                (0x01 << 5)
#define STB0899_OFFST_ONECHIP_TRX          5
#define STB0899_WIDTH_ONECHIP_TRX          1
#define STB0899_EXT_ENVELOP                (0x01 << 4)
#define STB0899_OFFST_EXT_ENVELOP          4
#define STB0899_WIDTH_EXT_ENVELOP          1
#define STB0899_PIN_SELECT                 (0x03 << 2)
#define STB0899_OFFST_PIN_SELCT            2
#define STB0899_WIDTH_PIN_SELCT            2
#define STB0899_IRQ_RXEND                  (0x01 << 1)
#define STB0899_OFFST_IRQ_RXEND            1
#define STB0899_WIDTH_IRQ_RXEND            1
#define STB0899_IRQ_4NBYTES                (0x01 << 0)
#define STB0899_OFFST_IRQ_4NBYTES          0
#define STB0899_WIDTH_IRQ_4NBYTES          1

#define STB0899_DISRX_ST0                  0xf0a4
#define STB0899_RXEND                      (0x01 << 7)
#define STB0899_OFFST_RXEND                7
#define STB0899_WIDTH_RXEND                1
#define STB0899_RXACTIVE                   (0x01 << 6)
#define STB0899_OFFST_RXACTIVE             6
#define STB0899_WIDTH_RXACTIVE             1
#define STB0899_SHORT22K                   (0x01 << 5)
#define STB0899_OFFST_SHORT22K             5
#define STB0899_WIDTH_SHORT22K             1
#define STB0899_CONTTONE                   (0x01 << 4)
#define STB0899_OFFST_CONTTONE             4
#define STB0899_WIDTH_CONTONE              1
#define STB0899_4BFIFOREDY                 (0x01 << 3)
#define STB0899_OFFST_4BFIFOREDY           3
#define STB0899_WIDTH_4BFIFOREDY           1
#define STB0899_FIFOEMPTY                  (0x01 << 2)
#define STB0899_OFFST_FIFOEMPTY            2
#define STB0899_WIDTH_FIFOEMPTY            1
#define STB0899_ABORTTRX                   (0x01 << 0)
#define STB0899_OFFST_ABORTTRX             0
#define STB0899_WIDTH_ABORTTRX             1

#define STB0899_DISRX_ST1                  0xf0a5
#define STB0899_RXFAIL                     (0x01 << 7)
#define STB0899_OFFST_RXFAIL               7
#define STB0899_WIDTH_RXFAIL               1
#define STB0899_FIFOPFAIL                  (0x01 << 6)
#define STB0899_OFFST_FIFOPFAIL            6
#define STB0899_WIDTH_FIFOPFAIL            1
#define STB0899_RXNONBYTES                 (0x01 << 5)
#define STB0899_OFFST_RXNONBYTES           5
#define STB0899_WIDTH_RXNONBYTES           1
#define STB0899_FIFOOVF                    (0x01 << 4)
#define STB0899_OFFST_FIFOOVF              4
#define STB0899_WIDTH_FIFOOVF              1
#define STB0899_FIFOBYTENBR                (0x0f << 0)
#define STB0899_OFFST_FIFOBYTENBR          0
#define STB0899_WIDTH_FIFOBYTENBR          4

#define STB0899_DISPARITY                  0xf0a6

#define STB0899_DISFIFO                    0xf0a7

#define STB0899_DISSTATUS                  0xf0a8
#define STB0899_FIFOFULL                   (0x01 << 6)
#define STB0899_OFFST_FIFOFULL             6
#define STB0899_WIDTH_FIFOFULL             1
#define STB0899_TXIDLE                     (0x01 << 5)
#define STB0899_OFFST_TXIDLE               5
#define STB0899_WIDTH_TXIDLE               1
#define STB0899_GAPBURST                   (0x01 << 4)
#define STB0899_OFFST_GAPBURST             4
#define STB0899_WIDTH_GAPBURST             1
#define STB0899_TXFIFOBYTES                (0x0f << 0)
#define STB0899_OFFST_TXFIFOBYTES          0
#define STB0899_WIDTH_TXFIFOBYTES          4
#define STB0899_DISF22                     0xf0a9

#define STB0899_DISF22RX                   0xf0aa

/* General Purpose */
#define STB0899_SYSREG                     0xf101
#define STB0899_ACRPRESC                   0xf110
#define STB0899_OFFST_RSVD2                7
#define STB0899_WIDTH_RSVD2                1
#define STB0899_OFFST_ACRPRESC             4
#define STB0899_WIDTH_ACRPRESC             3
#define STB0899_OFFST_RSVD1                3
#define STB0899_WIDTH_RSVD1                1
#define STB0899_OFFST_ACRPRESC2            0
#define STB0899_WIDTH_ACRPRESC2            3

#define STB0899_ACRDIV1                    0xf111
#define STB0899_ACRDIV2                    0xf112
#define STB0899_DACR1                      0xf113
#define STB0899_DACR2                      0xf114
#define STB0899_OUTCFG                     0xf11c
#define STB0899_MODECFG                    0xf11d
#define STB0899_NCOARSE                    0xf1b3

#define STB0899_SYNTCTRL                   0xf1b6
#define STB0899_STANDBY                    (0x01 << 7)
#define STB0899_OFFST_STANDBY              7
#define STB0899_WIDTH_STANDBY              1
#define STB0899_BYPASSPLL                  (0x01 << 6)
#define STB0899_OFFST_BYPASSPLL            6
#define STB0899_WIDTH_BYPASSPLL            1
#define STB0899_SEL1XRATIO                 (0x01 << 5)
#define STB0899_OFFST_SEL1XRATIO           5
#define STB0899_WIDTH_SEL1XRATIO           1
#define STB0899_SELOSCI                    (0x01 << 1)
#define STB0899_OFFST_SELOSCI              1
#define STB0899_WIDTH_SELOSCI              1

#define STB0899_FILTCTRL                   0xf1b7
#define STB0899_SYSCTRL                    0xf1b8

#define STB0899_STOPCLK1                   0xf1c2
#define STB0899_STOP_CKINTBUF108           (0x01 << 7)
#define STB0899_OFFST_STOP_CKINTBUF108     7
#define STB0899_WIDTH_STOP_CKINTBUF108     1
#define STB0899_STOP_CKINTBUF216           (0x01 << 6)
#define STB0899_OFFST_STOP_CKINTBUF216     6
#define STB0899_WIDTH_STOP_CKINTBUF216     1
#define STB0899_STOP_CHK8PSK               (0x01 << 5)
#define STB0899_OFFST_STOP_CHK8PSK         5
#define STB0899_WIDTH_STOP_CHK8PSK         1
#define STB0899_STOP_CKFEC108              (0x01 << 4)
#define STB0899_OFFST_STOP_CKFEC108        4
#define STB0899_WIDTH_STOP_CKFEC108        1
#define STB0899_STOP_CKFEC216              (0x01 << 3)
#define STB0899_OFFST_STOP_CKFEC216        3
#define STB0899_WIDTH_STOP_CKFEC216        1
#define STB0899_STOP_CKCORE216             (0x01 << 2)
#define STB0899_OFFST_STOP_CKCORE216       2
#define STB0899_WIDTH_STOP_CKCORE216       1
#define STB0899_STOP_CKADCI108             (0x01 << 1)
#define STB0899_OFFST_STOP_CKADCI108       1
#define STB0899_WIDTH_STOP_CKADCI108       1
#define STB0899_STOP_INVCKADCI108          (0x01 << 0)
#define STB0899_OFFST_STOP_INVCKADCI108    0
#define STB0899_WIDTH_STOP_INVCKADCI108    1

#define STB0899_STOPCLK2                   0xf1c3
#define STB0899_STOP_CKS2DMD108            (0x01 << 2)
#define STB0899_OFFST_STOP_CKS2DMD108      2
#define STB0899_WIDTH_STOP_CKS2DMD108      1
#define STB0899_STOP_CKPKDLIN108           (0x01 << 1)
#define STB0899_OFFST_STOP_CKPKDLIN108     1
#define STB0899_WIDTH_STOP_CKPKDLIN108     1
#define STB0899_STOP_CKPKDLIN216           (0x01 << 0)
#define STB0899_OFFST_STOP_CKPKDLIN216     0
#define STB0899_WIDTH_STOP_CKPKDLIN216     1

#define STB0899_TSTTNR1                    0xf1e0
#define STB0899_BYPASS_ADC                 (0x01 << 7)
#define STB0899_OFFST_BYPASS_ADC           7
#define STB0899_WIDTH_BYPASS_ADC           1
#define STB0899_INVADCICKOUT               (0x01 << 6)
#define STB0899_OFFST_INVADCICKOUT         6
#define STB0899_WIDTH_INVADCICKOUT         1
#define STB0899_ADCTEST_VOLTAGE            (0x03 << 4)
#define STB0899_OFFST_ADCTEST_VOLTAGE      4
#define STB0899_WIDTH_ADCTEST_VOLTAGE      1
#define STB0899_ADC_RESET                  (0x01 << 3)
#define STB0899_OFFST_ADC_RESET            3
#define STB0899_WIDTH_ADC_RESET            1
#define STB0899_TSTTNR1_2                  (0x01 << 2)
#define STB0899_OFFST_TSTTNR1_2            2
#define STB0899_WIDTH_TSTTNR1_2            1
#define STB0899_ADCPON                     (0x01 << 1)
#define STB0899_OFFST_ADCPON               1
#define STB0899_WIDTH_ADCPON               1
#define STB0899_ADCIN_MODE                 (0x01 << 0)
#define STB0899_OFFST_ADCIN_MODE           0
#define STB0899_WIDTH_ADCIN_MODE           1

#define STB0899_TSTTNR2                    0xf1e1
#define STB0899_TSTTNR2_7                  (0x01 << 7)
#define STB0899_OFFST_TSTTNR2_7            7
#define STB0899_WIDTH_TSTTNR2_7            1
#define STB0899_NOT_DISRX_WIRED            (0x01 << 6)
#define STB0899_OFFST_NOT_DISRX_WIRED      6
#define STB0899_WIDTH_NOT_DISRX_WIRED      1
#define STB0899_DISEQC_DCURRENT            (0x01 << 5)
#define STB0899_OFFST_DISEQC_DCURRENT      5
#define STB0899_WIDTH_DISEQC_DCURRENT      1
#define STB0899_DISEQC_ZCURRENT            (0x01 << 4)
#define STB0899_OFFST_DISEQC_ZCURRENT      4
#define STB0899_WIDTH_DISEQC_ZCURRENT      1
#define STB0899_DISEQC_SINC_SOURCE         (0x03 << 2)
#define STB0899_OFFST_DISEQC_SINC_SOURCE   2
#define STB0899_WIDTH_DISEQC_SINC_SOURCE   2
#define STB0899_SELIQSRC                   (0x03 << 0)
#define STB0899_OFFST_SELIQSRC             0
#define STB0899_WIDTH_SELIQSRC             2

#define STB0899_TSTTNR3                    0xf1e2

#define STB0899_I2CCFG                     0xf129
#define STB0899_I2CCFGRSVD                 (0x0f << 4)
#define STB0899_OFFST_I2CCFGRSVD           4
#define STB0899_WIDTH_I2CCFGRSVD           4
#define STB0899_I2CFASTMODE                (0x01 << 3)
#define STB0899_OFFST_I2CFASTMODE          3
#define STB0899_WIDTH_I2CFASTMODE          1
#define STB0899_STATUSWR                   (0x01 << 2)
#define STB0899_OFFST_STATUSWR             2
#define STB0899_WIDTH_STATUSWR             1
#define STB0899_I2CADDRINC                 (0x03 << 0)
#define STB0899_OFFST_I2CADDRINC           0
#define STB0899_WIDTH_I2CADDRINC           2

#define STB0899_I2CRPT                     0xf12a
#define STB0899_I2CTON                     (0x01 << 7)
#define STB0899_OFFST_I2CTON               7
#define STB0899_WIDTH_I2CTON               1
#define STB0899_ENARPTLEVEL                (0x01 << 6)
#define STB0899_OFFST_ENARPTLEVEL          6
#define STB0899_WIDTH_ENARPTLEVEL          2
#define STB0899_SCLTDELAY                  (0x01 << 3)
#define STB0899_OFFST_SCLTDELAY            3
#define STB0899_WIDTH_SCLTDELAY            1
#define STB0899_STOPENA                    (0x01 << 2)
#define STB0899_OFFST_STOPENA              2
#define STB0899_WIDTH_STOPENA              1
#define STB0899_STOPSDAT2SDA               (0x01 << 1)
#define STB0899_OFFST_STOPSDAT2SDA         1
#define STB0899_WIDTH_STOPSDAT2SDA         1

#define STB0899_IOPVALUE8                  0xf136
#define STB0899_IOPVALUE7                  0xf137
#define STB0899_IOPVALUE6                  0xf138
#define STB0899_IOPVALUE5                  0xf139
#define STB0899_IOPVALUE4                  0xf13a
#define STB0899_IOPVALUE3                  0xf13b
#define STB0899_IOPVALUE2                  0xf13c
#define STB0899_IOPVALUE1                  0xf13d
#define STB0899_IOPVALUE0                  0xf13e

#define STB0899_GPIO00CFG                  0xf140

#define STB0899_GPIO01CFG                  0xf141
#define STB0899_GPIO02CFG                  0xf142
#define STB0899_GPIO03CFG                  0xf143
#define STB0899_GPIO04CFG                  0xf144
#define STB0899_GPIO05CFG                  0xf145
#define STB0899_GPIO06CFG                  0xf146
#define STB0899_GPIO07CFG                  0xf147
#define STB0899_GPIO08CFG                  0xf148
#define STB0899_GPIO09CFG                  0xf149
#define STB0899_GPIO10CFG                  0xf14a
#define STB0899_GPIO11CFG                  0xf14b
#define STB0899_GPIO12CFG                  0xf14c
#define STB0899_GPIO13CFG                  0xf14d
#define STB0899_GPIO14CFG                  0xf14e
#define STB0899_GPIO15CFG                  0xf14f
#define STB0899_GPIO16CFG                  0xf150
#define STB0899_GPIO17CFG                  0xf151
#define STB0899_GPIO18CFG                  0xf152
#define STB0899_GPIO19CFG                  0xf153
#define STB0899_GPIO20CFG                  0xf154

#define STB0899_SDATCFG                    0xf155
#define STB0899_SCLTCFG                    0xf156
#define STB0899_AGCRFCFG                   0xf157
#define STB0899_GPIO22                     0xf158  /* AGCBB2CFG */
#define STB0899_GPIO21                     0xf159  /* AGCBB1CFG */
#define STB0899_DIRCLKCFG                  0xf15a
#define STB0899_CLKOUT27CFG                0xf15b
#define STB0899_STDBYCFG                   0xf15c
#define STB0899_CS0CFG                     0xf15d
#define STB0899_CS1CFG                     0xf15e
#define STB0899_DISEQCOCFG                 0xf15f

#define STB0899_GPIO32CFG                  0xf160
#define STB0899_GPIO33CFG                  0xf161
#define STB0899_GPIO34CFG                  0xf162
#define STB0899_GPIO35CFG                  0xf163
#define STB0899_GPIO36CFG                  0xf164
#define STB0899_GPIO37CFG                  0xf165
#define STB0899_GPIO38CFG                  0xf166
#define STB0899_GPIO39CFG                  0xf167

#define STB0899_IRQSTATUS_3                0xf120
#define STB0899_IRQSTATUS_2                0xf121
#define STB0899_IRQSTATUS_1                0xf122
#define STB0899_IRQSTATUS_0                0xf123

#define STB0899_IRQMSK_3                   0xf124
#define STB0899_IRQMSK_2                   0xf125
#define STB0899_IRQMSK_1                   0xf126
#define STB0899_IRQMSK_0                   0xf127

#define STB0899_IRQCFG                     0xf128

#define STB0899_GHOSTREG                   0xf000

#define STB0899_S2DEMOD                    0xf3fc
#define STB0899_S2FEC                      0xfafc

#endif  // __STB0899_REG_H
// vim:ts=4
