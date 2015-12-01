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


#define	LENA_AIR  0
#define	LENA_GROUND  1
#define	LENA_NULL  2

unsigned char device_lena_air_id=LENA_NULL;

static struct tvp514x_platform_data tvp5146_pdata = {
       .clk_polarity = 0,
       .hs_polarity = 1,
       .vs_polarity = 1
};

/* tvp7002 platform data, used during reset and probe operations */
static struct tvp7002_platform_data tvp7002_pdata = {
       .clk_polarity = 0,
       .hs_polarity = 0,
       .vs_polarity = 0,
       .fid_polarity = 0,
};

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
	u8 val;
#if 0
   val = __raw_readb(cpld + CPLD_POWER) | BIT(3) | BIT(11) | BIT(19) | BIT(27);
	__raw_writeb(val, (cpld + CPLD_POWER));

   val = __raw_readb(cpld + CPLD_MUX) | BIT(6) | BIT(14) | BIT(22) | BIT(30);
	__raw_writeb(val, (cpld + CPLD_MUX));

	/* Reset bit6 of CPLD_IMG_DIR2 */
	val = __raw_readb(cpld + CPLD_IMG_DIR2) & ~BIT(6);
	__raw_writeb(val, (cpld + CPLD_IMG_DIR2));	

	/* Set bit5 of CPLD_IMG_MUX5 */
	val = __raw_readb(cpld + CPLD_IMG_MUX5) | BIT(5);
	__raw_writeb(val, (cpld + CPLD_IMG_MUX5));	

	/* Reset bit 0 of CPLD_IMG_MUX5 */
	val = __raw_readb(cpld + CPLD_IMG_MUX5) & ~BIT(0);
	__raw_writeb(val, (cpld + CPLD_IMG_MUX5));	
#endif

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

#define V4L2_STD_MT9P031_STD_ALL  (V4L2_STD_525_60\
	|V4L2_STD_625_50|V4L2_STD_525P_60\
	|V4L2_STD_625P_50|V4L2_STD_720P_30\
   |V4L2_STD_720P_50|V4L2_STD_720P_60\
   |V4L2_STD_1080I_30|V4L2_STD_1080I_50\
	|V4L2_STD_1080I_60|V4L2_STD_1080P_30\
   |V4L2_STD_1080P_50|V4L2_STD_1080P_60)

/* Input available at the mt9p031 */
static struct v4l2_input mt9p031_inputs[] = {
	{
		.index = 0,
		.name = "Camera",
		.type = V4L2_INPUT_TYPE_CAMERA,
      .std = V4L2_STD_MT9P031_STD_ALL,
	}
};

#define TVP5150_STD_ALL        (V4L2_STD_NTSC | V4L2_STD_PAL)
/* Inputs available at the TVP5146 */
static struct v4l2_input tvp5150_inputs[] = {
	{
		.index = 0,
		.name = "Composite",
		.type = V4L2_INPUT_TYPE_CAMERA,
		.std = TVP5150_STD_ALL,
	},
	{
		.index = 1,
		.name = "S-Video",
		.type = V4L2_INPUT_TYPE_CAMERA,
		.std = TVP5150_STD_ALL,
	},
};

#define TVP7002_STD_ALL        (V4L2_STD_525P_60   | V4L2_STD_625P_50 	|\
				V4L2_STD_NTSC      | V4L2_STD_PAL   	|\
				V4L2_STD_720P_50   | V4L2_STD_720P_60 	|\
				V4L2_STD_1080I_50  | V4L2_STD_1080I_60 	|\
				V4L2_STD_1080P_50  | V4L2_STD_1080P_60)

/* Inputs available at the TVP7002 */
static struct v4l2_input tvp7002_inputs[] = {
	{
		.index = 0,
		.name = "Component",
		.type = V4L2_INPUT_TYPE_CAMERA,
		.std = TVP7002_STD_ALL,
	},
};

/*
 * this is the route info for connecting each input to decoder
 * ouput that goes to vpfe. There is a one to one correspondence
 * with tvp5146_inputs
 */
static struct vpfe_route tvp5150_routes[] = {
	{
		.input = INPUT_CVBS_VI2B,
		.output = OUTPUT_10BIT_422_EMBEDDED_SYNC,
	},
{
		.input = INPUT_SVIDEO_VI2C_VI1C,
		.output = OUTPUT_10BIT_422_EMBEDDED_SYNC,
	},
};

