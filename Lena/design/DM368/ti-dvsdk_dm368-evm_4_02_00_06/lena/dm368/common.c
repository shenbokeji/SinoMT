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

unsigned int g_AirGround = 0XFFFFFFFF;

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
 * output 	: iAirorGround: air and ground station flag,0:ground 1:air,others:invalid
 * author	version		date		note
 * feller	1.0		20150728	
 *----------------------------------------------------------------------------
*/
int GetAirGroundStationFlag()
{
    unsigned int uiAirorGround = 0xFFFFFFFF;
    int fid;

    fid = open( DEVICE_GPIO, O_RDONLY, 0 );
    if( fid < 0 )
    {
	printf( "ERROR:open failed "DEVICE_GPIO"!\n" );
	return LENA_FALSE;
     }

    //read the GPIO29 ,get the air ground station flag

    uiAirorGround = read( fid, NULL, AIR_GROUND_GPIO ); 
    printf( "uiAirorGround = %d\n", uiAirorGround >> AIR_GROUND_GPIO );
    close( fid );
    fid = NULL;
    g_AirGround = uiAirorGround;
    return uiAirorGround;
}

 /*--------------------------------------------------------------------------
 * function	: SetGPIO
 * input 	: iGPIOnumber;ivalue 
 * author	version		date		note
 * feller	1.0		20160101	
 *----------------------------------------------------------------------------
*/
int SetGPIO( const int iGPIOnumber, const char cvalue )
{
    int fid = -1;
    int iReturn = -1;

    fid = open( DEVICE_GPIO, O_RDWR, 0 );
    if( fid < 0 )
    {
	perror( "ERROR:open failed "DEVICE_GPIO"!\n" );
	return LENA_FALSE;
    }

    iReturn = write( fid, &cvalue, iGPIOnumber ); 
    close( fid );
    fid = NULL;
    return LENA_OK;
}

 /*--------------------------------------------------------------------------
 * function	: GetGPIO
 * input 	: iGPIOnumber
 * author	version		date		note
 * feller	1.0		20160101	
 *----------------------------------------------------------------------------
*/
int GetGPIO( const int iGPIOnumber )
{
    int fid = -1;
    int ivalue = -1;

    fid = open( DEVICE_GPIO, O_RDONLY, 0 );
    if( fid < 0 )
    {
	perror( "ERROR:open failed "DEVICE_GPIO"!\n" );
	return LENA_FALSE;
    }

    ivalue = read( fid, NULL, iGPIOnumber ); 
    close( fid );
    fid = NULL;
    return ivalue;
}
 /*--------------------------------------------------------------------------
 * name		: ResetFPGA
 * function	: reset fpga
 * intput 	: none
 * author	version		date		note
 * feller	1.0		20160101

 *----------------------------------------------------------------------------
*/
void ResetFPGA(void)
{
	SetGPIO( FPGA_RESET_GPIO, 1 ); 
	usleep(500);
	SetGPIO( FPGA_RESET_GPIO, 0 ); 
	usleep(500);
	SetGPIO( FPGA_RESET_GPIO, 1 ); 
	return ;
}
 