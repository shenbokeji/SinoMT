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
 * function		: InitMmapAddress
 * 				  initialize the mmap address 
 * output 		: iAirorGround: air and ground station flag,0:ground 1:air,others:invalid
 * author	version		date		note
 * feller	1.0		20150801	
 *----------------------------------------------------------------------------
*/
Int InitMmapAddress(  const Int iAirorGround  )
 {
 
     return 0 ;
 }
 