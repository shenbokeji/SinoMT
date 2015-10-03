/*
 * This source file is ushellagent.c file for the 'lena' on DM368 platform

 *****************************************************************************
 * Copyright(C) 2015, SinoMartin Corp.
 *----------------------------------------------------------------------------
 * filename	: ushellagent.c
 * function	: ushellagent
 * author	version		date		note
 * feller	1.0		20150928	create         
 *----------------------------------------------------------------------------
*/

#include "typedef.h"
#include "internal.h"
/*************************************************************************************
*                 internal macro defined                                             *
*************************************************************************************/
#define SIGTASKTRACK	(SIGRTMAX-20)      /* ��������ѭ������ר���ź� */
#define ARG_NUM 		(10)
#define ARG_LENGTH 		(512)

#define IPC_MSG_LENTH 		(512)
#define IPC_MSG_NUM 		(50)
#define IPC_FIFO_LENTH 		(4096)
#define USHELL_NAME_LENGTH  (100)

#define PROC_NAME "LENA.EXE"
#define FIFO_NAME  "ushell_fifo"
#define MSGQ_NAME "/ushell_msq"

/*************************************************************************************
*                 internal datatype defined                                         *
*************************************************************************************/

int (*testFUNC)(int, int, int, int, int, int, int, int, int, int);
typedef void (*TaskEntryProto)(void *); /* �������ԭ�� */

typedef  struct ThreadInfo_tag{
    CHAR            flag;
    CHAR            pri;
    CHAR            name[60];
    TaskEntryProto  pFun;
    VOID            *arg;
    WORD32      thread_id;
    INT             stacksize;      /* �̶߳�ջ��С */
    WORD32          stackguardsize; /* �̶߳�ջ����ҳ��С */
    WORD32         stackaddr;      /* �̶߳�ջ�͵�ַ��ʼ�� */
    /*  CPU�����ʼ��� */
    WORD32          thread_tid;     /* LWP id */
    WORD32          prevtick;       /*  ���һ��ͳ��ʱ�߳����е��ܵ�ticksֵ */
    WORD32          dur_us;         /*  ���β��������е�ʱ��,��λus  */
    WORD32          tSpareInfo;
}T_ThreadInfo;
typedef pthread_t       VOS_TASK_T;

/************************************************************************************
 *                     local variable                                               *
************************************************************************************/
/* working in appstart */
mqd_t mqd;
struct mq_attr attr;
char msg[IPC_MSG_LENTH];

int flag_print = 0; //flag of stdout direct (fifo ? 1 : 0)
int flag_debug = 0; //flag of debug command state

unsigned char *gpfifo_name = NULL;
unsigned char *gmsgq_name = NULL;

int g_ushell_debug = 0;
/************************************************************************************
 *                     local function declaration                                   *
************************************************************************************/

static void InitSignal(void);
static void handle_signal(int s) ;

/************************************************************************************
 *                    local function defined                                       *
************************************************************************************/
/*----------------------------------------------------------------------------
 * name		: translate_priority
 * function	: 
 * input 	: 
 * author	version		date		note
 * feller	1.0		20151010      
 *----------------------------------------------------------------------------
*/
static int translate_priority(int v2pthread_priority, int sched_policy, int *errp)
{
    int max_priority, min_priority, pthread_priority;

    //  Validate the range of the user's task priority.
    if ( ( v2pthread_priority > 255 ) | ( v2pthread_priority < 0 ) )
    {
		*errp = -1;
	}   

    min_priority = sched_get_priority_min(sched_policy);
    max_priority = sched_get_priority_max(sched_policy);

    pthread_priority = max_priority - (v2pthread_priority / 3);
    
    pthread_priority -= 2;  /*Ԥ�� 99�����ȼ����жϷ�������98�����ȼ���taskLock*/

    return pthread_priority;
}
/*----------------------------------------------------------------------------
 * name		: EXC_SendSignalToTask
 * function	: 
 * input 	: 
 * author	version		date		note
 * feller	1.0		20151010      
 *----------------------------------------------------------------------------
*/
static unsigned long EXC_SendSignalToTask(pthread_t threadid, unsigned long signo)
{
    int ret;
    ret = pthread_kill( threadid, signo );
    if( 0 == ret )
    {
        return ret;
    }
    else
    {
        return -1;
    }
}

