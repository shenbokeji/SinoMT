/*
 * This source file is FPGA file for the 'lena encode' on DM368 platform

 *****************************************************************************
 * Copyright(C) 2015, SinoMartin Corp.
 *----------------------------------------------------------------------------
 * filename	: FPGA.c
 * function	: initialize /write /read interface
 * author	version		date		note
 * feller	1.0		20150729	create         
 *----------------------------------------------------------------------------
*/
#include "UserDebug.h"
#include "common.h"
 /*****************************************************************************
 * filename	: ver
 * function	: print version info
 * author	version		date		note
 * feller	1.0		20150928	create         
 ******************************************************************************/
 int ver( void )
 {
	unsigned int uiFlag = 0XFFFFFFFF;
 	uiFlag = GetAirGroundStationFlag();

	printf( DSP_TIME"%s", uiFlag ? AIR_VERSION : GROUND_VERSION );
	return 0;
 }
/*****************************************************************************

 * filename	: wrgpio
 * function	: print version info
 * author	version		date		note
 * feller	1.0		20160101	create         
 ******************************************************************************/
 int wrgpio(  const int iGPIOnumber, const char cvalue  )
 {
	char cTmp;
	SetGPIO( iGPIOnumber, cvalue );
	cTmp = GetGPIO( iGPIOnumber );
	printf( "GPIO%d read back : %d\n", iGPIOnumber, (int)cTmp );
	return LENA_OK;
 }
/*****************************************************************************
 * filename	: rdgpio
 * function	: print version info
 * author	version		date		note
 * feller	1.0		20160101	create         
 ******************************************************************************/
 int rdgpio( const int iGPIOnumber )
 {
	char cTmp;
	cTmp = GetGPIO( iGPIOnumber );
	printf( "GPIO%d : %d\n", iGPIOnumber, (int)cTmp );
	return LENA_OK;
 }
