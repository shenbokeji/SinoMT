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
#define  EMIF_FPGA_WRITE_UINT16(offsetaddr,value)   (*(volatile UInt32 *)(((UInt32)g_uiRegBaseAddr) + offsetaddr) = (UInt16)(value))
#define  EMIF_FPGA_READ_UINT16(offsetaddr,value)    ((value) = *(volatile UInt32 *)(((UInt32)g_uiRegBaseAddr) + offsetaddr))




/*****************structure type definition*****************/
typedef struct FPGARegStruct {
    UInt uiAddr;//EMIF 32bit address bus
    UInt16 usValue;//EMIF 16bit data bus
} tFPGAReg;

typedef struct FPGACfgStruct{
    tFPGAReg FPGAReg[2][10];//FPGA register number
    Int iValidLen[2];
} tFPGACfg;

/*****************macro definition*****************/
//ce0:0x200 0000  
//ce1:0x400 0000  
#define EMIF_BASE_ADDRESS (0X2000000)
#define LINUX_PAGE_SIZE (0X1000)
#define DEV_MEM_MMAP "dev/mem/"

#define EMIF_FPGA_START_ADDR (0x600)
#define EMIF_FPGA_END_ADDR (0x700)
#define EMIF_FPGA_RAM_ADDR (0xC0)
#define EMIF_FPGA_SAMP_EN_ADDR (0x610)
#define FPGA_ADDR_VALID(a) ( ( a > EMIF_FPGA_END_ADDR ) || ( a < EMIF_FPGA_START_ADDR ) )

/*****************extern function declaration*****************/
extern void InitFPGA( const Int iAirorGround );
extern UInt16 GetFpgaReg( const UInt32 * const uiAddr );
extern Int SetFpgaReg( const UInt32 * const uiAddr, const UInt16 sValue );
#endif /* __FPGA_H */

