/*
 * TI DaVinci DM365 EVM board support
 *
 * Copyright (C) 2009 Texas Instruments Incorporated
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/i2c.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/i2c/at24.h>
#include <linux/i2c/tsc2004.h>
#include <linux/leds.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand.h>
#include <linux/input.h>
#include <linux/spi/spi.h>
#include <linux/spi/eeprom.h>

#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <mach/mux.h>
#include <mach/hardware.h>
#include <mach/dm365.h>
#include <mach/psc.h>
#include <mach/common.h>
#include <mach/i2c.h>
#include <mach/serial.h>
#include <mach/mmc.h>
#include <mach/mux.h>
#include <mach/nand.h>
#include <mach/keyscan.h>
#include <mach/gpio.h>
#include <mach/cputype.h>
#include <linux/videodev2.h>
#include <media/tvp514x.h>
#include <media/tvp7002.h>
#include <media/davinci/videohd.h>

static inline int have_tvp5150(void)
{
#ifdef CONFIG_VIDEO_TVP5150
	return 1;
#else
	return 0;
#endif
}
static inline int have_adv7611(void)
{
//#ifdef CONFIG_VIDEO_ADV7611
	return 1;
//#else
//	return 0;
//#endif
}


#define DM365_ASYNC_EMIF_CONTROL_BASE	0x01d10000
#define DM365_ASYNC_EMIF_DATA_CE0_BASE	0x02002000
#define DM365_ASYNC_EMIF_DATA_CE1_BASE	0x04000000

#define DM365_EVM_PHY_MASK		(0x2)
#define DM365_EVM_MDIO_FREQUENCY	(2200000) /* PHY bus frequency */

/*
 * A MAX-II CPLD is used for various board control functions.
 */
#define CPLD_OFFSET(a13a8,a2a1)		(((a13a8) << 10) + ((a2a1) << 3))

#define CPLD_VERSION	CPLD_OFFSET(0,0)	/* r/o */
#define CPLD_TEST	CPLD_OFFSET(0,1)
#define CPLD_LEDS	CPLD_OFFSET(0,2)
#define CPLD_MUX	CPLD_OFFSET(0,3)
#define CPLD_SWITCH	CPLD_OFFSET(1,0)	/* r/o */
#define CPLD_POWER	CPLD_OFFSET(1,1)
#define CPLD_VIDEO	CPLD_OFFSET(1,2)
#define CPLD_CARDSTAT	CPLD_OFFSET(1,3)	/* r/o */

#define CPLD_DILC_OUT	CPLD_OFFSET(2,0)
#define CPLD_DILC_IN	CPLD_OFFSET(2,1)	/* r/o */

#define CPLD_IMG_DIR0	CPLD_OFFSET(2,2)
#define CPLD_IMG_MUX0	CPLD_OFFSET(2,3)
#define CPLD_IMG_MUX1	CPLD_OFFSET(3,0)
#define CPLD_IMG_DIR1	CPLD_OFFSET(3,1)
#define CPLD_IMG_MUX2	CPLD_OFFSET(3,2)
#define CPLD_IMG_MUX3	CPLD_OFFSET(3,3)
#define CPLD_IMG_DIR2	CPLD_OFFSET(4,0)
#define CPLD_IMG_MUX4	CPLD_OFFSET(4,1)
#define CPLD_IMG_MUX5	CPLD_OFFSET(4,2)

#define CPLD_RESETS	CPLD_OFFSET(4,3)

#define CPLD_TS_CFG	CPLD_OFFSET(5,0)

#define CPLD_CCD_DIR1	CPLD_OFFSET(0x3e,0)
#define CPLD_CCD_IO1	CPLD_OFFSET(0x3e,1)
#define CPLD_CCD_DIR2	CPLD_OFFSET(0x3e,2)
#define CPLD_CCD_IO2	CPLD_OFFSET(0x3e,3)
#define CPLD_CCD_DIR3	CPLD_OFFSET(0x3f,0)
#define CPLD_CCD_IO3	CPLD_OFFSET(0x3f,1)

