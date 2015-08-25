/*********************************************************************
* ��Ȩ���� (C)2008, ����ͨѶ�ɷ����޹�˾��
*
* �ļ����ƣ� ushell.c
* �ļ���ʶ��
* ����ժҪ�� Linux�����ڵ��Խ��̵�ushell����
* ����˵���� ��
* ��ǰ�汾�� V1.0
* ��    �ߣ� ���ֺ�
* ������ڣ�2010-03-16
*
* �޸ļ�¼1��
*    �޸����ڣ�
*    �� �� �ţ�V1.0
*    �� �� �ˣ�
*    �޸����ݣ�����
**********************************************************************/
/***********************************************************
 *                      ͷ�ļ�                             *
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
 *                 �ļ��ڲ�ʹ�õĺ�                        *
***********************************************************/

#define IPC_MSG_LENTH 512
#define IPC_MSG_NUM 50
#define IPC_FIFO_LENTH 4096

#define FIFO_NAME  "/ushell_fifo"
#define MSGQ_NAME "/ushell_msq"

#define FIFO_LENGTH  32
#define MSGQ_LENGTH  32

#define MAX_PROCESS_NUM 10

#define   SIGTASKWAKUP               (SIGRTMAX-1)       /* ����fifo thread select*/

#define  PASSWORD "zte"

#define  TRYLOGINNUM 3


/***********************************************************
 *               �ļ��ڲ�ʹ�õ���������                    *
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
  bool IsUseed;/*�Ƿ�ʹ�ã�TRUE��ʹ�ã�FALSE û��ʹ��*/
  bool IsPrintState;/*��ӡ��Ϣ�Ƿ�ӹ�*/
  bool IsCommandState;/*�����״̬*/
  int pid;/*ǰ̨����ID*/
  int fifoFd;/*fifo fd*/
  mqd_t mqd;/*��Ϣ����ID*/
  char ucFifoName[FIFO_LENGTH];/*�ܵ���*/
  char ucMsgqName[MSGQ_LENGTH];/*��Ϣ������ */
	
}ProcessInfo;

/***********************************************************
 *                     ȫ�ֱ���                            *
***********************************************************/
ProcessInfo g_tProcessInfo[MAX_PROCESS_NUM];/*���Խ�����Ϣ�������*/
char msg_type[IPC_MSG_LENTH];/* ������ռ������� */
char msg_buf[IPC_MSG_LENTH]; /* ������������̶��� */

int Logined = 0;/*�Ƿ��½�ɹ�*/
int HasTryLoginNum = 0;/*���Ե�¼������������Ե�¼�����ﵽTRYLOGINNUM�����Ƴ�ushell*/

fd_set readfds;/*fifo fd ����*/

pthread_t fifo_thread;/*fifo�ػ��߳�ID*/

int g_ushell_debug = 0;/*ushell ���ߵ��Կ��أ�0��ʾ�رգ�1��ʾ�򿪣�Ĭ�Ϲر�*/

/***********************************************************
 *                     ���ر���                            *
***********************************************************/

/***********************************************************
 *                     ȫ�ֺ���                            *
***********************************************************/
extern char *readline (char * a );
extern char *getpass( const char * prompt );

/***********************************************************
 *                     �ֲ�����                            *
***********************************************************/

