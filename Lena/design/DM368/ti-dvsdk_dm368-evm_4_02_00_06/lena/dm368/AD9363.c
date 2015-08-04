/*
 * This source file is AD9363 file for the 'lena encode' on DM368 platform

 *****************************************************************************
 * Copyright(C) 2015, SinoMartin Corp.
 *----------------------------------------------------------------------------
 * filename	: AD9363.c
 * function	: initialize /write /read interface
 * author	version		date		note
 * feller	1.0		20150728	create         
 *----------------------------------------------------------------------------
*/


#include "AD9363.h"

/*****************Global variable for AD9363 configure Table*****************/
tAD9363Cfg g_tAD9363Cfg;


 /*----------------------------------------------------------------------------
 * name		: InitAD9363Reg
 * function	: initialize AD9363 Reg addr and value
 * input 		: no
 * author	version		date		note
 * feller	1.0		20150729      
 *----------------------------------------------------------------------------
*/
void InitAD9363Reg( void )
{
    g_tAD9363Cfg.AD9363Reg[GROUND_STATION][0].uiAddr = 0;
    g_tAD9363Cfg.AD9363Reg[GROUND_STATION][1].iValue = 1;
    g_tAD9363Cfg.iValidLen[GROUND_STATION]  = 5;







    g_tAD9363Cfg.AD9363Reg[AIR_STATION][0].uiAddr = 0;
    g_tAD9363Cfg.AD9363Reg[AIR_STATION][1].iValue = 1;
    g_tAD9363Cfg.iValidLen[AIR_STATION]  = 8;


	
    return ;
}



 /*--------------------------------------------------------------------------
 * name		: InitAD9363
 * fucntion 	: initialize AD9363 interface
 * input 		: iAirorGround: air and ground station flag,0:ground 1:air,others:invalid
 * author	version		date		note
 * feller	1.0		20150728	
 *----------------------------------------------------------------------------
*/
void InitAD9363( const Int iAirorGround )
{
    Int iLoop;
    UInt uiTmp;
    Int iTmp;
    Int iLen;	
   
    iLen =  g_tAD9363Cfg.iValidLen[iAirorGround];	
    InitAD9363Reg();
    for( iLoop = 0; iLoop <  iLen; iLoop++ )
    {
        uiTmp = g_tAD9363Cfg.AD9363Reg[iAirorGround][iLoop].uiAddr;
	 iTmp = g_tAD9363Cfg.AD9363Reg[iAirorGround][iLoop].iValue;	

        //configure AD9363 by SPI,through FPGA
	 
    }
		
    return ;
}






