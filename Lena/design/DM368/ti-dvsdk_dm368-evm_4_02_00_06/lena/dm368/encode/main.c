/*
 * This source file has the main for the 'lena encode' on DM368 platform
 *****************************************************************************
 * Copyright(C) 2015, SinoMartin Corp.
 *----------------------------------------------------------------------------
 * filename	: main.c
 * function	: initilize the encode system and video encode 
 * author	version		date		note
 * feller	1.0		20150728	create         
 *----------------------------------------------------------------------------
*/

#include "args.h"

GlobalData gbl = GBL_DATA_INIT;


 /*****************************************************************************
 * filename	: InitAirVideoProcess
 * function	: video encode pthread and communication object creat
 * author	version		date		note
 * feller	1.0		20150728	create         
 ******************************************************************************/
Int InitAirVideoProcess( Args *args )
{
    Uns                 initMask            = 0;
    Int                 status              = EXIT_SUCCESS;
    Pause_Attrs         pAttrs              = Pause_Attrs_DEFAULT;
    Rendezvous_Attrs    rzvAttrs            = Rendezvous_Attrs_DEFAULT;
    Fifo_Attrs          fAttrs              = Fifo_Attrs_DEFAULT;
    Rendezvous_Handle   hRendezvousCapStd   = NULL;
    Rendezvous_Handle   hRendezvousInit     = NULL;
    Rendezvous_Handle   hRendezvousWriter   = NULL;
    Rendezvous_Handle   hRendezvousCleanup  = NULL;
    Pause_Handle        hPauseProcess       = NULL;

    struct sched_param  schedParam;
    pthread_t           captureThread;
    pthread_t           writerThread;
    pthread_t           videoThread;
    CaptureEnv          captureEnv;
    WriterEnv           writerEnv;
    VideoEnv            videoEnv;
    CtrlEnv             ctrlEnv;
    Int                 numThreads = 4; /* Determine the number of threads needing synchronization */
    pthread_attr_t      attr;
    Void               *ret;
   // Bool                stopped;
	
    printf("****************SinoMartin Ari Station Encoder started****************\n");
    /* Initialize the mutex which protects the global data */
    pthread_mutex_init(&gbl.mutex, NULL);
    /* Zero out the thread environments */
    Dmai_clear(captureEnv);
    Dmai_clear(writerEnv);
    Dmai_clear(videoEnv);
    Dmai_clear(ctrlEnv);

    /* Initialize Codec Engine runtime */
    CERuntime_init();

    /* Initialize Davinci Multimedia Application Interface */
    Dmai_init();

    initMask |= LOGSINITIALIZED;

    
    /* Create the Pause object */
    hPauseProcess = Pause_create(&pAttrs);

    if (hPauseProcess == NULL) {
        ERR("******Encoder Failed to create Pause object******\n");
        cleanup(EXIT_FAILURE);
    }

    /* Create the objects which synchronizes the thread init and cleanup */
    hRendezvousCapStd  = Rendezvous_create(2, &rzvAttrs);
    hRendezvousInit = Rendezvous_create(numThreads, &rzvAttrs);
    hRendezvousCleanup = Rendezvous_create(numThreads, &rzvAttrs);
    hRendezvousWriter = Rendezvous_create(2, &rzvAttrs);

    if (hRendezvousCapStd  == NULL || hRendezvousInit == NULL || 
        hRendezvousCleanup == NULL || hRendezvousWriter == NULL) {
        ERR("******Encoder Failed to create Rendezvous objects******\\n");
        cleanup(EXIT_FAILURE);
    }

  
    /* Initialize the thread attributes */
    if (pthread_attr_init(&attr)) {
        ERR("******Encoder Failed to initialize thread attrs******\\n");
        cleanup(EXIT_FAILURE);
    }

    /* Force the thread to use custom scheduling attributes */
    if (pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED)) {
        ERR("******Encoder Failed to set schedule inheritance attribute******\\n");
        cleanup(EXIT_FAILURE);
    }

    /* Set the thread to be fifo real time scheduled */
    if (pthread_attr_setschedpolicy(&attr, SCHED_FIFO)) {
        ERR("******Encoder Failed to set FIFO scheduling policy******\\n");
        cleanup(EXIT_FAILURE);
    }

    /* Create the video threads if a file name is supplied */
    if (args->videoFile) {
        /* Create the capture fifos */
        captureEnv.hInFifo = Fifo_create(&fAttrs);
        captureEnv.hOutFifo = Fifo_create(&fAttrs);

        if (captureEnv.hInFifo == NULL || captureEnv.hOutFifo == NULL) {
            ERR("******Encoder Failed to open display fifos******\\n");
            cleanup(EXIT_FAILURE);
        }

        /* Create the capture thread */
        captureEnv.hRendezvousInit    = hRendezvousInit;
        captureEnv.hRendezvousCapStd  = hRendezvousCapStd;
        captureEnv.hRendezvousCleanup = hRendezvousCleanup;
        captureEnv.hPauseProcess      = hPauseProcess;
        captureEnv.videoStd           = args->videoStd;
        captureEnv.videoInput         = args->videoInput;
        captureEnv.imageWidth         = args->imageWidth;
        captureEnv.imageHeight        = args->imageHeight;
        captureEnv.previewDisabled    = args->previewDisabled;

        if (pthread_create(&captureThread, NULL, captureThrFxn, &captureEnv)) {
            ERR("******Encoder Failed to create capture thread******\\n");
            cleanup(EXIT_FAILURE);
        }

        initMask |= CAPTURETHREADCREATED;

        /*
         * Once the capture thread has detected the video standard, make it
         * available to other threads. The capture thread will set the
         * resolution of the buffer to encode in the environment (derived
         * from the video standard if the user hasn't passed a resolution).
         */
        Rendezvous_meet(hRendezvousCapStd);

        /* Create the writer fifos */
        writerEnv.hInFifo = Fifo_create(&fAttrs);
        writerEnv.hOutFifo = Fifo_create(&fAttrs);

        if (writerEnv.hInFifo == NULL || writerEnv.hOutFifo == NULL) {
            ERR("******Encoder Failed to open display fifos******\\n");
            cleanup(EXIT_FAILURE);
        }

        /* Set the video thread priority */
        schedParam.sched_priority = VIDEO_THREAD_PRIORITY;
        if (pthread_attr_setschedparam(&attr, &schedParam)) {
            ERR("******Encoder Failed to set scheduler parameters******\\n");
            cleanup(EXIT_FAILURE);
        }

        /* Create the video thread */
        videoEnv.hRendezvousInit    = hRendezvousInit;
        videoEnv.hRendezvousCleanup = hRendezvousCleanup;
        videoEnv.hRendezvousWriter  = hRendezvousWriter;
        videoEnv.hPauseProcess      = hPauseProcess;
        videoEnv.hCaptureOutFifo    = captureEnv.hOutFifo;
        videoEnv.hCaptureInFifo     = captureEnv.hInFifo;
        videoEnv.hWriterOutFifo     = writerEnv.hOutFifo;
        videoEnv.hWriterInFifo      = writerEnv.hInFifo;
        videoEnv.videoEncoder       = args->videoEncoder->codecName;
        videoEnv.params             = args->videoEncoder->params;
        videoEnv.dynParams          = args->videoEncoder->dynParams;
        videoEnv.videoBitRate       = args->videoBitRate;
        videoEnv.imageWidth         = captureEnv.imageWidth;
        videoEnv.imageHeight        = captureEnv.imageHeight;
        videoEnv.engineName         = engine->engineName;
        if (args->videoStd == VideoStd_D1_PAL) {
            videoEnv.videoFrameRate     = 25000;
        } else {
            videoEnv.videoFrameRate     = 30000;
        }

        if (pthread_create(&videoThread, &attr, videoThrFxn, &videoEnv)) {
            ERR("******Encoder Failed to create video thread******\\n");
            cleanup(EXIT_FAILURE);
        }

        initMask |= VIDEOTHREADCREATED;

        /*
         * Wait for the codec to be created in the video thread before
         * launching the writer thread (otherwise we don't know which size
         * of buffers to use).
         */
        Rendezvous_meet(hRendezvousWriter);

        /* Create the writer thread */
        writerEnv.hRendezvousInit    = hRendezvousInit;
        writerEnv.hRendezvousCleanup = hRendezvousCleanup;
        writerEnv.hPauseProcess      = hPauseProcess;
        writerEnv.videoFile          = args->videoFile;
        writerEnv.outBufSize         = videoEnv.outBufSize;
        writerEnv.writeDisabled      = args->writeDisabled;

        if (pthread_create(&writerThread, NULL, writerThrFxn, &writerEnv)) {
            ERR("******Encoder Failed to create writer thread******\n");
            cleanup(EXIT_FAILURE);
        }

        initMask |= WRITERTHREADCREATED;

    }




    /* Main thread becomes the control thread */
    ctrlEnv.hRendezvousInit    = hRendezvousInit;
    ctrlEnv.hRendezvousCleanup = hRendezvousCleanup;
    ctrlEnv.hPauseProcess      = hPauseProcess;
    ctrlEnv.keyboard           = args->keyboard;
    ctrlEnv.time               = args->time;
    ctrlEnv.engineName         = engine->engineName;
    ctrlEnv.osd                = args->osd;

    ret = ctrlThrFxn(&ctrlEnv);

    if (ret == THREAD_FAILURE) {
        status = EXIT_FAILURE;
    }

