/*
 * This source file is AD9363 drirver

 *****************************************************************************
 * Copyright(C) 2015, SinoMartin Corp.
 *----------------------------------------------------------------------------
 * filename	: ad9363.c
 * function	: AD9363 drirver
 * author	version		date		note
 * feller	1.0		20151229	create         
 *----------------------------------------------------------------------------
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/math64.h>
#include <linux/sched.h>
#include <linux/spi/spi.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>  
#include <linux/io.h>
#include <linux/fs.h>
extern unsigned char device_lena_air_id;
#define DEVICE_AD9363 "/dev/misc/ad9363"
#define AD9363_DRIVER_NAME  "ad9363"
#define AD9363_DEVICE_MINOR   (224)

#define MAX_MBYTE_SPI (1)
#define AD9363_READ	(0x0000)
#define AD9363_WRITE	(0x8000)

static struct spi_device *spi_ad9363=NULL;

 /*----------------------------------------------------------------------------
  * name	 : ad9363_spi_readm
  * function	 : ad9363 spi read interface 
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */
static int ad9363_spi_readm(struct spi_device *spi, unsigned short reg, 
			   unsigned char *rbuf, unsigned short num) 

{ 

	unsigned char buf[2]; 
	int ret; 
	unsigned short cmd; 

	if (num > MAX_MBYTE_SPI) 

		return -EINVAL; 

	cmd = AD9363_READ | (reg); 
	buf[0] = cmd >> 8; 
	buf[1] = cmd & 0xFF; 

	ret = spi_write_then_read(spi, &buf[0], 2, rbuf, num); 

	if (ret < 0) { 

		dev_err(&spi->dev, "Read Error %d", ret); 

		return ret; 

	} 

#ifdef AD9363_DEBUG 
	{ 
		int i; 

		for (i = 0; i < num; i++) 

			dev_dbg(&spi->dev, "%s: reg 0x%X val 0x%X\n", __func__, reg--, rbuf[i]); 
	} 

#endif 

	return 0; 
} 
 /*----------------------------------------------------------------------------
  * name	 : ad9363_spi_read 
  * function	 : ad9363 spi read interface 
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */

int ad9363_spi_read(struct spi_device *spi, unsigned short reg) 

{ 

	unsigned char buf; 
	int ret; 

	ret = ad9363_spi_readm(spi, reg, &buf, 1); 

	if (ret < 0) 
	{
		dev_err(&spi->dev, "Read Error %d", ret); 
		return ret; 
	}
	return buf; 

} 
 /*----------------------------------------------------------------------------
  * name	 : ad9363_spi_write
  * function	 : ad9363 spi write interface 
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */
int ad9363_spi_write(struct spi_device *spi, unsigned short reg, unsigned char val) 

{ 

	unsigned char buf[3]; 
	int ret; 
	unsigned short cmd; 

	cmd = AD9363_WRITE |(reg); 

	buf[0] = cmd >> 8; 
	buf[1] = cmd & 0xFF; 
	buf[2] = val; 

	ret = spi_write_then_read(spi, buf, 3, NULL, 0); 

	if (ret < 0) { 
		dev_err(&spi->dev, "Write Error %d", ret); 
		return ret; 
	} 
	
	dev_dbg(&spi->dev, "reg 0x%X val 0x%X\n", reg, buf[2]); 

	return 0; 
} 
 /*----------------------------------------------------------------------------
  * name	 : ad9363_read
  * function	 : ad9363 read interface 
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */
typedef struct AD9363RegStruct {
    unsigned int uiAddr;	//SPI bus, address  
    unsigned int uiValue;	//SPI bus,data
} tAD9363Reg;

static tAD9363Reg ad9363_reg={0};

