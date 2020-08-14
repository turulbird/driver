/*****************************************************************************
 *
 * proton.h
 *
 * (c) 2010 Spider-Team
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
 *****************************************************************************
 *
 * Spider-Box HL101 frontpanel driver.
 *
 * Devices:
 *	- /dev/vfd (vfd ioctls and read/write function)
 *	- /dev/rc  (reading of key events)
 *
 *****************************************************************************/
//extern static short paramDebug = 0;
#define TAGDEBUG "[proton] "
#define dprintk(level, x...) \
do \
{ \
	if ((paramDebug) && (paramDebug >= level) || level == 0) \
	printk(TAGDEBUG x); \
} while (0)


#define VFD_MAJOR             147
#define FRONTPANEL_MINOR_RC   1
#define LASTMINOR             2

/**************************************************
 *
 * IOCTL definitions
 *
 */

#define VFDBRIGHTNESS         0xc0425a03
#define VFDDRIVERINIT         0xc0425a08
#define VFDICONDISPLAYONOFF   0xc0425a0a
#define VFDDISPLAYWRITEONOFF  0xc0425a05
#define VFDDISPLAYCHARS       0xc0425a00

#define VFDGETWAKEUPMODE      0xc0425af9
#define VFDGETTIME            0xc0425afa
#define VFDSETTIME            0xc0425afb
#define VFDSTANDBY            0xc0425afc

#define VFDSETLED             0xc0425afe
#define VFDSETMODE            0xc0425aff
#define VFDDISPLAYCLR         0xc0425b00

/**************************************************
 *
 * Definitions for PT6311
 *
 */
// Commands
#define PT6311_DMODESET  0x00
// options for PT6311 display mode set command
#define PT6311_08DIGIT   0x00
#define PT6311_09DIGIT   0x08
#define PT6311_10DIGIT   0x09
#define PT6311_11DIGIT   0x0A
#define PT6311_12DIGIT   0x0B
#define PT6311_13DIGIT   0x0C
#define PT6311_14DIGIT   0x0D
#define PT6311_15DIGIT   0x0E
#define PT6311_16DIGIT   0x0F

#define PT6311_DATASET   0x40
// options for PT6311 dataset command
#define PT6311_TESTMODE  0x08  // opposite is normal mode
#define PT6311_FIXEDADR  0x04  // opposite is auto address increment
#define PT6311_READSW    0x03
#define PT6311_READKEY   0x02
#define PT6311_WRITELED  0x01
#define PT6311_WRITERAM  0x00

#define PT6311_DISP_CTL  0x80
// options for PT6311 address set command
#define PT6311_DISP_ON   0x08
// bit 0 -2 set display brightness
#define PT6311_DISPBR00  0x00
#define PT6311_DISPBRMIN PT6311_DISPBR00
#define PT6311_DISPBR01  0x01
#define PT6311_DISPBR02  0x02
#define PT6311_DISPBR03  0x03
#define PT6311_DISPBR04  0x04
#define PT6311_DISPBR05  0x05
#define PT6311_DISPBR06  0x06
#define PT6311_DISPBR07  0x07
#define PT6311_DISPBRMXX PT6311_DISPBR07

#define PT6311_ADDR_SET  0xC0
// options for PT6311 address set command
// bits 0 -5 set the address

/**************************************************
 *
 * Macro's for PT6311
 *
 */
#define VFD_CS_CLR()     {udelay(10); stpio_set_pin(cfg.cs, 0);}
#define VFD_CS_SET()     {udelay(10); stpio_set_pin(cfg.cs, 1);}

#define VFD_CLK_CLR()    {stpio_set_pin(cfg.clk, 0);udelay(4);}
#define VFD_CLK_SET()    {stpio_set_pin(cfg.clk, 1);udelay(4);}

#define VFD_DATA_CLR()   {stpio_set_pin(cfg.data, 0);}
#define VFD_DATA_SET()   {stpio_set_pin(cfg.data, 1);}

#define INVALID_KEY    	 -1
#define LOG_OFF     	 0
#define LOG_ON      	 1

