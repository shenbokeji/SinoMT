/*********************************************************************
* 版权所有 (C)2007, 深圳市中兴通讯股份有限公司。
* 
* 文件名称： ErrCode.h
* 文件标识： 
* 内容摘要：Boot版本的错误码定义
* 其它说明： 
* 当前版本： 1.0
* 作    者：        zhanglr
* 完成日期： 2012-06-20
**********************************************************************/
#ifndef BSPERRCODE_H
#define BSPERRCODE_H

/*--------------------------------------------------------------------------*/
#define BSP_ERROR_INVALID_PARA              (0xff000001)
#define BSP_ERROR_NULL_POINTER              (0xff000002)
#define BSP_ERROR_OUT_OF_RANGE            (0xff000003)
#define BSP_ERROR_FUNC_NOT_SUPPORT     (0xff000004)
#define BSP_ERROR_ADAPTED                      (0xff000005)
#define BSP_ERROR_BORDER_CUT                (0xff000006)
#define BSP_ERROR_DEVICE_NOT_EXIST      (0xff000007)
#define BSP_ERROR_DEVICE_NOT_READY     (0xff000008)
#define BSP_ERROR_MEM_MALLOC               (0xff000009)
#define BSP_ERROR_GET_CTRL                     (0xff00000a)
#define BSP_ERROR_HW_MISMATCH             (0xff00000b)
#define BSP_ERROR_OUT_OF_TIME               (0xff00000c)
#define BSP_ERROR_LINK_DOWN                  (0xff00000d)
#define BSP_ERROR_CRC                              (0xff00000e)

#define BSP_OK                                   (0x0)
#define BSP_COMMON                          (0x0)
#define ERROR_BASE_BSP                    (0x1100)     /* BSP错误码基值 */ 

/* BSP公用错误码基值 */
#define ERROR_BASE_BSP_COMMON               ((ERROR_BASE_BSP + BSP_COMMON)<<16)

/************************************************************************ 
                                               BOOT错误码结构定义

      ErrCode:       ERROR_BASE_BSP_COMMON  +   DEVICE(MODULE)_ERR_CODE  +  SPECIFIC_ERR_CODE
                        |                                   |      |                                   |     |                         |
                        |--------16bit----------|      |---------8bit----------|    |-------8bit----- |    
                        |                                   |      |                                    |    |                         |
*************************************************************************/


/************************************************************************ 
                                               模块错误基码定义
*************************************************************************/
#define  DEVICE_SPI_HOST                                      (0x01)
#define  DEVICE_SPI_FLASH                                    (0x02)





/************************************************************************ 
                                    单板各个模块的错误码宏定义
*************************************************************************/
/* SPI HOST错误码 基址*/                            
#define BSP_ERROR_BASE_DEVICE_SPI_HOST                     (ERROR_BASE_BSP_COMMON + (DEVICE_SPI_HOST<<8) )
#define BSP_ERROR_SPI_HOST_FIND_ID                              (BSP_ERROR_BASE_DEVICE_SPI_HOST + 0x01 )
#define BSP_ERROR_SPI_HOST_DRIVER_NOT_REGISTER    (BSP_ERROR_BASE_DEVICE_SPI_HOST + 0x02 )
#define BSP_ERROR_SPI_HOST_CLAIM_BUS                         (BSP_ERROR_BASE_DEVICE_SPI_FLASH + 0x03)


/* SPI FLASH错误码 基址*/                            
#define BSP_ERROR_BASE_DEVICE_SPI_FLASH                   (ERROR_BASE_BSP_COMMON + (DEVICE_SPI_FLASH<<8) )
#define BSP_ERROR_SPI_FLASH_SET_SLAVE                        (BSP_ERROR_BASE_DEVICE_SPI_FLASH + 0x01)
#define BSP_ERROR_SPI_FLASH_PROBE                                (BSP_ERROR_BASE_DEVICE_SPI_FLASH + 0x02)
#define BSP_ERROR_SPI_FLASH_MANUFACTURER_ID               (BSP_ERROR_BASE_DEVICE_SPI_FLASH + 0x03)
#define BSP_ERROR_SPI_FLASH_CMD                                     (BSP_ERROR_BASE_DEVICE_SPI_FLASH + 0x04)
#define BSP_ERROR_SPI_FLASH_WRITE_IN_PROGRESS       (BSP_ERROR_BASE_DEVICE_SPI_FLASH + 0x05)
#define BSP_ERROR_SPI_FLASH_ERASE_ADDR_LENGTH       (BSP_ERROR_BASE_DEVICE_SPI_FLASH + 0x06)
#define BSP_ERROR_SPI_FLASH_ADDR_LENGTH                     (BSP_ERROR_BASE_DEVICE_SPI_FLASH + 0x07)
#define BSP_ERROR_SPI_FLASH_ERASE_IN_PROTECT            (BSP_ERROR_BASE_DEVICE_SPI_FLASH + 0x08)
#define BSP_ERROR_SPI_FLASH_GET_HANDLE                       (BSP_ERROR_BASE_DEVICE_SPI_FLASH + 0x09)


#endif /* BSPERRCODE_H */

