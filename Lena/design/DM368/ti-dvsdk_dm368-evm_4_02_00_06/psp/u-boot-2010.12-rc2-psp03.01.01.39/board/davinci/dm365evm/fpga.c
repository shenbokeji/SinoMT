/*
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 * Keith Outwater, keith_outwater@mvis.com.
 *
 * (C) Copyright 2011
 * Andre Schwarz, Matrix Vision GmbH, andre.schwarz@matrix-vision.de
 * Michael Jones, Matrix Vision GmbH, michael.jones@matrix-vision.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <ACEX1K.h>
#include <command.h>
#include <asm/arch/gpio_defs.h>
#include <asm/arch/hardware.h>
#include <asm/io.h>
#include <linux/byteorder/generic.h>
#include "fpga.h"

#ifdef FPGA_DEBUG
#define fpga_debug(fmt, args...)      printf("%s: "fmt, __func__, ##args)
#else
#define fpga_debug(fmt, args...)
#endif

Altera_CYC2_Passive_Serial_fns altera_fns = {
	fpga_null_fn,   /* Altera_pre_fn */
	fpga_config_fn,
	fpga_status_fn,
	fpga_done_fn,
	fpga_wr_fn,
	fpga_null_fn,
	fpga_null_fn,
};

Altera_desc cyclone2 = {
	Altera_CYC2,
	passive_serial,
	Altera_EP2C20_SIZE,
	(void *) &altera_fns,
	NULL,
	0
};
#define LENA_2016//this is the second version of LENA
//#undef LENA_2016//this is the first version for LENA
#ifndef LENA_2016
#define GPIO_RESET	(9)
#define GPIO_DCLK	(6)
#define GPIO_nSTATUS	(8)
#define GPIO_CONF_DONE	(11)
#define GPIO_nCONFIG	(7)
#define GPIO_DATA0	(10)
#else
#define GPIO_RESET	(9)
#define GPIO_DCLK	(28)
#define GPIO_nSTATUS	(8)
#define GPIO_CONF_DONE	(11)
#define GPIO_nCONFIG	(7)
#define GPIO_DATA0	(27)
#define GPIO_DATA0_BUG	(10)//hardware design bug, GPIO10 must be input
#endif


DECLARE_GLOBAL_DATA_PTR;

/* return FPGA_SUCCESS on success, else FPGA_FAIL
 */
int dm365_init_fpga(void)
{

	fpga_debug("Initializing FPGA interface\n");
	fpga_init();
	fpga_add(fpga_altera, &cyclone2);

	/* Configure PINMUX 3 to enable FPGA pins */
	writel( (readl(PINMUX3) & 0xFFFFF81F) , PINMUX3 );
	writel( (readl(PINMUX4) & 0xFFFFFFF0) , PINMUX4 );
	
	//EMIF time config
	writel(0x0c7669b5, DAVINCI_ASYNC_EMIF_CNTRL_BASE+0x10);

	/* set up outputs */
	gpio_direction_output(GPIO_DCLK,  0);
	gpio_direction_output(GPIO_nCONFIG, 0);
	gpio_direction_output(GPIO_DATA0, 0);
	gpio_direction_output(GPIO_RESET, 1);

	/* set up inputs */
	gpio_direction_input(GPIO_nSTATUS);
#ifndef CONFIG_SYS_FPGA_DONT_USE_CONF_DONE
	gpio_direction_input(GPIO_CONF_DONE);
#endif
#ifdef LENA_2016
	gpio_direction_input(GPIO_DATA0_BUG);//for bug
#endif	
	fpga_config_fn(0, 1, 0);
	udelay(60);

	return FPGA_SUCCESS;
}

int fpga_null_fn(int cookie)
{
	return 0;
}

int fpga_config_fn(int assert, int flush, int cookie)
{
	fpga_debug("SET config : %s=%d\n", assert ? "low" : "high", assert);
	if (flush) {
		gpio_set_value(GPIO_nCONFIG, !assert);
		udelay(1);
		gpio_set_value(GPIO_nCONFIG, assert);
	}

	return assert;
}

int fpga_done_fn(int cookie)
{
	int result = 0;

	/* since revA of BLX, we will not get this signal. */
	udelay(10);
#ifdef CONFIG_SYS_FPGA_DONT_USE_CONF_DONE
	fpga_debug("not waiting for CONF_DONE.");
	result = 1;
#else
	fpga_debug("CONF_DONE check ... ");
	if (gpio_get_value(GPIO_CONF_DONE))  {
		fpga_debug("high\n");
		result = 1;
	} else
		fpga_debug("low\n");
	//gpio_free(GPIO_CONF_DONE);
#endif

	return result;
}

int fpga_status_fn(int cookie)
{
	int result = 0;
	fpga_debug("STATUS check ... ");

	result = gpio_get_value(GPIO_nSTATUS);

	if (result < 0)
		fpga_debug("error\n");
	else if (result > 0)
		fpga_debug("high\n");
	else
		fpga_debug("low\n");

	return result;
}

