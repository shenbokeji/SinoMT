#ifndef __IT66121_H__
#define __IT66121_H__

//#include "gpio_i2c.h"

//#define IT66121_write(chip_addr,reg_addr,value)	gpio_i2c_write(chip_addr,reg_addr,value) 
//#define IT66121_read(chip_addr,reg_addr) gpio_i2c_read(chip_addr,reg_addr) 

#define SUCCESS		1
#define FAIL		0
/*
 *	I2C Address.
 */
 
#define _80MHz 80000000
#define HDMI_TX_I2C_SLAVE_ADDR 0x9A
#define CEC_I2C_SLAVE_ADDR 0x9C


#define BYTE unsigned char
#define WORD unsigned short 
#define ULONG unsigned int
#define BOOL int 

#define TRUE 		1
#define FALSE 	0


/* 输往屏的数据格式 */
#define OUT_P888            0


///////////////////////////////////////////////////////////////////////
// Output Mode Type
///////////////////////////////////////////////////////////////////////

#define RES_ASPEC_4x3 0
#define RES_ASPEC_16x9 1
#define F_MODE_REPT_NO 0
#define F_MODE_REPT_TWICE 1
#define F_MODE_REPT_QUATRO 3
#define F_MODE_CSC_ITU601 0
#define F_MODE_CSC_ITU709 1







/////////////////////////////////////////////////////////////////////
// data structure
/////////////////////////////////////////////////////////////////////
typedef struct _HDMITXDEV_STRUCT {

	BYTE I2C_DEV ;
	BYTE I2C_ADDR ;

	/////////////////////////////////////////////////
	// Interrupt Type
	/////////////////////////////////////////////////
	BYTE bIntType ; // = 0 ;
	/////////////////////////////////////////////////
	// Video Property
	/////////////////////////////////////////////////
	BYTE bInputVideoSignalType ; // for Sync Embedded,CCIR656,InputDDR
	/////////////////////////////////////////////////
	// Audio Property
	/////////////////////////////////////////////////
	BYTE bOutputAudioMode ; // = 0 ;
	BYTE bAudioChannelSwap ; // = 0 ;
    BYTE bAudioChannelEnable ;
    BYTE bAudFs ;
    unsigned long TMDSClock ;
    unsigned long RCLK ;
	BYTE bAuthenticated:1 ;
	BYTE bHDMIMode: 1;
	BYTE bIntPOL:1 ; // 0 = Low Active
	BYTE bHPD:1 ;
	// 2009/11/11 added by jj_tseng@ite.com.tw
    BYTE bSPDIF_OUT;
    BYTE TxEMEMStatus:1 ;
    //~jau-chih.tseng@ite.com.tw 2009/11/11
} HDMITXDEV ;


#define REG_TX_VENDOR_ID0   0x00
#define REG_TX_VENDOR_ID1   0x01
#define REG_TX_DEVICE_ID0   0x02
#define REG_TX_DEVICE_ID1   0x03

    #define O_TX_DEVID 0
    #define M_TX_DEVID 0xF
    #define O_TX_REVID 4
    #define M_TX_REVID 0xF

#define REG_TX_SW_RST       0x04
    #define B_TX_ENTEST    (1<<7)
    #define B_TX_REF_RST_HDMITX (1<<5)
    #define B_TX_AREF_RST (1<<4)
    #define B_HDMITX_VID_RST (1<<3)
    #define B_HDMITX_AUD_RST (1<<2)
    #define B_TX_HDMI_RST (1<<1)
    #define B_TX_HDCP_RST_HDMITX (1<<0)

#define REG_TX_INT_CTRL 0x05
    #define B_TX_INTPOL_ACTL 0
    #define B_TX_INTPOL_ACTH (1<<7)
    #define B_TX_INT_PUSHPULL 0
    #define B_TX_INT_OPENDRAIN (1<<6)

#define REG_TX_INT_STAT1    0x06
    #define B_TX_INT_AUD_OVERFLOW  (1<<7)
    #define B_TX_INT_ROMACQ_NOACK  (1<<6)
    #define B_TX_INT_RDDC_NOACK    (1<<5)
    #define B_TX_INT_DDCFIFO_ERR   (1<<4)
    #define B_TX_INT_ROMACQ_BUS_HANG   (1<<3)
    #define B_TX_INT_DDC_BUS_HANG  (1<<2)
    #define B_TX_INT_RX_SENSE  (1<<1)
    #define B_TX_INT_HPD_PLUG  (1<<0)

#define REG_TX_INT_STAT2    0x07
    #define B_TX_INT_HDCP_SYNC_DET_FAIL  (1<<7)
    #define B_TX_INT_VID_UNSTABLE  (1<<6)
    #define B_TX_INT_PKTACP    (1<<5)
    #define B_TX_INT_PKTNULL  (1<<4)
    #define B_TX_INT_PKTGENERAL   (1<<3)
    #define B_TX_INT_KSVLIST_CHK   (1<<2)
    #define B_TX_INT_AUTH_DONE (1<<1)
    #define B_TX_INT_AUTH_FAIL (1<<0)

#define REG_TX_INT_STAT3    0x08
    #define B_TX_INT_AUD_CTS   (1<<6)
    #define B_TX_INT_VSYNC     (1<<5)
    #define B_TX_INT_VIDSTABLE (1<<4)
    #define B_TX_INT_PKTMPG    (1<<3)
    #define B_TX_INT_PKTSPD    (1<<2)
    #define B_TX_INT_PKTAUD    (1<<1)
    #define B_TX_INT_PKTAVI    (1<<0)


