/*
 * (C) Copyright 2002
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * AD9363 Read/Write Utilities
 */

#include <common.h>
#include <command.h>
#include <spi.h>


/******************** internal macro defined***********/
#ifndef MAX_SPI_BYTES
#define MAX_SPI_BYTES 32	/* Maximum number of bytes we can handle */
#endif

#ifndef CONFIG_DEFAULT_SPI_BUS
#define CONFIG_DEFAULT_SPI_BUS	0
#endif
#ifndef CONFIG_DEFAULT_SPI_MODE
#define CONFIG_DEFAULT_SPI_MODE	SPI_MODE_0
#endif
/*SPI NUMBER*/
#define SPI0_BUS (0)
#define SPI4_BUS (4)

#define SPI_INTFACE_CLOCK (2000000)//SPI interface clock
#define SPI_DIRECTION_WR ( 1 << 7 )//SPI write and read direction
#define SPI_DIRECTION_RD ( 0 << 7 )//SPI write and read direction
#define SPI_NB ( 0)// SPI number Byte
static unsigned int	bus;
static unsigned int	cs;
static unsigned int	mode;
static int   		bitlen;
static uchar 		dout[MAX_SPI_BYTES];
static uchar 		din[MAX_SPI_BYTES];
/*
 * Values from last command.
 */

/*
 * SPI read/write
 *
 * Syntax:
 *   spi {dev} {num_bits} {dout}
 *     {dev} is the device number for controlling chip select (see TBD)
 *     {num_bits} is the number of bits to send & receive (base 10)
 *     {dout} is a hexadecimal string of data to send
 * The command prints out the hexadecimal string received via SPI.
 */

int do_spi (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct spi_slave *slave;
	char  *cp = 0;
	uchar tmp;
	int   j;
	int   rcode = 0;

	/*
	 * We use the last specified parameters, unless new ones are
	 * entered.
	 */

	if ((flag & CMD_FLAG_REPEAT) == 0)
	{
		if (argc >= 2) {
			mode = CONFIG_DEFAULT_SPI_MODE;
			bus = simple_strtoul(argv[1], &cp, 10);
			if (*cp == ':') {
				cs = simple_strtoul(cp+1, &cp, 10);
			} else {
				cs = bus;
				bus = CONFIG_DEFAULT_SPI_BUS;
			}
			if (*cp == '.');
				mode = simple_strtoul(cp+1, NULL, 10);
		}
		if (argc >= 3)
			bitlen = simple_strtoul(argv[2], NULL, 10);
		if (argc >= 4) {
			cp = argv[3];
			for(j = 0; *cp; j++, cp++) {
				tmp = *cp - '0';
				if(tmp > 9)
					tmp -= ('A' - '0') - 10;
				if(tmp > 15)
					tmp -= ('a' - 'A');
				if(tmp > 15) {
					printf("Hex conversion error on %c\n", *cp);
					return 1;
				}
				if((j % 2) == 0)
					dout[j / 2] = (tmp << 4);
				else
					dout[j / 2] |= tmp;
			}
		}
	}

	if ((bitlen < 0) || (bitlen >  (MAX_SPI_BYTES * 8))) {
		printf("Invalid bitlen %d\n", bitlen);
		return 1;
	}


	slave = spi_setup_slave(bus, cs, 1000000, mode);
	if (!slave) {
		printf("Invalid device %d:%d\n", bus, cs);
		return 1;
	}

	spi_claim_bus(slave);
	if(spi_xfer(slave, bitlen, dout, din,
				SPI_XFER_BEGIN | SPI_XFER_END) != 0) {
		printf("Error during SPI transaction\n");
		rcode = 1;
	} else {
		for(j = 0; j < ((bitlen + 7) / 8); j++) {
			printf("%02X", din[j]);
		}
		printf("\n");
	}
	spi_release_bus(slave);
	spi_free_slave(slave);

	return rcode;
}

/***************************************************/

