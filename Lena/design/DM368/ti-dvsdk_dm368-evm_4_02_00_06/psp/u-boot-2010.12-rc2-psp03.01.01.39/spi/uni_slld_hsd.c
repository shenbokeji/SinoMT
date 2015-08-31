/*
 * Copyright (C) 2012 ZTE Corporation, Inc.
 *
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <uni_slld_hsd.h>
#include <asm/arch/ErrCode.h>

T_SPI_HOST_DEVICE  g_spi_host[]=
{
    {"DRA624",SPI_HOST_DRA624_ID}
};

T_SPI_HOST_DEVICE_ACT *g_ptspihostdevice = NULL;
unsigned long g_ulspihostdriverregflag = 0;


/*****************************************************************************
*  函数名称   : SpiFlashNameToId
*  功能描述   : 将spi host name 转换成相应的id
*  读全局变量 : g_spi_host
*  写全局变量 : 无
*  输入参数   : ptTbl      - 表地址
             ulNums     - 表条目数
             pacspihostname:
*  输出参数   :              pulspihostid    - 指向存储spi host id 变量
*  返 回 值   : 0:标识成功；其它标识失败；
*  其它说明   : 
*----------------------------------------------------------------------------
* 历史记录(变更单, 责任人@修改日期, 操作说明)
*  $0000000(N/A), zhanglr@2012-6-20
*----------------------------------------------------------------------------
*/
unsigned long SpiHostNameToId(void *ptTbl, unsigned long ulNums, char *pacspihostname,unsigned long *pulspihostid)
{
    unsigned long           ulLoop = 0;
    T_SPI_HOST_DEVICE   *ptItem = (T_SPI_HOST_DEVICE *)ptTbl;

    if(( NULL == ptItem) || (NULL == pacspihostname) || (NULL == pulspihostid))
    {
        error("input pointer is NULL!");
	return  BSP_ERROR_NULL_POINTER;
    }

    for (ulLoop = 0; ulLoop < ulNums; ulLoop++, ptItem++)
    {
        if (0 == (strcmp(ptItem->acspi_host_name,pacspihostname)))
        {
            *pulspihostid = ptItem->spi_host_id;
            return BSP_OK;
        }
    }

    return  BSP_ERROR_SPI_HOST_FIND_ID;
}

/*****************************************************************************
*  函数名称   : SpiFlashNameToId
*  功能描述   : 将spi host name 转换成相应的id
*  读全局变量 : g_spi_host
*  写全局变量 : 无
*  输入参数   : ptTbl      - 表地址
             ulNums     - 表条目数
             pacspihostname:
*  输出参数   :              pulspihostid    - 指向存储spi host id 变量
*  返 回 值   : 0:标识成功；其它标识失败；
*  其它说明   : 
*----------------------------------------------------------------------------
* 历史记录(变更单, 责任人@修改日期, 操作说明)
*  $0000000(N/A), zhanglr@2012-6-20
*----------------------------------------------------------------------------
*/
unsigned long SpiHostDriverRegister(char *pacspihostname)
{
    unsigned long  dwResult = BSP_OK;
    T_SPI_HOST_DEVICE_ACT * ptspihost = NULL;
    unsigned long ulItemNum = 0;
    unsigned long ulspihostid = 0;

     /* 寻找和spi host name对应的spi host id */
    ulItemNum =  sizeof(g_spi_host) / sizeof(g_spi_host[0]);
    dwResult = SpiHostNameToId(g_spi_host,ulItemNum,pacspihostname,&ulspihostid);
    if(BSP_OK != dwResult)
    {
        error("SpiHostNameToId failed!ErrCode is 0x%08x.",dwResult);
	return  dwResult;
    }

    /* 根据找到的spi host id挂接具体的driver */
    ptspihost = (T_SPI_HOST_DEVICE_ACT *)malloc(sizeof(T_SPI_HOST_DEVICE_ACT));
    if(NULL == ptspihost)
    {
        error("malloc failed!ptspihost is NULL!");
	dwResult = BSP_ERROR_NULL_POINTER;
	return  dwResult;
    }

    switch(ulspihostid)
    {
        case SPI_HOST_DRA624_ID:
	{
           strcpy(ptspihost->spi_host_device.acspi_host_name, g_spi_host[ulspihostid].acspi_host_name);
	   ptspihost->spi_host_device.spi_host_id = g_spi_host[ulspihostid].spi_host_id;
	   ptspihost->spi_host_driver.spi_setup_slave = spi_setup_slave;
	   ptspihost->spi_host_driver.spi_claim_bus=spi_claim_bus;
           ptspihost->spi_host_driver.spi_release_bus = spi_release_bus;
	   ptspihost->spi_host_driver.spi_xfer = spi_xfer;
	   ptspihost->spi_host_driver.spi_free_slave = spi_free_slave;
	   g_ptspihostdevice = ptspihost;
	   g_ulspihostdriverregflag = SPI_HOST_DRIVER_REGISTER;
	   break;
        }
	default:
	{
	    g_ulspihostdriverregflag = 0;
	    g_ptspihostdevice = NULL;
            error("spihostid %d is invalid!",ulspihostid);
	    dwResult = BSP_ERROR_INVALID_PARA;
	}
    }

    if(SPI_HOST_DRIVER_REGISTER == g_ulspihostdriverregflag)
    {
        return BSP_OK;
    }

    if(NULL != ptspihost)
    {
        free(ptspihost);
	ptspihost = NULL;
    }

     return dwResult;
}

/*****************************************************************************
*  函数名称   : GetSpiHostActDevice
*  功能描述   : 获取spi host act 设备句柄
*  读全局变量 : g_ulspihostdriverregflag,g_ptspihostdevice
*  写全局变量 : 无
*  输入参数   : 无
*  输出参数   : T_SPI_HOST_DEVICE_ACT **:指向spi host device的指针的指针； 
*  返 回 值   : 0:标识成功；其它标识失败；
*  其它说明   : 
*----------------------------------------------------------------------------
* 历史记录(变更单, 责任人@修改日期, 操作说明)
*  $0000000(N/A), zhanglr@2012-6-20
*----------------------------------------------------------------------------
*/
unsigned long GetSpiHostActDevice(T_SPI_HOST_DEVICE_ACT ** pptspihostdev)
{
    /* 入参检查*/
    if(NULL == pptspihostdev)
    {
        error("input pointer is NULL!");
	return  BSP_ERROR_NULL_POINTER;
    }
	
    if(SPI_HOST_DRIVER_REGISTER == g_ulspihostdriverregflag)
    {
        *pptspihostdev = g_ptspihostdevice;
        return BSP_OK;
    }
     else
     {
         *pptspihostdev = NULL;
         error("spihost has not registered!");
	 return  BSP_ERROR_SPI_HOST_DRIVER_NOT_REGISTER;
     }
}