static struct vpfe_subdev_info vpfe_sub_devs[] = {
	{
		.module_name = "tvp5150",
		.grp_id = VPFE_SUBDEV_TVP5150,
		.num_inputs = ARRAY_SIZE(tvp5150_inputs),
		.inputs = tvp5150_inputs,
		.routes = tvp5150_routes,
		.can_route = 1,
		.ccdc_if_params = {
			.if_type = VPFE_BT656,
			.hdpol = VPFE_PINPOL_POSITIVE,
			.vdpol = VPFE_PINPOL_POSITIVE,
		},
		.board_info = {
			I2C_BOARD_INFO("tvp5150", 0x5C),
			//.platform_data = &tvp5150_pdata,
		},
	},
	{
		.module_name = "adv7611",
		.grp_id = VPFE_SUBDEV_ADV7611,
		.board_info = {
			I2C_BOARD_INFO("adv7611", 0x4C),
		},
	},
/*
	{
		.module_name = "tvp5146",
		.grp_id = VPFE_SUBDEV_TVP5146,
		.num_inputs = ARRAY_SIZE(tvp5146_inputs),
		.inputs = tvp5146_inputs,
		.routes = tvp5146_routes,
		.can_route = 1,
		.ccdc_if_params = {
			.if_type = VPFE_BT656,
			.hdpol = VPFE_PINPOL_POSITIVE,
			.vdpol = VPFE_PINPOL_POSITIVE,
		},
		.board_info = {
			I2C_BOARD_INFO("tvp5146", 0x5d),
			.platform_data = &tvp5146_pdata,
		},
	},

	{
		.module_name = "tvp7002",
		.grp_id = VPFE_SUBDEV_TVP7002,
		.num_inputs = ARRAY_SIZE(tvp7002_inputs),
		.inputs = tvp7002_inputs,
		.ccdc_if_params = {
			.if_type = VPFE_BT1120,
			.hdpol = VPFE_PINPOL_POSITIVE,
			.vdpol = VPFE_PINPOL_POSITIVE,
		},
		.board_info = {
			I2C_BOARD_INFO("tvp7002", 0x5c),
			.platform_data = &tvp7002_pdata,
		},
	},
	{
		.module_name = "ths7353",
		.grp_id = VPFE_SUBDEV_TVP7002,
		.board_info = {
			I2C_BOARD_INFO("ths7353", 0x2e),
		},
	},
	{
		.module_name = "mt9p031",
		.is_camera = 1,
		.grp_id = VPFE_SUBDEV_MT9P031,
		.num_inputs = ARRAY_SIZE(mt9p031_inputs),
		.inputs = mt9p031_inputs,
		.ccdc_if_params = {
			.if_type = VPFE_RAW_BAYER,
			.hdpol = VPFE_PINPOL_POSITIVE,
			.vdpol = VPFE_PINPOL_POSITIVE,
		},
		.board_info = {
			I2C_BOARD_INFO("mt9p031", 0x5d),
			/* this is for PCLK rising edge 
			.platform_data = (void *)1,
		},
	}
*/
};