#define VIDEO_INPUT_MUX_MASK	0x3
#define VIDEO_INPUT_MUX_TVP5150	0x1
#define VIDEO_INPUT_MUX_ADV7611	0x2


void __iomem *fpga;


#define	LENA_AIR 	 	(1)
#define	LENA_GROUND  	(0)
#define	LENA_NULL  		(2)

unsigned char device_lena_air_id=LENA_NULL;


/* NOTE:  this is geared for the standard config, with a socketed
 * 2 GByte Micron NAND (MT29F16G08FAA) using 128KB sectors.  If you
 * swap chips, maybe with a different block size, partitioning may
 * need to be changed.
 */
/*define NAND_BLOCK_SIZE		SZ_128K*/

/* For Samsung 4K NAND (K9KAG08U0M) with 256K sectors */
/*#define NAND_BLOCK_SIZE		SZ_256K*/

/* For Micron 4K NAND with 512K sectors */
#define NAND_BLOCK_SIZE		SZ_512K

static struct mtd_partition davinci_nand_partitions[] = {
	{
		/* UBL (a few copies) plus U-Boot */
		.name		= "bootloader",
		.offset		= 0,
		.size		= 30 * NAND_BLOCK_SIZE,
		.mask_flags	= MTD_WRITEABLE, /* force read-only */
	}, {
		/* U-Boot environment */
		.name		= "params",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 2 * NAND_BLOCK_SIZE,
		.mask_flags	= 0,
	}, {
		.name		= "kernel",
		.offset		= MTDPART_OFS_APPEND,
		.size		= SZ_4M,
		.mask_flags	= 0,
	}, {
		.name		= "filesystem1",
		.offset		= MTDPART_OFS_APPEND,
		.size		= SZ_512M,
		.mask_flags	= 0,
	}, {
		.name		= "filesystem2",
		.offset		= MTDPART_OFS_APPEND,
		.size		= MTDPART_SIZ_FULL,
		.mask_flags	= 0,
	}
	/* two blocks with bad block table (and mirror) at the end */
};

static struct davinci_nand_pdata davinci_nand_data = {
	.mask_chipsel		= BIT(14),
	.parts			= davinci_nand_partitions,
	.nr_parts		= ARRAY_SIZE(davinci_nand_partitions),
	.ecc_mode		= NAND_ECC_HW,
	.options		= NAND_USE_FLASH_BBT,
	.ecc_bits		= 4,
};

static struct resource davinci_nand_resources[] = {
	{
		.start		= DM365_ASYNC_EMIF_DATA_CE0_BASE,
		.end		= DM365_ASYNC_EMIF_DATA_CE0_BASE + SZ_32M - 1,
		.flags		= IORESOURCE_MEM,
	}, {
		.start		= DM365_ASYNC_EMIF_CONTROL_BASE,
		.end		= DM365_ASYNC_EMIF_CONTROL_BASE + SZ_4K - 1,
		.flags		= IORESOURCE_MEM,
	},
};

static struct platform_device davinci_nand_device = {
	.name			= "davinci_nand",
	.id			= 0,
	.num_resources		= ARRAY_SIZE(davinci_nand_resources),
	.resource		= davinci_nand_resources,
	.dev			= {
		.platform_data	= &davinci_nand_data,
	},
};


/**
 * dm365evm_reset_imager() - reset the image sensor
 * @en: enable/disable flag
 */
static void dm365evm_reset_imager(int rst)
{
#if 1
/*need code to reset adv7611 tvp5151 ad9363*/
	gpio_direction_output(40, 1);

#else
	/**
	 * Configure GPIO40 to be output and high. This has dependency on MMC1
	 */
	davinci_cfg_reg(DM365_GPIO40);
	gpio_request(40, "sensor_reset");
	if (rst)
		gpio_direction_output(40, 1);
	else
		gpio_direction_output(40, 0);
#endif

	
}


