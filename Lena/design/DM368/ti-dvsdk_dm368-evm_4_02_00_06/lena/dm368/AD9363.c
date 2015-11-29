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
  * name		 : GetFpgaReg
  * function	 : Get the fpga reg value 
  * input		 :
  * author	 version	 date		 note
  * feller	 1.0	 20150805
  * feller	 1.0	 20151119		 I am at home with phyliss
  *----------------------------------------------------------------------------
 */
 Int GetAD9363Reg( const UInt32 uiAddr, unsigned char * const pucRdValue )
 {
	 int iRdByteNum;
	 FILE *fid;
	 int iReturn = LENA_OK;
	 tAD9363Reg tAD9363RegTmp;
 
 
	 tAD9363RegTmp.uiAddr = uiAddr;
	 
	 fid = (FILE *)open( DEVICE_AD9363, O_RDONLY, 0 );

	 if( (int)fid < 0 )
	 {
		 printf( "ERROR:open failed "DEVICE_AD9363"!\n" );
		 return LENA_FALSE;
	 }
	 
	 iRdByteNum = read( (int)fid, &tAD9363RegTmp, BYTE_EIGHT ); //8 byte for addr and value
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
	 close( (int)fid );
	 fid = NULL;
	 		 
	 return iReturn;
 }
  /*----------------------------------------------------------------------------
  * name		 : SetFpgaReg
  * function		 : set the fpga reg 
  * input			 :
  * author	 version	 date		 note
  * feller	 1.0	 20150805
  *----------------------------------------------------------------------------
 */
 Int SetAD9363Reg( const UInt32 uiAddr, const unsigned char ucValue )
 {
	 Int iWrByteNum;
	 FILE *fid;
	 tAD9363Reg tAD9363RegTmp;
	 int iReturn = LENA_OK;

	 tAD9363RegTmp.uiAddr = uiAddr;
	 tAD9363RegTmp.uiValue = (unsigned int)ucValue;
	 
	 fid = (FILE *)open( DEVICE_AD9363, O_RDWR, 0 );
	 if( (int)fid < 0 )
	 {
		 printf( "ERROR:open failed "DEVICE_AD9363"!\n" );
		 return LENA_FALSE;
	 }
	 iWrByteNum = write( (int)fid, &tAD9363RegTmp, BYTE_EIGHT );
	 if( BYTE_EIGHT != iWrByteNum )
	 {
		 printf( "ERROR:Write Byte is not %d "DEVICE_AD9363"!\n", BYTE_EIGHT );
		 iReturn =  LENA_FALSE;
	 } 
	 close( (int)fid );
	 fid = NULL;
	 return iReturn;
 }



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
    g_tAD9363Cfg.AD9363Reg[GROUND_STATION][1].uiValue = 1;
    g_tAD9363Cfg.iValidLen[GROUND_STATION]  = 5;







    g_tAD9363Cfg.AD9363Reg[AIR_STATION][0].uiAddr = 0;
    g_tAD9363Cfg.AD9363Reg[AIR_STATION][1].uiValue = 1;
    g_tAD9363Cfg.iValidLen[AIR_STATION]  = 8;


	
    return ;
}
void initclock()
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
	return;
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
	 iTmp = g_tAD9363Cfg.AD9363Reg[iAirorGround][iLoop].uiValue;	

        //configure AD9363 by SPI,through FPGA
	 
    }
		
    return ;
}




