/*
 * This source file is FPGA drirver
 *****************************************************************************
 * Copyright(C) 2015, SinoMartin Corp.
 *----------------------------------------------------------------------------
 * filename	: davinci_gpio.c
 * function	: gpio drirver
 * author	version		date		note
 * feller	1.0		20151229	create         
 *----------------------------------------------------------------------------
*/

#include <linux/device.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/crc32.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/smp_lock.h>
#include <linux/backing-dev.h>
#include <linux/compat.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <mach/edma.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>  
#include <linux/delay.h>

#define FPGA_DRIVER_NAME  "fpga"
#define FPGA_MINOR (223)

typedef struct fpga_data{
	unsigned int tb_size;
	unsigned int source_addr;
	unsigned int dst_addr;
	unsigned int byte_size;
	unsigned char device_busy;
}tfpga_data;

typedef struct FPGARegStruct {
    unsigned int uiAddr;//EMIF 32bit address bus
    unsigned int uiValue;//EMIF 16bit data bus,but kernel use 32bit datatype,so we use LSB16
} tFPGAReg;

static tFPGAReg fpga_reg={0};

static tfpga_data fpga_transfer_param;

#define	LENA_AIR  	(0)
#define	LENA_GROUND  	(1)

extern unsigned char device_lena_air_id;


#if 1
#define EDMA_CHA_CNT         64
#define EDMA_BASE_PRAM       0x01C04000u
#define EDMA_PRAM_START      EDMA_BASE_PRAM
#define EDMA_PRAM_SIZE       0x00001000u
#define EDMA_PRAM_SIZE_ERASE        0x00001400u
#define EDMA_PRAM_ERASE      0x00000200u


#define EDMA_ENTRY_SIZE      0x00000020u
#define EDMA_LINK_START      (EDMA_PRAM_START+EDMA_ENTRY_SIZE*EDMA_CHA_CNT)
#define EDMA_LINK_CNT          64


#define _EDMA_OPT_OFFSET			 0
#define _EDMA_SRC_OFFSET			 1
#define _EDMA_ABCNT_OFFSET		 2
#define _EDMA_DST_OFFSET			 3
#define _EDMA_BIDX_OFFSET			 4
#define _EDMA_BCNTLINK_OFFSET	 	 5
#define _EDMA_CIDX_OFFSET			 6
#define _EDMA_CCNT_OFFSET	 		 7



#define EDMACC_ECRH_ADDR              ( 0x01c0100C ) /* reg address: Event Clear H Register            */
#define EDMACC_ESR_ADDR              ( 0x01c01010 ) /* reg address: Event Set Register            */

#define EDMACC_EER_ADDR              ( 0x01c01020 ) /* reg address: Event Enable Register            */
#define EDMACC_EERH_ADDR             ( 0x01c01024 ) /* reg address: Event Enable Register High            */
#define EDMACC_EECR_ADDR              ( 0x01c01028 ) /* reg address: Event Enable Clear  Register           */
#define EDMACC_EECRH_ADDR           ( 0x01c0102C ) /* reg address: Event Enable Clear  Register High           */
#define EDMACC_EESR_ADDR            ( 0x01C01030 ) /* reg address: Event Enable Set  Register          */
#define EDMACC_EESRH_ADDR          ( 0x01C01034 ) /* reg address: Event Enable Set Register High       */
#define EDMACC_SER_ADDR              ( 0x01c01038 ) /* reg address: Secondary Event Register            */
#define EDMACC_SERH_ADDR             ( 0x01c0103C ) /* reg address: Secondary Event Register High            */
#define EDMACC_SECR_ADDR              ( 0x01c01040 ) /* reg address: Secondary Event Clear Register           */
#define EDMACC_SECRH_ADDR           ( 0x01c01044 ) /* reg address: Secondary Event Clear Register High           */

