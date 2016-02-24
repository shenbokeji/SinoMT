#include <linux/module.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/fb.h>
#include <asm/uaccess.h>
#include <linux/i2c.h>
#include <linux/i2c-id.h>
#include <linux/videodev2.h>
#include <mach/gpio.h>
#include <media/v4l2-device.h>
#include <media/v4l2-chip-ident.h>
#include <mach/hardware.h>

//#include <media/v4l2-i2c-drv.h>
#include <linux/miscdevice.h>  

#include <linux/delay.h>
#include <media/v4l2-subdev.h>
#include <media/davinci/videohd.h>



MODULE_DESCRIPTION("Analog Devices ADV7611 video encoder driver");
MODULE_AUTHOR("Dave Perks");
MODULE_LICENSE("GPL");

#include "adv7611.h"

#define ADV7611_DRIVER_NAME  "adv7611_reg"
#define ADV7611_DEVICE_MINOR   (227)


static int debug;
module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "Debug level (0-1)");


struct adv7611_decoder {
	struct v4l2_subdev sd;

	int ver;
	int streaming;

	struct v4l2_pix_format pix;
	//int num_fmts;
	//const struct v4l2_fmtdesc *fmt_list;

	int current_std;
	/*
	int num_stds;
	struct tvp7002_std_info *std_list;
	*/
	/* Input and Output Routing parameters */
	u32 input;
	u32 output;
};

static struct adv7611_decoder adv7611_devinfo = {
	.streaming = 0,

	//.fmt_list = tvp7002_fmt_list,
	//.num_fmts = ARRAY_SIZE(tvp7002_fmt_list),

	.pix = {
		/* Default to NTSC 8-bit YUV 422 */
		.width = HD_720_NUM_ACTIVE_PIXELS,
		.height = HD_720_NUM_ACTIVE_LINES,
		.pixelformat = V4L2_PIX_FMT_UYVY,
		.field = V4L2_FIELD_NONE,
		.bytesperline = HD_720_NUM_ACTIVE_PIXELS * 2,
		.sizeimage =
		HD_720_NUM_ACTIVE_PIXELS * 2 * HD_720_NUM_ACTIVE_LINES,
		.colorspace = V4L2_COLORSPACE_JPEG,
		},

	.current_std = 0,
	/*
	.std_list = tvp7002_std_list,
	.num_stds = ARRAY_SIZE(tvp7002_std_list),
	*/
};


/****************************************************************************
			I2C Client & Driver
 ****************************************************************************/

static struct v4l2_subdev *adv7611_sd;

#define I2C_RETRY_COUNT                 (5)

int adv7611_read(unsigned char devaddress, unsigned char address)
{
	//struct i2c_adapter *adap;
	int err, retry = 0;
	unsigned short client_temp;
	struct i2c_client *client = v4l2_get_subdevdata(adv7611_sd);

	//printk("adv7611_read(%02x,%02x,%02x)\n",client->addr,devaddress,address);

	client_temp = client->addr;
	client->addr = devaddress>>1;
	read_again:
	
		err = i2c_smbus_read_byte_data(client, address);
		if (err == -1) {
			if (retry <= I2C_RETRY_COUNT) {
				v4l2_warn(adv7611_sd, "Read: retry ... %d\n", retry);
				retry++;
				msleep_interruptible(10);
				goto read_again;
			}
		}
	client->addr = client_temp;
	return err;

}


int adv7611_write(unsigned char devaddress, unsigned char address, unsigned char value)
{
	int err, retry = 0;
	unsigned short client_temp;
	struct i2c_client *client = v4l2_get_subdevdata(adv7611_sd);

	printk("adv7611_write(%02x,%02x,%02x,%02x)\n",client->addr,devaddress,address,value);


	client_temp = client->addr;
	client->addr = devaddress>>1;

	write_again:
	
		err = i2c_smbus_write_byte_data(client, address, value);
		if (err) {
			if (retry <= I2C_RETRY_COUNT) {
				v4l2_warn(adv7611_sd, "Write: retry ... %d\n", retry);
				retry++;
				msleep_interruptible(10);
				goto write_again;
			}
		}

	client->addr = client_temp;
	return err;
}


#if 1

void adv7611_reset(void)
{

	adv7611_write(0x98, 0xFF, 0x80); // I2C reset
	// 56 17 0x02); // Encoder reset
}

// :6-1e Port A, 720p,1080i Any Color Space In (YCrCb 444 24bit from ADV761x) 
void adv7611_720P_1080i(void)
{
	//adv7611_write(0x98, 0xFF, 0x80); // I2C reset
	adv7611_write(0x98, 0x01, 0x06); // Prim_Mode =110b HDMI-GR	60 Hz
	adv7611_write(0x98, 0x02, 0xF5); // Auto CSC, YCrCb out, Set op_656 bit
	//adv7611_write(0x98, 0x03, 0x40); // 24 bit SDR 444 Mode 0
	adv7611_write(0x98, 0x03, 0x80); // 16-bit 4:2:2 SDR mode 0 
	//adv7611_write(0x98, 0x04, 0x80); // 16-bit 4:2:2 SDR mode 0 
	adv7611_write(0x98, 0x05, 0x28); // AV Codes Off
	adv7611_write(0x98, 0x06, 0xA6); // Invert VS,HS pins
	adv7611_write(0x98, 0x0B, 0x44); // Power up part
	adv7611_write(0x98, 0x0C, 0x42); // Power up part
	adv7611_write(0x98, 0x14, 0x7F); // Max Drive Strength
	adv7611_write(0x98, 0x15, 0x80); // Disable Tristate of Pins
	adv7611_write(0x98, 0x19, 0x83); // LLC DLL phase
	adv7611_write(0x98, 0x33, 0x40); // LLC DLL enable
	adv7611_write(0x44, 0xBA, 0x01); // Set HDMI FreeRun
	adv7611_write(0x64, 0x40, 0x81); // Disable HDCP 1.1 features
	adv7611_write(0x68, 0x9B, 0x03); // ADI recommended setting
	adv7611_write(0x68, 0xC1, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC2, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC3, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC4, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC5, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC6, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC7, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC8, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC9, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xCA, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xCB, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xCC, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0x00, 0x00); // Set HDMI Input Port A
	adv7611_write(0x68, 0x83, 0xFE); // Enable clock terminator for port A
	adv7611_write(0x68, 0x6F, 0x0C); // ADI recommended setting
	adv7611_write(0x68, 0x85, 0x1F); // ADI recommended setting
	adv7611_write(0x68, 0x87, 0x70); // ADI recommended setting
	adv7611_write(0x68, 0x8D, 0x04); // LFG
	adv7611_write(0x68, 0x8E, 0x1E); // HFG
	adv7611_write(0x68, 0x1A, 0x8A); // unmute audio
	adv7611_write(0x68, 0x57, 0xDA); // ADI recommended setting
	adv7611_write(0x68, 0x58, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0x03, 0x98); // DIS_I2C_ZERO_COMPR
	adv7611_write(0x68, 0x75, 0x10); // DDC drive strength
}

