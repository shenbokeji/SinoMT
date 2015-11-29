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
/*****************Global variable for FPGA configure Table*****************/
tFPGACfg g_tFPGACfg;
//UInt32 g_uiRegBaseAddr = 0;//EMIF FPGA register base address
/*----------------------------------------------------------------------------
 * name			: GetFpgaReg
 * function 	: Get the fpga reg value 
 * input 		:
 * author	version		date		note
 * feller	1.0		20150805
 * feller   1.0     20151119        I am at home with phyliss
 *----------------------------------------------------------------------------
*/
Int GetFpgaReg( const UInt32 uiAddr, UInt16 * const pusRdValue )
{
	int iRdByteNum;
	FILE *fid;
	tFPGAReg tFPGARegTmp;
	int iReturn = LENA_OK;


	tFPGARegTmp.uiAddr = uiAddr;
	
	fid = (FILE *)open( DEVICE_FPGA, O_RDONLY, 0 );
	if( (int)fid < 0 )
	{
		printf( "ERROR:open failed "DEVICE_FPGA"!\n" );
		return LENA_FALSE;
	}
	iRdByteNum = read( (int)fid, &tFPGARegTmp, BYTE_EIGHT ); //8 byte for addr and value
	if( BYTE_EIGHT != iRdByteNum )
	{
		iReturn = LENA_FALSE;
		*pusRdValue = ERROR_STRING;
		printf( "ERROR:read byte is not equal!\n" );
	}
	else 
	{
		*pusRdValue = (UInt16)tFPGARegTmp.uiValue;
	}
   	close( (int)fid );
   	fid = NULL;

		
    return iReturn;
}
 /*----------------------------------------------------------------------------
 * name			: SetFpgaReg
 * function 		: set the fpga reg 
 * input 			:
 * author	version		date		note
 * feller	1.0		20150805
 *----------------------------------------------------------------------------
*/
Int SetFpgaReg( const UInt32 uiAddr, const UInt16 usValue )
{
	Int iWrByteNum;
	FILE *fid;
	tFPGAReg tFPGARegTmp;
	int iReturn = LENA_OK;
	tFPGARegTmp.uiAddr = uiAddr;
	tFPGARegTmp.uiValue = (unsigned int)usValue;
	
	fid = (FILE *)open( DEVICE_FPGA, O_RDWR, 0 );
	if( (int)fid < 0 )
	{
		printf( "ERROR:open failed "DEVICE_FPGA"!\n" );
		return LENA_FALSE;
	}
	iWrByteNum = write( (int)fid, &tFPGARegTmp, BYTE_EIGHT );
 	if( BYTE_EIGHT != iWrByteNum )
	{
		iReturn = LENA_FALSE;
		printf( "ERROR:write byte is not equal!\n" );		
	} 
	else
	{
		;
	}
	close( (int)fid );
   	fid = NULL;
	return iReturn;

}

 /*----------------------------------------------------------------------------
 * name			: InitFPGAReg
 * function 	: initialize FPGA Reg addr and value
 * author	version		date		note
 * feller	1.0		20150729
 *----------------------------------------------------------------------------
*/
void InitFPGAReg( void )
{
    g_tFPGACfg.FPGAReg[GROUND_STATION][0].uiAddr = 0;
    g_tFPGACfg.FPGAReg[GROUND_STATION][1].uiValue = 1;
    g_tFPGACfg.iValidLen[GROUND_STATION]  = 0;







    g_tFPGACfg.FPGAReg[AIR_STATION][0].uiAddr = 0;
    g_tFPGACfg.FPGAReg[AIR_STATION][1].uiValue = 1;
    g_tFPGACfg.iValidLen[AIR_STATION]  = 0;


	
    return ;
}



 /*--------------------------------------------------------------------------
 * name			: InitFPGA
 * function 		: iAirorGround: air and ground station flag,0:ground 1:air,others:invalid
 * author	version		date		note
 * feller	1.0		20150729	
 *----------------------------------------------------------------------------
*/
void InitFPGA( const Int iAirorGround )
{
    Int iLoop;
    UInt32 uiTmp;
    UInt16 usTmp;
    Int iLen;
	
    iLen =  g_tFPGACfg.iValidLen[iAirorGround];
    for( iLoop = 0; iLoop < iLen; iLoop++ )
    {
        uiTmp = g_tFPGACfg.FPGAReg[iAirorGround][iLoop].uiAddr;
	 	usTmp = (UInt16)g_tFPGACfg.FPGAReg[iAirorGround][iLoop].uiValue;	

        //configure FPGA by EMIF
	 	SetFpgaReg( uiTmp, usTmp );
    }
		
    return ;
}
 