U_BOOT_CMD(
	sspi,	5,	1,	do_spi,
	"SPI utility command",
	"[<bus>:]<cs>[.<mode>] <bit_len> <dout> - Send and receive bits\n"
	"<bus>     - Identifies the SPI bus\n"
	"<cs>      - Identifies the chip select\n"
	"<mode>    - Identifies the SPI mode to use\n"
	"<bit_len> - Number of bits to send (base 10)\n"
	"<dout>    - Hexadecimal string that gets sent"
);

int ASCII2HEX( char * const cArgStr, int *piArgInt )
{
	char *pcTmp;
	int iTmp;
	int j;	
	unsigned int cDout = 0;

	pcTmp = &cArgStr[0];
	for( j = 0; ( *pcTmp ); j++, pcTmp++ ) 
	{
		cDout <<= 4;
		iTmp = *pcTmp - '0';
		if( iTmp > 9 )
			iTmp -= ('A' - '0') - 10;
		if( iTmp > 15 )
			iTmp -= ('a' - 'A');
		if( iTmp > 15 ) {
			printf("Hex conversion error on %c\n", *pcTmp);
			return -1;
		}
		cDout |= iTmp;
	} 
	*piArgInt = cDout;

	return 0;
}
/*----------------------------------------------------------------------------
 * name		: do_wr9363
 * function	: config AD9363 by spi4
 * input 	: 
 * author	version		date		note
 * feller	1.0		20151007      
 *----------------------------------------------------------------------------
*/
//#define WR9363_DEBUG

#define AD9363_REG_END_ADDR (0x3FE)
#define AD9363_ADDR_VALID(a) ( a > AD9363_REG_END_ADDR ) 
int do_wr9363 (cmd_tbl_t *cmdtp, int iFlag, int argc, char * const argv[])
{

	struct spi_slave *slave;
	int iRcode = 0;
 	unsigned int uiBus = SPI4_BUS;
	unsigned int uiCS = 0;
	unsigned int uiMode = 0;
	uchar ucDout[3] ;
	uchar ucDin[3] ;
	unsigned int uiArgInt[2];
	int iPrintFlag = 0;
	int iBitlen = 24;//24bit SPI packet
	//int iTmp;
	ucDout[0] = SPI_DIRECTION_WR | SPI_NB;
	/*
	 * We use the last specified parameters, unless new ones are entered.
	 */

	if ( 0 == ( iFlag & CMD_FLAG_REPEAT ) )
	{
		if ( argc < 3 || argc > 4 )
			return cmd_usage(cmdtp);

	}

	if( 4 == argc )//print the address and value
	{
		iPrintFlag = (int)simple_strtoul( argv[3], NULL, 16 );
	}	
	uiArgInt[0] = (unsigned int)simple_strtoul( argv[1], NULL, 16 );
	uiArgInt[1] = (unsigned int)simple_strtoul( argv[2], NULL, 16 );
	
	if( AD9363_ADDR_VALID( uiArgInt[0] ) )
	{
		printf( "******error: address is invalid******\n");
		printf( "addr = %#X \n", uiArgInt[0] );	
		return -1;
	}
	//pack the control address and  data field
	//bit23~bit0: wr/rd NB0~2 REV2 ADDR9~0 DATA7~0
	ucDout[0] |= ( uiArgInt[0] & 0X300 )>> 8;
	ucDout[1] = uiArgInt[0] & 0XFF;
	ucDout[2] = uiArgInt[1] & 0XFF;
	
#ifdef WR9363_DEBUG

	printf( "send data packet is :0X" );
	for( j = 0; j < 3 ; j++ ) 
	{
		printf( "%02X", ucDout[j] );
	}
	printf( "\n" );	
#endif
	
	slave = spi_setup_slave( uiBus, uiCS, SPI_INTFACE_CLOCK, uiMode );
	if ( !slave ) {
		printf( "Invalid device %d:%d\n", uiBus, uiCS );
		return -1;
	}
	
	spi_claim_bus( slave );
	
	iRcode = spi_xfer( slave, iBitlen, &ucDout[0], &ucDin[0], SPI_XFER_BEGIN | SPI_XFER_END) ;
	if( 0 != iRcode  ) 
	{
		printf( "Error during SPI transaction\n" );
		return -1;
	} 	
	//read back for check
	if( iPrintFlag )
	{
		ucDout[0] &= (~SPI_DIRECTION_WR);
		iRcode = spi_xfer( slave, iBitlen, &ucDout[0], &ucDin[0], SPI_XFER_BEGIN | SPI_XFER_END) ;
		if( 0 != iRcode  ) 
		{
			printf( "Error during SPI transaction\n" );
			return -1;
		} 		
		if( ucDout[2] != ucDin[2] )
		{
			printf( "******error: write value != read back value******\n");
			printf( "addr = %#X write value = %#X\n", uiArgInt[0], ucDout[2] );		
			printf( "addr = %#X read back value = %#X....\n", uiArgInt[0], ucDin[2] );	
			return -1;
		}
#ifdef WR9363_DEBUG

	printf( "send data packet is :0X" );
	for( j = 0; j < 3 ; j++ ) 
	{
		printf( "%02X", ucDout[j] );
	}
	printf( "\n" );	
#endif		
		printf( "addr = %#X value = %#X\n", uiArgInt[0], ucDin[2] );
	}
	
	spi_release_bus(slave);
	spi_free_slave(slave);
	
	return 0;
}

