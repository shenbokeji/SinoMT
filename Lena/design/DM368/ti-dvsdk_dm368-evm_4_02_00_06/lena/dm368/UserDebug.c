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
 * function 	: debug function for user
 * input 		:uiAddr:11bit offset address in FPGA, usValue: value 
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
 * function 	: debug function for user
 * input 		:uiAddr:11bit offset address in FPGA, uiLength: register number ,every 16bit 
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

 /*--------------------------------------------------------------------------
 * name			: physta
 * function		: wireless phyical layer KPI static
 * intput 		: iNum display times, per 3 second
 * author	version		date		note
 * feller	1.0		20151107
 *----------------------------------------------------------------------------
*/
	 
#define TB_STREAM_NUM (2)
#define SFN_NUM (8)
	 
typedef struct SFNStruct {
	 unsigned int iSFN;
	 float fBLER;//block error ratio
	 float fBER;//bit error ratio
} tSFNInfo;
	 
typedef struct FRAMEKPIStruct {
	 unsigned int iFN;//frame number
	 tSFNInfo tSFN[ SFN_NUM ];//subframe number
	 unsigned int iTBSize;//transfer block
	 unsigned int iMod;//modem 
	 float fTO;//throught output
} tFRAMEKPI;
 tFRAMEKPI tTBStream[ TB_STREAM_NUM ];
	 
char * sp[] = { "TB:", "FN:", "SFN:", "TBSIZE:", "BLER:", "BER:", "TO:", "Mbps" };
	 
	 
int iSFN[8] = { 0, 3, 4, 5, 6, 7, 8, 9 };
int iTBSize[2] = { 35160, 2728 };
char *sModemStr[] = { "16QAM" , "QPSK"};

void physta( int iFlag )
{
	int ii;
	int jj;
	int kk;
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



void wr9363()
{
	return;
}
void SPIWrite()
{
	return;
}
void rd9363()
{
	return;
}
