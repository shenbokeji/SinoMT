/*********************************************************************
* 版权所有 (C)2008, 中兴通讯股份有限公司。
*
* 文件名称： ushell.c
* 文件标识：
* 内容摘要： Linux下用于调试进程的ushell工具
* 其它说明： 无
* 当前版本： V1.0
* 作    者： 陈林海
* 完成日期：2010-03-16
*
* 修改记录1：
*    修改日期：
*    版 本 号：V1.0
*    修 改 人：
*    修改内容：创建
**********************************************************************/
/***********************************************************
 *                      头文件                             *
***********************************************************/
//#include "readline.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/select.h>
#include <linux/unistd.h>
#include <mqueue.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>


/***********************************************************
 *                 文件内部使用的宏                        *
***********************************************************/

#define IPC_MSG_LENTH 512
#define IPC_MSG_NUM 50
#define IPC_FIFO_LENTH 4096

#define FIFO_NAME  "/ushell_fifo"
#define MSGQ_NAME "/ushell_msq"

#define FIFO_LENGTH  32
#define MSGQ_LENGTH  32

#define MAX_PROCESS_NUM 10

#define   SIGTASKWAKUP               (SIGRTMAX-1)       /* 唤醒fifo thread select*/

#define  PASSWORD "zte"

#define  TRYLOGINNUM 3


/***********************************************************
 *               文件内部使用的数据类型                    *
***********************************************************/
#ifndef bool
#define bool  int
#endif
	
typedef enum 
{
	FALSE = 0,
	TRUE = 1
} boolean_t;

typedef struct T_ProcessInfo
{
  bool IsUseed;/*是否使用，TRUE，使用，FALSE 没有使用*/
  bool IsPrintState;/*打印信息是否接管*/
  bool IsCommandState;/*命令发送状态*/
  int pid;/*前台进程ID*/
  int fifoFd;/*fifo fd*/
  mqd_t mqd;/*消息队列ID*/
  char ucFifoName[FIFO_LENGTH];/*管道名*/
  char ucMsgqName[MSGQ_LENGTH];/*消息队列名 */
	
}ProcessInfo;

/***********************************************************
 *                     全局变量                            *
***********************************************************/
ProcessInfo g_tProcessInfo[MAX_PROCESS_NUM];/*调试进程信息存放数组*/
char msg_type[IPC_MSG_LENTH];/* 缓冲接收键盘输入 */
char msg_buf[IPC_MSG_LENTH]; /* 缓冲输出至进程队列 */

int Logined = 0;/*是否登陆成功*/
int HasTryLoginNum = 0;/*尝试登录次数，如果尝试登录次数达到TRYLOGINNUM，则推出ushell*/

fd_set readfds;/*fifo fd 集合*/

pthread_t fifo_thread;/*fifo守护线程ID*/

int g_ushell_debug = 0;/*ushell 工具调试开关，0表示关闭，1表示打开，默认关闭*/

/***********************************************************
 *                     本地变量                            *
***********************************************************/

/***********************************************************
 *                     全局函数                            *
***********************************************************/
extern char *readline (char * a );
extern char *getpass( const char * prompt );

/***********************************************************
 *                     局部函数                            *
***********************************************************/

/**********************************************************************
* 函数名称：Exc_handler
* 功能描述：SIGINT（Ctrl+C）信号处理函数
* 访问的表：无
* 修改的表：无
* 输入参数：无
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                创建
************************************************************************/
void ushell_exit(int signo, siginfo_t *info, void *context)
{    
    int i = 0;
    ProcessInfo * pProcessInfo = g_tProcessInfo;
	
    memset(msg_buf,0,IPC_MSG_LENTH); 
    memcpy(msg_buf, "not_print_fifo_and_debug", strlen("not_print_fifo_and_debug"));
	
	//初始化进程信息表
	for(i =0;i<MAX_PROCESS_NUM;i++)
	{
		if((pProcessInfo+i)->IsUseed == TRUE)
		{
		     mq_send(((pProcessInfo+i)->mqd), msg_buf, IPC_MSG_LENTH, 0);
			 mq_close((pProcessInfo+i)->mqd);
		    (pProcessInfo+i)->mqd = -1;
		    (pProcessInfo+i)->IsUseed = FALSE;
		    (pProcessInfo+i)->IsPrintState = FALSE;
		    (pProcessInfo+i)->IsCommandState = FALSE;
			if((pProcessInfo+i)->fifoFd != -1)
			{
				close((pProcessInfo+i)->fifoFd);
			}
		    (pProcessInfo+i)->fifoFd = -1;
			(pProcessInfo+i)->pid = -1;
		    
		}
	}
    printf("quit debug and exit ushell!\n");
    exit(0);
}
/**********************************************************************
* 函数名称：wake_up_select
* 功能描述：发送信号到fifo守护线程，
                             把fifo守护线程从select中唤醒，当执行fd_clr
                             之后需要调用这个函数，否则有可能导致
                             fifo守护线程阻塞在fd_clr 中的那个select中
* 访问的表：无
* 修改的表：无
* 输入参数：无
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                创建
***********************************************************************/
void ushell_wake_up_select(void)
{
    int ret;
    ret = pthread_kill(fifo_thread,SIGTASKWAKUP);
    if (0 == ret)
    {
        return ;
    }
    else
    {
        printf("pthread_kill failed to send signal! errno: %d %s \n",errno,strerror(errno));
        return ;
    }

}
/**********************************************************************
* 函数名称：wake_up_select_fun
* 功能描述：wake_up信号的处理函数，本函数不执行
                             具体的操作     
* 访问的表：无
* 修改的表：无
* 输入参数：无
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                创建
***********************************************************************/

