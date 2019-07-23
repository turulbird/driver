/*
 */
#define TAGDEBUG "[adb_box_fp] "

#define dprintk(level, x...) do \
	 { \
		if (((paramDebug) && (paramDebug >= level)) || level == 0) \
		{ \
			printk(TAGDEBUG x); \
		} \
	} while (0)
#define ERR(fmt, args...) \
	printk(KERN_ERR "[vfd] :: " fmt "\n", ## args )
//------------------------------------------------
#define VFD_MAJOR           147

#define SCP_TXD_BIT           6
#define SCP_SCK_BIT           8
#define SCP_ENABLE_BIT        5
#define VFD_PORT              0
#define SCP_DATA              3
#define SCP_CLK               4
#define SCP_CS                5

#define PIO_PORT_SIZE       0x1000
#define PIO_BASE            0xb8020000
#define STPIO_SET_OFFSET    0x4
#define STPIO_CLEAR_OFFSET  0x8
#define STPIO_POUT_OFFSET   0x00
#define STPIO_SET_PIN(PIO_ADDR, PIN, V) \
	writel(1 << PIN, PIO_ADDR + STPIO_POUT_OFFSET + ((V) ? STPIO_SET_OFFSET : STPIO_CLEAR_OFFSET))
#define PIO_PORT(n) \
	(((n)*PIO_PORT_SIZE) + PIO_BASE)
//------------------------------------------------

// PT6302 (VFD)
#define ADB_BOX_VFD_MAX_CHARS      16

#define ADB_BOX_VFD_PIO_PORT_SCS    1
#define ADB_BOX_VFD_PIO_PIN_SCS     2

/*
  PIO assignments
*/
// PT6302 (VFD driver)
#define ADB_BOX_VFD_PIO_PORT_SCL    4
#define ADB_BOX_VFD_PIO_PIN_SCL     0

#define ADB_BOX_VFD_PIO_PORT_SDA    4
#define ADB_BOX_VFD_PIO_PIN_SDA     1

// display on/off values
#define PT6302_LIGHTS_NORMAL 0  // normal display of text
#define PT6302_LIGHTS_OFF    1  // all segments off
#define PT6302_LIGHTS_ON     3  // all segments on

// PT6958 (button device)
#define PT6958_BUTTON_PIO_PORT_DOUT 2
#define PT6958_BUTTON_PIO_PIN_DOUT  2

#define PT6958_BUTTON_PIO_PORT_STB  1
#define PT6958_BUTTON_PIO_PIN_STB   6

#define BUTTON_DO_PORT       PT6958_BUTTON_PIO_PORT_DOUT
#define BUTTON_DO_PIN        PT6958_BUTTON_PIO_PIN_DOUT

#define BUTTON_DS_PORT       PT6958_BUTTON_PIO_PORT_STB
#define BUTTON_DS_PIN        PT6958_BUTTON_PIO_PIN_STB

/**********************************************************************
 * PT6958 Internal RAM Size: 14 bytes
 *
 *  1 - Data Setting Command
 *  1 - Address Setting Command
 * 10 - Display Data
 *  1 - Display Control Command
 **********************************************************************/
#define PT6958_RAM_SIZE (1 + 1 + 10 + 1)

// Command definitions for PT6958
#define PT6958_CMD_ADDR_LED1   0xC1 //power LED
#define PT6958_CMD_ADDR_LED2   0xC3 //timer LED
//#define PT6958_LED_LARGE_A     (0x77|0x80)

#define PT6958_CMD_ADDR_LED3   0xC5 //mail/encrypted LED
#define PT6958_CMD_ADDR_LED4   0xC7 //alert LED

/**********************************************************************
 * CMD 40: Data Setting Command
 * bits:   0100 dcba
 *
 * ba - Data Write & Read Mode Settings:
 * 00: Write Data to Display Mode
 * 10: Read Key Data
 *
 *  c - Address Incremental Mode Settings (Display Mode):
 *  0: Increment Address after Data has been Written
 *  1: Fixed Address
 *
 *  d - Mode Settings
 *  0: Normal Operation Mode
 *  1: Test Mode
 **********************************************************************/
#define PT6958_CMD_WRITE_INC   0x40
#define PT6958_CMD_READ_KEY    0x42
#define PT6958_CMD_WRITE_FIXED 0x44
#define PT6958_CMD_MODE_NORMAL 0x40
#define PT6958_CMD_MODE_TEST   0x48
/**********************************************************************
 * CMD 80: Display Control Command
 * bits:   1000 dcba
 *
 * cba - Dimming Quantity Settings:
 * 000: Pulse width = 1/16
 * 001: Pulse width = 2/16
 * 010: Pulse width = 4/16
 * 011: Pulse width = 10/16
 * 100: Pulse width = 11/16
 * 101: Pulse width = 12/16
 * 110: Pulse width = 13/16
 * 111: Pulse width = 14/16
 *
 *   d - Display Settings
 *   0: Display Off (Key Scan Continues)
 *   1: Display On
 **********************************************************************/
#define PT6958_CMD_DISPLAY_OFF        0x80
#define PT6958_CMD_DISPLAY_OFF_DIM(x) (0x80 + x) //0x80-0x87
#define PT6958_CMD_DISPLAY_ON         0x8e //0x8E org adb_box
#define PT6958_CMD_DISPLAY_ON_DIM(x)  (0x88 + x) //0x88-0x8F
/**********************************************************************
 * CMD C0: Address Setting Command
 * bits:   1100 dcba
 *
 * 0,2,4,6 = segment 1,2,3,4
 * 1,3,5,7 = power, clock, mail, warning LED
 * 8,9     = not connected
 *
 * dcba - Address 0x00 to 0x09
 **********************************************************************/
#define PT6958_CMD_ADDR_SET    0xC0 // 0xC0 to 0xC9
//------------------------------------------------------------------------------
#define PT6958_CMD_ADDR_DISP1  0xC0 // 8.:.. 
#define PT6958_CMD_ADDR_DISP2  0xC2 // .8:..  
#define PT6958_CMD_ADDR_DISP3  0xC4 // ..:8. 
#define PT6958_CMD_ADDR_DISP4  0xC6 // ..:.8
/*
PIO 1.6 [fp_nload  ] [OUT (push-pull)    ] []	- STB
//PIO 2.2 [fp_key    ] [IN  (Hi-Z)         ] []	- DOUT
//PIO 3.3 [fp_ir_in  ] [IN  (Hi-Z)         ] [] - IRDA
PIO 4.0 [fp_clk    ] [Alt-BI (open-drain)] [] 	- CLK
PIO 4.1 [fp_data   ] [Alt-BI (open-drain)] [] 	- DIN
*/
#define FP_GETKEY               STPIO_GET_PIN(PIO_PORT(2),2) //4 ?

//------------------------------------------------

// Global variables
static int paramDebug = 0;
static int delay      = 5;
static int rec        = 0;  //distinguishes between white/black (bska/bxzb vs. bsla/bzzb)
static int led_POW    = 0x02;  //green

static char                    *button_driver_name = "PT6958 frontpanel buttons";
static struct workqueue_struct *button_wq;
static struct input_dev        *button_dev;
static struct stpio_pin        *button_do = NULL;
static struct stpio_pin        *button_ds = NULL;
static struct stpio_pin        *button_reset = NULL;

struct pt6302_driver
{
	struct scp_driver *scp;
};

struct scp_driver
{
	struct stpio_pin *scs;
	struct stpio_pin *scl;
	struct stpio_pin *sda;
//	struct stpio_pin *ske;
//	struct stpio_pin *snl;
};

//------------------------------------------------

struct vfd_ioctl_data
{
	unsigned char address;
	unsigned char data[64];
	unsigned char length;
};

struct __vfd_scp
{
	__u8 tr_rp_ctrl;
	__u8 rp_data;
	__u8 tr_data;
	__u8 start_tr;
	__u8 status;
	__u8 reserved;
	__u8 clk_div;
};

#define SCP_TXD_CTRL  (vfd_scp_ctrl->tr_rp_ctrl)
#define SCP_TXD_DATA  (vfd_scp_ctrl->tr_data)
#define SCP_TXD_START (vfd_scp_ctrl->start_tr)
#define SCP_STATUS    (vfd_scp_ctrl->status)

// from adb_box_pt6302.c
extern struct pt6302_driver *pt6302_init(struct scp_driver *scp);
extern void pt6302_free(struct pt6302_driver *ptd);
extern void pt6302_setup(struct pt6302_driver *pfd);
extern int pt6302_write_dcram(struct pt6302_driver *ptd, unsigned char addr, unsigned char *data, unsigned char len);
extern void pt6302_set_brightness(struct pt6302_driver *ptd, int level);
extern void pt6302_set_lights(struct pt6302_driver *ptd, int onoff);
// from adb_box_pt6958.c
extern int pt6958_write(unsigned char cmd, unsigned char *data, int data_len);
extern unsigned char pt6958_read_key(void);
extern int pt6958_led_control(unsigned char led, unsigned char set);
extern void pt6958_display(char *str);
// vim:ts=4
