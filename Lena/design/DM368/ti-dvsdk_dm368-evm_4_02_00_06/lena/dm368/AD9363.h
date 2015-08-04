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
    Int iValue;		//SPI bus,data
} tAD9363Reg;

typedef struct AD9363CfgStruct{
    tAD9363Reg AD9363Reg[2][10];//AD9363 register number
    Int iValidLen[2];
} tAD9363Cfg;


/*****************extern function declaration*****************/
extern void InitAD9363( const Int iAirorGround );
#endif /* __AD9363_H */

