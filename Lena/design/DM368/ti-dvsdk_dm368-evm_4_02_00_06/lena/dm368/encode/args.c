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
 * filename	: usage
 * function	: argument usage 
 * author	version		date		note
 * feller	1.0		20150729	modify for lena encoder ,delete audio and speech  
 ******************************************************************************/
void usage(void)
{
    fprintf(stderr, "Usage: encode [options]\n\n"
      "Options:\n"
      "-v | --videofile        Video file to record to\n"
      "-y | --display_standard Video standard to use for display (see below).\n"
      "                        Same video standard is used at input.\n"
      "-r | --resolution       Video resolution ('width'x'height')\n"
      "                        [video standard default]\n"
      "-b | --videobitrate     Bit rate to encode video at [variable]\n"
      "-w | --preview_disable  Disable preview [preview enabled]\n"
      "-f | --write_disable    Disable recording of encoded file [enabled]\n"
      "-I | --video_input      Video input source [video standard default]\n"
      "-l | --linein           Use linein as sound input instead of mic \n"
      "                        [off]\n"
      "-k | --keyboard         Enable keyboard interface [off]\n"
      "-t | --time             Number of seconds to run the demo [infinite]\n"
      "-o | --osd              Show demo data on an OSD [off]\n"
      "-h | --help             Print this message\n\n"
      "Video standards available\n"
      "\t1\tD1 @ 30 fps (NTSC)\n"
      "\t2\tD1 @ 25 fps (PAL)\n"
      "\t3\t720P @ 60 fps [Default]\n"
      "\t5\t1080I @ 30 fps - for DM368\n"
      "Video inputs available:\n"
      "\t1\tComposite\n"
      "\t2\tS-video\n"
      "\t3\tComponent\n"
      "\t4\tImager/Camera - for DM368\n"
      "You must supply at least a video or a speech file or both\n"
      "with appropriate extensions for the file formats.\n\n");
}


/******************************************************************************
 * filename	: parseArgs
 * function	: parse encoder Args
 * author	version		date		note
 * feller	1.0		20150729	modify for lena encoder ,delete audio and speech  
 ******************************************************************************/
