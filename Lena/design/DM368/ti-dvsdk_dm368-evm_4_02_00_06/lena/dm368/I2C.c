/*
 * This source file is AD9363 file for the 'lena encode' on DM368 platform

 *****************************************************************************
 * Copyright(C) 2015, SinoMartin Corp.
 *----------------------------------------------------------------------------
 * filename	: I2C.c
 * function	: initialize /write /read interface
 * author	version		date		note
 * feller	1.0		20151229	create         
 *----------------------------------------------------------------------------
*/
#include "I2C.h"

 /*----------------------------------------------------------------------------
  * name		 : GetIT66121Reg
  * function	 : Get the IT66121 register value 
  * input		 : uiAddr:register address; pucRdValue:output value pointer
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */
 Int GetIT66121Reg( const unsigned int uiAddr, unsigned char * const pucRdValue )
 {
	int iRdByteNum;
	int fid;
	int iReturn = LENA_OK;
	tI2CReg tRegTmp;
 
	tRegTmp.uiAddr = uiAddr;
	 
	fid = open( DEVICE_IT66121, O_RDONLY, 0 );

	if( fid < 0 )
	{
		perror( DEVICE_IT66121"ERROR:open failed\n" );
		return LENA_FALSE;
	}
	 
	iRdByteNum = read( fid, &tRegTmp, BYTE_EIGHT ); //8 byte for addr and value
	if( BYTE_EIGHT != iRdByteNum )
	{
		printf( DEVICE_IT66121"ERROR:Read Byte is not %d\n", BYTE_EIGHT );
		iReturn = LENA_FALSE;
		*pucRdValue = (unsigned char)ERROR_STRING;
	}
	else
	{
		*pucRdValue = (unsigned char)tRegTmp.uiValue;
	}
	close( fid );
	fid = NULL;
	 		 
	return iReturn;
 }
  /*----------------------------------------------------------------------------
  * name		 : SetIT66121Reg
  * function	 : set the IT66121 register
  * input		 : uiAddr :register address ; ucValue: set register value 
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */
 Int SetIT66121Reg( const unsigned int uiAddr, const unsigned char ucValue )
 {
	 int iWrByteNum;
	 int fid;
	 tI2CReg tRegTmp;
	 int iReturn = LENA_OK;

	 tRegTmp.uiAddr = uiAddr;
	 tRegTmp.uiValue = (unsigned int)ucValue;
	 
	 fid = open( DEVICE_IT66121, O_RDWR, 0 );
	 if( fid < 0 )
	 {
		 perror( DEVICE_IT66121"ERROR:open failed\n" );
		 return LENA_FALSE;
	 }
	 iWrByteNum = write( fid, &tRegTmp, BYTE_EIGHT );
	 if( BYTE_EIGHT != iWrByteNum )
	 {
		 printf( DEVICE_IT66121"ERROR:Write Byte is not %d\n", BYTE_EIGHT );
		 iReturn =  LENA_FALSE;
	 } 
	 close( fid );
	 fid = NULL;
	 return iReturn;
 }



 /*----------------------------------------------------------------------------
  * name	: GetADV7611Reg
  * function	: Get the ADV7611 register value 
  * input	: uiAddr:register address; pucRdValue:output value pointer
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */
 Int GetADV7611Reg( const unsigned int uiAddr, unsigned char * const pucRdValue )
 {
	 int iRdByteNum;
	 int fid;
	 int iReturn = LENA_OK;
	 tI2CReg tRegTmp;
 
	 tRegTmp.uiAddr = uiAddr;
	 
	 fid = open( DEVICE_ADV7611, O_RDONLY, 0 );

	 if( fid < 0 )
	 {
		 perror( DEVICE_ADV7611"ERROR:open failed\n" );
		 return LENA_FALSE;
	 }
	 
	 iRdByteNum = read( fid, &tRegTmp, BYTE_EIGHT ); //8 byte for addr and value
	 if( BYTE_EIGHT != iRdByteNum )
	 {
		 printf( DEVICE_ADV7611"ERROR:Read Byte is not %d\n", BYTE_EIGHT );
		 iReturn = LENA_FALSE;
		*pucRdValue = (unsigned char)ERROR_STRING;
	 }
	else
	{
		*pucRdValue = (unsigned char)tRegTmp.uiValue;
	}
	 close( fid );
	 fid = NULL;
	 		 
	 return iReturn;
 }
  /*----------------------------------------------------------------------------
  * name	 : SetADV7611Reg
  * function	 : set the ADV7611 reg 
  * input	 : uiAddr :register address ; ucValue: set register value 
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */
 Int SetADV7611Reg( const unsigned int uiAddr, const unsigned char ucValue )
 {
	 int iWrByteNum;
	 int fid;
	 tI2CReg tRegTmp;
	 int iReturn = LENA_OK;

	 tRegTmp.uiAddr = uiAddr;
	 tRegTmp.uiValue = (unsigned int)ucValue;
	 
	 fid = open( DEVICE_ADV7611, O_RDWR, 0 );
	 if( fid < 0 )
	 {
		 perror( DEVICE_ADV7611"ERROR:open failed \n" );
		 return LENA_FALSE;
	 }
	 iWrByteNum = write( fid, &tRegTmp, BYTE_EIGHT );
	 if( BYTE_EIGHT != iWrByteNum )
	 {
		 printf( DEVICE_ADV7611"ERROR:Write Byte is not %d\n", BYTE_EIGHT );
		 iReturn =  LENA_FALSE;
	 } 
	 close( fid );
	 fid = NULL;
	 return iReturn;
 }
  
