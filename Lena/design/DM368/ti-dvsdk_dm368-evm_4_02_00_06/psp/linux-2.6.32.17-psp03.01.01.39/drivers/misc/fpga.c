/*
 * Character-device access to raw MTD devices.
 *
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
#if 0
#define WRFPGA( addr,b ) ((*(volatile unsigned short *) ( ( addr << 1 )+ fpga ) ) = (b))
#define RDFPGA( addr ) (*(volatile unsigned short *) ( ( addr << 1 ) + fpga ) )
#endif

#define ___swab16(x) \
	((unsigned short)( \
		(((unsigned short)(x) & (unsigned short)0x00ffU) << 8) | \
		(((unsigned short)(x) & (unsigned short)0xff00U) >> 8) ))

#define WRFPGA( addr,b ) ((*(volatile unsigned short *) ( ( addr << 1 )+ fpga ) ) =___swab16(b))
#define RDFPGA( addr ) ___swab16(*(volatile unsigned short *) ( ( addr << 1 ) + fpga ) )


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

static inline void EDMA_config(unsigned char channel_num, EDMA_Config *config) 
{
  unsigned int base;

  base = channel_num*EDMA_ENTRY_SIZE+EDMA_PRAM_START;
  
 // base[_EDMA_OPT_OFFSET] = 0x00000000;
 // base[_EDMA_SRC_OFFSET] = config->src;
 // base[_EDMA_ABCNT_OFFSET] = (config->acnt)|((config->bcnt)<<16);
 // base[_EDMA_DST_OFFSET] = config->dst;
 // base[_EDMA_BIDX_OFFSET] = (config->srcbidx)|((config->dstbidx)<<16);
 /// base[_EDMA_BCNTLINK_OFFSET] = (config->link)|((config->bcntrld)<<16);
 // base[_EDMA_CIDX_OFFSET] = (config->srccidx)|((config->dstcidx)<<16);
 // base[_EDMA_CCNT_OFFSET] = config->ccnt;
 // base[_EDMA_OPT_OFFSET] = config->opt;

  __raw_writel(0x00000000, IO_ADDRESS(base+_EDMA_OPT_OFFSET));
  __raw_writel(config->src, IO_ADDRESS(base+_EDMA_SRC_OFFSET));
  __raw_writel((config->acnt)|((config->bcnt)<<16), IO_ADDRESS(base+_EDMA_ABCNT_OFFSET));
  __raw_writel(config->dst, IO_ADDRESS(base+_EDMA_DST_OFFSET));
  __raw_writel((config->srcbidx)|((config->dstbidx)<<16), IO_ADDRESS(base+_EDMA_BIDX_OFFSET));
  __raw_writel((config->link)|((config->bcntrld)<<16), IO_ADDRESS(base+_EDMA_BCNTLINK_OFFSET));
  __raw_writel((config->srccidx)|((config->dstcidx)<<16), IO_ADDRESS(base+_EDMA_CIDX_OFFSET));
  __raw_writel(config->ccnt, IO_ADDRESS(base+_EDMA_CCNT_OFFSET));
  __raw_writel(config->opt, IO_ADDRESS(base+_EDMA_OPT_OFFSET));

//printk("edma config %x\n",__raw_readl(IO_ADDRESS(base+_EDMA_OPT_OFFSET)));

}

//unsigned char buf[0x42600];
int emif_send(struct fpga_data *transfer)
{
	EDMA_Config image_transfer;
	ulong	addr,count,write_count;
	volatile unsigned short *frame_header_p=NULL;
	//const unsigned short *buf;
	unsigned int len;
	unsigned int __user *argp;


	printk("..");
	count = 0x4260-0x10;
	len = transfer->byte_size;

	fpga_buf = ioremap(transfer->source_addr,len);
	if(fpga_buf==NULL) 
	{
		printk("ioremap error!\n");
	}
	addr =(unsigned int)fpga_buf;
	//argp = (unsigned int __user *)(transfer->source_addr);
	//copy_from_user( (void*)(&(buf[0])), argp, len);
	

	while(len>=count)
	{
		__raw_writel(1<<3, IO_ADDRESS(0x01c67034));

		while((__raw_readl(IO_ADDRESS(0x01c67034))&(1<<3)) ==0);

		frame_header_p = (volatile unsigned short *)addr;
		for(write_count=0;write_count<0x2128;write_count++,frame_header_p++)
				__raw_writew(*frame_header_p, ( 4 + fpga ));

		frame_header[1]=0;
		frame_header[2]=0x42504250;
		frame_header[3]	= crc32_le(~0, (unsigned char const *)frame_header, 0xC);
		
		frame_header_p = (volatile unsigned short *)frame_header;
		for(write_count=0;write_count<8;write_count++)
				__raw_writew(*frame_header_p, ( 4 + fpga ));

		frame_header[0]++;
		
		len -= count;
		addr += count;
	
	}

	if(len>0)
	{
		__raw_writel(1<<3, IO_ADDRESS(0x01c67034));

		while((__raw_readl(IO_ADDRESS(0x01c67034))&(1<<3)) ==0);

		frame_header_p = (volatile unsigned short *)addr;
		for(write_count=0;write_count<(len/2);write_count++,frame_header_p++)
				__raw_writew(*frame_header_p, ( 4 + fpga ));

		frame_header[1]=0;
		frame_header[2]=(len<<16)|len;
		frame_header[3]	= crc32_le(~0, (unsigned char const *)frame_header, 0xC);
		
		frame_header_p = (volatile unsigned short *)frame_header;
		for(write_count=0;write_count<8;write_count++)
				__raw_writew(*frame_header_p, ( 4 + fpga ));		
		
		frame_header[0]++;
		addr += len;
		len =0;
	
	}

	iounmap(fpga_buf);

	return 0;

}
#define FRAME_HEADER_TIMEOUT (6000)
int emif_recv(struct fpga_data *transfer)
{
		EDMA_Config image_transfer;
		ulong	addr,count,write_count;
		volatile unsigned short *frame_header_p=NULL;
		//const unsigned short *buf;
		unsigned int len;
		unsigned int __user *argp;
		unsigned int iSearchFrameHeader = 0 ; // search frame header time out counter
		unsigned int iRet = 0;
		count = 0x4260-0x10;
		//addr =(unsigned int)(&(buf[0]));
		//addr = __va(transfer->dst_addr);
		len = transfer->byte_size;

		fpga_buf = ioremap(transfer->dst_addr,len);
		if(fpga_buf==NULL) printk("ioremap error!\n");

		addr =(unsigned int)fpga_buf;
		//argp = (unsigned int __user *)(transfer->dst_addr);

		while(len>=count)
		{
			printk(".");

			__raw_writel(1<<3, IO_ADDRESS(0x01c67034));

			while((__raw_readl(IO_ADDRESS(0x01c67034))&(1<<3)) ==0);

			frame_header_p = (volatile unsigned short *)addr;
			for(write_count=0;write_count<0x2128;write_count++,frame_header_p++)
					*frame_header_p = __raw_readw((0x20 + fpga ));

			frame_header_p = (volatile unsigned short *)frame_header;
			for(write_count=0;write_count<8;write_count++,frame_header_p++)
					*frame_header_p = __raw_readw((0x20 + fpga ));

			//frame_header[1]=0;
			//frame_header[2]=0x42504250;
			///frame_header[3]	= crc32_le(~0, (unsigned char const *)frame_header, 0xC);
			
			if((frame_header[0]!=0)&&(0==frame_num)) 
			{
				iSearchFrameHeader++;
				if( iSearchFrameHeader >  FRAME_HEADER_TIMEOUT )// for timeout return
				{
					printk("<");
					iounmap(fpga_buf);
					iRet = -1;
					return iRet;
				}					
				else 
				{
					continue;
				}
			}
			len -= count;
			addr += count;
			frame_num++;


		}
	
		if(len>0)
		{
			printk("*\n");
			__raw_writel(1<<3, IO_ADDRESS(0x01c67034));

			while((__raw_readl(IO_ADDRESS(0x01c67034))&(1<<3)) ==0);

			frame_header_p = (volatile unsigned short *)addr;
			for(write_count=0;write_count<(len/2);write_count++,frame_header_p++)
					*frame_header_p = __raw_readw((0x20 + fpga ));

			frame_header_p = (volatile unsigned short *)frame_header;
			for(write_count=0;write_count<8;write_count++,frame_header_p++)
					*frame_header_p = __raw_readw((0x20 + fpga ));

			//frame_header[1]=0;
			//frame_header[2]=0x42504250;
			///frame_header[3]	= crc32_le(~0, (unsigned char const *)frame_header, 0xC);
			
			//if((frame_header[0]!=0)&&(0==frame_num)) continue;

			addr += len;
			len -= len;

			frame_num++;

		}

		//copy_to_user(argp, (void*)(&(buf[0])), transfer->byte_size); 
		
		iounmap(fpga_buf);
		return iRet;
	
}


#if 0
int dma_cpy (struct fpga_data *transfer)
{
	ulong	addr, dest, count;
	int	size;
	EDMA_Config image_transfer;

	size = transfer->tb_size;

	addr = transfer->source_addr;

	dest =  transfer->dst_addr;

	count = transfer->byte_size;

	//hEdma=_EDMA_MK_HANDLE(10*_EDMA_ENTRY_SIZE,EDMA_RSV6,_EDMA_TYPE_C);
	
	{
		image_transfer.opt=0x0010A00C;
		image_transfer.src=addr;
		image_transfer.acnt=size;
		image_transfer.bcnt=count/size;
		image_transfer.dst=dest;
		image_transfer.srcbidx=size;
		image_transfer.dstbidx=size;
		image_transfer.link=0xffff;
		image_transfer.bcntrld=0x0;
		image_transfer.srccidx=0x0;
		image_transfer.dstcidx=0x0;
		image_transfer.ccnt=0x01;

	}			
		
	EDMA_config(10, &image_transfer);

	//EDMA_enableChannel(10);                        //EESR
	IO_WRITE(EDMACC_EESR_ADDR, 1<<10);
	//EDMA_setChannel(10);                              //ESR
	IO_WRITE(EDMACC_ESR_ADDR, 1<<10);

	while((IO_READ(EDMACC_IPR_ADDR)&(1<<10)) ==0);
	IO_WRITE(EDMACC_ICR_ADDR, 1<<10);	

	return 0;

}

#endif

#endif
//static unsigned short fpga_rd_buffer[4][2048];
//static unsigned short fpga_tx_buffer[4][2048];


static int fpga_open(struct inode *inode, struct file *file)
{
	//*(unsigned int *)0x01c67008 = 0x1;
	//*(unsigned int *)0x01c67010 |= 0x8;
	//*(unsigned int *)0x01c67024 = 0x8;
	//*(unsigned int *)0x01c67030 = 0x8;

	__raw_writel(0x1, IO_ADDRESS(0x01c67008));
	__raw_writel(__raw_readl(IO_ADDRESS(0x01c67010))|0x8, IO_ADDRESS(0x01c67010));
	__raw_writel(0x8, IO_ADDRESS(0x01c67024));
	__raw_writel(0x8, IO_ADDRESS(0x01c67030));

	frame_header[0]=0;
	frame_header[1]=0;
	frame_header[2]=0;
	frame_header[3]=0;
	frame_num = 0;
	printk("fpga open OK!\n");

	return 0;
} /* fpga_open */