/*----------------------------------------------------------------------------
 * name		: reset
 * function	: reboot the system
 * input 	: 
 * author	version		date		note
 * feller	1.0		20151126      
 *----------------------------------------------------------------------------
*/
int  reset( void )
{
	system("reboot");
	return LENA_OK;
}
 /*----------------------------------------------------------------------------
 * name		: wrfpga
 * function	: write FPGA register
 * input 	: uiAddr:FPGA regiser address 
 	          usValue: write value
 		  uiFlag: read back flag
 * author	version		date		note
 * feller	1.0		20151016      
 *----------------------------------------------------------------------------
*/
int wrfpga(  const UInt32 uiAddr, const UInt16 usValue, const UInt uiFlag ) 
{
    Int iReturn;
	UInt16 usRdValue;
	
	if( FPGA_ADDR_INVALID(uiAddr) )
	{
		printf( "******error: address is invalid******\n");		
		printf( "addr = %#X \n", (unsigned int)uiAddr );	
	}
	iReturn = SetFpgaReg( uiAddr, usValue );
	if( 0 != uiFlag ) 
	{
	    iReturn = GetFpgaReg( uiAddr, &usRdValue );
		printf( "addr = %#X value =%#X\n", (unsigned int)uiAddr, usRdValue );	
	}
	return LENA_OK;
}
/*----------------------------------------------------------------------------
 * name		: rdfpga
 * function	: read FPGA register
 * input 	: uiAddr:FPGA regiser start address 
 			  uiRdNum: read register number
 * author	version		date		note
 * feller	1.0		20151016      
 *----------------------------------------------------------------------------
*/
int rdfpga(  const UInt32 uiAddr, const UInt uiRdNum ) 
{
    Int ii;
	UInt32 uiAddrTmp;
    Int iReturn;
	UInt16 usRdValue;

	if( FPGA_ADDR_INVALID(uiAddr) )
	{
		printf( "******error: start address is invalid******\n"); 	
		printf( "addr = %#X \n", (unsigned int)uiAddr );	
	}

	
	uiAddrTmp = uiAddr;
	for ( ii = 0; ii < uiRdNum; ii++ )
	{
		if( FPGA_ADDR_INVALID(uiAddrTmp) )
		{
			break;
		}

	    iReturn = GetFpgaReg( uiAddrTmp, &usRdValue );
		printf( "addr = %#X value =%#X\n", (unsigned int)uiAddrTmp, usRdValue );	
		uiAddrTmp += 2;
	}

	return LENA_OK;
}
 
 /*--------------------------------------------------------------------------
 * name			: resetfpga
 * function		: reset fpga
 * intput 		: none
 * author	version		date		note
 * feller	1.0		20151216
 *----------------------------------------------------------------------------

*/
int resetfpga(void)
{
	int fid;
	int ihigh = 1;
	int ilow = 0;

	fid = open( DEVICE_GPIO, O_RDWR, 0 );
    	if( fid < 0 )
    	{
		printf( "ERROR:open failed "DEVICE_GPIO"!\n" );
		return LENA_FALSE;
     	}
	write( fid, &ihigh, FPGA_RESET_GPIO ); 
	usleep(500);
	write( fid, &ilow, FPGA_RESET_GPIO ); 
	usleep(500);
	write( fid, &ihigh, FPGA_RESET_GPIO ); 
	close(fid);
	fid = NULL;
	//ResetFPGA();
	return 0;
}

 /*--------------------------------------------------------------------------
 * name			: initfpga
 * function		: init FPGA register
 * intput 		: iAirorGround: air or ground flag
 * author	version		date		note
 * feller	1.0		20151222
 *----------------------------------------------------------------------------

*/
int initfpga( const int iAirorGround )
{
	int iReturn;
	if( iAirorGround )
	{
		iReturn = SetAD9363Reg( 0x006, 0x90 );
		iReturn = SetAD9363Reg( 0x007, 0xa0 );
	}
	InitFPGA( iAirorGround );
	if( iAirorGround )
	{
		iReturn = SetAD9363Reg( 0x3f4, 0x00 );
	}
	return 0;
}
/*----------------------------------------------------------------------------
 * name		: sendfile
 * function	: send local file to another side 
 * input 	: number: :send times
		  len:send data length
 * author	version		date		note
 * feller	1.0		20151208      
 *----------------------------------------------------------------------------
*/

int sendfile( const int number, size_t ilen ) 
{
	FILE *fid;
	int fidfpga;
	int fsenddev;
	tfpgadata tFPGADataTmp;
	unsigned int uiRet; 
	unsigned int uiLoop;

	void *ptrSend;
	size_t uimaplen;
	int ifilelen;
	struct stat fstatbuf;

	uimaplen = TRANS_ODD2EVEN( ilen );// for odd length, must even

	//open the video file

	fid = fopen( SEND_VIDEO_FILE, "rb" );
	if( (int)fid <= 0 )
	{
		printf( SEND_VIDEO_FILE"ERROR:open failed!\n" );
		return LENA_FALSE;
	}

	//open the mem file 
	fsenddev = open( MEM_FILENAME,O_RDWR | O_SYNC );
 	if( fsenddev > 0 )
	{
	        printf( MEM_FILENAME" open the success\n");
	}else
	{
		printf( MEM_FILENAME" open the failed\n");
		fclose(fid);
		fid = NULL;
		return LENA_FALSE;		
	}	
	//Map the continus address
	ptrSend = mmap( NULL,
                uimaplen,
                PROT_READ|PROT_WRITE,
                MAP_SHARED,
                fsenddev,
                SEND_PHY_ADDR);
	if( MAP_FAILED == ptrSend )
	{
		fclose(fid);
		fid = NULL;
		close(fsenddev);
		fsenddev = NULL;
		return LENA_FALSE;
	}
	
	//get the file length
	stat ( SEND_VIDEO_FILE, &fstatbuf );
	ifilelen = fstatbuf.st_size;
	if( ifilelen < ilen )
	{
		ilen = ifilelen;
	}
	
	uiRet = fread( ptrSend, 1, ilen, fid );
	if( uiRet != ilen )
	{
		printf( "warning : read data %d byte ,expect data %d byte \n", uiRet,ilen );
	}
	fclose(fid);
	fid = NULL;
	printf( "read file over\n" );
	//open the transfer channel
	fidfpga = open( DEVICE_FPGA, O_RDWR, 0 );
	if( fidfpga < 0 )
	{
		printf( DEVICE_FPGA"ERROR:open failed !\n" );
		close(fsenddev);
		fsenddev = NULL;
		munmap(ptrSend,uimaplen);
		return LENA_FALSE;
	}

	tFPGADataTmp.source_addr = (unsigned int)SEND_PHY_ADDR;
	tFPGADataTmp.byte_size = uimaplen;
	printf( "send start\n" );
	for( uiLoop = 0; uiLoop < number; uiLoop++ )
	{
		uiRet = ioctl( fidfpga, FPGA_DMA_SEND, &tFPGADataTmp );

		if( uiRet < 0 )
		{
			printf( " uiRet = %d \n", uiRet );
		}
		printf( "send the %d times \n", uiLoop + 1 );
		sleep(1);//sleep or not?
	}
	close(fsenddev);
	fsenddev = NULL;
	munmap(ptrSend,uimaplen);
	close(fidfpga);
	fidfpga = NULL;
	printf( "send over \n" );
	return 0;
} 