#define ADV7611_STD_ALL        (V4L2_STD_720P_50   | V4L2_STD_720P_60 	|\
				V4L2_STD_1080I_50  | V4L2_STD_1080I_60 	|\
				V4L2_STD_1080P_50  | V4L2_STD_1080P_60)


/* Inputs available at the adv7611 */
static struct v4l2_input adv7611_inputs[] = {
	{
		.index = 0,
		.name = "Component",
		.type = V4L2_INPUT_TYPE_CAMERA,
		.std = ADV7611_STD_ALL,
	},
};


static struct vpfe_subdev_info vpfe_sub_devs[] = {
	{
		.module_name = "adv7611",
		.grp_id = VPFE_SUBDEV_ADV7611,
		.num_inputs = ARRAY_SIZE(adv7611_inputs),
		.inputs = adv7611_inputs,
		.ccdc_if_params = {
			.if_type = VPFE_YCBCR_SYNC_16,
			.hdpol = VPFE_PINPOL_NEGATIVE,
			.vdpol = VPFE_PINPOL_NEGATIVE,
		},		
		.board_info = {
			I2C_BOARD_INFO("adv7611", 0x4C),
		},
	},

};

/* Set the input mux for TVP7002/TVP5146/MTxxxx sensors */
static int dm365evm_setup_video_input(enum vpfe_subdev_id id)
{
	return 0;

}

static struct vpfe_config vpfe_cfg = {
       .setup_input = dm365evm_setup_video_input,
       .num_subdevs = ARRAY_SIZE(vpfe_sub_devs),
       .sub_devs = vpfe_sub_devs,
       .card_name = "DM365 EVM",
       .ccdc = "DM365 ISIF",
       .num_clocks = 1,
       .clocks = {"vpss_master"},
};

static void dm365evm_usb_configure(void)
{
	davinci_cfg_reg(DM365_GPIO33);
	gpio_request(33, "usb");
	gpio_direction_output(33, 1);
	setup_usb(500, 8);
}

static struct i2c_board_info i2c_info[] = {
	{
		I2C_BOARD_INFO("it66121", 0x4d),
	},

};


static struct davinci_i2c_platform_data i2c_pdata = {
	.bus_freq	= 100	/* kHz */,
	.bus_delay	= 20000	/* usec */,
};

static void __init evm_init_i2c(void)
{
	davinci_cfg_reg(DM365_I2C_SDA);
	davinci_cfg_reg(DM365_I2C_SCL);

	davinci_init_i2c(&i2c_pdata);
	//if (have_imager())
	//	i2c_add_driver(&pca9543a_driver);
	if( LENA_GROUND == device_lena_air_id )
	{
		i2c_register_board_info(1, i2c_info, ARRAY_SIZE(i2c_info));
	}
}

static struct platform_device *dm365_evm_nand_devices[] __initdata = {
	&davinci_nand_device,
};

static inline int have_leds(void)
{
#ifdef CONFIG_LEDS_CLASS
	return 1;
#else
	return 0;
#endif
}

struct cpld_led {
	struct led_classdev	cdev;
	u8			mask;
};

static const struct {
	const char *name;
	const char *trigger;
} cpld_leds[] = {
	{ "dm365evm::ds2", },
	{ "dm365evm::ds3", },
	{ "dm365evm::ds4", },
	{ "dm365evm::ds5", },
	{ "dm365evm::ds6", "nand-disk", },
	{ "dm365evm::ds7", "mmc1", },
	{ "dm365evm::ds8", "mmc0", },
	{ "dm365evm::ds9", "heartbeat", },
};

static int __init cpld_leds_init(void)
{
	//int	i;

	if (!have_leds())
		return 0;

	/* setup LEDs */


	return 0;
}
/* run after subsys_initcall() for LEDs */
fs_initcall(cpld_leds_init);


