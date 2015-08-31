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
#include <uni_slld_hal.h>
#include <asm/arch/ErrCode.h>
#include <flash.h>


/*
 * This file implements a Common Serial Flash Interface (SF) driver for
 * U-Boot.
 */	
T_SPI_FLASH_INFO  g_tSpiFlash[]=
{
    {
	  {SPANSION_S25FL512S_ID,512,512,256,"S25FL512S","SPANSION"},
	  {
        {SPANSION_SPI_FLASH_4BREAD_CMD,0},
	    {SPANSION_SPI_FLASH_WREN_CMD,0},
	    {SPANSION_SPI_FLASH_WRDI_CMD,0},
	    {SPANSION_SPI_FLASH_4PP_CMD,0},
	    {SPANSION_SPI_FLASH_4SE_CMD,0},
	    {SPANSION_SPI_FLASH_BE_CMD,0},
	    {SPANSION_SPI_FLASH_SOFTWARE_RESET_CMD,0},
	    {SPANSION_SPI_FLASH_STATUS_REG1_CMD,0},
	  },
	  0,
	  SPANSION_PAGE_PROGRAM_TIMEOUT,
	  SPANSION_SECTOR_ERASE_TIMEOUT,
	  4,
	},
	
    {
	  {MICRON_N25Q512_ID,256,256,1024,"MICRON_N25Q512","MICRON"},
	  {
        {MICRON_SPI_FLASH_4BREAD_CMD,0},
	    {MICRON_SPI_FLASH_WREN_CMD,0},
	    {MICRON_SPI_FLASH_WRDI_CMD,0},
	    {MICRON_SPI_FLASH_4PP_CMD,0},
	    {MICRON_SPI_FLASH_4SE_CMD,0},
	    {MICRON_SPI_FLASH_BE_CMD,0},
	    {MICRON_SPI_FLASH_SOFTWARE_RESET_CMD,0},
	    {MICRON_SPI_FLASH_STATUS_REG1_CMD,0},
	  },
	  0,
	  MICRON_PAGE_PROGRAM_TIMEOUT,
	  MICRON_SECTOR_ERASE_TIMEOUT,
	  4,
	},

    {
	  {MACRONIX_MX66L51235F_ID,256,256,1024,"MX66L51235F","MACRONIX"},
	  {
        {MACRONIX_SPI_FLASH_4BREAD_CMD,0},
	    {MACRONIX_SPI_FLASH_WREN_CMD,0},
	    {MACRONIX_SPI_FLASH_WRDI_CMD,0},
	    {MACRONIX_SPI_FLASH_4PP_CMD,0},
	    {MACRONIX_SPI_FLASH_4SE_CMD,0},
	    {MACRONIX_SPI_FLASH_BE_CMD,0},
	    {MACRONIX_SPI_FLASH_SOFTWARE_RESET_CMD,0},
	    {MACRONIX_SPI_FLASH_STATUS_REG1_CMD,0},
	  },
	  0,
	  MACRONIX_PAGE_PROGRAM_TIMEOUT,
	  MACRONIX_SECTOR_ERASE_TIMEOUT,
	  4,
	}        
        
};

T_SPI_FLASH_INFO *g_ptSpiFlashAct = NULL;
unsigned long  g_ulSpiFlashProbeDone = 0;

/* spi host name */
 char g_acspi_host_name[SPI_HOST_NAME_NUM];

/* spi flash调试全局变量*/
unsigned long g_ulSpiFlashDebug = 0;

#ifndef CFG_NO_FLASH
flash_info_t  flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips */
#endif

/* spi flash结构体变量指针*/
static struct spi_flash * g_ptspiflash = NULL;

 /* 外部全局变量*/
 extern  T_SPI_HOST_DEVICE_ACT *g_ptspihostdevice;
/**************************************************************************
* 函数名称：SetSpiFlashDebugFlag
* 功能描述：设置spi flash的调试标志
* 访问的表： 无
* 修改的表： 无
* 输入参数：unsigned long ulFlag:调试标志
* 输出参数： 无
* 返 回 值：     无
* 其它说明： 0:标识关闭调试信息；1标识打开调试信息；
* 修改日期     版本号            修改人	        修改内容
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             创建
**************************************************************************/
void SetSpiFlashDebugFlag(unsigned long ulFlag)
{
	if((0 != ulFlag) && (1 != ulFlag))
	{
		printf("ulFlag must be 0 or 1!0:close debug info;1:open debug info!\n");
		return;
	}

	g_ulSpiFlashDebug = ulFlag;

	if(0 == ulFlag)
	{
		printf("close debug info!\n");
		return;
	}
	else
	{
		printf("open debug info!\n");
		return;
	}
}
#ifndef  CFG_NO_FLASH

/**************************************************************************
* 函数名称：init_flash_info
* 功能描述：初始化Flash
* 访问的表： 无
* 修改的表： 
* 输入参数： spi_flash_info:当前flash信息
* 输出参数：无
* 返 回 值： 无
* 其它说明： 无
* 修改日期     版本号            修改人	        修改内容
* -------------------------------------------------------------------------
* 2012/09/12            V1.0	               李小东             创建
**************************************************************************/
void init_flash_info (T_SPI_FLASH_INFO *spi_flash_info)
{
    ushort      sector_count;
    ulong       index;
    ulong       page_size;
    ulong       pages_per_sector;

    if( NULL == spi_flash_info )
    {
        printf("Flash info is NULL!\n");
        return;
    }

    sector_count        = spi_flash_info->tSpiFlashDevPara.nr_sectors;
    page_size           = spi_flash_info->tSpiFlashDevPara.page_size;
    pages_per_sector    = spi_flash_info->tSpiFlashDevPara.pages_per_sector;
    
	flash_info[0].erase_size = pages_per_sector * page_size;
    flash_info[0].sector_count = sector_count;
    flash_info[0].size = sector_count * pages_per_sector * page_size;   

	printf("%s %d, pagesize:%d %d erasesize%d %d totalsize%d\n", __FUNCTION__, __LINE__, 
		page_size, 
		pages_per_sector, 
		flash_info[0].erase_size, 
		flash_info[0].sector_count, 
		flash_info[0].size);
	
    for(index = 0; index < flash_info[0].sector_count; index ++)
    {
        flash_info[0].start[index] = flash_info[0].erase_size * index;
    }

    flash_protect (FLAG_PROTECT_SET,
			       CFG_MONITOR_BASE,
			       CFG_MONITOR_BASE + monitor_flash_len -1,
			       &flash_info[0]);

}

