/*
 * This source file has the argument process for the 'lena encode' on DM368 platform
 * and it has the common definition for encode
 *****************************************************************************
 * Copyright(C) 2015, SinoMartin Corp.
 *----------------------------------------------------------------------------
 * filename	: args.c
 * function	: parse the arg and validate it
 * author	version		date		note
 * feller	1.0		20150728	create         
 *----------------------------------------------------------------------------*/

#include "args.h"


/******************************************************************************
 * usage
 ******************************************************************************/
Void usage(void)
{
    fprintf(stderr, "Usage: decode [options]\n\n"
      "Options:\n"
      "-a | --audiofile        Audio file to play\n"
      "-s | --speechfile       Speech file to play\n"
      "-v | --videofile        Video file to play\n"
      "-y | --display_standard Video standard to use for display (see below).\n"
      "-O | --display_output   Video output to use (see below).\n"
      "-k | --keyboard         Enable keyboard interface [off]\n"
      "-t | --time             Number of seconds to run the demo [infinite]\n"
      "-l | --loop             Loop to beginning of files when done [off]\n"
      "-o | --osd              Show demo data on an OSD [off]\n"
      "-h | --help             Print this message\n\n"
      "Video standards available:\n"
      "\t1\tD1 @ 30 fps (NTSC)\n"
      "\t2\tD1 @ 25 fps (PAL)\n"
      "\t3\t720P @ 60 fps [Default]\n"
      "\t5\t1080I @ 30 fps - for DM368\n"
      "Video outputs available:\n"
      "\tcomposite\n"
      "\tcomponent (Only 720P and 1080I available) [Default]\n"
      "You must supply at least a video or a speech or an audio file\n"
      "with appropriate extensions for the file formats.\n"
      "You must NOT supply BOTH an audio and a speech file.\n\n");
}

/******************************************************************************
 * parseArgs
 ******************************************************************************/
 void parseArgs(Int argc, Char *argv[], Args *argsp)
{
   // const Char shortOptions[] = "a:s:v:y:O:kt:lfoh";
   const Char shortOptions[] = "v:y:O:kt:lfoh";
    const struct option longOptions[] = {
        //{"audiofile",        required_argument, NULL, 'a'},
        //{"speechfile",       required_argument, NULL, 's'},
        {"videofile",        required_argument, NULL, 'v'},
        {"display_standard", required_argument, NULL, 'y'},
        {"display_output",   required_argument, NULL, 'O'},
        {"keyboard",         no_argument,       NULL, 'k'},
        {"time",             required_argument, NULL, 't'},
        {"loop",             no_argument,       NULL, 'l'},
        {"osd",              no_argument,       NULL, 'o'},
        {"help",             no_argument,       NULL, 'h'},
        {"exit",             no_argument,       NULL, 'e'},            
        {0, 0, 0, 0}
    };

    Int   index;
    Int   c;
    Char *extension;

    for (;;) {
        c = getopt_long(argc, argv, shortOptions, longOptions, &index);

        if (c == -1) {
            break;
        }

        switch (c) {
            case 0:
                break;
#if 0
            case 'a':
                extension = rindex(optarg, '.');
                if (extension == NULL) {
                    fprintf(stderr, "Audio file without extension: %s\n",
                            optarg);
                    exit(EXIT_FAILURE);
                }

                argsp->audioDecoder =
                    getCodec(extension, engine->audioDecoders);

                if (!argsp->audioDecoder) {
                    fprintf(stderr, "Unknown audio file extension: %s\n",
                            extension);
                    exit(EXIT_FAILURE);
                }
                argsp->audioFile = optarg;

                break;

            case 's':
                extension = rindex(optarg, '.');
                if (extension == NULL) {
                    fprintf(stderr, "Speech file without extension: %s\n",
                            optarg);
                    exit(EXIT_FAILURE);
                }

                argsp->speechDecoder =
                    getCodec(extension, engine->speechDecoders);

                if (!argsp->speechDecoder) {
                    fprintf(stderr, "Unknown speech file extension: %s\n",
                            extension);
                    exit(EXIT_FAILURE);
                }
                argsp->speechFile = optarg;

                break;
#endif
            case 'v':
                extension = rindex(optarg, '.');
                if (extension == NULL) {
                    fprintf(stderr, "Video file without extension: %s\n",
                            optarg);
                    exit(EXIT_FAILURE);
                }

                argsp->videoDecoder =
                    getCodec(extension, engine->videoDecoders);

                if (!argsp->videoDecoder) {
                    fprintf(stderr, "Unknown video file extension: %s\n",
                            extension);
                    exit(EXIT_FAILURE);
                }
                argsp->videoFile = optarg;

                break;

            case 'y':
                switch (atoi(optarg)) {
                    case 1:
                        argsp->videoStd = VideoStd_D1_NTSC;
                        argsp->videoStdString = "D1 NTSC";
                        break;
                    case 2:
                        argsp->videoStd = VideoStd_D1_PAL;
                        argsp->videoStdString = "D1 PAL";
                        break;
                    case 3:
                        argsp->videoStd = VideoStd_720P_60;
                        argsp->videoStdString = "720P 60Hz";
                        break;
                    case 5:
                        argsp->videoStd = VideoStd_1080I_30;
                        argsp->videoStdString = "1080I 30Hz";
                        break;
                    default:
                        fprintf(stderr, "Unknown display resolution\n");
                        usage();
                        exit(EXIT_FAILURE);
                }
                break;

            case 'O':
                if (strcmp(optarg, "component") == 0) {
                    argsp->displayOutput = Display_Output_COMPONENT;
                } else if (strcmp(optarg, "composite") == 0) {
                    argsp->displayOutput = Display_Output_COMPOSITE;
                } else {
                    fprintf(stderr, "Unknown video output: %s\n", optarg);
                    usage();
                    exit(EXIT_FAILURE);
                }
                break;

            case 'k':
                argsp->keyboard = TRUE;
                break;

            case 't':
                argsp->time = atoi(optarg);
                break;

            case 'l':
                argsp->loop = TRUE;
                break;

            case 'o':
                argsp->osd = TRUE;
                break;

            case 'h':
                usage();
                exit(EXIT_SUCCESS);

            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    if (argsp->displayOutput == Display_Output_COUNT) {
        if ((argsp->videoStd == VideoStd_D1_NTSC) || 
            (argsp->videoStd == VideoStd_D1_PAL)) {
            argsp->displayOutput = Display_Output_COMPOSITE;
        } else {
            argsp->displayOutput = Display_Output_COMPONENT;
        }
    }
}

/******************************************************************************
 * validateArgs
 ******************************************************************************/
Int validateArgs(Args *argsp)
{
    /* Need at least one file to decode and only one sound file */
    if ( !argsp->videoFile  ) {
        usage();
        return FAILURE;
    }
    
    return SUCCESS;
}