void ushell_wake_up_select_fun(void)
{
   return;
}


/**********************************************************************
* 函数名称：pthread_fifo
* 功能描述：FIFO守护线程
* 访问的表：无
* 修改的表：无
* 输入参数：无
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                创建
************************************************************************/
void *ushell_pthread_fifo(void *arg)
{
	int fdmax =0, read_num;
	char r_buf[IPC_FIFO_LENTH];
    int i =0;
	ProcessInfo * pProcessInfo = g_tProcessInfo;
	bool havefindfifo = FALSE;
	
   
    while (1)
	{       

	    FD_ZERO(&readfds);
		havefindfifo = FALSE;
		pProcessInfo = g_tProcessInfo;
		for(i=0;i<MAX_PROCESS_NUM;i++)
		{
			if(((pProcessInfo+i)->IsUseed == TRUE)
               &&((pProcessInfo+i)->fifoFd != -1)
               &&((pProcessInfo+i)->IsPrintState == TRUE)
			  )
			{
			    FD_SET((pProcessInfo+i)->fifoFd, &readfds);
				if(((pProcessInfo+i)->fifoFd) >fdmax)
				{
					fdmax = (pProcessInfo+i)->fifoFd;
				}
				havefindfifo = TRUE;
			}
		}
        if(havefindfifo == FALSE)
    	{
	    	sleep(1);
			continue;
    	}
	    if(-1 == select(fdmax+1, &readfds, NULL, NULL, NULL))   
	    {    
	        if(EINTR == errno)
        	{
	        	continue;
        	}
			else
			{
		        //printf("select error!\n");
		        break;
			}
	    }
		pProcessInfo = g_tProcessInfo;
		for(i=0;i<MAX_PROCESS_NUM;i++)
		{
			if(((pProcessInfo+i)->IsUseed == TRUE)
               &&((pProcessInfo+i)->fifoFd != -1)
               &&((pProcessInfo+i)->IsPrintState == TRUE)
			  )
			{
				if (FD_ISSET(((pProcessInfo+i)->fifoFd), &readfds))
		        {
		            memset(r_buf,0,IPC_FIFO_LENTH);
					//printf("ushell_pthread_fifo:before read pid :%d \n",((pProcessInfo+i)->pid));
		            read_num=read(((pProcessInfo+i)->fifoFd),r_buf, IPC_FIFO_LENTH);
		            if (read_num > 0)
	            	{
		                printf("\n[%d]\n%s", ((pProcessInfo+i)->pid),r_buf);
	            	}
		            else
		            {    
		                //printf("pid:%d read fifo error!\n",(pProcessInfo+i)->pid);
						FD_CLR(((pProcessInfo+i)->fifoFd),&readfds);
						if((pProcessInfo+i)->fifoFd != -1)
						{
							close((pProcessInfo+i)->fifoFd);
						}
						(pProcessInfo+i)->fifoFd = -1;
						(pProcessInfo+i)->IsPrintState == FALSE;
						if((pProcessInfo+i)->IsCommandState == FALSE)
						{
							if((pProcessInfo+i)->mqd != -1)
							{
								mq_close((pProcessInfo+i)->mqd);
							}
						    (pProcessInfo+i)->IsUseed = FALSE;
						    (pProcessInfo+i)->pid = -1;
						    (pProcessInfo+i)->mqd = -1;
						}
		            }  
		        }
			}
		}
         

	            
	}

}

/**********************************************************************
* 函数名称：ushell_help
* 功能描述：ushell_help使用帮助函数
* 访问的表：无
* 修改的表：无
* 输入参数：无
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                创建
***********************************************************************/

