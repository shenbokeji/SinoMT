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
#ifndef _UNI_SLLD_HSD_H_
#define _UNI_SLLD_HSD_H_

#include <common.h>
#include <malloc.h>
#include <spi.h>
#include <spi_flash.h>


#define  SPI_HOST_NAME_NUM      (64)
#define  SPI_HOST_DRIVER_REGISTER   (1)

/* spi host id define */
#define  SPI_HOST_DRA624_ID  (0x01)

/* 定义spi host name和spi host id关联数组*/
typedef struct
{
    char acspi_host_name[SPI_HOST_NAME_NUM];
    unsigned long spi_host_id; 
}T_SPI_HOST_DEVICE;


typedef struct
{
    /* 建立spi slave 函数*/
    struct spi_slave* (*spi_setup_slave)(unsigned int bus, unsigned int cs,unsigned int max_hz, unsigned int mode);
    /*初始化spi bus函数*/
    int (*spi_claim_bus)(struct spi_slave *slave);
    /*释放spi bus函数 */
    void (*spi_release_bus)(struct spi_slave *slave);
    /*spi host 传输数据函数 */
    int (*spi_xfer)(struct spi_slave *slave, unsigned int bitlen,const void *dout, void *din, unsigned long flags);
    /* 释放spi slave 函数*/
    void(* spi_free_slave)(struct spi_slave *slave);
}T_SPI_HOST_DRIVER;

typedef struct
{
    T_SPI_HOST_DEVICE spi_host_device;
    T_SPI_HOST_DRIVER spi_host_driver;
}T_SPI_HOST_DEVICE_ACT;

/* 函数声明*/
unsigned long SpiHostDriverRegister(char *pacspihostname);
unsigned long GetSpiHostActDevice(T_SPI_HOST_DEVICE_ACT ** pptspihostdev);


#endif