/* Set the input mux for TVP7002/TVP5146/MTxxxx sensors */
static int dm365evm_setup_video_input(enum vpfe_subdev_id id)
{
#if 0
	const char *label;
	u8 mux, resets;

	mux = __raw_readb(cpld + CPLD_MUX);
	mux &= ~CPLD_VIDEO_INPUT_MUX_MASK;
	resets = __raw_readb(cpld + CPLD_RESETS);
	switch (id) {
		case VPFE_SUBDEV_TVP5146:
			mux |= CPLD_VIDEO_INPUT_MUX_TVP5146;
			resets &= ~BIT(0);
			label = "tvp5146 SD";
			dm365evm_reset_imager(0);
			break;
		case VPFE_SUBDEV_MT9P031:
			mux |= CPLD_VIDEO_INPUT_MUX_IMAGER;
			resets |= BIT(0); /* Put TVP5146 in reset */
			label = "HD imager";

			dm365evm_reset_imager(1);
			/* Switch on pca9543a i2c switch */
			if (have_imager())
				dm365evm_enable_pca9543a(1);
			break;
		case VPFE_SUBDEV_TVP7002:
			resets &= ~BIT(2);
			mux |= CPLD_VIDEO_INPUT_MUX_TVP7002;
			label = "tvp7002 HD";
			break;
		default:
			return 0;
	}
	__raw_writeb(mux, cpld + CPLD_MUX);
	__raw_writeb(resets, cpld + CPLD_RESETS);

	pr_info("EVM: switch to %s video input\n", label);
	return 0;
#endif
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

static struct davinci_i2c_platform_data i2c_pdata = {
	.bus_freq	= 400	/* kHz */,
	.bus_delay	= 0	/* usec */,
};

static void __init evm_init_i2c(void)
{
	davinci_init_i2c(&i2c_pdata);
	//if (have_imager())
	//	i2c_add_driver(&pca9543a_driver);
	//i2c_register_board_info(1, i2c_info, ARRAY_SIZE(i2c_info));
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

static void cpld_led_set(struct led_classdev *cdev, enum led_brightness b)
{
	struct cpld_led *led = container_of(cdev, struct cpld_led, cdev);
	//u8 reg = __raw_readb(cpld + CPLD_LEDS);

	//if (b != LED_OFF)
	//	reg &= ~led->mask;
	//else
		//reg |= led->mask;
	//__raw_writeb(reg, cpld + CPLD_LEDS);
}

static enum led_brightness cpld_led_get(struct led_classdev *cdev)
{
	struct cpld_led *led = container_of(cdev, struct cpld_led, cdev);
	//u8 reg = __raw_readb(cpld + CPLD_LEDS);

	//return (reg & led->mask) ? LED_OFF : LED_FULL;
	return LED_OFF;
}

static int __init cpld_leds_init(void)
{
	int	i;

	if (!have_leds())
		return 0;

	/* setup LEDs */


	return 0;
}
/* run after subsys_initcall() for LEDs */
fs_initcall(cpld_leds_init);


static void __init evm_init(void)
{
	u8 mux, resets;
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

	/* Read SW5 to set up NAND + keypad _or_ OneNAND (sync read).
	 * NOTE:  SW4 bus width setting must match!
	 */


		//platform_add_devices(dm365_evm_nand_devices,ARRAY_SIZE(dm365_evm_nand_devices));


	/* Leave external chips in reset when unused. */
	//resets = BIT(3) | BIT(2) | BIT(1) | BIT(0);



	/* Static video input config with SN74CBT16214 1-of-3 mux:
	 *  - port b1 == tvp7002 (mux lowbits == 1 or 6)
	 *  - port b2 == imager (mux lowbits == 2 or 7)
	 *  - port b3 == tvp5146 (mux lowbits == 5)
	 *
	 * Runtime switching could work too, with limitations.
	 */
      if(device_lena_air_id == LENA_AIR){

		/* we can use MMC1 ... */
		//dm365evm_mmc_configure();
		//davinci_setup_mmc(1, &dm365evm_mmc_config);

		if (have_adv7611()) {
			mux |= VIDEO_INPUT_MUX_ADV7611;
			resets &= ~BIT(2);
			label = "ADV7611 HD";
		} else if (have_tvp5150()){
			/* default to tvp5146 */
			mux |= VIDEO_INPUT_MUX_TVP5150;
			resets &= ~BIT(0);
			label = "tvp5150 SD";
			dm365evm_reset_imager(0);
		}
	}
	//__raw_writeb(mux, cpld + CPLD_MUX);
	///__raw_writeb(resets, cpld + CPLD_RESETS);

	pr_info("EVM: %s video input\n", label);

	/* REVISIT export switches: NTSC/PAL (SW5.6), EXTRA1 (SW5.2), etc */
}

void enable_lcd(void)
{
	/* Turn on LCD backlight for DM368 */
	//if (cpu_is_davinci_dm368()) {
		///davinci_cfg_reg(DM365_GPIO80);

		/* Configure 9.25MHz clock to LCD */
		//__raw_writeb(0x80, cpld + CPLD_RESETS);

		/* CPLD_CONN_GIO17 is level high */
		//__raw_writeb(0xff, cpld + CPLD_CCD_IO1);

		/* CPLD_CONN_GIO17 is an output */
		//__raw_writeb(0xfb, cpld + CPLD_CCD_DIR1);
	
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
	if(device_lena_air_id == LENA_AIR)dm365_set_vpfe_config(&vpfe_cfg);
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

#define AIR_GROUND_GPIO (29) 
#define SPI0_CS_GPIO (25) 
#define SPI4_CS_GPIO (37) 
/*--------------------------------------------------------------------------
  * function : GPIO_init
  * output	 : GPIO init
  * author	 version	 date		 note
  * feller	 1.0	 20151129	 
  *----------------------------------------------------------------------------
*/
void GPIORequsetInit()
{
	gpio_request( SPI0_CS_GPIO, "SPI0_CS_GPIO" );
	gpio_request( SPI4_CS_GPIO, "SPI4_CS_GPIO" );
  	gpio_request( AIR_GROUND_GPIO, "AIR_GROUND_GPIO" );
  return;
}
 /*--------------------------------------------------------------------------
 * function	: GetAirGroundStationFlag
 * output 	: iAirorGround: air and ground station flag,0:ground 1:air,others:invalid
 * author	version		date		note
 * feller	1.0		20151129	
 *----------------------------------------------------------------------------
*/
void GetAirGroundStationFlag()
{
	unsigned int uiFlag = 0;
	//uiFlag = (__raw_readl( IO_ADDRESS( 0x01c67020) ) & 0x20000000 );
	uiFlag = gpio_direction_input( AIR_GROUND_GPIO );
	if( 0 != uiFlag ) 
	{
		device_lena_air_id = LENA_AIR;
		printk("EVM: LENA AIR device!\n");
	}
	else 
	{
		device_lena_air_id = LENA_GROUND;
		printk("EVM: LENA GROUND device!\n");
	}
	return ;
}

static __init void dm365_evm_init(void)
{
	//davinci_cfg_reg(DM365_GPIO29);

	//gpio_direction_input(29);
#if 0
	if((__raw_readl(IO_ADDRESS(0x01c67020))&0x20000000)!=0) 
		{
			device_lena_air_id = LENA_AIR;
			printk("EVM: LENA air device!\n");
		}
	else 
		{
			device_lena_air_id = LENA_GROUND;
			printk("EVM: LENA GROUND device!\n");
		}	
#endif
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