// 6-1f Port A, 1080p Any Color Space In (YCrCb 444 24bit from ADV761x) 
void adv7611_1080P(void)
{
	adv7611_write(0x98, 0xFF, 0x80); // I2C reset
	adv7611_write(0x98, 0xF4, 0x80); // CEC
	adv7611_write(0x98, 0xF5, 0x7C); // INFOFRAME
	adv7611_write(0x98, 0xF8, 0x4C); // DPLL
	adv7611_write(0x98, 0xF9, 0x64); // KSV
	adv7611_write(0x98, 0xFA, 0x6C); // EDID
	adv7611_write(0x98, 0xFB, 0x68); // HDMI
	adv7611_write(0x98, 0xFD, 0x44); // CP

	adv7611_write(0x98, 0x01, 0x06); // Prim_Mode =110b HDMI-GR
	adv7611_write(0x98, 0x02, 0xF5); // Auto CSC, YCrCb out, Set op_656 bit
	//adv7611_write(0x98, 0x03, 0x40); // 24 bit SDR 444 Mode 0
	adv7611_write(0x98, 0x03, 0x20); // 8-bit 4:2:2 DDR mode 2 (ITU-656 mode)
	//adv7611_write(0x98, 0x05, 0x28); // AV Codes Off
	adv7611_write(0x98, 0x05, 0x2E); // AV Codes Off
	adv7611_write(0x98, 0x06, 0xA6); // Invert VS,HS pins
	adv7611_write(0x98, 0x0B, 0x44); // Power up part
	adv7611_write(0x98, 0x0C, 0x42); // Power up part
	adv7611_write(0x98, 0x14, 0x7F); // Max Drive Strength
	adv7611_write(0x98, 0x15, 0x80); // Disable Tristate of Pins
	adv7611_write(0x98, 0x19, 0x83); // LLC DLL phase
	adv7611_write(0x98, 0x33, 0x40); // LLC DLL enable

	adv7611_write(0x98, 0x20, 0xF8); // Manually assert hot plug on port A
	//adv7611_write(0x80, 0x2A, 0x3F); // power down

	adv7611_write(0x44, 0xBA, 0x01); // Set HDMI FreeRun
	adv7611_write(0x64, 0x40, 0x81); // Disable HDCP 1.1 features

	adv7611_write(0x68, 0x9B, 0x03); // ADI recommended setting
	adv7611_write(0x68, 0xC1, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC2, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC3, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC4, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC5, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC6, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC7, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC8, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC9, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xCA, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xCB, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xCC, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0x00, 0x00); // Set HDMI Input Port A
	adv7611_write(0x68, 0x83, 0xFE); // Enable clock terminator for port A
	adv7611_write(0x68, 0x6F, 0x0C); // ADI recommended setting
	adv7611_write(0x68, 0x85, 0x1F); // ADI recommended setting
	adv7611_write(0x68, 0x87, 0x70); // ADI recommended setting
	adv7611_write(0x68, 0x8D, 0x04); // LFG
	adv7611_write(0x68, 0x8E, 0x1E); // HFG
	adv7611_write(0x68, 0x1A, 0x8A); // unmute audio
	adv7611_write(0x68, 0x57, 0xDA); // ADI recommended setting
	adv7611_write(0x68, 0x58, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0x03, 0x98); // DIS_I2C_ZERO_COMPR
	adv7611_write(0x68, 0x75, 0x10); // DDC drive strength
}

// HDMI PCM Audio ADV7611 ##
// 6-4a - 1920 x 1080p60, Any Color Space In (YCrCb 444 24bit from ADV761x) 
void adv7611_1920_1080P_60(void)
{
	adv7611_write(0x98, 0xFF, 0x80); // I2C reset
	adv7611_write(0x98, 0x01, 0x06); // Prim_Mode =110b HDMI-GR
	adv7611_write(0x98, 0x02, 0xF5); // Auto CSC, YCrCb out, Set op_656 bit
	//adv7611_write(0x98, 0x03, 0x40); // 24 bit SDR 444 Mode 0
	adv7611_write(0x98, 0x03, 0x2A); // 12-bit 4:2:2 DDR mode 2 (ITU-656 mode)
	adv7611_write(0x98, 0x05, 0x28); // AV Codes Off
	adv7611_write(0x98, 0x06, 0xA6); // Invert VS,HS pins
	adv7611_write(0x98, 0x0B, 0x44); // Power up part
	adv7611_write(0x98, 0x0C, 0x42); // Power up part
	adv7611_write(0x98, 0x14, 0x7F); // Max Drive Strength
	adv7611_write(0x98, 0x15, 0x80); // Disable Tristate of Pins
	adv7611_write(0x98, 0x19, 0x83); // LLC DLL phase
	adv7611_write(0x98, 0x33, 0x40); // LLC DLL enable
	adv7611_write(0x44, 0xBA, 0x01); // Set HDMI FreeRun
	adv7611_write(0x64, 0x40, 0x81); // Disable HDCP 1.1 features
	adv7611_write(0x68, 0x9B, 0x03); // ADI recommended setting
	adv7611_write(0x68, 0xC1, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC2, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC3, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC4, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC5, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC6, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC7, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC8, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC9, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xCA, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xCB, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xCC, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0x00, 0x00); // Set HDMI Input Port A
	adv7611_write(0x68, 0x83, 0xFE); // Enable clock terminator for port A
	adv7611_write(0x68, 0x6F, 0x0C); // ADI recommended setting
	adv7611_write(0x68, 0x85, 0x1F); // ADI recommended setting
	adv7611_write(0x68, 0x87, 0x70); // ADI recommended setting
	adv7611_write(0x68, 0x8D, 0x04); // LFG
	adv7611_write(0x68, 0x8E, 0x1E); // HFG
	adv7611_write(0x68, 0x1A, 0x8A); // unmute audio
	adv7611_write(0x68, 0x57, 0xDA); // ADI recommended setting
	adv7611_write(0x68, 0x58, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0x03, 0x98); // DIS_I2C_ZERO_COMPR
	adv7611_write(0x68, 0x75, 0x10); // DDC drive strength
}

