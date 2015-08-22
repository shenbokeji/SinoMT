/*
 * This source file has the argument process header file for the 'lena encode' on DM368 platform

 *****************************************************************************
 * Copyright(C) 2015, SinoMartin Corp.
 *----------------------------------------------------------------------------
 * filename	: args.h
 * function	: parse the arg and validate it
 * author	version		date		note
 * feller	1.0		20150728	create         
 *----------------------------------------------------------------------------
*/

#ifndef _ARGS_H

#define _ARGS_H

#include "../common.h"

#include "video.h"
#include "capture.h"
#include "writer.h"



/*****************structure type definition*****************/
// the video args
typedef struct Args {
    VideoStd_Type  videoStd;
    Char          *videoStdString;
    Capture_Input  videoInput;
    Char          *videoFile;
    Codec         *videoEncoder;
    Int32          imageWidth;
    Int32          imageHeight;
    Int            videoBitRate;
    Char          *videoBitRateString;
    Int            keyboard;
    Int            time;
    Int            osd;
    Bool           previewDisabled;//FALSE:preview enabled ; TRUE:preview disabled
    Bool           writeDisabled;//FALSE:write enabled ; TRUE:write disabled	
} Args;

/*****************macro definition*****************/




#define DEFAULT_ARGS\
    { VideoStd_720P_60,\
      "720P 60Hz",\
      Capture_Input_COUNT,\
      NULL,\
      NULL,\
      0,\
      0,\
      -1,\
      NULL,\
      FALSE,\
      FOREVER,\
      FALSE,\
      TRUE,\
      FALSE}

/* Add argument number x of string y */
#define addArg(x, y)                     \
    argv[(x)] = malloc(strlen((y)) + 1); \
    if (argv[(x)] == NULL)               \
        return FAILURE;                  \
    strcpy(argv[(x)++], (y))

/* The levels of initialization */
#define LOGSINITIALIZED         (0x1)
#define DISPLAYTHREADCREATED    (0x20)
#define CAPTURETHREADCREATED    (0x40)
#define WRITERTHREADCREATED     (0x80)
#define VIDEOTHREADCREATED      (0x100)


/* Thread priorities */
#define VIDEO_THREAD_PRIORITY   sched_get_priority_max(SCHED_FIFO) - 1

/*****************function declaration*****************/

extern void usage(void);
extern void parseArgs(Int argc, Char *argv[], Args *argsp);
extern Int validateArgs(Args *argsp);

#endif /* _ARGS_H */