#define NO_KEY_PRESS     -1
#define KEY_PRESS_DOWN 	 1
#define KEY_PRESS_UP   	 0

#define REC_NEW_KEY 	 34
#define REC_NO_KEY  	 0
#define REC_REPEAT_KEY   2

#define cMaxTransQueue   100
#define cMaxReceiveQueue 100

#define cMaxAckAttempts  150
#define cMaxQueueCount   5

#define BUFFERSIZE       256     //must be 2 ^ n

//#define ENABLE_SCROLL
//#define ENABLE_CLOCK_SECTION

typedef struct
{
	struct file      *fp;
	int              read;
	struct semaphore sem;

} tFrontPanelOpen;


typedef enum VFDMode_e
{
	VFDWRITEMODE,
	VFDREADMODE
} VFDMode_t;

typedef enum SegNum_e
{
	SEGNUM1 = 0,
	SEGNUM2
} SegNum_t;

typedef struct SegAddrVal_s
{
	unsigned char Segaddr1;
	unsigned char Segaddr2;
	unsigned char CurrValue1;
	unsigned char CurrValue2;
} SegAddrVal_t;

typedef enum PIO_Mode_e
{
	PIO_Out,
	PIO_In
} PIO_Mode_t;

struct VFD_config
{
	struct stpio_pin *clk;
	struct stpio_pin *data;
	struct stpio_pin *cs;
	int data_pin[2];
	int clk_pin[2];
	int cs_pin[2];
};

/**************************************************
 *
 * Comms definitions
 *
 */
struct VFD_config cfg;

/* structure to queue transmit data is necessary because
 * after most transmissions we need to wait for an acknowledge
 */
struct transmit_s
{
	unsigned char buffer[BUFFERSIZE];
	int           len;
	int           needAck; /* should we increase ackCounter? */

	int           ack_len; /* len of complete acknowledge sequence */
	int           ack_len_header; /* len of ack header ->contained in ack_buffer */
	unsigned char ack_buffer[BUFFERSIZE]; /* the ack sequence we wait for */

	int           requeueCount;
};

struct receive_s
{
	int           len;
	unsigned char buffer[BUFFERSIZE];
};

/**************************************************
 *
 * IOCTL structs
 *
 */
struct saved_data_s
{
	int   length;
	char  data[BUFFERSIZE];
};

struct set_brightness_s
{
	int level;
};

struct set_icon_s
{
	int icon_nr;
	int on;
};

struct set_led_s
{
	int led_nr;
	int on;
};

/* time must be given as follows:
 * time[0] & time[1] = mjd ???
 * time[2] = hour
 * time[3] = min
 * time[4] = sec
 */
struct set_standby_s
{
	char time[5];
};

struct set_time_s
{
	char time[5];
};

/* this sets up the mode temporarily (for one ioctl)
 * to the desired mode. Currently the "normal" mode
 * is the compatible vfd mode
 */
struct set_mode_s
{
	int compat; /* 0 = compatibility mode to vfd driver; 1 = nuvoton mode */
};

struct proton_ioctl_data
{
	union
	{
		struct set_icon_s icon;
		struct set_led_s led;
		struct set_brightness_s brightness;
		struct set_mode_s mode;
		struct set_standby_s standby;
		struct set_time_s time;
	} u;
};

struct vfd_ioctl_data
{
	unsigned char start_address;
	unsigned char data[64];
	unsigned char length;
};

typedef struct VFD_Format_s
{
	unsigned char LogNum;
	unsigned char LogSta;
} VFD_Format_c;

typedef struct VFD_Time_s
{
	unsigned char hour;
	unsigned char mint;
} VFD_Time_c;

/**************************************************
 *
 * Remote control definitions
 *
 */
typedef enum
{
	REMOTE_OLD = 0,
	REMOTE_NEW,
	REMOTE_TOPFIELD,
	REMOTE_UNKNOWN
} REMOTE_TYPE;

enum
{
	POWER_KEY               = 88,

	TIME_SET_KEY            = 87,
	UHF_KEY                 = 68,
	VFormat_KEY             = 67,
	MUTE_KEY                = 66,

