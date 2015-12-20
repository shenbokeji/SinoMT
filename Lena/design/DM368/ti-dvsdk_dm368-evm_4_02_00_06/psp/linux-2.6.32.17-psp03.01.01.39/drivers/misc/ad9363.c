/*
 * Character-device access to raw MTD devices.
 *
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

#define DEVICE_AD9363 "/dev/misc/ad9363"
#define AD9363_DRIVER_NAME  "ad9363"
#define AD9363_DEVICE_MINOR   224

#define MAX_MBYTE_SPI (1)
#define AD9363_READ	(0x0000)
#define AD9363_WRITE	(0x8000)

static struct spi_device *spi_ad9363=NULL;


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

typedef struct AD9363RegStruct {
    unsigned int uiAddr;	//SPI bus, address  
    unsigned int uiValue;	//SPI bus,data
} tAD9363Reg;

static tAD9363Reg ad9363_reg={0};

//static unsigned int rw_buf=0;
ssize_t ad9363_read(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{
	unsigned long ulRet;
	
	ulRet = copy_from_user( (void*)&ad9363_reg, buf, 8);
	ad9363_reg.uiValue = ad9363_spi_read(spi_ad9363,(unsigned short)ad9363_reg.uiAddr);

	ulRet = copy_to_user( (void*)buf, &ad9363_reg, 8); 

	return 8;
}


ssize_t ad9363_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{
	unsigned long ulRet;
    	ulRet = copy_from_user(&ad9363_reg, buf, 8);
	ulRet = ad9363_spi_write(spi_ad9363,(unsigned short)ad9363_reg.uiAddr,(unsigned char)ad9363_reg.uiValue);

    	return 8;

}

static int ad9363_open(struct inode *inode, struct file *file)
{
	//int iRet;
	//printk(AD9363_DRIVER_NAME"\topen the file \n" );
	
	return 0;
} /* fpga_open */

/*====================================================================*/

static int ad9363_close(struct inode *inode, struct file *file)
{
	//printk(AD9363_DRIVER_NAME"\tclose the file \n" );
	return 0;
} /* fpga_close */


#define GET_INFO	0U
#define SET_INFO	1U

#define AD9363_CMD_SIZE	2U



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

#define	LENA_AIR  	(0)
#define	LENA_GROUND  	(1)

extern unsigned char device_lena_air_id;
#if 0
static int __init ad9363_air_init(struct spi_device *spi)
{

	ad9363_spi_write(spi, 0x00, 0x00);
	ad9363_spi_write(spi, 0x00, 0x81);
	ad9363_spi_write(spi, 0x00, 0x00);
	return 0;

}
static int __init ad9363_ground_init(struct spi_device *spi)
{

	ad9363_spi_write(spi, 0x00, 0x00);
	ad9363_spi_write(spi, 0x00, 0x81);
	ad9363_spi_write(spi, 0x00, 0x00);


	return 0;
}
#endif
static const struct file_operations ad9363_fops = {
	.owner		= THIS_MODULE,
	.read           = ad9363_read,
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



static int __devinit ad9363_probe(struct spi_device *spi)
{

	int ret;

	ret = misc_register(&ad9363_dev);
	spi_ad9363 = spi;
	printk(AD9363_DRIVER_NAME"\t misc initialized %s!\n", (0==ret)?"successed":"failed");	


	if(device_lena_air_id == LENA_AIR)
	{	
		//ad9363_air_init(spi);
		printk("LENA id is air device!!!\n");
	}
	else if(device_lena_air_id == LENA_GROUND)
	{
		//ad9363_ground_init(spi);
		printk("LENA id is ground device!!!\n");
	}
	else printk("LENA air id ERROR!!!\n");

	return 0;
}


static int __devexit ad9363_remove(struct spi_device *spi)
{
	//struct m25p	*flash = dev_get_drvdata(&spi->dev);
	int		status;
	spi_ad9363 = NULL;
	misc_deregister(&ad9363_dev);

	return 0;
}



static struct spi_driver ad9363_driver = {
	.driver = {
		.name	= AD9363_DRIVER_NAME,
		//.bus	= &spi_bus_type,
		.owner	= THIS_MODULE,
	},
	.probe	= ad9363_probe,
	.remove	= __devexit_p(ad9363_remove),

	/* REVISIT: many of these chips have deep power-down modes, which
	 * should clearly be entered on suspend() to minimize power use.
	 * And also when they're otherwise idle...
	 */
};

static int __init ad9363_init(void)
{
	int ret;

	ret = spi_register_driver(&ad9363_driver);
	printk(AD9363_DRIVER_NAME"\t initialized %s!\n", (0==ret)?"successed":"failed");	

	return ret;

}

static void __exit ad9363_cleanup_module(void)
{
	spi_unregister_driver(&ad9363_driver);
}


module_init(ad9363_init);
module_exit(ad9363_cleanup_module);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("SinoMartin");
MODULE_DESCRIPTION(" Direct character-device access to AD9363 devices");

