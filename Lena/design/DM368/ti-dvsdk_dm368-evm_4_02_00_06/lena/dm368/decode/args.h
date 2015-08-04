/*
 * This source file has the argument process header file for the 'lena decoder' on DM368 platform

 *****************************************************************************
 * Copyright(C) 2015, SinoMartin Corp.
 *----------------------------------------------------------------------------
 * filename	: args.h
 * function	: decoder parse the arg and validate it
 * author	version		date		note
 * feller	1.0		20150731	create         
 *----------------------------------------------------------------------------
*/

#ifndef _ARGS_H
#define _ARGS_H

#include "display.h"
#include "video.h"
#include "loader.h"
#include "../ctrl.h"
#include "../demo.h"
#include "../ui.h"
#include "../common.h"

/* The levels of initialization */
#define LOGSINITIALIZED         (0x1)
#define DISPLAYTHREADCREATED    (0x2)
#define VIDEOTHREADCREATED      (0x4)
#define SPEECHTHREADCREATED     (0x8)
#define LOADERTHREADCREATED     (0x10)
#define AUDIOTHREADCREATED      (0x20)

/* Thread priorities */
#define LOADER_THREAD_PRIORITY  (sched_get_priority_max(SCHED_FIFO) - 2)
#define SPEECH_THREAD_PRIORITY  (sched_get_priority_max(SCHED_FIFO) - 2)
#define AUDIO_THREAD_PRIORITY   (sched_get_priority_max(SCHED_FIFO) - 2)
#define VIDEO_THREAD_PRIORITY   (sched_get_priority_max(SCHED_FIFO) - 1)
#define DISPLAY_THREAD_PRIORITY (sched_get_priority_max(SCHED_FIFO) )

/* Maximum arguments length to the qtinterface application */
#define MAX_INTERFACE_ARGS_LENGTH (1000)

/* Add argument number x of string y */
#define addArg(x, y)                     \
    argv[(x)] = malloc(strlen((y)) + 1); \
    if (argv[(x)] == NULL)               \
        return FAILURE;                  \
    strcpy(argv[(x)++], (y))

typedef struct Args {
    Display_Output displayOutput;
    VideoStd_Type  videoStd;
    Char          *videoStdString;
   // Char          *speechFile;
   // Char          *audioFile;
    Char          *videoFile;
    //Codec         *audioDecoder;
   // Codec         *speechDecoder;
    Codec         *videoDecoder;
    Int            loop;
    Int            keyboard;
    Int            time;
    Int            osd;
} Args;
#define DEFAULT_ARGS \
    {Display_Output_COUNT,\
    VideoStd_720P_60,\
    "720P 60Hz",\
    NULL,\
    NULL,\
    FALSE,\
    FALSE,\
    FOREVER,\
    FALSE}

/*****************function declaration*****************/
extern void usage(void);
extern void parseArgs(Int argc, Char *argv[], Args *argsp);
extern Int validateArgs(Args *argsp);

#endif /* _ARGS_H */
