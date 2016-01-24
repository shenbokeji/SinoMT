#include <linux/module.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/fb.h>
#include <asm/uaccess.h>
#include <linux/i2c.h>
#include <linux/i2c-id.h>
#include <linux/videodev2.h>
#include <mach/gpio.h>
#include <media/v4l2-device.h>
#include <media/v4l2-chip-ident.h>
#include <mach/hardware.h>

//#include <media/v4l2-i2c-drv.h>
#include <linux/miscdevice.h>  

#include <linux/delay.h>
#include <media/v4l2-subdev.h>
#include <media/davinci/videohd.h>



#define IT66121_DRIVER_NAME  "it66121_reg"
#define IT66121_DEVICE_MINOR   226

#define VIDSTD_TEST  4


MODULE_DESCRIPTION("ITW Devices IT66121 video HDMI encoder driver");
MODULE_AUTHOR("Dave Perks");
MODULE_LICENSE("GPL");

#include "it66121.h"


typedef enum _SYS_STATUS {
    ER_SUCCESS = 0,
    ER_FAIL,
    ER_RESERVED
} SYS_STATUS ;

// DDC Address
/////////////////////////////////////////
#define DDC_HDCP_ADDRESS 0x74
#define DDC_EDID_ADDRESS 0xA0
#define DDC_FIFO_MAXREQ 0x20



static int debug;
module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "Debug level (0-1)");

//static unsigned char buf[2]={};

#define TX0ADR		0x9A
//#define TX0DEV  	0x00
#define TX0CECADR   0x9C
#define RXADR   	0x90

#define SUPPORT_HDCP



static HDMITXDEV InstanceData =
{

    0,      // BYTE I2C_DEV ;
    HDMI_TX_I2C_SLAVE_ADDR,    // BYTE I2C_ADDR ;

    /////////////////////////////////////////////////
    // Interrupt Type
    /////////////////////////////////////////////////
    0x40,      // BYTE bIntType ; // = 0 ;
    /////////////////////////////////////////////////
    // Video Property
    /////////////////////////////////////////////////
    INPUT_SIGNAL_TYPE ,// BYTE bInputVideoSignalType ; // for Sync Embedded,CCIR656,InputDDR

    /////////////////////////////////////////////////
    // Audio Property
    /////////////////////////////////////////////////
    0, // BYTE bOutputAudioMode ; // = 0 ;
    FALSE , // BYTE bAudioChannelSwap ; // = 0 ;
    0x01, // BYTE bAudioChannelEnable ;
    0 ,// BYTE bAudFs ;
    0, // unsigned long TMDSClock ;
    FALSE, // BYTE bAuthenticated:1 ;
    FALSE, // BYTE bHDMIMode: 1;
    FALSE, // BYTE bIntPOL:1 ; // 0 = Low Active
    FALSE, // BYTE bHPD:1 ;
};

// HDMI Video Timing
struct hdmi_video_timing {
	struct fb_videomode mode;	// Video timing
	unsigned int vic;			// Video information code
	unsigned int pixelrepeat;	// Video pixel repeat rate
	unsigned int interface;		// Video input interface
};


static const struct hdmi_video_timing hdmi_mode [] = {
		//name				refresh		xres	yres	pixclock	h_bp	h_fp	v_bp	v_fp	h_pw	v_pw					polariry								PorI	flag	vic	pixelrepeat	interface
	{ {	"720x480p@60Hz",	60,			720,	480,	27000000,	60,		16,		30,		9,		62,		6,							0,									0,		0	},	2,  	1,		OUT_P888	},
	{ {	"720x576p@50Hz",	50,			720,	576,	27000000,	68,		12,		39,		5,		64,		5,							0,									0,		0	},	17,  	1,		OUT_P888	},
	{ {	"1280x720p@50Hz",	50,			1280,	720,	74250000,	220,	440,	20,		5,		40,		5,		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,			0,		0	},	19,  	1,		OUT_P888	},
	{ {	"1280x720p@60Hz",	60,			1280,	720,	74250000,	220,	110,	20,		5,		40,		5,		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,			0,		0	},	4,  	1,		OUT_P888	},
	{ {	"1920x1080p@50Hz",	50,			1920,	1080,	148500000,	148,	528,	36,		4,		44,		5,		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,			0,		0	},	31,  	1,		OUT_P888	},
	{ {	"1920x1080p@60Hz",	60,			1920,	1080,	148500000,	148,	88,		36,		4,		44,		5,		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,			0,		0	},	16,  	1,		OUT_P888	},		
	{ { "1920x1080p@30Hz",	30, 		1920,	1080,	148500000,	148,	88, 	36, 	4,		44, 	5,		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,			0,		0	},	34, 	1,		OUT_P888	},		

};


static RegSetEntry HDMITX_Init_Table[] = {

    {0x0F, 0x40, 0x00},

    {0x62, 0x08, 0x00},
    {0x64, 0x04, 0x00},
    {0x01,0x00,0x00},//idle(100);

    {0x04, 0x20, 0x20},
    {0x04, 0x1D, 0x1D},
    {0x01,0x00,0x00},//idle(100);
    {0x0F, 0x01, 0x00}, // bank 0 ;
    #ifdef INIT_CLK_LOW
        {0x62, 0x90, 0x10},
        {0x64, 0x89, 0x09},
        {0x68, 0x10, 0x10},
    #endif

    {0xD1, 0x0E, 0x0C},
    {0x65, 0x03, 0x00},
    #ifdef NON_SEQUENTIAL_YCBCR422 // for ITE HDMIRX
        {0x71, 0xFC, 0x1C},
    #else
        {0x71, 0xFC, 0x18},
    #endif

    {0x8D, 0xFF, CEC_I2C_SLAVE_ADDR},
    {0x0F, 0x08, 0x08},

    {0xF8,0xFF,0xC3},
    {0xF8,0xFF,0xA5},
    {0x20, 0x80, 0x80},
    {0x37, 0x01, 0x00},
    {0x20, 0x80, 0x00},
    {0xF8,0xFF,0xFF},

    {0x59, 0xD8, 0x40|PCLKINV},
    {0xE1, 0x20, InvAudCLK},
    {0x05, 0xC0, 0x40},
    {REG_TX_INT_MASK1, 0xFF, ~(B_TX_RXSEN_MASK|B_TX_HPD_MASK)},
    {REG_TX_INT_MASK2, 0xFF, ~(B_TX_KSVLISTCHK_MASK|B_TX_AUTH_DONE_MASK|B_TX_AUTH_FAIL_MASK)},
    {REG_TX_INT_MASK3, 0xFF, ~(B_TX_VIDSTABLE_MASK)},
    {0x0C, 0xFF, 0xFF},
    {0x0D, 0xFF, 0xFF},
    {0x0E, 0x03, 0x03},

    {0x0C, 0xFF, 0x00},
    {0x0D, 0xFF, 0x00},
    {0x0E, 0x02, 0x00},
    {0x09, 0x03, 0x00}, // Enable HPD and RxSen Interrupt
    {0,0,0}
};



static RegSetEntry HDMITX_DefaultVideo_Table[] = {

    ////////////////////////////////////////////////////
    // Config default output format.
    ////////////////////////////////////////////////////
    {0x72, 0xff, 0x00},
    {0x70, 0xff, 0x00},
#ifndef DEFAULT_INPUT_YCBCR
// GenCSC\RGB2YUV_ITU709_16_235.c
    {0x72, 0xFF, 0x02},
    {0x73, 0xFF, 0x00},
    {0x74, 0xFF, 0x80},
    {0x75, 0xFF, 0x00},
    {0x76, 0xFF, 0xB8},
    {0x77, 0xFF, 0x05},
    {0x78, 0xFF, 0xB4},
    {0x79, 0xFF, 0x01},
    {0x7A, 0xFF, 0x93},
    {0x7B, 0xFF, 0x00},
    {0x7C, 0xFF, 0x49},
    {0x7D, 0xFF, 0x3C},
    {0x7E, 0xFF, 0x18},
    {0x7F, 0xFF, 0x04},
    {0x80, 0xFF, 0x9F},
    {0x81, 0xFF, 0x3F},
    {0x82, 0xFF, 0xD9},
    {0x83, 0xFF, 0x3C},
    {0x84, 0xFF, 0x10},
    {0x85, 0xFF, 0x3F},
    {0x86, 0xFF, 0x18},
    {0x87, 0xFF, 0x04},
#else
// GenCSC\YUV2RGB_ITU709_16_235.c
    {0x0F, 0x01, 0x00},
    {0x72, 0xFF, 0x03},
    {0x73, 0xFF, 0x00},
    {0x74, 0xFF, 0x80},
    {0x75, 0xFF, 0x00},
    {0x76, 0xFF, 0x00},
    {0x77, 0xFF, 0x08},
    {0x78, 0xFF, 0x53},
    {0x79, 0xFF, 0x3C},
    {0x7A, 0xFF, 0x89},
    {0x7B, 0xFF, 0x3E},
    {0x7C, 0xFF, 0x00},
    {0x7D, 0xFF, 0x08},
    {0x7E, 0xFF, 0x51},
    {0x7F, 0xFF, 0x0C},
    {0x80, 0xFF, 0x00},
    {0x81, 0xFF, 0x00},
    {0x82, 0xFF, 0x00},
    {0x83, 0xFF, 0x08},
    {0x84, 0xFF, 0x00},
    {0x85, 0xFF, 0x00},
    {0x86, 0xFF, 0x87},
    {0x87, 0xFF, 0x0E},
#endif
    // 2012/12/20 added by Keming's suggestion test
    {0x88, 0xF0, 0x00},
    //~jauchih.tseng@ite.com.tw
    {0x04, 0x08, 0x00},
    {0,0,0}
};


static RegSetEntry HDMITX_SetHDMI_Table[] = {

    ////////////////////////////////////////////////////
    // Config default HDMI Mode
    ////////////////////////////////////////////////////
    {0xC0, 0x01, 0x01},
    {0xC1, 0x03, 0x03},
    {0xC6, 0x03, 0x03},
    {0,0,0}
};

static RegSetEntry HDMITX_PwrDown_Table[] = {
     // Enable GRCLK
     {0x0F, 0x40, 0x00},
     // PLL Reset
     {0x61, 0x10, 0x10},   // DRV_RST
     {0x62, 0x08, 0x00},   // XP_RESETB
     {0x64, 0x04, 0x00},   // IP_RESETB
     {0x01, 0x00, 0x00}, // idle(100);

     // PLL PwrDn
     {0x61, 0x20, 0x20},   // PwrDn DRV
     {0x62, 0x44, 0x44},   // PwrDn XPLL
     {0x64, 0x40, 0x40},   // PwrDn IPLL

     // HDMITX PwrDn
     {0x05, 0x01, 0x01},   // PwrDn PCLK
     {0x0F, 0x78, 0x78},   // PwrDn GRCLK
     {0x00, 0x00, 0x00} // End of Table.
};

static RegSetEntry HDMITX_PwrOn_Table[] = {
    {0x0F, 0x78, 0x38},   // PwrOn GRCLK
    {0x05, 0x01, 0x00},   // PwrOn PCLK

    // PLL PwrOn
    {0x61, 0x20, 0x00},   // PwrOn DRV
    {0x62, 0x44, 0x00},   // PwrOn XPLL
    {0x64, 0x40, 0x00},   // PwrOn IPLL

    // PLL Reset OFF
    {0x61, 0x10, 0x00},   // DRV_RST
    {0x62, 0x08, 0x08},   // XP_RESETB
    {0x64, 0x04, 0x04},   // IP_RESETB
    {0x0F, 0x78, 0x08},   // PwrOn IACLK
    {0x00, 0x00, 0x00} // End of Table.
};


/****************************************************************************
			I2C Client & Driver
 ****************************************************************************/
struct it66121_state {
	struct v4l2_subdev sd;
};
static struct v4l2_subdev *it66121_sd;


#define I2C_RETRY_COUNT                 (5)

int it66121_read(unsigned char address)
{
	{
		int err, retry = 0;
		struct i2c_client *client = v4l2_get_subdevdata(it66121_sd);
	
	read_again:
	
		err = i2c_smbus_read_byte_data(client, address);
		if (err == -1) {
			if (retry <= I2C_RETRY_COUNT) {
				v4l2_warn(it66121_sd, "Read: retry ... %d\n", retry);
				retry++;
				msleep_interruptible(10);
				goto read_again;
			}
		}
	
		return err;
	}

}

int it66121_write(unsigned char address, unsigned char value)
{
		int err, retry = 0;
		struct i2c_client *client = v4l2_get_subdevdata(it66121_sd);
	
	write_again:
	
		err = i2c_smbus_write_byte_data(client, address, value);
		if (err) {
			if (retry <= I2C_RETRY_COUNT) {
				v4l2_warn(it66121_sd, "Write: retry ... %d\n", retry);
				retry++;
				msleep_interruptible(10);
				goto write_again;
			}
		}
	
		return err;

}

int HDMITX_SetI2C_Byte(BYTE Reg,BYTE Mask,BYTE Value)
{
    BYTE Temp;
    if( Mask != 0xFF )
    {
        Temp=it66121_read(Reg);
        Temp&=(~Mask);
        Temp|=Value&Mask;
    }
    else
    {
        Temp=Value;
    }
    return it66121_write(Reg,Temp);
}

void it66121_reset(void)
{
	//apply 5V 
	__raw_writel(0x00FC0001,IO_ADDRESS(0x01c40000));
	//gpio_request(103, "IT66121_POWER_GPIO");
	//gpio_direction_output(103, 1);
	__raw_writel(0xFFFFFF7F,IO_ADDRESS(0x01c67088));
	__raw_writel(0x80,IO_ADDRESS(0x01c6708C));
	return;

}
static int it66121_device_init(void);

static ssize_t it66121_open(struct inode * indoe, struct file * file)
{
	return 0;
}

static ssize_t it66121_close(struct inode * indoe, struct file * file)
{
	return 0;
}


typedef struct IT66121RegStruct {
    unsigned int uiAddr;	//I2C reg, address  
    unsigned int uiValue;		//I2C bus,data
} tIT66121Reg;

static tIT66121Reg it66121_reg={0};