// 6-4b - 1920 x 1080p60, Any Color Space In (YCrCb 444 24bit from ADV761x)
void adv7611_1920_1080P_60_2(void)
{
	adv7611_write(0x98, 0xFF, 0x80); // I2C reset
	adv7611_write(0x98, 0x01, 0x06); // Prim_Mode =110b HDMI-GR
	adv7611_write(0x98, 0x02, 0xF5); // Auto CSC, YCrCb out, Set op_656 bit
	//adv7611_write(0x98, 0x03, 0x40); // 24 bit SDR 444 Mode 0
	adv7611_write(0x98, 0x03, 0x2A); // 12-bit 4:2:2 DDR mode 2 (ITU-656 mode)
	adv7611_write(0x98, 0x05, 0x28); // AV Codes Off
	adv7611_write(0x98, 0x06, 0xA6); // Invert VS,HS pins
	adv7611_write(0x98, 0x0B, 0x44); // Power up part
	adv7611_write(0x98, 0x0C, 0x42); // Power up part
	adv7611_write(0x98, 0x14, 0x7F); // Max Drive Strength
	adv7611_write(0x98, 0x15, 0x80); // Disable Tristate of Pins
	adv7611_write(0x98, 0x19, 0x83); // LLC DLL phase
	adv7611_write(0x98, 0x33, 0x40); // LLC DLL enable
	adv7611_write(0x44, 0xBA, 0x01); // Set HDMI FreeRun
	adv7611_write(0x64, 0x40, 0x81); // Disable HDCP 1.1 features
	adv7611_write(0x68, 0x9B, 0x03); // ADI recommended setting
	adv7611_write(0x68, 0xC1, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC2, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC3, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC4, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC5, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC6, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC7, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC8, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC9, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xCA, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xCB, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xCC, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0x00, 0x00); // Set HDMI Input Port A
	adv7611_write(0x68, 0x83, 0xFE); // Enable clock terminator for port A
	adv7611_write(0x68, 0x6F, 0x0C); // ADI recommended setting
	adv7611_write(0x68, 0x85, 0x1F); // ADI recommended setting
	adv7611_write(0x68, 0x87, 0x70); // ADI recommended setting
	adv7611_write(0x68, 0x8D, 0x04); // LFG
	adv7611_write(0x68, 0x8E, 0x1E); // HFG
	adv7611_write(0x68, 0x1A, 0x8A); // unmute audio
	adv7611_write(0x68, 0x57, 0xDA); // ADI recommended setting
	adv7611_write(0x68, 0x58, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0x03, 0x98); // DIS_I2C_ZERO_COMPR
	adv7611_write(0x68, 0x75, 0x10); // DDC drive strength
}

// 6-4c - 1920 x 1080p60, Any Color Space In (YCrCb 444 24bit from ADV761x) 
void adv7611_1920_1080P_60_3(void)
{
	adv7611_write(0x98, 0xFF, 0x80); // I2C reset
	adv7611_write(0x98, 0x01, 0x06); // Prim_Mode =110b HDMI-GR
	adv7611_write(0x98, 0x02, 0xF5); // Auto CSC, YCrCb out, Set op_656 bit
	//adv7611_write(0x98, 0x03, 0x40); // 24 bit SDR 444 Mode 0
	adv7611_write(0x98, 0x03, 0x2A); // 12-bit 4:2:2 DDR mode 2 (ITU-656 mode)
	adv7611_write(0x98, 0x05, 0x28); // AV Codes Off
	adv7611_write(0x98, 0x06, 0xA6); // Invert VS,HS pins
	adv7611_write(0x98, 0x0B, 0x44); // Power up part
	adv7611_write(0x98, 0x0C, 0x42); // Power up part
	adv7611_write(0x98, 0x14, 0x7F); // Max Drive Strength
	adv7611_write(0x98, 0x15, 0x80); // Disable Tristate of Pins
	adv7611_write(0x98, 0x19, 0x83); // LLC DLL phase
	adv7611_write(0x98, 0x33, 0x40); // LLC DLL enable
	adv7611_write(0x44, 0xBA, 0x01); // Set HDMI FreeRun
	adv7611_write(0x64, 0x40, 0x81); // Disable HDCP 1.1 features
	adv7611_write(0x68, 0x9B, 0x03); // ADI recommended setting
	adv7611_write(0x68, 0xC1, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC2, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC3, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC4, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC5, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC6, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC7, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC8, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xC9, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xCA, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xCB, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0xCC, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0x00, 0x00); // Set HDMI Input Port A
	adv7611_write(0x68, 0x83, 0xFE); // Enable clock terminator for port A
	adv7611_write(0x68, 0x6F, 0x0C); // ADI recommended setting
	adv7611_write(0x68, 0x85, 0x1F); // ADI recommended setting
	adv7611_write(0x68, 0x87, 0x70); // ADI recommended setting
	adv7611_write(0x68, 0x8D, 0x04); // LFG
	adv7611_write(0x68, 0x8E, 0x1E); // HFG
	adv7611_write(0x68, 0x1A, 0x8A); // unmute audio
	adv7611_write(0x68, 0x57, 0xDA); // ADI recommended setting
	adv7611_write(0x68, 0x58, 0x01); // ADI recommended setting
	adv7611_write(0x68, 0x03, 0x98); // DIS_I2C_ZERO_COMPR
	adv7611_write(0x68, 0x75, 0x10); // DDC drive strength
}