static void __init evm_init(void)
{
	u8 mux;
	const char *label;
	struct clk *aemif_clk;
	struct davinci_soc_info *soc_info = &davinci_soc_info;

	/* Make sure we can configure the CPLD through CS1.  Then
	 * leave it on for later access to MMC and LED registers.
	 */
	aemif_clk = clk_get(NULL, "aemif");
	if (IS_ERR(aemif_clk))
		return;
	clk_enable(aemif_clk);

	if (request_mem_region(DM365_ASYNC_EMIF_DATA_CE0_BASE, SECTION_SIZE,
			"fpga") == NULL)
		goto fail;
	fpga = ioremap(DM365_ASYNC_EMIF_DATA_CE0_BASE, SECTION_SIZE);

	if (!fpga) {
		release_mem_region(DM365_ASYNC_EMIF_DATA_CE0_BASE,
				SECTION_SIZE);
fail:
		pr_err("ERROR: can't map CPLD\n");
		clk_disable(aemif_clk);
		return;
	}

	/* External muxing for some signals */
	mux = 0;

	/* Read CPLD version number */
	//soc_info->cpld_version = __raw_readb(cpld + CPLD_VERSION);
	soc_info->cpld_version = 0x21;//for lena,this is init by ourself,please refer to cputype.h

      if(device_lena_air_id == LENA_AIR){

		if (have_adv7611()) {
			mux |= VIDEO_INPUT_MUX_ADV7611;
			label = "ADV7611 HD";
		} else if (have_tvp5150()){
			/* default to tvp5146 */
			mux |= VIDEO_INPUT_MUX_TVP5150;
			label = "tvp5150 SD";
			dm365evm_reset_imager(0);
		}
	}

	pr_info("EVM: %s video input\n", label);

}

void enable_lcd(void)
{
	return;	
}
EXPORT_SYMBOL(enable_lcd);

void enable_hd_clk(void)
{

}
EXPORT_SYMBOL(enable_hd_clk);

static struct davinci_uart_config uart_config __initdata = {
	.enabled_uarts = (1 << 0),
};

static void __init dm365_evm_map_io(void)
{
	/* setup input configuration for VPFE input devices */
	dm365_set_vpfe_config(&vpfe_cfg);
	dm365_init();
}

static struct spi_eeprom at25640 = {
	.byte_len	= SZ_64K / 8,
	.name		= "at25640",
	.page_size	= 32,
	.flags		= EE_ADDR2,
};

static struct spi_board_info dm365_evm_spi_info[] __initconst = {
	{
		.modalias	= "at25",
		.platform_data	= &at25640,
		.max_speed_hz	= 20 * 1000 * 1000,	/* at 3v3 */
		.bus_num	= 0,
		.chip_select	= 0,
		.mode		= SPI_MODE_0,
	},
};


static struct spi_board_info dm365_evm_spi4_info[] __initconst = {
	{
		.modalias	= "ad9363",
		.platform_data	= NULL,
		.max_speed_hz	= 800 * 1000,	/* at 3v3 */
		.bus_num	= 4,
		.chip_select	= 0,
		.mode		= SPI_MODE_1,
	},
};


#define SPI0_CS_GPIO (25) 
#define SPI4_CS_GPIO (37) 
#define AIR_GROUND_GPIO (29) 
#define FPGA_RESET_GPIO	(9)
#define IT66121_POWERON	(103)
#define ADV7611_POWERON	(39)

#define GROUND_LED_GPIO81  (81) 
#define AIR_LED_GPIO91 (91) //GREEN
#define AIR_LED_GPIO92 (92) //RED
#define AIR_LED_GPIO86	(86) //RED
#define AIR_LED_GPIO90	(90) //GREEN