ulong get_flash_erase_size (ushort bank)
{
    return flash_info[bank].erase_size; 
}
/**************************************************************************
* 函数名称：flash_init
* 功能描述：初始化Flash
* 访问的表： 无
* 修改的表： flash_info
* 输入参数： 无
* 输出参数：无
* 返 回 值：     flash字节容量大小；
* 其它说明： 无
* 修改日期     版本号            修改人	        修改内容
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             创建
**************************************************************************/
unsigned long flash_init (void)
{
    struct spi_flash * ptspiflash = NULL;
    unsigned long ulsize = 0;

#ifdef CONFIG_ZX2114XX
    ptspiflash = spi_flash_probe(0, 0,48000000, SPI_MODE_3);
#else
    ptspiflash = spi_flash_probe(0, 0,48000000, SPI_MODE_3);
#endif

    
    if(NULL == ptspiflash)
    {
        error("\nspi_flash_probe failed!\n");
	    return  0;
    }
    else
    {
        ulsize = ptspiflash->size;
        init_flash_info(g_ptSpiFlashAct);          
        printf("\nspi_flash_probe successfully!\n");        
        return ulsize;
    }

   
}

/**************************************************************************
* 函数名称：flash_real_protect
* 功能描述：flash sector物理保护操作
* 访问的表： 无
* 修改的表： 无
* 输入参数： 无
* 输出参数：无
* 返 回 值：      0标识成功；其它标识错误码；
* 其它说明： 无
* 修改日期     版本号            修改人	        修改内容
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             创建
**************************************************************************/
int flash_real_protect (flash_info_t * info, long sector, int prot)
{
    /* 目前暂时没有实现，后续需要添加对sector的DYB的支持*/
    return 0;
}
#endif
/**************************************************************************
* 函数名称：SpiFlashReadIDCmd
* 功能描述：读SF的ID
* 访问的表： 无
* 修改的表： 无
* 输入参数：struct spi_slave *spi:指向spi slave；
                                 void *response:指向读出的数据；
                                 unsigned long len:读出数据的字节数目；
* 输出参数： 无
* 返 回 值：     0标识成功；其它标识错误码；
* 其它说明： 无
* 修改日期     版本号            修改人	        修改内容
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             创建
**************************************************************************/
unsigned long  SpiFlashReadIDCmd(struct spi_slave *spi, void *response, unsigned long len)
{
    unsigned long dwResult = BSP_OK;

    /* 入参检查*/
    if((NULL == spi) ||(NULL == response))
    {
        error("input pointer is NULL!");
	return  BSP_ERROR_NULL_POINTER;
    }
	
    dwResult = sf_cmd(spi, CMD_READ_ID, response, len,0);
    if(BSP_OK != dwResult)
    {
        error("spi_flash_cmd Failed!ErrCode is 0x%08x!",dwResult);
	return  dwResult;
    }

    return BSP_OK;
}