/**********************************************************************
* �������ƣ�Exc_handler
* ����������SIGINT��Ctrl+C���źŴ�����
* ���ʵı���
* �޸ĵı���
* �����������
* �����������
* �� �� ֵ����
* ����˵������
* �޸�����      �汾��  �޸���      �޸�����
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                ����
************************************************************************/
void ushell_exit(int signo, siginfo_t *info, void *context)
{    
    int i = 0;
    ProcessInfo * pProcessInfo = g_tProcessInfo;
	
    memset(msg_buf,0,IPC_MSG_LENTH); 
    memcpy(msg_buf, "not_print_fifo_and_debug", strlen("not_print_fifo_and_debug"));
	
	//��ʼ��������Ϣ��
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
* �������ƣ�wake_up_select
* ���������������źŵ�fifo�ػ��̣߳�
                             ��fifo�ػ��̴߳�select�л��ѣ���ִ��fd_clr
                             ֮����Ҫ������������������п��ܵ���
                             fifo�ػ��߳�������fd_clr �е��Ǹ�select��
* ���ʵı���
* �޸ĵı���
* �����������
* �����������
* �� �� ֵ����
* ����˵������
* �޸�����      �汾��  �޸���      �޸�����
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                ����
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
* �������ƣ�wake_up_select_fun
* ����������wake_up�źŵĴ���������������ִ��
                             ����Ĳ���     
* ���ʵı���
* �޸ĵı���
* �����������
* �����������
* �� �� ֵ����
* ����˵������
* �޸�����      �汾��  �޸���      �޸�����
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                ����
***********************************************************************/

void ushell_wake_up_select_fun(void)
{
   return;
}


/**********************************************************************
* �������ƣ�pthread_fifo
* ����������FIFO�ػ��߳�
* ���ʵı���
* �޸ĵı���
* �����������
* �����������
* �� �� ֵ����
* ����˵������
* �޸�����      �汾��  �޸���      �޸�����
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                ����
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
* �������ƣ�ushell_help
* ����������ushell_helpʹ�ð�������
* ���ʵı���
* �޸ĵı���
* �����������
* �����������
* �� �� ֵ����
* ����˵������
* �޸�����      �汾��  �޸���      �޸�����
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                ����
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
* �������ƣ�ushell_take_over_print
* ����������pr xxx ����PR xxx :
                             �ӹ�ǰ̨xxx���̴�ӡ
* ���ʵı���
* �޸ĵı���
* �����������
* �����������
* �� �� ֵ����
* ����˵������
* �޸�����      �汾��  �޸���      �޸�����
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                ����
***********************************************************************/
void ushell_take_over_print(void)
{      
    /*��ȡ����Ҫ���ԵĽ���ID*/
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
	//�����PID�Ƿ��Ѿ����ڽӹܴ�ӡ״̬?
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
	//��g_tProcessInfo����һ�����е�pid��Ϣ�洢�ռ�
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
* �������ƣ�ushell_not_take_over_print
* ����������npr xxx ����NPR xxx :
                             �˳�ǰ̨xxx���̽ӹܴ�ӡ
* ���ʵı���
* �޸ĵı���
* �����������
* �����������
* �� �� ֵ����
* ����˵������
* �޸�����      �汾��  �޸���      �޸�����
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                ����
***********************************************************************/
void ushell_not_take_over_print(void)
{      
    /*��ȡ����Ҫ���ԵĽ���ID*/
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
	//�����PID�Ƿ��Ѿ����ڽӹܴ�ӡ״̬?
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
			    //�����˳���ӡ�ӹ�״̬
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
			//���ڵ���״̬���ͷ�ռ�õĽ�����Ϣ����
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
* �������ƣ�ushell_debug_process
* ����������pad  xxx����PAD xxx:
                             ���Ժͽӹ�xxxǰ̨����
* ���ʵı���
* �޸ĵı���
* �����������
* �����������
* �� �� ֵ����
* ����˵������
* �޸�����      �汾��  �޸���      �޸�����
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                ����
***********************************************************************/
void ushell_print_and_debug_process(void)
{      
    /*��ȡ����Ҫ���ԵĽ���ID*/
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
	//�����PID�Ƿ��Ѿ����ڵ��Ի��ߴ�ӡ�ӹ�״̬?
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
	//��g_tProcessInfo����һ�����е�pid��Ϣ�洢�ռ�
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
* �������ƣ�ushell_exit_print_and_debug_process
* ����������npad  xxx����NPAD xxx:
                             �˳����Ժͽӹܴ�ӡxxxǰ̨����
* ���ʵı���
* �޸ĵı���
* �����������
* �����������
* �� �� ֵ����
* ����˵������
* �޸�����      �汾��  �޸���      �޸�����
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                ����
***********************************************************************/
void ushell_exit_print_and_debug_process(void)
{      
    /*��ȡ����Ҫ���ԵĽ���ID*/
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
	//�����PID�Ƿ��Ѿ����ڽӹܴ�ӡ״̬?
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
* �������ƣ�ushell_debug_process
* ����������db  xxx����DB xxx:
                             ����xxxǰ̨����
* ���ʵı���
* �޸ĵı���
* �����������
* �����������
* �� �� ֵ����
* ����˵������
* �޸�����      �汾��  �޸���      �޸�����
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                ����
***********************************************************************/
void ushell_debug_process(void)
{      
    /*��ȡ����Ҫ���ԵĽ���ID*/
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
	//�����PID�Ƿ��Ѿ����ڵ���״̬?
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
	//��g_tProcessInfo����һ�����е�pid��Ϣ�洢�ռ�
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
* �������ƣ�ushell_exit_debug_process
* ����������ndb  xxx����NDB xxx:
                             �˳�����xxxǰ̨����
* ���ʵı���
* �޸ĵı���
* �����������
* �����������
* �� �� ֵ����
* ����˵������
* �޸�����      �汾��  �޸���      �޸�����
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                ����
***********************************************************************/
void ushell_exit_debug_process(void)
{      
    /*��ȡ����Ҫ���ԵĽ���ID*/
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
	//�����PID�Ƿ��Ѿ����ڽӹܴ�ӡ״̬?
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
			
			//��ǰpid�����ڴ�ӡ�ӹ�״̬
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
* �������ƣ�ushell_exit_proccess_debug
* ����������q ����Q :
                             �˳���ǰ���е��Ժͽӹܴ�ӡ��ǰ̨����   
* ���ʵı���
* �޸ĵı���
* �����������
* �����������
* �� �� ֵ����
* ����˵������
* �޸�����      �汾��  �޸���      �޸�����
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                ����
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
* �������ƣ�ushell_printf_all_Process_info
* ����������pall ����PALL:
                             ��ʾ��ǰ���Ժͽӹܴ�ӡ��ǰ̨������Ϣ
* ���ʵı���
* �޸ĵı���
* �����������
* �����������
* �� �� ֵ����
* ����˵������
* �޸�����      �汾��  �޸���      �޸�����
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                ����
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
* �������ƣ�ushell_send_command
* �����������������ǰ̨���Խ���
* ���ʵı���
* �޸ĵı���
* �����������
* �����������
* �� �� ֵ����
* ����˵������
* �޸�����      �汾��  �޸���      �޸�����
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                ����
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
* �������ƣ�ushell_print_msg_from_input
* ����������ushell ���Ժ�������ӡ�ӱ�׼�����ж�ȡ
                             ������Ϣ
* ���ʵı���
* �޸ĵı���
* �����������
* �����������
* �� �� ֵ����
* ����˵������
* �޸�����      �汾��  �޸���      �޸�����
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                ����
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
* �������ƣ�main
* ����������ushell����ں���
* ���ʵı���
* �޸ĵı���
* �����������
* �����������
* �� �� ֵ����
* ����˵������
* �޸�����      �汾��  �޸���      �޸�����
* ---------------------------------------------------------------------
* 2010/03/16    V1.0                ����
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
	sched_setscheduler(0, SCHED_FIFO, &sp);/*Ԥ�� 99�����ȼ����жϷ�������98�����ȼ���taskLock*/
    sched_getparam(0, &sp2);
	printf("sched_getscheduler = %d\t prio = %d\n", sched_getscheduler(0), sp2.sched_priority);
	printf("sched mode is SCHED_FIFO!\n");
	
	/*Add End*/


	
	//��ʼ��������Ϣ��
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
        //�Ƿ�ɹ�
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
					printf("-> Input password exceed %d times��ushell exit \n",HasTryLoginNum);
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
		/*�������ǰִ̨��*/
        else
    	{
            ushell_send_command();
    	}
	USHELL_Input_ERROR:
	   add_history(msg_type);

   } /* while(1) */

   return 0;
}