cleanup:

   

    /* Make sure the other threads aren't waiting for init to complete */
    if (hRendezvousCapStd) Rendezvous_force(hRendezvousCapStd);
    if (hRendezvousWriter) Rendezvous_force(hRendezvousWriter);
    if (hRendezvousInit) Rendezvous_force(hRendezvousInit);
    if (hPauseProcess) Pause_off(hPauseProcess);


    if (initMask & VIDEOTHREADCREATED) {
        if (pthread_join(videoThread, &ret) == 0) {
            if (ret == THREAD_FAILURE) {
                status = EXIT_FAILURE;
            }
        }
    }

    if (initMask & WRITERTHREADCREATED) {
        if (pthread_join(writerThread, &ret) == 0) {
            if (ret == THREAD_FAILURE) {
                status = EXIT_FAILURE;
            }
        }
    }

    if (writerEnv.hOutFifo) {
        Fifo_delete(writerEnv.hOutFifo);
    }

    if (writerEnv.hInFifo) {
        Fifo_delete(writerEnv.hInFifo);
    }

    if (initMask & CAPTURETHREADCREATED) {
        if (pthread_join(captureThread, &ret) == 0) {
            if (ret == THREAD_FAILURE) {
                status = EXIT_FAILURE;
            }
        }
    }

    if (captureEnv.hOutFifo) {
        Fifo_delete(captureEnv.hOutFifo);
    }

    if (captureEnv.hInFifo) {
        Fifo_delete(captureEnv.hInFifo);
    }

    if (hRendezvousCleanup) {
        Rendezvous_delete(hRendezvousCleanup);
    }

    if (hRendezvousInit) {
        Rendezvous_delete(hRendezvousInit);
    }

    if (hPauseProcess) {
        Pause_delete(hPauseProcess);
    }
 
    /* 
     * In the past, there were instances where we have seen system memory
     * continually reduces by 28 bytes at a time whenever there are file 
     * reads or file writes. This is for the application to recapture that
     * memory (SDOCM00054899)
     */
    system("sync");
    system("echo 3 > /proc/sys/vm/drop_caches");


    pthread_mutex_destroy(&gbl.mutex);

    exit(status);
}



 /*****************************************************************************
 * filename	: main
 * function	: main
 * author	version		date		note
 * feller	1.0		20150728	create         
 ******************************************************************************/

