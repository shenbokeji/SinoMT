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
#ifndef _UNI_SLLD_H_
#define _UNI_SLLD_H_

#include <common.h>
#include <malloc.h>
#include <spi.h>
#include <spi_flash.h>

/* SPI Flash Common commands */
#define CMD_READ_ID			(0x9f)

#define SPI_FLASH_ID_NUM    (3)

/* SPANSION-------------------------------------------------------------------------------*/
/* SPANSION 厂家ID宏定义*/
#define  SPANSION_SPI_FLASH_ID   (0x01)

/* SPANSION  厂家FLASH ID宏定义*/
#define  SPANSION_S25FL512S_ID    (0x0220)

/* SPANSION SPI FLASH命令集宏定义*/
#define  SPANSION_SPI_FLASH_4BREAD_CMD   (0x13)
#define  SPANSION_SPI_FLASH_WREN_CMD   (0x06)   
#define  SPANSION_SPI_FLASH_WRDI_CMD   (0x04)   
#define  SPANSION_SPI_FLASH_4PP_CMD   (0x12)   
#define  SPANSION_SPI_FLASH_4SE_CMD   (0xDC)   
#define  SPANSION_SPI_FLASH_BE_CMD   (0x60)   
#define  SPANSION_SPI_FLASH_SOFTWARE_RESET_CMD   (0xF0)
#define  SPANSION_SPI_FLASH_STATUS_REG1_CMD   (0x05)

/* SPANSION spi flash pp 编程超时等待时间,单位是s*/
#define SPANSION_PAGE_PROGRAM_TIMEOUT		(1 * CONFIG_SYS_HZ)
/* SPANSION spi flash se擦除超时等待时间,单位是s*/
#define SPANSION_SECTOR_ERASE_TIMEOUT		(4 * CONFIG_SYS_HZ)

/* MICRON-------------------------------------------------------------------------------*/
/* MICRON 厂家ID宏定义*/
#define  MICRON_SPI_FLASH_ID   (0x20)

/* MICRON  厂家FLASH ID宏定义*/
#define  MICRON_N25Q512_ID    (0xBA20)

/* MICRON SPI FLASH命令集宏定义*/
#define  MICRON_SPI_FLASH_4BREAD_CMD            (0x13)
#define  MICRON_SPI_FLASH_WREN_CMD              (0x06)   
#define  MICRON_SPI_FLASH_WRDI_CMD              (0x04)   
#define  MICRON_SPI_FLASH_4PP_CMD               (0x12)   
#define  MICRON_SPI_FLASH_4SE_CMD               (0xDC)   
#define  MICRON_SPI_FLASH_BE_CMD                (0xC7)   
#define  MICRON_SPI_FLASH_SOFTWARE_RESET_CMD    (0x99)
#define  MICRON_SPI_FLASH_STATUS_REG1_CMD       (0x05)

/* MICRON spi flash pp 编程超时等待时间,单位是s*/
#define MICRON_PAGE_PROGRAM_TIMEOUT		(1 * CONFIG_SYS_HZ)
/* MICRON spi flash se擦除超时等待时间,单位是s*/
#define MICRON_SECTOR_ERASE_TIMEOUT		(3 * CONFIG_SYS_HZ)

/* MACRONIX-------------------------------------------------------------------------------*/
/* MACRONIX 厂家ID宏定义*/
#define  MACRONIX_SPI_FLASH_ID   (0xC2)

/* MACRONIX  厂家FLASH ID宏定义*/
#define  MACRONIX_MX66L51235F_ID    (0x201A)

/* MACRONIX SPI FLASH命令集宏定义*/
#define  MACRONIX_SPI_FLASH_4BREAD_CMD            (0x13)
#define  MACRONIX_SPI_FLASH_WREN_CMD              (0x06)   
#define  MACRONIX_SPI_FLASH_WRDI_CMD              (0x04)   
#define  MACRONIX_SPI_FLASH_4PP_CMD               (0x12)   
#define  MACRONIX_SPI_FLASH_4SE_CMD               (0xDC)   
#define  MACRONIX_SPI_FLASH_BE_CMD                (0xC7)   
#define  MACRONIX_SPI_FLASH_SOFTWARE_RESET_CMD    (0x99)
#define  MACRONIX_SPI_FLASH_STATUS_REG1_CMD       (0x05)

/* MACRONIX spi flash pp 编程超时等待时间,单位是s*/
#define MACRONIX_PAGE_PROGRAM_TIMEOUT		(2 * CONFIG_SYS_HZ)
/* MACRONIX spi flash se擦除超时等待时间,单位是s*/
#define MACRONIX_SECTOR_ERASE_TIMEOUT		(4 * CONFIG_SYS_HZ)

#define  SPI_FLASH_PROBEED  (1)

/* spi flash 设备参数定义*/
typedef struct 
{
    unsigned short idcode1;
    unsigned long page_size;
    unsigned long pages_per_sector;
    unsigned long nr_sectors;
    const char *name;
    const char *manufacturer_name;
}T_SPI_FLASH_DEVICE_PARA;

/* 单个指令结构体*/
typedef struct
{
    unsigned char ucopcode;
    unsigned char ucdummycycle;
}T_SPI_FLASH_CMD;

/* SPI FLASH 基本的指令集结构体*/
typedef struct
{
    T_SPI_FLASH_CMD tread;
    T_SPI_FLASH_CMD twren;
    T_SPI_FLASH_CMD twrdi;
    T_SPI_FLASH_CMD tpp;
    T_SPI_FLASH_CMD tse;
    T_SPI_FLASH_CMD tbe;
    T_SPI_FLASH_CMD tsfreset;
    T_SPI_FLASH_CMD trdstatusreg;
}T_SPI_FLASH_CMD_SET;

/* Spi Flash信息结构体数组*/
typedef struct 
{
    T_SPI_FLASH_DEVICE_PARA  tSpiFlashDevPara;
    T_SPI_FLASH_CMD_SET  tSpiFlashCmdSet;
    unsigned long ulwribitoffset;//write in progress bit offset
    unsigned long ulppwaittime;//page program wait time(s)
    unsigned long ulsewaittime;//sector erase wait time(s)
    unsigned long uladdrlength;//flash access addr lengrh(byte)
}T_SPI_FLASH_INFO;

/* Spi Flash结构体数组*/
typedef struct 
{
    T_SPI_FLASH_INFO  *ptSpiFlashInfo;
    struct spi_flash spi_flash_drv;
}T_SPI_FLASH;

struct spi_flash *GetSpiFlashHandle(void);

#ifndef CFG_NO_FLASH
//int flash_erase (flash_info_t * info, int s_first, int s_last);
//void flash_print_info (flash_info_t * info);
#endif

#endif