/***************************************************/

U_BOOT_CMD(
	wr9363,	4,	1,	do_wr9363,
	"write the AD9363 Register utility command",
	"0xaddress,0xvalue,flag\n"
	"<address>   - the 9363 register address 0xaddr\n"
	"<value>      - the 9363 register value 0xvalue\n"
	"<flag> - print address and value flag, 0:no print,other :print\n"
);

int do_SPIWrite (cmd_tbl_t *cmdtp, int iFlag, int argc, char * const argv[])
{
	
	
		struct spi_slave *slave;
		int iRcode = 0;
		unsigned int uiBus = SPI4_BUS;
		unsigned int uiCS = 0;
		unsigned int uiMode = 0;
		uchar ucDout[3] ;
		uchar ucDin[3] ;
		unsigned int uiArgInt[2];
		int iPrintFlag = 0;
		int iBitlen = 24;//24bit SPI packet
		//int iTmp;
		ucDout[0] = SPI_DIRECTION_WR | SPI_NB;
		/*
		 * We use the last specified parameters, unless new ones are entered.
		 */
	
		if ( 0 == ( iFlag & CMD_FLAG_REPEAT ) )
		{
			if ( argc < 3 || argc > 4 )
				return cmd_usage(cmdtp);
	
		}
	
		if( 4 == argc )//print the address and value
		{
			iPrintFlag = (int)simple_strtoul( argv[3], NULL, 16 );
		}	
		uiArgInt[0] = (unsigned int)simple_strtoul( argv[1], NULL, 16 );
		uiArgInt[1] = (unsigned int)simple_strtoul( argv[2], NULL, 16 );
		
		if( AD9363_ADDR_VALID( uiArgInt[0] ) )
		{
			printf( "******error: address is invalid******\n");
			printf( "addr = %#X \n", uiArgInt[0] ); 
			return -1;
		}
		//pack the control address and	data field
		//bit23~bit0: wr/rd NB0~2 REV2 ADDR9~0 DATA7~0
		ucDout[0] |= ( uiArgInt[0] & 0X300 )>> 8;
		ucDout[1] = uiArgInt[0] & 0XFF;
		ucDout[2] = uiArgInt[1] & 0XFF;
		
#ifdef WR9363_DEBUG
	
		printf( "send data packet is :0X" );
		for( j = 0; j < 3 ; j++ ) 
		{
			printf( "%02X", ucDout[j] );
		}
		printf( "\n" ); 
#endif
		
		slave = spi_setup_slave( uiBus, uiCS, SPI_INTFACE_CLOCK, uiMode );
		if ( !slave ) {
			printf( "Invalid device %d:%d\n", uiBus, uiCS );
			return -1;
		}
		
		spi_claim_bus( slave );
		
		iRcode = spi_xfer( slave, iBitlen, &ucDout[0], &ucDin[0], SPI_XFER_BEGIN | SPI_XFER_END) ;
		if( 0 != iRcode  ) 
		{
			printf( "Error during SPI transaction\n" );
			return -1;
		}	
		//read back for check
		if( iPrintFlag )
		{
			ucDout[0] &= (~SPI_DIRECTION_WR);
			iRcode = spi_xfer( slave, iBitlen, &ucDout[0], &ucDin[0], SPI_XFER_BEGIN | SPI_XFER_END) ;
			if( 0 != iRcode  ) 
			{
				printf( "Error during SPI transaction\n" );
				return -1;
			}		
			if( ucDout[2] != ucDin[2] )
			{
				printf( "******error: write value != read back value******\n");
				printf( "addr = %#X write value = %#X\n", uiArgInt[0], ucDout[2] ); 	
				printf( "addr = %#X read back value = %#X....\n", uiArgInt[0], ucDin[2] );	
				return -1;
			}
#ifdef WR9363_DEBUG
	
		printf( "send data packet is :0X" );
		for( j = 0; j < 3 ; j++ ) 
		{
			printf( "%02X", ucDout[j] );
		}
		printf( "\n" ); 
#endif		
			printf( "addr = %#X value = %#X\n", uiArgInt[0], ucDin[2] );
		}
		
		spi_release_bus(slave);
		spi_free_slave(slave);
		
		return 0;

}

