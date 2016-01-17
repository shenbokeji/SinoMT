/* --------------------------------------------------------------------------
  FILE      : spi_mem.c
  PROJECT   : Catalog Boot-Loader and Flasher
  AUTHOR    : Daniel Allred
  DESC      : Generic SPI memory driver file
-------------------------------------------------------------------------- */

// General type include
#include "tistdtypes.h"

// Device specific CSL
#include "device.h"

// Util functions
#include "util.h"

// SPI module's header file 
#include "spi.h"

// This module's header file
#include "spi_mem.h"


/************************************************************
* Explicit External Declarations                            *
************************************************************/


/************************************************************
* Local Macro Declarations                                  *
************************************************************/


/************************************************************
* Local Function Declarations                               *
************************************************************/

static void LOCAL_xferAddrBytes(Uint32 spi_id, Uint32 addr);
static void LOCAL_readDataBytes(Uint32 spi_id, Uint32 byteCnt, Uint8 *data);
static void LOCAL_writeDataBytes(Uint32 spi_id, Uint32 byteCnt, Uint8 *data);

/************************************************************
* Local Variable Definitions                                *
************************************************************/


/************************************************************
* Global Variable Definitions                               *
************************************************************/
extern SPI_InfoObj gSPIInfo;
/************************************************************
* Global Function Definitions                               *
************************************************************/

// Initialze SPI interface
void SPI_MEM_open(Uint32 spiPeripheralNum)
{
  Uint8 spibuf;
  SPI_Config spiCfg;
  //SPI_MemInfoHandle hSPIMemInfo;
  
  //hSPIMemInfo = (SPI_MemInfoHandle) &gSPIInfo;

DEBUG_printString("Starting SPI_open...\r\n");
  // Transfer 8 bits at a time
  spiCfg.charLen = 8;
  
  // Use industry standard mode 3 (note that our SPI peripheral phase value is
  // inverted compared to all other industry players)
  spiCfg.phase = 0;
  spiCfg.polarity = 1;
  spiCfg.prescalar = 79;

  SPI_open(spiPeripheralNum,SPI_ROLE_MASTER,SPI_MODE_3PIN,&spiCfg);

  // Try to determine if this is flash or EEPROM by
  // issuing a DEVICE ID Read command

     Uint32  *p=(Uint32 *)SPI0;
	Uint32  i=0,j;
  for(i=0;i<0x8;i++)
	{
		for(j=0;j<0x4;j++)
			{
			DEBUG_printHexInt(*p++);
			DEBUG_printString("  ");
			}
		DEBUG_printString("\r\n");
	}
  

  // Assert chip select
  SPI_enableCS();

  // Send memory read command
  SPI_xferOneChar(SPI_MEM_CMD_JEDEC_ID);

  // Send dummy data, receive manufacture ID
  spibuf = SPI_xferOneChar(0x00);
  DEBUG_printHexInt(spibuf);
  DEBUG_printString(".\r\n");

  {
    //hSPIMemInfo->memType = SPI_MEM_TYPE_FLASH;
	DEBUG_printString("SPI_MEM_TYPE_FLASH.\r\n");
    
    // Send dummy data, receive devicd ID1
    spibuf = SPI_xferOneChar(0x00);
	DEBUG_printHexInt(spibuf);
	DEBUG_printString(".\r\n");

    // Send dummy data, receive manufacture ID
    spibuf = SPI_xferOneChar(0x00);
	  DEBUG_printHexInt(spibuf);
  DEBUG_printString(".\r\n");
  }


  SPI_disableCS();
}


// Routine to read data from SPI
Uint32 SPI_MEM_readBytes(Uint32 spi_id, Uint32 addr, Uint32 byteCnt, Uint8 *dest)
{
  SPI_enableCS();

  // Send memory read command
  SPI_xferOneChar(0x13);

  // Send the address bytes
  LOCAL_xferAddrBytes(0,addr);

  // Receive data bytes
  LOCAL_readDataBytes(0, byteCnt, dest);

  SPI_disableCS();

  return E_PASS;
}

/************************************************************
* Local Function Definitions                                *
************************************************************/

static void LOCAL_xferAddrBytes(Uint32 spi_id, Uint32 addr)
{
  Uint32 i;

  for (i=0; i<32; i+=8)
  {
    Uint8 addrByte = ((addr >> (32 - i - 8)) & 0xFF);
    SPI_xferOneChar(addrByte);
  }
}

static void LOCAL_writeDataBytes(Uint32 spi_id, Uint32 byteCnt, Uint8 *data)
{
  Uint32 i;

  for (i=0; i< byteCnt; i++)
  {
    SPI_xferOneChar(data[i]);
  }
}

static void LOCAL_readDataBytes(Uint32 spi_id, Uint32 byteCnt, Uint8 *data)
{
  Uint32 i;

  for (i=0; i< byteCnt; i++)
  {
    data[i] = SPI_xferOneChar(0x00);
  }
}


/***********************************************************
* End file                                                 *
***********************************************************/

/* --------------------------------------------------------------------------
  HISTORY
    v1.00 - DJA - 19-Aug-2008
      Initial release
-------------------------------------------------------------------------- */

