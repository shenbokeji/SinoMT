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
//#include <media/v4l2-i2c-drv.h>
#include <linux/miscdevice.h>  

#include <linux/delay.h>
#include <media/v4l2-subdev.h>
#include <media/davinci/videohd.h>



#define IT66121_DRIVER_NAME  "it66121_reg"
#define IT66121_DEVICE_MINOR   226


MODULE_DESCRIPTION("ITW Devices IT66121 video HDMI encoder driver");
MODULE_AUTHOR("Dave Perks");
MODULE_LICENSE("GPL");

#include "it66121.h"

static int debug;
module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "Debug level (0-1)");

static unsigned char buf[2]={};

#define TX0ADR		0x9A
//#define TX0DEV  	0x00
#define TX0CECADR   0x9C
#define RXADR   	0x90


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

unsigned char it66121_read(unsigned char address)
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
	gpio_request(24, NULL);
	gpio_direction_output(24, 1);

	// Reset it66121
	gpio_request(17, NULL);
	gpio_direction_output(17, 1);


}

static ssize_t it66121_open(struct inode * indoe, struct file * file)
{
	return 0;
}

static ssize_t it66121_close(struct inode * indoe, struct file * file)
{
	return 0;
}

static void it66121_write_table(unsigned char addr, unsigned char num, unsigned char * value)
{
	unsigned char i;
	unsigned char j=0;
	unsigned char k=1;

	for ( i = 0; i < num; i++) {
		it66121_write(*(value + j), *(value + k));
		j += 2; k += 2;
	}
}

typedef struct IT66121RegStruct {
    unsigned int uiAddr;	//I2C reg, address  
    unsigned int uiValue;		//I2C bus,data
} tIT66121Reg;

static tIT66121Reg it66121_reg={0};


//static unsigned int rw_buf=0;
ssize_t it66121_readreg(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{

		copy_from_user(&it66121_reg, buf, 8);
		it66121_reg.uiValue = it66121_read((unsigned char)it66121_reg.uiAddr);
		copy_to_user(buf, &it66121_reg, 8); 
		return 8;
}

ssize_t it66121_writereg(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{

    copy_from_user(&it66121_reg, buf, 8);
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
void DumpHDMITXReg()
{
    int i,j ;
    BYTE ucData ;

    printk("       ");
    for(j = 0 ; j < 16 ; j++)
    {
        printk(" %02X",(int)j);
        if((j == 3)||(j==7)||(j==11))
        {
            printk("  ");
        }
    }
    printk("\n        -----------------------------------------------------\n");

    Switch_HDMITX_Bank(0);

    for(i = 0 ; i < 0x100 ; i+=16)
    {
        printk("[%3X]  ",i);
        for(j = 0 ; j < 16 ; j++)
        {
            if( (i+j)!= 0x17)
            {
                ucData = it66121_read((BYTE)((i+j)&0xFF));
                printk(" %02X",(int)ucData);
            }
            else
            {
                printk(" XX",(int)ucData); // for DDC FIFO
            }
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





void InitHDMITX()
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

void HDMITX_DisableVideoOutput()
{
    BYTE uc = it66121_read(REG_TX_SW_RST) | B_HDMITX_VID_RST ;
    it66121_write(REG_TX_SW_RST,uc);
    it66121_write(REG_TX_AFE_DRV_CTRL,B_TX_AFE_DRV_RST|B_TX_AFE_DRV_PWD);
    HDMITX_SetI2C_Byte(0x62, 0x90, 0x00);
    HDMITX_SetI2C_Byte(0x64, 0x89, 0x00);
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

    //hdmitx_SetupAFE(level); // pass if High Freq request
    it66121_write(REG_TX_SW_RST,          B_HDMITX_AUD_RST|B_TX_AREF_RST|B_TX_HDCP_RST_HDMITX);

   // hdmitx_FireAFE();

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
    int i ;
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
#define SUPPORT_SYNCEMBEDDED

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

void HDMITX_DisableAudioOutput()
{
    //BYTE uc = (HDMITX_ReadI2C_Byte(REG_TX_SW_RST) | (B_HDMITX_AUD_RST | B_TX_AREF_RST));
    //it66121_write(REG_TX_SW_RST,uc);
    AudioDelayCnt=250;
    LastRefaudfreqnum=0;
    HDMITX_SetI2C_Byte(REG_TX_SW_RST, (B_HDMITX_AUD_RST | B_TX_AREF_RST), (B_HDMITX_AUD_RST | B_TX_AREF_RST) );
    HDMITX_SetI2C_Byte(0x0F, 0x10, 0x10 );
}


//int it66121_config_video(struct hdmi_video *vpara)
int it66121_config_video(void)
{
	unsigned int tmdsclk;
	VIDEOPCLKLEVEL level ;
    struct fb_videomode *vmode;
	char bHDMIMode, pixelrep, bInputColorMode, bOutputColorMode, aspec, Colorimetry;
	
	//vmode = (struct fb_videomode*)hdmi_vic_to_videomode(vpara->vic);
	vmode = (struct fb_videomode*)hdmi_vic_to_videomode(34);
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
	//HDMITX_EnableHDCP(FALSE);

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
	    setHDMITX_SyncEmbeddedByVIC(34,InstanceData.bInputVideoSignalType);
	}
    #endif

    printk("level = %d, ,bInputColorMode=%x,bOutputColorMode=%x,bHDMIMode=%x\n",(int)level,(int)bInputColorMode,(int)bOutputColorMode ,(int)bHDMIMode) ;
	HDMITX_EnableVideoOutput(level,bInputColorMode,bOutputColorMode ,bHDMIMode);

    if( bHDMIMode )
    {
        #ifdef OUTPUT_3D_MODE
        ConfigfHdmiVendorSpecificInfoFrame(OUTPUT_3D_MODE);
        #endif
        ConfigAVIInfoFrame(34, pixelrep, aspec, Colorimetry, bOutputColorMode);
    }
	else
	{
		HDMITX_EnableAVIInfoFrame(FALSE ,NULL);
        HDMITX_EnableVSInfoFrame(FALSE,NULL);
	}

    setHDMITX_AVMute(FALSE);
	HDMITX_PowerOn();
    DumpHDMITXReg() ;
	
	return HDMI_ERROR_SUCESS;
}


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
		hdmitx_SetInputMode(F_MODE_YUV422,T_MODE_CCIR656);
		it66121_config_video();
		return 0;
	}
	printk(KERN_ERR "IT66121: Device not found!\n");

	return 1;
}

static int it66121_s_std_output(struct v4l2_subdev *sd, v4l2_std_id norm)
{
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


static int it66121_g_chip_ident(struct v4l2_subdev *sd, struct v4l2_dbg_chip_ident *chip)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	return v4l2_chip_ident_i2c_client(client, chip, V4L2_IDENT_IT66121, 0);
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
	.read           = it66121_readreg,
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
	.command = it66121_ioctl,
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