void ushell_help(void)
{
    printf("                                ushell tool menu:                              \n");	 
    printf("------------------------------------------------------------------------------ \n");
	printf(" 'ps'       or     'PS'          list process run on the board \n");
    printf(" 'pr xxx'   or     'PR xxx'      take over  xxx process printf info \n");																									
    printf(" 'npr xxx'  or     'NPR xxx' not take over  xxx process printf info \n");																									
    printf(" 'db xxx'   or     'DB xxx'      debug xxx process printf info \n");																									
    printf(" 'ndb xxx'  or     'NDB xxx' not debug xxx process printf info \n");	
    printf(" 'pad xxx'  or     'PAD xxx'     debug and take over  xxx process printf info \n");																									
    printf(" 'npad xxx' or     'NPAD xxx'not debug and take over  xxx process printf info \n");	
    printf(" 'pall'     or     'PALL'        display current debug and take over info \n");	
	printf(" 'Q'        or     'q'           cancel all process  debug and printf info \n");
    printf(" 'exit'     or     'EXIT'        cancel ushell \n");
	printf("                    xxx  is process id you want to debug or take over printf info \n");
    printf("------------------------------------------------------------------------------ \n");   	

}
/**********************************************************************
* 函数名称：ushell_take_over_print
* 功能描述：pr xxx 或者PR xxx :
                             接管前台xxx进程打印
* 访问的表：无
* 修改的表：无
* 输入参数：无
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                创建
***********************************************************************/
void ushell_take_over_print(void)
{      
    /*提取出需要调试的进程ID*/
	int i = 0;
	int j = 0;
	int pid = -1;
	bool havefindpid = FALSE;
	char pidchar[10];
	ProcessInfo * pProcessInfo = g_tProcessInfo;
	
	for(i=3,j=0;msg_type[i]!='\0'&&i<IPC_MSG_LENTH&&j<9;i++)
	{
	   
	   if(msg_type[i]==' ')
	   {
          continue;
	   }
	   pidchar[j++]=msg_type[i];
	}
    pidchar[j]='\0';
	//printf("pid:%s\n",pidchar);
	pid = atoi(pidchar);
	//输入的PID是否已经处于接管打印状态?
	pProcessInfo = g_tProcessInfo;
	for(i=0;i<MAX_PROCESS_NUM;i++)
	{
	   if(((pProcessInfo+i)->pid   == pid)
	   	  &&((pProcessInfo+i)->IsUseed == TRUE))
	   	{
	   	    if((pProcessInfo+i)->IsPrintState == TRUE)
   	    	{
		   	    printf(" Proccess :%d is already take over printf info \n",pid);
			   	return;
   	    	}
			else
			{
				(pProcessInfo+i)->IsPrintState = TRUE;
				havefindpid = TRUE;
				break;
			}
	   	}
	}
	//从g_tProcessInfo查找一个空闲的pid信息存储空间
	if(havefindpid == FALSE)
	{
		pProcessInfo = g_tProcessInfo;
		for(i=0;i<MAX_PROCESS_NUM;i++)
		{
		   if(((pProcessInfo+i)->IsUseed == FALSE))
		   	{
		   	    break;
		   	}
		}
	}
	if(i>=MAX_PROCESS_NUM)	
	{
	   printf("can not find free proccess from array g_tProcessInfo\n");
	   return;
	}
	
	pProcessInfo = g_tProcessInfo;
	pProcessInfo = pProcessInfo+i;
	pProcessInfo->IsUseed = TRUE;
	pProcessInfo->IsPrintState = TRUE;
	pProcessInfo->pid = pid;
	
	strcpy((char *)pProcessInfo->ucFifoName,FIFO_NAME);
	strcat((char *)pProcessInfo->ucFifoName,pidchar);

	strcpy((char *)pProcessInfo->ucMsgqName,MSGQ_NAME);
	strcat((char *)pProcessInfo->ucMsgqName,pidchar);
	
	{
		//unlink((char *)g_aucMsgqName);
		if(pProcessInfo->mqd == -1)
		{
		    pProcessInfo->mqd = mq_open((char *)pProcessInfo->ucMsgqName, O_WRONLY);
		    if ((mqd_t)-1 == pProcessInfo->mqd)
		    {    
		        printf("open msgQ:%s error!\n",pProcessInfo->ucMsgqName);

			    pProcessInfo->IsUseed = FALSE;
			    pProcessInfo->IsPrintState = FALSE;
			    pProcessInfo->IsCommandState = FALSE;
			    pProcessInfo->pid = -1;
				if(pProcessInfo->fifoFd != -1)
				{
				    FD_CLR((pProcessInfo->fifoFd),&readfds);
					close(pProcessInfo->fifoFd);
				}
			    pProcessInfo->fifoFd = -1;
			    pProcessInfo->mqd = -1;
				ushell_wake_up_select();
				return;
				
		    }
		}
		pProcessInfo->fifoFd = open((char *)pProcessInfo->ucFifoName, O_RDONLY|O_NONBLOCK,0);
	    if (-1 == pProcessInfo->fifoFd)
	    {    
	        printf("open fifo:%s error!\n",pProcessInfo->ucFifoName);
			pProcessInfo->IsPrintState = FALSE;
			if(FALSE == pProcessInfo->IsCommandState)
			{
				if(pProcessInfo->mqd != -1)
				{
					mq_close(pProcessInfo->mqd);
				}
			    pProcessInfo->IsUseed = FALSE;
			    pProcessInfo->IsPrintState = FALSE;
			    pProcessInfo->pid = -1;
			    pProcessInfo->fifoFd = -1;
			    pProcessInfo->mqd = -1;
				ushell_wake_up_select();
			}
			return;
			
	    }
		
	}
    {
		memset(msg_buf,0,IPC_MSG_LENTH); 
		memcpy(msg_buf, "print_fifo", strlen("print_fifo"));   
		mq_send(pProcessInfo->mqd, msg_buf, IPC_MSG_LENTH, 0);
		ushell_wake_up_select();
    }
}
/**********************************************************************
* 函数名称：ushell_not_take_over_print
* 功能描述：npr xxx 或者NPR xxx :
                             退出前台xxx进程接管打印
* 访问的表：无
* 修改的表：无
* 输入参数：无
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                创建
***********************************************************************/
void ushell_not_take_over_print(void)
{      
    /*提取出需要调试的进程ID*/
	int i = 0;
	int j = 0;
	int pid = -1;
	bool havefindpid = FALSE;
	char pidchar[10];
	ProcessInfo * pProcessInfo = g_tProcessInfo;
	
	for(i=4,j=0;msg_type[i]!='\0'&&i<IPC_MSG_LENTH&&j<9;i++)
	{
	   
	   if(msg_type[i]==' ')
	   {
          continue;
	   }
	   pidchar[j++]=msg_type[i];
	}
    pidchar[j]='\0';
	//printf("pid:%s\n",pidchar);
	pid = atoi(pidchar);
	//输入的PID是否已经处于接管打印状态?
	pProcessInfo = g_tProcessInfo;
	for(i=0;i<MAX_PROCESS_NUM;i++)
	{
	   if(((pProcessInfo+i)->pid   == pid)
	   	  &&((pProcessInfo+i)->IsUseed == TRUE))
	   	{
	   	    if((pProcessInfo+i)->IsPrintState != TRUE)
   	    	{
		   	    printf(" Proccess :%d is not already display \n",pid);
			   	return;
   	    	}
			else
			{
			    //发送退出打印接管状态
			    {
					memset(msg_buf,0,IPC_MSG_LENTH); 
					memcpy(msg_buf, "not_print_fifo", strlen("not_print_fifo"));   
					mq_send(((pProcessInfo+i)->mqd), msg_buf, IPC_MSG_LENTH, 0);
	            }
				(pProcessInfo+i)->IsPrintState = FALSE;
				if(((pProcessInfo+i)->fifoFd )!= -1)
				{
				    FD_CLR(((pProcessInfo+i)->fifoFd),&readfds);
					close((pProcessInfo+i)->fifoFd );
				}
				
			}
			//不在调试状态，释放占用的进程信息数组
			if((pProcessInfo+i)->IsCommandState != TRUE)
			{
			    if(((pProcessInfo+i)->mqd) != -1)
				{
					mq_close((pProcessInfo+i)->mqd);
				}
				(pProcessInfo+i)->IsUseed = FALSE;
			    (pProcessInfo+i)->IsPrintState = FALSE;
			    (pProcessInfo+i)->IsCommandState = FALSE;
			    (pProcessInfo+i)->pid = -1;
			    (pProcessInfo+i)->fifoFd = -1;
			    (pProcessInfo+i)->mqd = -1;
			}
			ushell_wake_up_select();
	   	}
	}
                    
}
/**********************************************************************
* 函数名称：ushell_debug_process
* 功能描述：pad  xxx或者PAD xxx:
                             调试和接管xxx前台进程
* 访问的表：无
* 修改的表：无
* 输入参数：无
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                创建
***********************************************************************/
void ushell_print_and_debug_process(void)
{      
    /*提取出需要调试的进程ID*/
	int i = 0;
	int j = 0;
	int pid = -1;
	bool havefindpid = FALSE;
	char pidchar[10];
	ProcessInfo * pProcessInfo = g_tProcessInfo;
	
	for(i=4,j=0;msg_type[i]!='\0'&&i<IPC_MSG_LENTH&&j<9;i++)
	{
	   
	   if(msg_type[i]==' ')
	   {
          continue;
	   }
	   pidchar[j++]=msg_type[i];
	}
    pidchar[j]='\0';
	//printf("pid:%s\n",pidchar);
	pid = atoi(pidchar);
	//输入的PID是否已经处于调试或者打印接管状态?
	for(i=0;i<MAX_PROCESS_NUM;i++)
	{
	   if(((pProcessInfo+i)->pid   == pid)
	   	  &&((pProcessInfo+i)->IsUseed == TRUE))
	   	{
	   	    printf(" Proccess :%d is already enter take over print or/and debug mod\n",pid);
			printf(" IsCommandState:%d,IsPrintState:%d \n",(pProcessInfo+i)->IsCommandState,(pProcessInfo+i)->IsPrintState);
		   	return;
	   	}
	}
	//从g_tProcessInfo查找一个空闲的pid信息存储空间
	for(i=0;i<MAX_PROCESS_NUM;i++)
	{
	   if(((pProcessInfo+i)->IsUseed == FALSE))
	   	{
	   	    break;
	   	}
	}

	if(i>=MAX_PROCESS_NUM)	
	{
	   printf("can not find free proccess from array g_tProcessInfo\n");
	   return;
	}
	pProcessInfo = pProcessInfo+i;
	pProcessInfo->IsUseed = TRUE;
	pProcessInfo->pid = pid;
	strcpy((char *)pProcessInfo->ucFifoName,FIFO_NAME);
	strcat((char *)pProcessInfo->ucFifoName,pidchar);

	strcpy((char *)pProcessInfo->ucMsgqName,MSGQ_NAME);
	strcat((char *)pProcessInfo->ucMsgqName,pidchar);
	
	{
		//unlink((char *)g_aucMsgqName);
	    pProcessInfo->mqd = mq_open((char *)pProcessInfo->ucMsgqName, O_WRONLY);
	    if ((mqd_t)-1 == pProcessInfo->mqd)
	    {    
	        printf("open msgQ:%s error!\n",pProcessInfo->ucMsgqName);
		    pProcessInfo->IsUseed = FALSE;
		    pProcessInfo->IsPrintState = FALSE;
		    pProcessInfo->IsCommandState = FALSE;
			if(pProcessInfo->fifoFd != -1)
			{
			    FD_CLR((pProcessInfo->fifoFd),&readfds);
				close(pProcessInfo->fifoFd);
			}
		    pProcessInfo->pid = -1;
		    pProcessInfo->fifoFd = -1;
		    pProcessInfo->mqd = -1;
			ushell_wake_up_select();
			return;
	    }
		pProcessInfo->fifoFd = open((char *)pProcessInfo->ucFifoName, O_RDONLY|O_NONBLOCK,0);
		if (-1 == pProcessInfo->fifoFd)
		{
	        printf("open fifo:%s error!\n",pProcessInfo->ucFifoName);
			if(-1 == pProcessInfo->mqd)
			{
				mq_close(pProcessInfo->mqd);
			}
		    pProcessInfo->IsUseed = FALSE;
		    pProcessInfo->IsPrintState = FALSE;
		    pProcessInfo->IsCommandState = FALSE;
		    pProcessInfo->pid = -1;
		    pProcessInfo->fifoFd = -1;
		    pProcessInfo->mqd = -1;
			ushell_wake_up_select();
			return;
		}
		
	}
	
    {
		pProcessInfo->IsCommandState = TRUE;
		pProcessInfo->IsPrintState = TRUE;
		memset(msg_buf,0,IPC_MSG_LENTH); 
		memcpy(msg_buf, "print_fifo_and_debug", strlen("print_fifo_and_debug"));   
		mq_send(pProcessInfo->mqd, msg_buf, IPC_MSG_LENTH, 0);
		ushell_wake_up_select();
    }
    
                    
}
/**********************************************************************
* 函数名称：ushell_exit_print_and_debug_process
* 功能描述：npad  xxx或者NPAD xxx:
                             退出调试和接管打印xxx前台进程
* 访问的表：无
* 修改的表：无
* 输入参数：无
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                创建
***********************************************************************/
void ushell_exit_print_and_debug_process(void)
{      
    /*提取出需要调试的进程ID*/
	int i = 0;
	int j = 0;
	int pid = -1;
	bool havefindpid = FALSE;
	char pidchar[10];
	ProcessInfo * pProcessInfo = g_tProcessInfo;
	
	for(i=5,j=0;msg_type[i]!='\0'&&i<IPC_MSG_LENTH&&j<9;i++)
	{
	   
	   if(msg_type[i]==' ')
	   {
          continue;
	   }
	   pidchar[j++]=msg_type[i];
	}
    pidchar[j]='\0';
	//printf("pid:%s\n",pidchar);
	pid = atoi(pidchar);
	//输入的PID是否已经处于接管打印状态?
	pProcessInfo = g_tProcessInfo;
	for(i=0;i<MAX_PROCESS_NUM;i++)
	{
	   if(((pProcessInfo+i)->pid   == pid)
	   	  &&((pProcessInfo+i)->IsUseed == TRUE))
	   	{
	   	    if(((pProcessInfo+i)->IsCommandState != TRUE)
				||((pProcessInfo+i)->IsPrintState != TRUE))
   	    	{
		   	    printf(" Proccess :%d is not in print and debug mod\n",pid);
			   	return;
   	    	}
			memset(msg_buf,0,IPC_MSG_LENTH); 
			memcpy(msg_buf, "not_print_fifo_and_debug", strlen("not_print_fifo_and_debug"));   
			mq_send(((pProcessInfo+i)->mqd), msg_buf, IPC_MSG_LENTH,0);
			(pProcessInfo+i)->IsCommandState = FALSE;
			(pProcessInfo+i)->IsPrintState = FALSE;

			(pProcessInfo+i)->IsUseed = FALSE;
		    (pProcessInfo+i)->pid = -1;
			if((pProcessInfo+i)->fifoFd != -1)
			{
			    FD_CLR(((pProcessInfo+i)->fifoFd),&readfds);
				close((pProcessInfo+i)->fifoFd);
			}
			if((pProcessInfo+i)->mqd != -1)
			{
				mq_close((pProcessInfo+i)->mqd);
			}
		    (pProcessInfo+i)->fifoFd = -1;
		    (pProcessInfo+i)->mqd = -1;
			ushell_wake_up_select();

			
	   	}
	}
                    
}

