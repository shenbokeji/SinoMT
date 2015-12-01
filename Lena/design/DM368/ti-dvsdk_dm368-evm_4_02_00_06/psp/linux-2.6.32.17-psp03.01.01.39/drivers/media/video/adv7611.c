#include <linux/module.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <asm/uaccess.h>
#include <linux/i2c.h>
#include <linux/i2c-id.h>
#include <linux/videodev2.h>
#include <media/v4l2-device.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-i2c-drv.h>

MODULE_DESCRIPTION("Analog Devices ADV7175 video encoder driver");
MODULE_AUTHOR("Dave Perks");
MODULE_LICENSE("GPL");

#include "adv7611.h"

static int debug;
module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "Debug level (0-1)");

static unsigned char buf[2]={};

/****************************************************************************
			I2C Client & Driver
 ****************************************************************************/
struct adv7611_state {
	struct v4l2_subdev sd;
};
static struct v4l2_subdev *adv7611_sd;


unsigned char adv7611_read(unsigned char devaddress, unsigned char address)
{
	struct i2c_adapter *adap;
	struct i2c_client *c = v4l2_get_subdevdata(adv7611_sd);
	struct i2c_msg msg[2];
	int ret;

	adap=c->adapter;

	buf[0] = address;

	msg[0].addr = devaddress;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = (unsigned char *)buf;

	msg[1].addr = devaddress;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = (unsigned char *)buf;

	ret = i2c_transfer(adap, &msg, 2);

	/* If everything went ok (i.e. 1 msg transmitted), return #bytes
	   transmitted, else error code. */
	return ret;
}

void adv7611_write(unsigned char devaddress, unsigned char address, unsigned char value)
{
	struct i2c_adapter *adap;
	struct i2c_client *c = v4l2_get_subdevdata(adv7611_sd);
	struct i2c_msg msg;

	adap=c->adapter;

	buf[0] = address;
	buf[1] = value;

	msg.addr = devaddress;
	msg.flags = 0;
	msg.len = 2;
	msg.buf = (char *)buf;

	i2c_transfer(adap, &msg, 1);

	return 0;
}

#if 1

// Added Recommended Settings for non-fast switching scripts
void adv7611_adi_recommended_setting(void)
{
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
}

void adv7611_reset(void)
{
	adv7611_write(0x98, 0xFF, 0x80); // I2C reset
	// 56 17 0x02); // Encoder reset
}

// :6-1e Port A, 720p,1080i Any Color Space In (YCrCb 444 24bit from ADV761x) 
void adv7611_720P_1080i(void)
{
	adv7611_write(0x98, 0xFF, 0x80); // I2C reset
	adv7611_write(0x98, 0x01, 0x06); // Prim_Mode =110b HDMI-GR	60 Hz
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
	adv7611_write(0x64, 0x77, 0x00); // Disable the Internal EDID
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

/*
 * Initialization settings for HDMI Mode.
 */
static void initialize_hdmi_mode(unsigned char addr, unsigned char num ,unsigned char * value)
{
	adv7611_write_table(CP___MAP_I2C, sizeof(cp_map)/2 , cp_map);
	adv7611_write_table(addr , num , value);
}

static long adv7611_ioctl(struct file * file, unsigned int cmd, unsigned long arg)
{
	unsigned int value;
	unsigned int __user *argp = (unsigned int __user *)arg;
	device_operation operation;

	switch(_IOC_NR(cmd)) {

		/* 
		 * read all register 
		 */
		case _IOC_NR(ADV7611_GET_ALL_REG):	
			break;
		
		/* 
		 * set single register 
		 */
		case _IOC_NR(ADV7611_SET_SINGLE_REG):
			
			if (copy_from_user(&operation, argp, sizeof(operation))) {
				printk(KERN_ALERT "\tERROR: ADV7611_SET_SINGLE_REG\n");
				return -EINVAL;
			}

			adv7611_write(operation.addr, operation.reg, operation.value);
			break;

		/*
		 * read single register 
		 */
		case _IOC_NR(ADV7611_GET_SINGLE_REG):	

			if (copy_from_user(&operation, argp, sizeof(operation))) {
				printk(KERN_ALERT "\tERROR: ADV7611_GET_SINGLE_REG \n");
				return -EINVAL;
			}

			operation.value = adv7611_read(operation.addr, (u8)operation.reg);
			// printk("[[kernel 0x%02x 0x%02x 0x%02x]]\n", operation.addr, operation.reg, operation.value);

			if (copy_to_user(argp, &operation, sizeof(operation)))
				return -1;

			break;

		default:
			return -EINVAL;
	}

	return 0;
}

/*
 *	¼ì²â HDMI cable½ÓÈë
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
 *	¼ì²â HDMI Clock
 */
static int check_hdmi_clock(void)
{
	unsigned char reg;

	reg = adv7611_read(IO___MAP_I2C,0x6A); 
	if (reg & 0x10)
		return 1;
	
	return 0;
}

static int adv7611_device_init(void)
{
    unsigned char ireg, reg;

	ireg = adv7611_read(ADV7611__I2C,0xF4); 
    adv7611_write(ADV7611__I2C,0xF4, 0x80);
    reg = adv7611_read(ADV7611__I2C,0xF4); 
	if( reg != 0x80 ) {
        printk(KERN_ALERT "ERROR : read adv7611 register is 0x%02x\n",reg);
        return -EFAULT;        
    }
	adv7611_write(ADV7611__I2C,0xF4, ireg);

	adv7611_reset();
	write_map_i2c_address( ADV7611__I2C, sizeof ( i2c_address ) / 2, i2c_address );
	//write_map_i2c_address( ADV7611__I2C, sizeof ( io_map ) / 2, io_map );
	//initialize_hdmi_mode ( HDMI_MAP_I2C, sizeof (hdmi_map) / 2, hdmi_map);
	adv7611_1080P();
	adv7611_edid_8_bit();
	reg = check_hdmi_cable();
	printk("\t[check_hdmi_cable = 0x%02x]\n", reg);
	reg = check_hdmi_clock();
	printk("\t[check_hdmi_clock = 0x%02x]\n", reg);

	return 0;
}

static int adv7611_g_chip_ident(struct v4l2_subdev *sd, struct v4l2_dbg_chip_ident *chip)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	return v4l2_chip_ident_i2c_client(client, chip, V4L2_IDENT_ADV7175, 0);
}
/* ----------------------------------------------------------------------- */