ssize_t ad9363_read(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{
	unsigned long ulRet;
	
	ulRet = copy_from_user( (void*)&ad9363_reg, buf, 8);
	ad9363_reg.uiValue = ad9363_spi_read(spi_ad9363,(unsigned short)ad9363_reg.uiAddr);

	ulRet = copy_to_user( (void*)buf, &ad9363_reg, 8); 

	return 8;
}

 /*----------------------------------------------------------------------------
  * name	 : ad9363_write
  * function	 : ad9363 write interface 
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */
ssize_t ad9363_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{
	unsigned long ulRet;
    	ulRet = copy_from_user(&ad9363_reg, buf, 8);
	ulRet = ad9363_spi_write(spi_ad9363,(unsigned short)ad9363_reg.uiAddr,(unsigned char)ad9363_reg.uiValue);

    	return 8;

}
 /*----------------------------------------------------------------------------
  * name	 : ad9363_open
  * function	 : ad9363 open interface 
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */
static int ad9363_open(struct inode *inode, struct file *file)
{
	return 0;
} /* fpga_open */

/*----------------------------------------------------------------------------
  * name	 : ad9363_close
  * function	 : ad9363 close interface 
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */
static int ad9363_close(struct inode *inode, struct file *file)
{
	return 0;
} /* fpga_close */


#define GET_INFO	0U
#define SET_INFO	1U
#define AD9363_CMD_SIZE	2U
/*----------------------------------------------------------------------------
  * name	 : ad9363_ioctl
  * function	 : ad9363 ioctl interface 
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */

static int ad9363_ioctl(struct inode *inode, struct file *file,
		     u_int cmd, u_long arg)
{
	int ret = 0;

	switch (cmd) {
	
		default:
			ret = -ENOTTY;
	}

	return ret;
}



static const struct file_operations ad9363_fops = {
	.owner		= THIS_MODULE,
	.read           = (void*)ad9363_read,
	.write          = ad9363_write,
	.ioctl		= ad9363_ioctl,
	.open		= ad9363_open,
	.release	= ad9363_close,

};

static struct miscdevice ad9363_dev = {
 	.minor = AD9363_DEVICE_MINOR,  
	.name = AD9363_DRIVER_NAME,  
	.fops = &ad9363_fops,  
};

/*----------------------------------------------------------------------------
  * name	 : __devinit ad9363_probe
  * function	 : ad9363 device probe function
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */

static int __devinit ad9363_probe(struct spi_device *spi)
{

	int ret;

	ret = misc_register(&ad9363_dev);
	spi_ad9363 = spi;
	printk(AD9363_DRIVER_NAME"\t misc initialized %s!\n", (0==ret)?"successed":"failed");	

	return 0;
}

/*----------------------------------------------------------------------------
  * name	 : __devinit ad9363_remove
  * function	 : ad9363 device remove function
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */
static int __devexit ad9363_remove(struct spi_device *spi)
{
	spi_ad9363 = NULL;
	misc_deregister(&ad9363_dev);

	return 0;
}



static struct spi_driver ad9363_driver = {
	.driver = {
		.name	= AD9363_DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
	.probe	= ad9363_probe,
	.remove	= __devexit_p(ad9363_remove),

	/* REVISIT: many of these chips have deep power-down modes, which
	 * should clearly be entered on suspend() to minimize power use.
	 * And also when they're otherwise idle...
	 */
};
/*----------------------------------------------------------------------------
  * name	 : __init ad9363_init
  * function	 : ad9363 device init function
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */
static int __init ad9363_init(void)
{
	int ret;

	ret = spi_register_driver(&ad9363_driver);
	printk(AD9363_DRIVER_NAME"\t initialized %s!\n", (0==ret)?"successed":"failed");	

	return ret;

}
/*----------------------------------------------------------------------------
  * name	 : __exit ad9363_cleanup_module
  * function	 : ad9363 device module exit function
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */
static void __exit ad9363_cleanup_module(void)
{
	spi_unregister_driver(&ad9363_driver);
}


module_init(ad9363_init);
module_exit(ad9363_cleanup_module);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("SinoMartin");
MODULE_DESCRIPTION(" Direct character-device access to AD9363 devices");