int sf( int number, int iflag ) 
{
	int fidfpga;
	int fsenddev;
	FILE *fid;
	tfpgadata tFPGADataTmp;
	unsigned int uiRet; 
	unsigned int uiLoop;
	void *ptrSend;
	struct stat fstatbuf;
	int ilen;
	size_t uimaplen;
	const char *cvideofile;


	cvideofile =  ( 0 == iflag ) ? SEND_VIDEO_FILE_384 : SEND_VIDEO_FILE_720P;

	printf( "send video file : %s\n", cvideofile );

	//open the video file
	fid = (FILE *)fopen( cvideofile, "rb" );
	if( (int)fid <= 0 )
	{
		printf( "ERROR:open failed %s \n", cvideofile );
		return LENA_FALSE;
	}
	//get the file length
	stat ( cvideofile, &fstatbuf );
	ilen = (int)fstatbuf.st_size;

	uimaplen = TRANS_ODD2EVEN( ilen );// for odd length, must even

	//open the mem file 
	fsenddev = open( MEM_FILENAME,O_RDWR | O_SYNC );

 	if( fsenddev < 0 )
	{
		printf( MEM_FILENAME" open the failed\n");
		fclose(fid);
		fid = NULL;
		return LENA_FALSE;
	}	
	//Map the continus address
	ptrSend = mmap( NULL,
                uimaplen,
                PROT_READ|PROT_WRITE,
                MAP_SHARED,
                fsenddev,
                SEND_PHY_ADDR);
	if( MAP_FAILED == ptrSend )
	{
		perror("mmap");
		fclose(fid);
		fid = NULL;
		close(fsenddev);
		fsenddev = NULL;
		return LENA_FALSE;
	}

	printf( "start read file\n" );
	//read the video file to continus address
	uiRet = fread( ptrSend, 1, ilen, fid );
	if( uiRet != ilen )
	{
		printf( "warning : read data %d byte ,expect data %d byte \n", uiRet,ilen );

	}
	fclose(fid);
	fid = NULL;
	printf( "read file over\n" );

	//open the transfer channel
	fidfpga = open( DEVICE_FPGA, O_RDWR, 0 );
	if( fidfpga < 0 )
	{
		printf( DEVICE_FPGA"ERROR:open failed !\n" );
		close(fsenddev);
		fsenddev = NULL;
		munmap(ptrSend,uimaplen);
		return LENA_FALSE;
	}

	tFPGADataTmp.source_addr = (unsigned int)SEND_PHY_ADDR;
	tFPGADataTmp.byte_size = uimaplen;
	printf( "send start\n" );
	for( uiLoop = 0; uiLoop < number; uiLoop++ )
	{
		uiRet = ioctl( fidfpga, FPGA_DMA_SEND, &tFPGADataTmp );
		if( uiRet < 0 )
		{
			printf( " uiRet = %d \n", uiRet );
		}
		printf( "send the %d times \n", uiLoop + 1 );
		sleep(2);//sleep or not?
	}

	munmap(ptrSend,uimaplen);	
	close(fsenddev);
	fsenddev = NULL;
	close(fidfpga);
	fidfpga = NULL;
	
	return 0;
} 
/*----------------------------------------------------------------------------
 * name		: receivefile
 * function	: send local file to another side 
 * input 	: ilen :data length
 * author	version		date		note
 * feller	1.0		20151208      
 *----------------------------------------------------------------------------
*/