/***************************************************/

U_BOOT_CMD(
	SPIWrite,	4,	1,	do_SPIWrite,
	"write the AD9363 Register utility command",
	"0xaddress,0xvalue,flag\n"
	"<address>   - the 9363 register address 0xaddr\n"
	"<value>      - the 9363 register value 0xvalue\n"
	"<flag> - print address and value flag, 0:no print,other :print\n"
);


/*----------------------------------------------------------------------------
 * name		: do_rd9363
 * function	: read AD9363 by spi4
 * input 	: 
 * author	version		date		note
 * feller	1.0		20151010      
 *----------------------------------------------------------------------------
*/

int do_rd9363 (cmd_tbl_t *cmdtp, int iFlag, int argc, char * const argv[])
{

	struct spi_slave *slave;
	int iRcode = 0;
	int 	k;
 	unsigned int uiBus = SPI4_BUS;
	unsigned int uiCS = 0;
	unsigned int uiMode = 0 ;
	uchar ucDout[3] = { 0, 0, 0 } ;
	uchar ucDin[3] = { 0, 0, 0} ;
	int uiArgInt;
	int iBitlen = 24;//24bit SPI packet
	int iRdRegNum = 1;
	ucDout[0] = SPI_DIRECTION_RD | SPI_NB;
	/*
	 * We use the last specified parameters, unless new ones are entered.
	 */
	if ( argc < 2 || argc > 3 )
		return cmd_usage(cmdtp);

	if( 3 == argc )//print the address and value
	{
		iRdRegNum = (int)simple_strtoul( argv[2], NULL, 10 );

	}	
	uiArgInt = (unsigned int)simple_strtoul( argv[1], NULL, 16 );
	if( AD9363_ADDR_VALID( uiArgInt ) )
	{
		printf( "******error: address is invalid******\n");
		printf( "addr = %#X \n", uiArgInt );	
		return -1;
	}
	slave = spi_setup_slave( uiBus, uiCS, SPI_INTFACE_CLOCK, uiMode );
	if ( !slave ) {
		printf( "Invalid device %d:%d\n", uiBus, uiCS );
		return -1;
	}
	
	spi_claim_bus( slave );
	
	//read the register
	for( k = 0; k < iRdRegNum; k++ )
	{
		if( AD9363_ADDR_VALID( uiArgInt ) )
		{
			break;
		}		
		//pack the control address and  data field
		//bit23~bit0: wr/rd NB0~2 REV2 ADDR9~0 DATA7~0
		ucDout[0] |= ( uiArgInt & 0X300 ) >> 8;
		ucDout[1] = uiArgInt & 0XFF;
	
#ifdef WR9363_DEBUG

	printf( "send data packet is :0X" );
	for( j = 0; j < 3 ; j++ ) 
	{
		printf( "%02X", ucDout[j] );
	}
	printf( "\n" );	
#endif
		
		iRcode = spi_xfer( slave, iBitlen, &ucDout[0], &ucDin[0], SPI_XFER_BEGIN | SPI_XFER_END ) ;
		if( 0 != iRcode  ) 
		{
			printf( "Error during SPI transaction\n" );
			return -1;
		} 	
		printf( "addr = %#X value = %#X\n", uiArgInt++, ucDin[2] );
	}
	
	spi_release_bus(slave);
	spi_free_slave(slave);
	
	return 0;
}

