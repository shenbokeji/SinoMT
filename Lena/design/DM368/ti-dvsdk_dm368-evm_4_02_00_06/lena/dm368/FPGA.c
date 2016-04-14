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
void InitFPGAReg( const int iAirGround )
{
	if( 0 == iAirGround )
	{
    	// ground FPGA register init value
    	g_tFPGACfg[GROUND_STATION].FPGAReg[0].uiAddr = 0X61E;
    	g_tFPGACfg[GROUND_STATION].FPGAReg[0].uiValue = 0X23;
    	g_tFPGACfg[GROUND_STATION].FPGAReg[1].uiAddr = 0X602;
    	g_tFPGACfg[GROUND_STATION].FPGAReg[1].uiValue = 0X6;
    	g_tFPGACfg[GROUND_STATION].FPGAReg[1].uiAddr = 0X674;
    	g_tFPGACfg[GROUND_STATION].FPGAReg[1].uiValue = 0X1;		
    	g_tFPGACfg[GROUND_STATION].iValidLen  = 2;
			
	}
	else
	{
    	// air FPGA register init value
    	g_tFPGACfg[AIR_STATION].FPGAReg[0].uiAddr = 0X620;
    	g_tFPGACfg[AIR_STATION].FPGAReg[0].uiValue = 0X1;
    	g_tFPGACfg[AIR_STATION].FPGAReg[1].uiAddr = 0X622;
    	g_tFPGACfg[AIR_STATION].FPGAReg[1].uiValue = 0X1;
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
		//0:normal signal 1:single tone,2:constant;3:loopaback
    	g_tFPGACfg[AIR_STATION].FPGAReg[10].uiValue = 0X0;

    	g_tFPGACfg[AIR_STATION].iValidLen  = 11;
	}
    return ;
}



 /*--------------------------------------------------------------------------
 * name			: InitFPGA
 * function 	: iAirorGround: air and ground station flag,0:ground 1:air,others:invalid
 * author	version		date		note
 * feller	1.0		20150729	
 *----------------------------------------------------------------------------
*/
void InitFPGA( const int iAirorGround )
{
    int iLoop;
    unsigned int uiTmp;
    unsigned short usTmp;
    int iLen;
	int iReturn;
	
    iLen =  g_tFPGACfg[iAirorGround].iValidLen;
    printf( "iAirorGround=%d\n", iAirorGround );
    printf( "iLen=%d\n", iLen );
    for( iLoop = 0; iLoop < iLen; iLoop++ )
    {
        uiTmp = g_tFPGACfg[iAirorGround].FPGAReg[iLoop].uiAddr;
		usTmp = (UInt16)g_tFPGACfg[iAirorGround].FPGAReg[iLoop].uiValue;	
		printf( "%#X,%#X\n", uiTmp, usTmp );
        //configure FPGA by EMIF
		iReturn = SetFpgaReg( uiTmp, usTmp );
    }
	//iReturn = GetFpgaReg();
    return ;
}
/*--------------------------------------------------------------------------
 * name			: ReFreshFPGAAirInterface
 * function 	: iAirorGround: air and ground station flag,0:ground 1:air,others:invalid
 * author	version		date		note
 * feller	1.0		20160402
 *----------------------------------------------------------------------------
*/
int ReFreshAirInterface( const int iAirorGround )
{
	int iReturn = LENA_OK;
	//unsigned int uiAddr = 0x61C;
	unsigned int uiUEIDAddr = 0x6C8;
	unsigned short usRdValue;
	int icount = 0; 
	//sync	
	if( 0 == iAirorGround ) //for ground ,we should sync for air interface
	{
		iReturn = GetFpgaReg( uiUEIDAddr, &usRdValue );
		if( 1 == usRdValue )
		{
			iReturn = LENA_OK;
			printf( "Ground ReFreshAirInterface ok\n" );
		}
		else
		{
			iReturn = GetFpgaReg( 0X688, &usRdValue );
			printf( "CRC =%#X\n", usRdValue );	
			
			ResetFPGA();
			iReturn = SetFpgaReg( 0X61E, 0X23 );//refresh sync
			iReturn = SetFpgaReg( 0X674, 0X1 );//refresh sync
			usleep(20000);
			iReturn = SetFpgaReg( 0X61C, 1 );//refresh sync			
			printf( "Ground ReFreshAirInterface not ok\n" );
			sleep(FPGA_SYN_FREASH_DELAY);
			iReturn = LENA_FALSE;
		}
	}
	else
	{	
		printf( "Air ReFreshAirInterface ok\n" );//by now ,do nothing 
	}	
	return iReturn;
}