/**************************************************************************
* 函数名称：SpiFlashSpansionIDParse
* 功能描述：SPANSION spi flash id解析
* 访问的表： 无
* 修改的表： 无
* 输入参数：unsigned char *pucID:指向id数组的指针；
                                 unsigned long len:id数组的大小；
* 输出参数： 无
* 返 回 值：     0标识成功；其它标识错误码；
* 其它说明： 无
* 修改日期     版本号            修改人	        修改内容
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             创建
**************************************************************************/
unsigned long SpiFlashIDParse(unsigned char *pucID, unsigned long len)
{
    unsigned long  ulindex = 0;
    unsigned long  ulItemSize = 0;
    unsigned short  usID = 0;

    /* 入参检查*/
    if(NULL == pucID)
    {
        error("input pointer is NULL!");
		return  BSP_ERROR_NULL_POINTER;
    }

    if(SPI_FLASH_ID_NUM != len)
    {
        error("input para is invalid!len is %d.",len);
		return  BSP_ERROR_INVALID_PARA;
    }

    usID = (unsigned short)pucID[1]<<8;
    usID |= (unsigned short)pucID[2];

    ulItemSize = sizeof(g_tSpiFlash)/sizeof(g_tSpiFlash[0]);
    for(ulindex = 0;ulindex < ulItemSize;ulindex ++)
    {
        if(g_tSpiFlash[ulindex].tSpiFlashDevPara.idcode1 == usID)
        {
            g_ptSpiFlashAct = &g_tSpiFlash[ulindex];
		    g_ulSpiFlashProbeDone = SPI_FLASH_PROBEED;
		    return  BSP_OK;
        }
    }

    g_ulSpiFlashProbeDone = 0;
    return  BSP_ERROR_SPI_FLASH_PROBE;
}
/**************************************************************************
* 函数名称：GetSpiFlashInfoFromID
* 功能描述：根据id信息获取spi flash信息
* 访问的表： 无
* 修改的表： 无
* 输入参数：unsigned char *pucID:指向id数组的指针；
                                 unsigned long len:id数组的大小；
* 输出参数： 无
* 返 回 值：     0标识成功；其它标识错误码；
* 其它说明： 无
* 修改日期     版本号            修改人	        修改内容
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             创建
**************************************************************************/
unsigned long GetSpiFlashInfoFromID(unsigned char *pucID, unsigned long len)
{
    unsigned long  dwResult = BSP_OK;

    /* 入参检查*/
    if(NULL == pucID)
    {
        error("input pointer is NULL!");
		return  BSP_ERROR_NULL_POINTER;
    }

    if(SPI_FLASH_ID_NUM != len)
    {
        error("input para is invalid!len is %d.",len);
		return  BSP_ERROR_INVALID_PARA;
    }

    switch (pucID[0]) 
    {
	case SPANSION_SPI_FLASH_ID:
	{
    	    dwResult = SpiFlashIDParse(pucID, len);
            if(BSP_OK != dwResult)
            {
                 error("SpiFlashIDParse Failed!");
            }
    	    break;
	}
    case MICRON_SPI_FLASH_ID:
	{
    	    dwResult = SpiFlashIDParse(pucID, len);
            if(BSP_OK != dwResult)
            {
                 error("SpiFlashIDParse Failed!");
            }
    	    break;
	}
    case MACRONIX_SPI_FLASH_ID:
	{
    	    dwResult = SpiFlashIDParse(pucID, len);
            if(BSP_OK != dwResult)
            {
                 error("SpiFlashIDParse Failed!");
            }
    	    break;
	}
    
	default:
	{
	     error("pucID[0] is invalid !pucID[0] is 0x%02x.",pucID[0]);
	     dwResult = BSP_ERROR_SPI_FLASH_MANUFACTURER_ID;
	}
    }

     return dwResult;
}
/**************************************************************************
* 函数名称：sf_wait_ready
* 功能描述：等待flas完成写操作或着擦除操作
* 访问的表： 无
* 修改的表： 无
* 输入参数： struct spi_flash *flash:指向spi flash结构体；
                                  unsigned long timeout:超时的时间，单位是秒；
* 输出参数： 无
* 返 回 值：     0标识成功；其它标识错误码；
* 其它说明： 无
* 修改日期     版本号            修改人	        修改内容
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             创建
**************************************************************************/
static int sf_wait_ready(struct spi_flash *flash, unsigned long timeout)
{
    T_SPI_FLASH  *ptSpiFlash = NULL;
	struct spi_slave *spi = flash->spi;
	unsigned long timebase = 0;
	int ret = 0;
	u8 status = 0;
    u8 status_flag = 0;
	unsigned char uccmd = 0;
	unsigned long uldummy = 0;
	unsigned long ulwribitoffset = 0;

        /* 入参检查*/
        if((NULL == flash) || (NULL == spi))
        {
            error("input pointer is NULL!");
	    return  BSP_ERROR_NULL_POINTER;
        }

        ptSpiFlash = to_spi_flash(flash);
	 if(NULL == ptSpiFlash) 
        {
            error("input pointer is NULL!");
	    return  BSP_ERROR_NULL_POINTER;
        }

        ulwribitoffset = ptSpiFlash->ptSpiFlashInfo->ulwribitoffset;
	uccmd  = ptSpiFlash->ptSpiFlashInfo->tSpiFlashCmdSet.trdstatusreg.ucopcode;
	uldummy = ptSpiFlash->ptSpiFlashInfo->tSpiFlashCmdSet.trdstatusreg.ucdummycycle;
	timebase = get_timer(0);
	do 
	{
		ret = sf_cmd(spi, uccmd, &status, sizeof(status),uldummy);
		if (ret)
		{
		    return  BSP_ERROR_SPI_FLASH_CMD;
		}
		if(1 == g_ulSpiFlashDebug)
		{
		    printf("status:0x%x.\n",status);
		}
		if ((status & (1<<(ulwribitoffset))) == 0)
			break;
	} while (get_timer(timebase) < timeout);

    sf_cmd(spi, 0x70, &status_flag, sizeof(status_flag),uldummy);
    // printf("status_flag:0x%x.\n",status);
	if ((status & (1<<(ulwribitoffset))) == 0)
		return  BSP_OK;

	/* Timed out */
	return  BSP_ERROR_SPI_FLASH_WRITE_IN_PROGRESS;
}
/**************************************************************************
* 函数名称：sf_read
* 功能描述：spi flash的读操作
* 访问的表： 无
* 修改的表： 无
* 输入参数： struct spi_flash *flash:指向spi flash结构体；
                                  u32 offset:读flash的偏移地址；
                                  size_t len:读出的字节长度；
                                  void *buf:指向保存flash读出数据的缓冲区首地址；
* 输出参数： 无
* 返 回 值：     0标识成功；其它标识错误码；
* 其它说明： 无
* 修改日期     版本号            修改人	        修改内容
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             创建
**************************************************************************/
int sf_read(struct spi_flash *flash,u32 offset, size_t len, void *buf)
{
        T_SPI_FLASH  *ptSpiFlash = NULL;
	unsigned char  cmd[5];
	size_t cmd_len = 0;

         /* 入参检查*/
        if((NULL == flash) || (NULL == buf))
        {
            error("input pointer is NULL!");
	    return  BSP_ERROR_NULL_POINTER;
        }

        ptSpiFlash = to_spi_flash(flash);
	 if(NULL == ptSpiFlash) 
        {
            error("input pointer is NULL!");
	    return  BSP_ERROR_NULL_POINTER;
        }

	 if(3 == ptSpiFlash->ptSpiFlashInfo->uladdrlength)
	 {
	     cmd[0] = ptSpiFlash->ptSpiFlashInfo->tSpiFlashCmdSet.tread.ucopcode;
    	     cmd[1] = (unsigned char)(offset >> 16);
    	     cmd[2] = (unsigned char)(offset >> 8);
    	     cmd[3] = (unsigned char)offset;
	     cmd_len = 4;
	 }
	 else if(4 == ptSpiFlash->ptSpiFlashInfo->uladdrlength)
	 {
 	    cmd[0] = ptSpiFlash->ptSpiFlashInfo->tSpiFlashCmdSet.tread.ucopcode;
 	    cmd[1] = (unsigned char)(offset >> 24);
 	    cmd[2] = (unsigned char)(offset >> 16);
 	    cmd[3] = (unsigned char)(offset >> 8);
 	    cmd[4] = (unsigned char)offset;
	    cmd_len = 5;
	 }
	 else
	 {
	    error("uladdrlength is invalid!uladdrlength is %d.\n",ptSpiFlash->ptSpiFlashInfo->uladdrlength);
	    return  BSP_ERROR_SPI_FLASH_ADDR_LENGTH;
	 }

	return sf_read_common(flash, cmd, cmd_len, buf, len);
}
/**************************************************************************
* 函数名称：sf_write
* 功能描述：spi flash的写操作
* 访问的表： 无
* 修改的表： 无
* 输入参数： struct spi_flash *flash:指向spi flash结构体；
                                  u32 offset:写flash的偏移地址；
                                  size_t len:写flash的字节长度；
                                  void *buf:指向待写入flash的数据缓冲区首地址；
* 输出参数： 无
* 返 回 值：     0标识成功；其它标识错误码；
* 其它说明： 无
* 修改日期     版本号            修改人	        修改内容
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             创建
**************************************************************************/
int sf_write(struct spi_flash *flash,u32 offset, size_t len, const void *buf)
{
	T_SPI_FLASH  *ptSpiFlash = NULL;
	unsigned long pro_addr = 0;
	unsigned long page_addr_no = 0;
	unsigned long byte_addr = 0;
	unsigned long page_size = 0;
	size_t chunk_len = 0;;
	size_t actual = 0;
	int ret = BSP_OK;
	u8 cmd[5];
	u8 uccmd = 0;
	unsigned long uldummy = 0;
	struct spi_slave *spi = flash->spi;
	size_t cmd_len = 0;

    /* 入参检查*/
	if((NULL == flash) || (NULL == buf) || (NULL == spi))
	{
	    error("input pointer is NULL!");
	    return  BSP_ERROR_NULL_POINTER;
	}

    ptSpiFlash = to_spi_flash(flash);
	if(NULL == ptSpiFlash) 
    {
        error("input pointer is NULL!");
	    return  BSP_ERROR_NULL_POINTER;
    }
	
	page_size = ptSpiFlash->ptSpiFlashInfo->tSpiFlashDevPara.page_size;
	page_addr_no = offset / page_size;
	byte_addr = offset % page_size;
	cmd[0] = ptSpiFlash->ptSpiFlashInfo->tSpiFlashCmdSet.tpp.ucopcode;

	#ifdef CONFIG_ZX2114XX
	g_ptspihostdevice->spi_host_driver.spi_claim_bus(spi);
	#endif

	for (actual = 0; actual < len; actual += chunk_len) 
	{
		chunk_len = min(len - actual, page_size - byte_addr);
		
		pro_addr = page_addr_no * page_size + byte_addr;
		if(3 == ptSpiFlash->ptSpiFlashInfo->uladdrlength)
		{
            cmd[1] = (unsigned char)(pro_addr >> 16);
            cmd[2] = (unsigned char)(pro_addr >> 8);
            cmd[3] = (unsigned char)pro_addr;
		    cmd_len = 4;
		    if(1 == g_ulSpiFlashDebug)
		    {
			    printf("PP: 0x%08x => cmd = { 0x%02x 0x%02x %02x %02x} chunk_len = %d\n",
		               buf + actual, cmd[0], cmd[1], cmd[2], cmd[3],chunk_len);
		    }

		}
		else if(4 == ptSpiFlash->ptSpiFlashInfo->uladdrlength)
		{
            cmd[1] = (unsigned char)(pro_addr >> 24);
            cmd[2] = (unsigned char)(pro_addr >> 16);
            cmd[3] = (unsigned char)(pro_addr >> 8);
            cmd[4] = (unsigned char)pro_addr;
		    cmd_len = 5;
		    if(1 == g_ulSpiFlashDebug)
		    {
			    printf("PP: 0x%08x => cmd = { 0x%02x 0x%02x %02x %02x %02x} chunk_len = %d\n",
		                buf + actual, cmd[0], cmd[1], cmd[2], cmd[3], cmd[4],chunk_len);
		    }
		}
		else
        {
            error("uladdrlength is invalid!uladdrlength is %d.\n",ptSpiFlash->ptSpiFlashInfo->uladdrlength);
            return  BSP_ERROR_SPI_FLASH_ADDR_LENGTH;
         }

        uccmd = ptSpiFlash->ptSpiFlashInfo->tSpiFlashCmdSet.twren.ucopcode;
		uldummy = ptSpiFlash->ptSpiFlashInfo->tSpiFlashCmdSet.twren.ucdummycycle;
		
		ret = sf_cmd(spi, uccmd,NULL,0,uldummy);
		if (0 != ret) 
		{
		    error("SF: Enabling Write failed\n");
		    goto err1;
		}

		ret = sf_cmd_write(flash, cmd, cmd_len, buf + actual, chunk_len);
		if (0 != ret)  
		{
	            error("SF:  Page Program failed\n");
		    goto err2;
		}

		ret = sf_wait_ready(flash, ptSpiFlash->ptSpiFlashInfo->ulppwaittime);
		if (0 != ret) 
		{
		    error("SF:  page programming timed out\n");
		    goto err2;
		}

		page_addr_no++;
		byte_addr = 0;
	}

    if(1 == g_ulSpiFlashDebug)
    {
        printf("SF: Successfully programmed %u bytes @ 0x%x\n",len, offset);
    }
err2:

#if 0
	uccmd = ptSpiFlash->ptSpiFlashInfo->tSpiFlashCmdSet.twrdi.ucopcode;
	uldummy = ptSpiFlash->ptSpiFlashInfo->tSpiFlashCmdSet.twrdi.ucdummycycle;
	ret = sf_cmd(spi, uccmd,NULL,0,uldummy);
	if (0 != ret) 
	{
	    error("SF: Disable Write failed\n");
	}
#endif

err1:

	#ifdef CONFIG_ZX2114XX
	g_ptspihostdevice->spi_host_driver.spi_release_bus(spi);
	#endif

	return ret;
}
/**************************************************************************
* 函数名称：sf_erase
* 功能描述：spi flash的擦除操作
* 访问的表： 无
* 修改的表： 无
* 输入参数： struct spi_flash *flash:指向spi flash结构体；
                                  u32 offset:flash擦除的偏移地址；
                                  size_t len:flash擦除的字节长度；
* 输出参数： 无
* 返 回 值：     0标识成功；其它标识错误码；
* 其它说明： 无
* 修改日期     版本号            修改人	        修改内容
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             创建
**************************************************************************/
int sf_erase(struct spi_flash *flash, u32 offset, size_t len)
{
    T_SPI_FLASH  *ptSpiFlash = NULL;
	unsigned long sector_size = 0;
	size_t actual = 0;
	int ret = 0;
	u8 cmd[5];
	u8 uccmd;
        unsigned long erase_addr = 0;
	unsigned long uldummy = 0;
	struct spi_slave *spi = flash->spi;
	size_t cmd_len = 0;
	/*
	 * This function currently uses sector erase only.
	 * probably speed things up by using bulk erase
	 * when possible.
	 */
	  /* 入参检查*/
	if((NULL == flash) || (NULL == spi))
	{
	    error("input pointer is NULL!");
	    return  BSP_ERROR_NULL_POINTER;
	}

    ptSpiFlash = to_spi_flash(flash);
	sector_size = ptSpiFlash->ptSpiFlashInfo->tSpiFlashDevPara.page_size * ptSpiFlash->ptSpiFlashInfo->tSpiFlashDevPara.pages_per_sector;

	if ((offset % sector_size) || (len % sector_size))
	{
        error("SF: Erase offset/length not multiple of sector size\n");
	    return  BSP_ERROR_SPI_FLASH_ERASE_ADDR_LENGTH;
	}

	cmd[0] = ptSpiFlash->ptSpiFlashInfo->tSpiFlashCmdSet.tse.ucopcode;

	#ifdef CONFIG_ZX2114XX
	g_ptspihostdevice->spi_host_driver.spi_claim_bus(spi);
	#endif

	len = len / sector_size;
	for (actual = 0; actual < len; actual++) 
	{
		erase_addr = offset + sector_size * actual;
		if(3 == ptSpiFlash->ptSpiFlashInfo->uladdrlength)
		{
            cmd[1] = (unsigned char)(erase_addr >> 16);
            cmd[2] = (unsigned char)(erase_addr >> 8);
            cmd[3] = (unsigned char)erase_addr;
		    cmd_len = 4;
		    if(1 == g_ulSpiFlashDebug)
		    {
			    printf("SE 3: 0x%08x => cmd = { 0x%02x 0x%02x %02x %02x }\n",
		                               erase_addr, cmd[0], cmd[1], cmd[2], cmd[3]);
		    }

		}
		else if(4 == ptSpiFlash->ptSpiFlashInfo->uladdrlength)
		{
            cmd[1] = (unsigned char)(erase_addr >> 24);
            cmd[2] = (unsigned char)(erase_addr >> 16);
            cmd[3] = (unsigned char)(erase_addr >> 8);
            cmd[4] = (unsigned char)erase_addr;
		    cmd_len = 5;
		    if(1 == g_ulSpiFlashDebug)
		    {
			    printf("SE 4: 0x%08x => cmd = { 0x%02x 0x%02x %02x %02x %02x}\n",
		                               erase_addr, cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]);
		    }
		}
		else
        {
            error("uladdrlength is invalid!uladdrlength is %d.\n",ptSpiFlash->ptSpiFlashInfo->uladdrlength);
            return  BSP_ERROR_SPI_FLASH_ADDR_LENGTH;
        }

        uccmd = ptSpiFlash->ptSpiFlashInfo->tSpiFlashCmdSet.twren.ucopcode;
		uldummy = ptSpiFlash->ptSpiFlashInfo->tSpiFlashCmdSet.twren.ucdummycycle;
		ret = sf_cmd(spi, uccmd,NULL,0,uldummy);
		if (0 != ret) 
		{
		    error("SF: Enabling Write failed\n");
		    goto err1;
		}

		ret = sf_cmd_write(flash, cmd, cmd_len, NULL, 0);
		if (0 != ret)  
		{
            error("SF: Sector Erase failed\n");
		    goto err2;
		}

		ret = sf_wait_ready(flash, ptSpiFlash->ptSpiFlashInfo->ulsewaittime);
		if (0 != ret) 
		{
		    error("SF: sector erase timed out\n");
		    goto err2;
		}
		
	}
	
	if(1 == g_ulSpiFlashDebug)
	{
        printf("SF: Successfully erased %d bytes @ 0x%x\n", len * sector_size, offset);
	}
