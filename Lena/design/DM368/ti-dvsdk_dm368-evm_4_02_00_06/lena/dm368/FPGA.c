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
tFPGACfg g_tFPGACfg[2];

/*----------------------------------------------------------------------------
 * name			: GetFpgaReg
 * function 	: Get the fpga reg value 
 * input 		:
 * author	version		date		note
 * feller	1.0		20150805
 * feller   1.0     20151119        I am at home with phyliss
 *----------------------------------------------------------------------------
*/
int GetFpgaReg( const unsigned int uiAddr, unsigned short * const pusRdValue )
{
	int iRdByteNum;
	int fid;
	tFPGAReg tFPGARegTmp;
	int iReturn = LENA_OK;


	tFPGARegTmp.uiAddr = uiAddr;
	
	fid = open( DEVICE_FPGA, O_RDONLY, 0 );
	if( fid < 0 )
	{
		perror( "ERROR:open failed "DEVICE_FPGA"!\n" );
		return LENA_FALSE;
	}
	iRdByteNum = read( fid, &tFPGARegTmp, BYTE_EIGHT ); //8 byte for addr and value
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
   	close( fid );
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
int SetFpgaReg( const unsigned int uiAddr, const unsigned short usValue )
{
	Int iWrByteNum;
	int fid;
	tFPGAReg tFPGARegTmp;
	int iReturn = LENA_OK;
	tFPGARegTmp.uiAddr = uiAddr;
	tFPGARegTmp.uiValue = (unsigned int)usValue;
	
	fid = open( DEVICE_FPGA, O_RDWR, 0 );
	if( fid < 0 )
	{
		perror( "ERROR:open failed "DEVICE_FPGA"!\n" );
		return LENA_FALSE;
	}
	iWrByteNum = write( fid, &tFPGARegTmp, BYTE_EIGHT );
 	if( BYTE_EIGHT != iWrByteNum )
	{
		iReturn = LENA_FALSE;
		printf( "ERROR:write byte is not equal!\n" );		
	} 
	else
	{
		;
	}
	close( fid );
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
    // ground FPGA register init value
    g_tFPGACfg[GROUND_STATION].FPGAReg[0].uiAddr = 0X620;
    g_tFPGACfg[GROUND_STATION].FPGAReg[0].uiValue = 0X0;
    g_tFPGACfg[GROUND_STATION].FPGAReg[1].uiAddr = 0X622;
    g_tFPGACfg[GROUND_STATION].FPGAReg[1].uiValue = 0X0;
    g_tFPGACfg[GROUND_STATION].FPGAReg[2].uiAddr = 0X624;
    g_tFPGACfg[GROUND_STATION].FPGAReg[2].uiValue = 0X0;
    g_tFPGACfg[GROUND_STATION].FPGAReg[3].uiAddr = 0X626;
    g_tFPGACfg[GROUND_STATION].FPGAReg[3].uiValue = 0X0;
    g_tFPGACfg[GROUND_STATION].FPGAReg[4].uiAddr = 0X628;
    g_tFPGACfg[GROUND_STATION].FPGAReg[4].uiValue = 0X1;
    g_tFPGACfg[GROUND_STATION].FPGAReg[5].uiAddr = 0X62A;
    g_tFPGACfg[GROUND_STATION].FPGAReg[5].uiValue = 0X1;
    g_tFPGACfg[GROUND_STATION].iValidLen  = 6;


    // air FPGA register init value
    g_tFPGACfg[AIR_STATION].FPGAReg[0].uiAddr = 0X620;
    g_tFPGACfg[AIR_STATION].FPGAReg[0].uiValue = 0X1;
    g_tFPGACfg[AIR_STATION].FPGAReg[1].uiAddr = 0X622;
    g_tFPGACfg[AIR_STATION].FPGAReg[1].uiValue = 0X0;
    g_tFPGACfg[AIR_STATION].FPGAReg[2].uiAddr = 0X624;
    g_tFPGACfg[AIR_STATION].FPGAReg[2].uiValue = 0X1;
    g_tFPGACfg[AIR_STATION].FPGAReg[3].uiAddr = 0X626;
    g_tFPGACfg[AIR_STATION].FPGAReg[3].uiValue = 0X0;
    g_tFPGACfg[AIR_STATION].FPGAReg[4].uiAddr = 0X628;
    g_tFPGACfg[AIR_STATION].FPGAReg[4].uiValue = 0X0;
    g_tFPGACfg[AIR_STATION].FPGAReg[5].uiAddr = 0X62A;
    g_tFPGACfg[AIR_STATION].FPGAReg[5].uiValue = 0X0;
    g_tFPGACfg[AIR_STATION].FPGAReg[6].uiAddr = 0X612;
    g_tFPGACfg[AIR_STATION].FPGAReg[6].uiValue = 0X200;
    g_tFPGACfg[AIR_STATION].FPGAReg[7].uiAddr = 0X614;
    g_tFPGACfg[AIR_STATION].FPGAReg[7].uiValue = 0X200;
    g_tFPGACfg[AIR_STATION].FPGAReg[8].uiAddr = 0X616;
    g_tFPGACfg[AIR_STATION].FPGAReg[8].uiValue = 0X200;
    g_tFPGACfg[AIR_STATION].FPGAReg[9].uiAddr = 0X618;
    g_tFPGACfg[AIR_STATION].FPGAReg[9].uiValue = 0X200;
    g_tFPGACfg[AIR_STATION].FPGAReg[10].uiAddr = 0X61A;
    g_tFPGACfg[AIR_STATION].FPGAReg[10].uiValue = 0X0;


    g_tFPGACfg[AIR_STATION].iValidLen  = 11;
	
    return ;
}



 /*--------------------------------------------------------------------------
 * name			: InitFPGA
 * function 		: iAirorGround: air and ground station flag,0:ground 1:air,others:invalid
 * author	version		date		note
 * feller	1.0		20150729	
 *----------------------------------------------------------------------------
*/
void InitFPGA( const int iAirorGround )
{
    int iLoop;
    unsigned int uiTmp;
    unsigned short usTmp;
    Int iLen;
	
    iLen =  g_tFPGACfg[iAirorGround].iValidLen;
    printf( "iAirorGround=%d\n", iAirorGround );
    printf( "iLen=%d\n", iLen );
    for( iLoop = 0; iLoop < iLen; iLoop++ )
    {
        uiTmp = g_tFPGACfg[iAirorGround].FPGAReg[iLoop].uiAddr;
	usTmp = (UInt16)g_tFPGACfg[iAirorGround].FPGAReg[iLoop].uiValue;	
	printf( "%#X,%#X\n", uiTmp, usTmp );
        //configure FPGA by EMIF
	SetFpgaReg( uiTmp, usTmp );
    }
		
    return ;
}
 
