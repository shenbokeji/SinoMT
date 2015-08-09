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
UInt16 GetFpgaReg( const UInt32 * const uiAddr )
{
    UInt16 usRead;


    EMIF_FPGA_READ_UINT16( uiAddr, usRead );


    return usRead;
}
 /*----------------------------------------------------------------------------
 * name			: SetFpgaReg
 * function 		: set the fpga reg 
 * input 			:
 * author	version		date		note
 * feller	1.0		20150805
 *----------------------------------------------------------------------------
*/
Int SetFpgaReg( const UInt32 * const uiAddr, const UInt16 usValue )
{
    UInt16 usRead;
    Int iErrCnt;
	
    while(1)
    {
        //set the reg 
        EMIF_FPGA_WRITE_UINT16( uiAddr, usValue );

        //DelayUsec(50);
        
        //get the value again 
       EMIF_FPGA_READ_UINT16( uiAddr, usRead );
        // DelayUsec(50);

        if ( usValue != usRead )
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
    g_tFPGACfg.FPGAReg[GROUND_STATION][1].usValue = 1;
    g_tFPGACfg.iValidLen[GROUND_STATION]  = 0;







    g_tFPGACfg.FPGAReg[AIR_STATION][0].uiAddr = 0;
    g_tFPGACfg.FPGAReg[AIR_STATION][1].usValue = 1;
    g_tFPGACfg.iValidLen[AIR_STATION]  = 0;


	
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
    UInt32 *puiTmp;
    Int16 sTmp;
    Int iLen;
	
    InitFPGAReg();
    iLen =  g_tFPGACfg.iValidLen[iAirorGround];
    for( iLoop = 0; iLoop < iLen; iLoop++ )
    {
        puiTmp = ( UInt32 * )g_tFPGACfg.FPGAReg[iAirorGround][iLoop].uiAddr;
	 sTmp = g_tFPGACfg.FPGAReg[iAirorGround][iLoop].usValue;	

        //configure FPGA by EMIF
	 SetFpgaReg( puiTmp, sTmp );
    }
		
    return ;
}