/*----------------------------------------------------------------------------
* name	   : Get66121Sta
* function : get IT66121 status
* input    : none
* author   version	   date 	   note
* feller   1.0	   20160331 	 
*----------------------------------------------------------------------------
*/
void Get66121Sta( void )
{
	unsigned int uiAddrTmp = IT66121_STATUS_ADDR;
	int iReturn;
	unsigned char ucRdValue;
	unsigned char ucValue[4];
	iReturn = GetIT66121Reg( uiAddrTmp, &ucRdValue );
	printf( "addr=%#X	value=%#X\n", uiAddrTmp, ucRdValue );
	ucValue[0] =	ucRdValue & 0x80;
	printf( "HDMI interrupt is %s\n", ucValue[0] ? "active": "non active" );  
	ucValue[1] =	ucRdValue & 0x40;
	printf( "HDMI hot plug is %s\n", ucValue[1] ? "plug on": "plug off" );	  
	ucValue[2] =	ucRdValue & 0x10;
	printf( "HDMI video input  is %s\n", ucValue[2] ? "stable": "unstable" ); 
	ucValue[3] =	ucRdValue & 0x01;
	printf( "HDMI make interrupt clear %s\n", ucValue[3] ? "active": "disable" ); 		  
	return ;
}
  
/*----------------------------------------------------------------------------
* name	   : Get7611Sta
* function : get adv7611 status
* input    : none
* author   version	   date 	   note
* feller   1.0	   20160331 	 
*----------------------------------------------------------------------------
*/
void Get7611Sta( void )
{
  unsigned int uiAddrTmp = ADV611_STATUS_ADDR;
  int iReturn;
  unsigned char ucRdValue = 0;
  unsigned char ucValue[5];
  iReturn = GetADV7611Reg( uiAddrTmp, &ucRdValue );
  
  printf( "addr=%#X	value=%#X\n", uiAddrTmp, ucRdValue );
  ucValue[0] =	ucRdValue & 0x40;
  printf( "HDMI TMDS PLL %s\n", ucValue[0] ? "lock": "not lock" );  
  ucValue[1] =	ucRdValue & 0x10;
  printf( "HDMI TMDS clock is %s\n", ucValue[1] ? "detect": "not detect" );	  
  ucValue[2] =	ucRdValue & 0x2;
  printf( "HDMI vertical sync parameters is %s\n", ucValue[2] ? "valid": "invalid" ); 
  ucValue[3] =	ucRdValue & 0x01;
  printf( "HDMI DE regeneration block %s\n", ucValue[3] ? "lock": "not lock" ); 
 
  return ;
}


