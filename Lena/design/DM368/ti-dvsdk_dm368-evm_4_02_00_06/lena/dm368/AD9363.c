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
 Int GetAD9363Reg( const UInt32 uiAddr, UInt16 * const pusRdValue )
 {
	 int iRdByteNum;
	 FILE *fid;
	 tAD9363Reg tAD9363RegTmp;
 
 
	 tAD9363RegTmp.uiAddr = uiAddr;
	 
	 fid = (FILE *)open( DEVICE_AD9363, O_RDONLY | O_RSYNC, 0 );
	 if( fid < 0 )
	 {
		 printf( "ERROR:open failed "DEVICE_FPGA"!\n" );
		 exit(1);
	 }
	 
	 iRdByteNum = read( (int)fid, &tAD9363RegTmp, BYTE_EIGHT ); //8 byte for addr and value
	 if( BYTE_EIGHT != iRdByteNum )
	 {
		 printf( "ERROR:Read Byte is not %d "DEVICE_FPGA"!\n", BYTE_EIGHT );
		 exit(1);
	 }
	 close( (int)fid );
	 fid = NULL;
	 
	 *pusRdValue = (UInt16)tAD9363RegTmp.uiValue;
		 
	 return LENA_OK;
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
 
	 tAD9363RegTmp.uiAddr = uiAddr;
	 tAD9363RegTmp.uiValue = (unsigned int)ucValue;
	 
	 fid = (FILE *)open( DEVICE_AD9363, O_RDONLY | O_RSYNC, 0 );
	 if( fid < 0 )
	 {
		 printf( "ERROR:open failed "DEVICE_FPGA"!\n" );
		 exit(1);
	 }
	 iWrByteNum = write( (int)fid, &tAD9363RegTmp, BYTE_EIGHT );
	 if( BYTE_EIGHT != iWrByteNum )
	 {
		 printf( "ERROR:Read Byte is not %d "DEVICE_FPGA"!\n", BYTE_EIGHT );
		 exit(1);
	 } 
	 close( (int)fid );
	 fid = NULL;
	 return LENA_OK;/* ?y3�� */
 
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