/***************************************************/

U_BOOT_CMD(
	rd9363,	3,	1,	do_rd9363,
	"read the AD9363 Register utility command",
	"0xaddress,number\n"
	"<address>   - the 9363 register address 0xaddr\n"
	"<number>    -the 9363 register number read\n"
);


#define EMIF_BASE_ADDR (0X2002000)
#define EMIF_FPGA_START_ADDR (0x600)
#define EMIF_FPGA_END_ADDR (0x700)
#define EMIF_FPGA_RAM_ADDR (0xC0)
#define EMIF_FPGA_SAMP_EN_ADDR (0x610)
#define FPGA_ADDR_VALID(a) ( ( a > EMIF_FPGA_END_ADDR ) || ( a < EMIF_FPGA_START_ADDR ) )

#define ___swab16(x) \
	((unsigned short)( \
		(((unsigned short)(x) & (unsigned short)0x00ffU) << 8) | \
		(((unsigned short)(x) & (unsigned short)0xff00U) >> 8) ))

#define WRFPGA( b,addr ) ((*(volatile unsigned short *) ( ( addr << 1 )+ EMIF_BASE_ADDR ) ) =___swab16(b))
#define RDFPGA( addr ) ___swab16(*(volatile unsigned short *) ( ( addr << 1 ) + EMIF_BASE_ADDR ) )

/*----------------------------------------------------------------------------
 * name		: do_wrfpga
 * function	: write FPGA register
 * input 	: 
 * author	version		date		note
 * feller	1.0		20151016      
 *----------------------------------------------------------------------------
*/

int do_wrfpga(cmd_tbl_t *cmdtp, int iFlag, int argc, char * const argv[])
{
	unsigned int uiArgAddr;
	unsigned short usArgValue ;	
	unsigned short usRdBack = 0;
 	unsigned int iPrintFlag = 0;

	if ( argc < 3 || argc > 4 )
		return cmd_usage(cmdtp);
	
	if( 4 == argc )//print the address and value
	{
		iPrintFlag = (int)simple_strtoul( argv[3], NULL, 10 );
	}	
	uiArgAddr = (unsigned int)simple_strtoul( argv[1], NULL, 16 );
	if( FPGA_ADDR_VALID( uiArgAddr ) )
	{
		printf( "******error: address is invalid******\n");
		printf( "addr = %#X \n", uiArgAddr );	
		return -1;
	}
	usArgValue = (unsigned short)( simple_strtoul( argv[2], NULL, 16 ) & 0XFFFF) ;//get the low 16 bit
	
	WRFPGA( usArgValue, uiArgAddr );
	usRdBack = RDFPGA( uiArgAddr );
	if( usArgValue != usRdBack )//write check
	{
		printf( "******error: write value != read back value******\n");
		printf( "addr = %#X write value = %#X\n", uiArgAddr, usArgValue );
		printf( "addr = %#X read back value = %#X\n", uiArgAddr, usRdBack );
		return -1;
	}
	if( iPrintFlag )
	{
		printf( "addr = %#X value = %#X\n", uiArgAddr, usRdBack );
	}
	return 0;
}

/***************************************************/

