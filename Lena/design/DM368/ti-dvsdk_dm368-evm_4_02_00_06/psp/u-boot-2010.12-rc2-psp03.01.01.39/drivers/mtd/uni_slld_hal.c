/*
 * (C) Copyright 2012-2014
 * zhanglirong, zte, zhang.lirong@zte.com.cn
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
 *
 */

#include <common.h>
#include <uni_slld.h>
#include <uni_slld_hsd.h>
//#include <kuboot_net.h>
#include <asm/arch/ErrCode.h>

/*
 * This file implements a Common Serial Flash Interface (SF) driver for
 * U-Boot.
 */	
 /* 外部全局变量*/
 extern  T_SPI_HOST_DEVICE_ACT *g_ptspihostdevice;

T_SPI_FLASH *to_spi_flash(struct spi_flash *flash)
{
	return container_of(flash, T_SPI_FLASH, spi_flash_drv);
}

/**************************************************************************
* 函数名称：sf_cmd_read
* 功能描述：spi flash读帧操作
* 访问的表： 无
* 修改的表： 无
* 输入参数：struct spi_flash *flash:指向spi flash结构体；
                                 const u8 *cmd:指向读命令数组的首地址；
                                 size_t cmd_len:读命令数组的字节长度；
                                 void *data:指向从 flash中读出数据的缓冲区首地址；
                                 size_t data_len:读出数据的字节长度；
* 输出参数： 无
* 返 回 值：     0标识成功；其它标识错误码；
* 其它说明： 无
* 修改日期     版本号            修改人	        修改内容
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             创建
**************************************************************************/
int sf_cmd_read(struct spi_flash *flash, const u8 *cmd,
		                                   size_t cmd_len, void *data, size_t data_len)
{
	unsigned long flags = SPI_XFER_BEGIN;
	int ret;
	T_SPI_FLASH  *ptSpiFlash = to_spi_flash(flash);
	struct spi_slave *spi = flash->spi;
	unsigned long uldummycycle = 0;
        unsigned char *pucDummyByte = NULL;

	if (data_len == 0)
	{
	    flags |= SPI_XFER_END;
	}

         /* 发出命令字段*/
	ret = g_ptspihostdevice->spi_host_driver.spi_xfer(spi, cmd_len * 8, (const void *)cmd, NULL, flags);
	if (ret) 
	{
	    error("SF: Failed to send read command (%d bytes): %d\n",cmd_len, ret);
	    goto err1;
	} 

	/* 分析是否发出dummy */
        uldummycycle = ptSpiFlash->ptSpiFlashInfo->tSpiFlashCmdSet.tread.ucdummycycle;
        if(0 != uldummycycle)
        {
            pucDummyByte = (unsigned char *)malloc(uldummycycle);
    	    if(NULL == pucDummyByte)
            {
                error("malloc failed!pucDummyByte is NULL!");
                ret = BSP_ERROR_NULL_POINTER;
    	       goto err1;
            }
       	    else
       	    {
       	       memset(pucDummyByte,0,uldummycycle);
       	       ret = g_ptspihostdevice->spi_host_driver.spi_xfer(spi, uldummycycle * 8, (const void *)pucDummyByte, NULL, 0);
       	       if (ret)
   	       {
   	           error("SF: Failed to send dummy byte (%d bytes): %d\n",uldummycycle, ret);
	           goto err2;
   	       }
       	    }
        }
		 
	/* 分析是否接收数据 */
	if (data_len != 0) 
	{
		ret = g_ptspihostdevice->spi_host_driver.spi_xfer(spi, data_len * 8, NULL, data, SPI_XFER_END);
		if (ret)
		{
		    error("SF: Failed to read %d bytes of data: %d\n",data_len, ret);
		}	
	}
err2:
	if(NULL != pucDummyByte)
	{
	    free(pucDummyByte);
	    pucDummyByte = NULL;
	}
err1:
	return ret;
}