//static unsigned int rw_buf=0;
ssize_t it66121_readreg(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{
	int iRet;
	iRet = copy_from_user( &it66121_reg, buf, 8 );
	it66121_reg.uiValue = it66121_read((unsigned char)it66121_reg.uiAddr);
	iRet = copy_to_user( (void*)buf, &it66121_reg, 8 ); 
	return 8;
}

ssize_t it66121_writereg(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{
    int iRet;
    iRet = copy_from_user(&it66121_reg, buf, 8);
    it66121_write((unsigned char)it66121_reg.uiAddr,(unsigned char)it66121_reg.uiValue);
     
    return 8;

}

/*
 * Initialization settings for HDMI Mode.
 */

#define GET_INFO	0U
#define SET_INFO	1U

#define IT66121_CMD_SIZE	2U



static int it66121_ioctl(struct inode *inode, struct file *file,
		     u_int cmd, u_long arg)
{
	int ret = 0;

	switch (cmd) {
	
		default:
			ret = -ENOTTY;
	}

	return ret;
}


void HDMITX_PowerOn(void)
{
    hdmitx_LoadRegSetting(HDMITX_PwrOn_Table);
}

void HDMITX_PowerDown(void)
{
    hdmitx_LoadRegSetting(HDMITX_PwrDown_Table);
    hdmiTxDev[0].bHPD = 0;
}

void setHDMITX_AVMute(BYTE bEnable)
{
    Switch_HDMITX_Bank(0);
    HDMITX_SetI2C_Byte(REG_TX_GCP,B_TX_SETAVMUTE, bEnable?B_TX_SETAVMUTE:0 );
    it66121_write(REG_TX_PKT_GENERAL_CTRL,B_TX_ENABLE_PKT|B_TX_REPEAT_PKT);
}



//////////////////////////////////////////////////////////////////////
// Function: hdmitx_LoadRegSetting()
// Input: RegSetEntry SettingTable[] ;
// Return: N/A
// Remark: if an entry {0, 0, 0} will be terminated.
//////////////////////////////////////////////////////////////////////

void hdmitx_LoadRegSetting(RegSetEntry table[])
{
    int i ;

	printk("table addr =%x\n",(int)table);
    for( i = 0 ;  ; i++ )
    {
    	//printk("table i =%x\n", i);
        if( table[i].offset == 0 && table[i].invAndMask == 0 && table[i].OrMask == 0 )
        {
            return ;
        }
        else if( table[i].invAndMask == 0 && table[i].OrMask == 0 )
        {
        	//printk("table case 1\n");
            printk("delay(%d)\n",(int)table[i].offset);
            mdelay(table[i].offset);
        }
        else if( table[i].invAndMask == 0xFF )
        {
        	//printk("table case 2\n");
            printk("it66121_write(%02x,%02x)\n",(int)table[i].offset,(int)table[i].OrMask);
            it66121_write(table[i].offset,table[i].OrMask);
        }
        else
        {
        	//printk("table case 3\n");
            printk("HDMITX_SetI2C_Byte(%02x,%02x,%02x)\n",table[i].offset,table[i].invAndMask,table[i].OrMask);
            HDMITX_SetI2C_Byte(table[i].offset,table[i].invAndMask,table[i].OrMask);
        }
    }
}


void HDMITX_InitTxDev(HDMITXDEV *pInstance)
{
	if(pInstance && 0 < HDMITX_MAX_DEV_COUNT)
	{
		hdmiTxDev[0] = *pInstance ;
	}
}

#if 1
void DumpHDMITXReg(void)
{
    int i,j ;
    BYTE ucData = 0;

    printk("       ");
    for(j = 0 ; j < 16 ; j++)
    {
        printk( "%#X",(int)j);
        if((j == 3)||(j==7)||(j==11))
        {
            printk("  ");
        }
    }
    printk("\n        -----------------------------------------------------\n");

    Switch_HDMITX_Bank(0);

    for(i = 0 ; i < 0x100 ; i+=16)
    {
        printk("[%#X]  ",i);
        for(j = 0 ; j < 16 ; j++)
        {
            if( (i+j)!= 0x17)
            {
                ucData = it66121_read((BYTE)((i+j)&0xFF));
            }
            else
            {

            }
            printk( "%#X",(int)ucData); // for DDC FIFO
            if((j == 3)||(j==7)||(j==11))
            {
                printk(" -");
            }
        }
        printk("\n");
        if((i % 0x40) == 0x30)
        {
            printk("        -----------------------------------------------------\n");
        }
    }
    Switch_HDMITX_Bank(1);
    for(i = 0x130; i < 0x200 ; i+=16)
    {
        printk("[%3X]  ",i);
        for(j = 0 ; j < 16 ; j++)
        {
            ucData = it66121_read((BYTE)((i+j)&0xFF));
            printk(" %02X",(int)ucData);
            if((j == 3)||(j==7)||(j==11))
            {
                printk(" -");
            }
        }
        printk("\n");
        if((i % 0x40) == 0x20)
        {
            printk("        -----------------------------------------------------\n");
        }
    }
            printk("        -----------------------------------------------------\n");
    Switch_HDMITX_Bank(0);
}

#endif





void InitHDMITX(void)
{
	printk("Init HDMITX in\n");


    hdmitx_LoadRegSetting(HDMITX_Init_Table);
	printk("Init HDMITX_Init_Table \n");
    it66121_write(REG_TX_PLL_CTRL,0xff);
    hdmiTxDev[0].bIntPOL = (hdmiTxDev[0].bIntType&B_TX_INTPOL_ACTH)?TRUE:FALSE ;

    // Avoid power loading in un play status.
	//////////////////////////////////////////////////////////////////
	// Setup HDCP ROM
	//////////////////////////////////////////////////////////////////
#ifdef HDMITX_INPUT_INFO
   // hdmiTxDev[0].RCLK = CalcRCLK();
#endif
	printk("Init HDMITX_DefaultVideo_Table \n");

    hdmitx_LoadRegSetting(HDMITX_DefaultVideo_Table);
	printk("Init HDMITX_SetHDMI_Table \n");
    hdmitx_LoadRegSetting(HDMITX_SetHDMI_Table);
   // hdmitx_LoadRegSetting(HDMITX_DefaultAVIInfo_Table);
   // hdmitx_LoadRegSetting(HDMITX_DeaultAudioInfo_Table);
    //hdmitx_LoadRegSetting(HDMITX_Aud_CHStatus_LPCM_20bit_48Khz);
    ///hdmitx_LoadRegSetting(HDMITX_AUD_SPDIF_2ch_24bit);
    HDMITX_PowerDown();

    printk("Init HDMITX\n");

    DumpHDMITXReg();

	
}

/**
 * hdmi_vic_to_videomode: transverse vic mode to video mode
 * @vmode: vic to transverse
 * 
 */
const struct fb_videomode* hdmi_vic_to_videomode(int vic)
{
	int i;
	
	if(vic == 0)
		return NULL;
	
	for(i = 0; i < ARRAY_SIZE(hdmi_mode); i++)
	{
		if(hdmi_mode[i].vic == vic)
			return &hdmi_mode[i].mode;
	}
	return NULL;
}

static void hdmitx_SetInputMode(BYTE InputColorMode,BYTE bInputSignalType)
{
    BYTE ucData ;

    ucData = it66121_read(REG_TX_INPUT_MODE);
    ucData &= ~(M_TX_INCOLMOD|B_TX_2X656CLK|B_TX_SYNCEMB|B_TX_INDDR|B_TX_PCLKDIV2);
    ucData |= 0x01;//input clock delay 1 for 1080P DDR

    switch(InputColorMode & F_MODE_CLRMOD_MASK)
    {
    case F_MODE_YUV422:
        ucData |= B_TX_IN_YUV422 ;
        break ;
    case F_MODE_YUV444:
        ucData |= B_TX_IN_YUV444 ;
        break ;
    case F_MODE_RGB444:
    default:
        ucData |= B_TX_IN_RGB ;
        break ;
    }
    if(bInputSignalType & T_MODE_PCLKDIV2)
    {
        ucData |= B_TX_PCLKDIV2 ; printk("PCLK Divided by 2 mode\n");
    }
    if(bInputSignalType & T_MODE_CCIR656)
    {
        ucData |= B_TX_2X656CLK ; printk("CCIR656 mode\n");
    }
    if(bInputSignalType & T_MODE_SYNCEMB)
    {
        ucData |= B_TX_SYNCEMB ; printk("Sync Embedded mode\n");
    }
    if(bInputSignalType & T_MODE_INDDR)
    {
        ucData |= B_TX_INDDR ; printk("Input DDR mode\n");
    }
    it66121_write(REG_TX_INPUT_MODE,ucData);
}

//////////////////////////////////////////////////////////////////////
// export this for dynamic change input signal
//////////////////////////////////////////////////////////////////////
BOOL setHDMITX_VideoSignalType(BYTE inputSignalType)
{
	hdmiTxDev[0].bInputVideoSignalType = inputSignalType ;
    // hdmitx_SetInputMode(inputColorMode,hdmiTxDev[0].bInputVideoSignalType);
    return TRUE ;
}
//////////////////////////////////////////////////////////////////////
// Function Body.
//////////////////////////////////////////////////////////////////////

void HDMITX_DisableVideoOutput(void)
{
    BYTE uc = it66121_read(REG_TX_SW_RST) | B_HDMITX_VID_RST ;
    it66121_write(REG_TX_SW_RST,uc);
    it66121_write(REG_TX_AFE_DRV_CTRL,B_TX_AFE_DRV_RST|B_TX_AFE_DRV_PWD);
    HDMITX_SetI2C_Byte(0x62, 0x90, 0x00);
    HDMITX_SetI2C_Byte(0x64, 0x89, 0x00);
}

//////////////////////////////////////////////////////////////////////
// Function: hdmitx_SetupAFE
// Parameter: VIDEOPCLKLEVEL level
//            PCLK_LOW - for 13.5MHz (for mode less than 1080p)
//            PCLK MEDIUM - for 25MHz~74MHz
//            PCLK HIGH - PCLK > 80Hz (for 1080p mode or above)
// Return: N/A
// Remark: set reg62~reg65 depended on HighFreqMode
//         reg61 have to be programmed at last and after video stable input.
// Side-Effect:
//////////////////////////////////////////////////////////////////////

static void hdmitx_SetupAFE(VIDEOPCLKLEVEL level)
{

    it66121_write(REG_TX_AFE_DRV_CTRL,B_TX_AFE_DRV_RST);/* 0x10 */
    switch(level)
    {
        case PCLK_HIGH:
            HDMITX_SetI2C_Byte(0x62, 0x90, 0x80);
            HDMITX_SetI2C_Byte(0x64, 0x89, 0x80);
            HDMITX_SetI2C_Byte(0x68, 0x10, 0x80);
            printk("hdmitx_SetupAFE()===================HIGHT\n");
            break ;
        default:
            HDMITX_SetI2C_Byte(0x62, 0x90, 0x10);
            HDMITX_SetI2C_Byte(0x64, 0x89, 0x09);
            HDMITX_SetI2C_Byte(0x68, 0x10, 0x10);
            printk("hdmitx_SetupAFE()===================LOW\n");
            break ;
    }
    HDMITX_SetI2C_Byte(REG_TX_SW_RST,B_TX_REF_RST_HDMITX|B_HDMITX_VID_RST,0);
    it66121_write(REG_TX_AFE_DRV_CTRL,0);
    mdelay(1);
}

//////////////////////////////////////////////////////////////////////
// Function: hdmitx_FireAFE
// Parameter: N/A
// Return: N/A
// Remark: write reg61 with 0x04
//         When program reg61 with 0x04,then audio and video circuit work.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static void hdmitx_FireAFE(void)
{
    Switch_HDMITX_Bank(0);
    it66121_write(REG_TX_AFE_DRV_CTRL,0);
}

BOOL HDMITX_EnableVideoOutput(VIDEOPCLKLEVEL level,BYTE inputColorMode,BYTE outputColorMode,BYTE bHDMI)
{
    // bInputVideoMode,bOutputVideoMode,hdmiTxDev[0].bInputVideoSignalType,bAudioInputType,should be configured by upper F/W or loaded from EEPROM.
    // should be configured by initsys.c
    // VIDEOPCLKLEVEL level ;

    it66121_write(REG_TX_SW_RST,B_HDMITX_VID_RST|B_HDMITX_AUD_RST|B_TX_AREF_RST|B_TX_HDCP_RST_HDMITX);

    hdmiTxDev[0].bHDMIMode = (BYTE)bHDMI ;
    // 2009/12/09 added by jau-chih.tseng@ite.com.tw
    Switch_HDMITX_Bank(1);
    it66121_write(REG_TX_AVIINFO_DB1,0x00);
    Switch_HDMITX_Bank(0);
    //~jau-chih.tseng@ite.com.tw

    if(hdmiTxDev[0].bHDMIMode)
    {
        setHDMITX_AVMute(TRUE);
    }
    hdmitx_SetInputMode(inputColorMode,hdmiTxDev[0].bInputVideoSignalType);

    //hdmitx_SetCSCScale(inputColorMode,outputColorMode);
    //set csc = B_HDMITX_CSC_BYPASS 
    HDMITX_SetI2C_Byte(0xF, 0x10, 0x10);
	it66121_write(REG_TX_CSC_CTRL,0);

    if(hdmiTxDev[0].bHDMIMode)
    {
        it66121_write(REG_TX_HDMI_MODE,B_TX_HDMI_MODE);
    }
    else
    {
        it66121_write(REG_TX_HDMI_MODE,B_TX_DVI_MODE);
    }
#ifdef INVERT_VID_LATCHEDGE
    uc = it66121_read(REG_TX_CLK_CTRL1);
    uc |= B_TX_VDO_LATCH_EDGE ;
    it66121_write(REG_TX_CLK_CTRL1, uc);
#endif

    hdmitx_SetupAFE(level); // pass if High Freq request
    it66121_write(REG_TX_SW_RST,          B_HDMITX_AUD_RST|B_TX_AREF_RST|B_TX_HDCP_RST_HDMITX);

    hdmitx_FireAFE();

	return TRUE ;
}


static unsigned char CommunBuff[128] ;

static void ConfigAVIInfoFrame(BYTE VIC, BYTE pixelrep, BYTE aspec, BYTE Colorimetry, BYTE bOutputColorMode)
{
    AVI_InfoFrame *AviInfo;
    AviInfo = (AVI_InfoFrame *)CommunBuff ;

    AviInfo->pktbyte.AVI_HB[0] = AVI_INFOFRAME_TYPE|0x80 ;
    AviInfo->pktbyte.AVI_HB[1] = AVI_INFOFRAME_VER ;
    AviInfo->pktbyte.AVI_HB[2] = AVI_INFOFRAME_LEN ;

    switch(bOutputColorMode)
    {
    case F_MODE_YUV444:
        // AviInfo->info.ColorMode = 2 ;
        AviInfo->pktbyte.AVI_DB[0] = (2<<5)|(1<<4);
        break ;
    case F_MODE_YUV422:
        // AviInfo->info.ColorMode = 1 ;
        AviInfo->pktbyte.AVI_DB[0] = (1<<5)|(1<<4);
        break ;
    case F_MODE_RGB444:
    default:
        // AviInfo->info.ColorMode = 0 ;
        AviInfo->pktbyte.AVI_DB[0] = (0<<5)|(1<<4);
        break ;
    }
    AviInfo->pktbyte.AVI_DB[1] = 8 ;
    AviInfo->pktbyte.AVI_DB[1] |= (aspec != HDMI_16x9)?(1<<4):(2<<4); // 4:3 or 16:9
    AviInfo->pktbyte.AVI_DB[1] |= (Colorimetry != HDMI_ITU709)?(1<<6):(2<<6); // 4:3 or 16:9
    AviInfo->pktbyte.AVI_DB[2] = 0 ;
    AviInfo->pktbyte.AVI_DB[3] = VIC ;
    AviInfo->pktbyte.AVI_DB[4] =  pixelrep & 3 ;
    AviInfo->pktbyte.AVI_DB[5] = 0 ;
    AviInfo->pktbyte.AVI_DB[6] = 0 ;
    AviInfo->pktbyte.AVI_DB[7] = 0 ;
    AviInfo->pktbyte.AVI_DB[8] = 0 ;
    AviInfo->pktbyte.AVI_DB[9] = 0 ;
    AviInfo->pktbyte.AVI_DB[10] = 0 ;
    AviInfo->pktbyte.AVI_DB[11] = 0 ;
    AviInfo->pktbyte.AVI_DB[12] = 0 ;

    HDMITX_EnableAVIInfoFrame(TRUE, (unsigned char *)AviInfo);
}

int hdmitx_SetVSIInfoFrame(VendorSpecific_InfoFrame *pVSIInfoFrame)
{
    
    unsigned char ucData=0 ;

    if(!pVSIInfoFrame)
    {
        return -1 ;
    }

    Switch_HDMITX_Bank(1);
    it66121_write(0x80,pVSIInfoFrame->pktbyte.VS_DB[3]);
    it66121_write(0x81,pVSIInfoFrame->pktbyte.VS_DB[4]);

    ucData -= pVSIInfoFrame->pktbyte.VS_DB[3] ;
    ucData -= pVSIInfoFrame->pktbyte.VS_DB[4] ;

    if(  pVSIInfoFrame->pktbyte.VS_DB[4] & (1<<7 ))
    {
        ucData -= pVSIInfoFrame->pktbyte.VS_DB[5] ;
        it66121_write(0x82,pVSIInfoFrame->pktbyte.VS_DB[5]);
        ucData -= VENDORSPEC_INFOFRAME_TYPE + VENDORSPEC_INFOFRAME_VER + 6 + 0x0C + 0x03 ;
    }
    else
    {
        ucData -= VENDORSPEC_INFOFRAME_TYPE + VENDORSPEC_INFOFRAME_VER + 5 + 0x0C + 0x03 ;
    }

    pVSIInfoFrame->pktbyte.CheckSum=ucData;

    it66121_write(0x83,pVSIInfoFrame->pktbyte.CheckSum);
    Switch_HDMITX_Bank(0);
    it66121_write(REG_TX_3D_INFO_CTRL,B_TX_ENABLE_PKT|B_TX_REPEAT_PKT);
    return 0 ;
}

BOOL HDMITX_EnableVSInfoFrame(BYTE bEnable,BYTE *pVSInfoFrame)
{
    if(!bEnable)
    {
        hdmitx_DISABLE_VSDB_PKT();
        return TRUE ;
    }
    if(hdmitx_SetVSIInfoFrame((VendorSpecific_InfoFrame *)pVSInfoFrame) == 0)
    {
        return TRUE ;
    }
    return FALSE ;
}

//////////////////////////////////////////////////////////////////////
// Function: hdmitx_SetAVIInfoFrame()
// Parameter: pAVIInfoFrame - the pointer to HDMI AVI Infoframe ucData
// Return: N/A
// Remark: Fill the AVI InfoFrame ucData,and count checksum,then fill into
//         AVI InfoFrame registers.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

int hdmitx_SetAVIInfoFrame(AVI_InfoFrame *pAVIInfoFrame)
{
    int i ;
    unsigned char checksum ;

    if(!pAVIInfoFrame)
    {
        return -1 ;
    }
    Switch_HDMITX_Bank(1);
    it66121_write(REG_TX_AVIINFO_DB1,pAVIInfoFrame->pktbyte.AVI_DB[0]);
    it66121_write(REG_TX_AVIINFO_DB2,pAVIInfoFrame->pktbyte.AVI_DB[1]);
    it66121_write(REG_TX_AVIINFO_DB3,pAVIInfoFrame->pktbyte.AVI_DB[2]);
    it66121_write(REG_TX_AVIINFO_DB4,pAVIInfoFrame->pktbyte.AVI_DB[3]);
    it66121_write(REG_TX_AVIINFO_DB5,pAVIInfoFrame->pktbyte.AVI_DB[4]);
    it66121_write(REG_TX_AVIINFO_DB6,pAVIInfoFrame->pktbyte.AVI_DB[5]);
    it66121_write(REG_TX_AVIINFO_DB7,pAVIInfoFrame->pktbyte.AVI_DB[6]);
    it66121_write(REG_TX_AVIINFO_DB8,pAVIInfoFrame->pktbyte.AVI_DB[7]);
    it66121_write(REG_TX_AVIINFO_DB9,pAVIInfoFrame->pktbyte.AVI_DB[8]);
    it66121_write(REG_TX_AVIINFO_DB10,pAVIInfoFrame->pktbyte.AVI_DB[9]);
    it66121_write(REG_TX_AVIINFO_DB11,pAVIInfoFrame->pktbyte.AVI_DB[10]);
    it66121_write(REG_TX_AVIINFO_DB12,pAVIInfoFrame->pktbyte.AVI_DB[11]);
    it66121_write(REG_TX_AVIINFO_DB13,pAVIInfoFrame->pktbyte.AVI_DB[12]);
    for(i = 0,checksum = 0; i < 13 ; i++)
    {
        checksum -= pAVIInfoFrame->pktbyte.AVI_DB[i] ;
    }
    /*
    printk(("SetAVIInfo(): "));
    printk(("%02X ",(int)it66121_read(REG_TX_AVIINFO_DB1)));
    printk(("%02X ",(int)it66121_read(REG_TX_AVIINFO_DB2)));
    printk(("%02X ",(int)it66121_read(REG_TX_AVIINFO_DB3)));
    printk(("%02X ",(int)it66121_read(REG_TX_AVIINFO_DB4)));
    printk(("%02X ",(int)it66121_read(REG_TX_AVIINFO_DB5)));
    printk(("%02X ",(int)it66121_read(REG_TX_AVIINFO_DB6)));
    printk(("%02X ",(int)it66121_read(REG_TX_AVIINFO_DB7)));
    printk(("%02X ",(int)it66121_read(REG_TX_AVIINFO_DB8)));
    printk(("%02X ",(int)it66121_read(REG_TX_AVIINFO_DB9)));
    printk(("%02X ",(int)it66121_read(REG_TX_AVIINFO_DB10)));
    printk(("%02X ",(int)it66121_read(REG_TX_AVIINFO_DB11)));
    printk(("%02X ",(int)it66121_read(REG_TX_AVIINFO_DB12)));
    printk(("%02X ",(int)it66121_read(REG_TX_AVIINFO_DB13)));
    printk(("\n"));
    */
    checksum -= AVI_INFOFRAME_VER+AVI_INFOFRAME_TYPE+AVI_INFOFRAME_LEN ;
    it66121_write(REG_TX_AVIINFO_SUM,checksum);

    Switch_HDMITX_Bank(0);
    hdmitx_ENABLE_AVI_INFOFRM_PKT();
    return 0 ;
}

BOOL HDMITX_EnableAVIInfoFrame(BYTE bEnable,BYTE *pAVIInfoFrame)
{
    if(!bEnable)
    {
        hdmitx_DISABLE_AVI_INFOFRM_PKT();
        return TRUE ;
    }
    if(hdmitx_SetAVIInfoFrame((AVI_InfoFrame *)pAVIInfoFrame) == 0)
    {
        return TRUE ;
    }
    return FALSE ;
}

#undef OUTPUT_3D_MODE
#undef SUPPORT_SYNCEMBEDDED

#ifdef SUPPORT_SYNCEMBEDDED

struct CRT_TimingSetting {
	BYTE fmt;
    WORD HActive;
    WORD VActive;
    WORD HTotal;
    WORD VTotal;
    WORD H_FBH;
    WORD H_SyncW;
    WORD H_BBH;
    WORD V_FBH;
    WORD V_SyncW;
    WORD V_BBH;
    BYTE Scan:1;
    BYTE VPolarity:1;
    BYTE HPolarity:1;
};

//   VDEE_L,   VDEE_H, VRS2S_L, VRS2S_H, VRS2E_L, VRS2E_H, HalfL_L, HalfL_H, VDE2S_L, VDE2S_H, HVP&Progress
const struct CRT_TimingSetting TimingTable[] =
{
    //  VIC   H     V    HTotal VTotal  HFT   HSW     HBP VF VSW   VB
    {  1,  640,  480,    800,  525,   16,    96,    48, 10, 2,  33,      PROG, Vneg, Hneg},// 640x480@60Hz         - CEA Mode [ 1]
    {  2,  720,  480,    858,  525,   16,    62,    60,  9, 6,  30,      PROG, Vneg, Hneg},// 720x480@60Hz         - CEA Mode [ 2]
    {  3,  720,  480,    858,  525,   16,    62,    60,  9, 6,  30,      PROG, Vneg, Hneg},// 720x480@60Hz         - CEA Mode [ 3]
    {  4, 1280,  720,   1650,  750,  110,    40,   220,  5, 5,  20,      PROG, Vpos, Hpos},// 1280x720@60Hz        - CEA Mode [ 4]
    {  5, 1920,  540,   2200,  562,   88,    44,   148,  2, 5,  15, INTERLACE, Vpos, Hpos},// 1920x1080(I)@60Hz    - CEA Mode [ 5]
    {  6,  720,  240,    858,  262,   19,    62,    57,  4, 3,  15, INTERLACE, Vneg, Hneg},// 720x480(I)@60Hz      - CEA Mode [ 6]
    {  7,  720,  240,    858,  262,   19,    62,    57,  4, 3,  15, INTERLACE, Vneg, Hneg},// 720x480(I)@60Hz      - CEA Mode [ 7]
    // {  8,  720,  240,    858,  262,   19,    62,    57,  4, 3,  15,      PROG, Vneg, Hneg},// 720x480(I)@60Hz      - CEA Mode [ 8]
    // {  9,  720,  240,    858,  262,   19,    62,    57,  4, 3,  15,      PROG, Vneg, Hneg},// 720x480(I)@60Hz      - CEA Mode [ 9]
    // { 10,  720,  240,    858,  262,   19,    62,    57,  4, 3,  15, INTERLACE, Vneg, Hneg},// 720x480(I)@60Hz      - CEA Mode [10]
    // { 11,  720,  240,    858,  262,   19,    62,    57,  4, 3,  15, INTERLACE, Vneg, Hneg},// 720x480(I)@60Hz      - CEA Mode [11]
    // { 12,  720,  240,    858,  262,   19,    62,    57,  4, 3,  15,      PROG, Vneg, Hneg},// 720x480(I)@60Hz      - CEA Mode [12]
    // { 13,  720,  240,    858,  262,   19,    62,    57,  4, 3,  15,      PROG, Vneg, Hneg},// 720x480(I)@60Hz      - CEA Mode [13]
    // { 14, 1440,  480,   1716,  525,   32,   124,   120,  9, 6,  30,      PROG, Vneg, Hneg},// 1440x480@60Hz        - CEA Mode [14]
    // { 15, 1440,  480,   1716,  525,   32,   124,   120,  9, 6,  30,      PROG, Vneg, Hneg},// 1440x480@60Hz        - CEA Mode [15]
    { 16, 1920, 1080,   2200, 1125,   88,    44,   148,  4, 5,  36,      PROG, Vpos, Hpos},// 1920x1080@60Hz       - CEA Mode [16]
    { 17,  720,  576,    864,  625,   12,    64,    68,  5, 5,  39,      PROG, Vneg, Hneg},// 720x576@50Hz         - CEA Mode [17]
    { 18,  720,  576,    864,  625,   12,    64,    68,  5, 5,  39,      PROG, Vneg, Hneg},// 720x576@50Hz         - CEA Mode [18]
    { 19, 1280,  720,   1980,  750,  440,    40,   220,  5, 5,  20,      PROG, Vpos, Hpos},// 1280x720@50Hz        - CEA Mode [19]
    { 20, 1920,  540,   2640,  562,  528,    44,   148,  2, 5,  15, INTERLACE, Vpos, Hpos},// 1920x1080(I)@50Hz    - CEA Mode [20]
    { 21,  720,  288,    864,  312,   12,    63,    69,  2, 3,  19, INTERLACE, Vneg, Hneg},// 1440x576(I)@50Hz     - CEA Mode [21]
    { 22,  720,  288,    864,  312,   12,    63,    69,  2, 3,  19, INTERLACE, Vneg, Hneg},// 1440x576(I)@50Hz     - CEA Mode [22]
    // { 23,  720,  288,    864,  312,   12,    63,    69,  2, 3,  19,      PROG, Vneg, Hneg},// 1440x288@50Hz        - CEA Mode [23]
    // { 24,  720,  288,    864,  312,   12,    63,    69,  2, 3,  19,      PROG, Vneg, Hneg},// 1440x288@50Hz        - CEA Mode [24]
    // { 25,  720,  288,    864,  312,   12,    63,    69,  2, 3,  19, INTERLACE, Vneg, Hneg},// 1440x576(I)@50Hz     - CEA Mode [25]
    // { 26,  720,  288,    864,  312,   12,    63,    69,  2, 3,  19, INTERLACE, Vneg, Hneg},// 1440x576(I)@50Hz     - CEA Mode [26]
    // { 27,  720,  288,    864,  312,   12,    63,    69,  2, 3,  19,      PROG, Vneg, Hneg},// 1440x288@50Hz        - CEA Mode [27]
    // { 28,  720,  288,    864,  312,   12,    63,    69,  2, 3,  19,      PROG, Vneg, Hneg},// 1440x288@50Hz        - CEA Mode [28]
    // { 29, 1440,  576,   1728,  625,   24,   128,   136,  5, 5,  39,      PROG, Vpos, Hneg},// 1440x576@50Hz        - CEA Mode [29]
    // { 30, 1440,  576,   1728,  625,   24,   128,   136,  5, 5,  39,      PROG, Vpos, Hneg},// 1440x576@50Hz        - CEA Mode [30]
    { 31, 1920, 1080,   2640, 1125,  528,    44,   148,  4, 5,  36,      PROG, Vpos, Hpos},// 1920x1080@50Hz       - CEA Mode [31]
    { 32, 1920, 1080,   2750, 1125,  638,    44,   148,  4, 5,  36,      PROG, Vpos, Hpos},// 1920x1080@24Hz       - CEA Mode [32]
    { 33, 1920, 1080,   2640, 1125,  528,    44,   148,  4, 5,  36,      PROG, Vpos, Hpos},// 1920x1080@25Hz       - CEA Mode [33]
    { 34, 1920, 1080,   2200, 1125,   88,    44,   148,  4, 5,  36,      PROG, Vpos, Hpos},// 1920x1080@30Hz       - CEA Mode [34]
    // { 35, 2880,  480, 1716*2,  525, 32*2, 124*2, 120*2,  9, 6,  30,      PROG, Vneg, Hneg},// 2880x480@60Hz        - CEA Mode [35]
    // { 36, 2880,  480, 1716*2,  525, 32*2, 124*2, 120*2,  9, 6,  30,      PROG, Vneg, Hneg},// 2880x480@60Hz        - CEA Mode [36]
    // { 37, 2880,  576,   3456,  625, 24*2, 128*2, 136*2,  5, 5,  39,      PROG, Vneg, Hneg},// 2880x576@50Hz        - CEA Mode [37]
    // { 38, 2880,  576,   3456,  625, 24*2, 128*2, 136*2,  5, 5,  39,      PROG, Vneg, Hneg},// 2880x576@50Hz        - CEA Mode [38]
    // { 39, 1920,  540,   2304,  625,   32,   168,   184, 23, 5,  57, INTERLACE, Vneg, Hpos},// 1920x1080@50Hz       - CEA Mode [39]
    // { 40, 1920,  540,   2640,  562,  528,    44,   148,  2, 5,  15, INTERLACE, Vpos, Hpos},// 1920x1080(I)@100Hz   - CEA Mode [40]
    // { 41, 1280,  720,   1980,  750,  440,    40,   220,  5, 5,  20,      PROG, Vpos, Hpos},// 1280x720@100Hz       - CEA Mode [41]
    // { 42,  720,  576,    864,  625,   12,    64,    68,  5, 5,  39,      PROG, Vneg, Hneg},// 720x576@100Hz        - CEA Mode [42]
    // { 43,  720,  576,    864,  625,   12,    64,    68,  5, 5,  39,      PROG, Vneg, Hneg},// 720x576@100Hz        - CEA Mode [43]
    // { 44,  720,  288,    864,  312,   12,    63,    69,  2, 3,  19, INTERLACE, Vneg, Hneg},// 1440x576(I)@100Hz    - CEA Mode [44]
    // { 45,  720,  288,    864,  312,   12,    63,    69,  2, 3,  19, INTERLACE, Vneg, Hneg},// 1440x576(I)@100Hz    - CEA Mode [45]
    // { 46, 1920,  540,   2200,  562,   88,    44,   148,  2, 5,  15, INTERLACE, Vpos, Hpos},// 1920x1080(I)@120Hz   - CEA Mode [46]
    // { 47, 1280,  720,   1650,  750,  110,    40,   220,  5, 5,  20,      PROG, Vpos, Hpos},// 1280x720@120Hz       - CEA Mode [47]
    // { 48,  720,  480,    858,  525,   16,    62,    60,  9, 6,  30,      PROG, Vneg, Hneg},// 720x480@120Hz        - CEA Mode [48]
    // { 49,  720,  480,    858,  525,   16,    62,    60,  9, 6,  30,      PROG, Vneg, Hneg},// 720x480@120Hz        - CEA Mode [49]
    // { 50,  720,  240,    858,  262,   19,    62,    57,  4, 3,  15, INTERLACE, Vneg, Hneg},// 720x480(I)@120Hz     - CEA Mode [50]
    // { 51,  720,  240,    858,  262,   19,    62,    57,  4, 3,  15, INTERLACE, Vneg, Hneg},// 720x480(I)@120Hz     - CEA Mode [51]
    // { 52,  720,  576,    864,  625,   12,    64,    68,  5, 5,  39,      PROG, Vneg, Hneg},// 720x576@200Hz        - CEA Mode [52]
    // { 53,  720,  576,    864,  625,   12,    64,    68,  5, 5,  39,      PROG, Vneg, Hneg},// 720x576@200Hz        - CEA Mode [53]
    // { 54,  720,  288,    864,  312,   12,    63,    69,  2, 3,  19, INTERLACE, Vneg, Hneg},// 1440x576(I)@200Hz    - CEA Mode [54]
    // { 55,  720,  288,    864,  312,   12,    63,    69,  2, 3,  19, INTERLACE, Vneg, Hneg},// 1440x576(I)@200Hz    - CEA Mode [55]
    // { 56,  720,  480,    858,  525,   16,    62,    60,  9, 6,  30,      PROG, Vneg, Hneg},// 720x480@120Hz        - CEA Mode [56]
    // { 57,  720,  480,    858,  525,   16,    62,    60,  9, 6,  30,      PROG, Vneg, Hneg},// 720x480@120Hz        - CEA Mode [57]
    // { 58,  720,  240,    858,  262,   19,    62,    57,  4, 3,  15, INTERLACE, Vneg, Hneg},// 720x480(I)@120Hz     - CEA Mode [58]
    // { 59,  720,  240,    858,  262,   19,    62,    57,  4, 3,  15, INTERLACE, Vneg, Hneg},// 720x480(I)@120Hz     - CEA Mode [59]
    { 60, 1280,  720,   3300,  750, 1760,    40,   220,  5, 5,  20,      PROG, Vpos, Hpos},// 1280x720@24Hz        - CEA Mode [60]
    { 61, 1280,  720,   3960,  750, 2420,    40,   220,  5, 5,  20,      PROG, Vpos, Hpos},// 1280x720@25Hz        - CEA Mode [61]
    { 62, 1280,  720,   3300,  750, 1760,    40,   220,  5, 5,  20,      PROG, Vpos, Hpos},// 1280x720@30Hz        - CEA Mode [62]
    // { 63, 1920, 1080,   2200, 1125,   88,    44,   148,  4, 5,  36,      PROG, Vpos, Hpos},// 1920x1080@120Hz      - CEA Mode [63]
    // { 64, 1920, 1080,   2640, 1125,  528,    44,   148,  4, 5,  36,      PROG, Vpos, Hpos},// 1920x1080@100Hz      - CEA Mode [64]
};

#define MaxIndex (sizeof(TimingTable)/sizeof(struct CRT_TimingSetting))
BOOL setHDMITX_SyncEmbeddedByVIC(BYTE VIC,BYTE bInputType)
{
    int i ;
    BYTE fmt_index=0;

    // if Embedded Video,need to generate timing with pattern register
    Switch_HDMITX_Bank(0);

    printk("setHDMITX_SyncEmbeddedByVIC(%d,%x)\n",(int)VIC,(int)bInputType);
    if( VIC > 0 )
    {
        for(i=0;i< MaxIndex;i ++)
        {
            if(TimingTable[i].fmt==VIC)
            {
                fmt_index=i;
                printk("fmt_index=%02x)\n",(int)fmt_index);
                printk("***Fine Match Table ***\n");
                break;
            }
        }
    }
    else
    {
        printk("***No Match VIC == 0 ***\n");
        return FALSE ;
    }

    if(i>=MaxIndex)
    {
        //return FALSE;
        printk("***No Match VIC ***\n");
        return FALSE ;
    }
    //if( bInputSignalType & T_MODE_SYNCEMB )
    {
        int HTotal, HDES, VTotal, VDES;
        int HDEW, VDEW, HFP, HSW, VFP, VSW;
        int HRS, HRE;
        int VRS, VRE;
        int H2ndVRRise;
        int VRS2nd, VRE2nd;
        BYTE Pol;

        HTotal  =TimingTable[fmt_index].HTotal;
        HDEW    =TimingTable[fmt_index].HActive;
        HFP     =TimingTable[fmt_index].H_FBH;
        HSW     =TimingTable[fmt_index].H_SyncW;
        HDES    =HSW+TimingTable[fmt_index].H_BBH;
        VTotal  =TimingTable[fmt_index].VTotal;
        VDEW    =TimingTable[fmt_index].VActive;
        VFP     =TimingTable[fmt_index].V_FBH;
        VSW     =TimingTable[fmt_index].V_SyncW;
        VDES    =VSW+TimingTable[fmt_index].V_BBH;

        Pol = (TimingTable[fmt_index].HPolarity==Hpos)?(1<<1):0 ;
        Pol |= (TimingTable[fmt_index].VPolarity==Vpos)?(1<<2):0 ;

        // SyncEmb case=====
        if( bInputType & T_MODE_CCIR656)
        {
            HRS = HFP - 1;
        }
        else
        {
            HRS = HFP - 2;
            /*
            if(VIC==HDMI_1080p60 ||
               VIC==HDMI_1080p50 )
            {
                HDMITX_OrReg_Byte(0x59, (1<<3));
            }
            else
            {
                HDMITX_AndReg_Byte(0x59, ~(1<<3));
            }
            */
        }
        HRE = HRS + HSW;
        H2ndVRRise = HRS+ HTotal/2;

        VRS = VFP;
        VRE = VRS + VSW;

        // VTotal>>=1;

        if(PROG == TimingTable[fmt_index].Scan)
        { // progressive mode
            VRS2nd = 0xFFF;
            VRE2nd = 0x3F;
        }
        else
        { // interlaced mode
            if(39 == TimingTable[fmt_index].fmt)
            {
                VRS2nd = VRS + VTotal - 1;
                VRE2nd = VRS2nd + VSW;
            }
            else
            {
                VRS2nd = VRS + VTotal;
                VRE2nd = VRS2nd + VSW;
            }
        }
        #ifdef DETECT_VSYNC_CHG_IN_SAV
        if( EnSavVSync )
        {
            VRS -= 1;
            VRE -= 1;
            if( !pSetVTiming->ScanMode ) // interlaced mode
            {
                VRS2nd -= 1;
                VRE2nd -= 1;
            }
        }
        #endif // DETECT_VSYNC_CHG_IN_SAV
        HDMITX_SetI2C_Byte(0x90, 0x06, Pol);
        // write H2ndVRRise
        HDMITX_SetI2C_Byte(0x90, 0xF0, (H2ndVRRise&0x0F)<<4);
        it66121_write(0x91, (H2ndVRRise&0x0FF0)>>4);
        // write HRS/HRE
        it66121_write(0x95, HRS&0xFF);
        it66121_write(0x96, HRE&0xFF);
        it66121_write(0x97, ((HRE&0x0F00)>>4)+((HRS&0x0F00)>>8));
        // write VRS/VRE
        it66121_write(0xa0, VRS&0xFF);
        it66121_write(0xa1, ((VRE&0x0F)<<4)+((VRS&0x0F00)>>8));
        it66121_write(0xa2, VRS2nd&0xFF);
        it66121_write(0xa6, (VRE2nd&0xF0)+((VRE&0xF0)>>4));
        it66121_write(0xa3, ((VRE2nd&0x0F)<<4)+((VRS2nd&0xF00)>>8));
        it66121_write(0xa4, H2ndVRRise&0xFF);
        it66121_write(0xa5, (/*EnDEOnly*/0<<5)+((TimingTable[fmt_index].Scan==INTERLACE)?(1<<4):0)+((H2ndVRRise&0xF00)>>8));
        HDMITX_SetI2C_Byte(0xb1, 0x51, ((HRE&0x1000)>>6)+((HRS&0x1000)>>8)+((HDES&0x1000)>>12));
        HDMITX_SetI2C_Byte(0xb2, 0x05, ((H2ndVRRise&0x1000)>>10)+((H2ndVRRise&0x1000)>>12));
    }
    return TRUE ;
}

#endif // SUPPORT_SYNCEMBEDDED

///*****************************************
//   @file   <hdmitx_aud.c>
//******************************************/

BYTE AudioDelayCnt=0;
BYTE LastRefaudfreqnum=0;

void HDMITX_DisableAudioOutput(void)
{
    //BYTE uc = (it66121_read(REG_TX_SW_RST) | (B_HDMITX_AUD_RST | B_TX_AREF_RST));
    //it66121_write(REG_TX_SW_RST,uc);
    AudioDelayCnt=250;
    LastRefaudfreqnum=0;
    HDMITX_SetI2C_Byte(REG_TX_SW_RST, (B_HDMITX_AUD_RST | B_TX_AREF_RST), (B_HDMITX_AUD_RST | B_TX_AREF_RST) );
    HDMITX_SetI2C_Byte(0x0F, 0x10, 0x10 );
}

int HDMITX_ReadI2C_ByteN(BYTE RegAddr,BYTE *pData,int N)
{
    int flag = 0;
    unsigned char i;

    //flag =i2c_read_byte(TX0ADR,RegAddr,N,pData,TX0DEV);

	for(i=0;i<N;i++)
    {
        flag = it66121_read(RegAddr+i);
		if (flag >= 0) *pData = flag;
		else break;
    }

    return flag;
}


#ifdef SUPPORT_HDCP

#define REG_TX_HDCP_DESIRE 0x20
    #define B_TX_ENABLE_HDPC11 (1<<1)
    #define B_TX_CPDESIRE  (1<<0)

#define REG_TX_AUTHFIRE    0x21
#define REG_TX_LISTCTRL    0x22
    #define B_TX_LISTFAIL  (1<<1)
    #define B_TX_LISTDONE  (1<<0)

#define REG_TX_AKSV    0x23
#define REG_TX_AKSV0   0x23
#define REG_TX_AKSV1   0x24
#define REG_TX_AKSV2   0x25
#define REG_TX_AKSV3   0x26
#define REG_TX_AKSV4   0x27

#define REG_TX_AN  0x28
#define REG_TX_AN_GEN  0x30
#define REG_TX_ARI     0x38
#define REG_TX_ARI0    0x38
#define REG_TX_ARI1    0x39
#define REG_TX_APJ     0x3A

#define REG_TX_BKSV    0x3B
#define REG_TX_BRI     0x40
#define REG_TX_BRI0    0x40
#define REG_TX_BRI1    0x41
#define REG_TX_BPJ     0x42
#define REG_TX_BCAP    0x43
    #define B_TX_CAP_HDMI_REPEATER (1<<6)
    #define B_TX_CAP_KSV_FIFO_RDY  (1<<5)
    #define B_TX_CAP_HDMI_FAST_MODE    (1<<4)
    #define B_CAP_HDCP_1p1  (1<<1)
    #define B_TX_CAP_FAST_REAUTH   (1<<0)
#define REG_TX_BSTAT   0x44
#define REG_TX_BSTAT0   0x44
#define REG_TX_BSTAT1   0x45
    #define B_TX_CAP_HDMI_MODE (1<<12)
    #define B_TX_CAP_DVI_MODE (0<<12)
    #define B_TX_MAX_CASCADE_EXCEEDED  (1<<11)
    #define M_TX_REPEATER_DEPTH    (0x7<<8)
    #define O_TX_REPEATER_DEPTH    8
    #define B_TX_DOWNSTREAM_OVER   (1<<7)
    #define M_TX_DOWNSTREAM_COUNT  0x7F

#define REG_TX_AUTH_STAT 0x46
#define B_TX_AUTH_DONE (1<<7)

#define SUPPORT_SHA

#define HDMITX_OrReg_Byte(reg,ormask) HDMITX_SetI2C_Byte(reg,(ormask),(ormask))
#define HDMITX_AndReg_Byte(reg,andmask) it66121_write(reg,(it66121_read(reg) & (andmask)))


#ifdef SUPPORT_SHA
BYTE SHABuff[64] ;
BYTE V[20] ;
BYTE KSVList[32] ;
BYTE Vr[20] ;
BYTE M0[8] ;
#endif

void hdmitx_AbortDDC(void);
void hdmitx_ClearDDCFIFO(void);
SYS_STATUS hdmitx_hdcp_Authenticate_Repeater(void);


BOOL getHDMITX_AuthenticationDone(void)
{
    //HDCP_DEBUG_PRINTF((" getHDMITX_AuthenticationDone() = %s\n",hdmiTxDev[0].bAuthenticated?"TRUE":"FALSE" ));
    return hdmiTxDev[0].bAuthenticated;
}

//////////////////////////////////////////////////////////////////////
// Authentication
//////////////////////////////////////////////////////////////////////
void hdmitx_hdcp_ClearAuthInterrupt(void)
{
    // BYTE uc ;
    // uc = it66121_read(REG_TX_INT_MASK2) & (~(B_TX_KSVLISTCHK_MASK|B_TX_AUTH_DONE_MASK|B_TX_AUTH_FAIL_MASK));
    HDMITX_SetI2C_Byte(REG_TX_INT_MASK2, B_TX_KSVLISTCHK_MASK|B_TX_AUTH_DONE_MASK|B_TX_AUTH_FAIL_MASK, 0);
    it66121_write(REG_TX_INT_CLR0,B_TX_CLR_AUTH_FAIL|B_TX_CLR_AUTH_DONE|B_TX_CLR_KSVLISTCHK);
    it66121_write(REG_TX_INT_CLR1,0);
    it66121_write(REG_TX_SYS_STATUS,B_TX_INTACTDONE);
}

void hdmitx_hdcp_ResetAuth(void)
{
    it66121_write(REG_TX_LISTCTRL,0);
    it66121_write(REG_TX_HDCP_DESIRE,0);
    HDMITX_OrReg_Byte(REG_TX_SW_RST,B_TX_HDCP_RST_HDMITX);
    it66121_write(REG_TX_DDC_MASTER_CTRL,B_TX_MASTERDDC|B_TX_MASTERHOST);
    hdmitx_hdcp_ClearAuthInterrupt();
    hdmitx_AbortDDC();
}

//////////////////////////////////////////////////////////////////////
// Function: hdmitx_hdcp_Auth_Fire()
// Parameter: N/A
// Return: N/A
// Remark: write anything to reg21 to enable HDCP authentication by HW
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

void hdmitx_hdcp_Auth_Fire(void)
{
    // HDCP_DEBUG_PRINTF(("hdmitx_hdcp_Auth_Fire():\n"));
    it66121_write(REG_TX_DDC_MASTER_CTRL,B_TX_MASTERDDC|B_TX_MASTERHDCP); // MASTERHDCP,no need command but fire.
    it66121_write(REG_TX_AUTHFIRE,1);
}

//////////////////////////////////////////////////////////////////////
// Function: hdmitx_hdcp_StartAnCipher
// Parameter: N/A
// Return: N/A
// Remark: Start the Cipher to free run for random number. When stop,An is
//         ready in Reg30.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

void hdmitx_hdcp_StartAnCipher(void)
{
    it66121_write(REG_TX_AN_GENERATE,B_TX_START_CIPHER_GEN);
    mdelay(1); // delay 1 ms
}
//////////////////////////////////////////////////////////////////////
// Function: hdmitx_hdcp_StopAnCipher
// Parameter: N/A
// Return: N/A
// Remark: Stop the Cipher,and An is ready in Reg30.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

void hdmitx_hdcp_StopAnCipher(void)
{
    it66121_write(REG_TX_AN_GENERATE,B_TX_STOP_CIPHER_GEN);
}

//////////////////////////////////////////////////////////////////////
// Function: hdmitx_hdcp_GenerateAn
// Parameter: N/A
// Return: N/A
// Remark: start An ciper random run at first,then stop it. Software can get
//         an in reg30~reg38,the write to reg28~2F
// Side-Effect:
//////////////////////////////////////////////////////////////////////

void hdmitx_hdcp_GenerateAn(void)
{
    BYTE Data[8];
    BYTE i=0;
#if 1
    hdmitx_hdcp_StartAnCipher();
    // it66121_write(REG_TX_AN_GENERATE,B_TX_START_CIPHER_GEN);
    // mdelay(1); // delay 1 ms
    // it66121_write(REG_TX_AN_GENERATE,B_TX_STOP_CIPHER_GEN);

    hdmitx_hdcp_StopAnCipher();

    Switch_HDMITX_Bank(0);
    // new An is ready in reg30
    HDMITX_ReadI2C_ByteN(REG_TX_AN_GEN,Data,8);
#else
    Data[0] = 0 ;Data[1] = 0 ;Data[2] = 0 ;Data[3] = 0 ;
    Data[4] = 0 ;Data[5] = 0 ;Data[6] = 0 ;Data[7] = 0 ;
#endif
    for(i=0;i<8;i++)
    {
        it66121_write(REG_TX_AN+i,Data[i]);
    }
    //it66121_writeN(REG_TX_AN,Data,8);
}

//////////////////////////////////////////////////////////////////////
// Function: hdmitx_hdcp_GetBCaps
// Parameter: pBCaps - pointer of byte to get BCaps.
//            pBStatus - pointer of two bytes to get BStatus
// Return: ER_SUCCESS if successfully got BCaps and BStatus.
// Remark: get B status and capability from HDCP reciever via DDC bus.
// Side-Effect:
//////////////////////////////////////////////////////////////////////

SYS_STATUS hdmitx_hdcp_GetBCaps(unsigned char *pBCaps ,unsigned short *pBStatus)
{
    BYTE ucdata ;
    BYTE TimeOut ;

    Switch_HDMITX_Bank(0);
    it66121_write(REG_TX_DDC_MASTER_CTRL,B_TX_MASTERDDC|B_TX_MASTERHOST);
    it66121_write(REG_TX_DDC_HEADER,DDC_HDCP_ADDRESS);
    it66121_write(REG_TX_DDC_REQOFF,0x40); // BCaps offset
    it66121_write(REG_TX_DDC_REQCOUNT,3);
    it66121_write(REG_TX_DDC_CMD,CMD_DDC_SEQ_BURSTREAD);

    for(TimeOut = 200 ; TimeOut > 0 ; TimeOut --)
    {
        mdelay(1);

        ucdata = it66121_read(REG_TX_DDC_STATUS);

        if(ucdata & B_TX_DDC_DONE)
        {
            //HDCP_DEBUG_PRINTF(("hdmitx_hdcp_GetBCaps(): DDC Done.\n"));
            break ;
        }
        if(ucdata & B_TX_DDC_ERROR)
        {
//            HDCP_DEBUG_PRINTF(("hdmitx_hdcp_GetBCaps(): DDC fail by reg16=%02X.\n",ucdata));
            return ER_FAIL ;
        }
    }
    if(TimeOut == 0)
    {
        return ER_FAIL ;
    }
#if 1
    ucdata = it66121_read(REG_TX_BSTAT+1);

    *pBStatus = (unsigned short)ucdata ;
    *pBStatus <<= 8 ;
    ucdata = it66121_read(REG_TX_BSTAT);
    *pBStatus |= ((unsigned short)ucdata&0xFF);
    *pBCaps = it66121_read(REG_TX_BCAP);
#else
    *pBCaps = it66121_read(0x17);
    *pBStatus = it66121_read(0x17) & 0xFF ;
    *pBStatus |= (int)(it66121_read(0x17)&0xFF)<<8;
    HDCP_DEBUG_PRINTF(("hdmitx_hdcp_GetBCaps(): ucdata = %02X\n",(int)it66121_read(0x16)));
#endif
    return ER_SUCCESS ;
}

//////////////////////////////////////////////////////////////////////
// Function: hdmitx_hdcp_GetBKSV
// Parameter: pBKSV - pointer of 5 bytes buffer for getting BKSV
// Return: ER_SUCCESS if successfuly got BKSV from Rx.
// Remark: Get BKSV from HDCP reciever.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

SYS_STATUS hdmitx_hdcp_GetBKSV(BYTE *pBKSV)
{
    BYTE ucdata ;
    BYTE TimeOut ;

    Switch_HDMITX_Bank(0);
    it66121_write(REG_TX_DDC_MASTER_CTRL,B_TX_MASTERDDC|B_TX_MASTERHOST);
    it66121_write(REG_TX_DDC_HEADER,DDC_HDCP_ADDRESS);
    it66121_write(REG_TX_DDC_REQOFF,0x00); // BKSV offset
    it66121_write(REG_TX_DDC_REQCOUNT,5);
    it66121_write(REG_TX_DDC_CMD,CMD_DDC_SEQ_BURSTREAD);

    for(TimeOut = 200 ; TimeOut > 0 ; TimeOut --)
    {
        mdelay(1);

        ucdata = it66121_read(REG_TX_DDC_STATUS);
        if(ucdata & B_TX_DDC_DONE)
        {
            printk("hdmitx_hdcp_GetBCaps(): DDC Done.\n");
            break ;
        }
        if(ucdata & B_TX_DDC_ERROR)
        {
            printk("hdmitx_hdcp_GetBCaps(): DDC No ack or arbilose,%x,maybe cable did not connected. Fail.\n",ucdata);
            return ER_FAIL ;
        }
    }
    if(TimeOut == 0)
    {
        return ER_FAIL ;
    }
    HDMITX_ReadI2C_ByteN(REG_TX_BKSV,pBKSV,5);

    return ER_SUCCESS ;
}

//////////////////////////////////////////////////////////////////////
// Function:hdmitx_hdcp_Authenticate
// Parameter: N/A
// Return: ER_SUCCESS if Authenticated without error.
// Remark: do Authentication with Rx
// Side-Effect:
//  1. hdmiTxDev[0].bAuthenticated global variable will be TRUE when authenticated.
//  2. Auth_done interrupt and AUTH_FAIL interrupt will be enabled.
//////////////////////////////////////////////////////////////////////
static BYTE countbit(BYTE b)
{
    BYTE i,count ;
    for( i = 0, count = 0 ; i < 8 ; i++ )
    {
        if( b & (1<<i) )
        {
            count++ ;
        }
    }
    return count ;
}

void hdmitx_hdcp_Reset(void)
{
    BYTE uc ;
    uc = it66121_read(REG_TX_SW_RST) | B_TX_HDCP_RST_HDMITX ;
    it66121_write(REG_TX_SW_RST,uc);
    it66121_write(REG_TX_HDCP_DESIRE,0);
    it66121_write(REG_TX_LISTCTRL,0);
    it66121_write(REG_TX_DDC_MASTER_CTRL,B_TX_MASTERHOST);
    hdmitx_ClearDDCFIFO();
    hdmitx_AbortDDC();
}


SYS_STATUS hdmitx_hdcp_Authenticate(void)
{
    BYTE ucdata ;
    BYTE BCaps ;
    unsigned short BStatus ;
    unsigned short TimeOut ;

    BYTE BKSV[5] ;

    hdmiTxDev[0].bAuthenticated = FALSE ;
    if(0==(B_TXVIDSTABLE&it66121_read(REG_TX_SYS_STATUS)))
    {
        return ER_FAIL;
    }
    // Authenticate should be called after AFE setup up.

    printk("hdmitx_hdcp_Authenticate():\n");
    hdmitx_hdcp_Reset();

    Switch_HDMITX_Bank(0);

    for( TimeOut = 0 ; TimeOut < 80 ; TimeOut++ )
    {
        mdelay(15);

        if(hdmitx_hdcp_GetBCaps(&BCaps,&BStatus) != ER_SUCCESS)
        {
            printk("hdmitx_hdcp_GetBCaps fail.\n");
            return ER_FAIL ;
        }
        // HDCP_DEBUG_PRINTF(("(%d)Reg16 = %02X\n",idx++,(int)it66121_read(0x16)));

        if(B_TX_HDMI_MODE == (it66121_read(REG_TX_HDMI_MODE) & B_TX_HDMI_MODE ))
        {
            if((BStatus & B_TX_CAP_HDMI_MODE)==B_TX_CAP_HDMI_MODE)
            {
                break;
            }
        }
        else
        {
            if((BStatus & B_TX_CAP_HDMI_MODE)!=B_TX_CAP_HDMI_MODE)
            {
                break;
            }
        }
    }
    /*
    if((BStatus & M_TX_DOWNSTREAM_COUNT)> 6)
    {
        HDCP_DEBUG_PRINTF(("Down Stream Count %d is over maximum supported number 6,fail.\n",(int)(BStatus & M_TX_DOWNSTREAM_COUNT)));
        return ER_FAIL ;
    }
    */
	printk("BCAPS = %02X BSTATUS = %04X\n", (int)BCaps, BStatus);
    hdmitx_hdcp_GetBKSV(BKSV);
    printk("BKSV %02X %02X %02X %02X %02X\n",(int)BKSV[0],(int)BKSV[1],(int)BKSV[2],(int)BKSV[3],(int)BKSV[4]);

    for(TimeOut = 0, ucdata = 0 ; TimeOut < 5 ; TimeOut ++)
    {
        ucdata += countbit(BKSV[TimeOut]);
    }
    if( ucdata != 20 )
    {
        printk("countbit error\n");
        return ER_FAIL ;

    }
    Switch_HDMITX_Bank(0); // switch bank action should start on direct register writting of each function.

    HDMITX_AndReg_Byte(REG_TX_SW_RST,~(B_TX_HDCP_RST_HDMITX));

    it66121_write(REG_TX_HDCP_DESIRE,B_TX_CPDESIRE);
    hdmitx_hdcp_ClearAuthInterrupt();

    hdmitx_hdcp_GenerateAn();
    it66121_write(REG_TX_LISTCTRL,0);
    hdmiTxDev[0].bAuthenticated = FALSE ;

    hdmitx_ClearDDCFIFO();

    if((BCaps & B_TX_CAP_HDMI_REPEATER) == 0)
    {
        hdmitx_hdcp_Auth_Fire();
        // wait for status ;

        for(TimeOut = 250 ; TimeOut > 0 ; TimeOut --)
        {
            mdelay(5); // delay 1ms
            ucdata = it66121_read(REG_TX_AUTH_STAT);
            // HDCP_DEBUG_PRINTF(("reg46 = %02x reg16 = %02x\n",(int)ucdata,(int)it66121_read(0x16)));

            if(ucdata & B_TX_AUTH_DONE)
            {
                hdmiTxDev[0].bAuthenticated = TRUE ;
                break ;
            }
            ucdata = it66121_read(REG_TX_INT_STAT2);
            if(ucdata & B_TX_INT_AUTH_FAIL)
            {

                it66121_write(REG_TX_INT_CLR0,B_TX_CLR_AUTH_FAIL);
                it66121_write(REG_TX_INT_CLR1,0);
                it66121_write(REG_TX_SYS_STATUS,B_TX_INTACTDONE);
                it66121_write(REG_TX_SYS_STATUS,0);

                printk("hdmitx_hdcp_Authenticate()-receiver: Authenticate fail\n");
                hdmiTxDev[0].bAuthenticated = FALSE ;
                return ER_FAIL ;
            }
        }
        if(TimeOut == 0)
        {
             printk("hdmitx_hdcp_Authenticate()-receiver: Time out. return fail\n");
             hdmiTxDev[0].bAuthenticated = FALSE ;
             return ER_FAIL ;
        }
        return ER_SUCCESS ;
    }
    return hdmitx_hdcp_Authenticate_Repeater();
}

//////////////////////////////////////////////////////////////////////
// Function: hdmitx_hdcp_VerifyIntegration
// Parameter: N/A
// Return: ER_SUCCESS if success,if AUTH_FAIL interrupt status,return fail.
// Remark: no used now.
// Side-Effect:
//////////////////////////////////////////////////////////////////////

SYS_STATUS hdmitx_hdcp_VerifyIntegration(void)
{
    // if any interrupt issued a Auth fail,returned the Verify Integration fail.

    if(it66121_read(REG_TX_INT_STAT1) & B_TX_INT_AUTH_FAIL)
    {
        hdmitx_hdcp_ClearAuthInterrupt();
        hdmiTxDev[0].bAuthenticated = FALSE ;
        return ER_FAIL ;
    }
    if(hdmiTxDev[0].bAuthenticated == TRUE)
    {
        return ER_SUCCESS ;
    }
    return ER_FAIL ;
}

//////////////////////////////////////////////////////////////////////
// Function: hdmitx_hdcp_Authenticate_Repeater
// Parameter: BCaps and BStatus
// Return: ER_SUCCESS if success,if AUTH_FAIL interrupt status,return fail.
// Remark:
// Side-Effect: as Authentication
//////////////////////////////////////////////////////////////////////

void hdmitx_hdcp_CancelRepeaterAuthenticate(void)
{
    printk("hdmitx_hdcp_CancelRepeaterAuthenticate");
    it66121_write(REG_TX_DDC_MASTER_CTRL,B_TX_MASTERDDC|B_TX_MASTERHOST);
    hdmitx_AbortDDC();
    it66121_write(REG_TX_LISTCTRL,B_TX_LISTFAIL|B_TX_LISTDONE);
    hdmitx_hdcp_ClearAuthInterrupt();
}

void hdmitx_hdcp_ResumeRepeaterAuthenticate(void)
{
    it66121_write(REG_TX_LISTCTRL,B_TX_LISTDONE);
    it66121_write(REG_TX_DDC_MASTER_CTRL,B_TX_MASTERHDCP);
}

#ifdef SUPPORT_SHA

SYS_STATUS hdmitx_hdcp_CheckSHA(BYTE pM0[],unsigned short BStatus,BYTE pKSVList[],int cDownStream,BYTE Vr[])
{
    int i,n ;

    for(i = 0 ; i < cDownStream*5 ; i++)
    {
        SHABuff[i] = pKSVList[i] ;
    }
    SHABuff[i++] = BStatus & 0xFF ;
    SHABuff[i++] = (BStatus>>8) & 0xFF ;
    for(n = 0 ; n < 8 ; n++,i++)
    {
        SHABuff[i] = pM0[n] ;
    }
    n = i ;
    // SHABuff[i++] = 0x80 ; // end mask
    for(; i < 64 ; i++)
    {
        SHABuff[i] = 0 ;
    }
  
    for(i = 0 ; i < 20 ; i++)
    {
        if(V[i] != Vr[i])
        {
            printk("V[] =");
            for(i = 0 ; i < 20 ; i++)
            {
                printk(" %02X",(int)V[i]);
            }
            printk(("\nVr[] ="));
            for(i = 0 ; i < 20 ; i++)
            {
                printk(" %02X",(int)Vr[i]);
            }
            return ER_FAIL ;
        }
    }
    return ER_SUCCESS ;
}

#endif // SUPPORT_SHA

SYS_STATUS hdmitx_hdcp_GetKSVList(BYTE *pKSVList,BYTE cDownStream)
{
    BYTE TimeOut = 100 ;
    BYTE ucdata ;

    if( cDownStream == 0 )
    {
        return ER_SUCCESS ;
    }
    if( /* cDownStream == 0 || */ pKSVList == NULL)
    {
        return ER_FAIL ;
    }
    it66121_write(REG_TX_DDC_MASTER_CTRL,B_TX_MASTERHOST);
    it66121_write(REG_TX_DDC_HEADER,0x74);
    it66121_write(REG_TX_DDC_REQOFF,0x43);
    it66121_write(REG_TX_DDC_REQCOUNT,cDownStream * 5);
    it66121_write(REG_TX_DDC_CMD,CMD_DDC_SEQ_BURSTREAD);

    for(TimeOut = 200 ; TimeOut > 0 ; TimeOut --)
    {

        ucdata = it66121_read(REG_TX_DDC_STATUS);
        if(ucdata & B_TX_DDC_DONE)
        {
            printk("hdmitx_hdcp_GetKSVList(): DDC Done.\n");
            break ;
        }
        if(ucdata & B_TX_DDC_ERROR)
        {
            printk("hdmitx_hdcp_GetKSVList(): DDC Fail by REG_TX_DDC_STATUS = %x.\n",ucdata);
            return ER_FAIL ;
        }
        mdelay(5);
    }
    if(TimeOut == 0)
    {
        return ER_FAIL ;
    }
    printk("hdmitx_hdcp_GetKSVList(): KSV");
    for(TimeOut = 0 ; TimeOut < cDownStream * 5 ; TimeOut++)
    {
        pKSVList[TimeOut] = it66121_read(REG_TX_DDC_READFIFO);
        printk(" %02X",(int)pKSVList[TimeOut]);
    }
    printk(("\n"));
    return ER_SUCCESS ;
}

SYS_STATUS hdmitx_hdcp_GetVr(BYTE *pVr)
{
    BYTE TimeOut  ;
    BYTE ucdata ;

    if(pVr == NULL)
    {
        return ER_FAIL ;
    }
    it66121_write(REG_TX_DDC_MASTER_CTRL,B_TX_MASTERHOST);
    it66121_write(REG_TX_DDC_HEADER,0x74);
    it66121_write(REG_TX_DDC_REQOFF,0x20);
    it66121_write(REG_TX_DDC_REQCOUNT,20);
    it66121_write(REG_TX_DDC_CMD,CMD_DDC_SEQ_BURSTREAD);

    for(TimeOut = 200 ; TimeOut > 0 ; TimeOut --)
    {
        ucdata = it66121_read(REG_TX_DDC_STATUS);
        if(ucdata & B_TX_DDC_DONE)
        {
            printk("hdmitx_hdcp_GetVr(): DDC Done.\n");
            break ;
        }
        if(ucdata & B_TX_DDC_ERROR)
        {
            printk("hdmitx_hdcp_GetVr(): DDC fail by REG_TX_DDC_STATUS = %x.\n",(int)ucdata);
            return ER_FAIL ;
        }
        mdelay(5);
    }
    if(TimeOut == 0)
    {
        printk("hdmitx_hdcp_GetVr(): DDC fail by timeout.\n");
        return ER_FAIL ;
    }
    Switch_HDMITX_Bank(0);

    for(TimeOut = 0 ; TimeOut < 5 ; TimeOut++)
    {
        it66121_write(REG_TX_SHA_SEL ,TimeOut);
        pVr[TimeOut*4]  = (ULONG)it66121_read(REG_TX_SHA_RD_BYTE1);
        pVr[TimeOut*4+1] = (ULONG)it66121_read(REG_TX_SHA_RD_BYTE2);
        pVr[TimeOut*4+2] = (ULONG)it66121_read(REG_TX_SHA_RD_BYTE3);
        pVr[TimeOut*4+3] = (ULONG)it66121_read(REG_TX_SHA_RD_BYTE4);
//        HDCP_DEBUG_PRINTF(("V' = %02X %02X %02X %02X\n",(int)pVr[TimeOut*4],(int)pVr[TimeOut*4+1],(int)pVr[TimeOut*4+2],(int)pVr[TimeOut*4+3]));
    }
    return ER_SUCCESS ;
}

SYS_STATUS hdmitx_hdcp_GetM0(BYTE *pM0)
{
    int i ;

    if(!pM0)
    {
        return ER_FAIL ;
    }
    it66121_write(REG_TX_SHA_SEL,5); // read m0[31:0] from reg51~reg54
    pM0[0] = it66121_read(REG_TX_SHA_RD_BYTE1);
    pM0[1] = it66121_read(REG_TX_SHA_RD_BYTE2);
    pM0[2] = it66121_read(REG_TX_SHA_RD_BYTE3);
    pM0[3] = it66121_read(REG_TX_SHA_RD_BYTE4);
    it66121_write(REG_TX_SHA_SEL,0); // read m0[39:32] from reg55
    pM0[4] = it66121_read(REG_TX_AKSV_RD_BYTE5);
    it66121_write(REG_TX_SHA_SEL,1); // read m0[47:40] from reg55
    pM0[5] = it66121_read(REG_TX_AKSV_RD_BYTE5);
    it66121_write(REG_TX_SHA_SEL,2); // read m0[55:48] from reg55
    pM0[6] = it66121_read(REG_TX_AKSV_RD_BYTE5);
    it66121_write(REG_TX_SHA_SEL,3); // read m0[63:56] from reg55
    pM0[7] = it66121_read(REG_TX_AKSV_RD_BYTE5);

    printk("M[] =");
    for(i = 0 ; i < 8 ; i++)
    {
        printk("0x%02x,",(int)pM0[i]);
    }
    printk("\n");
    return ER_SUCCESS ;
}

void hdmitx_GenerateDDCSCLK(void);

SYS_STATUS hdmitx_hdcp_Authenticate_Repeater(void)
{
    BYTE uc ,ii;
    // BYTE revoked ;
    // int i ;
    BYTE cDownStream ;

    BYTE BCaps;
    unsigned short BStatus ;
    unsigned short TimeOut ;

    printk("Authentication for repeater\n");
 

    hdmitx_hdcp_GetBCaps(&BCaps,&BStatus);
    mdelay(2);
    if((B_TX_INT_HPD_PLUG|B_TX_INT_RX_SENSE)&it66121_read(REG_TX_INT_STAT1))
    {
        printk("HPD Before Fire Auth\n");
        goto hdmitx_hdcp_Repeater_Fail ;
    }
    hdmitx_hdcp_Auth_Fire();
    //mdelay(550); // emily add for test
    for(ii=0;ii<55;ii++)    //mdelay(550); // emily add for test
    {
        if((B_TX_INT_HPD_PLUG|B_TX_INT_RX_SENSE)&it66121_read(REG_TX_INT_STAT1))
        {
            goto hdmitx_hdcp_Repeater_Fail ;
        }
        mdelay(10);
    }
    for(TimeOut = /*250*6*/10 ; TimeOut > 0 ; TimeOut --)
    {
        printk("TimeOut = %d wait part 1\n",TimeOut);
        if((B_TX_INT_HPD_PLUG|B_TX_INT_RX_SENSE)&it66121_read(REG_TX_INT_STAT1))
        {
            printk("HPD at wait part 1\n");
            goto hdmitx_hdcp_Repeater_Fail ;
        }
        uc = it66121_read(REG_TX_INT_STAT1);
        if(uc & B_TX_INT_DDC_BUS_HANG)
        {
            printk("DDC Bus hang\n");
            goto hdmitx_hdcp_Repeater_Fail ;
        }
        uc = it66121_read(REG_TX_INT_STAT2);

        if(uc & B_TX_INT_AUTH_FAIL)
        {
            /*
            it66121_write(REG_TX_INT_CLR0,B_TX_CLR_AUTH_FAIL);
            it66121_write(REG_TX_INT_CLR1,0);
            it66121_write(REG_TX_SYS_STATUS,B_TX_INTACTDONE);
            it66121_write(REG_TX_SYS_STATUS,0);
            */
            printk("hdmitx_hdcp_Authenticate_Repeater(): B_TX_INT_AUTH_FAIL.\n");
            goto hdmitx_hdcp_Repeater_Fail ;
        }
        // emily add for test
        // test =(it66121_read(0x7)&0x4)>>2 ;
        if(uc & B_TX_INT_KSVLIST_CHK)
        {
            it66121_write(REG_TX_INT_CLR0,B_TX_CLR_KSVLISTCHK);
            it66121_write(REG_TX_INT_CLR1,0);
            it66121_write(REG_TX_SYS_STATUS,B_TX_INTACTDONE);
            it66121_write(REG_TX_SYS_STATUS,0);
            printk("B_TX_INT_KSVLIST_CHK\n");
            break ;
        }
        mdelay(5);
    }
    if(TimeOut == 0)
    {
        printk("Time out for wait KSV List checking interrupt\n");
        goto hdmitx_hdcp_Repeater_Fail ;
    }
    ///////////////////////////////////////
    // clear KSVList check interrupt.
    ///////////////////////////////////////

    for(TimeOut = 500 ; TimeOut > 0 ; TimeOut --)
    {
        printk("TimeOut=%d at wait FIFO ready\n",TimeOut);
        if((B_TX_INT_HPD_PLUG|B_TX_INT_RX_SENSE)&it66121_read(REG_TX_INT_STAT1))
        {
            printk("HPD at wait FIFO ready\n");
            goto hdmitx_hdcp_Repeater_Fail ;
        }
        if(hdmitx_hdcp_GetBCaps(&BCaps,&BStatus) == ER_FAIL)
        {
            printk("Get BCaps fail\n");
            goto hdmitx_hdcp_Repeater_Fail ;
        }
        if(BCaps & B_TX_CAP_KSV_FIFO_RDY)
        {
             printk("FIFO Ready\n");
             break ;
        }
        mdelay(5);

    }
    if(TimeOut == 0)
    {
        printk("Get KSV FIFO ready TimeOut\n");
        goto hdmitx_hdcp_Repeater_Fail ;
    }
    printk("Wait timeout = %d\n",TimeOut);

    hdmitx_ClearDDCFIFO();
    hdmitx_GenerateDDCSCLK();
    cDownStream =  (BStatus & M_TX_DOWNSTREAM_COUNT);

    if(/*cDownStream == 0 ||*/ cDownStream > 6 || BStatus & (B_TX_MAX_CASCADE_EXCEEDED|B_TX_DOWNSTREAM_OVER))
    {
        printk("Invalid Down stream count,fail\n");
        goto hdmitx_hdcp_Repeater_Fail ;
    }
#ifdef SUPPORT_SHA
    if(hdmitx_hdcp_GetKSVList(KSVList,cDownStream) == ER_FAIL)
    {
        goto hdmitx_hdcp_Repeater_Fail ;
    }

    if(hdmitx_hdcp_GetVr(Vr) == ER_FAIL)
    {
        goto hdmitx_hdcp_Repeater_Fail ;
    }
    if(hdmitx_hdcp_GetM0(M0) == ER_FAIL)
    {
        goto hdmitx_hdcp_Repeater_Fail ;
    }
    // do check SHA
    if(hdmitx_hdcp_CheckSHA(M0,BStatus,KSVList,cDownStream,Vr) == ER_FAIL)
    {
        goto hdmitx_hdcp_Repeater_Fail ;
    }
    if((B_TX_INT_HPD_PLUG|B_TX_INT_RX_SENSE)&it66121_read(REG_TX_INT_STAT1))
    {
        printk("HPD at Final\n");
        goto hdmitx_hdcp_Repeater_Fail ;
    }
#endif // SUPPORT_SHA

    hdmitx_hdcp_ResumeRepeaterAuthenticate();
    hdmiTxDev[0].bAuthenticated = TRUE ;
    return ER_SUCCESS ;

hdmitx_hdcp_Repeater_Fail:
    hdmitx_hdcp_CancelRepeaterAuthenticate();
    return ER_FAIL ;
}

//////////////////////////////////////////////////////////////////////
// Function: hdmitx_hdcp_ResumeAuthentication
// Parameter: N/A
// Return: N/A
// Remark: called by interrupt handler to restart Authentication and Encryption.
// Side-Effect: as Authentication and Encryption.
//////////////////////////////////////////////////////////////////////

void hdmitx_hdcp_ResumeAuthentication(void)
{
    setHDMITX_AVMute(TRUE);
    if(hdmitx_hdcp_Authenticate() == ER_SUCCESS)
    {
    }
    setHDMITX_AVMute(FALSE);
}

#endif // SUPPORT_HDCP

BOOL HDMITX_EnableHDCP(BYTE bEnable)
{
#ifdef SUPPORT_HDCP
    if(bEnable)
    {
        if(ER_FAIL == hdmitx_hdcp_Authenticate())
        {
            //printf("ER_FAIL == hdmitx_hdcp_Authenticate\n");
            hdmitx_hdcp_ResetAuth();
			return FALSE ;
        }
    }
    else
    {
        hdmiTxDev[0].bAuthenticated=FALSE;
        hdmitx_hdcp_ResetAuth();
    }
#endif
    return TRUE ;
}



//int it66121_config_video(struct hdmi_video *vpara)
int it66121_config_video(void)
{
	unsigned int tmdsclk;
	VIDEOPCLKLEVEL level ;
    struct fb_videomode *vmode;
	char bHDMIMode, pixelrep, bInputColorMode, bOutputColorMode, aspec, Colorimetry;
	
	//vmode = (struct fb_videomode*)hdmi_vic_to_videomode(vpara->vic);
	vmode = (struct fb_videomode*)hdmi_vic_to_videomode(VIDSTD_TEST);
	if(vmode == NULL)
		return HDMI_ERROR_FALSE;
	
	tmdsclk = vmode->pixclock;
	//bHDMIMode = hdmi->edid.sink_hdmi;
	bHDMIMode = 1;
	
	if(vmode->xres == 1280 || vmode->xres == 1920)
    {
    	aspec = HDMI_16x9;
    	Colorimetry = HDMI_ITU709;
    }
    else
    {
    	aspec = HDMI_4x3;
    	Colorimetry = HDMI_ITU601;
    }
    
    if(vmode->xres == 1440)
    	pixelrep = 1;
    else if(vmode->xres == 2880)
    	pixelrep = 3;
    else
    	pixelrep = 0;
	bInputColorMode = F_MODE_YUV422;

	bOutputColorMode = F_MODE_YUV422 ;

    HDMITX_DisableAudioOutput();
	HDMITX_EnableHDCP(FALSE);

    if( tmdsclk > 80000000L )
    {
        level = PCLK_HIGH ;
    }
    else if(tmdsclk > 20000000L)
    {
        level = PCLK_MEDIUM ;
    }
    else
    {
        level = PCLK_LOW ;
    }

	setHDMITX_VideoSignalType(InstanceData.bInputVideoSignalType);
    #ifdef SUPPORT_SYNCEMBEDDED
	if(InstanceData.bInputVideoSignalType & T_MODE_SYNCEMB)
	{
	    setHDMITX_SyncEmbeddedByVIC(VIDSTD_TEST,InstanceData.bInputVideoSignalType);
	}
    #endif

    printk("level = %d, ,bInputColorMode=%x,bOutputColorMode=%x,bHDMIMode=%x\n",(int)level,(int)bInputColorMode,(int)bOutputColorMode ,(int)bHDMIMode) ;
	HDMITX_EnableVideoOutput(level,bInputColorMode,bOutputColorMode ,bHDMIMode);

    if( bHDMIMode )
    {
        #ifdef OUTPUT_3D_MODE
        ConfigfHdmiVendorSpecificInfoFrame(OUTPUT_3D_MODE);
        #endif
        ConfigAVIInfoFrame(VIDSTD_TEST, pixelrep, aspec, Colorimetry, bOutputColorMode);
    }
	else
	{
		HDMITX_EnableAVIInfoFrame(FALSE ,NULL);
        HDMITX_EnableVSInfoFrame(FALSE,NULL);
	}

    setHDMITX_AVMute(0);
	//HDMITX_PowerOn();
    DumpHDMITXReg() ;
	
	return HDMI_ERROR_SUCESS;
}

#ifndef __EDID_H__
#define __EDID_H__

#define EDID_LENGTH				0x80
#define EDID_HEADER				0x00
#define EDID_HEADER_END				0x07

#define ID_MANUFACTURER_NAME			0x08
#define ID_MANUFACTURER_NAME_END		0x09
#define ID_MODEL				0x0a

#define ID_SERIAL_NUMBER			0x0c

#define MANUFACTURE_WEEK			0x10
#define MANUFACTURE_YEAR			0x11

#define EDID_STRUCT_VERSION			0x12
#define EDID_STRUCT_REVISION			0x13

#define EDID_STRUCT_DISPLAY                     0x14

#define DPMS_FLAGS				0x18
#define ESTABLISHED_TIMING_1			0x23
#define ESTABLISHED_TIMING_2			0x24
#define MANUFACTURERS_TIMINGS			0x25

/* standard timings supported */
#define STD_TIMING                              8
#define STD_TIMING_DESCRIPTION_SIZE             2
#define STD_TIMING_DESCRIPTIONS_START           0x26

#define DETAILED_TIMING_DESCRIPTIONS_START	0x36
#define DETAILED_TIMING_DESCRIPTION_SIZE	18
#define NO_DETAILED_TIMING_DESCRIPTIONS		4

#define DETAILED_TIMING_DESCRIPTION_1		0x36
#define DETAILED_TIMING_DESCRIPTION_2		0x48
#define DETAILED_TIMING_DESCRIPTION_3		0x5a
#define DETAILED_TIMING_DESCRIPTION_4		0x6c

#define DESCRIPTOR_DATA				5

#define UPPER_NIBBLE( x ) \
        (((128|64|32|16) & (x)) >> 4)

#define LOWER_NIBBLE( x ) \
        ((1|2|4|8) & (x))

#define COMBINE_HI_8LO( hi, lo ) \
        ( (((unsigned)hi) << 8) | (unsigned)lo )

#define COMBINE_HI_4LO( hi, lo ) \
        ( (((unsigned)hi) << 4) | (unsigned)lo )

#define PIXEL_CLOCK_LO     (unsigned)block[ 0 ]
#define PIXEL_CLOCK_HI     (unsigned)block[ 1 ]
#define PIXEL_CLOCK	   (COMBINE_HI_8LO( PIXEL_CLOCK_HI,PIXEL_CLOCK_LO )*10000)
#define H_ACTIVE_LO        (unsigned)block[ 2 ]
#define H_BLANKING_LO      (unsigned)block[ 3 ]
#define H_ACTIVE_HI        UPPER_NIBBLE( (unsigned)block[ 4 ] )
#define H_ACTIVE           COMBINE_HI_8LO( H_ACTIVE_HI, H_ACTIVE_LO )
#define H_BLANKING_HI      LOWER_NIBBLE( (unsigned)block[ 4 ] )
#define H_BLANKING         COMBINE_HI_8LO( H_BLANKING_HI, H_BLANKING_LO )

#define V_ACTIVE_LO        (unsigned)block[ 5 ]
#define V_BLANKING_LO      (unsigned)block[ 6 ]
#define V_ACTIVE_HI        UPPER_NIBBLE( (unsigned)block[ 7 ] )
#define V_ACTIVE           COMBINE_HI_8LO( V_ACTIVE_HI, V_ACTIVE_LO )
#define V_BLANKING_HI      LOWER_NIBBLE( (unsigned)block[ 7 ] )
#define V_BLANKING         COMBINE_HI_8LO( V_BLANKING_HI, V_BLANKING_LO )

#define H_SYNC_OFFSET_LO   (unsigned)block[ 8 ]
#define H_SYNC_WIDTH_LO    (unsigned)block[ 9 ]

#define V_SYNC_OFFSET_LO   UPPER_NIBBLE( (unsigned)block[ 10 ] )
#define V_SYNC_WIDTH_LO    LOWER_NIBBLE( (unsigned)block[ 10 ] )

#define V_SYNC_WIDTH_HI    ((unsigned)block[ 11 ] & (1|2))
#define V_SYNC_OFFSET_HI   (((unsigned)block[ 11 ] & (4|8)) >> 2)

#define H_SYNC_WIDTH_HI    (((unsigned)block[ 11 ] & (16|32)) >> 4)
#define H_SYNC_OFFSET_HI   (((unsigned)block[ 11 ] & (64|128)) >> 6)

#define V_SYNC_WIDTH       COMBINE_HI_4LO( V_SYNC_WIDTH_HI, V_SYNC_WIDTH_LO )
#define V_SYNC_OFFSET      COMBINE_HI_4LO( V_SYNC_OFFSET_HI, V_SYNC_OFFSET_LO )

#define H_SYNC_WIDTH       COMBINE_HI_8LO( H_SYNC_WIDTH_HI, H_SYNC_WIDTH_LO )
#define H_SYNC_OFFSET      COMBINE_HI_8LO( H_SYNC_OFFSET_HI, H_SYNC_OFFSET_LO )

#define H_SIZE_LO          (unsigned)block[ 12 ]
#define V_SIZE_LO          (unsigned)block[ 13 ]

#define H_SIZE_HI          UPPER_NIBBLE( (unsigned)block[ 14 ] )
#define V_SIZE_HI          LOWER_NIBBLE( (unsigned)block[ 14 ] )

#define H_SIZE             COMBINE_HI_8LO( H_SIZE_HI, H_SIZE_LO )
#define V_SIZE             COMBINE_HI_8LO( V_SIZE_HI, V_SIZE_LO )

#define H_BORDER           (unsigned)block[ 15 ]
#define V_BORDER           (unsigned)block[ 16 ]

#define FLAGS              (unsigned)block[ 17 ]

#define INTERLACED         (FLAGS&128)
#define SYNC_TYPE          (FLAGS&3<<3)	/* bits 4,3 */
#define SYNC_SEPARATE      (3<<3)
#define HSYNC_POSITIVE     (FLAGS & 4)
#define VSYNC_POSITIVE     (FLAGS & 2)

#define V_MIN_RATE              block[ 5 ]
#define V_MAX_RATE              block[ 6 ]
#define H_MIN_RATE              block[ 7 ]
#define H_MAX_RATE              block[ 8 ]
#define MAX_PIXEL_CLOCK         (((int)block[ 9 ]) * 10)
#define GTF_SUPPORT		block[10]

#define DPMS_ACTIVE_OFF		(1 << 5)
#define DPMS_SUSPEND		(1 << 6)
#define DPMS_STANDBY		(1 << 7)



#define hdmi_edid_error(fmt, ...) \
        printk(KERN_ERR pr_fmt(fmt), ##__VA_ARGS__)

#if 0
#define hdmi_edid_debug(fmt, ...) \
        printk(KERN_INFO pr_fmt(fmt), ##__VA_ARGS__)
#else
#define hdmi_edid_debug(fmt, ...)	
#endif

/* HDMI EDID Block Size */
#define HDMI_EDID_BLOCK_SIZE	128


typedef enum HDMI_EDID_ERRORCODE
{
	E_HDMI_EDID_SUCCESS = 0,
	E_HDMI_EDID_PARAM,
	E_HDMI_EDID_HEAD,
	E_HDMI_EDID_CHECKSUM,
	E_HDMI_EDID_VERSION,
	E_HDMI_EDID_UNKOWNDATA,
	E_HDMI_EDID_NOMEMORY
}HDMI_EDID_ErrorCode;

// HDMI EDID Information
struct hdmi_edid {
	unsigned char sink_hdmi;			//HDMI display device flag
	unsigned char ycbcr444;				//Display device support YCbCr444
	unsigned char ycbcr422;				//Display device support YCbCr422
	unsigned char deepcolor;			//bit3:DC_48bit; bit2:DC_36bit; bit1:DC_30bit; bit0:DC_Y444;
	unsigned int  cecaddress;			//CEC physical address
	unsigned int  maxtmdsclock;			//Max supported tmds clock
	unsigned char fields_present;		//bit7£ºlatency£»bit6£ºi_lantency£»bit5£ºhdmi_video
	unsigned char video_latency;
	unsigned char audio_latency;
	unsigned char interlaced_video_latency;
	unsigned char interlaced_audio_latency;
	
	struct fb_monspecs	*specs;			//Device spec
	struct list_head modelist;			//Device supported display mode list
	struct hdmi_audio *audio;			//Device supported audio info
	int	audio_num;						//Device supported audio type number
};

static struct hdmi_edid  it66121_edid;
static struct hdmi_edid *pedid;


static int hdmi_edid_checksum(unsigned char *buf)
{
	int i;
	int checksum = 0;
	
	for(i = 0; i < HDMI_EDID_BLOCK_SIZE; i++)
		checksum += buf[i];	
	
	checksum &= 0xff;
	
	if(checksum == 0)
		return E_HDMI_EDID_SUCCESS;
	else
		return E_HDMI_EDID_CHECKSUM;
}

/*
	@Des	Parse Detail Timing Descriptor.
	@Param	buf	:	pointer to DTD data.
	@Param	pvic:	VIC of DTD descripted.
 */
static int hdmi_edid_parse_dtd(unsigned char *block, struct fb_videomode *mode)
{
	mode->xres = H_ACTIVE;
	mode->yres = V_ACTIVE;
	mode->pixclock = PIXEL_CLOCK;
//	mode->pixclock /= 1000;
//	mode->pixclock = KHZ2PICOS(mode->pixclock);
	mode->right_margin = H_SYNC_OFFSET;
	mode->left_margin = (H_ACTIVE + H_BLANKING) -
		(H_ACTIVE + H_SYNC_OFFSET + H_SYNC_WIDTH);
	mode->upper_margin = V_BLANKING - V_SYNC_OFFSET -
		V_SYNC_WIDTH;
	mode->lower_margin = V_SYNC_OFFSET;
	mode->hsync_len = H_SYNC_WIDTH;
	mode->vsync_len = V_SYNC_WIDTH;
	if (HSYNC_POSITIVE)
		mode->sync |= FB_SYNC_HOR_HIGH_ACT;
	if (VSYNC_POSITIVE)
		mode->sync |= FB_SYNC_VERT_HIGH_ACT;
	mode->refresh = PIXEL_CLOCK/((H_ACTIVE + H_BLANKING) *
				     (V_ACTIVE + V_BLANKING));
	if (INTERLACED) {
		mode->yres *= 2;
		mode->upper_margin *= 2;
		mode->lower_margin *= 2;
		mode->vsync_len *= 2;
		mode->vmode |= FB_VMODE_INTERLACED;
	}
	mode->flag = FB_MODE_IS_DETAILED;

	hdmi_edid_debug("<<<<<<<<Detailed Time>>>>>>>>>\n");
	hdmi_edid_debug("%d KHz Refresh %d Hz",  PIXEL_CLOCK/1000, mode->refresh);
	hdmi_edid_debug("%d %d %d %d ", H_ACTIVE, H_ACTIVE + H_SYNC_OFFSET,
	       H_ACTIVE + H_SYNC_OFFSET + H_SYNC_WIDTH, H_ACTIVE + H_BLANKING);
	hdmi_edid_debug("%d %d %d %d ", V_ACTIVE, V_ACTIVE + V_SYNC_OFFSET,
	       V_ACTIVE + V_SYNC_OFFSET + V_SYNC_WIDTH, V_ACTIVE + V_BLANKING);
	hdmi_edid_debug("%sHSync %sVSync\n\n", (HSYNC_POSITIVE) ? "+" : "-",
	       (VSYNC_POSITIVE) ? "+" : "-");
	return E_HDMI_EDID_SUCCESS;
}

int hdmi_edid_parse_base(unsigned char *buf, int *extend_num, struct hdmi_edid *pedid)
{
	int rc, i;
	
	if(buf == NULL || extend_num == NULL)
		return E_HDMI_EDID_PARAM;
		
	#if 1	
	for(i = 0; i < HDMI_EDID_BLOCK_SIZE; i++)
	{
		printk("%02x ", buf[i]&0xff);
		if((i+1) % 16 == 0)
			printk("\n");
	}
	#endif
	
	// Check first 8 byte to ensure it is an edid base block.
	if( buf[0] != 0x00 ||
	    buf[1] != 0xFF ||
	    buf[2] != 0xFF ||
	    buf[3] != 0xFF ||
	    buf[4] != 0xFF ||
	    buf[5] != 0xFF ||
	    buf[6] != 0xFF ||
	    buf[7] != 0x00)
    {
        hdmi_edid_error("[EDID] check header error\n");
        return E_HDMI_EDID_HEAD;
    }
    
    *extend_num = buf[0x7e];
    #ifdef DEBUG
    hdmi_edid_debug("[EDID] extend block num is %d\n", buf[0x7e]);
    #endif
    
    // Checksum
    rc = hdmi_edid_checksum(buf);
    if( rc != E_HDMI_EDID_SUCCESS)
    {
    	hdmi_edid_error("[EDID] base block checksum error\n");
    	return E_HDMI_EDID_CHECKSUM;
    }

	pedid->specs = kzalloc(sizeof(struct fb_monspecs), GFP_KERNEL);
	if(pedid->specs == NULL)
		return E_HDMI_EDID_NOMEMORY;
		
	fb_edid_to_monspecs(buf, pedid->specs);
	
    return E_HDMI_EDID_SUCCESS;
}

/* HDMI mode list*/
struct display_modelist {
	struct list_head 	list;
	struct fb_videomode	mode;
	unsigned int 		vic;
	unsigned int		format_3d;
	unsigned int		detail_3d;
};

/**
 * hdmi_videomode_to_vic: transverse video mode to vic
 * @vmode: videomode to transverse
 * 
 */
int hdmi_videomode_to_vic(struct fb_videomode *vmode)
{
	struct fb_videomode *mode;
	unsigned char vic = 0;
	int i = 0;
	
	for(i = 0; i < ARRAY_SIZE(hdmi_mode); i++)
	{
		mode = (struct fb_videomode*) &(hdmi_mode[i].mode);
		if(	vmode->vmode == mode->vmode &&
			vmode->refresh == mode->refresh &&
			vmode->xres == mode->xres && 
			vmode->left_margin == mode->left_margin &&
			vmode->right_margin == mode->right_margin &&
			vmode->upper_margin == mode->upper_margin &&
			vmode->lower_margin == mode->lower_margin && 
			vmode->hsync_len == mode->hsync_len && 
			vmode->vsync_len == mode->vsync_len)
		{
			if( (vmode->vmode == FB_VMODE_NONINTERLACED && vmode->yres == mode->yres) || 
				(vmode->vmode == FB_VMODE_INTERLACED && vmode->yres == mode->yres/2))
			{								
				vic = hdmi_mode[i].vic;
				break;
			}
		}
	}
	return vic;
}

int hdmi_add_vic(int vic, struct list_head *head)
{
	struct list_head *pos;
	struct display_modelist *modelist;
	int found = 0, v;

//	DBG("%s vic %d", __FUNCTION__, vic);
	if(vic == 0)
		return -1;
		
	list_for_each(pos, head) {
		modelist = list_entry(pos, struct display_modelist, list);
		v = modelist->vic;
		if (v == vic) {
			found = 1;
			break;
		}
	}
	if (!found) {
		modelist = kmalloc(sizeof(struct display_modelist),
						  GFP_KERNEL);

		if (!modelist)
			return -ENOMEM;
		memset(modelist, 0, sizeof(struct display_modelist));
		modelist->vic = vic;
		list_add_tail(&modelist->list, head);
	}
	return 0;
}

// Parse CEA Short Video Descriptor
static int hdmi_edid_get_cea_svd(unsigned char *buf, struct hdmi_edid *pedid)
{
	int count, i, vic;
	
	count = buf[0] & 0x1F;
	for(i = 0; i < count; i++)
	{
		hdmi_edid_debug("[EDID-CEA] %02x VID %d native %d\n", buf[1 + i], buf[1 + i] & 0x7f, buf[1 + i] >> 7);
		vic = buf[1 + i] & 0x7f;
		hdmi_add_vic(vic, &pedid->modelist);
	}
	
//	struct list_head *pos;
//	struct display_modelist *modelist;
//
//	list_for_each(pos, &pedid->modelist) {
//		modelist = list_entry(pos, struct display_modelist, list);
//		printk("%s vic %d\n", __FUNCTION__, modelist->vic);
//	}	
	return 0;
}
#if 0
// Parse CEA Short Audio Descriptor
static int hdmi_edid_parse_cea_sad(unsigned char *buf, struct hdmi_edid *pedid)
{
	int i, count;
	
	count = buf[0] & 0x1F;
	pedid->audio = kmalloc((count/3)*sizeof(struct hdmi_audio), GFP_KERNEL);
	if(pedid->audio == NULL)
		return E_HDMI_EDID_NOMEMORY;

	pedid->audio_num = count/3;
	for(i = 0; i < pedid->audio_num; i++)
	{
		pedid->audio[i].type = (buf[1 + i*3] >> 3) & 0x0F;
		pedid->audio[i].channel = (buf[1 + i*3] & 0x07) + 1;
		pedid->audio[i].rate = buf[1 + i*3 + 1];
		if(pedid->audio[i].type == HDMI_AUDIO_LPCM)//LPCM 
		{
			pedid->audio[i].word_length = buf[1 + i*3 + 2];
		}
//		printk("[EDID-CEA] type %d channel %d rate %d word length %d\n", 
//			pedid->audio[i].type, pedid->audio[i].channel, pedid->audio[i].rate, pedid->audio[i].word_length);
	}
	return E_HDMI_EDID_SUCCESS;
}

static int hdmi_edid_parse_3dinfo(unsigned char *hdmi_edid_parse_3dinfobuf, struct list_head *head)
{
	int i, j, len = 0, format_3d, vic_mask;
	unsigned char offset = 2, vic_2d, structure_3d;
	struct list_head *pos;
	struct display_modelist *modelist;
	
	if(buf[1] & 0xF0) {
		len = (buf[1] & 0xF0) >> 4;
		for(i = 0; i < len; i++) {
			hdmi_add_vic( (buf[offset++] | HDMI_VIDEO_EXT), head);
		}
	}
	
	if(buf[0] & 0x80) {
		//3d supported
		len += (buf[0] & 0x0F) + 2;
		if( ( (buf[0] & 0x60) == 0x40) || ( (buf[0] & 0x60) == 0x20) ) {
			format_3d = buf[offset++] << 8;
			format_3d |= buf[offset++];
		}
		if( (buf[0] & 0x60) == 0x40)
			vic_mask = 0xFFFF;
		else {
			vic_mask  = buf[offset++] << 8;
			vic_mask |= buf[offset++];
		}

		for(i = 0; i < 16; i++)
		{
			if(vic_mask & (1 << i)) {
				j = 0;
				for (pos = (head)->next; pos != (head); pos = pos->next) {
					j++;
					if(j == i) {
						modelist = list_entry(pos, struct display_modelist, list);
						modelist->format_3d = format_3d;
						break;
					}
				}
			}
		}
		while(offset < len)
		{
			vic_2d = (buf[offset] & 0xF0) >> 4;
			structure_3d = (buf[offset++] & 0x0F);
			j = 0;
			for (pos = (head)->next; pos != (head); pos = pos->next) {
				j++;
				if(j == vic_2d) {
					modelist = list_entry(pos, struct display_modelist, list);
					modelist->format_3d = format_3d;
					if(structure_3d & 0x80)
					modelist->detail_3d = (buf[offset++] & 0xF0) >> 4;
					break;
				}
			}
		}
	}
	
	return 0;
}
#endif

// Parse CEA 861 Serial Extension.
static int hdmi_edid_parse_extensions_cea(unsigned char *buf, struct hdmi_edid *pedid)
{
	unsigned int ddc_offset, native_dtd_num, cur_offset = 4, buf_offset;
//	unsigned int underscan_support, baseaudio_support;
	unsigned int tag, IEEEOUI = 0, count;
	
	if(buf == NULL)
		return E_HDMI_EDID_PARAM;
		
	// Check ces extension version
	if(buf[1] != 3)
	{
		hdmi_edid_error("[EDID-CEA] error version.\n");
		return E_HDMI_EDID_VERSION;
	}
	
	ddc_offset = buf[2];
//	underscan_support = (buf[3] >> 7) & 0x01;
//	baseaudio_support = (buf[3] >> 6) & 0x01;
	pedid->ycbcr444 = (buf[3] >> 5) & 0x01;
	pedid->ycbcr422 = (buf[3] >> 4) & 0x01;
	native_dtd_num = buf[3] & 0x0F;
//	hdmi_edid_debug("[EDID-CEA] ddc_offset %d underscan_support %d baseaudio_support %d yuv_support %d native_dtd_num %d\n", ddc_offset, underscan_support, baseaudio_support, yuv_support, native_dtd_num);
	// Parse data block
	while(cur_offset < ddc_offset)
	{
		tag = buf[cur_offset] >> 5;
		count = buf[cur_offset] & 0x1F;
		switch(tag)
		{
			case 0x02:	// Video Data Block
				hdmi_edid_debug("[EDID-CEA] It is a Video Data Block.\n");
				hdmi_edid_get_cea_svd(buf + cur_offset, pedid);
				break;
			case 0x01:	// Audio Data Block
				hdmi_edid_debug("[EDID-CEA] It is a Audio Data Block.\n");
				//hdmi_edid_parse_cea_sad(buf + cur_offset, pedid);
				break;
			case 0x04:	// Speaker Allocation Data Block
				hdmi_edid_debug("[EDID-CEA] It is a Speaker Allocatio Data Block.\n");
				break;
			case 0x03:	// Vendor Specific Data Block
				hdmi_edid_debug("[EDID-CEA] It is a Vendor Specific Data Block.\n");

				IEEEOUI = buf[cur_offset + 3];
				IEEEOUI <<= 8;
				IEEEOUI += buf[cur_offset + 2];
				IEEEOUI <<= 8;
				IEEEOUI += buf[cur_offset + 1];
				hdmi_edid_debug("[EDID-CEA] IEEEOUI is 0x%08x.\n", IEEEOUI);
				if(IEEEOUI == 0x0c03)
					pedid->sink_hdmi = 1;
				pedid->cecaddress = buf[cur_offset + 5];
				pedid->cecaddress |= buf[cur_offset + 4] << 8;
				hdmi_edid_debug("[EDID-CEA] CEC Physical addres is 0x%08x.\n", pedid->cecaddress);
				if(count > 6)
					pedid->deepcolor = (buf[cur_offset + 6] >> 3) & 0x0F;					
				if(count > 7) {
					pedid->maxtmdsclock = buf[cur_offset + 7] * 5000000;
					hdmi_edid_debug("[EDID-CEA] maxtmdsclock is %d.\n", pedid->maxtmdsclock);
				}
				if(count > 8) {
					pedid->fields_present = buf[cur_offset + 8];
					hdmi_edid_debug("[EDID-CEA] fields_present is 0x%02x.\n", pedid->fields_present);
				}
				buf_offset = cur_offset + 9;		
				if(pedid->fields_present & 0x80)
				{
					pedid->video_latency = buf[buf_offset++];
					pedid->audio_latency = buf[buf_offset++];
				}
				if(pedid->fields_present & 0x40)
				{
					pedid->interlaced_video_latency = buf[buf_offset++];
					pedid->interlaced_audio_latency = buf[buf_offset++];
				}
				if(pedid->fields_present & 0x20) {
					//hdmi_edid_parse_3dinfo(buf + buf_offset, &pedid->modelist);
				}
				break;		
			case 0x05:	// VESA DTC Data Block
				hdmi_edid_debug("[EDID-CEA] It is a VESA DTC Data Block.\n");
				break;
			case 0x07:	// Use Extended Tag
				hdmi_edid_debug("[EDID-CEA] It is a Use Extended Tag Data Block.\n");
				break;
			default:
				hdmi_edid_error("[EDID-CEA] unkowned data block tag.\n");
				break;
		}
		cur_offset += (buf[cur_offset] & 0x1F) + 1;
	}
#if 1	
{
	// Parse DTD
	struct fb_videomode *vmode = kmalloc(sizeof(struct fb_videomode), GFP_KERNEL);
	if(vmode == NULL)
		return E_HDMI_EDID_SUCCESS; 
	while(ddc_offset < HDMI_EDID_BLOCK_SIZE - 2)	//buf[126] = 0 and buf[127] = checksum
	{
		if(!buf[ddc_offset] && !buf[ddc_offset + 1])
			break;
		memset(vmode, 0, sizeof(struct fb_videomode));
		hdmi_edid_parse_dtd(buf + ddc_offset, vmode);		
		hdmi_add_vic(hdmi_videomode_to_vic(vmode), &pedid->modelist);
		ddc_offset += 18;
	}
	kfree(vmode);
}
#endif
	return E_HDMI_EDID_SUCCESS;
}

int hdmi_edid_parse_extensions(unsigned char *buf, struct hdmi_edid *pedid)
{
	int rc;
	
	if(buf == NULL || pedid == NULL)
		return E_HDMI_EDID_PARAM;
		
	// Checksum
    rc = hdmi_edid_checksum(buf);
    if( rc != E_HDMI_EDID_SUCCESS)
    {
    	hdmi_edid_error("[EDID] extensions block checksum error\n");
    	return E_HDMI_EDID_CHECKSUM;
    }
    
    switch(buf[0])
    {
    	case 0xF0:
    		hdmi_edid_debug("[EDID-EXTEND] It is a extensions block map.\n");
    		break;
    	case 0x02:
    		hdmi_edid_debug("[EDID-EXTEND] It is a  CEA 861 Series Extension.\n");
    		hdmi_edid_parse_extensions_cea(buf, pedid);
    		break;
    	case 0x10:
    		hdmi_edid_debug("[EDID-EXTEND] It is a Video Timing Block Extension.\n");
    		break;
    	case 0x40:
    		hdmi_edid_debug("[EDID-EXTEND] It is a Display Information Extension.\n");
    		break;
    	case 0x50:
    		hdmi_edid_debug("[EDID-EXTEND] It is a Localized String Extension.\n");
    		break;
    	case 0x60:
    		hdmi_edid_debug("[EDID-EXTEND] It is a Digital Packet Video Link Extension.\n");
    		break;
    	default:
    		hdmi_edid_error("[EDID-EXTEND] Unkowned extension.\n");
    		return E_HDMI_EDID_UNKOWNDATA;
    }
    
    return E_HDMI_EDID_SUCCESS;
}



#endif /* __EDID_H__ */


#if 1


//////////////////////////////////////////////////////////////////////
// Function: hdmitx_ClearDDCFIFO
// Parameter: N/A
// Return: N/A
// Remark: clear the DDC FIFO.
// Side-Effect: DDC master will set to be HOST.
//////////////////////////////////////////////////////////////////////

void hdmitx_ClearDDCFIFO()
{
    it66121_write(REG_TX_DDC_MASTER_CTRL,B_TX_MASTERDDC|B_TX_MASTERHOST);
    it66121_write(REG_TX_DDC_CMD,CMD_FIFO_CLR);
}

void hdmitx_GenerateDDCSCLK()
{
    it66121_write(REG_TX_DDC_MASTER_CTRL,B_TX_MASTERDDC|B_TX_MASTERHOST);
    it66121_write(REG_TX_DDC_CMD,CMD_GEN_SCLCLK);
}

//////////////////////////////////////////////////////////////////////
// Function: hdmitx_AbortDDC
// Parameter: N/A
// Return: N/A
// Remark: Force abort DDC and reset DDC bus.
// Side-Effect:
//////////////////////////////////////////////////////////////////////
#define REG_TX_HDCP_DESIRE 0x20
    #define B_TX_ENABLE_HDPC11 (1<<1)
    #define B_TX_CPDESIRE  (1<<0)


void hdmitx_AbortDDC()
{
    BYTE CPDesire,SWReset,DDCMaster ;
    BYTE uc, timeout, i ;
    // save the SW reset,DDC master,and CP Desire setting.
    SWReset = it66121_read(REG_TX_SW_RST);
    CPDesire = it66121_read(REG_TX_HDCP_DESIRE);
    DDCMaster = it66121_read(REG_TX_DDC_MASTER_CTRL);

    it66121_write(REG_TX_HDCP_DESIRE,CPDesire&(~B_TX_CPDESIRE)); // @emily change order
    it66121_write(REG_TX_SW_RST,SWReset|B_TX_HDCP_RST_HDMITX);         // @emily change order
    it66121_write(REG_TX_DDC_MASTER_CTRL,B_TX_MASTERDDC|B_TX_MASTERHOST);

    // 2009/01/15 modified by Jau-Chih.Tseng@ite.com.tw
    // do abort DDC twice.
    for( i = 0 ; i < 2 ; i++ )
    {
        it66121_write(REG_TX_DDC_CMD,CMD_DDC_ABORT);

        for( timeout = 0 ; timeout < 200 ; timeout++ )
        {
            uc = it66121_read(REG_TX_DDC_STATUS);
            if (uc&B_TX_DDC_DONE)
            {
                break ; // success
            }
            if( uc & (B_TX_DDC_NOACK|B_TX_DDC_WAITBUS|B_TX_DDC_ARBILOSE) )
            {
//                HDMITX_DEBUG_PRINTF(("hdmitx_AbortDDC Fail by reg16=%02X\n",(int)uc));
                break ;
            }
            mdelay(1); // delay 1 ms to stable.
        }
    }
    //~Jau-Chih.Tseng@ite.com.tw

}




//////////////////////////////////////////////////////////////////////
// Function: getHDMITX_EDIDBytes
// Parameter: pData - the pointer of buffer to receive EDID ucdata.
//            bSegment - the segment of EDID readback.
//            offset - the offset of EDID ucdata in the segment. in byte.
//            count - the read back bytes count,cannot exceed 32
// Return: ER_SUCCESS if successfully getting EDID. ER_FAIL otherwise.
// Remark: function for read EDID ucdata from reciever.
// Side-Effect: DDC master will set to be HOST. DDC FIFO will be used and dirty.
//////////////////////////////////////////////////////////////////////

SYS_STATUS getHDMITX_EDIDBytes(BYTE *pData,BYTE bSegment,BYTE offset,short Count)
{
    short RemainedCount,ReqCount ;
    BYTE bCurrOffset ;
    short TimeOut ;
    BYTE *pBuff = pData ;
    BYTE ucdata ;

    // printk(("getHDMITX_EDIDBytes(%08lX,%d,%d,%d)\n",(ULONG)pData,(int)bSegment,(int)offset,(int)Count));
    if(!pData)
    {
//        printk(("getHDMITX_EDIDBytes(): Invallid pData pointer %08lX\n",(ULONG)pData));
        return ER_FAIL ;
    }
    if(it66121_read(REG_TX_INT_STAT1) & B_TX_INT_DDC_BUS_HANG)
    {
        printk("Called hdmitx_AboutDDC()\n");
        hdmitx_AbortDDC();

    }
    // HDMITX_OrReg_Byte(REG_TX_INT_CTRL,(1<<1));

    hdmitx_ClearDDCFIFO();

    RemainedCount = Count ;
    bCurrOffset = offset ;

    Switch_HDMITX_Bank(0);

    while(RemainedCount > 0)
    {

        ReqCount = (RemainedCount > DDC_FIFO_MAXREQ)?DDC_FIFO_MAXREQ:RemainedCount ;
        printk("getHDMITX_EDIDBytes(): ReqCount = %d,bCurrOffset = %d\n",(int)ReqCount,(int)bCurrOffset);

        it66121_write(REG_TX_DDC_MASTER_CTRL,B_TX_MASTERDDC|B_TX_MASTERHOST);
        it66121_write(REG_TX_DDC_CMD,CMD_FIFO_CLR);

        for(TimeOut = 0 ; TimeOut < 200 ; TimeOut++)
        {
            ucdata = it66121_read(REG_TX_DDC_STATUS);

            if(ucdata&B_TX_DDC_DONE)
            {
                break ;
            }
            if((ucdata & B_TX_DDC_ERROR)||(it66121_read(REG_TX_INT_STAT1) & B_TX_INT_DDC_BUS_HANG))
            {
                printk("Called hdmitx_AboutDDC()\n");
                hdmitx_AbortDDC();
                return ER_FAIL ;
            }
        }
        it66121_write(REG_TX_DDC_MASTER_CTRL,B_TX_MASTERDDC|B_TX_MASTERHOST);
        it66121_write(REG_TX_DDC_HEADER,DDC_EDID_ADDRESS); // for EDID ucdata get
        it66121_write(REG_TX_DDC_REQOFF,bCurrOffset);
        it66121_write(REG_TX_DDC_REQCOUNT,(BYTE)ReqCount);
        it66121_write(REG_TX_DDC_EDIDSEG,bSegment);
        it66121_write(REG_TX_DDC_CMD,CMD_EDID_READ);

        bCurrOffset += ReqCount ;
        RemainedCount -= ReqCount ;

        for(TimeOut = 250 ; TimeOut > 0 ; TimeOut --)
        {
            mdelay(1);
            ucdata = it66121_read(REG_TX_DDC_STATUS);
            if(ucdata & B_TX_DDC_DONE)
            {
                break ;
            }
            if(ucdata & B_TX_DDC_ERROR)
            {
                printk("getHDMITX_EDIDBytes(): DDC_STATUS = %02X,fail.\n",(int)ucdata);
                // HDMITX_AndReg_Byte(REG_TX_INT_CTRL,~(1<<1));
                return ER_FAIL ;
            }
        }
        if(TimeOut == 0)
        {
            printk("getHDMITX_EDIDBytes(): DDC TimeOut. \n");
            // HDMITX_AndReg_Byte(REG_TX_INT_CTRL,~(1<<1));
            return ER_FAIL ;
        }
        do
        {
            *(pBuff++) = it66121_read(REG_TX_DDC_READFIFO);
            ReqCount -- ;
        }while(ReqCount > 0);

    }
    // HDMITX_AndReg_Byte(REG_TX_INT_CTRL,~(1<<1));
    return ER_SUCCESS ;
}


int it66121_read_edid(int block, unsigned char *buff)
{
    if(getHDMITX_EDIDBytes(buff,block/2,(block%2)*128,128) == ER_FAIL)
    {
        return FALSE ;
    }
    return TRUE ;

}


static void hdmi_wq_parse_edid(void)
{
	//struct hdmi_edid *pedid;
	unsigned char *buff = NULL;
	int rc = HDMI_ERROR_SUCESS, extendblock = 0, i;
	
	//if(hdmi == NULL) return;
		
	printk("%s", __FUNCTION__);
	
	//pedid = &(hdmi->edid);
	pedid = &(it66121_edid);
	//fb_destroy_modelist(&pedid->modelist);
	memset(pedid, 0, sizeof(struct hdmi_edid));
	INIT_LIST_HEAD(&pedid->modelist);
	
	buff = kmalloc(HDMI_EDID_BLOCK_SIZE, GFP_KERNEL);
	if(buff == NULL) {		
		printk("hdmi_wq_parse_edid can not allocate memory for edid buff.\n");
		rc = HDMI_ERROR_FALSE;
		goto out;
	}
	
	// Read base block edid.
	memset(buff, 0 , HDMI_EDID_BLOCK_SIZE);
	rc = it66121_read_edid(0, buff);
	if(rc) {
		printk( "[HDMI] read edid base block error\n");
		goto out;
	}
	
	rc = hdmi_edid_parse_base(buff, &extendblock, pedid);
	if(rc) {
		printk("[HDMI] parse edid base block error\n");
		goto out;
	}
	
	for(i = 1; i < extendblock + 1; i++) {
		memset(buff, 0 , HDMI_EDID_BLOCK_SIZE);
		rc = it66121_read_edid(i, buff);
		if(rc) {
			printk("[HDMI] read edid block %d error\n", i);	
			goto out;
		}

		rc = hdmi_edid_parse_extensions(buff, pedid);
		if(rc) {
			printk("[HDMI] parse edid block %d error\n",i);
			continue;
		}
	}
out:
	if(buff)
		kfree(buff);
	//rc = hdmi_ouputmode_select(hdmi, rc);
}

#endif

static int it66121_device_init(void)
{

	unsigned char VendorID0, VendorID1, DeviceID0, DeviceID1;

	it66121_reset();
	
	Switch_HDMITX_Bank(0);
	VendorID0 = it66121_read(REG_TX_VENDOR_ID0);
	VendorID1 = it66121_read(REG_TX_VENDOR_ID1);
	DeviceID0 = it66121_read(REG_TX_DEVICE_ID0);
	DeviceID1 = it66121_read(REG_TX_DEVICE_ID1);
	printk("Reg[0-3] = 0x[%02x].[%02x].[%02x].[%02x]",
			   VendorID0, VendorID1, DeviceID0, DeviceID1);
	if( (VendorID0 == 0x54) && (VendorID1 == 0x49) &&
		(DeviceID0 == 0x12) && (DeviceID1 == 0x16) )
	{
		HDMITX_InitTxDev(&InstanceData);
		InitHDMITX();
		
		HDMITX_PowerOn();
		hdmi_wq_parse_edid();
		it66121_config_video();
		return 0;
	}
	printk(KERN_ERR "IT66121: Device not found!\n");

	return 1;
}

static int it66121_s_std_output(struct v4l2_subdev *sd, v4l2_std_id norm)
{

	it66121_device_init();
	if (norm & (V4L2_STD_ALL & ~V4L2_STD_SECAM))
		return 0;
	else if (norm & (V4L2_STD_525P_60 | V4L2_STD_625P_50))
		return 0;
	else if (norm & (V4L2_STD_720P_60 | V4L2_STD_720P_50 |
				V4L2_STD_1080I_60 | V4L2_STD_1080I_50))
		return 0;
	else if (norm & (V4L2_STD_1080P_60 | V4L2_STD_1080P_50))
		return 0;
	else
		return -EINVAL;
}

/* ----------------------------------------------------------------------- */

static const struct v4l2_subdev_core_ops it66121_core_ops = {
	.s_std = it66121_s_std_output,
};

static const struct v4l2_subdev_video_ops it66121_video_ops = {
	//.querystd = tvp7002_querystd,
	//.g_fmt = tvp7002_g_fmt_cap,
	//.s_stream = tvp7002_s_stream,

};

static const struct v4l2_subdev_ops it66121_ops = {
	.core = &it66121_core_ops,
	.video = &it66121_video_ops,
};

static const struct file_operations it66121_fops = {
	.owner		= THIS_MODULE,
	.read           = (void*)it66121_readreg,
	.write           = it66121_writereg,
	.ioctl		= it66121_ioctl,
	.open		= it66121_open,
	.release	= it66121_close,

};

static struct miscdevice it66121_dev = {
 	.minor = IT66121_DEVICE_MINOR,  
	.name = IT66121_DRIVER_NAME,  
	.fops = &it66121_fops,  
};

static int it66121_probe(struct i2c_client *c,
			 const struct i2c_device_id *id)
{
	int ret;
	
	/* Check if the adapter supports the needed features */
	if (!i2c_check_functionality(c->adapter,
	     I2C_FUNC_SMBUS_READ_BYTE | I2C_FUNC_SMBUS_WRITE_BYTE_DATA))
		return -EIO;

	it66121_sd = kzalloc(sizeof(struct v4l2_subdev), GFP_KERNEL);
	if (it66121_sd == NULL)
		return -ENOMEM;
	v4l2_i2c_subdev_init(it66121_sd, c, &it66121_ops);
	v4l_info(c, "chip found @ 0x%02x (%s)\n",
		 c->addr << 1, c->adapter->name);

	it66121_device_init();

	printk("IT66121: probe OK!\n");

	ret = misc_register(&it66121_dev);
	printk(IT66121_DRIVER_NAME"\t misc initialized %s!\n", (0==ret)?"successed":"failed");

	return 0;
}


static int it66121_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);

	v4l2_device_unregister_subdev(sd);

	misc_deregister(&it66121_dev);

	return 0;
}
/* ----------------------------------------------------------------------- */

static const struct i2c_device_id it66121_id[] = {
	{ "it66121", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, it66121_id);

static struct i2c_driver it66121_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "it66121",
	},	
	.command = (void*)it66121_ioctl,
	.probe = it66121_probe,
	.remove = it66121_remove,
	.id_table = it66121_id,
};
static int __init it66121_init(void)
{
	return i2c_add_driver(&it66121_driver);
}

static void __exit it66121_exit(void)
{
	i2c_del_driver(&it66121_driver);
}

module_init(it66121_init);
module_exit(it66121_exit);
MODULE_LICENSE("GPL");


