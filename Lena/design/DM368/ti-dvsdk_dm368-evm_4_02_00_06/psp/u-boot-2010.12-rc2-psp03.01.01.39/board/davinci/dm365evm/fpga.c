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

#define GPIO_RESET		9
#define GPIO_DCLK		6
#define GPIO_nSTATUS	8
#define GPIO_CONF_DONE	11
#define GPIO_nCONFIG	7
#define GPIO_DATA0		10


DECLARE_GLOBAL_DATA_PTR;

/* return FPGA_SUCCESS on success, else FPGA_FAIL
 */
int dm365_init_fpga(void)
{

	fpga_debug("Initializing FPGA interface\n");
	fpga_init();
	fpga_add(fpga_altera, &cyclone2);

	/* Configure PINMUX 3 to enable FPGA pins */
	writel((readl(PINMUX3) & 0xFFFFF81F), PINMUX3);

	
	writel(0x04222385, DAVINCI_ASYNC_EMIF_CNTRL_BASE+0x10);

	/* set up outputs */
	gpio_direction_output(GPIO_DCLK,  0);
	gpio_direction_output(GPIO_nCONFIG, 0);
	gpio_direction_output(GPIO_DATA0, 0);
	gpio_direction_output(GPIO_RESET, 1);

	/* NB omap_free_gpio() resets to an input, so we can't
	 * free ie. nCONFIG, or else the FPGA would reset
	 * Q: presumably gpio_free() has the same effect?
	 */

	/* set up inputs */
	gpio_direction_input(GPIO_nSTATUS);
#ifndef CONFIG_SYS_FPGA_DONT_USE_CONF_DONE
	gpio_direction_input(GPIO_CONF_DONE);
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
#if 0
int fpga_wr_fn(const void *buf, size_t len, int flush, int cookie)
{
	unsigned char *data = (unsigned char *) buf;
	int i;
	int headerlen = len - cyclone2.size;

	if (headerlen < 0)
		return FPGA_FAIL;
	else if (headerlen == sizeof(uint32_t)) {
		const unsigned int fpgavers_len = 11; /* '0x' + 8 hex digits + \0 */
		char fpgavers_str[fpgavers_len];
		sprintf(fpgavers_str, "0x%08x",
				be32_to_cpup((uint32_t*)data));
		setenv("fpgavers", fpgavers_str);
	}

	fpga_debug("fpga_wr: buf %p / size %d\n", buf, len);
	for (i = headerlen; i < len; i++)
		_write_fpga(data[i]);
	fpga_debug("-%s\n", __func__);

	return FPGA_SUCCESS;
}
#else 

int fpga_wr_fn(const void *buf, size_t len, int flush, int cookie)
{
			unsigned char *data = (unsigned char *) buf;
			int i;
			size_t bytecount = 0;
			//int headerlen = len - cyclone2.size;

		/* Load the data */
		while (bytecount < len) {
			unsigned char val=0;

			/* Altera detects an error if INIT goes low (active)
			   while DONE is low (inactive) */

			val = data [bytecount ++ ];
			i = 8;
			do {
				#if 0
				/* Deassert the clock */
				(*fn->clk) (false, true, cookie);
				CONFIG_FPGA_DELAY ();
				/* Write data */
				//(*fn->data) ((val & 0x01), true, cookie);
				gpio_set_value(GPIO_DATA0, val & 0x01);
				CONFIG_FPGA_DELAY ();
				/* Assert the clock */
				(*fn->clk) (true, true, cookie);
				CONFIG_FPGA_DELAY ();
				#else
				/* Write data */
				gpio_set_value(GPIO_DATA0, val & 0x01);
				//udelay(1);
					/* clock */
				gpio_set_value(GPIO_DCLK, 1);
				//udelay(1);
				gpio_set_value(GPIO_DCLK, 0);
				//udelay(1);		

				#endif
				val >>= 1;
				i --;
			} while (i > 0);

#if 1
			if (bytecount % (len / 40) == 0)
				putc ('.');		/* let them know we are alive */
#endif
		}

		return FPGA_SUCCESS;
}
#endif

int dm365_reset_fpga(void)
{
	gpio_set_value(GPIO_RESET, 1);
	udelay(500);
	gpio_set_value(GPIO_RESET, 0);
	udelay(500);
	gpio_set_value(GPIO_RESET, 1);
	return 0;

}