/*----------------------------------------------------------------------------
 * name		: Vos_StartTask
 * function	: 
 * input   	: 	pucName      - ������
 *               wPriority    - �������ȼ�
 *               dwStacksize  - ����ջ
 *               tTaskEntry   - �������
 *               lpTaskInPara - ������ڲ���
 * author	version		date		note
 * feller	1.0		20151010      
 *----------------------------------------------------------------------------
*/
static VOS_TASK_T Vos_StartTask(
    unsigned char *pucName,
    signed short wPriority,
    unsigned int dwStacksize,
    int sdwOptions,
    TaskEntryProto tTaskEntry,
    unsigned int dwTaskPara1    
)
{
	unsigned int iReturn = 0;
    pthread_attr_t attr;
    pthread_t threadID;
    struct sched_param scheparam;
    unsigned int no;
    int errcode   = 0;
    int swMemPageSize = getpagesize();

    /*consider data align*/

    /* modify by lizl,20070904,thread's stack has minimal size  */
    if( swMemPageSize > 0 )
    {
        dwStacksize += (1+1) * swMemPageSize;    /* ����4������ҳ,����һҳ������pthreadʹ�õ� */
    }

    if( dwStacksize < IPC_FIFO_LENTH )
    {
        dwStacksize = IPC_FIFO_LENTH; 
    }

    pthread_attr_init(&attr);

    if( pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED) != 0 )
    {
        printf("Vos_StartTask: set attr failed!\n");
    }
   
    pthread_attr_setschedpolicy(&attr,SCHED_FIFO);
    pthread_attr_getschedparam(&attr,&scheparam);
    scheparam.__sched_priority = translate_priority(wPriority,SCHED_FIFO,&errcode);
	//   scheparam.__sched_priority  = wPriority;
    pthread_attr_setschedparam(&attr,&scheparam);
    pthread_attr_setstacksize(&attr,dwStacksize);
	
    if( swMemPageSize > 0 )
    {
        pthread_attr_setguardsize(&attr,1*swMemPageSize);
    }
    
    errcode = pthread_create(&threadID,&attr,(void *)tTaskEntry,(void *)no);
	iReturn = errcode;

	return iReturn;
}
/*----------------------------------------------------------------------------
 * name		: InitSignal
 * function	: 
 * input 	: 
 * author	version		date		note
 * feller	1.0		20151010      
 *----------------------------------------------------------------------------
*/
/*��ʼ��ʱ����*/
static void InitSignal(void)
{
    signal(SIGPIPE,handle_signal);  
	return;
}

/*----------------------------------------------------------------------------
 * name		: handle_signal
 * function	: 
 * input 	: 
 * author	version		date		note
 * feller	1.0		20151010      
 *----------------------------------------------------------------------------
*/
/*�źŴ�����*/
static void handle_signal(int s) 
{ 
    printf("Rev SIGPIPE signal,do not exit!!\n");

    InitSignal();
	return;
}

/*----------------------------------------------------------------------------
 * name		: OSS_HexDump
 * function	: 
 * input 	: 
 * author	version		date		note
 * feller	1.0		20151010      
 *----------------------------------------------------------------------------
*/
static void OSS_HexDump (unsigned char *buf, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if ((i % 16) == 0)
		{
			printf( "%s%08x: ", i ? "\n" : "", (unsigned int)&buf[i] );
		}	
		printf ( "%s%02x", (i % 4) ? "" : " ", buf[i] );
	}
	printf("\n");
	return;
}

/*----------------------------------------------------------------------------
 * name		: SetFdBlockMode
 * function	: ����ushell������ģʽ
 * input 	: fd: �ļ�������
 *   			isBlock:*TRUE: ����Ϊ����ģʽ*FALSE: ����Ϊ������ģʽ
 * author	version		date		note
 * feller	1.0		20151010      
 *----------------------------------------------------------------------------
*/
static VOID SetFdBlockMode(int fd, BOOLEAN isBlock)
{   
    int flags;    

    flags = fcntl(fd, F_GETFL);    
    if(TRUE == isBlock)    
    {        
        flags &= ~O_NONBLOCK;     
    }    
    else    
    {        
        flags |= O_NONBLOCK;    
    }       
    if(fcntl(fd, F_SETFL, flags) < 0)    
    {        
        printf("SetFdBlockMode fcntl fd %d isBlock %d\n", fd, isBlock);    
    }
	return;
}

