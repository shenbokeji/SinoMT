/*
 * This source file is FPGA header file for the 'lena encode' on DM368 platform

 *****************************************************************************
 * Copyright(C) 2015, SinoMartin Corp.
 *----------------------------------------------------------------------------
 * filename	: FPGA.h
 * function	: no
 * author	version		date		note
 * feller	1.0		20150729	create         
 *----------------------------------------------------------------------------
*/
#ifndef __FPGA_H
#define __FPGA_H

#include "../common.h"


/*****************global variable  declaration *****************/
extern UInt32 g_uiRegBaseAddr ;




//define the EMIF FPGA register read write 
//#define  EMIF_FPGA_WRITE_UINT16(offsetaddr,value)   (*(volatile UInt32 *)(((UInt32)g_uiRegBaseAddr) + offsetaddr) = (UInt16)(value))
//#define  EMIF_FPGA_READ_UINT16(offsetaddr,value)    ((value) = *(volatile UInt32 *)(((UInt32)g_uiRegBaseAddr) + offsetaddr))




/*****************structure type definition*****************/
typedef struct FPGARegStruct {
    UInt uiAddr;//EMIF 32bit address bus
    UInt uiValue;//EMIF 16bit data bus,but kernel use 32bit datatype,so we use LSB16
} tFPGAReg;

typedef struct FPGACfgStruct{
    tFPGAReg FPGAReg[11];//FPGA register number
    Int iValidLen;
} tFPGACfg;





typedef struct fpga_data{
	unsigned int tb_size;
	unsigned int source_addr;
	unsigned int dst_addr;
	unsigned int byte_size;
	unsigned char device_busy;
}tfpgadata;
/*****************macro definition*****************/
//ce0:0x200 0000  
//ce1:0x400 0000  
#define EMIF_BASE_ADDRESS (0X2000000)
#define EMIF_OFFSET_ADDRESS (0X0002000)

#define LINUX_PAGE_SIZE (0X1000)
#define DEVICE_FPGA "/dev/fpga"


#define EMIF_FPGA_START_ADDR (0x600)
#define EMIF_FPGA_END_ADDR (0x800)
#define EMIF_FPGA_RAM_ADDR (0xC0)
#define EMIF_FPGA_SAMP_EN_ADDR (0x610)
#define FPGA_ADDR_INVALID(a) ( ( a > EMIF_FPGA_END_ADDR ) || ( a < EMIF_FPGA_START_ADDR ) )

#define SEND_PHY_ADDR (0x86000000)
#define RECEIVE_PHY_ADDR (0x87000000)
#define MEM_FILENAME "/dev/mem"
#define SEND_VIDEO_FILE_384 "/video384.264"
#define SEND_VIDEO_FILE_720P "/video720p.264"
#define SEND_VIDEO_FILE "/sendvideo.264"
#define RECE_VIDEO_FILE "/recevideo.264"
#define FPGA_RAM_DATA "/data.dat"
#define TRANS_ODD2EVEN(i) ( ( i + 1 ) & 0XFFFFFFFE ) 
#define FPGA_DMA_RECV	(0U)
#define FPGA_DMA_SEND	(1U)

/*****************extern function declaration*****************/
extern void InitFPGAReg( void );
extern void InitFPGA( const Int iAirorGround );
extern int GetFpgaReg( const unsigned int uiAddr, unsigned short * const pusRdValue );
extern int SetFpgaReg( const unsigned int uiAddr, const unsigned short usValue );


#endif /* __FPGA_H */

