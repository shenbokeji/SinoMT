/* --------------------------------------------------------------------------
  FILE      : spi.c
  PROJECT   : TI Booting and Flashing Utilities
  AUTHOR    : Daniel Allred
  DESC      : Generic SPI driver file for common SPI peripheral
-------------------------------------------------------------------------- */

// General type include
#include "tistdtypes.h"

// Device specific CSL
#include "device.h"

// Util functions
#include "util.h"

// This module's header file 
#include "spi.h"


/************************************************************
* Explicit External Declarations                            *
************************************************************/


/************************************************************
* Local Macro Declarations                                  *
************************************************************/

#define DELAY_AFTER_CS_DN     (4700)
#define DELAY_BEFORE_CS_UP    (3800)
#define DELAY_AFTER_CS_UP     (9400)

/************************************************************
* Local Function Declarations                               *
************************************************************/

static Uint32 LOCAL_setupMode(SPI_Config *config);

/************************************************************
* Local Variable Definitions                                *
************************************************************/


/************************************************************
* Global Variable Definitions                               *
************************************************************/


SPI_InfoObj gSPIInfo;

/************************************************************
* Global Function Definitions                               *
************************************************************/

// Initialze NAND interface and find the details of the NAND used
void SPI_open(Uint32 spiPeripheralNum, SPI_Role role, SPI_Mode mode, SPI_Config *config)
{
  DEVICE_SPIRegs *SPI;
 // SPI_InfoHandle hSPIInfo;
  
  //hSPIInfo = (SPI_InfoHandle) &gSPIInfo;


  // Assign the correct register base
  //hSPIInfo->peripheralNum = spiPeripheralNum;  
 // hSPIInfo->regs = (void *) ((Uint32)SPI0);
  SPI = (DEVICE_SPIRegs *) ((void *) ((Uint32)SPI0));
  
  // Reset the SPI
  SPI_reset();

  LOCAL_setupMode(config);

  // Enable SPI
  SPI->SPIGCR1 |= ( 1 << 24 );
  
}

// Routine to reset the SPI device
Uint32 SPI_reset()
{
  DEVICE_SPIRegs *SPI = (DEVICE_SPIRegs *) ((void *) ((Uint32)SPI0));

  // Put the SPI in reset
  SPI->SPIGCR0 = 0x00;
  UTIL_waitLoop( 1000 );

  // Release SPI from reset
  SPI->SPIGCR0 = 0x01;

  return E_PASS;
}


Uint32 SPI_xferOneChar(Uint32 dataOut)
{
  Uint32 spiflg, spibuf;
  DEVICE_SPIRegs *SPI = (DEVICE_SPIRegs *) ((void *) ((Uint32)SPI0));

//DEBUG_printString("SPI_xferOneChar\r\n");
  //DEBUG_printHexInt(dataOut);
  //DEBUG_printString(".\r\n");
 
  //DEBUG_printHexInt(SPI->SPIFLG);
  //DEBUG_printString(".\r\n");

   while (!(SPI->SPIFLG & 0x00000200u));
  // Write output data
  SPI->SPIDAT0 = 0x04020000|(dataOut & 0xff);
   //SPI->SPIDAT0 = dataOut & 0xff;
  //DEBUG_printHexInt(SPI->SPIFLG);
  //DEBUG_printString(".\r\n");
  //DEBUG_printHexInt(SPI->SPIEMU);
  //DEBUG_printString(".\r\n");
  while (!(SPI->SPIFLG & SPI_RXINTFLAG));
  //DEBUG_printHexInt(SPI->SPIFLG);
  //DEBUG_printString(".\r\n");
  //DEBUG_printHexInt(SPI->SPIEMU);
  //DEBUG_printString(".\r\n");
  return (Uint8)(SPI->SPIBUF);
}

/*
Uint32 SPI_xferOneChar(Uint32 dataOut)
{
  Uint32 spiflg, spibuf;
  DEVICE_SPIRegs *SPI = (DEVICE_SPIRegs *)SPI0;

  Uint32 mask = 0xff;

  // Write output data
  SPI->SPIDAT0 = dataOut & mask;

  do 
  { 
    spiflg = SPI->SPIFLG;
    if (spiflg & SPI_RXINTFLAG)
    {
      spibuf = (SPI->SPIBUF);
      break;
    }
    if (spiflg & SPI_OVRNINTFLG)
    {
      spibuf = (SPI->SPIBUF);
      SPI->SPIFLG &= SPI_OVRNINTFLG;
      continue;
    }
    if (spiflg & SPI_BITERRFLG)
    {                      
      spibuf = 0x0;
      break;
    }
  } 
  while (TRUE);

  return (spibuf & mask);
}
*/
void SPI_enableCS()
{
	DEVICE_GPIORegs *GPIOp = (DEVICE_GPIORegs *)GPIO;
	GPIOp->DIR01 &=0xFDFFFFFF;
	GPIOp->CLRDATA01 =0x02000000;
}

void SPI_disableCS()
{
	DEVICE_GPIORegs *GPIOp = (DEVICE_GPIORegs *)GPIO;
	GPIOp->DIR01 &=0xFDFFFFFF;
	GPIOp->SETDATA01 =0x02000000;
}


/************************************************************
* Local Function Definitions                                *
************************************************************/

static Uint32 LOCAL_setupMode(SPI_Config *config)
{
  DEVICE_SPIRegs *SPI = (DEVICE_SPIRegs *) ((void *) ((Uint32)SPI0));

    SPI->SPIGCR1 = 0
        | ( 0 << 24 )
        | ( 0 << 16 )
        | ( 1 << 1 )
        | ( 1 << 0 );

      SPI->SPIPC0 =  0
        | ( 1 << 11 )   // DI
        | ( 1 << 10 )   // DO
        | ( 1 << 9 )    // CLK
        | ( 1 << 0 );   // CS0

  SPI->SPIFMT[0] = 0x0;

    SPI->SPIFMT[0] |= ( 1 << 17 );   // Polarity
    SPI->SPIFMT[0] |= ( 0 << 16 );   // Phase
  SPI->SPIFMT[0] |= 8;
  SPI->SPIFMT[0] |= ((config->prescalar & 0xFF) << 8);

  // CSHOLD off, FMT[0] used  
  SPI->SPIDAT1 = 0x00;

  // All chip selects go high when no transfer
  //SPI->SPIDEF = 0x01;
SPI->SPIDEF = 0x03;
  SPI->SPIDELAY = 0x14140000;

  // Disable interrupts
  SPI->SPIINT = 0x00;
  SPI->SPILVL = 0x00;
  
  return E_PASS;
}



/***********************************************************
* End file                                                 *
***********************************************************/

/* --------------------------------------------------------------------------
  HISTORY
    v1.00 - DJA - 19-Aug-2008
      Initial release
-------------------------------------------------------------------------- */

