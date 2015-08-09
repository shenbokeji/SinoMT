/*
 * This source file is FPGA file for the 'lena encode' on DM368 platform

 *****************************************************************************
 * Copyright(C) 2015, SinoMartin Corp.
 *----------------------------------------------------------------------------
 * filename	: FPGA.c
 * function	: initialize /write /read interface
 * author	version		date		note
 * feller	1.0		20150729	create         
 *----------------------------------------------------------------------------
*/

#include "FPGA.h"

 /*--------------------------------------------------------------------------
 * name			: wrfpga
 * function 		: debug function for user
 * input 			:uiAddr:11bit offset address in FPGA, usValue: value 
 * author	version		date		note
 * feller	1.0		20150809	
 *----------------------------------------------------------------------------
*/
void wrfpga( const UInt32 uiAddr, const UInt16 usValue )
{
    // Address left shift 1bit ,please check the EMIF 16bit mode .it is connected with the  UB/LB pin
    SetFpgaReg( ( UInt32 * )( uiAddr << 1 ), usValue );
    //unsigned int be used directly for complie warning
    printf( "FPGAReg:%#X,data:%#X\n" , ( unsigned int )uiAddr, usValue );
    return;
}

 /*--------------------------------------------------------------------------
 * name			: wrfpga
 * function 		: debug function for user
 * input 			:uiAddr:11bit offset address in FPGA, uiLength: register number ,every 16bit 
 * author	version		date		note
 * feller	1.0		20150809	
 *----------------------------------------------------------------------------
*/
void rdfpga(  const UInt32 uiAddr, UInt32 uiLength )
{
    UInt16 usValue = 0;
    Int32 uiLoop = 0;
   	
    if ( 0 == uiLength )
    {
    	uiLength = 1;
    }

    printf("\n");
    for ( uiLoop = 0; uiLoop < uiLength; uiLoop++)
    {
        // Address left shift 1bit ,please check the EMIF 16bit mode .it is connected with the  UB/LB pin
        usValue = GetFpgaReg( ( UInt32 * )( uiAddr << 1 ) ); 
        printf( "FPGAReg:%#X,data:%#X\n" , ( unsigned int )uiAddr, usValue );		
    }
    
    return ;
}