static inline int _write_fpga(u8 byte)
{
	gpio_set_value(GPIO_DATA0, byte & 0x01);


	/* clock */
	gpio_set_value(GPIO_DCLK, 1);
	udelay(1);
	gpio_set_value(GPIO_DCLK, 0);
	udelay(1);

	return 0;
}
/**************************************************************************
 函数名称：fpga_wr_fn 
 功能描述：load fpga ,write gpio interface
 输入参数：
 返   回：
 
**************************************************************************/
#define FPGA_SET_DATA_ADDR (0x01C67018)
#define FPGA_CLR_DATA_ADDR (0x01C6701C)

#ifdef LENA_2016
#define  gpio_set_fpgadata(addr)  (*(volatile unsigned int *)(addr)=0x08000000)
#define  gpio_set_fpgaclk(addr)  (*(volatile unsigned int *)(addr)=0x10000000)

#else
#define  gpio_set_fpgadata(addr)  (*(volatile unsigned int *)(addr)=0x400)
#define  gpio_set_fpgaclk(addr)  (*(volatile unsigned int *)(addr)=0x40)

#endif
int fpga_wr_fn(const void *buf, size_t len, int flush, int cookie)
{
			unsigned char *data = (unsigned char *) buf;
			size_t bytecount = 0;

		/* Load the data */
		while (bytecount < len) {
			unsigned char val = 0;
			unsigned char valbit[8] = {0};
			/* Altera detects an error if INIT goes low (active)
			   while DONE is low (inactive) */

			val = data [bytecount ++ ];
			valbit[0] = val & 0x01;
			valbit[1] = val & 0x02;
			valbit[2] = val & 0x04;
			valbit[3] = val & 0x08;
			valbit[4] = val & 0x10;
			valbit[5] = val & 0x20;
			valbit[6] = val & 0x40;
			valbit[7] = val & 0x80;				
			if( valbit[0] )
			{
				gpio_set_fpgadata( FPGA_SET_DATA_ADDR );
			}
			else
			{
				gpio_set_fpgadata( FPGA_CLR_DATA_ADDR );
			}
     		/* clock */;
			gpio_set_fpgaclk(FPGA_SET_DATA_ADDR);
    		gpio_set_fpgaclk(FPGA_CLR_DATA_ADDR);
			if( valbit[1] )
			{
				gpio_set_fpgadata( FPGA_SET_DATA_ADDR );
			}
			else
			{
				gpio_set_fpgadata( FPGA_CLR_DATA_ADDR );
			}
     		/* clock */;
			gpio_set_fpgaclk(FPGA_SET_DATA_ADDR);
    		gpio_set_fpgaclk(FPGA_CLR_DATA_ADDR);

			if( valbit[2] )
			{
				gpio_set_fpgadata( FPGA_SET_DATA_ADDR );
			}
			else
			{
				gpio_set_fpgadata( FPGA_CLR_DATA_ADDR );
			}
     		/* clock */;
			gpio_set_fpgaclk(FPGA_SET_DATA_ADDR);
    		gpio_set_fpgaclk(FPGA_CLR_DATA_ADDR);

			if( valbit[3] )
			{
				gpio_set_fpgadata( FPGA_SET_DATA_ADDR );
			}
			else
			{
				gpio_set_fpgadata( FPGA_CLR_DATA_ADDR );
			}

     		/* clock */;
			gpio_set_fpgaclk(FPGA_SET_DATA_ADDR);
    		gpio_set_fpgaclk(FPGA_CLR_DATA_ADDR);				
			if( valbit[4] )
			{
				gpio_set_fpgadata( FPGA_SET_DATA_ADDR );
			}
			else
			{
				gpio_set_fpgadata( FPGA_CLR_DATA_ADDR );
			}

     		/* clock */;
			gpio_set_fpgaclk(FPGA_SET_DATA_ADDR);
    		gpio_set_fpgaclk(FPGA_CLR_DATA_ADDR);
			if( valbit[5] )
			{
				gpio_set_fpgadata( FPGA_SET_DATA_ADDR );
			}
			else
			{
				gpio_set_fpgadata( FPGA_CLR_DATA_ADDR );
			}
     		/* clock */;
			gpio_set_fpgaclk(FPGA_SET_DATA_ADDR);
    		gpio_set_fpgaclk(FPGA_CLR_DATA_ADDR);

			if( valbit[6] )
			{
				gpio_set_fpgadata( FPGA_SET_DATA_ADDR );
			}
			else
			{
				gpio_set_fpgadata( FPGA_CLR_DATA_ADDR );
			}
     		/* clock */;
			gpio_set_fpgaclk(FPGA_SET_DATA_ADDR);
    		gpio_set_fpgaclk(FPGA_CLR_DATA_ADDR);

			if( valbit[7] )
			{
				gpio_set_fpgadata( FPGA_SET_DATA_ADDR );
			}
			else
			{
				gpio_set_fpgadata( FPGA_CLR_DATA_ADDR );
			}
     		/* clock */;
			gpio_set_fpgaclk(FPGA_SET_DATA_ADDR);
    		gpio_set_fpgaclk(FPGA_CLR_DATA_ADDR);

			//if ( 0 == ( bytecount & 0X7FFFF ) )
				//putc ('.');		/* let them know we are alive */

		}

		return FPGA_SUCCESS;
}


int dm365_reset_fpga(void)
{
	gpio_set_value(GPIO_RESET, 1);
	udelay(500);
	gpio_set_value(GPIO_RESET, 0);
	udelay(500);
	gpio_set_value(GPIO_RESET, 1);
	return 0;

}