/**********************************************************************
* 函数名称：ushell_debug_process
* 功能描述：db  xxx或者DB xxx:
                             调试xxx前台进程
* 访问的表：无
* 修改的表：无
* 输入参数：无
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                创建
***********************************************************************/
void ushell_debug_process(void)
{      
    /*提取出需要调试的进程ID*/
	int i = 0;
	int j = 0;
	int pid = -1;
	bool havefindpid = FALSE;
	char pidchar[10];
	ProcessInfo * pProcessInfo = g_tProcessInfo;
	
	for(i=3,j=0;msg_type[i]!='\0'&&i<IPC_MSG_LENTH&&j<9;i++)
	{
	   
	   if(msg_type[i]==' ')
	   {
          continue;
	   }
	   pidchar[j++]=msg_type[i];
	}
    pidchar[j]='\0';
	//printf("pid:%s\n",pidchar);
	pid = atoi(pidchar);
	//输入的PID是否已经处于调试状态?
	pProcessInfo = g_tProcessInfo;
	for(i=0;i<MAX_PROCESS_NUM;i++)
	{
	   if(((pProcessInfo+i)->pid   == pid)
	   	  &&((pProcessInfo+i)->IsUseed == TRUE))
	   	{
	   	    if((pProcessInfo+i)->IsCommandState == TRUE)
   	    	{
		   	    printf(" Proccess :%d is already enter debug mod\n",pid);
			   	return;
   	    	}
			else
			{
				(pProcessInfo+i)->IsCommandState = TRUE;
				havefindpid = TRUE;
				break;
			}
	   	}
	}
	//从g_tProcessInfo查找一个空闲的pid信息存储空间
	if(havefindpid == FALSE)
	{
		pProcessInfo = g_tProcessInfo;
		for(i=0;i<MAX_PROCESS_NUM;i++)
		{
		   if(((pProcessInfo+i)->IsUseed == FALSE))
		   	{
		   	    break;
		   	}
		}
	}
	if(i>=MAX_PROCESS_NUM)	
	{
	   printf("can not find free proccess from array g_tProcessInfo\n");
	   return;
	}
	
	pProcessInfo = g_tProcessInfo;
	pProcessInfo = pProcessInfo+i;
	pProcessInfo->IsUseed = TRUE;
	
	strcpy((char *)pProcessInfo->ucMsgqName,MSGQ_NAME);
	strcat((char *)pProcessInfo->ucMsgqName,pidchar);

	if(-1 == pProcessInfo->mqd)
	{
	    pProcessInfo->mqd = mq_open((char *)pProcessInfo->ucMsgqName, O_WRONLY);
	    if ((mqd_t)-1 == pProcessInfo->mqd)
	    {    
	        printf("open msgQ:%s error!\n",pProcessInfo->ucMsgqName);
		    pProcessInfo->IsUseed = FALSE;
		    pProcessInfo->IsPrintState = FALSE;
		    pProcessInfo->IsCommandState = FALSE;
			if(pProcessInfo->fifoFd != -1)
			{
			    FD_CLR((pProcessInfo->fifoFd),&readfds);
				close(pProcessInfo->fifoFd);
			}
		    pProcessInfo->pid = -1;
		    pProcessInfo->fifoFd = -1;
		    pProcessInfo->mqd = -1;
			ushell_wake_up_select();
			return;
	    }
	}
	pProcessInfo->IsCommandState = TRUE;
	pProcessInfo->pid = pid;
	memset(msg_buf,0,IPC_MSG_LENTH); 
	memcpy(msg_buf, "debug", strlen("debug"));   
	mq_send(pProcessInfo->mqd, msg_buf, IPC_MSG_LENTH, 0);
                    
}
/**********************************************************************
* 函数名称：ushell_exit_debug_process
* 功能描述：ndb  xxx或者NDB xxx:
                             退出调试xxx前台进程
* 访问的表：无
* 修改的表：无
* 输入参数：无
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                创建
***********************************************************************/
void ushell_exit_debug_process(void)
{      
    /*提取出需要调试的进程ID*/
	int i = 0;
	int j = 0;
	int pid = -1;
	bool havefindpid = FALSE;
	char pidchar[10];
	ProcessInfo * pProcessInfo = g_tProcessInfo;
	
	for(i=4,j=0;msg_type[i]!='\0'&&i<IPC_MSG_LENTH&&j<9;i++)
	{
	   
	   if(msg_type[i]==' ')
	   {
          continue;
	   }
	   pidchar[j++]=msg_type[i];
	}
    pidchar[j]='\0';
	//printf("pid:%s\n",pidchar);
	pid = atoi(pidchar);
	//输入的PID是否已经处于接管打印状态?
	pProcessInfo = g_tProcessInfo;
	for(i=0;i<MAX_PROCESS_NUM;i++)
	{
	   if(((pProcessInfo+i)->pid   == pid)
	   	  &&((pProcessInfo+i)->IsUseed == TRUE))
	   	{
	   	    if((pProcessInfo+i)->IsCommandState != TRUE)
   	    	{
		   	    printf(" Proccess :%d is not in debug mod\n",pid);
			   	return;
   	    	}
			memset(msg_buf,0,IPC_MSG_LENTH); 
			memcpy(msg_buf, "not_debug", strlen("not_debug"));   
			mq_send(((pProcessInfo+i)->mqd), msg_buf, IPC_MSG_LENTH,0);
			(pProcessInfo+i)->IsCommandState = FALSE;
			
			//当前pid不处于打印接管状态
			if((pProcessInfo+i)->IsPrintState == FALSE)
			{
				(pProcessInfo+i)->IsUseed = FALSE;
			    (pProcessInfo+i)->pid = -1;
				if((pProcessInfo+i)->fifoFd != -1)
				{
				    FD_CLR(((pProcessInfo+i)->fifoFd),&readfds);
					close((pProcessInfo+i)->fifoFd);
				}
			    (pProcessInfo+i)->fifoFd = -1;
				if((pProcessInfo+i)->mqd != -1)
				{
					mq_close((pProcessInfo+i)->mqd);
				}
			    (pProcessInfo+i)->mqd = -1;
			}
			
	   	}
	}
                    
}