/**************************************************************************
* 函数名称：sf_read_common
* 功能描述：spi flash读操作
* 访问的表： 无
* 修改的表： 无
* 输入参数：struct spi_flash *flash:指向spi flash结构体；
                                 const u8 *cmd:指向读命令数组的首地址；
                                 size_t cmd_len:读命令数组的字节长度；
                                 void *data:指向从 flash中读出数据的缓冲区首地址；
                                 size_t data_len:读出数据的字节长度；
* 输出参数： 无
* 返 回 值：     0标识成功；其它标识错误码；
* 其它说明： 无
* 修改日期     版本号            修改人	        修改内容
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             创建
**************************************************************************/
int sf_read_common(struct spi_flash *flash, const u8 *cmd, size_t cmd_len, void *data, size_t data_len)
{
	struct spi_slave *spi = flash->spi;
	int ret;

	#ifdef CONFIG_ZX2114XX
        g_ptspihostdevice->spi_host_driver.spi_claim_bus(spi);
	#endif
	ret = sf_cmd_read(flash, cmd, cmd_len, data, data_len);
	#ifdef CONFIG_ZX2114XX
        g_ptspihostdevice->spi_host_driver.spi_release_bus(spi);
	#endif
	return ret;
}
/**************************************************************************
* 函数名称：sf_cmd
* 功能描述：向spi flash发出1个字节的命令操作
* 访问的表： 无
* 修改的表： 无
* 输入参数：struct spi_slave *spi:指向spi slave结构体；
                                 u8 cmd:1个字节的命令字；
                                 void *response:指向该命令返回数据的缓冲区首地址；
                                 size_t len:从 flash中读出数据的字节长度；
                                 unsigned long uldummycycle:该命令执行过程中需要发送的dummy字节的长度；
* 输出参数： 无
* 返 回 值：     0标识成功；其它标识错误码；
* 其它说明： 无
* 修改日期     版本号            修改人	        修改内容
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             创建
**************************************************************************/
int sf_cmd(struct spi_slave *spi, u8 cmd, void *response, size_t len,unsigned long uldummycycle)
{
	unsigned long flags = SPI_XFER_BEGIN;
	int ret;
        unsigned char *pucDummyByte = NULL;

	if (len == 0)
	{
	    flags |= SPI_XFER_END;
	}

        /* 发出命令字段*/
	ret = g_ptspihostdevice->spi_host_driver.spi_xfer(spi, 8, (const void *)&cmd, NULL, flags);
	if (ret) 
	{
	    error("SF: Failed to send read command (%d bytes): %d\n",cmd, ret);
	    goto err1;
	} 

	/* 分析是否发出dummy */
        if(0 != uldummycycle)
        {
            pucDummyByte = (unsigned char *)malloc(uldummycycle);
    	    if(NULL == pucDummyByte)
            {
                error("malloc failed!pucDummyByte is NULL!");
                ret = BSP_ERROR_NULL_POINTER;
    	       goto err1;
            }
       	    else
       	    {
       	       memset(pucDummyByte,0,uldummycycle);
       	       ret = g_ptspihostdevice->spi_host_driver.spi_xfer(spi, uldummycycle * 8, (const void *)pucDummyByte, NULL, 0);
       	       if (ret)
   	       {
   	           error("SF: Failed to send dummy byte (%d bytes): %d\n",uldummycycle, ret);
	           goto err2;
   	       }
       	    }
        }

	/* 分析是否接收数据 */
	if (len != 0) 
	{
		ret = g_ptspihostdevice->spi_host_driver.spi_xfer(spi, len * 8, NULL, response, SPI_XFER_END);
		if (ret)
		{
		    error("SF: Failed to read response (%d bytes): %d\n",len, ret);
		}
	}	
err2:
	if(NULL != pucDummyByte)
	{
	    free(pucDummyByte);
	    pucDummyByte = NULL;
	}
err1:
	return ret;
}
/**************************************************************************
* 函数名称：sf_cmd_write
* 功能描述：spi flash写帧操作
* 访问的表： 无
* 修改的表： 无
* 输入参数：struct spi_flash *flash:指向spi flash结构体；
                                 const u8 *cmd:指向写命令数组的首地址；
                                 size_t cmd_len:写命令数组的字节长度；
                                 void *data:指向待写入flash的数据缓冲区首地址；
                                 size_t data_len:待写入flash的数据缓冲区字节长度；
* 输出参数： 无
* 返 回 值：     0标识成功；其它标识错误码；
* 其它说明： 无
* 修改日期     版本号            修改人	        修改内容
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             创建
**************************************************************************/
int sf_cmd_write(struct spi_flash *flash, const u8 *cmd,size_t cmd_len, const void *data, size_t data_len)
{
	unsigned long flags = SPI_XFER_BEGIN;
	int ret;
	T_SPI_FLASH  *ptSpiFlash = to_spi_flash(flash);
	struct spi_slave *spi = flash->spi;
	unsigned long uldummycycle = 0;
        unsigned char *pucDummyByte = NULL;

	if (data_len == 0)
	{
	    flags |= SPI_XFER_END;
	}

        /* 发出命令字段*/
	ret = g_ptspihostdevice->spi_host_driver.spi_xfer(spi, cmd_len * 8, (const void *)cmd, NULL, flags);
	if (ret) 
	{
	    error("SF: Failed to send write command (%d bytes): %d\n",cmd_len, ret);
	    goto err1;
	} 

	/* 分析是否发出dummy */
        uldummycycle = ptSpiFlash->ptSpiFlashInfo->tSpiFlashCmdSet.tpp.ucdummycycle;
        if(0 != uldummycycle)
        {
            pucDummyByte = (unsigned char *)malloc(uldummycycle);
    	    if(NULL == pucDummyByte)
            {
                error("malloc failed!pucDummyByte is NULL!");
                ret = BSP_ERROR_NULL_POINTER;
    	       goto err1;
            }
       	    else
       	    {
       	       memset(pucDummyByte,0,uldummycycle);
       	       ret = g_ptspihostdevice->spi_host_driver.spi_xfer(spi, uldummycycle * 8, (const void *)pucDummyByte, NULL, 0);
       	       if (ret)
   	       {
   	           error("SF: Failed to send dummy byte (%d bytes): %d\n",uldummycycle, ret);
	           goto err2;
   	       }
       	    }
        }

	if (data_len != 0) 
	{
    	    ret = g_ptspihostdevice->spi_host_driver.spi_xfer(spi, data_len * 8, (const void *)data, NULL, SPI_XFER_END);
    	    if (ret)
    	    {
    	        error("SF: Failed to write %d bytes of data: %d\n",data_len, ret);
    	    }
	}
err2:
	if(NULL != pucDummyByte)
	{
	    free(pucDummyByte);
	    pucDummyByte = NULL;
	}
err1:
	return ret;
}

