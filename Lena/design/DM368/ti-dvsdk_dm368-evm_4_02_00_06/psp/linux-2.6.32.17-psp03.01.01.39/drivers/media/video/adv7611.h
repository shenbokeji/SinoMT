#ifndef __ADV7611_H__
#define __ADV7611_H__

//#include "gpio_i2c.h"

//#define adv7611_write(chip_addr,reg_addr,value)	gpio_i2c_write(chip_addr,reg_addr,value) 
//#define adv7611_read(chip_addr,reg_addr) gpio_i2c_read(chip_addr,reg_addr) 

#define SUCCESS		1
#define FAIL		0
/*
 *	I2C Address.
 */
#define ADV7611__I2C		0x98
//#define ADV7611__I2C		0x9A

/*
 *	I2C Map Address
 */
#define IO___MAP_I2C		0x98	/* same of the ADV7611__I2C */
#define CEC__MAP_I2C		0x80
#define INFO_MAP_I2C		0x7c
#define DPLL_MAP_I2C		0x4c
#define KSV__MAP_I2C		0x64
#define EDID_MAP_I2C		0x6c
#define HDMI_MAP_I2C		0x68
#define CP___MAP_I2C		0x44




typedef struct device_operation
{
	unsigned char addr;
	unsigned char reg;
	unsigned char value;
}device_operation;


/*
 * ioctl cmd
 */
#define ADV7611_IOC_MAGIC		'v'

#define ADV7611_GET_ALL_REG			_IO(ADV7611_IOC_MAGIC, 1)
#define ADV7611_GET_SINGLE_REG		_IOWR(ADV7611_IOC_MAGIC, 2, struct device_operation)
#define ADV7611_SET_SINGLE_REG		_IOWR(ADV7611_IOC_MAGIC, 3, struct device_operation)

void adv7611_adi_recommended_setting(void);
void adv7611_720P_1080i(void);
void adv7611_1080P(void);
void adv7611_1920_1080P_60(void);
void adv7611_1920_1080P_60_2(void);
void adv7611_1920_1080P_60_3(void);
void adv7611_edid_8_bit(void);
void adv7611_reset(void);


#endif	/* __ADV7611_H__ */