/**********************************************************************
* 函数名称：ushell_exit_proccess_debug
* 功能描述：q 或者Q :
                             退出当前所有调试和接管打印的前台进程   
* 访问的表：无
* 修改的表：无
* 输入参数：无
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                创建
***********************************************************************/
void ushell_exit_all_proccess_debug(void)
{    
	int i = 0;
    ProcessInfo * pProcessInfo;
	
	memset(msg_buf,0,IPC_MSG_LENTH); 
	memcpy(msg_buf, "not_print_fifo_and_debug", strlen("not_print_fifo_and_debug"));
	pProcessInfo = g_tProcessInfo;
	for(i=0;i<MAX_PROCESS_NUM;i++)
	{
	    if((pProcessInfo+i)->IsUseed == TRUE)
	   	{
	        mq_send((pProcessInfo+i)->mqd, msg_buf, IPC_MSG_LENTH, 0);

		    (pProcessInfo+i)->IsUseed = FALSE;
	        (pProcessInfo+i)->IsPrintState = FALSE;
	        (pProcessInfo+i)->IsCommandState = FALSE;
	        (pProcessInfo+i)->pid = -1;
		    if((pProcessInfo+i)->fifoFd != -1)
			{
			    FD_CLR(((pProcessInfo+i)->fifoFd),&readfds);
				close((pProcessInfo+i)->fifoFd);
			}
			(pProcessInfo+i)->fifoFd = -1;
		    if((pProcessInfo+i)->mqd != -1)
			{
				mq_close((pProcessInfo+i)->mqd);
			}
	        (pProcessInfo+i)->mqd = -1;
			 ushell_wake_up_select();
	   	}
	    
	}
}