#define REG_TX_INT_MASK1    0x09
    #define B_TX_AUDIO_OVFLW_MASK (1<<7)
    #define B_TX_DDC_NOACK_MASK (1<<5)
    #define B_TX_DDC_FIFO_ERR_MASK (1<<4)
    #define B_TX_DDC_BUS_HANG_MASK (1<<2)
    #define B_TX_RXSEN_MASK (1<<1)
    #define B_TX_HPD_MASK (1<<0)

#define REG_TX_INT_MASK2    0x0A
    #define B_TX_PKT_AVI_MASK (1<<7)
    #define B_TX_PKT_VID_UNSTABLE_MASK (1<<6)
    #define B_TX_PKT_ACP_MASK (1<<5)
    #define B_TX_PKT_NULL_MASK (1<<4)
    #define B_TX_PKT_GEN_MASK (1<<3)
    #define B_TX_KSVLISTCHK_MASK (1<<2)
    #define B_TX_AUTH_DONE_MASK (1<<1)
    #define B_TX_AUTH_FAIL_MASK (1<<0)

#define REG_TX_INT_MASK3    0x0B
    #define B_TX_HDCP_SYNC_DET_FAIL_MASK (1<<6)
    #define B_TX_AUDCTS_MASK (1<<5)
    #define B_TX_VSYNC_MASK (1<<4)
    #define B_TX_VIDSTABLE_MASK (1<<3)
    #define B_TX_PKT_MPG_MASK (1<<2)
    #define B_TX_PKT_SPD_MASK (1<<1)
    #define B_TX_PKT_AUD_MASK (1<<0)


#define REG_TX_INT_CLR0      0x0C
    #define B_TX_CLR_PKTACP    (1<<7)
    #define B_TX_CLR_PKTNULL   (1<<6)
    #define B_TX_CLR_PKTGENERAL    (1<<5)
    #define B_TX_CLR_KSVLISTCHK    (1<<4)
    #define B_TX_CLR_AUTH_DONE  (1<<3)
    #define B_TX_CLR_AUTH_FAIL  (1<<2)
    #define B_TX_CLR_RXSENSE   (1<<1)
    #define B_TX_CLR_HPD       (1<<0)

#define REG_TX_INT_CLR1       0x0D
    #define B_TX_CLR_VSYNC (1<<7)
    #define B_TX_CLR_VIDSTABLE (1<<6)
    #define B_TX_CLR_PKTMPG    (1<<5)
    #define B_TX_CLR_PKTSPD    (1<<4)
    #define B_TX_CLR_PKTAUD    (1<<3)
    #define B_TX_CLR_PKTAVI    (1<<2)
    #define B_TX_CLR_HDCP_SYNC_DET_FAIL  (1<<1)
    #define B_TX_CLR_VID_UNSTABLE        (1<<0)

#define REG_TX_SYS_STATUS     0x0E
    // readonly
    #define B_TX_INT_ACTIVE    (1<<7)
    #define B_TX_HPDETECT      (1<<6)
    #define B_TX_RXSENDETECT   (1<<5)
    #define B_TXVIDSTABLE   (1<<4)
    // read/write
    #define O_TX_CTSINTSTEP    2
    #define M_TX_CTSINTSTEP    (3<<2)
    #define B_TX_CLR_AUD_CTS     (1<<1)
    #define B_TX_INTACTDONE    (1<<0)

#define REG_TX_BANK_CTRL        0x0F
    #define B_TX_BANK0 0
    #define B_TX_BANK1 1


// DDC

#define REG_TX_DDC_MASTER_CTRL   0x10
    #define B_TX_MASTERROM (1<<1)
    #define B_TX_MASTERDDC (0<<1)
    #define B_TX_MASTERHOST    (1<<0)
    #define B_TX_MASTERHDCP    (0<<0)

#define REG_TX_DDC_HEADER  0x11
#define REG_TX_DDC_REQOFF  0x12
#define REG_TX_DDC_REQCOUNT    0x13
#define REG_TX_DDC_EDIDSEG 0x14
#define REG_TX_DDC_CMD 0x15
    #define CMD_DDC_SEQ_BURSTREAD 0
    #define CMD_LINK_CHKREAD  2
    #define CMD_EDID_READ   3
    #define CMD_FIFO_CLR    9
    #define CMD_GEN_SCLCLK  0xA
    #define CMD_DDC_ABORT   0xF

#define REG_TX_DDC_STATUS  0x16
    #define B_TX_DDC_DONE  (1<<7)
    #define B_TX_DDC_ACT   (1<<6)
    #define B_TX_DDC_NOACK (1<<5)
    #define B_TX_DDC_WAITBUS   (1<<4)
    #define B_TX_DDC_ARBILOSE  (1<<3)
    #define B_TX_DDC_ERROR     (B_TX_DDC_NOACK|B_TX_DDC_WAITBUS|B_TX_DDC_ARBILOSE)
    #define B_TX_DDC_FIFOFULL  (1<<2)
    #define B_TX_DDC_FIFOEMPTY (1<<1)

#define REG_TX_DDC_READFIFO    0x17
#define REG_TX_ROM_STARTADDR   0x18
#define REG_TX_HDCP_HEADER 0x19
#define REG_TX_ROM_HEADER  0x1A
#define REG_TX_BUSHOLD_T   0x1B
#define REG_TX_ROM_STAT    0x1C
    #define B_TX_ROM_DONE  (1<<7)
    #define B_TX_ROM_ACTIVE	(1<<6)
    #define B_TX_ROM_NOACK	(1<<5)
    #define B_TX_ROM_WAITBUS	(1<<4)
    #define B_TX_ROM_ARBILOSE	(1<<3)
    #define B_TX_ROM_BUSHANG	(1<<2)

