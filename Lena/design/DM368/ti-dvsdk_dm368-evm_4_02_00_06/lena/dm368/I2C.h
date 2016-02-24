/*
 * This source file is AD9363 header file for the 'lena encode' on DM368 platform

 *****************************************************************************
 * Copyright(C) 2015, SinoMartin Corp.
 *----------------------------------------------------------------------------
 * filename	: I2C.h
 * function	: no
 * author	version		date		note
 * feller	1.0		20151229	create         
 *----------------------------------------------------------------------------
*/
#ifndef __I2C_DEVICE_H
#define __I2C_DEVICE_H
#include "../common.h"
/*****************structure type definition*****************/
typedef struct I2CRegStruct {
    unsigned int uiAddr;	//I2C  bus, address  
    unsigned int uiValue;	//I2C bus,data
} tI2CReg;

/*****************macro definition*****************/
#define IT66121_REG_START_ADDR (0X0)
#define IT66121_REG_END_ADDR (0X2FF)
#define IT66121_ADDR_INVALID(a) ( ( a > IT66121_REG_END_ADDR ) || ( a < IT66121_REG_START_ADDR ) ) 
#define DEVICE_IT66121 "/dev/it66121_reg"
#define DEVICE_ADV7611 "/dev/adv7611_reg"
/*****************extern function declaration*****************/
extern Int GetIT66121Reg( const unsigned int uiAddr, unsigned char * const pusRdValue );
extern Int SetIT66121Reg( const unsigned int uiAddr, const unsigned char ucValue );
extern Int GetADV7611Reg( const unsigned int uiAddr, unsigned char * const pusRdValue );
extern Int SetADV7611Reg( const unsigned int uiAddr, const unsigned char ucValue );
#endif /* __I2C_DEVICE_H */

