/*
 * This source file has the main for the 'lena decode' on DM368 platform
 *****************************************************************************
 * Copyright(C) 2015, SinoMartin Corp.
 *----------------------------------------------------------------------------
 * filename	: main.c
 * function	: initilize the decode system and video decode
 * author	version		date		note
 * feller	1.0		20150731	create         
 *----------------------------------------------------------------------------
*/


#include "args.h"


/* Global variable declarations for this application */
GlobalData gbl = GBL_DATA_INIT;

 /*****************************************************************************
 * filename	: InitGroundVideoProcess
 * function	: video decoder pthread and communication object creat
 * author	version		date		note
 * feller	1.0		20150728	create         
 ******************************************************************************/
Int InitGroundVideoProcess( Args *args )
{

    Uns                     initMask            = 0;
    Int                     status              = EXIT_SUCCESS;
    Pause_Attrs             pAttrs              = Pause_Attrs_DEFAULT;
    Rendezvous_Attrs        rzvAttrs            = Rendezvous_Attrs_DEFAULT;
    Fifo_Attrs              fAttrs              = Fifo_Attrs_DEFAULT;
    UI_Attrs                uiAttrs;
    Rendezvous_Handle       hRendezvousInit     = NULL;
    Rendezvous_Handle       hRendezvousCleanup  = NULL;
    Rendezvous_Handle       hRendezvousLoop     = NULL;
    Rendezvous_Handle       hRendezvousLoader   = NULL;
    Pause_Handle            hPauseProcess       = NULL;
    Pause_Handle            hPausePrime         = NULL;
    UI_Handle               hUI                 = NULL;
    Int                     syncCnt             = 0;
    struct sched_param      schedParam;
    pthread_attr_t          attr;
    pthread_t               displayThread;
    pthread_t               videoThread;
    pthread_t               loaderThread;
    LoaderEnv               loaderEnv;
    DisplayEnv              displayEnv;
    VideoEnv                videoEnv;

    CtrlEnv                 ctrlEnv;
    Int                     numThreads = 4; /* Determine the number of threads needing synchronization */
    Void                   *ret;
   // Bool                    stopped;
	
    /* Initialize the mutex which protects the global data */
    pthread_mutex_init(&gbl.mutex, NULL);
	
    /* Zero out the thread environments */
    Dmai_clear(loaderEnv);
    Dmai_clear(displayEnv);
    Dmai_clear(videoEnv);
    Dmai_clear(ctrlEnv);


    /* Initialize Codec Engine runtime */
    CERuntime_init();

    /* Initialize Davinci Multimedia Application Interface */
    Dmai_init();

    initMask |= LOGSINITIALIZED;

    /* Create the user interface */
    uiAttrs.osd = args->osd;
    uiAttrs.videoStd = args->videoStd;
    
    /* Create the Pause objects */
    hPauseProcess = Pause_create(&pAttrs);
    hPausePrime = Pause_create(&pAttrs);

    if (hPauseProcess == NULL || hPausePrime == NULL) {
        ERR("Failed to create Pause objects\n");
        cleanup(EXIT_FAILURE);
    }

  
    /* Create the objects which synchronizes the thread init and cleanup */
    hRendezvousInit = Rendezvous_create(numThreads, &rzvAttrs);
    hRendezvousCleanup = Rendezvous_create(numThreads, &rzvAttrs);
    hRendezvousLoop = Rendezvous_create(syncCnt, &rzvAttrs);
    hRendezvousLoader = Rendezvous_create(2, &rzvAttrs);

    if (hRendezvousInit == NULL || hRendezvousCleanup == NULL ||
        hRendezvousLoop == NULL || hRendezvousLoader == NULL) {

        ERR("Failed to create Rendezvous objects\n");
        cleanup(EXIT_FAILURE);
    }

    /* Initialize the thread attributes */
    if (pthread_attr_init(&attr)) {
        ERR("Failed to initialize thread attrs\n");
        cleanup(EXIT_FAILURE);
    }

    /* Force the thread to use custom scheduling attributes */
    if (pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED)) {
        ERR("Failed to set schedule inheritance attribute\n");
        cleanup(EXIT_FAILURE);
    }

    /* Set the thread to be fifo real time scheduled */
    if (pthread_attr_setschedpolicy(&attr, SCHED_FIFO)) {
        ERR("Failed to set FIFO scheduling policy\n");
        cleanup(EXIT_FAILURE);
    }

    /* Create the video threads if a file name is supplied */
    if (args->videoFile) {
        /* Create the display fifos */
        displayEnv.hInFifo = Fifo_create(&fAttrs);
        displayEnv.hOutFifo = Fifo_create(&fAttrs);

        if (displayEnv.hInFifo == NULL || displayEnv.hOutFifo == NULL) {
            ERR("Failed to create display fifos\n");
            cleanup(EXIT_FAILURE);
        }

        /* Set the display thread priority */
        schedParam.sched_priority = DISPLAY_THREAD_PRIORITY;
        if (pthread_attr_setschedparam(&attr, &schedParam)) {
            ERR("Failed to set scheduler parameters\n");
            cleanup(EXIT_FAILURE);
        }

        /* Create the display thread */
        displayEnv.displayOutput      = args->displayOutput;        
        displayEnv.videoStd           = args->videoStd;
        displayEnv.hRendezvousInit    = hRendezvousInit;
        displayEnv.hRendezvousCleanup = hRendezvousCleanup;
        displayEnv.hPauseProcess      = hPauseProcess;
        displayEnv.hPausePrime        = hPausePrime;
        displayEnv.osd                = args->osd;

        if (pthread_create(&displayThread, &attr, displayThrFxn, &displayEnv)) {
            ERR("Failed to create display thread\n");
            cleanup(EXIT_FAILURE);
        }

        initMask |= DISPLAYTHREADCREATED;

        /* Set the video thread priority */
        schedParam.sched_priority = VIDEO_THREAD_PRIORITY;
        if (pthread_attr_setschedparam(&attr, &schedParam)) {
            ERR("Failed to set scheduler parameters\n");
            cleanup(EXIT_FAILURE);
        }

        /* Create the video thread */
        videoEnv.hRendezvousInit    = hRendezvousInit;
        videoEnv.hRendezvousCleanup = hRendezvousCleanup;
        videoEnv.hRendezvousLoop    = hRendezvousLoop;
        videoEnv.hRendezvousLoader  = hRendezvousLoader;
        videoEnv.hPauseProcess      = hPauseProcess;
        videoEnv.hPausePrime        = hPausePrime;
        videoEnv.hDisplayInFifo     = displayEnv.hInFifo;
        videoEnv.hDisplayOutFifo    = displayEnv.hOutFifo;
        videoEnv.videoFile          = args->videoFile;
        videoEnv.videoDecoder       = args->videoDecoder->codecName;
        videoEnv.params             = args->videoDecoder->params;
        videoEnv.dynParams          = args->videoDecoder->dynParams;
        videoEnv.loop               = args->loop;
        videoEnv.engineName         = engine->engineName;
        videoEnv.videoStd           = args->videoStd;        

        if (pthread_create(&videoThread, &attr, videoThrFxn, &videoEnv)) {
            ERR("Failed to create video thread\n");
            cleanup(EXIT_FAILURE);
        }

        initMask |= VIDEOTHREADCREATED;

        /*
         * Wait for the Loader to be created in the video thread before
         * launching the loader thread.
         */
        Rendezvous_meet(hRendezvousLoader);

        /* Set the loader thread priority */
        schedParam.sched_priority = LOADER_THREAD_PRIORITY;
        if (pthread_attr_setschedparam(&attr, &schedParam)) {
            ERR("Failed to set scheduler parameters\n");
            return -1;
        }

        /* Create the loader thread */
        loaderEnv.hRendezvousInit    = hRendezvousInit;
        loaderEnv.hRendezvousCleanup = hRendezvousCleanup;
        loaderEnv.loop               = args->loop;
        loaderEnv.hLoader            = videoEnv.hLoader;

        if (pthread_create(&loaderThread, &attr, loaderThrFxn, &loaderEnv)) {
            ERR("Failed to create loader thread\n");
            cleanup(EXIT_FAILURE);
        }

        initMask |= LOADERTHREADCREATED;
    }


    /* Main thread becomes the control thread */
    ctrlEnv.hRendezvousInit    = hRendezvousInit;
    ctrlEnv.hRendezvousCleanup = hRendezvousCleanup;
    ctrlEnv.hPauseProcess      = hPauseProcess;
    ctrlEnv.keyboard           = args->keyboard;
    ctrlEnv.time               = args->time;
    ctrlEnv.hUI                = hUI;
    ctrlEnv.engineName         = engine->engineName;
    ctrlEnv.osd                = args->osd;

    ret = ctrlThrFxn(&ctrlEnv);

    if (ret == THREAD_FAILURE) {
        status = EXIT_FAILURE;
    }