err2:
#if 0
	uccmd = ptSpiFlash->ptSpiFlashInfo->tSpiFlashCmdSet.twrdi.ucopcode;
	uldummy = ptSpiFlash->ptSpiFlashInfo->tSpiFlashCmdSet.twrdi.ucdummycycle;
	ret = sf_cmd(spi, uccmd,NULL,0,uldummy);
	if (0 != ret) 
	{
	    error("SF: Disable Write failed\n");
	}
#endif

err1:

	#ifdef CONFIG_ZX2114XX
	g_ptspihostdevice->spi_host_driver.spi_release_bus(spi);
	#endif

	return ret;
}

#ifndef CFG_NO_FLASH
/**************************************************************************
* 函数名称：flash_erase
* 功能描述：根据flash_info_t结构体信息完成擦除flash扇区操作
* 访问的表： flash_info
* 修改的表： 无
* 输入参数： flash_info_t * info:指向保存flash信息的结构体；
                                  int s_first:擦除的起始flash扇区号；
                                  int s_last:擦除的结束flash扇区号；
* 输出参数： 无
* 返 回 值：     0标识成功；其它标识错误码；
* 其它说明： 无
* 修改日期     版本号            修改人	        修改内容
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             创建
**************************************************************************/
int flash_erase (flash_info_t * info, int s_first, int s_last)
{
    unsigned long  ulProtect = 0;
    unsigned long  ulSector = 0;
    struct spi_flash * ptspiflash = NULL;
    int ret = 0;
    unsigned long  uloffset = 0;
    unsigned long  ullength = 0;

    /* 入参判断*/
    if ((s_first < 0) || (s_first > s_last))
    {
        if (info->flash_id == FLASH_UNKNOWN)
        {
            error ("- missing\n");
        } 
        else
        {
            error ("- no sectors to erase\n");
        }
        return  BSP_ERROR_INVALID_PARA;
    }

    if(NULL == info)
    {
        error("input pointer is NULL!");
	return  BSP_ERROR_NULL_POINTER;
    }
    /* 判断Flash 类型*/
    if ((info->flash_id == FLASH_UNKNOWN) ||
	  ((info->flash_id > FLASH_AMD_COMP) &&
	  ((info->flash_id & FLASH_VENDMASK) != FLASH_MAN_INTEL)))
    {
        error ("Can't erase unknown flash type - aborted\n");
        return  BSP_ERROR_INVALID_PARA;
    }
	
    ulProtect = 0;
    /* 统计待擦除范围内，受保护的Sector 个数*/
    for (ulSector = s_first; ulSector <= s_last; ulSector++)
    {
        if (info->protect[ulSector])
        {
            ulProtect++;
        }
    }
	
    if (ulProtect)
    {
        error ("- Error: %d protected sectors will not be erased!\n", ulProtect);
	return BSP_ERROR_SPI_FLASH_ERASE_IN_PROTECT;
    } 
	
    /* Disable interrupts which might cause a timeout here */
    disable_interrupts();
	
    ptspiflash = GetSpiFlashHandle();
    if(NULL == ptspiflash)
    {
        error("ptspiflash is NULL.flash_erase failed!\n");
	return  BSP_ERROR_SPI_FLASH_GET_HANDLE;
    }

    uloffset = info->start[s_first];
    ullength = (s_last - s_first + 1) * get_flash_erase_size(0);
    ret = sf_erase(ptspiflash,uloffset, ullength);
    if(BSP_OK != ret)
    {
        error("sf_erase failed!\n");
    }
	
    enable_interrupts();
    return ret;
}
#endif
#if 1
/**************************************************************************
* 函数名称：SpiFlashEnter_4B_Mode
* 功能描述：读SF的ID
* 访问的表： 无
* 修改的表： 无
* 输入参数：struct spi_slave *spi:指向spi slave；
                                 void *response:指向读出的数据；
                                 unsigned long len:读出数据的字节数目；
* 输出参数： 无
* 返 回 值：     0标识成功；其它标识错误码；
* 其它说明： 无
* 修改日期     版本号            修改人	        修改内容
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             创建
**************************************************************************/
unsigned long  SpiFlashEnter_4B_Mode(struct spi_flash *flash)
{
    T_SPI_FLASH  *ptSpiFlash = NULL;
	int ret = BSP_OK;
	u8 uccmd = 0;
    u8 flag_status = 0;
	unsigned long uldummy = 0;
	struct spi_slave *spi = NULL;

    /* 入参检查*/
    if(NULL == flash)
    {
        error("input pointer is NULL!");
	    return  BSP_ERROR_NULL_POINTER;
    }

    ptSpiFlash = to_spi_flash(flash);
    if(NULL == ptSpiFlash) 
    {
        error("input pointer is NULL!");
        return  BSP_ERROR_NULL_POINTER;
    }

    printf("\nEnter 4 bytes address mode!\n");

    spi = flash->spi;
	
	#ifdef CONFIG_ZX2114XX
	g_ptspihostdevice->spi_host_driver.spi_claim_bus(spi);
	#endif 

    uccmd = ptSpiFlash->ptSpiFlashInfo->tSpiFlashCmdSet.twren.ucopcode;
	uldummy = ptSpiFlash->ptSpiFlashInfo->tSpiFlashCmdSet.twren.ucdummycycle;
	
	ret = sf_cmd(spi, uccmd,NULL,0,uldummy);
	if (0 != ret) 
	{
	    error("SF: Enabling Write failed\n");
        goto out;
	}

    ret = sf_cmd(spi, 0xb7,NULL,0,uldummy);
    if (0 != ret) 
	{
	    error("SF: Enter 4B address failed\n");
        goto out;
	}

    ret = sf_wait_ready(flash, ptSpiFlash->ptSpiFlashInfo->ulppwaittime);
	if (0 != ret) 
	{
	    error("SF:  page programming timed out\n");
	    goto out;
	}
    ret = sf_cmd(spi, 0x70, &flag_status, sizeof(flag_status),0);
    if (0 != ret) 
	{
	    error("SF:  read flag_status faild!\n");
	    goto out;
	}

    printf("%s flag_status=0x%02x\n", __FUNCTION__, flag_status);

#if 0
    uccmd = ptSpiFlash->ptSpiFlashInfo->tSpiFlashCmdSet.twrdi.ucopcode; 	
	ret = sf_cmd(spi, uccmd,NULL,0,uldummy);
	if (0 != ret) 
	{
	    error("SF: Disable Write failed\n");
        goto out;
	}
#endif

out:

	#ifdef CONFIG_ZX2114XX
    g_ptspihostdevice->spi_host_driver.spi_release_bus(spi);
	#endif

    return ret;
}
#endif