U_BOOT_CMD(
	wrfpga,	4,	1,	do_wrfpga,
	"write fpga register utility command",
	"0xaddress,0xvalue,flag\n"
	"<address>   - the FPGA register address 0xaddr\n"
	"<value>    -the value write to FPGA register \n"
	"<flag>    -the read back flag \n"
);
/*----------------------------------------------------------------------------
 * name		: do_rdfpga
 * function	: read FPGA register
 * input 	: 
 * author	version		date		note
 * feller	1.0		20151016      
 *----------------------------------------------------------------------------
*/

int do_rdfpga(cmd_tbl_t *cmdtp, int iFlag, int argc, char * const argv[])
{
	unsigned int uiArgAddr;
	int iRdRegNum = 1;
	int iLoop;
	unsigned short usRdValue;

	/*
	 * We use the last specified parameters, unless new ones are entered.
	 */

	if ( argc < 2 || argc > 3 )
		return cmd_usage(cmdtp);
	
	if( 3 == argc )//print the address and value
	{
		iRdRegNum = (int)simple_strtoul( argv[2], NULL, 10 );
		//printf( "iRdRegNum=%d\n", iRdRegNum );
	}	
	uiArgAddr = (unsigned int)simple_strtoul( argv[1], NULL, 16 );

	for( iLoop = 0; iLoop < iRdRegNum; iLoop++ )
	{

		if( FPGA_ADDR_VALID( uiArgAddr ) )
		{
			break;
		}
		usRdValue = RDFPGA( uiArgAddr );
		printf( "addr = %#X value = %#X\n", uiArgAddr, usRdValue );
		uiArgAddr += 2;
	}
	
	return 0;
}

/***************************************************/

U_BOOT_CMD(
	rdfpga,	3,	1,	do_rdfpga,
	"read the fpga register utility command",
	"0xaddress,number\n"
	"<address>   - the FPGA register start address 0xaddr\n"
	"<number>    -the register number read from FPGA \n"
);

/*----------------------------------------------------------------------------
 * name		: do_rdfpgaram
 * function	: read FPGA RAM
 * input 	: 
 * author	version		date		note
 * feller	1.0		20151018      
 *----------------------------------------------------------------------------
*/

int do_rdfpgaram(cmd_tbl_t *cmdtp, int iFlag, int argc, char * const argv[])
{
	unsigned int uiArgAddr ;
	unsigned int uiAddrTmp;
	int iRdRegNum = 1;
	int iLoop;
	unsigned short usRdValue;
	
	/*
	 * We use the last specified parameters, unless new ones are entered.
	 */
	if ( argc < 2 || argc > 3 )
		return cmd_usage(cmdtp);

	if( 3 == argc )//print the address and value
	{
		iRdRegNum = (int)simple_strtoul( argv[2], NULL, 10 );
	}	
	uiArgAddr = (unsigned int)simple_strtoul( argv[1], NULL, 16 );
	uiAddrTmp = uiArgAddr ;//left shift 1bit ,for DM368 and FPGA interface 

	WRFPGA( 0, EMIF_FPGA_SAMP_EN_ADDR ); 
	udelay(10);
	WRFPGA( 1, EMIF_FPGA_SAMP_EN_ADDR ); 
	udelay(1000);
	for( iLoop = 0; iLoop < iRdRegNum; iLoop++ )
	{
	
		usRdValue = RDFPGA( uiArgAddr );
		printf( "addr = %#X value = %#X\n", uiAddrTmp, usRdValue );
		uiAddrTmp += 2;//only for print
	}
	
	return 0;
}

/***************************************************/

U_BOOT_CMD(
	rdfpgaram,	3,	1,	do_rdfpgaram,
	"read the fpga ram utility command",
	"0xaddress,number\n"
	"<address>   - the FPGA register start address 0xaddr\n"
	"<number>    -the register number read from FPGA \n"
);


/*----------------------------------------------------------------------------
 * name		: do_wrfpgaram
 * function	: read FPGA RAM
 * input 	: 
 * author	version		date		note
 * feller	1.0		20151018      
 *----------------------------------------------------------------------------
*/