int receivefile( const int ilen ) 
{
	int fidfpga;
	int receivedev;
	FILE *fid;
	void *ptrrece;
	tfpgadata tFPGADataTmp; 
	unsigned int uiRet; 
	size_t uirdlen;

	uirdlen = TRANS_ODD2EVEN( ilen );// for odd length, must even

	printf( "receive over ,start write file\n" );
	//open the mem device
	receivedev = open( MEM_FILENAME,O_RDWR|O_NDELAY );
 	if( receivedev <= 0 )
	{
		printf( MEM_FILENAME" ERROR:open failed !\n" );
		return -1;
	}
	ptrrece=mmap( NULL,
                uirdlen,
                PROT_READ|PROT_WRITE,
                MAP_SHARED,
                receivedev,
                RECEIVE_PHY_ADDR);
	if( MAP_FAILED == ptrrece )
	{
		close(receivedev);
		receivedev = NULL;
		return LENA_FALSE;
	}

	
	//open the transfer channel
	fidfpga = open( DEVICE_FPGA, O_RDONLY, 0 );

	if( fidfpga < 0 )
	{
		printf( DEVICE_FPGA"ERROR:open failed !\n" );
		return LENA_FALSE;
	}

	tFPGADataTmp.dst_addr = (unsigned int)RECEIVE_PHY_ADDR;

	tFPGADataTmp.byte_size = uirdlen;
	printf( "receive start\n" );
	{
		uiRet = ioctl( fidfpga, FPGA_DMA_RECV, &tFPGADataTmp );
		if( uiRet < 0 )
		{
			printf( " uiRet = %d \n", uiRet );
		}
	}	

   	close( fidfpga );
   	fid = NULL;

	fid = fopen( RECE_VIDEO_FILE, "wb"  );
	if( (int)fid <= 0 )
	{
		printf( RECE_VIDEO_FILE"ERROR:open failed !\n" );
		munmap( ptrrece,uirdlen ); 
		close(receivedev);
		receivedev = NULL;
		return LENA_FALSE;
	}
	uiRet = fwrite( ptrrece, 1, ilen, fid );
	if( uiRet != ilen )
	{
		printf( "write data %d byte ,expect data %d byte \n", uiRet,ilen );
	}

	fclose(fid);
	fid = NULL;

	munmap( ptrrece,uirdlen ); 
	close(receivedev);
	receivedev = NULL;
	printf( "receive over\n" );
	return 0;
} 
void rf( int ilen )
{
	printf( "receive file length %d", ilen );
	receivefile( ilen );
	return;
}
 /*----------------------------------------------------------------------------
  * name	 : wr9363
  * function : config AD9363 by spi4
  * input	 : 
  * author	 version	 date		 note
  * feller	 1.0	 20151007	   
  *----------------------------------------------------------------------------
 */
 
 int wr9363( const UInt32 uiAddr, const unsigned char ucValue, const UInt uiFlag  )
 {
    	Int iReturn;
	unsigned char  ucRdValue;
	
	if( AD9363_ADDR_INVALID(uiAddr) )
	{
		printf( "******error: address is invalid******\n");		
		printf( "addr = %#X \n", (unsigned int)uiAddr );	
	}
	
	iReturn = SetAD9363Reg( uiAddr, ucValue );
	if( 0 != uiFlag ) 
	{
	    	iReturn = GetAD9363Reg( uiAddr, &ucRdValue );
		printf( "addr = %#X value =%#X\n", (unsigned int)uiAddr, ucRdValue );	
	}
	return LENA_OK;	 
 }
 /*----------------------------------------------------------------------------
  * name	 : SPIWrite
  * function : the same as wr9363,for somebody is used to use it
  * input	 : 
  * author	 version	 date		 note
  * feller	 1.0	 20151119	
  *----------------------------------------------------------------------------
 */
 
 int SPIWrite( const UInt32 uiAddr, const unsigned char ucValue, const UInt uiFlag )

 {
 	wr9363( uiAddr, ucValue, uiFlag );
	return LENA_OK;
 }
 /*----------------------------------------------------------------------------
  * name	 : rd9363
  * function : read AD9363 by spi4
  * input	 : 
  * author	 version	 date		 note
  * feller	 1.0	 20151119	   
  *----------------------------------------------------------------------------
 */
 
 int rd9363( const UInt32 uiAddr, const UInt uiRdNum )
 {
  	Int ii;
  	UInt32 uiAddrTmp;
  	Int iReturn;
  	unsigned char ucRdValue = 0;
	
 	if( AD9363_ADDR_INVALID(uiAddr) )
  	{
	  	printf( "******error: start address is invalid******\n");   
	  	printf( "addr = %#X \n", (unsigned int)uiAddr );	  
  	}
   
  	uiAddrTmp = uiAddr;
  	for ( ii = 0; ii < uiRdNum; ii++ )
  	{
	  	if( AD9363_ADDR_INVALID(uiAddrTmp) )
	  	{
		  	break;
	  	}
 
	  	iReturn = GetAD9363Reg( uiAddrTmp, &ucRdValue );
	  	printf( "addr = %#X value =%#X\n", (unsigned int)uiAddrTmp, ucRdValue );   
		uiAddrTmp++;
  	}
	return LENA_OK;
 }



 /*--------------------------------------------------------------------------
 * name		: init9363
 * fucntion 	: initialize AD9363 interface
 * input 	: iAirorGround: air and ground station flag,0:ground 1:air,others:invalid
 * author	version		date		note
 * feller	1.0		20150728	
 *----------------------------------------------------------------------------
*/
int init9363( const int iAirorGround )
{
    InitAD9363( iAirorGround );
    return LENA_OK;
}



 /*----------------------------------------------------------------------------
  * name	 : wr66121
  * function : config AD9363 by spi4

  * input	 : 
  * author	 version	 date		 note
  * feller	 1.0	 20151007	   
  *----------------------------------------------------------------------------

 */
 
 int wr66121( const unsigned int uiAddr, const unsigned char ucValue, const UInt uiFlag  )
 {
    	Int iReturn;
	unsigned char  ucRdValue;
	
	if( IT66121_ADDR_INVALID(uiAddr) )
	{
		printf( "******error: address is invalid******\n");		
		printf( "addr = %#X \n", (unsigned int)uiAddr );
		return LENA_FALSE;
	}
	
	iReturn = SetIT66121Reg( uiAddr, ucValue );
	if( 0 != uiFlag ) 
	{
	    	iReturn = GetIT66121Reg( uiAddr, &ucRdValue );
		printf( "addr = %#X value =%#X\n", (unsigned int)uiAddr, ucRdValue );	
	}
	return LENA_OK;	 
 }

 /*----------------------------------------------------------------------------
  * name	: rd66121
  * function 	: read it66121 by I2C
  * input	 : 
  * author	 version	 date		 note
  * feller	 1.0		 20151229	   
  *----------------------------------------------------------------------------
 */
 
 int rd66121( const unsigned int uiAddr, const unsigned int uiRdNum )
 {
  	int ii;
  	unsigned int uiAddrTmp;
  	int iReturn;
  	unsigned char ucRdValue = 0;
	
 	if( IT66121_ADDR_INVALID(uiAddr) )
  	{
	  	printf( "******error: start address is invalid******\n");   
	  	printf( "addr = %#X \n", (unsigned int)uiAddr );
		return LENA_FALSE;	  
  	}
 
  
  	uiAddrTmp = uiAddr;
  	for ( ii = 0; ii < uiRdNum; ii++ )
  	{
	  	if( IT66121_ADDR_INVALID(uiAddrTmp) )
	  	{
		  	break;
	  	}
 
	  	iReturn = GetIT66121Reg( uiAddrTmp, &ucRdValue );
	  	printf( "addr = %#X value =%#X\n", (unsigned int)uiAddrTmp, ucRdValue );   
		uiAddrTmp++;
  	}
	return LENA_OK;
 }



 /*--------------------------------------------------------------------------
 * name		: attshow
 * function	: attenuation value show
 * input 	:none
 * author	version		date		note
 * feller	1.0		20151223
 *----------------------------------------------------------------------------
*/
int attshow( void )
{
	unsigned char ucValue[4] = {0};
	float fvalue[2] = {0.0f};
	int iReturn;

	iReturn = GetAD9363Reg( 0x073, &ucValue[0] );
	iReturn = GetAD9363Reg( 0x074, &ucValue[1] );
	iReturn = GetAD9363Reg( 0x075, &ucValue[2] );
	iReturn = GetAD9363Reg( 0x076, &ucValue[3] );

	fvalue[0] = (float)( ( ucValue[1] << 8 ) | ucValue[0] ) * 0.25f;// LSB FOR 0.25dB
 	fvalue[1] = (float)( ( ucValue[3] << 8 ) | ucValue[2] )* 0.25f;// LSB FOR 0.25dB

	printf( "TX 0 att :%f dB\n", fvalue[0] );
	printf( "TX 1 att :%f dB\n", fvalue[1] );
	return LENA_OK;
}
 /*--------------------------------------------------------------------------
 * name		: settxatt
 * function	: set tx attenuation
 * intput 	: chan:channel num; 0:channel 0 ,1:channel 1; 2 :both channel. other invalid
		value :attenuation value,0.25dB step, 0 is 0 dB
 * author	version		date		note
 * feller	1.0		20151223
 * note1   this function use some magic number ,you can find them is AD9363 datasheet
 * note2   can not use float input parameter because of ushell:(
 * note3   so we use 0.25/100dB unit for input parameter
 *----------------------------------------------------------------------------
*/
int settxatt( const unsigned int ichan, const int iattvalue )
{
	int iReturn;
	int itmp = 0;
	float ftmp;
	unsigned int uiAddrTmp[2];
	
	ftmp = (float)iattvalue / 100.0f;
	printf( "INFO: You want set the chan %d att :  %f dB\n", ichan, ftmp );
	
	if( AD9363_CHAN_INVALID(ichan) )
	{
		printf( "invalid channel number:  %d \n", ichan );	
		return LENA_FALSE;
	}

	if( ( iattvalue < 0 ) || ( iattvalue > 359*25 ) )// LSB FOR 0.25dB ,max register value 359
	{
		printf( "the tx att value should between 0 dB and 89.75 dB \n" );
		return LENA_FALSE;
	}

	iReturn = SetAD9363Reg( 0X014, 0X03 );

	itmp = (int)( iattvalue * 4 / 100 );// LSB FOR 0.25dB, debug command,excute speed is ignore, so we use "/"


	uiAddrTmp[0] = ( 0 == ichan ) ? 0x73 : 0x75;
	uiAddrTmp[1] = ( 0 == ichan ) ? 0x74 : 0x76;

	iReturn = SetAD9363Reg( uiAddrTmp[0], (unsigned char)( itmp & 0XFF ) );
	iReturn = SetAD9363Reg( uiAddrTmp[1], (unsigned char)( ( itmp >> 8 ) & 0X1 ) );

	iReturn = SetAD9363Reg( 0X014, 0X23 );
	iReturn = attshow();
	return iReturn;
}
 /*--------------------------------------------------------------------------
 * name			: physta
 * function		: wireless phyical layer KPI static
 * intput 		: iNum display times, per 3 second
 * author	version		date		note
 * feller	1.0		20151107
 *----------------------------------------------------------------------------
*/
 
