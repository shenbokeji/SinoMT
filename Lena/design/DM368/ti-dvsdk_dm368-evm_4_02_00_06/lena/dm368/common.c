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
 * output 	: iAirorGround: air and ground station flag,0:ground 1:air,others:invalid
 * author	version		date		note
 * feller	1.0		20150728	
 *----------------------------------------------------------------------------
*/
int GetAirGroundStationFlag()
{
    unsigned int uiAirorGround = 0xFFFFFFFF;
	
	uiAirorGround = GetGPIO( AIR_GROUND_GPIO );
	g_AirGround = uiAirorGround >> AIR_GROUND_GPIO ;
	printf( "uiAirorGround = %d\n", g_AirGround );
	
    return g_AirGround;
}

 /*--------------------------------------------------------------------------
 * function	: SetGPIO
 * input 	: iGPIOnumber;ivalue 
 * author	version		date		note
 * feller	1.0		20160101	
 *----------------------------------------------------------------------------
*/
int SetGPIO( const int iGPIOnumber, const char cvalue )
{
    int fid = -1;
    int iReturn = -1;

    fid = open( DEVICE_GPIO, O_RDWR, 0 );
    if( fid < 0 )
    {
	perror( "ERROR:open failed "DEVICE_GPIO"!\n" );
	return LENA_FALSE;
    }

    iReturn = write( fid, &cvalue, iGPIOnumber ); 
    close( fid );
    fid = NULL;
    return LENA_OK;
}

 /*--------------------------------------------------------------------------
 * function	: GetGPIO
 * input 	: iGPIOnumber
 * author	version		date		note
 * feller	1.0		20160101	
 *----------------------------------------------------------------------------
*/
int GetGPIO( const int iGPIOnumber )
{
    int fid = -1;
    int ivalue = -1;

    fid = open( DEVICE_GPIO, O_RDONLY, 0 );
    if( fid < 0 )
    {
	perror( "ERROR:open failed "DEVICE_GPIO"!\n" );
	return LENA_FALSE;
    }

    ivalue = read( fid, NULL, iGPIOnumber ); 
    close( fid );
    fid = NULL;
    return ivalue;
}
 /*--------------------------------------------------------------------------
 * name		: ResetFPGA
 * function	: reset fpga
 * intput 	: none
 * author	version		date		note
 * feller	1.0		20160101

 *----------------------------------------------------------------------------
*/
void ResetFPGA(void)
{
	SetGPIO( FPGA_RESET_GPIO, 1 ); 
	usleep(500);
	SetGPIO( FPGA_RESET_GPIO, 0 ); 
	usleep(500);
	SetGPIO( FPGA_RESET_GPIO, 1 ); 
	return ;
}
 /*--------------------------------------------------------------------------
 * name 	 : SetPwmLed
 * function  : set the pwm to light led
 * intput	 :  * iLenNum: led index
 *iLedMode:
 *    0: PWM
 *    1: static ,on or off mode
 *iPram: 
 *iLedMode==0 iFreqHz is (1/16)Hz unit
 *iLedMode==1 iFreqHz is 0 or 1, 0:on ,1: off ¡ê?for lena
 * author	 version	 date		 note
 * feller	 1.0	 20160313
 *----------------------------------------------------------------------------
 */
int SetPwmLed( const int iLedNum, const int iLedMode, const int iFreqHz )
{
	int fid = -1;
	int iReturn = -1;
 	int iLedStatus = 0;
	
	fid = open( PWM_DRIVER_NAME, O_RDWR, 0 );
	if( fid < 0 )
	{
  		perror( PWM_DRIVER_NAME" ERROR:open failed !\n" );
  		return LENA_FALSE;
	 }
	iLedStatus = ( iLedNum << 28) | iFreqHz ;
  	iReturn = ioctl( fid, iLedMode, iLedStatus );
	
 	close( fid );
	fid = NULL;
	return LENA_OK;
 }