/*--------------------------------------------------------------------------
  * function : GPIO_init
  * output	 : GPIO init
  * author	 version	 date		 note
  * feller	 1.0	 20151129	 
  *----------------------------------------------------------------------------
*/
void GPIORequsetInit(void)
{
	gpio_request( SPI0_CS_GPIO, "SPI0_CS_GPIO" );
	gpio_request( SPI4_CS_GPIO, "SPI4_CS_GPIO" );
  	gpio_request( AIR_GROUND_GPIO, "AIR_GROUND_GPIO" );
	gpio_request( FPGA_RESET_GPIO, "FPGA_RESET_GPIO" );
	gpio_request( IT66121_POWERON, "IT66121_POWERON" );
	gpio_request( ADV7611_POWERON, "ADV7611_POWERON" );

	//gpio requset for LED
	gpio_request( GROUND_LED_GPIO81, "GROUND_LED_GPIO81" );
	gpio_request( AIR_LED_GPIO91, "AIR_LED_GPIO91" );
	gpio_request( AIR_LED_GPIO92, "AIR_LED_GPIO92" );
	gpio_request( AIR_LED_GPIO86, "AIR_LED_GPIO86" );
	gpio_request( AIR_LED_GPIO90, "AIR_LED_GPIO90" );
	
  	return;
}
 /*--------------------------------------------------------------------------
 * function	: GetAirGroundStationFlag
 * output 	: iAirorGround: air and ground station flag,0:ground 1:air,others:invalid
 * author	version		date		note
 * feller	1.0		20151129	
 *----------------------------------------------------------------------------
*/
void GetAirGroundStationFlag(void)
{
	unsigned int uiFlag = 0;


	gpio_direction_input( AIR_GROUND_GPIO );
	uiFlag = gpio_get_value( AIR_GROUND_GPIO );
	if( uiFlag ) 
	{
		davinci_cfg_reg(DM365_VIN_CAM_WEN);
		davinci_cfg_reg(DM365_VIN_CAM_VD);
		davinci_cfg_reg(DM365_VIN_CAM_HD);
		davinci_cfg_reg(DM365_VIN_YIN4_7_EN);
		davinci_cfg_reg(DM365_VIN_YIN0_3_EN);		
		davinci_cfg_reg( DM365_SD1_DATA1 );//config the gio39
		gpio_direction_output( ADV7611_POWERON, 1 );//supply the 5v for adv7611	
		device_lena_air_id = LENA_AIR;
		printk("EVM: LENA AIR device!\n");
	}
	else 
	{
		davinci_cfg_reg(DM365_VOUT_FIELD_G81);
		davinci_cfg_reg(DM365_VOUT_COUTL_EN);
		davinci_cfg_reg(DM365_VOUT_COUTH_EN);		
		davinci_cfg_reg( DM365_VOUT_LCDOE );//config the gio82 to LCD_OE
		davinci_cfg_reg( DM365_GPIO103 );//config the gio103 
		gpio_direction_output( IT66121_POWERON, 1 );//supply the 5v for it66121
		device_lena_air_id = LENA_GROUND;
		printk("EVM: LENA GROUND device!\n");
	}
	return ;
}

static __init void dm365_evm_init(void)
{


	GPIORequsetInit();
	GetAirGroundStationFlag();
		
	evm_init_i2c();
	davinci_serial_init(&uart_config);

	//dm365evm_emac_configure();
	dm365evm_usb_configure();

	//davinci_setup_mmc(0, &dm365evm_mmc_config);

	/* maybe setup mmc1/etc ... _after_ mmc0 */
	evm_init();

	//dm365_init_asp(&dm365_evm_snd_data);
	dm365_init_rtc();
	//dm365_init_ks(&dm365evm_ks_data);

	dm365_init_spi0(BIT(0), dm365_evm_spi_info,
			ARRAY_SIZE(dm365_evm_spi_info));

	dm365_init_spi4(BIT(0), dm365_evm_spi4_info,
			ARRAY_SIZE(dm365_evm_spi4_info));
	//dm365_init_tsc2004();
}

static __init void dm365_evm_irq_init(void)
{
	davinci_irq_init();
}

MACHINE_START(DAVINCI_DM365_EVM, "DaVinci DM36x EVM")
	.phys_io	= IO_PHYS,
	.io_pg_offst	= (__IO_ADDRESS(IO_PHYS) >> 18) & 0xfffc,
	.boot_params	= (0x80000100),
	.map_io		= dm365_evm_map_io,
	.init_irq	= dm365_evm_irq_init,
	.timer		= &davinci_timer,
	.init_machine	= dm365_evm_init,
MACHINE_END

