/*
 * Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Driver for SPI controller on DaVinci. Based on atmel_spi.c
 * by Atmel Corporation
 *
 * Copyright (C) 2007 Atmel Corporation
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <spi.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include "davinci_spi.h"

#define CONFIG_SYS_SPI0_BASE  (0x01c66000)
#define CONFIG_SYS_SPI4_BASE  (0x01c23000)

#define CONFIG_SYS_SPI0_CLK  (170000000)
#define CONFIG_SYS_SPI4_CLK  (24000000)

/*SPI NUMBER*/
#define SPI0_BUS (0)
#define SPI4_BUS (4)
#define GPIO_SPI0_CS0 (25)
#define GPIO_SPI4_CS0 (37)

void spi_init()
{
	/* do nothing */
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
			unsigned int max_hz, unsigned int mode)
{
	struct davinci_spi_slave	*ds;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;
//printf("%s:%d\n",__FUNCTION__,__LINE__);
	ds = malloc(sizeof(*ds));
	if (!ds)
		return NULL;

	ds->slave.bus = bus;
	ds->slave.cs = cs;

	if( SPI0_BUS == bus ) 
	{
		ds->regs = (struct davinci_spi_regs *)CONFIG_SYS_SPI0_BASE;
	}
	else  if( SPI4_BUS == bus ) 
	{
		ds->regs = (struct davinci_spi_regs *)CONFIG_SYS_SPI4_BASE;
	}
	ds->freq = max_hz;
//printf("%s:%d\n",__FUNCTION__,__LINE__);
//printf("bus=%x  cs=%x ds addr =%x  reg=%x\n",bus,cs,ds,ds->regs);

	return &ds->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct davinci_spi_slave *ds = to_davinci_spi(slave);

	free(ds);
}


int spi_claim_bus( struct spi_slave *slave )
{
	struct davinci_spi_slave *ds = to_davinci_spi(slave);
	unsigned int scalar;
	unsigned int uiBus; 
	unsigned int uiGPIO; 

	uiBus = slave->bus;
	//set GPIO and clock
	if( SPI0_BUS == uiBus ) 
	{
		uiGPIO = GPIO_SPI0_CS0;
		scalar = ( ( CONFIG_SYS_SPI0_CLK / ds->freq) - 1 ) & 0xFF;
	}
	else  if( SPI4_BUS == uiBus ) 
	{
		uiGPIO = GPIO_SPI4_CS0;
		 scalar = ( ( CONFIG_SYS_SPI4_CLK / ds->freq) - 1 ) & 0xFF;
	}
	
	gpio_direction_output( uiGPIO,  0 );
	
	/* Enable the SPI hardware */
	writel(SPIGCR0_SPIRST_MASK, &ds->regs->gcr0);
	udelay(1000);
	writel(SPIGCR0_SPIENA_MASK, &ds->regs->gcr0);

	/* Set master mode, powered up and not activated */
	writel(SPIGCR1_MASTER_MASK | SPIGCR1_CLKMOD_MASK, &ds->regs->gcr1);

	/* CS, CLK, SIMO and SOMI are functional pins */
	writel((SPIPC0_EN0FUN_MASK | SPIPC0_CLKFUN_MASK |
		SPIPC0_DOFUN_MASK | SPIPC0_DIFUN_MASK), &ds->regs->pc0);
	
	//set GPIO and clock
	if( SPI0_BUS == uiBus ) 
	{
		//uiGPIO = GPIO_SPI0_CS0;
		//scalar = ( ( CONFIG_SYS_SPI0_CLK / ds->freq) - 1 ) & 0xFF;
			/*
	 * Use following format:
	 *   character length = 8,
	 *   clock signal delayed by half clk cycle,
	 *   clock low in idle state - Mode 1,
	 *   MSB shifted out first
	 */
	 writel(8 | (scalar << SPIFMT_PRESCALE_SHIFT) |
		(1 << SPIFMT_PHASE_SHIFT), &ds->regs->fmt0);
	
	}
	else  if( SPI4_BUS == uiBus ) 
	{
		//uiGPIO = GPIO_SPI4_CS0;
		// scalar = ( ( CONFIG_SYS_SPI4_CLK / ds->freq) - 1 ) & 0xFF;
	 	/*
	* Use following format:
	*   character length = 8,
	*   clock signal delayed by half clk cycle,
	*   clock low in idle state - Mode 0,
	*   MSB shifted out first
	*/
	writel( 8 | (scalar << SPIFMT_PRESCALE_SHIFT) |
			(0 << SPIFMT_PHASE_SHIFT), &ds->regs->fmt0);
	}
	

	/*
	 * Including a minor delay. No science here. Should be good even with
	 * no delay
	 */
	writel((50 << SPI_C2TDELAY_SHIFT) |
		(50 << SPI_T2CDELAY_SHIFT), &ds->regs->delay);

	/* default chip select register */
	writel(SPIDEF_CSDEF0_MASK, &ds->regs->def);

	/* no interrupts */
	writel(0, &ds->regs->int0);
	writel(0, &ds->regs->lvl);

	/* enable SPI */
	writel((readl(&ds->regs->gcr1) | SPIGCR1_SPIENA_MASK), &ds->regs->gcr1);

	//printf("bus=%x  cs=%x ds addr =%x  reg=%x  \n",uiBus,uiGPIO,ds,ds->regs);
	//printf("fmt0  reg=%x  \n",readl( &ds->regs->fmt0));
	
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	struct davinci_spi_slave *ds = to_davinci_spi(slave);

	/* Disable the SPI hardware */
	writel(SPIGCR0_SPIRST_MASK, &ds->regs->gcr0);
}