static const struct v4l2_subdev_core_ops adv7611_core_ops = {
	//.log_status = adv7611_log_status,
	//.g_ctrl = adv7611_g_ctrl,
	//.s_ctrl = adv7611_s_ctrl,
	//.queryctrl = adv7611_queryctrl,
	//.s_std = adv7611_s_std,
	//.reset = adv7611_reset,
	.g_chip_ident = adv7611_g_chip_ident,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	//.g_register = adv7611_g_register,
	//.s_register = adv7611_s_register,
#endif
};

static const struct v4l2_subdev_video_ops adv7611_video_ops = {
	//.s_routing = adv7611_s_routing,
	//.g_fmt = adv7611_g_fmt,
	//.s_fmt = adv7611_s_fmt,
	//.g_sliced_vbi_cap = tadv7611_g_sliced_vbi_cap,
};

static const struct v4l2_subdev_ops adv7611_ops = {
	.core = &adv7611_core_ops,
	//.tuner = &tvp5150_tuner_ops,
	.video = &adv7611_video_ops,
};


static int adv7611_probe(struct i2c_client *c,
			 const struct i2c_device_id *id)
{
	struct adv7611_state *state;
	

	/* Check if the adapter supports the needed features */
	if (!i2c_check_functionality(c->adapter,
	     I2C_FUNC_SMBUS_READ_BYTE | I2C_FUNC_SMBUS_WRITE_BYTE_DATA))
		return -EIO;

	state = kzalloc(sizeof(struct adv7611_state), GFP_KERNEL);
	if (state == NULL)
		return -ENOMEM;
	adv7611_sd = &state->sd;
	v4l2_i2c_subdev_init(adv7611_sd, c, &adv7611_ops);
	v4l_info(c, "chip found @ 0x%02x (%s)\n",
		 c->addr << 1, c->adapter->name);

	//core->norm = V4L2_STD_ALL;	/* Default is autodetect */
	//core->input = TVP5150_COMPOSITE1;
	//core->enable = 1;
	//core->bright = 128;
	//core->contrast = 128;
	//core->hue = 0;
	//core->sat = 128;
	adv7611_device_init();

	//if (debug > 1)
		//adv7611_log_status(sd);
	return 0;
}
static inline struct adv7611_state *to_state(struct v4l2_subdev *sd)
{
	return container_of(sd, struct adv7611_state, sd);
}

static int adv7611_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);

	v4l2_device_unregister_subdev(sd);
	kfree(to_state(sd));

	return 0;
}
/* ----------------------------------------------------------------------- */

static const struct i2c_device_id adv7611_id[] = {
	{ "adv7611", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, adv7611_id);

static struct v4l2_i2c_driver_data v4l2_i2c_data = {
	.name = "adv7611",
	.command = adv7611_ioctl,
	.probe = adv7611_probe,
	.remove = adv7611_remove,
	.id_table = adv7611_id,
};