void parseArgs(Int argc, Char *argv[], Args *argsp)
{
    const Char shortOptions[] = "v:y:r:b:wfI:lkt:oh";
    const struct option longOptions[] = {
        {"videofile",        required_argument, NULL, 'v'},
        {"display_standard", required_argument, NULL, 'y'},
        {"resolution",       required_argument, NULL, 'r'},
        {"videobitrate",     required_argument, NULL, 'b'},
        {"preview_disable",  no_argument,       NULL, 'w'},
        {"write_disable",    no_argument,       NULL, 'f'},
        {"video_input",      required_argument, NULL, 'I'},
        {"keyboard",         no_argument,       NULL, 'k'},
        {"time",             required_argument, NULL, 't'},
        {"osd",              no_argument,       NULL, 'o'},
        {"help",             no_argument,       NULL, 'h'},
        {0, 0, 0, 0}
    };

    Int     index;
    Int     c;
    Char    *extension;

    for (;;) {
        c = getopt_long(argc, argv, shortOptions, longOptions, &index);

        if (c == -1) {
            break;
        }

        switch (c) {
            case 0:
                break;

            case 'v':
                extension = rindex(optarg, '.');
                if (extension == NULL) {
	              ERR("******Encoder Video file without extension: %s ******\n",optarg);	
                    exit(EXIT_FAILURE);
                }

                argsp->videoEncoder =
                    getCodec(extension, engine->videoEncoders);

                if (!argsp->videoEncoder) {
			ERR("******Encoder Unknown video file extension: %s ******\n",extension);		
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
                        ERR("******Encoder Unknown display standard ******\n");
                        usage();
                        exit(EXIT_FAILURE);
                }
                break;

            case 'I':
                switch (atoi(optarg)) {
                    case 1:
                        argsp->videoInput = Capture_Input_COMPOSITE;
                        break;
                    case 2:
                        argsp->videoInput = Capture_Input_SVIDEO;
                        break;
                    case 3:
                        argsp->videoInput = Capture_Input_COMPONENT;
                        break;
                    case 4:
                        argsp->videoInput = Capture_Input_CAMERA;
                        break;
                    default:
                        fprintf(stderr, "Unknown video input\n");
                        usage();
                        exit(EXIT_FAILURE);
                }
                break;

            case 'r':
            {
                Int32 imageWidth, imageHeight;

                if (sscanf(optarg, "%ldx%ld", &imageWidth,
                                              &imageHeight) != 2) {
                   ERR("******Encoder Invalid resolution supplied (%s) ******\n",optarg);
                    usage();
                    exit(EXIT_FAILURE);
                }

                /* Sanity check resolution */
                if (imageWidth < 2UL || imageHeight < 2UL ||
                    imageWidth > VideoStd_1080I_WIDTH ||
                    imageHeight > VideoStd_1080I_HEIGHT) {
                    ERR("******Encoder Video resolution must be between %dx%d and %dx%d******\n",
						2, 2, VideoStd_1080I_WIDTH, VideoStd_1080I_HEIGHT);
                    exit(EXIT_FAILURE);
                }

                argsp->imageWidth  = imageWidth;
                argsp->imageHeight = imageHeight;
                break;
            }

            case 'b':
                argsp->videoBitRate = atoi(optarg);
                argsp->videoBitRateString = optarg;
                break;

            case 'k':
                argsp->keyboard = TRUE;
                break;

            case 't':
                argsp->time = atoi(optarg);
                break;

            case 'o':
                argsp->osd = TRUE;
                break;

            case 'w':
                argsp->previewDisabled = TRUE;
                break;

            case 'f':
                argsp->writeDisabled = TRUE;
                break;

            case 'h':
                usage();
                exit(EXIT_SUCCESS);

            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    /* 
     * If video input is not set, set it to the default based on display
     * video standard.
     */
    if (argsp->videoInput == Capture_Input_COUNT) {
        switch (argsp->videoStd) {
            case VideoStd_D1_NTSC:
            case VideoStd_D1_PAL:
                argsp->videoInput = Capture_Input_COMPOSITE;
                break;
            case VideoStd_720P_60:
                argsp->videoInput = Capture_Input_COMPONENT;
                break;
            case VideoStd_1080I_30:
                argsp->videoInput = Capture_Input_CAMERA;
                break;
            default:
  		  ERR("******Encoder Unknown display standard******\n");
                usage();
                exit(EXIT_FAILURE);
                break;
        }
    }
}

/******************************************************************************
 * filename	: validateArgs
 * function	: validate the video args
 * author	version		date		note
 * feller	1.0		20150729	modify for lena encoder ,delete audio and speech  
 ******************************************************************************/

Int validateArgs(Args *argsp)
{
    Bool failed = FALSE;

    /* Need at least one file to enco */
    if ( !argsp->videoFile  ) {
        usage();
        return FAILURE;
    }

    /* Verify video standard is supported by input */
    switch (argsp->videoInput) {
        case Capture_Input_COMPOSITE:
        case Capture_Input_SVIDEO:
            if ((argsp->videoStd != VideoStd_D1_PAL) && (argsp->videoStd !=
                VideoStd_D1_NTSC)) {
                failed = TRUE;
            }
            break;
        case Capture_Input_COMPONENT:
            if ((argsp->videoStd != VideoStd_720P_60) && (argsp->videoStd
                != VideoStd_1080I_30)) {
                failed = TRUE;
            }
            break;
        case Capture_Input_CAMERA:
            break;
        default:
 	     ERR("******Encoder Invalid video input found in validation******\n");
            usage();
            return FAILURE;
    }

    if (failed) {
	     ERR("******Encoder This combination of video input and video standard is not supported******\n");		
            usage();
            return FAILURE;
    }
    
    return SUCCESS;
}