cleanup:

    /* Make sure the other threads aren't waiting for us */
    if (hRendezvousInit) Rendezvous_force(hRendezvousInit);
    if (hRendezvousLoader) Rendezvous_force(hRendezvousLoader);
    if (hRendezvousLoop) Rendezvous_force(hRendezvousLoop);
    if (hPauseProcess) Pause_off(hPauseProcess);
    if (hPausePrime) Pause_off(hPausePrime);

     /* Wait until the other threads terminate */

    if (initMask & LOADERTHREADCREATED) {
        if (pthread_join(loaderThread, &ret) == 0) {
            if (ret == THREAD_FAILURE) {
                status = EXIT_FAILURE;
            }
        }
    }

    if (initMask & VIDEOTHREADCREATED) {
        if (pthread_join(videoThread, &ret) == 0) {
            if (ret == THREAD_FAILURE) {
                status = EXIT_FAILURE;
            }
        }
    }

    if (initMask & DISPLAYTHREADCREATED) {
        if (pthread_join(displayThread, &ret) == 0) {
            if (ret == THREAD_FAILURE) {
                status = EXIT_FAILURE;
            }
        }
    }

    if (displayEnv.hOutFifo) {
        Fifo_delete(displayEnv.hOutFifo);
    }

    if (displayEnv.hInFifo) {
        Fifo_delete(displayEnv.hInFifo);
    }

    if (hRendezvousLoop) {
        Rendezvous_delete(hRendezvousLoop);
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

    if (hPausePrime) {
        Pause_delete(hPausePrime);
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
 * feller	1.0		20150731	create         
 ******************************************************************************/

Int main(Int argc, Char *argv[])
{
    Args                args                ;//= DEFAULT_ARGS;
    Int                 status              = EXIT_SUCCESS;	
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
	
    printf("**********SinoMartin Ground_Station decoder started.**********\n");
    

    /* Set the priority of this whole process to max (requires root) */
    setpriority(PRIO_PROCESS, 0, -20);
    //mmap the physical address to virtual address
    status = InitMmapAddress( );
    if( 0 != status )
    {
        ERR("******Encoder Failed InitMmapAddress******\n");
        cleanup(status);
    }

    //Initialize FPGA configuration and release reset 
    InitFPGA( GROUND_STATION );

    //Initialize AD9363 transiver and release reset
    InitAD9363( GROUND_STATION );
    
    //release the control of AD9363
#endif

    //Initialize the air station video process,include capture/encode/write pthread
    //status = InitAirVideoProcess( &args );
cleanup:    
	
    return 0;
}
 