// HDCP
#define REG_TX_AN_GENERATE 0x1F
    #define B_TX_START_CIPHER_GEN  1
    #define B_TX_STOP_CIPHER_GEN   0

#define REG_TX_CLK_CTRL0 0x58
    #define O_TX_OSCLK_SEL 5
    #define M_TX_OSCLK_SEL 3
    #define B_TX_AUTO_OVER_SAMPLING_CLOCK (1<<4)
    #define O_TX_EXT_MCLK_SEL  2
    #define M_TX_EXT_MCLK_SEL  (3<<O_TX_EXT_MCLK_SEL)
    #define B_TX_EXT_128FS (0<<O_TX_EXT_MCLK_SEL)
    #define B_TX_EXT_256FS (1<<O_TX_EXT_MCLK_SEL)
    #define B_TX_EXT_512FS (2<<O_TX_EXT_MCLK_SEL)
    #define B_TX_EXT_1024FS (3<<O_TX_EXT_MCLK_SEL)

#define REG_TX_SHA_SEL       0x50
#define REG_TX_SHA_RD_BYTE1  0x51
#define REG_TX_SHA_RD_BYTE2  0x52
#define REG_TX_SHA_RD_BYTE3  0x53
#define REG_TX_SHA_RD_BYTE4  0x54
#define REG_TX_AKSV_RD_BYTE5 0x55


#define REG_TX_CLK_CTRL1 0x59
    #define B_TX_EN_TXCLK_COUNT    (1<<5)
    #define B_TX_VDO_LATCH_EDGE    (1<<3)

#define REG_TX_CLK_STATUS1 0x5E
#define REG_TX_CLK_STATUS2 0x5F
    #define B_TX_IP_LOCK (1<<7)
    #define B_TX_XP_LOCK (1<<6)
    #define B_TX_OSF_LOCK (1<<5)

#define REG_TX_AUD_COUNT 0x60
#define REG_TX_AFE_DRV_CTRL 0x61

    #define B_TX_AFE_DRV_PWD    (1<<5)
    #define B_TX_AFE_DRV_RST    (1<<4)

#define REG_TX_PLL_CTRL 0x6A


// Input Data Format Register
#define REG_TX_INPUT_MODE  0x70
    #define O_TX_INCLKDLY	0
    #define M_TX_INCLKDLY	3
    #define B_TX_INDDR	    (1<<2)
    #define B_TX_SYNCEMB	(1<<3)
    #define B_TX_2X656CLK	(1<<4)
	#define B_TX_PCLKDIV2  (1<<5)
    #define M_TX_INCOLMOD	(3<<6)
    #define B_TX_IN_RGB    0
    #define B_TX_IN_YUV422 (1<<6)
    #define B_TX_IN_YUV444 (2<<6)

#define REG_TX_TXFIFO_RST  0x71
    #define B_TX_ENAVMUTERST	1
    #define B_TXFFRST	(1<<1)

#define REG_TX_CSC_CTRL    0x72
    #define B_HDMITX_CSC_BYPASS    0
    #define B_HDMITX_CSC_RGB2YUV   2
    #define B_HDMITX_CSC_YUV2RGB   3
    #define M_TX_CSC_SEL       3
    #define B_TX_EN_DITHER      (1<<7)
    #define B_TX_EN_UDFILTER    (1<<6)
    #define B_TX_DNFREE_GO      (1<<5)

#define SIZEOF_CSCMTX 21
#define SIZEOF_CSCGAIN 6
#define SIZEOF_CSCOFFSET 3


#define REG_TX_CSC_YOFF 0x73
#define REG_TX_CSC_COFF 0x74
#define REG_TX_CSC_RGBOFF 0x75


#define REG_TX_CSC_MTX11_L 0x76
#define REG_TX_CSC_MTX11_H 0x77
#define REG_TX_CSC_MTX12_L 0x78
#define REG_TX_CSC_MTX12_H 0x79
#define REG_TX_CSC_MTX13_L 0x7A
#define REG_TX_CSC_MTX13_H 0x7B
#define REG_TX_CSC_MTX21_L 0x7C
#define REG_TX_CSC_MTX21_H 0x7D
#define REG_TX_CSC_MTX22_L 0x7E
#define REG_TX_CSC_MTX22_H 0x7F
#define REG_TX_CSC_MTX23_L 0x80
#define REG_TX_CSC_MTX23_H 0x81
#define REG_TX_CSC_MTX31_L 0x82
#define REG_TX_CSC_MTX31_H 0x83
#define REG_TX_CSC_MTX32_L 0x84
#define REG_TX_CSC_MTX32_H 0x85
#define REG_TX_CSC_MTX33_L 0x86
#define REG_TX_CSC_MTX33_H 0x87

#define REG_TX_CSC_GAIN1V_L 0x88
#define REG_TX_CSC_GAIN1V_H 0x89
#define REG_TX_CSC_GAIN2V_L 0x8A
#define REG_TX_CSC_GAIN2V_H 0x8B
#define REG_TX_CSC_GAIN3V_L 0x8C
#define REG_TX_CSC_GAIN3V_H 0x8D

