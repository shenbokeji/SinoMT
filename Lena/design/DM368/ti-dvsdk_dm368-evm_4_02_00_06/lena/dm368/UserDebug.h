#ifndef _USERDEBUG_H_
#define _USERDEBUG_H_

#include "FPGA.h"
#include "AD9363.h"
#include "common.h"
#define TB_STREAM_NUM (2)
#define SFN_NUM (8)
	 
typedef struct SFNStruct {
	 unsigned int iSFN;
	 float fBLER;//block error ratio
	 float fBER;//bit error ratio
} tSFNInfo;
	 
typedef struct FRAMEKPIStruct {
	 unsigned int iFN;//frame number
	 tSFNInfo tSFN[ SFN_NUM ];//subframe number
	 unsigned int iTBSize;//transfer block
	 unsigned int iMod;//modem 
	 float fTO;//throught output
} tFRAMEKPI;
tFRAMEKPI tTBStream[ TB_STREAM_NUM ];

#endif