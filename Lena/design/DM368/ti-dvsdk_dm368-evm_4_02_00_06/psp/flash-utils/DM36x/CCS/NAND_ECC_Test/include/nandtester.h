/* --------------------------------------------------------------------------
  FILE        : nandwriter.h
  PURPOSE     : UBL main header file
  PROJECT     : DaVinci CCS NAND Flashing Utility
  AUTHOR      : Daniel Allred
  DATE        : 04-Jun-2007  
 
    HISTORY
 	     v1.00 - DJA - 04-Jun-2007
 	        Completion (with support for DM6441 and DM6441_LV)
 	     
 ----------------------------------------------------------------------------- */

#ifndef _NANDWRITER_H_
#define _NANDWRITER_H_

#include "tistdtypes.h"

// Prevent C++ name mangling
#ifdef __cplusplus
extern far "c" {
#endif

/**************************************************
* Global Macro Declarations                       *
**************************************************/

//UBL version number
#define UBL_VERSION_STRING "1.00"
#ifdef UBL_NAND
#define UBL_FLASH_TYPE "NAND"
#else
#define UBL_FLASH_TYPE "NOR"
#endif

// Define MagicNumber constants
#define MAGIC_NUMBER_VALID          (0xA1ACED00)

// Used by UBL when doing UART boot
#define UBL_UART_BOOT               (0xA1ACED00)		/* Safe boot mode */
#define UBL_NOR_BURN                (0xA1ACED11)		/* Download via UART & Burn NOR with UBL readable header and BIN data*/
#define UBL_NOR_ERASE               (0xA1ACED22)		/* Download via UART & Global erase the NOR Flash*/
#define UBL_NAND_BURN               (0xA1ACED33)		/* Download via UART & Burn NAND - Image is binary */
#define UBL_NAND_ERASE              (0xA1ACED44)		/* Download via UART & Global erase the NAND Flash*/

// Used by RBL when doing NAND boot
#define UBL_MAGIC_SAFE              (0xA1ACED00)		/* Safe boot mode */
#define UBL_MAGIC_DMA               (0xA1ACED11)		/* DMA boot mode */
#define UBL_MAGIC_IC                (0xA1ACED22)		/* I Cache boot mode */
#define UBL_MAGIC_FAST              (0xA1ACED33)		/* Fast EMIF boot mode */
#define UBL_MAGIC_DMA_IC            (0xA1ACED44)		/* DMA + ICache boot mode */
#define UBL_MAGIC_DMA_IC_FAST       (0xA1ACED55)		/* DMA + ICache + Fast EMIF boot mode */

// Used by UBL when doing UART boot, UBL Nor Boot, or NAND boot
//#define UBL_MAGIC_BIN_IMG           (0xA1ACED66)		/* Execute in place supported*/
#define UBL_MAGIC_BIN_IMG           (0xB1ACED22)		/* Execute in place supported*/

// Used by UBL when doing UART boot
#define UBL_MAGIC_NOR_RESTORE       (0xA1ACED77)		/* Download via UART & Restore NOR with binary data */
#define UBL_MAGIC_NOR_SREC_BURN     (0xA1ACED88)		/* Download via UART & Burn NOR with UBL readable header and SREC data*/
#define UBL_MAGIC_NOR_BIN_BURN      (0xA1ACED99)		/* Download via UART & Burn NOR with UBL readable header and BIN data*/
#define UBL_MAGIC_NOR_GLOBAL_ERASE  (0xA1ACEDAA)		/* Download via UART & Global erase the NOR Flash*/
#define UBL_MAGIC_NAND_SREC_BURN    (0xA1ACEDBB)		/* Download via UART & Burn NAND - Image is S-record*/
#define UBL_MAGIC_NAND_BIN_BURN     (0xA1ACEDCC)		/* Download via UART & Burn NAND - Image is binary */
#define UBL_MAGIC_NAND_GLOBAL_ERASE (0xA1ACEDDD)		/* Download via UART & Global erase the NAND Flash*/

// Define max UBL image size
#define UBL_IMAGE_SIZE              (0x00003800u)

// Define max app image size
#define APP_IMAGE_SIZE              (0x02000000u)

// Flags to indicate what is being written to NAND
#define NAND_UBL_WRITE	        (0)
#define NAND_APP_WRITE		      (1)

// Defines for which blocks are valid for writing UBL and APP data
#define START_UBL_BLOCK_NUM     (1)
#define END_UBL_BLOCK_NUM       (24)
#define START_APP_BLOCK_NUM     (26)
#define END_APP_BLOCK_NUM       (50)



/************************************************
* Global Typedef declarations                   *
************************************************/

typedef struct
{
  Uint32 magicNum;    // Expected magic number
  Uint32 entryPoint;  // Entry point of the user application
  Uint32 numPage;     // Number of pages where boot loader is stored
  Uint32 block;       // Starting block number where User boot loader is stored
  Uint32 page;        // Starting page number where boot-loader is stored
  Uint32 ldAddress;   // Starting RAM address where image is to copied - XIP Mode
}
NANDWRITER_Boot;


/******************************************************
* Global Function Declarations                        *
******************************************************/

void main(void);


/***********************************************************
* End file                                                 *
***********************************************************/

#ifdef __cplusplus
}
#endif

#endif //_NANDWRITER_H_

/* --------------------------------------------------------------------------
  HISTORY
    v1.00 - DJA - 02-Nov-2007
      Initial release
-------------------------------------------------------------------------- */