/**************************************************************************
* 函数名称：spi_flash_probe
* 功能描述：探测spi flash并做相应的初始化操作
* 访问的表： g_tSpiFlash
* 修改的表： 无
* 输入参数： unsigned int bus:spi flash挂在哪个spi bus上；
                                  unsigned int cs:spi flash挂在spi bus的哪个cs上；
                                  unsigned int max_hz:spi 的最大时钟频率；
                                  unsigned int spi_mode:spi 的模式;
* 输出参数： 无
* 返 回 值：     struct spi_flash *:指向探测到的spi flash结构体；
* 其它说明： 无
* 修改日期     版本号            修改人	        修改内容
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             创建
**************************************************************************/
struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,unsigned int max_hz, unsigned int spi_mode)
{
	struct spi_slave *spi;
	u8 idcode[3];
	unsigned long dwResult = 0;
    T_SPI_HOST_DEVICE_ACT *ptspihostdevice = NULL;
	static T_SPI_FLASH  *ptSpiFlash = NULL;
	static unsigned int s_bus = 0;
	static unsigned int s_cs = 0;
	static unsigned int s_max_hz = 0;
    static unsigned int s_spi_mode = 0;
		
    /* spi flash已经进行了初始化操作*/
	if(NULL != g_ptspiflash)
	{
	    if((s_bus == bus) && (s_cs == cs) && (s_max_hz == max_hz) && (s_spi_mode == spi_mode))
	    {
            printf("Spi Flash:%s,%s;Page Size:%dB;Size:%dMB.",
				ptSpiFlash->ptSpiFlashInfo->tSpiFlashDevPara.manufacturer_name,\
                ptSpiFlash->ptSpiFlashInfo->tSpiFlashDevPara.name,\
                ptSpiFlash->ptSpiFlashInfo->tSpiFlashDevPara.page_size,\
                ptSpiFlash->spi_flash_drv.size >> 20);
			
		    return  g_ptspiflash;
	    }
        else
        {
            /* free 已有的指针，重新初始化*/
            dwResult = GetSpiHostActDevice(&ptspihostdevice);
            if(BSP_ERROR_SPI_HOST_DRIVER_NOT_REGISTER == dwResult)
            {
                error("Initialization has been finished,but GetSpiHostActDevice failed!ErrCode is 0x%08x.",dwResult);
                return  NULL;
            }
            else
            {
                ptspihostdevice->spi_host_driver.spi_free_slave(g_ptspiflash->spi);
                free(ptspihostdevice);
                ptspihostdevice = NULL;
                free(to_spi_flash(g_ptspiflash));
                g_ptspiflash = NULL;
            }
        }		
	}
	   
	/*1. 需要添加相关代码，根据某个信息获取spi host id，挂接spi host driver */
	strcpy(g_acspi_host_name,"DRA624");

	/*2.根据spi host name注册驱动*/
	dwResult = SpiHostDriverRegister(g_acspi_host_name);
	if(BSP_OK != dwResult)
	{
		error("SpiHostDriverRegister failed!ErrCode is 0x%08x.",dwResult);
		return  NULL;
	}
	   
	/*3.获取spi host act dev句柄*/
    dwResult = GetSpiHostActDevice(&ptspihostdevice);
	if(BSP_OK != dwResult)
	{
		error("GetSpiHostActDevice failed!ErrCode is 0x%08x.",dwResult);
		goto err1;    
	}

	/*4.建立spi slave */
	spi = ptspihostdevice->spi_host_driver.spi_setup_slave(bus, cs, max_hz, spi_mode);
	if (!spi) 
	{
	   error("spi_setup_slave failed!ErrCode is 0x%08x.",dwResult);
	   goto err1;
	}

    s_bus = bus;
	s_cs = cs;
	s_max_hz = max_hz;
	s_spi_mode = spi_mode;
	
    /*5.根据spi slave初始化spi host */
	dwResult = ptspihostdevice->spi_host_driver.spi_claim_bus(spi);
	if (BSP_OK != dwResult) 
	{
	   error("spi_claim_bus failed!ErrCode is 0x%08x.",dwResult);
   	   goto err2;
	}

	/* 6. Read the ID codes */
	dwResult = SpiFlashReadIDCmd(spi, &idcode, sizeof(idcode));
	if (BSP_OK != dwResult)
    {
	   error("SpiFlashReadIDCmd failed!ErrCode is 0x%08x.",dwResult);
   	   goto err2;
	}

	printf("SF: Got idcode %02x %02x %02x.\n", idcode[0],idcode[1], idcode[2]);	

     /* 7. 根据id找到芯片 */
    dwResult = GetSpiFlashInfoFromID(idcode, sizeof(idcode));
	if (BSP_OK != dwResult)
    {
	   error("GetSpiFlashFromID failed!ErrCode is 0x%08x.",dwResult);
   	   goto err2;
	}

	/*8. 挂接spi flash驱动*/
    ptSpiFlash = (T_SPI_FLASH *)malloc(sizeof(T_SPI_FLASH));
    if(NULL == ptSpiFlash)
    {
        dwResult = BSP_ERROR_NULL_POINTER;
        error("malloc failed!ptSpiFlash is NULL!ErrCode is 0x%08x.",dwResult);
        goto err2;
    }

    ptSpiFlash->ptSpiFlashInfo = g_ptSpiFlashAct;
	ptSpiFlash->spi_flash_drv.spi = spi;
	ptSpiFlash->spi_flash_drv.name = g_ptSpiFlashAct->tSpiFlashDevPara.name;
	ptSpiFlash->spi_flash_drv.size = g_ptSpiFlashAct->tSpiFlashDevPara.nr_sectors * g_ptSpiFlashAct->tSpiFlashDevPara.pages_per_sector *\
		                                           g_ptSpiFlashAct->tSpiFlashDevPara.page_size;
	ptSpiFlash->spi_flash_drv.read = sf_read;
	ptSpiFlash->spi_flash_drv.write = sf_write;
	ptSpiFlash->spi_flash_drv.erase = sf_erase;

    printf("Spi Flash:%s,%s;Page Size:%dB;Size:%dMB.",ptSpiFlash->ptSpiFlashInfo->tSpiFlashDevPara.manufacturer_name,\
		                                                                   ptSpiFlash->ptSpiFlashInfo->tSpiFlashDevPara.name,\
		                                                                   ptSpiFlash->ptSpiFlashInfo->tSpiFlashDevPara.page_size,\
		                                                                   ptSpiFlash->spi_flash_drv.size >> 20);

    g_ptspiflash = &(ptSpiFlash->spi_flash_drv);
//  SpiFlashEnter_4B_Mode(g_ptspiflash);
	
	return g_ptspiflash;
err2:
	ptspihostdevice->spi_host_driver.spi_free_slave(spi);
err1:
	if(NULL != ptspihostdevice)
	{
	    free(ptspihostdevice);
	    ptspihostdevice = NULL;
	}
	return NULL;
}