/**********************************************************************
* 函数名称：ushell_printf_all_Process_info
* 功能描述：pall 或者PALL:
                             显示当前调试和接管打印的前台进程信息
* 访问的表：无
* 修改的表：无
* 输入参数：无
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                创建
***********************************************************************/
void ushell_printf_all_Process_info(void)
{    
	int i = 0;
    ProcessInfo * pProcessInfo;
	
	pProcessInfo = g_tProcessInfo;
	printf("index		IsUseed		IsPrintState		IsCommandState		pid		fifoFd		mqd\n");
	for(i=0;i<MAX_PROCESS_NUM;i++)
	{
	    if((pProcessInfo+i)->IsUseed == TRUE)
	   	{
	        printf(" %d		%d		%d				%d		%d		%d		%d \n",
				    i,
				   (pProcessInfo+i)->IsUseed,
				   (pProcessInfo+i)->IsPrintState,
				   (pProcessInfo+i)->IsCommandState,
				   (pProcessInfo+i)->pid,
				   (pProcessInfo+i)->fifoFd,
				   (pProcessInfo+i)->mqd);
	   	}
	    
	}
			

}
/**********************************************************************
* 函数名称：ushell_send_command
* 功能描述：发送命令到前台调试进程
* 访问的表：无
* 修改的表：无
* 输入参数：无
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                创建
***********************************************************************/
void ushell_send_command(void)
{
	int i = 0;
    ProcessInfo * pProcessInfo;
	bool havefindpid = FALSE;
	
	pProcessInfo = g_tProcessInfo;
	for(i=0;i<MAX_PROCESS_NUM;i++)
	{
	   if(((pProcessInfo+i)->IsUseed == TRUE)
		   &&((pProcessInfo+i)->IsCommandState == TRUE)
		 )
		{
		   mq_send((pProcessInfo+i)->mqd, msg_type, IPC_MSG_LENTH, 0);
		   havefindpid = TRUE;
		}
		
	}
	if(FALSE == havefindpid)
	{
		printf("Input error!:%s\n",msg_type);
	}
}
/**********************************************************************
* 函数名称：ushell_print_msg_from_input
* 功能描述：ushell 调试函数，打印从标准输入中读取
                             到的消息
* 访问的表：无
* 修改的表：无
* 输入参数：无
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                创建
***********************************************************************/
void ushell_print_msg_from_input(char * msg)
{
    int i = 0;
    int msgLength = 0;
    if(NULL == msg )
    {
        printf("msg is null \n");
        return;
    }
    msgLength = strlen(msg);
    printf("%d \n",msgLength);
    for(i=0;i<msgLength;i++)
    {  
        if((0== i%16)&&(i>0))
        {
            printf("\n");  
        }
        printf("%d ",*(msg+i));
    }
}


