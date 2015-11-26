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

 /*----------------------------------------------------------------------------
 * name		: wrfpga
 * function	: write FPGA register
 * input 	: uiAddr:FPGA regiser address 
 			  usValue: write value
 			  uiFlag: read back flag
 * author	version		date		note
 * feller	1.0		20151016      
 *----------------------------------------------------------------------------
*/
void wrfpga(  const UInt32 uiAddr, const UInt16 usValue, const UInt uiFlag ) 
{
    Int iReturn;
	UInt16 usRdValue;
	
	if( FPGA_ADDR_VALID(uiAddr) )
	{
		printf( "******error: address is invalid******\n");		
		printf( "addr = %#X \n", (unsigned int)uiAddr );	
	}
	
	iReturn = SetFpgaReg( uiAddr<<1, usValue );
	if( 0 != uiFlag ) 
	{
	    iReturn = GetFpgaReg( uiAddr<<1, &usRdValue );
		printf( "addr = %#X value =%#X\n", (unsigned int)uiAddr, usRdValue );	
	}
	return;
}
/*----------------------------------------------------------------------------
 * name		: rdfpga
 * function	: read FPGA register
 * input 	: uiAddr:FPGA regiser start address 
 			  uiRdNum: read register number
 * author	version		date		note
 * feller	1.0		20151016      
 *----------------------------------------------------------------------------
*/
void rdfpga(  const UInt32 uiAddr, const UInt uiRdNum ) 
{
    Int ii;
	UInt32 uiAddrTmp;
    Int iReturn;
	UInt16 usRdValue;

	if( FPGA_ADDR_VALID(uiAddr) )
	{
		printf( "******error: start address is invalid******\n"); 	
		printf( "addr = %#X \n", (unsigned int)uiAddr );	
	}

	
	uiAddrTmp = uiAddr;
	for ( ii = 0; ii < uiRdNum; ii++ )
	{
		if( FPGA_ADDR_VALID(uiAddrTmp) )
		{
			break;
		}

	    iReturn = GetFpgaReg( uiAddr<<1, &usRdValue );
		printf( "addr = %#X value =%#X\n", (unsigned int)uiAddr, usRdValue );	
	}

	return;
}
 /*--------------------------------------------------------------------------
 * name			: physta
 * function		: wireless phyical layer KPI static
 * intput 		: iNum display times, per 3 second
 * author	version		date		note
 * feller	1.0		20151107
 *----------------------------------------------------------------------------
*/

	 
char * sp[] = { "TB:", "FN:", "SFN:", "TBSIZE:", "BLER:", "BER:", "TO:", "Mbps" };
	 
	 
int iSFN[8] = { 0, 3, 4, 5, 6, 7, 8, 9 };
int iTBSize[2] = { 35160, 2728 };
char *sModemStr[] = { "16QAM" , "QPSK"};

void physta( int iFlag )
{
	int ii;
	int jj;
	//int kk;
	unsigned int iFNtmp;
	float fBLERTmp;
	float fBERTmp;
	float fTO;

//	for( kk = 0; kk < iNum; kk++ )
	{
		printf( "\n****************************WIRELESS PHYSICAL LAYER KPI STATISTIC****************************\n");
		for( ii = 0; ii < 1; ii++ )
		{
			for( jj = 0; jj < 8; jj++ )
			{
				if( iFlag > 0 )
				{
					iFNtmp = ii;
					fBLERTmp = 0.001;
					fBERTmp = 0.0001;
					fTO = 35160 * 0.8 / 1000;
					printf( "%3.3s1\t%3.3s%d\t%4.4s%d\t", sp[0], sp[1], iFNtmp, sp[2], iSFN[jj]);
					printf( "%7.7s\t%5d\t%5.5s\t%5.5s%.3f\t", sp[3], iTBSize[0], sModemStr[0], sp[4], fBLERTmp );
					printf( "%4.4s%.4f\t%3.3s%6.3f%4.4s\n", sp[5], fBERTmp, sp[6], fTO, sp[7] );
				
					fBLERTmp = 0.002;
					fBERTmp = 0.0002;
					fTO = 2728 * 0.8 / 1000;
				
					printf( "%3.3s2\t%3.3s%d\t%4.4s%d\t", sp[0], sp[1], iFNtmp, sp[2], iSFN[jj]);
					printf( "%7.7s\t%5d\t%5.5s\t%5.5s%.3f\t", sp[3], iTBSize[1], sModemStr[1], sp[4], fBLERTmp );
					printf( "%4.4s%.4f\t%3.3s%6.3f%4.4s\n", sp[5], fBERTmp, sp[6], fTO, sp[7] );					
				}
				else if( 0 == iFlag )//print for static info
				{
					if( ( 0 == ii ) && ( jj < 2 ) )
					{	
						printf( "TB1\tFN:%d\tSFN:%d\tTBSIZE:%d\t16QAM\tBLER:0.001%%\tBER:0.0001%%\tTO:28.128\n", ii, iSFN[jj], iTBSize[0] );
						printf( "TB2\tFN:%d\tSFN:%d\tTBSIZE:%d\tQPSK\tBLER:0.001%%\tBER:0.0001%%\tTO:2.182\n", ii, iSFN[jj], iTBSize[1] );
					}
					else
					{
						printf( "TB1\tFN:%d\tSFN:%d\tTBSIZE:%d\t16QAM\tBLER:0.0%%\tBER:0.0%%\tTO:28.128\n", ii, iSFN[jj], iTBSize[0] );
						printf( "TB2\tFN:%d\tSFN:%d\tTBSIZE:%d\tQPSK\tBLER:0.0%%\tBER:0.0%%\tTO:2.182\n", ii, iSFN[jj], iTBSize[1] );
					}		
				}
			}
		}
		printf( "\n*********************************************************************************************\n");
		sleep( 1 );
	}
}


/*----------------------------------------------------------------------------
 * name		: wr9363
 * function	: config AD9363 by spi4
 * input 	: 
 * author	version		date		note
 * feller	1.0		20151007      
 *----------------------------------------------------------------------------
*/

void wr9363(   const UInt32 uiAddr, const unsigned char usValue, const UInt uiFlag  )
{
	
	return;
}
/*----------------------------------------------------------------------------
 * name		: SPIWrite
 * function	: the same as wr9363,for somebody is used to use it
 * input 	: 
 * author	version		date		note
 * feller	1.0		20151119   
 *----------------------------------------------------------------------------
*/

void SPIWrite()
{

	return;
}
/*----------------------------------------------------------------------------
 * name		: rd9363
 * function	: read AD9363 by spi4
 * input 	: 
 * author	version		date		note
 * feller	1.0		20151119      
 *----------------------------------------------------------------------------
*/

void rd9363()
{
	return;
}


