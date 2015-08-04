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

