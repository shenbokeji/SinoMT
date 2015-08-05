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
    Int iValue;//EMIF 16bit data bus
} tFPGAReg;

typedef struct FPGACfgStruct{
    tFPGAReg FPGAReg[2][10];//FPGA register number
    Int iValidLen[2];
} tFPGACfg;

/*****************macro definition*****************/


/*****************extern function declaration*****************/
extern void InitFPGA( const Int iAirorGround );
#endif /* __FPGA_H */