/*----------------------------------------------------------------------------
 * name		: pthread_msgqueue
 * function	: 
 * input 	: 
 * author	version		date		note
 * feller	1.0		20151010      
 *----------------------------------------------------------------------------
*/
static void *pthread_msgqueue(void *arg)
{
/* debug command return */
  int i,j,k;
  char command_buf[30],argstr[10][30],argtemp[30];
  int argint[10],argflag[10];
  int value;

  int fd, fd_old,fd_STDOUT;
  int  symtype,symsize; 

/* do with the ushell command */
	/* **********************************
	* ushell������֣�
	* �������
	* 1��printfifo����������ܵ���
	* 2�������������׼����豸��
	* 3��ִ�е����������������10���� 
	*************************************/
  while(1) 
  {
    memset(msg,0,IPC_MSG_LENTH);
    if(-1 == mq_receive(mqd, msg, IPC_MSG_LENTH, NULL))//len must bigger than the msg len sent
    {
		perror("mq_receive()");
	}
    

    /*1�� output direct to fifo */
    printf("%s\n",msg);

    if(strcmp(msg,"print_fifo") == 0)
    {    /* if output direct to fifo stdout */
        if(flag_print == 0) 
        {
            fd = -1;            
            fd=open((char *)gpfifo_name,O_WRONLY,0);    
            
            if(-1 == fd)            
            {               
                printf("open fifo:%s \n",gpfifo_name);                
                return NULL;            
            }  
            
            /* ����ushell�ķ�����ģʽ */            
            SetFdBlockMode(fd, FALSE);   
            
            fd_STDOUT=dup(STDOUT_FILENO); 
            
            dup2(fd,STDOUT_FILENO);
            
            printf("ushell enter print mod \n "); 
            
            flag_print = 1;
        }
    }
    /*2�� output direct to fifo stdout */
    else if((strcmp(msg,"quit")) == 0)
    {    /* if output direct to fifo */
        if(flag_print == 1)
        {
            dup2(fd_old,STDOUT_FILENO);
            flag_print = 0;
        }
        flag_debug = 0;

    }
    /* debug command */
    else if((strcmp(msg,"debug")) == 0)
    {
        printf("ushell enter debug mod \n ");
        flag_debug = 1;
    }

    else if((strcmp(msg,"print_fifo_and_debug")) == 0)
    {
            if(flag_print == 0)         
            {            
            fd = -1;            
            fd=open((char *)gpfifo_name,O_WRONLY,0);            
            if(-1 == fd)            
            {                
                printf("open fifo:%s error\n",gpfifo_name);               
                return NULL;            
            }            /* ����ushell�ķ�����ģʽ */           
            SetFdBlockMode(fd, FALSE);                
            fd_STDOUT=dup(STDOUT_FILENO);           
            dup2(fd,STDOUT_FILENO);           
            printf("ushell enter print mod \n ");           
            flag_print = 1;        
            }                
            printf("ushell enter debug mod \n ");       
            flag_debug = 1;
    }

   else if((strcmp(msg,"not_print_fifo_and_debug")) == 0)
    {
            dup2(fd_STDOUT,STDOUT_FILENO);
            close(fd_STDOUT);                
            close(fd);
            flag_print =0;
    }

    /* 3��excute the debug command */
    else
    {   
        if(1 == flag_debug)
        {
        /* get the command */
        memset(command_buf,'\0',30);
        memset(argstr[0],'\0',300);
        for(i=0;i<10;i++)
        {
          argflag[i]=0;
        }
        if(g_ushell_debug == 1)
        {
            printf("%s\n",msg);
        }

       /* ���ַ�����������ֽ�Ϊ�����Ͳ��� */
        i=0;j=0;k=0;
        for(i = 0;(i<IPC_MSG_LENTH);i++)
        {/* ���������� */
            if(j == 0)
            {
                        if((msg[i]!=' ')&&(msg[i]!='\0'))
                    {    
                        command_buf[k] = msg[i];
                        k++;
                    }
                    else   
                    {          
                        command_buf[k] = '\0';      
                        j++;
                        k = 0;                    
                    }
            }
            else
            {/* �������룬�ַ�����ʽ */
                if((msg[i]!=',')&&(msg[i]!='\0'))
                {    
                    //argstr[j-1][29]�������\0,������಻�ܳ���29���ַ�
                    if(k<29) 
                    {
                        argstr[j-1][k] = msg[i];
                        k++;
                    }
                    else
                    {
                        printf(" para %d is too long \n",j);
                        goto USHELL_PARA_ERROR;
                    }
                }
                else   
                {       
                    argstr[j-1][k] = '\0';         
                    j++;
                    k = 0;
                    if(msg[i]=='\0')
                    {
                        break;
                    }
                }
            }
        }
        
        /* �����ַ�����ʽת��Ϊ������ */
        for(i = 0; i < 10; i++)
        {
            //����Ϊ16������
            if(((argstr[i][0] == '0')&&(argstr[i][1] == 'x'))
                ||((argstr[i][0] == '0')&&(argstr[i][1] == 'X')))
            {
                sscanf(argstr[i],"0x%x",&argint[i]);
                if(g_ushell_debug == 1)
                {
                    printf("line :%d argstr[%d] :%s argint[%d]:0x%x \n",__LINE__,i,argstr[i],i,argint[i]);
                }
            }
            //����Ϊ�ַ���
            else if(argstr[i][0] == '\"')
            {
                argflag[i]=1;
                //����"
                memcpy(argtemp,&argstr[i][1],strlen(&argstr[i][1]));
                argtemp[strlen(&argstr[i][1])-1]='\0';
                strcpy(argstr[i],argtemp);
                if(g_ushell_debug == 1)
                {
                    printf("line :%d argstr[%d] :%s  \n",__LINE__,i,argstr[i]);
                }
                
            }
            //����Ϊʮ����
            else
            {
                argint[i] = atoi(argstr[i]);
                if(g_ushell_debug == 1)
                {
                    printf("line :%d argstr[%d] :%s argint[%d]:0x%x \n",__LINE__,i,argstr[i],i,argint[i]);
                }
            }
        }
        
        /* �ֽ�����Ժ�����ʽ: command_buf(argint[0], argint[1], argint[2]) */        

        /* excute debug command */

        if(BspSymFindByName(command_buf,&testFUNC,&symsize,&symtype) == 0) 
        {      
            //ִ�к���
            if(STT_FUNC == symtype )
            {
                //printf("0x%x 0x%x 0x%x 0x%x ",argint[0],argint[1],argint[2],argint[3]);
                
                value = testFUNC(((argflag[0]== 0)?argint[0]:(int)argstr[0]),
                                 ((argflag[1]== 0)?argint[1]:(int)argstr[1]),
                                 ((argflag[2]== 0)?argint[2]:(int)argstr[2]),
                                 ((argflag[3]== 0)?argint[3]:(int)argstr[3]),
                                 ((argflag[4]== 0)?argint[4]:(int)argstr[4]),
                                 ((argflag[5]== 0)?argint[5]:(int)argstr[5]),
                                 ((argflag[6]== 0)?argint[6]:(int)argstr[6]),
                                 ((argflag[7]== 0)?argint[7]:(int)argstr[7]),
                                 ((argflag[8]== 0)?argint[8]:(int)argstr[8]),
                                 ((argflag[9]== 0)?argint[9]:(int)argstr[9])
                                 );
                printf("value = %d\n",value);
            }
            //��ʾ����ֵ
            else if(STT_OBJECT == symtype )
            {
                switch(symsize)
                {
                
                case 1:
                    {
                        printf("%s = 0x%x value=0x%02x\n",command_buf,(int)testFUNC,*(char *)testFUNC);
                        break;
                    }
                case 2:
                    {
                        printf("%s = 0x%x value=0x%04x\n",command_buf,(int)testFUNC,*(unsigned short *)testFUNC);
                        break;
                    }
               case 4:
                    {
                        printf("%s = 0x%x value=0x%08x\n",command_buf,(int)testFUNC,*(unsigned long *)testFUNC);
                        break;
                    }
               default:
                    {
                        int i;
                        char * ucptemp;
                        
                        ucptemp = (char *)testFUNC;
                        printf("%s= addr:0x%x\n",command_buf,testFUNC);
                        for(i=0;i<symsize;i++)
                        {
                            if((i%4==0)&&(i>0))
                            {
                                printf("  ");
                            }
                            if((i%16==0)&&(i>0))
                            {
                                printf("\n");
                            }
                            printf("%02x",*(ucptemp+i));
                            
                        }
                        printf("\n");
                    }

                    
                }
            }
            else
            {
               printf("symtype:%d error \n",symtype);
            }
            
            
        }
        else 
        {
            printf("sym:%s not found!\n",command_buf);
        }
		USHELL_PARA_ERROR:
       continue;
        
      }
    } 
  }
}