#define REG_TX_HVPol 0x90
#define REG_TX_HfPixel 0x91
#define REG_TX_HSSL 0x95
#define REG_TX_HSEL 0x96
#define REG_TX_HSH 0x97
#define REG_TX_VSS1 0xA0
#define REG_TX_VSE1 0xA1
#define REG_TX_VSS2 0xA2
#define REG_TX_VSE2 0xA3

// HDMI General Control Registers

#define REG_TX_HDMI_MODE   0xC0
    #define B_TX_HDMI_MODE 1
    #define B_TX_DVI_MODE  0
#define REG_TX_AV_MUTE 0xC1
#define REG_TX_GCP     0xC1
    #define B_TX_CLR_AVMUTE    0
    #define B_TX_SET_AVMUTE    1
    #define B_TX_SETAVMUTE        (1<<0)
    #define B_TX_BLUE_SCR_MUTE   (1<<1)
    #define B_TX_NODEF_PHASE    (1<<2)
    #define B_TX_PHASE_RESYNC   (1<<3)

    #define O_TX_COLOR_DEPTH     4
    #define M_TX_COLOR_DEPTH     7
    #define B_TX_COLOR_DEPTH_MASK (M_TX_COLOR_DEPTH<<O_TX_COLOR_DEPTH)
    #define B_TX_CD_NODEF  0
    #define B_TX_CD_24     (4<<4)
    #define B_TX_CD_30     (5<<4)
    #define B_TX_CD_36     (6<<4)
    #define B_TX_CD_48     (7<<4)
#define REG_TX_PKT_GENERAL_CTRL    0xC6

#define REG_TX_OESS_CYCLE  0xC3


///////////////////////////////////////////////////////////////////////
// Video Data Type
///////////////////////////////////////////////////////////////////////

#define F_MODE_RGB444  0
#define F_MODE_YUV422 1
#define F_MODE_YUV444 2
#define F_MODE_CLRMOD_MASK 3


#define F_MODE_INTERLACE  1

#define F_VIDMODE_ITU709  (1<<4)
#define F_VIDMODE_ITU601  0

#define F_VIDMODE_0_255   0
#define F_VIDMODE_16_235  (1<<5)

#define F_VIDMODE_EN_UDFILT (1<<6)
#define F_VIDMODE_EN_DITHER (1<<7)

#define T_MODE_CCIR656 (1<<0)
#define T_MODE_SYNCEMB (1<<1)
#define T_MODE_INDDR   (1<<2)
#define T_MODE_PCLKDIV2 (1<<3)
#define T_MODE_DEGEN (1<<4)
#define T_MODE_SYNCGEN (1<<5)

/////////////////////////////////////////////////////////////////////
// Packet and Info Frame definition and datastructure.
/////////////////////////////////////////////////////////////////////


#define VENDORSPEC_INFOFRAME_TYPE 0x81
#define AVI_INFOFRAME_TYPE  0x82
#define SPD_INFOFRAME_TYPE 0x83
#define AUDIO_INFOFRAME_TYPE 0x84
#define MPEG_INFOFRAME_TYPE 0x85

#define VENDORSPEC_INFOFRAME_VER 0x01
#define AVI_INFOFRAME_VER  0x02
#define SPD_INFOFRAME_VER 0x01
#define AUDIO_INFOFRAME_VER 0x01
#define MPEG_INFOFRAME_VER 0x01

#define VENDORSPEC_INFOFRAME_LEN 5
#define AVI_INFOFRAME_LEN 13
#define SPD_INFOFRAME_LEN 25
#define AUDIO_INFOFRAME_LEN 10
#define MPEG_INFOFRAME_LEN 10

#define ACP_PKT_LEN 9
#define ISRC1_PKT_LEN 16
#define ISRC2_PKT_LEN 16




//////////////////////////////////////////
// Bank 1
//////////////////////////////////////////

#define REGPktAudCTS0 0x30  // 7:0
#define REGPktAudCTS1 0x31  // 15:8
#define REGPktAudCTS2 0x32  // 19:16
#define REGPktAudN0 0x33    // 7:0
#define REGPktAudN1 0x34    // 15:8
#define REGPktAudN2 0x35    // 19:16
#define REGPktAudCTSCnt0 0x35   // 3:0
#define REGPktAudCTSCnt1 0x36   // 11:4
#define REGPktAudCTSCnt2 0x37   // 19:12


#define REG_TX_AUDCHST_MODE    0x91 // 191 REG_TX_AUD_CHSTD[2:0] 6:4
                                 //     REG_TX_AUD_CHSTC 3
                                 //     REG_TX_AUD_NLPCM 2
                                 //     REG_TX_AUD_MONO 0
#define REG_TX_AUDCHST_CAT     0x92 // 192 REG_TX_AUD_CHSTCAT 7:0
#define REG_TX_AUDCHST_SRCNUM  0x93 // 193 REG_TX_AUD_CHSTSRC 3:0
#define REG_TX_AUD0CHST_CHTNUM 0x94 // 194 REG_TX_AUD0_CHSTCHR 7:4
                                 //     REG_TX_AUD0_CHSTCHL 3:0
#define REG_TX_AUD1CHST_CHTNUM 0x95 // 195 REG_TX_AUD1_CHSTCHR 7:4
                                 //     REG_TX_AUD1_CHSTCHL 3:0
#define REG_TX_AUD2CHST_CHTNUM 0x96 // 196 REG_TX_AUD2_CHSTCHR 7:4
                                 //     REG_TX_AUD2_CHSTCHL 3:0
#define REG_TX_AUD3CHST_CHTNUM 0x97 // 197 REG_TX_AUD3_CHSTCHR 7:4
                                 //     REG_TX_AUD3_CHSTCHL 3:0