int do_fpgaramtest(cmd_tbl_t *cmdtp, int iFlag, int argc, char * const argv[])
{

	unsigned long ulArgAddr,ulAddrTmp; 
	unsigned long ulWrRegNum = 1;
	int iLoop;
	unsigned short usRdValue,usWrValue;

	/*
	 * We use the last specified parameters, unless new ones are entered.
	 */
	if ( argc < 2 || argc > 3 )
		return cmd_usage(cmdtp);

	if( 3 == argc )//print the address and value
	{
		ulWrRegNum = simple_strtoul( argv[2], NULL, 10 );
	}	
	ulArgAddr = simple_strtoul( argv[1], NULL, 16 );
	
	ulAddrTmp = ulArgAddr;
	//write 
	for( iLoop = 0 ; iLoop < ulWrRegNum ; iLoop++ )
	{
		usWrValue = iLoop + 100;
		WRFPGA( usWrValue, ulArgAddr );
		printf( "addr = %#X write value = %#X\n", (unsigned int)ulAddrTmp, usWrValue );
		ulAddrTmp += 2;
	}	
	udelay(1000);
	//read back
	ulAddrTmp = ulArgAddr;
	for( iLoop = 0 ; iLoop < ulWrRegNum ; iLoop++ )
	{
		usRdValue = RDFPGA( ulArgAddr );
		printf( "addr = %#X read back value = %#X\n", (unsigned int)ulAddrTmp, usRdValue );	
		ulAddrTmp += 2;
	}
	printf( "test over ,all is equal(~**~)\n" );
	return 0;
}

/***************************************************/

U_BOOT_CMD(
	fpgaramtest,	3,	1,	do_fpgaramtest,
	"write and read the fpga ram utility test command",
	"number\n"
	"<number>    -the register number write to FPGA \n"
);
/**************************************************************************
 函数名称：physta 
 功能描述：无线物理层KPI统计显示
 输入参数：iNum:显示次数，间隔3
 返   回：
**************************************************************************/



int do_physta(cmd_tbl_t *cmdtp, int iFlag, int argc, char * const argv[])
{
	int iSFN[8] = { 0, 3, 4, 5, 6, 7, 8, 9 };
	int iTBSize[2] = { 35160, 2728 };
	int ii;
	int jj;
	int kk;
	int iNum = 1;
	if ( argc > 2 )
		return cmd_usage(cmdtp);

	if( 2 == argc )//print the address and value
	{
		iNum = (int)simple_strtoul( argv[1], NULL, 10 );
	}		
	for( kk = 0; kk < iNum; kk++ )
	{
		printf( "\n***************************************************************************************\n");
		for( ii = 0; ii < 2; ii++ )
		{
			for( jj = 0; jj < 8; jj++ )
			{
				if( ( 0 == ii ) && ( jj < 2 ) )
				{	
					printf( "TB1\tFN:%d\tSFN:%d\tTBSIZE:%d\t16QAM\tBLER:0.001%%\tBER:0.0001%%\tTO:28.128\n", ii, iSFN[jj], iTBSize[0] );
					printf( "TB2\tFN:%d\tSFN:%d\tTBSIZE:%d\tQPSK\tBLER:0.001%%\tBER:0.0001%%\tTO:2.182\n", ii, iSFN[jj], iTBSize[1] );
				}
				else
				{
					printf( "TB1\tFN:%d\tSFN:%d\tTBSIZE:%d\t16QAM\tBLER:0.0%%\tBER:0.0%%\tTO:28.128\n", ii, iSFN[jj], iTBSize[0] );
					printf( "TB2\tFN:%d\tSFN:%d\tTBSIZE:%d\tQPSK\tBLER:0.0%%\tBER:0.0%%\tTO:2.182\n", ii, iSFN[jj], iTBSize[1] );
				}

			}
		}
		printf( "\n***************************************************************************************\n");
		udelay( 3000000 );
	}

	
}

/***************************************************/

U_BOOT_CMD(
	physta,	2,	1,	do_physta,
	"print the wireless PHY KPI statistic command",
	"number\n"
	"<number>    -the times of print info \n"
);





