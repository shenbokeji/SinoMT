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

#include "FPGA.h"
/*****************Global variable for FPGA configure Table*****************/
tFPGACfg g_tFPGACfg;
UInt32 g_uiRegBaseAddr = 0;//EMIF FPGA register base address
/*----------------------------------------------------------------------------
 * name			: GetFpgaReg
 * function 		: Get the fpga reg value 
 * input 			:
 * author	version		date		note
 * feller	1.0		20150805
 *----------------------------------------------------------------------------
*/
Int16 GetFpgaReg( const UInt32 * const uiAddr )
{
    Int16 wRead;


    EMIF_FPGA_READ_UINT16( uiAddr, wRead );


    return wRead;
}
 /*----------------------------------------------------------------------------
 * name			: SetFpgaReg
 * function 		: set the fpga reg 
 * input 			:
 * author	version		date		note
 * feller	1.0		20150805
 *----------------------------------------------------------------------------
*/
Int SetFpgaReg( const UInt32 * const uiAddr, const Int16 sValue )
{
     Int16 wRead;
    Int iErrCnt;
	
    while(1)
    {
        //set the reg 
        EMIF_FPGA_WRITE_UINT16( uiAddr ,sValue );

        //DelayUsec(50);
        
        //get the value again 
       EMIF_FPGA_READ_UINT16( uiAddr, wRead );
        // DelayUsec(50);

        if ( sValue != wRead )
        {
             iErrCnt++;

             if ( 3 == iErrCnt )
            {
                return FPGA_SET_REG_ERROR;/* ´íÎó */
            }
        }
        else
        {
            return LENA_OK;/* Õý³£ */
        }
    }

}

 /*----------------------------------------------------------------------------
 * name			: InitFPGAReg
 * function 		: initialize FPGA Reg addr and value
 * author	version		date		note
 * feller	1.0		20150729
 *----------------------------------------------------------------------------
*/
void InitFPGAReg( void )
{
    g_tFPGACfg.FPGAReg[GROUND_STATION][0].uiAddr = 0;
    g_tFPGACfg.FPGAReg[GROUND_STATION][1].iValue = 1;
    g_tFPGACfg.iValidLen[GROUND_STATION]  = 5;







    g_tFPGACfg.FPGAReg[AIR_STATION][0].uiAddr = 0;
    g_tFPGACfg.FPGAReg[AIR_STATION][1].iValue = 1;
    g_tFPGACfg.iValidLen[AIR_STATION]  = 8;


	
    return ;
}



 /*--------------------------------------------------------------------------
 * name			: InitFPGA
 * function 		: iAirorGround: air and ground station flag,0:ground 1:air,others:invalid
 * author	version		date		note
 * feller	1.0		20150729	
 *----------------------------------------------------------------------------
*/
void InitFPGA( const Int iAirorGround )
{
    Int iLoop;
    UInt uiTmp;
    Int iTmp;
    Int iLen;
	
    InitFPGAReg();
    iLen =  g_tFPGACfg.iValidLen[iAirorGround];
    for( iLoop = 0; iLoop < iLen; iLoop++ )
    {
        uiTmp = g_tFPGACfg.FPGAReg[iAirorGround][iLoop].uiAddr;
	 iTmp = g_tFPGACfg.FPGAReg[iAirorGround][iLoop].iValue;	

        //configure FPGA by EMIF
	 
    }
		
    return ;
}


