/*
 * This source file is ushellagent test file for the 'lena' on DM368 platform

 *****************************************************************************
 * Copyright(C) 2015, SinoMartin Corp.
 *----------------------------------------------------------------------------
 * filename	: main.c
 * function	: test ushellagent
 * author	version		date		note
 * feller	1.0		20150928	create         
 *----------------------------------------------------------------------------
*/
#include <stdio.h>
#include <unistd.h>
#include "ushellagent.h"

 /*****************************************************************************
 * filename	: ushellver
 * function	: ushellver
 * author	version		date		note
 * feller	1.0		20150928	create         
 ******************************************************************************/
int ushellver( int a , int b )
{
	printf( USHELLAGENT_VER );
    return 0;
}

 /*****************************************************************************
 * filename	: main
 * function	: main
 * author	version		date		note
 * feller	1.0		20150728	create         
 ******************************************************************************/

int main( void )
{ 
    int a = 0x35;
    int b = 0xfe;
	int iReturn = -1;
    ushellver( a, b ); 
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
    while(1)
    {
        usleep(1000000);
    }	
    return 0;
}
 
