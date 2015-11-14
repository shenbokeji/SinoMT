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