#define EDMACC_IER_ADDR              ( 0x01c01050 ) /* reg address: Interrupt Enable Register            */
#define EDMACC_IERH_ADDR             ( 0x01c01054 ) /* reg address: Interrupt Enable Register High            */
#define EDMACC_IECR_ADDR              ( 0x01c01058 ) /* reg address: Interrupt Clear Enable Register           */
#define EDMACC_IECRH_ADDR           ( 0x01c0105C ) /* reg address: Interrupt Clear Enable Register High           */
#define EDMACC_IESR_ADDR            ( 0x01C01060 ) /* reg address: Interrupt Enable Set  Register          */
#define EDMACC_IESRH_ADDR          ( 0x01C01064 ) /* reg address: Interrupt Enable Set Register High       */
#define EDMACC_IPR_ADDR            ( 0x01C01068 ) /* reg address: Interrupt  Pending  Register          */
#define EDMACC_IPRH_ADDR          ( 0x01C0106C ) /* reg address: Interrupt  Pending Register High       */
#define EDMACC_ICR_ADDR            ( 0x01C01070 ) /* reg address: Interrupt  Clear  Register          */
#define EDMACC_ICRH_ADDR          ( 0x01C01074 ) /* reg address: Interrupt  Clear Register High       */
#define EDMACC_IEVAL_ADDR          ( 0x01C01078 ) /* reg address: Interrupt Evaluate Register       */

extern void __iomem *fpga;

static void __iomem *fpga_buf=NULL;

#define IO_WRITE(addr, val) (*(volatile unsigned short *)(addr) = (val))
#define IO_READ(addr) (*(volatile unsigned short *)(addr))


#define ___swab16(x) \
	((unsigned short)( \
		(((unsigned short)(x) & (unsigned short)0x00ffU) << 8) | \
		(((unsigned short)(x) & (unsigned short)0xff00U) >> 8) ))

#define WRFPGA( addr,b ) ((*(volatile unsigned short *) ( ( addr << 1 )+ fpga ) ) =___swab16(b))
// below is a bug ,every read ,in fact two read operation because of macor 
//#define RDFPGA( addr ) ___swab16(*(volatile unsigned short *) ( ( addr << 1 ) + fpga ) )
 /*----------------------------------------------------------------------------
  * name	 : RDFPGA
  * function	 : RDFPGA function
  * author	 version	 date		 note
  * feller	 1.0	 20160219
  *----------------------------------------------------------------------------
 */
unsigned short RDFPGA( unsigned int uiaddr )
{
	unsigned short usTmp;
	usTmp = __raw_readw( ( uiaddr << 1 ) + fpga ) ;
	return ___swab16(usTmp);
}

typedef struct {
  unsigned int opt;
  unsigned int src;
  unsigned short acnt;
  unsigned short bcnt;
  unsigned int dst;
  short srcbidx;	  
  short dstbidx;  
  unsigned short link;	  
  unsigned short bcntrld;  
  short srccidx;	  
  short dstcidx;  
  unsigned short ccnt;	
  unsigned short resv; 

} EDMA_Config;