#define REG_TX_AUDCHST_CA_FS   0x98 // 198 REG_TX_AUD_CHSTCA 5:4
                                 //     REG_TX_AUD_CHSTFS 3:0
#define REG_TX_AUDCHST_OFS_WL  0x99 // 199 REG_TX_AUD_CHSTOFS 7:4
                                 //     REG_TX_AUD_CHSTWL 3:0

#define REG_TX_PKT_SINGLE_CTRL 0xC5
    #define B_TX_SINGLE_PKT    1
    #define B_TX_BURST_PKT
    #define B_TX_SW_CTS    (1<<1)

#define REG_TX_NULL_CTRL 0xC9
#define REG_TX_ACP_CTRL 0xCA
#define REG_TX_ISRC1_CTRL 0xCB
#define REG_TX_ISRC2_CTRL 0xCC
#define REG_TX_AVI_INFOFRM_CTRL 0xCD
#define REG_TX_AUD_INFOFRM_CTRL 0xCE
#define REG_TX_SPD_INFOFRM_CTRL 0xCF
#define REG_TX_MPG_INFOFRM_CTRL 0xD0
    #define B_TX_ENABLE_PKT    1
    #define B_TX_REPEAT_PKT    (1<<1)

#define REG_TX_3D_INFO_CTRL 0xD2

//////////////////////////////////////////
// COMMON PACKET for NULL,ISRC1,ISRC2,SPD
//////////////////////////////////////////

#define	REG_TX_PKT_HB00 0x38
#define	REG_TX_PKT_HB01 0x39
#define	REG_TX_PKT_HB02 0x3A

#define	REG_TX_PKT_PB00 0x3B
#define	REG_TX_PKT_PB01 0x3C
#define	REG_TX_PKT_PB02 0x3D
#define	REG_TX_PKT_PB03 0x3E
#define	REG_TX_PKT_PB04 0x3F
#define	REG_TX_PKT_PB05 0x40
#define	REG_TX_PKT_PB06 0x41
#define	REG_TX_PKT_PB07 0x42
#define	REG_TX_PKT_PB08 0x43
#define	REG_TX_PKT_PB09 0x44
#define	REG_TX_PKT_PB10 0x45
#define	REG_TX_PKT_PB11 0x46
#define	REG_TX_PKT_PB12 0x47
#define	REG_TX_PKT_PB13 0x48
#define	REG_TX_PKT_PB14 0x49
#define	REG_TX_PKT_PB15 0x4A
#define	REG_TX_PKT_PB16 0x4B
#define	REG_TX_PKT_PB17 0x4C
#define	REG_TX_PKT_PB18 0x4D
#define	REG_TX_PKT_PB19 0x4E
#define	REG_TX_PKT_PB20 0x4F
#define	REG_TX_PKT_PB21 0x50
#define	REG_TX_PKT_PB22 0x51
#define	REG_TX_PKT_PB23 0x52
#define	REG_TX_PKT_PB24 0x53
#define	REG_TX_PKT_PB25 0x54
#define	REG_TX_PKT_PB26 0x55
#define	REG_TX_PKT_PB27 0x56

#define REG_TX_AVIINFO_DB1 0x58
#define REG_TX_AVIINFO_DB2 0x59
#define REG_TX_AVIINFO_DB3 0x5A
#define REG_TX_AVIINFO_DB4 0x5B
#define REG_TX_AVIINFO_DB5 0x5C
#define REG_TX_AVIINFO_DB6 0x5E
#define REG_TX_AVIINFO_DB7 0x5F
#define REG_TX_AVIINFO_DB8 0x60
#define REG_TX_AVIINFO_DB9 0x61
#define REG_TX_AVIINFO_DB10 0x62
#define REG_TX_AVIINFO_DB11 0x63
#define REG_TX_AVIINFO_DB12 0x64
#define REG_TX_AVIINFO_DB13 0x65
#define REG_TX_AVIINFO_SUM 0x5D

#define REG_TX_PKT_AUDINFO_CC 0x68 // [2:0]
#define REG_TX_PKT_AUDINFO_SF 0x69 // [4:2]
#define REG_TX_PKT_AUDINFO_CA 0x6B // [7:0]

#define REG_TX_PKT_AUDINFO_DM_LSV 0x6C // [7][6:3]
#define REG_TX_PKT_AUDINFO_SUM 0x6D // [7:0]

// Source Product Description Info Frame
#define REG_TX_PKT_SPDINFO_SUM 0x70
#define REG_TX_PKT_SPDINFO_PB1 0x71
#define REG_TX_PKT_SPDINFO_PB2 0x72
#define REG_TX_PKT_SPDINFO_PB3 0x73
#define REG_TX_PKT_SPDINFO_PB4 0x74
#define REG_TX_PKT_SPDINFO_PB5 0x75
#define REG_TX_PKT_SPDINFO_PB6 0x76
#define REG_TX_PKT_SPDINFO_PB7 0x77
#define REG_TX_PKT_SPDINFO_PB8 0x78
#define REG_TX_PKT_SPDINFO_PB9 0x79
#define REG_TX_PKT_SPDINFO_PB10 0x7A
#define REG_TX_PKT_SPDINFO_PB11 0x7B
#define REG_TX_PKT_SPDINFO_PB12 0x7C
#define REG_TX_PKT_SPDINFO_PB13 0x7D
#define REG_TX_PKT_SPDINFO_PB14 0x7E
#define REG_TX_PKT_SPDINFO_PB15 0x7F
#define REG_TX_PKT_SPDINFO_PB16 0x80
#define REG_TX_PKT_SPDINFO_PB17 0x81
#define REG_TX_PKT_SPDINFO_PB18 0x82
#define REG_TX_PKT_SPDINFO_PB19 0x83
#define REG_TX_PKT_SPDINFO_PB20 0x84
#define REG_TX_PKT_SPDINFO_PB21 0x85
#define REG_TX_PKT_SPDINFO_PB22 0x86
#define REG_TX_PKT_SPDINFO_PB23 0x87
#define REG_TX_PKT_SPDINFO_PB24 0x88
#define REG_TX_PKT_SPDINFO_PB25 0x89

