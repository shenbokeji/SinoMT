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