Int main(Int argc, Char *argv[])
{
    Args args = DEFAULT_ARGS;
    Int status = EXIT_SUCCESS;	

	Int	iReturn;	
	
	iReturn = ushell_init();
	if( 0 == iReturn )
	{
		printf( "\nushell_init ok (~!~)\n" );
	}
	else
	{
		printf( "\nushell_init error\n" );
		return 0;
	}
#if 0	
    //Get the Air or Ground Station flag
    GetAirGroundStationFlag();

	
    /* Parse the arguments given to the app and set the app environment */    
    parseArgs(argc, argv, &args);
    /* Validate arguments */
    if (validateArgs(&args) == FAILURE) {
        cleanup(EXIT_FAILURE);
    }
	
    printf("**********SinoMartin Ari_Station Encoder started.**********\n");
    


    /* Set the priority of this whole process to max (requires root) */
    setpriority(PRIO_PROCESS, 0, -20);
    //mmap the physical address to virtual address
    status = InitMmapAddress(  );
    if( 0 != status )
    {
        ERR("******Encoder Failed InitMmapAddress******\n");
        cleanup(status);
    }
    
    //Initialize FPGA configuration and release reset 
    InitFPGA( AIR_STATION );

    //Initialize AD9363 transiver and release reset
    InitAD9363( AIR_STATION );
    
    //release the control of AD9363


    //Initialize the air station video process,include capture/encode/write pthread
#endif
    //status = InitAirVideoProcess( &args );

	while(1)
	{
		sleep(1);
	}
cleanup:    
	
    return 0;
}
 
