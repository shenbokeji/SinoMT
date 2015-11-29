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

#ifndef _COMMON_H
#define _COMMON_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <strings.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <signal.h>
#include <fcntl.h>

#include <xdc/std.h>

#include <ti/sdo/ce/trace/gt.h>
#include <ti/sdo/ce/CERuntime.h>

#include <ti/sdo/dmai/Dmai.h>
#include <ti/sdo/dmai/Fifo.h>
#include <ti/sdo/dmai/Pause.h>
#include <ti/sdo/dmai/Sound.h>
#include <ti/sdo/dmai/VideoStd.h>
#include <ti/sdo/dmai/Capture.h>
#include <ti/sdo/dmai/BufferGfx.h>
#include <ti/sdo/dmai/Rendezvous.h>

#include <ti/sdo/fc/rman/rman.h>
#include "demo.h"
#include "ctrl.h"
#include "FPGA.h"
#include "AD9363.h"
#include "../ushellagent.h"
//This is the air and ground station flag, configured by GPIO29 pin u2
#define AIR_STATION (1)
#define GROUND_STATION (0)

//air and ground version string
#define AIR_VERSION "AIR_V1.01.001\n"
#define GROUND_VERSION "GROUND_V0.01.001p\n"
#define DSP_TIME "DSP_"__DATE__ __TIME__"\n"
#define LENA_OK (0)
#define LENA_FALSE (-1)

/*****************system error code *****************/
#define FPGA_SET_REG_ERROR (0x10010001)
#define BYTE_EIGHT (8)
#define ERROR_STRING (0X4553)
/*****************extern function declaration*****************/
extern Codec *getCodec(Char *extension, Codec *codecs);
extern Int GetAirGroundStationFlag( void );
extern Int InitMmapAddress( void );
#endif /* _COMMON_H */

