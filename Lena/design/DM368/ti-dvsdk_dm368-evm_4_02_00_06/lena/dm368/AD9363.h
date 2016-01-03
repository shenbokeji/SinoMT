/*
 * This source file is AD9363 header file for the 'lena encode' on DM368 platform

 *****************************************************************************
 * Copyright(C) 2015, SinoMartin Corp.
 *----------------------------------------------------------------------------
 * filename	: AD9363.h
 * function	: no
 * author	version		date		note
 * feller	1.0		20150728	create         
 *----------------------------------------------------------------------------
*/
#ifndef __AD9363_H
#define __AD9363_H
#include "../common.h"
/*****************structure type definition*****************/
typedef struct AD9363RegStruct {
    UInt uiAddr;	//SPI bus, address  
    UInt uiValue;		//SPI bus,data
} tAD9363Reg;

typedef struct AD9363CfgStruct{
    tAD9363Reg AD9363Reg[2][10];//AD9363 register number
    Int iValidLen[2];
} tAD9363Cfg;

/*****************macro definition*****************/

#define AD9363_REG_END_ADDR (0x3FE)
#define AD9363_ADDR_INVALID(a) ( a > AD9363_REG_END_ADDR ) 
#define AD9363_CHAN_INVALID(a) ( ( a > 2 ) || ( a < 0 ) ) 
#define DEVICE_AD9363 "/dev/ad9363"
#define AD9363_REGVALUE_G_FILE "/usr/src/ad9363_regvalue_g.txt"
#define AD9363_REGVALUE_A_FILE "/usr/src/ad9363_regvalue_a.txt"
/*****************extern function declaration*****************/
extern void InitAD9363( const int iAirorGround );
extern int GetAD9363Reg( const unsigned int uiAddr, unsigned char * const pusRdValue );
extern int SetAD9363Reg( const unsigned int uiAddr, const unsigned char ucValue );
#endif /* __AD9363_H */