static unsigned int frame_header[4]={0};
static unsigned int frame_num=0;

 /*----------------------------------------------------------------------------
  * name	 : EDMA_config
  * function	 : EDMA config 
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */
static inline void EDMA_config(unsigned char channel_num, EDMA_Config *config) 
{
  unsigned int base;

  base = channel_num*EDMA_ENTRY_SIZE+EDMA_PRAM_START;

  __raw_writel(0x00000000, IO_ADDRESS(base+_EDMA_OPT_OFFSET));
  __raw_writel(config->src, IO_ADDRESS(base+_EDMA_SRC_OFFSET));
  __raw_writel((config->acnt)|((config->bcnt)<<16), IO_ADDRESS(base+_EDMA_ABCNT_OFFSET));
  __raw_writel(config->dst, IO_ADDRESS(base+_EDMA_DST_OFFSET));
  __raw_writel((config->srcbidx)|((config->dstbidx)<<16), IO_ADDRESS(base+_EDMA_BIDX_OFFSET));
  __raw_writel((config->link)|((config->bcntrld)<<16), IO_ADDRESS(base+_EDMA_BCNTLINK_OFFSET));
  __raw_writel((config->srccidx)|((config->dstcidx)<<16), IO_ADDRESS(base+_EDMA_CIDX_OFFSET));
  __raw_writel(config->ccnt, IO_ADDRESS(base+_EDMA_CCNT_OFFSET));
  __raw_writel(config->opt, IO_ADDRESS(base+_EDMA_OPT_OFFSET));

	return;

}
#define BUFFER_LEN_ERR (0X80000000)
#define IOMAP_ERR (0X40000000)
#define TIMEOUT_ERR (0X20000000)
#define BUFFER_OVERFLOW_ERR (0X10000000)
#define RECE_LEN_ERROR (0x08000000)
#define CRC_ERROR (0x04000000)
#define RECE_FRAMELEN_ERROR (0x02000000)
#define RECE_TBLEN_ERROR (0x01000000)
#define FRAME_NUM_ERROR (0x00800000)
 /*----------------------------------------------------------------------------
  * name	 : emif_send
  * function	 : emif_send 
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */
static int siSendFrameCount = 0;
int emif_send(struct fpga_data *transfer)
{
	ulong	addr,count,write_count;
	volatile unsigned short *frame_header_p=NULL;
	unsigned int tb_payplod,tb_datalen;
	unsigned int len;
	unsigned int iFrameNum=0;
	//calc the TBX data payload, we keep some bytes as frame header

	tb_payplod = transfer->tb_size-0x10;
	len = transfer->byte_size;

	fpga_buf = ioremap(transfer->source_addr,len);
	if(fpga_buf==NULL) 
	{
		printk("ioremap error!\n");
		return IOMAP_ERR;
	}
	addr =(unsigned int)fpga_buf;

	while(len>0)
	{
		//clear the event from FPGA ,period 10ms
		__raw_writel(1<<3, IO_ADDRESS(0x01c67034));

		if( len == transfer->byte_size )
		{
			frame_header[0]=0xAAAAAAAA;//first wireless frame for pic frame
		}
		else 
		{
			//frame_header[0]=0x55555555;
			frame_header[0] = iFrameNum++;
		}
		//printk( "frame_header[0]=%#x\n", frame_header[0] );
		frame_header[1] = transfer->byte_size;
		frame_header[2] = siSendFrameCount++;
		//calc the wireless frame payload 
		if(len >= tb_payplod)
		{	
			tb_datalen = tb_payplod;
		}
		else 
		{	
			tb_datalen = len;
		}
		frame_header[3]=(tb_datalen<<16)|tb_datalen;

		
		//wait for 10ms event from FPGA
		while((__raw_readl(IO_ADDRESS(0x01c67034))&(1<<3)) == 0)
		{
			//msleep(5);
		}
		//send frame header
		frame_header_p = (volatile unsigned short *)frame_header;
		for(write_count=0;write_count<8;write_count++,frame_header_p++)
		{
				__raw_writew(*frame_header_p, ( 4 + fpga ));
		}
		count = (tb_datalen+1)/2;
		//send the data to FPGA
		frame_header_p = (volatile unsigned short *)addr;
		for(write_count=0;write_count<count;write_count++,frame_header_p++)
		{
				__raw_writew(*frame_header_p, ( 4 + fpga ));
		}		
		//update the addresd and rest length 
		len -= tb_datalen;
		addr += tb_datalen;
	
	}
	iounmap(fpga_buf);

	return 0;

}

 /*----------------------------------------------------------------------------
  * name	 : emif_recv
  * function	 : emif_recv 
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */

int emif_recv(struct fpga_data *transfer)
{
	ulong	addr,count,read_count;
	volatile unsigned short *frame_header_p = NULL;
	volatile int wait_10ms = 1000;
	unsigned int tb_datalen = 0;
	unsigned int tb_payplod;
	unsigned int frame_length = 0;
	unsigned int frame_rece_count = 0;
	unsigned int uifound_frame_header = 0;
	int frame_tb_count = 0;
	unsigned short usCRCRecord = 0;
	unsigned int uiCRCReg ;
	unsigned int uiAirFrameNum = 0;
	unsigned int uiReturn = 0;

	//calc the TBX data payload, we keep some bytes as frame header
	tb_payplod = transfer->tb_size-0x10;
	
	//set sync lost time
	if( ( transfer->source_addr < 10000 ) && ( transfer->source_addr > 1 )) 
	{
		wait_10ms = transfer->source_addr;
	}

	if( transfer->byte_size < tb_payplod )	
	{
		printk( "input buffer size smaller than tb_payplod size\n" );
		return BUFFER_LEN_ERR;			
	}	
	
	fpga_buf = ioremap( transfer->dst_addr, transfer->byte_size );
	if( NULL ==fpga_buf ) 
	{
		printk("ioremap error!\n");
		return IOMAP_ERR;
	}

	uiCRCReg = (unsigned int)( ( 0x688<<1)  + fpga );

	uifound_frame_header = 0;
	addr =(unsigned int)fpga_buf;
	//search the frame header
	do{
		//clear the event from FPGA ,period 10ms
		__raw_writel(1<<3, IO_ADDRESS(0x01c67034));
		//wait for 10ms event from FPGA
		while( (__raw_readl(IO_ADDRESS(0x01c67034))&(1<<3)) == 0 )
		{
			//msleep(4);
		}
		frame_tb_count++;
		usCRCRecord = __raw_readw( uiCRCReg );
		//check the TB CRC 

		if( ( usCRCRecord & 0xFF00 ) != 0xFF00 ) 
		{	
			wait_10ms -= frame_tb_count;
			frame_tb_count = 0;
			if(wait_10ms <= 0) 
			{	
				uiReturn = TIMEOUT_ERR;
				break;							
			}
			printk("c");
			//wait_10ms--;
			continue;
		}

		//receive frame header
		frame_header_p = (volatile unsigned short *)frame_header;
		for( read_count=0; read_count<8; read_count++,frame_header_p++)
		{
			*frame_header_p = __raw_readw((0x20 + fpga ));
		}

		//check the first wireless header field of pic   
		if( frame_header[0] != 0xAAAAAAAA )
		{
			//printk("2");
			wait_10ms--;
			continue;			
		}		
		else
		{	
			frame_length = frame_header[1];//get the total length of pic 
			if( frame_length > transfer->byte_size ) 
			{ 
				printk( ">" );
				uiReturn = BUFFER_OVERFLOW_ERR;//buffer overflow
				break;
			}
			frame_rece_count = 0;							
			uifound_frame_header = 1;
			usCRCRecord = 0XFF00;//clear for receive frame
			break;
		}
	}while( wait_10ms > 0 );//check the frame header
			
	while( 1 == uifound_frame_header  )
	{
		//decode the frame header
		tb_datalen = frame_header[3]&0xFFFF;

		if( tb_datalen > tb_payplod )
		{
			printk("t=%d", tb_datalen );
			uiReturn = RECE_TBLEN_ERROR;
			break;
		}	


		//calc the number ,and receive wireless frame data
		frame_header_p = (volatile unsigned short *)addr;
		count = (tb_datalen+1)/2;
			
		for(read_count=0;read_count<count;read_count++,frame_header_p++)
		{
			*frame_header_p = __raw_readw((0x20 + fpga ));
		}

		frame_rece_count += tb_datalen;//have received data length
					
		if( frame_rece_count == frame_length )//must be equal 
		{
			//printk("i");
			break;
		}
		else if( frame_rece_count > frame_length )
		{
			printk("f");
			uiReturn = RECE_LEN_ERROR;
			break;
		}		
		addr += tb_datalen;
		//clear the event from FPGA ,period 10ms
		__raw_writel(1<<3, IO_ADDRESS(0x01c67034));
		//wait for 10ms event from FPGA
		while( (__raw_readl(IO_ADDRESS(0x01c67034))&(1<<3)) == 0 )
		{
			//msleep(4);//release CPU for load
		}	
		frame_tb_count++;
		usCRCRecord &= __raw_readw( uiCRCReg );
						
		//receive frame header
		
		frame_header_p = (volatile unsigned short *)frame_header;
		for(read_count=0;read_count<8;read_count++,frame_header_p++)
		{
			*frame_header_p = __raw_readw((0x20 + fpga ));
		}
		if( frame_header[1] != frame_length ) //for frame lost, dynamic pic
		{
			printk("f");
			uiReturn = RECE_FRAMELEN_ERROR;
			break;
		}
		if( uiAirFrameNum != frame_header[0] ) //for static pic 
		{
			printk( "n" );
			uiReturn = FRAME_NUM_ERROR;
			break;
		}
		uiAirFrameNum++;
		
	}
	iounmap(fpga_buf);
		
	if( 0XFF00 != ( usCRCRecord & 0XFF00 ) )
	{
		printk( "C" );
		uiReturn |=  CRC_ERROR;
	}
	transfer->byte_size = frame_rece_count;
	return uiReturn;
	
}
#endif

 /*----------------------------------------------------------------------------
  * name	 : fpga_open 
  * function	 : fpga open interface
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */

static int fpga_open(struct inode *inode, struct file *file)
{

	__raw_writel(0x1, IO_ADDRESS(0x01c67008));
	__raw_writel(__raw_readl(IO_ADDRESS(0x01c67010))|0x8, IO_ADDRESS(0x01c67010));
	__raw_writel(0x8, IO_ADDRESS(0x01c67024));
	__raw_writel(0x8, IO_ADDRESS(0x01c67030));

	frame_header[0]=0;
	frame_header[1]=0;
	frame_header[2]=0;
	frame_header[3]=0;
	frame_num = 0;

	return 0;
} /* fpga_open */

 /*----------------------------------------------------------------------------
  * name	 : fpga_close 
  * function	 : fpga close interface
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */

static int fpga_close(struct inode *inode, struct file *file)
{
	return 0;
} /* fpga_close */


 /*----------------------------------------------------------------------------
  * name	 : fpga_read 
  * function	 : fpga read interface
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */
ssize_t fpga_read(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{
	unsigned long ulRet;
	ulRet = copy_from_user( (void*)&fpga_reg, buf, count);
	fpga_reg.uiValue = (unsigned int)RDFPGA( fpga_reg.uiAddr );
	ulRet = copy_to_user( (void*)buf, &fpga_reg, count); 
	
	return count;
}
 /*----------------------------------------------------------------------------
  * name	 : fpga_write 
  * function	 : fpga write interface
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */

ssize_t fpga_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{
	unsigned long ulRet;
    ulRet = copy_from_user(&fpga_reg, buf, 8);
	WRFPGA( fpga_reg.uiAddr, (unsigned short)( fpga_reg.uiValue & 0XFFFF ) );
    return 8;
}


#define DMA_SEND	(1U)
#define DMA_RECV	(0U)
#define FPGA_CMD_SIZE	(2U)

 /*----------------------------------------------------------------------------
  * name	 : fpga_ioctl 
  * function	 : fpga io control interface
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */
static int fpga_ioctl(struct inode *inode, struct file *file,
		     u_int cmd, u_long arg)
{
	int  ret = 0;
	int  iret = 0;
	unsigned int __user *argp = (unsigned int __user *)arg;

	if (cmd>FPGA_CMD_SIZE) return -EFAULT;

	switch (cmd) {
	
		case DMA_RECV:
			frame_header[0]=0;
			frame_header[1]=0;
			frame_header[2]=0;
			frame_header[3]=0;
			frame_num = 0;
	
			iret = copy_from_user( (void*)&fpga_transfer_param, argp, sizeof(fpga_transfer_param));		
			ret = emif_recv(&fpga_transfer_param);
			iret = copy_to_user( (void*)argp, &fpga_transfer_param, sizeof(fpga_transfer_param) );
			break;	

		case DMA_SEND:
			frame_header[0]=0;
			frame_header[1]=0;
			frame_header[2]=0;
			frame_header[3]=0;
			frame_num = 0;
			iret = copy_from_user( (void*)&fpga_transfer_param, argp, sizeof(fpga_transfer_param));
			ret = emif_send(&fpga_transfer_param);
			iret = copy_to_user(argp, (void*)&fpga_transfer_param, sizeof(fpga_transfer_param));
			break;		
	
		default:
			ret = -ENOTTY;
	}

	return ret;
} /* memory_ioctl */


static const struct file_operations fpga_fops = {
	.owner		= THIS_MODULE,
	.read       = (void*)fpga_read,
	.write      = fpga_write,
	.ioctl		= fpga_ioctl,
	.open		= fpga_open,
	.release	= fpga_close,

};

static struct miscdevice fpga_dev = {
 	//.minor = MISC_DYNAMIC_MINOR, 
	.minor = FPGA_MINOR,
	.name = FPGA_DRIVER_NAME,  
	.fops = &fpga_fops,  
};
 /*----------------------------------------------------------------------------
  * name	 : fpga_init 
  * function	 : fpga_init
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */
static int __init fpga_init(void)
{
	int ret;

	ret = misc_register(&fpga_dev);
	printk(FPGA_DRIVER_NAME"\t initialized %s!\n", (0==ret)?"successed":"failed");	
	return ret;

}
 /*----------------------------------------------------------------------------
  * name	 : fpga_cleanup_module 
  * function	 : fpga_cleanup_module
  * author	 version	 date		 note
  * feller	 1.0	 20151229
  *----------------------------------------------------------------------------
 */
static void __exit fpga_cleanup_module(void)
{
	misc_deregister(&fpga_dev);
	return;
}

module_init(fpga_init);
module_exit(fpga_cleanup_module);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("zhushukui");
MODULE_DESCRIPTION(" Direct character-device access to FPGA devices");