/*====================================================================*/

static int fpga_close(struct inode *inode, struct file *file)
{
	return 0;
} /* fpga_close */



//static unsigned int rw_buf=0;
ssize_t fpga_read(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{
	unsigned long ulRet;
	ulRet = copy_from_user( (void*)&fpga_reg, buf, count);
	fpga_reg.uiValue = (unsigned int)RDFPGA( fpga_reg.uiAddr );
	ulRet = copy_to_user( (void*)buf, &fpga_reg, count); 
	
	return count;
}


ssize_t fpga_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{
	unsigned long ulRet;
    ulRet = copy_from_user(&fpga_reg, buf, 8);
	WRFPGA( fpga_reg.uiAddr, (unsigned short)( fpga_reg.uiValue & 0XFFFF ) );
    return 8;
}


//#define GET_INFO	0U
//#define SET_INFO	1U
#define DMA_SEND	1U
#define DMA_RECV	0U

#define FPGA_CMD_SIZE	2U



static int fpga_ioctl(struct inode *inode, struct file *file,
		     u_int cmd, u_long arg)
{
	int  ret = 0;
	unsigned int __user *argp = (unsigned int __user *)arg;
	printk("fpga ioctl in!\n");
	//DEBUG(MTD_DEBUG_LEVEL0, "MTD_ioctl\n");
	printk("fpga ioctl cmd=%d!\n",cmd);
	if (cmd>FPGA_CMD_SIZE) return -EFAULT;

	switch (cmd) {
		//case GET_INFO:
			//printk("fpga ioctl OK!  GET_INFO!\n");
			//ret = copy_to_user(argp, &fpga_transfer_param, sizeof(fpga_transfer_param));
			//break;
		case DMA_RECV:
			frame_header[0]=0;
			frame_header[1]=0;
			frame_header[2]=0;
			frame_header[3]=0;
			frame_num = 0;
			printk("fpga ioctl OK!  DMA_RECV!\n");
			copy_from_user( (void*)&fpga_transfer_param, argp, sizeof(fpga_transfer_param));		
			printk("fpga_transfer_param.dst_addr =%x\n",fpga_transfer_param.dst_addr );
			printk("fpga_transfer_param.byte_size =%x\n",fpga_transfer_param.byte_size );
			ret = emif_recv(&fpga_transfer_param);

			break;	

		case DMA_SEND:
			//ret = copy_from_user( (void*)&fpga_transfer_param, argp, sizeof(fpga_transfer_param));
			frame_header[0]=0;
			frame_header[1]=0;
			frame_header[2]=0;
			frame_header[3]=0;
			frame_num = 0;
			//dma_cpy (&fpga_transfer_param);
			printk("fpga ioctl OK! SET_INFO!\n");
		//	break;
		//case DMA_SEND:
			printk("fpga ioctl OK! DMA_SEND IN!\n");
			copy_from_user( (void*)&fpga_transfer_param, argp, sizeof(fpga_transfer_param));
			printk("fpga_transfer_param.source_addr =%x\n",fpga_transfer_param.source_addr );
			printk("fpga_transfer_param.byte_size =%x\n",fpga_transfer_param.byte_size );
			ret = emif_send(&fpga_transfer_param);
			printk("fpga ioctl OK! DMA_SEND! OUT\n");
			break;		
	
		default:
			ret = -ENOTTY;
	}

	return ret;
} /* memory_ioctl */


static const struct file_operations fpga_fops = {
	.owner		= THIS_MODULE,
	.read           = fpga_read,
	.write           = fpga_write,
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

static int __init fpga_init(void)
{
	int ret;

	ret = misc_register(&fpga_dev);
	printk(FPGA_DRIVER_NAME"\t initialized %s!\n", (0==ret)?"successed":"failed");	
	return ret;

}

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

