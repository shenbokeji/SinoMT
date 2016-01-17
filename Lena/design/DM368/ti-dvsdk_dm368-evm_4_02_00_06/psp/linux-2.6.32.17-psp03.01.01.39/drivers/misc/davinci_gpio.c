/*
 * Character-device access to GPIO
 *
 */

#include <linux/fs.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h> 
#define GPIO_DEVICE_NAME "davinci_gpio"   
#define GPIO_DEVICE_MINOR   (225)	 

static int gpio_open(struct inode *inode, struct file *file)
{
	//printk( GPIO_DEVICE_NAME"\t gpio open");	

	return 0;
} /* gpio_open */

/*====================================================================*/

static int gpio_close(struct inode *inode, struct file *file)
{
	//printk( GPIO_DEVICE_NAME"\t gpio close");

	return 0;
} /* gpio_close */



//count :used as GPIO number

ssize_t gpio_read(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{
	unsigned int uiFlag = 0XFFFFFFFF;
	gpio_direction_input( count );
	uiFlag = gpio_get_value(count);

	return uiFlag;
} /* gpio_read */


ssize_t gpio_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{

	gpio_direction_output( count, (unsigned int)*buf );
    return 0;
}/* gpio_write */

static int gpio_ioctl(struct inode *inode, struct file *file,
		     u_int cmd, u_long arg)
{
	return 0;
} /* gpio_ioctl */


static const struct file_operations gpio_fops = {
	.owner		= THIS_MODULE,
	.read       = gpio_read,
	.write      = gpio_write,
	.ioctl	    = gpio_ioctl,
	.open	    = gpio_open,
	.release    = gpio_close,

};

static struct miscdevice gpio_dev = { 
	.minor = GPIO_DEVICE_MINOR,
	.name = GPIO_DEVICE_NAME,  
	.fops = &gpio_fops,  
};

static int __init gpio_init(void)
{
	int ret;

	ret = misc_register( &gpio_dev );
	printk(GPIO_DEVICE_NAME"\t initialized %s!\n", (0==ret)?"successed":"failed");	
	return ret;
}

static void __exit gpio_cleanup_module(void)
{
	misc_deregister( &gpio_dev );
	return;
}

module_init(gpio_init);
module_exit(gpio_cleanup_module);	  
	 
MODULE_LICENSE("GPL");	 
MODULE_AUTHOR("SinoMartin");
MODULE_DESCRIPTION("Davinci DM368 gpio driver");