// ## EDID ##
// ADV7611 EDID 8 bit only NO DSD or HBR Support:
void adv7611_edid_8_bit(void)
{
	adv7611_write(0x64, 0x74, 0x00); // Disable the Internal EDID
	adv7611_write(0x6C, 0x00, 0x00); // 
	adv7611_write(0x6C, 0x01, 0xFF); // 
	adv7611_write(0x6C, 0x02, 0xFF); // 
	adv7611_write(0x6C, 0x03, 0xFF); // 
	adv7611_write(0x6C, 0x04, 0xFF); // 
	adv7611_write(0x6C, 0x05, 0xFF); // 
	adv7611_write(0x6C, 0x06, 0xFF); // 
	adv7611_write(0x6C, 0x07, 0x00); // 
	adv7611_write(0x6C, 0x08, 0x06); // 
	adv7611_write(0x6C, 0x09, 0x8F); // 
	adv7611_write(0x6C, 0x0A, 0x07); // 
	adv7611_write(0x6C, 0x0B, 0x11); // 
	adv7611_write(0x6C, 0x0C, 0x01); // 
	adv7611_write(0x6C, 0x0D, 0x00); // 
	adv7611_write(0x6C, 0x0E, 0x00); // 
	adv7611_write(0x6C, 0x0F, 0x00); // 
	adv7611_write(0x6C, 0x10, 0x17); // 
	adv7611_write(0x6C, 0x11, 0x11); // 
	adv7611_write(0x6C, 0x12, 0x01); // 
	adv7611_write(0x6C, 0x13, 0x03); // 
	adv7611_write(0x6C, 0x14, 0x80); // 
	adv7611_write(0x6C, 0x15, 0x0C); // 
	adv7611_write(0x6C, 0x16, 0x09); // 
	adv7611_write(0x6C, 0x17, 0x78); // 
	adv7611_write(0x6C, 0x18, 0x0A); // 
	adv7611_write(0x6C, 0x19, 0x1E); // 
	adv7611_write(0x6C, 0x1A, 0xAC); // 
	adv7611_write(0x6C, 0x1B, 0x98); // 
	adv7611_write(0x6C, 0x1C, 0x59); // 
	adv7611_write(0x6C, 0x1D, 0x56); // 
	adv7611_write(0x6C, 0x1E, 0x85); // 
	adv7611_write(0x6C, 0x1F, 0x28); // 
	adv7611_write(0x6C, 0x20, 0x29); // 
	adv7611_write(0x6C, 0x21, 0x52); // 
	adv7611_write(0x6C, 0x22, 0x57); // 
	adv7611_write(0x6C, 0x23, 0x00); // 
	adv7611_write(0x6C, 0x24, 0x00); // 
	adv7611_write(0x6C, 0x25, 0x00); // 
	adv7611_write(0x6C, 0x26, 0x01); // 
	adv7611_write(0x6C, 0x27, 0x01); // 
	adv7611_write(0x6C, 0x28, 0x01); // 
	adv7611_write(0x6C, 0x29, 0x01); // 
	adv7611_write(0x6C, 0x2A, 0x01); // 
	adv7611_write(0x6C, 0x2B, 0x01); // 
	adv7611_write(0x6C, 0x2C, 0x01); // 
	adv7611_write(0x6C, 0x2D, 0x01); // 
	adv7611_write(0x6C, 0x2E, 0x01); // 
	adv7611_write(0x6C, 0x2F, 0x01); // 
	adv7611_write(0x6C, 0x30, 0x01); // 
	adv7611_write(0x6C, 0x31, 0x01); // 
	adv7611_write(0x6C, 0x32, 0x01); // 
	adv7611_write(0x6C, 0x33, 0x01); // 
	adv7611_write(0x6C, 0x34, 0x01); // 
	adv7611_write(0x6C, 0x35, 0x01); // 
	adv7611_write(0x6C, 0x36, 0x8C); // 
	adv7611_write(0x6C, 0x37, 0x0A); // 
	adv7611_write(0x6C, 0x38, 0xD0); // 
	adv7611_write(0x6C, 0x39, 0x8A); // 
	adv7611_write(0x6C, 0x3A, 0x20); // 
	adv7611_write(0x6C, 0x3B, 0xE0); // 
	adv7611_write(0x6C, 0x3C, 0x2D); // 
	adv7611_write(0x6C, 0x3D, 0x10); // 
	adv7611_write(0x6C, 0x3E, 0x10); // 
	adv7611_write(0x6C, 0x3F, 0x3E); // 
	adv7611_write(0x6C, 0x40, 0x96); // 
	adv7611_write(0x6C, 0x41, 0x00); // 
	adv7611_write(0x6C, 0x42, 0x81); // 
	adv7611_write(0x6C, 0x43, 0x60); // 
	adv7611_write(0x6C, 0x44, 0x00); // 
	adv7611_write(0x6C, 0x45, 0x00); // 
	adv7611_write(0x6C, 0x46, 0x00); // 
	adv7611_write(0x6C, 0x47, 0x18); // 
	adv7611_write(0x6C, 0x48, 0x01); // 
	adv7611_write(0x6C, 0x49, 0x1D); // 
	adv7611_write(0x6C, 0x4A, 0x80); // 
	adv7611_write(0x6C, 0x4B, 0x18); // 
	adv7611_write(0x6C, 0x4C, 0x71); // 
	adv7611_write(0x6C, 0x4D, 0x1C); // 
	adv7611_write(0x6C, 0x4E, 0x16); // 
	adv7611_write(0x6C, 0x4F, 0x20); // 
	adv7611_write(0x6C, 0x50, 0x58); // 
	adv7611_write(0x6C, 0x51, 0x2C); // 
	adv7611_write(0x6C, 0x52, 0x25); // 
	adv7611_write(0x6C, 0x53, 0x00); // 
	adv7611_write(0x6C, 0x54, 0x81); // 
	adv7611_write(0x6C, 0x55, 0x49); // 
	adv7611_write(0x6C, 0x56, 0x00); // 
	adv7611_write(0x6C, 0x57, 0x00); // 
	adv7611_write(0x6C, 0x58, 0x00); // 
	adv7611_write(0x6C, 0x59, 0x9E); // 
	adv7611_write(0x6C, 0x5A, 0x00); // 
	adv7611_write(0x6C, 0x5B, 0x00); // 
	adv7611_write(0x6C, 0x5C, 0x00); // 
	adv7611_write(0x6C, 0x5D, 0xFC); // 
	adv7611_write(0x6C, 0x5E, 0x00); // 
	adv7611_write(0x6C, 0x5F, 0x56); // 
	adv7611_write(0x6C, 0x60, 0x41); // 
	adv7611_write(0x6C, 0x61, 0x2D); // 
	adv7611_write(0x6C, 0x62, 0x31); // 
	adv7611_write(0x6C, 0x63, 0x38); // 
	adv7611_write(0x6C, 0x64, 0x30); // 
	adv7611_write(0x6C, 0x65, 0x39); // 
	adv7611_write(0x6C, 0x66, 0x41); // 
	adv7611_write(0x6C, 0x67, 0x0A); // 
	adv7611_write(0x6C, 0x68, 0x20); // 
	adv7611_write(0x6C, 0x69, 0x20); // 
	adv7611_write(0x6C, 0x6A, 0x20); // 
	adv7611_write(0x6C, 0x6B, 0x20); // 
	adv7611_write(0x6C, 0x6C, 0x00); // 
	adv7611_write(0x6C, 0x6D, 0x00); // 
	adv7611_write(0x6C, 0x6E, 0x00); // 
	adv7611_write(0x6C, 0x6F, 0xFD); // 
	adv7611_write(0x6C, 0x70, 0x00); // 
	adv7611_write(0x6C, 0x71, 0x17); // 
	adv7611_write(0x6C, 0x72, 0x3D); // 
	adv7611_write(0x6C, 0x73, 0x0D); // 
	adv7611_write(0x6C, 0x74, 0x2E); // 
	adv7611_write(0x6C, 0x75, 0x11); // 
	adv7611_write(0x6C, 0x76, 0x00); // 
	adv7611_write(0x6C, 0x77, 0x0A); // 
	adv7611_write(0x6C, 0x78, 0x20); // 
	adv7611_write(0x6C, 0x79, 0x20); // 
	adv7611_write(0x6C, 0x7A, 0x20); // 
	adv7611_write(0x6C, 0x7B, 0x20); // 
	adv7611_write(0x6C, 0x7C, 0x20); // 
	adv7611_write(0x6C, 0x7D, 0x20); // 
	adv7611_write(0x6C, 0x7E, 0x01); // 
	adv7611_write(0x6C, 0x7F, 0x1C); // 
	adv7611_write(0x6C, 0x80, 0x02); // 
	adv7611_write(0x6C, 0x81, 0x03); // 
	adv7611_write(0x6C, 0x82, 0x34); // 
	adv7611_write(0x6C, 0x83, 0x71); // 
	adv7611_write(0x6C, 0x84, 0x4D); // 
	adv7611_write(0x6C, 0x85, 0x82); // 
	adv7611_write(0x6C, 0x86, 0x05); // 
	adv7611_write(0x6C, 0x87, 0x04); // 
	adv7611_write(0x6C, 0x88, 0x01); // 
	adv7611_write(0x6C, 0x89, 0x10); // 
	adv7611_write(0x6C, 0x8A, 0x11); // 
	adv7611_write(0x6C, 0x8B, 0x14); // 
	adv7611_write(0x6C, 0x8C, 0x13); // 
	adv7611_write(0x6C, 0x8D, 0x1F); // 
	adv7611_write(0x6C, 0x8E, 0x06); // 
	adv7611_write(0x6C, 0x8F, 0x15); // 
	adv7611_write(0x6C, 0x90, 0x03); // 
	adv7611_write(0x6C, 0x91, 0x12); // 
	adv7611_write(0x6C, 0x92, 0x35); // 
	adv7611_write(0x6C, 0x93, 0x0F); // 
	adv7611_write(0x6C, 0x94, 0x7F); // 
	adv7611_write(0x6C, 0x95, 0x07); // 
	adv7611_write(0x6C, 0x96, 0x17); // 
	adv7611_write(0x6C, 0x97, 0x1F); // 
	adv7611_write(0x6C, 0x98, 0x38); // 
	adv7611_write(0x6C, 0x99, 0x1F); // 
	adv7611_write(0x6C, 0x9A, 0x07); // 
	adv7611_write(0x6C, 0x9B, 0x30); // 
	adv7611_write(0x6C, 0x9C, 0x2F); // 
	adv7611_write(0x6C, 0x9D, 0x07); // 
	adv7611_write(0x6C, 0x9E, 0x72); // 
	adv7611_write(0x6C, 0x9F, 0x3F); // 
	adv7611_write(0x6C, 0xA0, 0x7F); // 
	adv7611_write(0x6C, 0xA1, 0x72); // 
	adv7611_write(0x6C, 0xA2, 0x57); // 
	adv7611_write(0x6C, 0xA3, 0x7F); // 
	adv7611_write(0x6C, 0xA4, 0x00); // 
	adv7611_write(0x6C, 0xA5, 0x37); // 
	adv7611_write(0x6C, 0xA6, 0x7F); // 
	adv7611_write(0x6C, 0xA7, 0x72); // 
	adv7611_write(0x6C, 0xA8, 0x83); // 
	adv7611_write(0x6C, 0xA9, 0x4F); // 
	adv7611_write(0x6C, 0xAA, 0x00); // 
	adv7611_write(0x6C, 0xAB, 0x00); // 
	adv7611_write(0x6C, 0xAC, 0x67); // 
	adv7611_write(0x6C, 0xAD, 0x03); // 
	adv7611_write(0x6C, 0xAE, 0x0C); // 
	adv7611_write(0x6C, 0xAF, 0x00); // 
	adv7611_write(0x6C, 0xB0, 0x10); // 
	adv7611_write(0x6C, 0xB1, 0x00); // 
	adv7611_write(0x6C, 0xB2, 0x88); // 
	adv7611_write(0x6C, 0xB3, 0x2D); // 
	adv7611_write(0x6C, 0xB4, 0x00); // 
	adv7611_write(0x6C, 0xB5, 0x00); // 
	adv7611_write(0x6C, 0xB6, 0x00); // 
	adv7611_write(0x6C, 0xB7, 0xFF); // 
	adv7611_write(0x6C, 0xB8, 0x00); // 
	adv7611_write(0x6C, 0xB9, 0x0A); // 
	adv7611_write(0x6C, 0xBA, 0x20); // 
	adv7611_write(0x6C, 0xBB, 0x20); // 
	adv7611_write(0x6C, 0xBC, 0x20); // 
	adv7611_write(0x6C, 0xBD, 0x20); // 
	adv7611_write(0x6C, 0xBE, 0x20); // 
	adv7611_write(0x6C, 0xBF, 0x20); // 
	adv7611_write(0x6C, 0xC0, 0x20); // 
	adv7611_write(0x6C, 0xC1, 0x20); // 
	adv7611_write(0x6C, 0xC2, 0x20); // 
	adv7611_write(0x6C, 0xC3, 0x20); // 
	adv7611_write(0x6C, 0xC4, 0x20); // 
	adv7611_write(0x6C, 0xC5, 0x20); // 
	adv7611_write(0x6C, 0xC6, 0x00); // 
	adv7611_write(0x6C, 0xC7, 0x00); // 
	adv7611_write(0x6C, 0xC8, 0x00); // 
	adv7611_write(0x6C, 0xC9, 0xFF); // 
	adv7611_write(0x6C, 0xCA, 0x00); // 
	adv7611_write(0x6C, 0xCB, 0x0A); // 
	adv7611_write(0x6C, 0xCC, 0x20); // 
	adv7611_write(0x6C, 0xCD, 0x20); // 
	adv7611_write(0x6C, 0xCE, 0x20); // 
	adv7611_write(0x6C, 0xCF, 0x20); // 
	adv7611_write(0x6C, 0xD0, 0x20); // 
	adv7611_write(0x6C, 0xD1, 0x20); // 
	adv7611_write(0x6C, 0xD2, 0x20); // 
	adv7611_write(0x6C, 0xD3, 0x20); // 
	adv7611_write(0x6C, 0xD4, 0x20); // 
	adv7611_write(0x6C, 0xD5, 0x20); // 
	adv7611_write(0x6C, 0xD6, 0x20); // 
	adv7611_write(0x6C, 0xD7, 0x20); // 
	adv7611_write(0x6C, 0xD8, 0x00); // 
	adv7611_write(0x6C, 0xD9, 0x00); // 
	adv7611_write(0x6C, 0xDA, 0x00); // 
	adv7611_write(0x6C, 0xDB, 0xFF); // 
	adv7611_write(0x6C, 0xDC, 0x00); // 
	adv7611_write(0x6C, 0xDD, 0x0A); // 
	adv7611_write(0x6C, 0xDE, 0x20); // 
	adv7611_write(0x6C, 0xDF, 0x20); // 
	adv7611_write(0x6C, 0xE0, 0x20); // 
	adv7611_write(0x6C, 0xE1, 0x20); // 
	adv7611_write(0x6C, 0xE2, 0x20); // 
	adv7611_write(0x6C, 0xE3, 0x20); // 
	adv7611_write(0x6C, 0xE4, 0x20); // 
	adv7611_write(0x6C, 0xE5, 0x20); // 
	adv7611_write(0x6C, 0xE6, 0x20); // 
	adv7611_write(0x6C, 0xE7, 0x20); // 
	adv7611_write(0x6C, 0xE8, 0x20); // 
	adv7611_write(0x6C, 0xE9, 0x20); // 
	adv7611_write(0x6C, 0xEA, 0x00); // 
	adv7611_write(0x6C, 0xEB, 0x00); // 
	adv7611_write(0x6C, 0xEC, 0x00); // 
	adv7611_write(0x6C, 0xED, 0x00); // 
	adv7611_write(0x6C, 0xEE, 0x00); // 
	adv7611_write(0x6C, 0xEF, 0x00); // 
	adv7611_write(0x6C, 0xF0, 0x00); // 
	adv7611_write(0x6C, 0xF1, 0x00); // 
	adv7611_write(0x6C, 0xF2, 0x00); // 
	adv7611_write(0x6C, 0xF3, 0x00); // 
	adv7611_write(0x6C, 0xF4, 0x00); // 
	adv7611_write(0x6C, 0xF5, 0x00); // 
	adv7611_write(0x6C, 0xF6, 0x00); // 
	adv7611_write(0x6C, 0xF7, 0x00); // 
	adv7611_write(0x6C, 0xF8, 0x00); // 
	adv7611_write(0x6C, 0xF9, 0x00); // 
	adv7611_write(0x6C, 0xFA, 0x00); // 
	adv7611_write(0x6C, 0xFB, 0x00); // 
	adv7611_write(0x6C, 0xFC, 0x00); // 
	adv7611_write(0x6C, 0xFD, 0x00); // 
	adv7611_write(0x6C, 0xFE, 0x00); // 
	adv7611_write(0x6C, 0xFF, 0xDA); // 
	adv7611_write(0x64, 0x77, 0x00); // Set the Most Significant Bit of the SPA location to 0
	adv7611_write(0x64, 0x52, 0x20); // Set the SPA for port B.
	adv7611_write(0x64, 0x53, 0x00); // Set the SPA for port B.
	adv7611_write(0x64, 0x70, 0x9E); // Set the Least Significant Byte of the SPA location
	adv7611_write(0x64, 0x74, 0x03); // Enable the Internal EDID for Ports
}