/**************************************************************************
* 函数名称：GetSpiFlashHandle
* 功能描述：获取spi_flash指针
* 访问的表： 无
* 修改的表： 无
* 输入参数： 无
* 输出参数： 无
* 返 回 值：     struct spi_flash *:指向spi_flash指针
* 其它说明： 无
* 修改日期     版本号            修改人	        修改内容
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             创建
**************************************************************************/
struct spi_flash *GetSpiFlashHandle(void)
{
    return g_ptspiflash;
}

/**************************************************************************
* 函数名称：spi_flash_free
* 功能描述：释放spi flash相关数据结构操作
* 访问的表： 无
* 修改的表： 无
* 输入参数： struct spi_flash *flash:指向spi flash结构体；
* 输出参数： 无
* 返 回 值：     无
* 其它说明： 无
* 修改日期     版本号            修改人	        修改内容
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             创建
**************************************************************************/
void spi_flash_free(struct spi_flash *flash)
{
        T_SPI_FLASH  *ptSpiFlash = NULL;

        spi_free_slave(flash->spi);
	ptSpiFlash = to_spi_flash(flash);
	if(NULL != ptSpiFlash)
	{
	    free(ptSpiFlash);
	}

	return;
}

#ifndef CFG_NO_FLASH
/**************************************************************************
* 函数名称：flash_print_info
* 功能描述： 打印Flash型号和每个Sector的起始地址
* 访问的表： flash_info
* 修改的表： 无
* 输入参数： flash_info_t * info  - Flash信息结构体
* 输出参数： 无
* 返 回 值：     无
* 其它说明： 无
* 修改日期    版本号     修改人	     修改内容
* -------------------------------------------------------------------------
* 2010/09/02             V1.0	                韩韬	             创建
**************************************************************************/
void flash_print_info (flash_info_t * info)
{
    unsigned long  ulIdx = 0;

    /* 入参检查*/
    if(NULL == info)
    {
        error("input pointer is NULL!");
		return;
    }

    if (info->flash_id == FLASH_UNKNOWN) 
    {
        printf ("missing or unknown FLASH type\n");
        return;
    }

    switch (info->flash_id & FLASH_VENDMASK) 
    {
    case FLASH_MAN_SPANSION:
        printf("SPANSION ");
		break;

    case FLASH_MAN_MX:
        printf("MXIC ");
		break;

    default:
        printf ("Unknown Vendor ");
    }
	
    switch (info->flash_id & FLASH_TYPEMASK) 
    {
    case FLASH_S25FL512S:
        printf ("S25FL512S (512 Mbit)");
        break;

    default:
        printf ("Unknown Chip Type");
		break;
    }

    printf ("  Size: %ld MB in %d Sectors\n",  info->size >> 20, info->sector_count);
    printf ("  Sector Start Addresses:");
    for (ulIdx = 0; ulIdx < info->sector_count; ulIdx++)
    {
        if (0==(ulIdx & 0x3))
        {
            printf ("\n   ");
        }
        printf (" %08lX%s", info->start[ulIdx], info->protect[ulIdx] ? " (RO)" : "     ");
    }
    printf ("\n");
    return;
}
#endif