#define REG_TX_PKT_MPGINFO_FMT 0x8A
#define B_TX_MPG_FR 1
#define B_TX_MPG_MF_I  (1<<1)
#define B_TX_MPG_MF_B  (2<<1)
#define B_TX_MPG_MF_P  (3<<1)
#define B_TX_MPG_MF_MASK (3<<1)
#define REG_TX_PKG_MPGINFO_DB0 0x8B
#define REG_TX_PKG_MPGINFO_DB1 0x8C
#define REG_TX_PKG_MPGINFO_DB2 0x8D
#define REG_TX_PKG_MPGINFO_DB3 0x8E
#define REG_TX_PKG_MPGINFO_SUM 0x8F

#define Frame_Pcaking 0
#define Top_and_Botton 6
#define Side_by_Side 8


////////////////////////////////////////////////////
// Function Prototype
////////////////////////////////////////////////////
#define hdmitx_ENABLE_NULL_PKT()         { it66121_write(REG_TX_NULL_CTRL,B_TX_ENABLE_PKT|B_TX_REPEAT_PKT); }
#define hdmitx_ENABLE_ACP_PKT()          { it66121_write(REG_TX_ACP_CTRL,B_TX_ENABLE_PKT|B_TX_REPEAT_PKT); }
#define hdmitx_ENABLE_ISRC1_PKT()        { it66121_write(REG_TX_ISRC1_CTRL,B_TX_ENABLE_PKT|B_TX_REPEAT_PKT); }
#define hdmitx_ENABLE_ISRC2_PKT()        { it66121_write(REG_TX_ISRC2_CTRL,B_TX_ENABLE_PKT|B_TX_REPEAT_PKT); }
#define hdmitx_ENABLE_AVI_INFOFRM_PKT()  { it66121_write(REG_TX_AVI_INFOFRM_CTRL,B_TX_ENABLE_PKT|B_TX_REPEAT_PKT); }
#define hdmitx_ENABLE_AUD_INFOFRM_PKT()  { it66121_write(REG_TX_AUD_INFOFRM_CTRL,B_TX_ENABLE_PKT|B_TX_REPEAT_PKT); }
#define hdmitx_ENABLE_SPD_INFOFRM_PKT()  { it66121_write(REG_TX_SPD_INFOFRM_CTRL,B_TX_ENABLE_PKT|B_TX_REPEAT_PKT); }
#define hdmitx_ENABLE_MPG_INFOFRM_PKT()  { it66121_write(REG_TX_MPG_INFOFRM_CTRL,B_TX_ENABLE_PKT|B_TX_REPEAT_PKT); }
#define hdmitx_ENABLE_GeneralPurpose_PKT() { it66121_write(REG_TX_NULL_CTRL,B_TX_ENABLE_PKT|B_TX_REPEAT_PKT); }
#define hdmitx_DISABLE_VSDB_PKT()        { it66121_write(REG_TX_3D_INFO_CTRL,0); }
#define hdmitx_DISABLE_NULL_PKT()        { it66121_write(REG_TX_NULL_CTRL,0); }
#define hdmitx_DISABLE_ACP_PKT()         { it66121_write(REG_TX_ACP_CTRL,0); }
#define hdmitx_DISABLE_ISRC1_PKT()       { it66121_write(REG_TX_ISRC1_CTRL,0); }
#define hdmitx_DISABLE_ISRC2_PKT()       { it66121_write(REG_TX_ISRC2_CTRL,0); }
#define hdmitx_DISABLE_AVI_INFOFRM_PKT() { it66121_write(REG_TX_AVI_INFOFRM_CTRL,0); }
#define hdmitx_DISABLE_AUD_INFOFRM_PKT() { it66121_write(REG_TX_AUD_INFOFRM_CTRL,0); }
#define hdmitx_DISABLE_SPD_INFOFRM_PKT() { it66121_write(REG_TX_SPD_INFOFRM_CTRL,0); }
#define hdmitx_DISABLE_MPG_INFOFRM_PKT() { it66121_write(REG_TX_MPG_INFOFRM_CTRL,0); }
#define hdmitx_DISABLE_GeneralPurpose_PKT() { it66121_write(REG_TX_NULL_CTRL,0); }


//////////////////////////////////////////////////////////////////////
// Authentication status
//////////////////////////////////////////////////////////////////////

// #define TIMEOUT_WAIT_AUTH MS(2000)



#ifndef INV_INPUT_PCLK
#define PCLKINV 0
#else
#define PCLKINV B_TX_VDO_LATCH_EDGE
#endif

#ifndef INV_INPUT_ACLK
    #define InvAudCLK 0
#else
    #define InvAudCLK B_TX_AUDFMT_FALL_EDGE_SAMPLE_WS
#endif

#define INIT_CLK_HIGH
// #define INIT_CLK_LOW