	TVSAT_KEY               = 65,
	MUSIC_KEY               = 64,
	FIND_KEY                = 63,
	FAV_KEY                 = 62,

	MENU_KEY                = 102,	//HOME
	i_KEY                   = 61,
	EPG_KEY                 = 18,
	EXIT_KEY                = 48,	//B
	RECALL_KEY              = 30,
	RECORD_KEY              = 19,

	UP_KEY                  = 103,	//UP
	DOWN_KEY                = 108,	//DOWN
	LEFT_KEY                = 105,	//LEFT
	RIGHT_KEY               = 106,	//RIGTHT
	SELECT_KEY              = 0x160,	//ENTER

	PLAY_KEY                = 25,
	PAGE_UP_KEY             = 104,	//P_UP
	PAUSE_KEY               = 22,
	PAGE_DOWN_KEY           = 109,	//P_DOWN

	STOP_KEY                = 20,
	SLOW_MOTION_KEY         = 50,
	FASTREWIND_KEY          = 33,
	FASTFORWARD_KEY         = 49,

	DOCMENT_KEY             = 32,
	SWITCH_PIC_KEY          = 17,
	PALY_MODE_KEY           = 24,
	USB_KEY                 = 111,

	RADIO_KEY               = 110,
	SAT_KEY                 = 15,
	F1_KEY                  = 59,
	F2_KEY                  = 60,

	RED_KEY                 = 44,	//Z
	GREEN_KEY               = 45,	//X
	YELLOW_KEY              = 46,	//C
	BLUE_KEY                = 47	//V
};

enum
{
	KEY_DIGIT0 = 11,
	KEY_DIGIT1 = 2,
	KEY_DIGIT2 = 3,
	KEY_DIGIT3 = 4,
	KEY_DIGIT4 = 5,
	KEY_DIGIT5 = 6,
	KEY_DIGIT6 = 7,
	KEY_DIGIT7 = 8,
	KEY_DIGIT8 = 9,
	KEY_DIGIT9 = 10
};

/**************************************************
 *
 * Icon definitions
 *
 */
typedef enum LogNum_e
{
	/*----------------------------------11G-------------------------------------*/
	PLAY_FASTBACKWARD = 11 * 16 + 1,
	PLAY_HEAD,
	PLAY_LOG,
	PLAY_TAIL,
	PLAY_FASTFORWARD,
	PLAY_PAUSE,
	REC1,
	MUTE,
	CYCLE,
	DUBI,
	CA,
	CI,
	USB,
	DOUBLESCREEN,
	REC2,
	/*----------------------------------12G-------------------------------------*/
	HDD_A8 = 12 * 16 + 1,
	HDD_A7,
	HDD_A6,
	HDD_A5,
	HDD_A4,
	HDD_A3,
	HDD_FULL,
	HDD_A2,
	HDD_A1,
	MP3,
	AC3,
	TVMODE_LOG,
	AUDIO,
	ALERT,
	HDD_A9,
	/*----------------------------------13G-------------------------------------*/
	CLOCK_PM = 13 * 16 + 1,
	CLOCK_AM,
	CLOCK,
	TIME_SECOND,
	DOT2,
	STANDBY,
	TER,
	DISK_S3,
	DISK_S2,
	DISK_S1,
	DISK_S0,
	SAT,
	TIMESHIFT,
	DOT1,
	CAB,
	/*----------------------------------end-------------------------------------*/
	LogNum_Max
} LogNum_c;

#define BASE_VFD_PRIVATE 0x00

#define VFD_GetRevision _IOWR('s',(BASE_VFD_PRIVATE + 0), char *)
#define VFD_ShowLog     _IOWR('s',(BASE_VFD_PRIVATE + 1), VFD_Format_t)
#define VFD_ShowTime    _IOWR('s',(BASE_VFD_PRIVATE + 2), VFD_Time_t)
#define VFD_ShowStr     _IOWR('s',(BASE_VFD_PRIVATE + 3), char *)
#define VFD_ClearTime   _IOWR('s',(BASE_VFD_PRIVATE + 4), int)
// vim:ts=4