#endif
unsigned char i2c_address[] = {
	/*  reg   value */
	0xF4, CEC__MAP_I2C,
	0xF5, INFO_MAP_I2C,
	0xF8, DPLL_MAP_I2C,
	0xF9, KSV__MAP_I2C,
	0xFA, EDID_MAP_I2C,
	0xFB, HDMI_MAP_I2C,
	0xFD, CP___MAP_I2C,
};

/*
 *	address is CP___MAP_I2C
 *
 *	CP : Component Processor
 */
unsigned char cp_map[] = {
	0x6C, 0x00
};

/*
 *	address is IO___MAP_I2C
 */
unsigned char io_map[] = {
	0x01, (0x0<<4)|0x6,	/* 60Hz, HDMI-GR */
	0x02, 0x00,
	0x03, 0x2A,			/* 12-bit 4:2:2 DDR mode 2 (ITU-656 mode) */
	0x04, 0x01<<1,		/* XTAL : 28.63636 MHZ */
};

/*
 *	address is HDMI_MAP_I2C
 */
unsigned char hdmi_map[] = {
	0x9B, 0x03,
	0x6F, 0x0c,
	0x85, 0x1f,
	0x87, 0x70,
	0x57, 0xda, 
	0x58, 0x01,
	0x03, 0x98,
	0x4C, 0x44,
	0xC1, 0x01,
	0xC2, 0x01,
	0xC3, 0x01, 
	0xC4, 0x01,
	0xC5, 0x01,
	0xC6, 0x01,
	0xC7, 0x01,
	0xC8, 0x01,
	0xC9, 0x01,
	0xCA, 0x01,
	0xCB, 0x01,
	0xCC, 0x01,
};