#define HDMITX_MAX_DEV_COUNT 1

HDMITXDEV hdmiTxDev[HDMITX_MAX_DEV_COUNT] ;


typedef struct structRegSetEntry {
    BYTE offset ;
    BYTE invAndMask ;
    BYTE OrMask ;
} RegSetEntry;


#define Switch_HDMITX_Bank(x)   HDMITX_SetI2C_Byte(0x0f,1, (x)&1)





//////////////////////////////////////////////////////////////////////
// External Interface
//////////////////////////////////////////////////////////////////////

typedef enum {
    PCLK_LOW = 0 ,
    PCLK_MEDIUM,
    PCLK_HIGH
} VIDEOPCLKLEVEL ;



#ifndef INPUT_SIGNAL_TYPE
#define INPUT_SIGNAL_TYPE 0 // 24 bit sync seperate
#endif




////////////////////////////////////////////////////////////////////////////////
// Internal Data Type
////////////////////////////////////////////////////////////////////////////////

typedef enum tagHDMI_Video_Type {
    HDMI_Unkown = 0 ,
    HDMI_640x480p60 = 1 ,
    HDMI_480p60,
    HDMI_480p60_16x9,
    HDMI_720p60,
    HDMI_1080i60,
    HDMI_480i60,
    HDMI_480i60_16x9,
    HDMI_1080p60 = 16,
    HDMI_576p50,
    HDMI_576p50_16x9,
    HDMI_720p50,
    HDMI_1080i50,
    HDMI_576i50,
    HDMI_576i50_16x9,
    HDMI_1080p50 = 31,
    HDMI_1080p24,
    HDMI_1080p25,
    HDMI_1080p30,
    HDMI_720p30 = 61,
} HDMI_Video_Type ;

typedef enum tagHDMI_Aspec {
    HDMI_4x3 ,
    HDMI_16x9
} HDMI_Aspec;

typedef enum tagHDMI_OutputColorMode {
    HDMI_RGB444,
    HDMI_YUV444,
    HDMI_YUV422
} HDMI_OutputColorMode ;

typedef enum tagHDMI_Colorimetry {
    HDMI_ITU601,
    HDMI_ITU709
} HDMI_Colorimetry ;

struct VideoTiming {
    u32 VideoPixelClock ;
    BYTE VIC ;
    BYTE pixelrep ;
	BYTE outputVideoMode ;
} ;

// HDMI Video Parameters
struct hdmi_video {
	unsigned int vic;					// Video information code
	unsigned int color_input;			// Input video color mode
	unsigned int color_output;			// Output video color mode
	unsigned int sink_hdmi;				// Output signal is DVI or HDMI
	unsigned int format_3d;				// Output 3D mode
};


// HDMI Error Code
enum hdmi_error_code {
	HDMI_ERROR_SUCESS = 0,
	HDMI_ERROR_FALSE,
	HDMI_ERROR_I2C,
	HDMI_ERROR_EDID,
};

// HDMI Video Data Color Mode
enum hdmi_video_color_mode {
	HDMI_COLOR_RGB = 0,
	HDMI_COLOR_YCbCr444,
	HDMI_COLOR_YCbCr422,
};

// HDMI video information code according CEA-861-E
enum hdmi_video_infomation_code
{
	HDMI_640x480p_60HZ = 1,
	HDMI_720x480p_60HZ_4_3,
	HDMI_720x480p_60HZ_16_9,
	HDMI_1280x720p_60HZ,
	HDMI_1920x1080i_60HZ,		//5
	HDMI_720x480i_60HZ_4_3,
	HDMI_720x480i_60HZ_16_9,
	HDMI_720x240p_60HZ_4_3,
	HDMI_720x240p_60HZ_16_9,
	HDMI_2880x480i_60HZ_4_3,	//10
	HDMI_2880x480i_60HZ_16_9,
	HDMI_2880x240p_60HZ_4_3,
	HDMI_2880x240p_60HZ_16_9,
	HDMI_1440x480p_60HZ_4_3,
	HDMI_1440x480p_60HZ_16_9,	//15
	HDMI_1920x1080p_60HZ,
	HDMI_720x576p_50HZ_4_3,
	HDMI_720x576p_50HZ_16_9,
	HDMI_1280x720p_50HZ,
	HDMI_1920x1080i_50HZ,		//20
	HDMI_720x576i_50HZ_4_3,
	HDMI_720x576i_50HZ_16_9,
	HDMI_720x288p_50HZ_4_3,
	HDMI_720x288p_50HZ_16_9,
	HDMI_2880x576i_50HZ_4_3,	//25
	HDMI_2880x576i_50HZ_16_9,
	HDMI_2880x288p_50HZ_4_3,
	HDMI_2880x288p_50HZ_16_9,
	HDMI_1440x576p_50HZ_4_3,
	HDMI_1440x576p_50HZ_16_9,	//30
	HDMI_1920x1080p_50HZ,
	HDMI_1920x1080p_24HZ,
	HDMI_1920x1080p_25HZ,
	HDMI_1920x1080p_30HZ,
	HDMI_2880x480p_60HZ_4_3,	//35
	HDMI_2880x480p_60HZ_16_9,
	HDMI_2880x576p_50HZ_4_3,
	HDMI_2880x576p_50HZ_16_9,
	HDMI_1920x1080i_50HZ_1250,	// V Line 1250 total
	HDMI_1920x1080i_100HZ,		//40
	HDMI_1280x720p_100HZ,
	HDMI_720x576p_100HZ_4_3,
	HDMI_720x576p_100HZ_16_9,
	HDMI_720x576i_100HZ_4_3,
	HDMI_720x576i_100HZ_16_9,	//45
	HDMI_1920x1080i_120HZ,
	HDMI_1280x720p_120HZ,
	HDMI_720x480p_120HZ_4_3,
	HDMI_720x480p_120HZ_16_9,	
	HDMI_720x480i_120HZ_4_3,	//50
	HDMI_720x480i_120HZ_16_9,
	HDMI_720x576p_200HZ_4_3,
	HDMI_720x576p_200HZ_16_9,
	HDMI_720x576i_200HZ_4_3,
	HDMI_720x576i_200HZ_16_9,	//55
	HDMI_720x480p_240HZ_4_3,
	HDMI_720x480p_240HZ_16_9,	
	HDMI_720x480i_240HZ_4_3,
	HDMI_720x480i_240HZ_16_9,
	HDMI_1280x720p_24HZ,		//60
	HDMI_1280x720p_25HZ,
	HDMI_1280x720p_30HZ,
	HDMI_1920x1080p_120HZ,
	HDMI_1920x1080p_100HZ,
};



