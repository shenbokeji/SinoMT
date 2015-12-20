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
 * output 		: iAirorGround: air and ground station flag,0:ground 1:air,others:invalid
 * author	version		date		note
 * feller	1.0		20150728	
 *----------------------------------------------------------------------------
*/
Int GetAirGroundStationFlag()
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
    printf( "uiAirorGround = %d\n", uiAirorGround );
    close( fid );
    fid = NULL;
    g_AirGround = uiAirorGround;
    return uiAirorGround;
}
 #if 0
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
#endif