/*
 * This functions needs to act like a macro to avoid pipeline reloads in the
 * loops below. Use always_inline. This gains us about 160KiB/s and the bloat
 * appears to be zero bytes (da830).
 */
__attribute__((always_inline))
static inline u32 davinci_spi_xfer_data(struct davinci_spi_slave *ds, u32 data)
{
	u32	buf_reg_val;

	/* send out data */
	writel(data, &ds->regs->dat1);

	/* wait for the data to clock in/out */
	while ((buf_reg_val = readl(&ds->regs->buf)) & SPIBUF_RXEMPTY_MASK)
		;

	return buf_reg_val;
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
	     const void *dout, void *din, unsigned long flags)
{ 
 struct davinci_spi_slave *ds = to_davinci_spi(slave);
 //struct pl022 *pl022 = (struct pl022 *)ps->regs; 
 u32 len_tx = 0, len_rx = 0, len; 
 u32 ret = 0; 
 const u8 *txp = dout; 
 u8 *rxp = din, value; 
 
 if (bitlen == 0) 
 /* Finish any previously submitted transfers */ 
 goto out; 
  //printf("%s:%d\n",__FUNCTION__,__LINE__);
 /* 
 * TODO: The controller can do non-multiple-of-8 bit 
 * transfers, but this driver currently doesn't support it. 
 * 
 * It's also not clear how such transfers are supposed to be 
 * represented as a stream of bytes...this is a limitation of 
 * the current SPI interface. 
 */ 
 if (bitlen % 8) { 
 ret = -1; 
  //printf("%s:%d\n",__FUNCTION__,__LINE__);
 /* Errors always terminate an ongoing transfer */ 
 flags |= SPI_XFER_END; 
 goto out; 
 } 
 
 len = bitlen / 8; 
  //printf("%s:%d\n",__FUNCTION__,__LINE__);
 if (flags & SPI_XFER_BEGIN) 
 spi_cs_activate(slave); 
 
 while (len_tx < len) { 
 if (readl(&ds->regs->flg) & 0x200) { 
 value = (txp != NULL) ? *txp++ : 0; 
 //writew(value, SSP_DR_REG); 
   //printf("ssp_sr=%x\n",IO_READ(SSP_SR_REG));
  writel(value, &ds->regs->dat1);
 len_tx++; 
 } 
  //printf("%s:%d\n",__FUNCTION__,__LINE__);
 if (readl(&ds->regs->flg) & 0x100)  { 
 value = readl(&ds->regs->buf); 
 //printf("read back=%x\n",value);
 if (rxp) 
 *rxp++ = value; 
 len_rx++; 
 } 
 } 
  //printf("%s:%d\n",__FUNCTION__,__LINE__);
 while (len_rx < len_tx) { 
 if (readl(&ds->regs->flg) & 0x100)  { 
 value = readl(&ds->regs->buf); 
 if (rxp) 
 *rxp++ = value; 
 len_rx++; 
 } 
 } 
  //printf("%s:%d\n",__FUNCTION__,__LINE__);
out: 
 if (flags & SPI_XFER_END) 
 spi_cs_deactivate(slave); 
  //printf("%s:%d\n",__FUNCTION__,__LINE__);
 return ret; 
} 

int spi_cs_is_valid(unsigned int iBus, unsigned int iCS )
{
	int iReturn;
	iReturn = (( SPI0_BUS == iBus ) || ( SPI4_BUS == iBus ) ) && ( 0 == iCS );
	return iReturn;
}

void spi_cs_activate(struct spi_slave *slave)
{
	unsigned int uiBus; 
	unsigned int uiGPIO; 
	uiBus = slave->bus;
	//set SPIX CS0 , in fact GPIO 
	if( SPI0_BUS == uiBus ) 
	{
		uiGPIO = GPIO_SPI0_CS0;
	}
	else  if( SPI4_BUS == uiBus ) 
	{
		uiGPIO = GPIO_SPI4_CS0;
	}
	gpio_direction_output( uiGPIO,  0 );
	return ;
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	unsigned int uiBus; 
	unsigned int uiGPIO; 
	uiBus = slave->bus;
	//set SPIX CS0 , in fact GPIO 
	if( SPI0_BUS == uiBus ) 
	{
		uiGPIO = GPIO_SPI0_CS0;
	}
	else  if( SPI4_BUS == uiBus ) 
	{
		uiGPIO = GPIO_SPI4_CS0;
	}
	gpio_direction_output( uiGPIO,  1 );
	udelay( 50 );
	return ;
}