///////////////////////////////////////////////////////////////////////////
// Using for interface.
///////////////////////////////////////////////////////////////////////////

#define PROG 1
#define INTERLACE 0
#define Vneg 0
#define Hneg 0
#define Vpos 1
#define Hpos 1


typedef struct {
    WORD    H_ActiveStart;
    WORD    H_ActiveEnd;
    WORD    H_SyncStart;
    WORD    H_SyncEnd;
    WORD    V_ActiveStart;
    WORD    V_ActiveEnd;
    WORD    V_SyncStart;
    WORD    V_SyncEnd;
    WORD    V2_ActiveStart;
    WORD    V2_ActiveEnd;
    WORD    HTotal;
    WORD    VTotal;
} CEAVTiming;
typedef struct {
    BYTE VIC ;
    BYTE PixelRep ;
    WORD    HActive;
    WORD    VActive;
    WORD    HTotal;
    WORD    VTotal;
    ULONG    PCLK;
    BYTE    xCnt;
    WORD    HFrontPorch;
    WORD    HSyncWidth;
    WORD    HBackPorch;
    BYTE    VFrontPorch;
    BYTE    VSyncWidth;
    BYTE    VBackPorch;
    BYTE    ScanMode:1;
    BYTE    VPolarity:1;
    BYTE    HPolarity:1;
} HDMI_VTiming;


typedef union _AVI_InfoFrame
{

    struct {
        BYTE Type;
        BYTE Ver;
        BYTE Len;

        BYTE checksum ;

        BYTE Scan:2;
        BYTE BarInfo:2;
        BYTE ActiveFmtInfoPresent:1;
        BYTE ColorMode:2;
        BYTE FU1:1;

        BYTE ActiveFormatAspectRatio:4;
        BYTE PictureAspectRatio:2;
        BYTE Colorimetry:2;

        BYTE Scaling:2;
        BYTE FU2:6;

        BYTE VIC:7;
        BYTE FU3:1;

        BYTE PixelRepetition:4;
        BYTE FU4:4;

        short Ln_End_Top;
        short Ln_Start_Bottom;
        short Pix_End_Left;
        short Pix_Start_Right;
    } info;

    struct {
        BYTE AVI_HB[3];
        BYTE checksum ;
        BYTE AVI_DB[AVI_INFOFRAME_LEN];
    } pktbyte;
} AVI_InfoFrame;


typedef union _VendorSpecific_InfoFrame
{
    struct {
        BYTE Type ;
        BYTE Ver ;
        BYTE Len ;

        BYTE CheckSum;

        BYTE IEEE_0;//PB1
        BYTE IEEE_1;//PB2
        BYTE IEEE_2;//PB3

        BYTE Rsvd:5 ;//PB4
        BYTE HDMI_Video_Format:3 ;

        BYTE Reserved_PB5:4 ;//PB5
        BYTE _3D_Structure:4 ;

        BYTE Reserved_PB6:4 ;//PB6
        BYTE _3D_Ext_Data:4 ;
    } info ;
    struct {
        BYTE VS_HB[3] ;
        BYTE CheckSum;
        BYTE VS_DB[28] ;
    } pktbyte ;
} VendorSpecific_InfoFrame ;


typedef struct device_operation
{
	unsigned char addr;
	unsigned char reg;
	unsigned char value;
}device_operation;

/*
 * ioctl cmd
 */
#define IT66121_IOC_MAGIC		'v'

#define IT66121_GET_ALL_REG			_IO(IT66121_IOC_MAGIC, 1)
#define IT66121_GET_SINGLE_REG		_IOWR(IT66121_IOC_MAGIC, 2, struct device_operation)
#define IT66121_SET_SINGLE_REG		_IOWR(IT66121_IOC_MAGIC, 3, struct device_operation)


struct it66121 {
	struct i2c_client *client;
	struct hdmi *hdmi;

	int irq;
	int io_irq_pin;
	int io_pwr_pin;
	int io_rst_pin;
	
	unsigned long tmdsclk;
	struct work_struct	irq_work;
	struct delayed_work delay_work;
	struct workqueue_struct *workqueue;
	
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend	early_suspend;
#endif
	int enable;
};


void hdmitx_LoadRegSetting(RegSetEntry table[]);
BOOL HDMITX_EnableAVIInfoFrame(BYTE bEnable,BYTE *pAVIInfoFrame);


#endif	/* __IT66121_H__ */
