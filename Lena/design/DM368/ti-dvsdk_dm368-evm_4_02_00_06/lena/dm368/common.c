/*
 * This source file has the argument process header file for the 'lena encode' on DM368 platform

 *****************************************************************************
 * Copyright(C) 2015, SinoMartin Corp.
 *----------------------------------------------------------------------------
 * filename	: common.h
 * function	: the common definition for air and ground
 * author	version		date		note
 * feller	1.0		20150728	create         
 *----------------------------------------------------------------------------
*/
#include <sys/mman.h>
#include <fcntl.h>
#include "common.h"

/******************************************************************************
 * getCodec
 ******************************************************************************/
Codec *getCodec(Char *extension, Codec *codecs)
{
    Codec *codec = NULL;
    Int i, j;

    i = 0;
    while (codecs[i].codecName) {
        j = 0;
        while (codecs[i].fileExtensions[j]) {
            if (strcmp(extension, codecs[i].fileExtensions[j]) == 0) {
                codec = &codecs[i];
            }
            j++;
        }
        i++;
    }

    return codec;
}

 /*--------------------------------------------------------------------------
 * function	: GetAirGroundStationFlag
 * output 		: iAirorGround: air and ground station flag,0:ground 1:air,others:invalid
 * author	version		date		note
 * feller	1.0		20150728	
 *----------------------------------------------------------------------------
*/
Int GetAirGroundStationFlag()
{
    Int iAirorGround = 0xFFFFFFFF;
    //read the GPIO29 ,get the air ground station flag
    //iAirorGround = 0;
    
    return iAirorGround;
}
 /*--------------------------------------------------------------------------
 * name			: InitMmapAddress
 * function		:  initialize the mmap address 
 * output 		:
 * author	version		date		note
 * feller	1.0		20150807	
 *----------------------------------------------------------------------------
*/
Int InitMmapAddress( void  )
 {

    Int32 fd = -1;
 
    fd = open( DEV_MEM_MMAP, O_RDWR | O_CREAT );	 
    if( fd < 0)
    {
        exit( 1 );
    }

    g_uiRegBaseAddr=( UInt32 )mmap( 
	 	( void * )EMIF_BASE_ADDRESS, 
	 	LINUX_PAGE_SIZE,
	 	PROT_READ | PROT_WRITE,
	 	MAP_SHARED,
	 	fd,  
	 	0);
    if( MAP_FAILED == ( void* )g_uiRegBaseAddr )
    {
        close( fd );
        exit( 0 );
    }
    return 0 ;
 }

 /*--------------------------------------------------------------------------
 * name			: physta
 * function		: 无线物理层KPI统计显示
 * intput 		: iNum display times, per 3 second
 * author	version		date		note
 * feller	1.0		20151107
 *----------------------------------------------------------------------------
*/
int iSFN[8] = { 0, 3, 4, 5, 6, 7, 8, 9 };
int iTBSize[2] = { 35160, 2728 };
void physta( int iNum )
{
	int ii;
	int jj;
	int kk;
	for( kk = 1; kk < iNum; kk++ )
	{
		printf( "\n***************************************************************************************\n");
		for( ii = 0; ii < 2; ii++ )
		{
			for( jj = 0; jj < 8; jj++ )
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
		printf( "\n***************************************************************************************\n");
		sleep( 3 );
	}
}
