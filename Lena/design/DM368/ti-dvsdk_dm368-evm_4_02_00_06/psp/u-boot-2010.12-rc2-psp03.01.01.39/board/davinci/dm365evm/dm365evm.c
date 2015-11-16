/*
 * Copyright (C) 2009 Texas Instruments Incorporated
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <nand.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/emif_defs.h>
#include <asm/arch/nand_defs.h>
#include <asm/arch/gpio_defs.h>
#include <netdev.h>
#include "../common/misc.h"
#ifdef CONFIG_DAVINCI_MMC
#include <mmc.h>
#include <asm/arch/sdmmc_defs.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define GPIO_AD9363_RESET (43)

int board_init(void)
{
unsigned short dummy=0;
 gd->bd->bi_arch_number = MACH_TYPE_DAVINCI_DM365_EVM;
 gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;
 /* PINMUX4 for AIR GROUND FLAG to GPIO29  */
 writel((readl(PINMUX4) & 0xFFFFFFCF), PINMUX4);
 /* PINMUX4 for i2c  */
 writel((readl(PINMUX4) & 0x3CFFFFFF), PINMUX4);
 /* PINMUX4 for SPI4_SCS to GPIO37  */
 writel((readl(PINMUX4) & 0xFFCFFFFF), PINMUX4);
 //
 writel((readl(PINMUX4) & 0x3CCFFFCF), PINMUX4);
 
 //config GPIO43 
 writel((readl(PINMUX0) & 0xFFFCFFFF), PINMUX0);
 //release AD9363 reset by config GPIO43 0
 //gpio_direction_output( GPIO_AD9363_RESET,  1 );
 gpio_direction_input( GPIO_AD9363_RESET );
 //release the lena ground AD9363 reset, for the ground i/o bug
 dummy = *(volatile unsigned short *) ( ( 0 << 1 ) + 0X2002000 );
 return 0;

}

/*
 * Routine: misc_init_r
 * Description: Configure board specific parts
 */
int misc_init_r(void)
{
	printf("DM365 FPGA init OK!\n");

	dm365_init_fpga();

	return 0;
}



