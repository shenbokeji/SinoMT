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
  * name	: GetAD9363Reg
  * function	: Get the AD9363 reg value 
  * input	:
  * author	 version	 date		 note
  * feller	 1.0	 20150805
  * feller	 1.0	 20151119		 I am at home with phyliss
  *----------------------------------------------------------------------------
 */
 int GetAD9363Reg( const unsigned int uiAddr, unsigned char * const pucRdValue )
 {
	 int iRdByteNum;
	 int fid;
	 int iReturn = LENA_OK;
	 tAD9363Reg tAD9363RegTmp;
 
 
	 tAD9363RegTmp.uiAddr = uiAddr;
	 
	 fid = open( DEVICE_AD9363, O_RDONLY, 0 );

	 if( fid < 0 )
	 {
		 perror( "ERROR:open failed "DEVICE_AD9363"!\n" );
		 return LENA_FALSE;
	 }
	 
	 iRdByteNum = read( fid, &tAD9363RegTmp, BYTE_EIGHT ); //8 byte for addr and value
	 if( BYTE_EIGHT != iRdByteNum )
	 {
		 printf( "ERROR:Read Byte is not %d "DEVICE_AD9363"! \n", BYTE_EIGHT );
		 iReturn = LENA_FALSE;
		*pucRdValue = (unsigned char)ERROR_STRING;
	 }
	else
	{
		*pucRdValue = (unsigned char)tAD9363RegTmp.uiValue;
	}
	 close( fid );
	 fid = NULL;
	 		 
	 return iReturn;
 }
  /*----------------------------------------------------------------------------
  * name		: SetAD9363Reg
  * function		: set the AD9363 reg 
  * input		:
  * author	 version	 date		 note
  * feller	 1.0	 20150805
  *----------------------------------------------------------------------------
 */
 int SetAD9363Reg( const unsigned int uiAddr, const unsigned char ucValue )
 {
	 Int iWrByteNum;
	 int fid;
	 tAD9363Reg tAD9363RegTmp;
	 int iReturn = LENA_OK;

	 tAD9363RegTmp.uiAddr = uiAddr;
	 tAD9363RegTmp.uiValue = (unsigned int)ucValue;
	 
	 fid = open( DEVICE_AD9363, O_RDWR, 0 );
	 if( fid < 0 )
	 {
		 perror( "ERROR:open failed "DEVICE_AD9363"!\n" );
		 return LENA_FALSE;
	 }
	 iWrByteNum = write( fid, &tAD9363RegTmp, BYTE_EIGHT );
	 if( BYTE_EIGHT != iWrByteNum )
	 {
		 printf( "ERROR:Write Byte is not %d "DEVICE_AD9363"!\n", BYTE_EIGHT );
		 iReturn =  LENA_FALSE;
	 } 
	 close( fid );
	 fid = NULL;
	 return iReturn;
 }


 /*--------------------------------------------------------------------------
 * name		: InitAD9363
 * fucntion 	: initialize AD9363 register
 * input 	: iAirorGround: air and ground station flag,0:ground 1:air,others:invalid
 * author	version		date		note
 * feller	1.0		20150728	
 *----------------------------------------------------------------------------
*/
void InitAD9363( const int iAirorGround )
{
	FILE *fid;
	char *cfile = NULL;
	unsigned int iReturn;
	unsigned int str[3] = {0};


	if( iAirorGround > 1 )
	{
		printf( "command format: init9363 para \n 0:ground;1:air\n");
		return;
	}

	cfile = ( iAirorGround == 1 ) ? AD9363_REGVALUE_A_FILE : AD9363_REGVALUE_G_FILE;


	fid = fopen( cfile, "r" );
	if( NULL == fid )
	{
		perror( cfile );
		return;
	}
	printf( "open: %s \n", cfile );
	while( EOF != fscanf( fid, "%x,%x,%d", &str[0], &str[1], &str[2] ) )
	{
		//printf( "%#05X,%#04X,%d\n", str[0], str[1], str[2] );
		iReturn = SetAD9363Reg( (unsigned int)str[0], (unsigned char)str[1] );
		usleep( str[2] );
	}
	printf( "init AD9363 %s over\n", iAirorGround  ? AIR_VERSION : GROUND_VERSION );
	fclose(fid);
		
    return ;
}
 /*--------------------------------------------------------------------------
 * name		: initclock
 * fucntion 	: initialize AD9363 clock ,so FPGA can work
 * input 	: none
 * author	version		date		note
 * feller	1.0		20151222	
 *----------------------------------------------------------------------------
*/
int initclock( void )
{
	int iReturn;
	iReturn = SetAD9363Reg( 0x000, 0x00 );
	iReturn |= SetAD9363Reg( 0x000, 0x81 );
	iReturn |= SetAD9363Reg( 0x000, 0x00 );
	iReturn |= SetAD9363Reg( 0x3DF, 0x01 );
	iReturn |= SetAD9363Reg( 0x2A6, 0x0E );
	iReturn |= SetAD9363Reg( 0x2A8, 0x0E );
	iReturn |= SetAD9363Reg( 0x2AB, 0x07 );
	iReturn |= SetAD9363Reg( 0x2AC, 0xFF );
	iReturn |= SetAD9363Reg( 0x009, 0x17 );
	iReturn |= SetAD9363Reg( 0x002, 0xCE );
	iReturn |= SetAD9363Reg( 0x003, 0xCE );
	iReturn |= SetAD9363Reg( 0x004, 0x03 );
	iReturn |= SetAD9363Reg( 0x00A, 0x12 );
	return 0;
}