/*----------------------------------------------------------------------------
 * name		: symbol_init
 * function	: 
 * input 	: filename
 * author	version		date		note
 * feller	1.0		20151010      
 *----------------------------------------------------------------------------
*/
/* ����elf�ļ������������ű��ɸ��ݽ������޸�elf�ļ�����ʹ�þ���·���� */
static int symbol_init(char * filename)
{
    InitSymbolTable(PROC_NAME);
	return 0;
}
/*----------------------------------------------------------------------------
 * name		: ushell_init
 * function	: ushell initial function 
 * input 	: 
 * author	version		date		note
 * feller	1.0		20151010      
 *----------------------------------------------------------------------------
*/
int ushell_init()
{
    int fd;
    int iResult = 0;
    pthread_t msgqueue_thread;

    unsigned long ulpid = 0;

    ulpid = getpid();

    InitSignal();
     
    /* elf symtab init */
    symbol_init(PROC_NAME);

    gpfifo_name = malloc(USHELL_NAME_LENGTH);
    gmsgq_name = malloc(USHELL_NAME_LENGTH);

    memset(gpfifo_name,0,USHELL_NAME_LENGTH);
    memset(gmsgq_name,0,USHELL_NAME_LENGTH); 

    sprintf(gpfifo_name,"%s%d",FIFO_NAME,ulpid);    
    sprintf(gmsgq_name,"%s%d",MSGQ_NAME,ulpid);      
	
    /* fifo for redirect stdout */
    unlink(gpfifo_name);   
	iResult	= mkfifo(gpfifo_name,O_CREAT);
    if( 0 != iResult )
    {
		printf( "the errno = %d\n", errno );
        printf( "Create fifo error!\n" );
        return iResult;
    }
    /* msg queue */
    shm_unlink(gmsgq_name);
    memset(&attr, 0, sizeof(attr));
    attr.mq_maxmsg  = IPC_MSG_NUM;
    attr.mq_msgsize = IPC_MSG_LENTH;

    mqd = mq_open(gmsgq_name, O_RDONLY | O_CREAT,666,&attr);
    if ((mqd_t)-1 == mqd)
    {
		iResult = mqd;
        printf("Open MsgQueue error!");
        return iResult;
    }

	iResult = Vos_StartTask("ushellAgent",
              1,
              32*1024,
              0,
              (TaskEntryProto)pthread_msgqueue,
              0);
    if ( 0 != iResult )
    {
        printf("ushell_init Create thread error!\n ");
        return iResult;
    }
    
    return iResult;

}

/* fuctions for test */

/*----------------------------------------------------------------------------
 * name		: m
 * function	: modify addr value
 * input 	: iaddr: address; len: data number
 * author	version		date		note
 * feller	1.0		20151010      
 *----------------------------------------------------------------------------
*/
int m(int iaddr,int value)
{
     char * puctemp = NULL;
     printf("iaddr:0x%x value:%d\n",iaddr,value);
     if( (int)NULL == iaddr )
     {
        return -1;
     }
     puctemp = (char *)iaddr;
     *puctemp = (char)value;
     return *puctemp;
}
/*----------------------------------------------------------------------------
 * name		: d
 * function	: read memory
 * input 	: iaddr: address; len: data number
 * author	version		date		note
 * feller	1.0		20151010      
 *----------------------------------------------------------------------------
*/
int d(int iaddr,int len)
{
     int i;
     char * ucptemp=NULL;
     ucptemp = (char *)iaddr;
     printf("addr:0x%x \n",iaddr);

	if( (int)NULL == iaddr )
    {
        return -1;
    }
 
    OSS_HexDump( (unsigned char *)iaddr,len );
     
    return 0;
}



