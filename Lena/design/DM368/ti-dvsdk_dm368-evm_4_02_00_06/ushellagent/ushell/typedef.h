#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <elf.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <linux/unistd.h>
#include <mqueue.h>
#include <math.h>
#include <signal.h>



/****************data type defined***********************/
typedef unsigned char   BOOLEAN;
typedef void			VOID;
typedef int 			INT;
typedef char            CHAR;
typedef unsigned char   BYTE;
typedef unsigned char   UCHAR;
typedef unsigned short  WORD16;
typedef signed short    SWORD16;
typedef unsigned long   WORD32;
typedef signed long     SWORD32;
typedef unsigned long   OSS_STATUS; 
typedef unsigned long   ULONG; 
typedef int             SOCKET;
typedef signed long long    SWORD64;
typedef unsigned long long  WORD64;
typedef unsigned long int   WORDPTR;   /* 与指针大小相同的WORD类型, 32位或者64位 */

/****************macro defined***********************/
#ifndef NULL 
#define NULL	(0)
#endif
#define ERROR	(-1)       /* 无效套接字 */
#define TRUE    (1)
#define FALSE   (0)
#define OK      (0)


#define    I80X86    (0)
#define    ARM       (1)
#define    POWERPC   (2)
#define    MIPS      (3)

#define CPU_FAMILY ARM

#endif