char * sp[] = { "TB:", "FN:", "SFN:", "TBSIZE:", "BLER:", "BER:", "TO:", "Mbps" };	 
int iSFN[8] = { 0, 3, 4, 5, 6, 7, 8, 9 };
int iTBSize[2] = { 35160, 2728 };
char *sModemStr[] = { "16QAM" , "QPSK"};

int physta( const int iFlag )
{
	int ii;
	int jj;
	//int kk;
	unsigned int iFNtmp;
	float fBLERTmp;
	float fBERTmp;
	float fTO;

//	for( kk = 0; kk < iNum; kk++ )
	{
		printf( "\n****************************WIRELESS PHYSICAL LAYER KPI STATISTIC****************************\n");
		for( ii = 0; ii < 1; ii++ )
		{
			for( jj = 0; jj < 8; jj++ )
			{
				if( iFlag > 0 )
				{
					iFNtmp = ii;
					fBLERTmp = 0.001;
					fBERTmp = 0.0001;
					fTO = 35160 * 0.8 / 1000;
					printf( "%3.3s1\t%3.3s%d\t%4.4s%d\t", sp[0], sp[1], iFNtmp, sp[2], iSFN[jj]);
					printf( "%7.7s\t%5d\t%5.5s\t%5.5s%.3f\t", sp[3], iTBSize[0], sModemStr[0], sp[4], fBLERTmp );
					printf( "%4.4s%.4f\t%3.3s%6.3f%4.4s\n", sp[5], fBERTmp, sp[6], fTO, sp[7] );
				
					fBLERTmp = 0.002;
					fBERTmp = 0.0002;
					fTO = 2728 * 0.8 / 1000;
				
					printf( "%3.3s2\t%3.3s%d\t%4.4s%d\t", sp[0], sp[1], iFNtmp, sp[2], iSFN[jj]);
					printf( "%7.7s\t%5d\t%5.5s\t%5.5s%.3f\t", sp[3], iTBSize[1], sModemStr[1], sp[4], fBLERTmp );
					printf( "%4.4s%.4f\t%3.3s%6.3f%4.4s\n", sp[5], fBERTmp, sp[6], fTO, sp[7] );					
				}
				else if( 0 == iFlag )//print for static info
				{
					if( ( 0 == ii ) && ( jj < 2 ) )
					{	
						printf( "TB1\tFN:%d\tSFN:%d\tTBSIZE:%d\t16QAM\tBLER:0.001%%\tBER:0.0001%%\tTO:28.128\n", ii, iSFN[jj], iTBSize[0] );
						printf( "TB2\tFN:%d\tSFN:%d\tTBSIZE:%d\tQPSK\tBLER:0.001%%\tBER:0.0001%%\tTO:2.182\n", ii, iSFN[jj], iTBSize[1] );
					}
					else
					{
						printf( "TB1\tFN:%d\tSFN:%d\tTBSIZE:%d\t16QAM\tBLER:0.0%%\tBER:0.0%%\tTO:28.128\n", ii, iSFN[jj], iTBSize[0] );
						printf( "TB2\tFN:%d\tSFN:%d\tTBSIZE:%d\tQPSK\tBLER:0.0%%\tBER:0.0%%\tTO:2.182\n", ii, iSFN[jj], iTBSize[1] );
					}		
				}
			}
		}
		printf( "\n*********************************************************************************************\n");
		sleep( 1 );
	}
	return LENA_OK;
}