/*--------------------------------------------------------------------------
 * name		 : LedNormal
 * function	 : light air or ground led, 1Hz
 * intput		 : none 
 * author	 version	 date		 note
 * feller	 1.0	 20160402
 *----------------------------------------------------------------------------
*/
void LedAlarm( const int iAirorGround, int* const piStatus )
{
	if( 0 == *piStatus )
	{
		printf("LedAlarm %d\n",iAirorGround);
		*piStatus = 1;
		if( iAirorGround ) 
		{
			SetPwmLed( AIR_LED_LG, PWM_MODE_RUN, ABNORMAL_LIGHT_FRQ );
			//turn off all the other led,reseved
			SetPwmLed( AIR_LED_LR, PWM_MODE_STATIC, LED_OFF );
			SetPwmLed( AIR_LED_RR, PWM_MODE_STATIC, LED_OFF );
			SetPwmLed( AIR_LED_RG, PWM_MODE_STATIC, LED_OFF );		
		}
		else
		{
			SetPwmLed( GROUND_LED_G, PWM_MODE_RUN, ABNORMAL_LIGHT_FRQ );
		}
	}
	return;
}
/*--------------------------------------------------------------------------
 * name		 : LedNormal
 * function	 : light air or ground led, 1Hz
 * intput		 : none 
 * author	 version	 date		 note
 * feller	 1.0	 20160402
 *----------------------------------------------------------------------------
*/
void LedNormal( const int iAirorGround, int* const piStatus )
{
	*piStatus = 0;
	if( iAirorGround ) 
	{
		SetPwmLed( AIR_LED_LG, PWM_MODE_RUN, NORMAL_LIGHT_FRQ );
		//turn off all the other led,reseved
		SetPwmLed( AIR_LED_LR, PWM_MODE_STATIC, LED_OFF );
		SetPwmLed( AIR_LED_RR, PWM_MODE_STATIC, LED_OFF );
		SetPwmLed( AIR_LED_RG, PWM_MODE_STATIC, LED_OFF );
	}
	else
	{
		SetPwmLed( GROUND_LED_G, PWM_MODE_RUN, NORMAL_LIGHT_FRQ );
	}	
	return ;
}

 
/*--------------------------------------------------------------------------
 * name		: CheckVideoConnect
 * function	: Check Video Connect fpga
 * intput 	: none
 * author	version		date		note
 * feller	1.0		20160101

 *----------------------------------------------------------------------------
*/ 
int CheckVideoConnect( const int iAirorGround )
{
 	int iReturn = LENA_FALSE;
 	unsigned char ucRdValue;
	unsigned char ucstatus;
	if( iAirorGround )
	{
		iReturn = GetADV7611Reg( ADV611_STATUS_ADDR, &ucRdValue );
		if( 0x53 == ucRdValue )
		{
			iReturn = LENA_OK;	
			printf( "Air CheckVideoConnect ok\n" );
		}
		else
		{
			perror( "Air CheckVideoConnect not ok\n" );

			Get7611Sta();
			iReturn = LENA_FALSE;
		}
	}
	else
	{
		iReturn = GetIT66121Reg( IT66121_STATUS_ADDR, &ucRdValue );
		ucstatus = ucRdValue & 0x50;
		if( 0x50 == ucstatus )
		{
			iReturn = LENA_OK;	
			printf( "Ground CheckVideoConnect ok\n" );
		}
		else
		{
			perror( "Ground CheckVideoConnect not ok\n" );
			Get66121Sta();
			iReturn = LENA_FALSE;
		}
	}

 	return iReturn;
 }
/*--------------------------------------------------------------------------
 * name		: InitStatus
 * function	: Init system status
 * intput 	: none
 * author	version		date		note
 * feller	1.0		20160402
 *----------------------------------------------------------------------------
*/ 
 void InitStatus(void)
 {
 	memset( &gtSystemStartStatus, 0x0, sizeof(tSystemStartStatus) );
 	return;
 }