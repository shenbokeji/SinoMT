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
#ifndef _UNI_SLLD_HAL_H_
#define _UNI_SLLD_HAL_H_

#include <common.h>

/* º¯ÊýÉùÃ÷*/
int sf_cmd(struct spi_slave *spi, u8 cmd, void *response, size_t len,unsigned long uldummycycle);
int sf_read_common(struct spi_flash *flash, const u8 *cmd, size_t cmd_len, void *data, size_t data_len);
int sf_cmd_write(struct spi_flash *flash, const u8 *cmd,size_t cmd_len, const void *data, size_t data_len);
T_SPI_FLASH *to_spi_flash(struct spi_flash *flash);



#endif