static ssize_t adv7611_open(struct inode * indoe, struct file * file)
{
	return 0;
}

static ssize_t adv7611_close(struct inode * indoe, struct file * file)
{
	return 0;
}

static void adv7611_write_table(unsigned char addr, unsigned char num, unsigned char * value)
{
	unsigned char i;
	unsigned char j=0;
	unsigned char k=1;

	for ( i = 0; i < num; i++) {
		adv7611_write(addr, *(value + j), *(value + k));
		j += 2; k += 2;
	}
}

static void write_map_i2c_address(unsigned char addr, unsigned char num,  unsigned char * value)
{
	adv7611_write_table(addr, num, value);
}

typedef struct ADV7611RegStruct {
    unsigned int uiAddr;	//I2C reg, address  
    unsigned int uiValue;		//I2C bus,data
} tadv7611Reg;

static tadv7611Reg adv7611_reg={0};


//static unsigned int rw_buf=0;
ssize_t adv7611_readreg(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{

		copy_from_user(&adv7611_reg, buf, 8);
		adv7611_reg.uiValue = adv7611_read((unsigned char)(adv7611_reg.uiAddr>>16),(unsigned char)(adv7611_reg.uiAddr&0xff));
		copy_to_user(buf, &adv7611_reg, 8); 
		//printk("adv7611_readreg(%x,%x)\n",adv7611_reg.uiAddr,adv7611_reg.uiValue);
		
		return 8;
}

ssize_t adv7611_writereg(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{

    copy_from_user(&adv7611_reg, buf, 8);
	printk("adv7611_writereg(%x,%x)\n",adv7611_reg.uiAddr,adv7611_reg.uiValue);
	adv7611_write((unsigned char)(adv7611_reg.uiAddr>>16),(unsigned char)(adv7611_reg.uiAddr&0xff),(unsigned char)adv7611_reg.uiValue);
     
    return 8;

}

/*
 * Initialization settings for HDMI Mode.
 */

#define GET_INFO	0U
#define SET_INFO	1U

#define ADV7611_CMD_SIZE	2U



static int adv7611_ioctl(struct inode *inode, struct file *file,
		     u_int cmd, u_long arg)
{
	int ret = 0;

	switch (cmd) {
	
		default:
			ret = -ENOTTY;
	}

	return ret;
}


/*
 *	ºÏ≤‚ HDMI cableΩ”»Î
 */
static int check_hdmi_cable(void)
{
	unsigned char reg;

	reg = adv7611_read(IO___MAP_I2C,0x6F); 
	if (reg & 0x01)
		return 1;
	
	return 0;
}

/*
 *	ºÏ≤‚ HDMI Clock
 */
static int check_hdmi_clock(void)
{
	unsigned char reg;

	reg = adv7611_read(IO___MAP_I2C,0x6A); 
	if (reg & 0x10)
		return 1;
	
	return 0;
}
#if 1
void DumpHDMIRXReg()
{
    int i,j ;
    unsigned char ucData ;

    printk("       ");
    for(j = 0 ; j < 16 ; j++)
    {
        printk(" %02X",(int)j);
        if((j == 3)||(j==7)||(j==11))
        {
            printk("  ");
        }
    }
    printk("\n        -----------------------------------------------------\n");

    //Switch_HDMITX_Bank(0);

    for(i = 0 ; i < 0x100 ; i+=16)
    {
        printk("[%3X]  ",i);
        for(j = 0 ; j < 16 ; j++)
        {

            ucData = adv7611_read(0x98,((i+j)&0xFF));
            printk(" %02X",(int)ucData);


            if((j == 3)||(j==7)||(j==11))
            {
                printk(" -");
            }
        }
        printk("\n");
        if((i % 0x40) == 0x30)
        {
            printk("        -----------------------------------------------------\n");
        }
    }
}

#endif


static int adv7611_device_init(void)
{
    unsigned char ireg, reg;

	__raw_writel(0xFFFFFF7F,IO_ADDRESS(0x01c67038));
	//udelay(20000);
	__raw_writel(0x14456745,IO_ADDRESS(0x01c40010));
	//udelay(20000);
	__raw_writel(0x0000080,IO_ADDRESS(0x01c67040));
	//udelay(20000);	

	ireg = adv7611_read(ADV7611__I2C,0xF4); 
    adv7611_write(ADV7611__I2C,0xF4, 0x80);
    reg = adv7611_read(ADV7611__I2C,0xF4); 
	if( reg != 0x80 ) {
        printk( "ERROR : read adv7611 register is 0x%02x\n",reg);
        return -EFAULT;        
    }
	adv7611_write(ADV7611__I2C,0xF4, ireg);

	adv7611_reset();
	//udelay(1000000);
	write_map_i2c_address( ADV7611__I2C, sizeof ( i2c_address ) / 2, i2c_address );
	//write_map_i2c_address( ADV7611__I2C, sizeof ( io_map ) / 2, io_map );
	//initialize_hdmi_mode ( HDMI_MAP_I2C, sizeof (hdmi_map) / 2, hdmi_map);
	adv7611_720P_1080i();
	adv7611_edid_8_bit();
	reg = check_hdmi_cable();
	printk("\t[check_hdmi_cable = 0x%02x]\n", reg);
	reg = check_hdmi_clock();
	printk("\t[check_hdmi_clock = 0x%02x]\n", reg);

	DumpHDMIRXReg();

	return 0;
}

/* adv7611_querystd :
* Function to return standard detected by decoder
*/
static int adv7611_querystd(struct v4l2_subdev *sd, v4l2_std_id *id)
{
	int err = 0;
	u32 val, val1, val_t;
	u32  width,height,field;

	v4l2_dbg(1, debug, sd, "Starting querystd function...\n");
	if (id == NULL) {
		v4l2_err(sd, "NULL Pointer.\n");
		return -EINVAL;
	}

	val = adv7611_read(0x68, 0x07);
	val &= 0x1F;
	val <<= 8;
	val += adv7611_read(0x68, 0x08);
	if(val == 0x1FFF) return -ENODEV;
	width =val;
	
	val1 = adv7611_read(0x68, 0x09);
	val1 &= 0x1F;
	val1 <<= 8;
	val1 += adv7611_read(0x68, 0x0A);	
	if(val1 == 0x1FFF) return -ENODEV;
	height = val1;
		
	val_t = adv7611_read(0x68, 0x0B)&0x20;

	if(val_t==0x0) field= V4L2_FIELD_NONE;
	else  field=V4L2_FIELD_INTERLACED;



	if ((V4L2_FIELD_INTERLACED == field)&& (HD_1080_NUM_ACTIVE_PIXELS == width) &&
		(height == HD_1080_NUM_ACTIVE_LINES))
		*id = V4L2_STD_1080I_60;
	else if ((V4L2_FIELD_NONE == field)&& (HD_1080_NUM_ACTIVE_PIXELS == width) &&
		(height == HD_1080_NUM_ACTIVE_LINES))
		*id = V4L2_STD_1080P_60;
	else if ((V4L2_FIELD_NONE == field)&& (HD_720_NUM_ACTIVE_PIXELS == width) &&
		(height == HD_720_NUM_ACTIVE_LINES))
		*id = V4L2_STD_720P_60;
	else {
		v4l2_err(sd,
			"querystd, erorxxx, val = %x, val1 = %x\n", val, val1);
		return -EINVAL;
	}

	printk( "End of querystd function.\n");

	return err;
}


static int adv7611_g_fmt_cap(struct v4l2_subdev *sd, struct v4l2_format *f)
{
	struct adv7611_decoder *decoder = &adv7611_devinfo;
	struct v4l2_pix_format *pix = &(adv7611_devinfo.pix);
	u32 value=0;

	if (f == NULL)
		return -EINVAL;

	if (f->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		/* only capture is supported */
		return -EINVAL;

	
	value = adv7611_read(0x68, 0x07);
	value &= 0x1F;
	value <<= 8;
	value += adv7611_read(0x68, 0x08);
	if(value == 0x1FFF) return -ENODEV;
	pix->width		=  value;

	value = adv7611_read(0x68, 0x09);
	value &= 0x1F;
	value <<= 8;
	value += adv7611_read(0x68, 0x0A);	
	if(value == 0x1FFF) return -ENODEV;
	if(0x20==(adv7611_read(0x68, 0x0B)&0x20)) value <<= 1;
	pix->height		= value;
	
	pix->pixelformat	= V4L2_PIX_FMT_YUYV;
	pix->field		= V4L2_FIELD_NONE;
	pix->colorspace		= V4L2_COLORSPACE_JPEG;	
	decoder->pix.bytesperline = pix->width * 2;

	f->fmt.pix = adv7611_devinfo.pix;

	//v4l2_dbg(1, debug, sd, "Current FMT: bytesperline - %d"
	printk("Current FMT: bytesperline - %d Width - %d, Height - %d",
			decoder->pix.bytesperline,
			decoder->pix.width, decoder->pix.height);
	return 0;
}

static int adv7611_s_stream(struct v4l2_subdev *sd, int enable)
{
	int err = 0;
	struct adv7611_decoder *decoder = &adv7611_devinfo;

	if (decoder->streaming == enable)
		return 0;

	switch (enable) {
	case 0:
	{
		//err = tvp7002_write_reg(sd, TVP7002_MISC_CONTROL_2, 0x1);
		//if (err) {
		//	v4l2_err(sd, "Unable to turn off decoder\n");
		//	return err;
		//}
		decoder->streaming = enable;
		break;
	}
	case 1:
	{
		/* Power Up Sequence */
		//err = tvp7002_write_reg(sd, TVP7002_MISC_CONTROL_2, 0x0);
		//if (err) {
			//v4l2_err(sd, "Unable to turn off decoder\n");
			//return err;
		//}
		decoder->streaming = enable;
		break;
	}
	default:
		err = -ENODEV;
		break;
	}

	return err;
}


/* ----------------------------------------------------------------------- */

static const struct v4l2_subdev_core_ops adv7611_core_ops = {
	//.log_status = adv7611_log_status,
	//.g_ctrl = adv7611_g_ctrl,
	//.s_ctrl = adv7611_s_ctrl,
	//.queryctrl = adv7611_queryctrl,
	//.s_std = adv7611_s_std,
	//.reset = adv7611_reset,
	//.g_chip_ident = adv7611_g_chip_ident,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	//.g_register = adv7611_g_register,
	//.s_register = adv7611_s_register,
#endif
};

static const struct v4l2_subdev_video_ops adv7611_video_ops = {
	.querystd = adv7611_querystd,
	.g_fmt = adv7611_g_fmt_cap,
	.s_stream = adv7611_s_stream,

};

static const struct v4l2_subdev_ops adv7611_ops = {
	.core = &adv7611_core_ops,
	//.tuner = &tvp5150_tuner_ops,
	.video = &adv7611_video_ops,
};

static const struct file_operations adv7611_fops = {
	.owner		= THIS_MODULE,
	.read           = adv7611_readreg,
	.write           = adv7611_writereg,
	.ioctl		= adv7611_ioctl,
	.open		= adv7611_open,
	.release	= adv7611_close,

};

static struct miscdevice adv7611_dev = {
 	.minor = ADV7611_DEVICE_MINOR,  
	.name = ADV7611_DRIVER_NAME,  
	.fops = &adv7611_fops,  
};


static int adv7611_probe(struct i2c_client *c,
			 const struct i2c_device_id *id)
{
	//struct adv7611_state *state;
	int ret;

		/* Check if the adapter supports the needed features */
	if (!i2c_check_functionality(c->adapter,
	     I2C_FUNC_SMBUS_READ_BYTE | I2C_FUNC_SMBUS_WRITE_BYTE_DATA))
		return -EIO;

	adv7611_sd = kzalloc(sizeof(struct v4l2_subdev), GFP_KERNEL);
	if (adv7611_sd == NULL)
		return -ENOMEM;

	v4l2_i2c_subdev_init(adv7611_sd, c, &adv7611_ops);
	v4l_info(c, "chip found @ 0x%02x (%s)\n",
		 c->addr << 1, c->adapter->name);

	adv7611_device_init();

	ret = misc_register(&adv7611_dev);
	printk(ADV7611_DRIVER_NAME"\t misc initialized %s!\n", (0==ret)?"successed":"failed");

	//if (debug > 1)
		//adv7611_log_status(sd);
	return 0;
}


static int adv7611_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);

	v4l2_device_unregister_subdev(sd);

	misc_deregister(&adv7611_dev);

	return 0;
}
/* ----------------------------------------------------------------------- */

static const struct i2c_device_id adv7611_id[] = {
	{ "adv7611", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, adv7611_id);

static struct i2c_driver adv7611_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "adv7611",
	},	
	.command = adv7611_ioctl,
	.probe = adv7611_probe,
	.remove = adv7611_remove,
	.id_table = adv7611_id,
};


static int __init adv7611_init(void)
{
	if( LENA_AIR == device_lena_air_id )
	{
		return i2c_add_driver(&adv7611_driver);
	}
	return 0;
}

static void __exit adv7611_exit(void)
{
	if( LENA_AIR == device_lena_air_id )
	{
		i2c_del_driver(&adv7611_driver);
	}
}

module_init(adv7611_init);
module_exit(adv7611_exit);
MODULE_LICENSE("GPL");