/**********************************************************************
* 函数名称：main
* 功能描述：ushell的入口函数
* 访问的表：无
* 修改的表：无
* 输入参数：无
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期      版本号  修改人      修改内容
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                创建
***********************************************************************/
int main(int argc, char *argv[])
{
	char pidchar[10];
    struct sigaction act;
    int i =0;
	ProcessInfo * pProcessInfo = g_tProcessInfo;

    /*Added by qinfei for V3.11.001, 2011.01.06*/
    /*Add Begin*/
    
	struct sched_param sp, sp1,sp2;
    sched_getparam(0, &sp1);
	printf("sched_getscheduler = %d\t prio = %d\n", sched_getscheduler(0), sp1.sched_priority);
	sp.sched_priority = 97;
	sched_setscheduler(0, SCHED_FIFO, &sp);/*预留 99的优先级给中断服务任务，98的优先级给taskLock*/
    sched_getparam(0, &sp2);
	printf("sched_getscheduler = %d\t prio = %d\n", sched_getscheduler(0), sp2.sched_priority);
	printf("sched mode is SCHED_FIFO!\n");
	
	/*Add End*/


	
	//初始化进程信息表
	for(i =0;i<MAX_PROCESS_NUM;i++)
	{
	   (pProcessInfo+i)->IsUseed = FALSE;
	   (pProcessInfo+i)->IsPrintState = FALSE;
	   (pProcessInfo+i)->IsCommandState = FALSE;
	   (pProcessInfo+i)->pid = -1;
	   (pProcessInfo+i)->fifoFd = -1;
	   (pProcessInfo+i)->mqd = -1;
	}
	//ctrl + C
    sigemptyset(&act.sa_mask);
    act.sa_flags=SA_SIGINFO;
    act.sa_sigaction=ushell_exit;	
    sigaction(SIGINT,&act,NULL);
    sigaction(SIGSEGV,&act,NULL);
    sigaction(SIGILL,&act,NULL);
    sigaction(SIGFPE,&act,NULL);
    sigaction(SIGBUS,&act,NULL);
    sigaction(SIGABRT,&act,NULL);
    sigaction(SIGTERM,&act,NULL);
    
	//wake_up_select
    sigemptyset(&act.sa_mask);
    act.sa_flags=SA_SIGINFO;
    act.sa_sigaction=ushell_wake_up_select_fun;	
    sigaction(SIGTASKWAKUP,&act,NULL);
	
	if(0 != pthread_create(&fifo_thread, NULL, ushell_pthread_fifo,NULL))
	{	 
		printf("create fifo_read thread error!\n");
		exit(0);
	}
	
    while(1) 
    {

        memset(msg_type,0,IPC_MSG_LENTH);
        //是否成功
        if(0 == Logined)
        {
            printf("-> Please input password!\n-> ");
        }
		else
		{
		    printf("$$");
		}

        //fgets(msg_type, IPC_MSG_LENTH,stdin);
        char *buf;
        if(0 == Logined)
    	{
	    	buf = getpass("");
			for(i=0;i<strlen(buf);i++)
			{
				printf("*");
			}
			printf("\n");
    	}
		else
		{
	        buf  = readline("");

		}
        strcpy(msg_type,buf); 
		if(1 == g_ushell_debug)
		{
			ushell_print_msg_from_input(msg_type);
		}
        if(0 == Logined)
        {
            if (strcmp(msg_type,PASSWORD)==0)
        	{
	      	    printf("-> Login success!!\n");
				ushell_help();
	      	    Logined = 1;
				continue;
        	}
			else if (strcmp(msg_type,"exit")==0)
        	{
                ushell_exit(0,0,0);
				return;
        	}
			else if (strcmp(msg_type,"ushell_debug")==0)
        	{
                g_ushell_debug = 1;
        	}
			else if (strcmp(msg_type,"exit_debug")==0)
        	{
                g_ushell_debug = 0;
        	}
			else
			{
			    printf("-> Error  password!!\n");
				HasTryLoginNum++;
				if(TRYLOGINNUM <= HasTryLoginNum)
				{
					printf("-> Input password exceed %d times，ushell exit \n",HasTryLoginNum);
					return;
				}
				goto USHELL_Input_ERROR;
			}
            
        }
		
        if(strlen(msg_type) == 0)
    	{
    	    continue;
    	}
		else if (strcmp(msg_type,"ushell_debug")==0)
    	{
            g_ushell_debug = 1;
    	}
		else if (strcmp(msg_type,"exit_debug")==0)
    	{
            g_ushell_debug = 0;
    	}
		//pr xxx PR xxx
        else if((strncmp(msg_type,"pr ",3) == 0)||(strncmp(msg_type,"PR ",3) == 0))
    	{
	    	ushell_take_over_print();
    	}
		//npr xxx NPR xxx
        else if((strncmp(msg_type,"npr ",4) == 0)||(strncmp(msg_type,"NPR ",4) == 0))
    	{
	        ushell_not_take_over_print();
		}
		//db xxx DB xxx
		else if((strncmp(msg_type,"db ",3) == 0)||(strncmp(msg_type,"DB ",3) == 0))
		{
			ushell_debug_process();
		}
		//ndb xxx NDB XXX 
		else if((strncmp(msg_type,"ndb ",4) == 0)||(strncmp(msg_type,"NDB ",4) == 0))
		{
			ushell_exit_debug_process();
		}
		//pad xxx PAD XXX 
		else if((strncmp(msg_type,"pad ",4) == 0)||(strncmp(msg_type,"PAD ",4) == 0))
		{
			ushell_print_and_debug_process();
		}
		//npad xxx NPAD XXX 
		else if((strncmp(msg_type,"npad ",5) == 0)||(strncmp(msg_type,"NPAD ",5) == 0))
		{
			ushell_exit_print_and_debug_process();
		}
        else if((strcmp(msg_type,"q")==0)||(strcmp(msg_type,"Q")==0))
    	{
	    	ushell_exit_all_proccess_debug();
    	}
		else if((strcmp(msg_type,"exit")==0)||(strcmp(msg_type,"EXIT")==0))
        {    
		    ushell_exit(0,0,0);
        }
		else if((strcmp(msg_type,"help")==0)||(strcmp(msg_type,"HELP")==0))
        {    
		    ushell_help();
        }
		else if((strcmp(msg_type,"pall")==0)||(strcmp(msg_type,"PALL")==0))
		{
			ushell_printf_all_Process_info();
		}
		else if((strcmp(msg_type,"PS")==0)||(strcmp(msg_type,"ps")==0))
        {    
		    system("ps");
        }
		/*发送命令到前台执行*/
        else
    	{
            ushell_send_command();
    	}
	USHELL_Input_ERROR:
	   add_history(msg_type);

   } /* while(1) */

   return 0;
}


