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

/* spi flash����ȫ�ֱ���*/
unsigned long g_ulSpiFlashDebug = 0;

#ifndef CFG_NO_FLASH
flash_info_t  flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips */
#endif

/* spi flash�ṹ�����ָ��*/
static struct spi_flash * g_ptspiflash = NULL;

 /* �ⲿȫ�ֱ���*/
 extern  T_SPI_HOST_DEVICE_ACT *g_ptspihostdevice;
/**************************************************************************
* �������ƣ�SetSpiFlashDebugFlag
* ��������������spi flash�ĵ��Ա�־
* ���ʵı� ��
* �޸ĵı� ��
* ���������unsigned long ulFlag:���Ա�־
* ��������� ��
* �� �� ֵ��     ��
* ����˵���� 0:��ʶ�رյ�����Ϣ��1��ʶ�򿪵�����Ϣ��
* �޸�����     �汾��            �޸���	        �޸�����
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             ����
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
* �������ƣ�init_flash_info
* ������������ʼ��Flash
* ���ʵı� ��
* �޸ĵı� 
* ��������� spi_flash_info:��ǰflash��Ϣ
* �����������
* �� �� ֵ�� ��
* ����˵���� ��
* �޸�����     �汾��            �޸���	        �޸�����
* -------------------------------------------------------------------------
* 2012/09/12            V1.0	               ��С��             ����
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
* �������ƣ�flash_init
* ������������ʼ��Flash
* ���ʵı� ��
* �޸ĵı� flash_info
* ��������� ��
* �����������
* �� �� ֵ��     flash�ֽ�������С��
* ����˵���� ��
* �޸�����     �汾��            �޸���	        �޸�����
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             ����
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
* �������ƣ�flash_real_protect
* ����������flash sector����������
* ���ʵı� ��
* �޸ĵı� ��
* ��������� ��
* �����������
* �� �� ֵ��      0��ʶ�ɹ���������ʶ�����룻
* ����˵���� ��
* �޸�����     �汾��            �޸���	        �޸�����
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             ����
**************************************************************************/
int flash_real_protect (flash_info_t * info, long sector, int prot)
{
    /* Ŀǰ��ʱû��ʵ�֣�������Ҫ��Ӷ�sector��DYB��֧��*/
    return 0;
}
#endif
/**************************************************************************
* �������ƣ�SpiFlashReadIDCmd
* ������������SF��ID
* ���ʵı� ��
* �޸ĵı� ��
* ���������struct spi_slave *spi:ָ��spi slave��
                                 void *response:ָ����������ݣ�
                                 unsigned long len:�������ݵ��ֽ���Ŀ��
* ��������� ��
* �� �� ֵ��     0��ʶ�ɹ���������ʶ�����룻
* ����˵���� ��
* �޸�����     �汾��            �޸���	        �޸�����
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             ����
**************************************************************************/
unsigned long  SpiFlashReadIDCmd(struct spi_slave *spi, void *response, unsigned long len)
{
    unsigned long dwResult = BSP_OK;

    /* ��μ��*/
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
* �������ƣ�SpiFlashSpansionIDParse
* ����������SPANSION spi flash id����
* ���ʵı� ��
* �޸ĵı� ��
* ���������unsigned char *pucID:ָ��id�����ָ�룻
                                 unsigned long len:id����Ĵ�С��
* ��������� ��
* �� �� ֵ��     0��ʶ�ɹ���������ʶ�����룻
* ����˵���� ��
* �޸�����     �汾��            �޸���	        �޸�����
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             ����
**************************************************************************/
unsigned long SpiFlashIDParse(unsigned char *pucID, unsigned long len)
{
    unsigned long  ulindex = 0;
    unsigned long  ulItemSize = 0;
    unsigned short  usID = 0;

    /* ��μ��*/
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
* �������ƣ�GetSpiFlashInfoFromID
* ��������������id��Ϣ��ȡspi flash��Ϣ
* ���ʵı� ��
* �޸ĵı� ��
* ���������unsigned char *pucID:ָ��id�����ָ�룻
                                 unsigned long len:id����Ĵ�С��
* ��������� ��
* �� �� ֵ��     0��ʶ�ɹ���������ʶ�����룻
* ����˵���� ��
* �޸�����     �汾��            �޸���	        �޸�����
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             ����
**************************************************************************/
unsigned long GetSpiFlashInfoFromID(unsigned char *pucID, unsigned long len)
{
    unsigned long  dwResult = BSP_OK;

    /* ��μ��*/
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
* �������ƣ�sf_wait_ready
* �����������ȴ�flas���д�������Ų�������
* ���ʵı� ��
* �޸ĵı� ��
* ��������� struct spi_flash *flash:ָ��spi flash�ṹ�壻
                                  unsigned long timeout:��ʱ��ʱ�䣬��λ���룻
* ��������� ��
* �� �� ֵ��     0��ʶ�ɹ���������ʶ�����룻
* ����˵���� ��
* �޸�����     �汾��            �޸���	        �޸�����
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             ����
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

        /* ��μ��*/
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
* �������ƣ�sf_read
* ����������spi flash�Ķ�����
* ���ʵı� ��
* �޸ĵı� ��
* ��������� struct spi_flash *flash:ָ��spi flash�ṹ�壻
                                  u32 offset:��flash��ƫ�Ƶ�ַ��
                                  size_t len:�������ֽڳ��ȣ�
                                  void *buf:ָ�򱣴�flash�������ݵĻ������׵�ַ��
* ��������� ��
* �� �� ֵ��     0��ʶ�ɹ���������ʶ�����룻
* ����˵���� ��
* �޸�����     �汾��            �޸���	        �޸�����
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             ����
**************************************************************************/
int sf_read(struct spi_flash *flash,u32 offset, size_t len, void *buf)
{
        T_SPI_FLASH  *ptSpiFlash = NULL;
	unsigned char  cmd[5];
	size_t cmd_len = 0;

         /* ��μ��*/
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
* �������ƣ�sf_write
* ����������spi flash��д����
* ���ʵı� ��
* �޸ĵı� ��
* ��������� struct spi_flash *flash:ָ��spi flash�ṹ�壻
                                  u32 offset:дflash��ƫ�Ƶ�ַ��
                                  size_t len:дflash���ֽڳ��ȣ�
                                  void *buf:ָ���д��flash�����ݻ������׵�ַ��
* ��������� ��
* �� �� ֵ��     0��ʶ�ɹ���������ʶ�����룻
* ����˵���� ��
* �޸�����     �汾��            �޸���	        �޸�����
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             ����
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

    /* ��μ��*/
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
* �������ƣ�sf_erase
* ����������spi flash�Ĳ�������
* ���ʵı� ��
* �޸ĵı� ��
* ��������� struct spi_flash *flash:ָ��spi flash�ṹ�壻
                                  u32 offset:flash������ƫ�Ƶ�ַ��
                                  size_t len:flash�������ֽڳ��ȣ�
* ��������� ��
* �� �� ֵ��     0��ʶ�ɹ���������ʶ�����룻
* ����˵���� ��
* �޸�����     �汾��            �޸���	        �޸�����
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             ����
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
	  /* ��μ��*/
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
* �������ƣ�flash_erase
* ��������������flash_info_t�ṹ����Ϣ��ɲ���flash��������
* ���ʵı� flash_info
* �޸ĵı� ��
* ��������� flash_info_t * info:ָ�򱣴�flash��Ϣ�Ľṹ�壻
                                  int s_first:��������ʼflash�����ţ�
                                  int s_last:�����Ľ���flash�����ţ�
* ��������� ��
* �� �� ֵ��     0��ʶ�ɹ���������ʶ�����룻
* ����˵���� ��
* �޸�����     �汾��            �޸���	        �޸�����
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             ����
**************************************************************************/
int flash_erase (flash_info_t * info, int s_first, int s_last)
{
    unsigned long  ulProtect = 0;
    unsigned long  ulSector = 0;
    struct spi_flash * ptspiflash = NULL;
    int ret = 0;
    unsigned long  uloffset = 0;
    unsigned long  ullength = 0;

    /* ����ж�*/
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
    /* �ж�Flash ����*/
    if ((info->flash_id == FLASH_UNKNOWN) ||
	  ((info->flash_id > FLASH_AMD_COMP) &&
	  ((info->flash_id & FLASH_VENDMASK) != FLASH_MAN_INTEL)))
    {
        error ("Can't erase unknown flash type - aborted\n");
        return  BSP_ERROR_INVALID_PARA;
    }
	
    ulProtect = 0;
    /* ͳ�ƴ�������Χ�ڣ��ܱ�����Sector ����*/
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
* �������ƣ�SpiFlashEnter_4B_Mode
* ������������SF��ID
* ���ʵı� ��
* �޸ĵı� ��
* ���������struct spi_slave *spi:ָ��spi slave��
                                 void *response:ָ����������ݣ�
                                 unsigned long len:�������ݵ��ֽ���Ŀ��
* ��������� ��
* �� �� ֵ��     0��ʶ�ɹ���������ʶ�����룻
* ����˵���� ��
* �޸�����     �汾��            �޸���	        �޸�����
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             ����
**************************************************************************/
unsigned long  SpiFlashEnter_4B_Mode(struct spi_flash *flash)
{
    T_SPI_FLASH  *ptSpiFlash = NULL;
	int ret = BSP_OK;
	u8 uccmd = 0;
    u8 flag_status = 0;
	unsigned long uldummy = 0;
	struct spi_slave *spi = NULL;

    /* ��μ��*/
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
* �������ƣ�spi_flash_probe
* ����������̽��spi flash������Ӧ�ĳ�ʼ������
* ���ʵı� g_tSpiFlash
* �޸ĵı� ��
* ��������� unsigned int bus:spi flash�����ĸ�spi bus�ϣ�
                                  unsigned int cs:spi flash����spi bus���ĸ�cs�ϣ�
                                  unsigned int max_hz:spi �����ʱ��Ƶ�ʣ�
                                  unsigned int spi_mode:spi ��ģʽ;
* ��������� ��
* �� �� ֵ��     struct spi_flash *:ָ��̽�⵽��spi flash�ṹ�壻
* ����˵���� ��
* �޸�����     �汾��            �޸���	        �޸�����
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             ����
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
		
    /* spi flash�Ѿ������˳�ʼ������*/
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
            /* free ���е�ָ�룬���³�ʼ��*/
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
	   
	/*1. ��Ҫ�����ش��룬����ĳ����Ϣ��ȡspi host id���ҽ�spi host driver */
	strcpy(g_acspi_host_name,"DRA624");

	/*2.����spi host nameע������*/
	dwResult = SpiHostDriverRegister(g_acspi_host_name);
	if(BSP_OK != dwResult)
	{
		error("SpiHostDriverRegister failed!ErrCode is 0x%08x.",dwResult);
		return  NULL;
	}
	   
	/*3.��ȡspi host act dev���*/
    dwResult = GetSpiHostActDevice(&ptspihostdevice);
	if(BSP_OK != dwResult)
	{
		error("GetSpiHostActDevice failed!ErrCode is 0x%08x.",dwResult);
		goto err1;    
	}

	/*4.����spi slave */
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
	
    /*5.����spi slave��ʼ��spi host */
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

     /* 7. ����id�ҵ�оƬ */
    dwResult = GetSpiFlashInfoFromID(idcode, sizeof(idcode));
	if (BSP_OK != dwResult)
    {
	   error("GetSpiFlashFromID failed!ErrCode is 0x%08x.",dwResult);
   	   goto err2;
	}

	/*8. �ҽ�spi flash����*/
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
* �������ƣ�GetSpiFlashHandle
* ������������ȡspi_flashָ��
* ���ʵı� ��
* �޸ĵı� ��
* ��������� ��
* ��������� ��
* �� �� ֵ��     struct spi_flash *:ָ��spi_flashָ��
* ����˵���� ��
* �޸�����     �汾��            �޸���	        �޸�����
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             ����
**************************************************************************/
struct spi_flash *GetSpiFlashHandle(void)
{
    return g_ptspiflash;
}

/**************************************************************************
* �������ƣ�spi_flash_free
* �����������ͷ�spi flash������ݽṹ����
* ���ʵı� ��
* �޸ĵı� ��
* ��������� struct spi_flash *flash:ָ��spi flash�ṹ�壻
* ��������� ��
* �� �� ֵ��     ��
* ����˵���� ��
* �޸�����     �汾��            �޸���	        �޸�����
* -------------------------------------------------------------------------
* 2012/06/20            V1.0	               zhanglr	             ����
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
* �������ƣ�flash_print_info
* ���������� ��ӡFlash�ͺź�ÿ��Sector����ʼ��ַ
* ���ʵı� flash_info
* �޸ĵı� ��
* ��������� flash_info_t * info  - Flash��Ϣ�ṹ��
* ��������� ��
* �� �� ֵ��     ��
* ����˵���� ��
* �޸�����    �汾��     �޸���	     �޸�����
* -------------------------------------------------------------------------
* 2010/09/02             V1.0	                ���	             ����
**************************************************************************/
void flash_print_info (flash_info_t * info)
{
    unsigned long  ulIdx = 0;

    /* ��μ��*/
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

