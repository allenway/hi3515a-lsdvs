/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**	文件名: recordDel.cpp
**  description  : 删除日期最老的文件,供循环录像使用,每次删除一整天
**  date           :  2013.10.18
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sched.h>
#include "const.h"
#include "debug.h"
#include "thread.h"
#include "const.h"
#include "malloc.h"
#include "record.h"
#include "hdd.h"
#include "recordDel.h"
#include "mutex.h"
#include "message.h"
#include "linuxFile.h"
#include "hddrw.h"
#include "log.h"
#include "dcpInsLocal.h"
#include "dcpTypes.h"

#if !defined(S_ISDIR)
#define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)
#endif

static CMutexLock g_recDelIfMutex;

static int g_startDelFlag = 0;
static pthread_t g_delFileForLoopRecThreadId;
static DEL_OLD_DATE *g_delRecFileList = NULL;
static int g_isHddFull = 0;  // 硬盘满报警

/*
* 检查分区目录下的目录是否为录像目录
* 返回:FI_TRUE-是录像目录,FI_FALSE-非录像目录
*/
static int CheckRecordDataDir(char *rec_dir)
{
	int count = 0;
	char *token;
	char *running = rec_dir;
	if(NULL == rec_dir)
    {
    	SVPrint("error:NULL == rec_dir!\r\n");
    	return FI_FALSE;
    }
	if(strlen(rec_dir) != 10) //"1910-11-03"
    {
    	return FI_FALSE;
    }
	while(1)
    {
    	if(NULL != (token=strsep(&running,"-")) && (2==strlen(token) || 4==strlen(token)))
        {        
        	count++;            
        }
    	else
        {
        	break;
        }
    }
	if(3 == count)
    {
    	return FI_TRUE;
    }
	else
    {
    	return FI_FALSE;
    }
}

/*
* 在所有的分区中找出日期最老的文件夹,考虑了多个分区存在同样日期的情况
*/
static DEL_OLD_DATE *FindOldestRecordDate(void)    
{
	char pathname[48] = {0};
	char old[48] = {0};    
	char check_dir[16] = {0};
	int i,ret;
	DIR     *dp;
	struct  dirent  *dirp;
	DEL_OLD_DATE *loop_node,*new_node;
	DEL_OLD_DATE *del_old_date_list = NULL;

	SVPrint("start FindOldestRecordDate,thread ID:%d\r\n",ThreadSelf());
    
	int part_num = FiHddGetMaxPartitionNum();
	old[0]='\0';
	g_startDelFlag = FI_TRUE;
    
	for(i=0;i<part_num;i++)
    {        
    	if(FI_FAILED == FiHddGetHddPartitionPath(i,pathname))
        	continue;    
            
    	if(NULL == (dp=HddrwOpendir(pathname)))
        {
        	SVPrint("HddrwOpendir(%s) error:%s\r\n",pathname,STRERROR_ERRNO);
        	continue;
        }
        
    	while(NULL != (dirp=HddrwReaddir(dp)))
        {
        	if (0 == (Strcmp(dirp->d_name, ".")) || (0 == Strcmp(dirp->d_name, "..")))
                continue;
            strncpy(check_dir,dirp->d_name,sizeof(check_dir)-1);
        	if(FI_FALSE == CheckRecordDataDir(check_dir))
            	continue;
        	if(old[0] == '\0')
            {
            	new_node = (DEL_OLD_DATE *)Malloc(sizeof(DEL_OLD_DATE));
            	if(NULL == new_node)
                	continue;
            	new_node->next = NULL;
                
            	strcpy(old,dirp->d_name);
            	sprintf(new_node->dirName,"%s/%s",pathname,old);
            	del_old_date_list = new_node;
            }
        	else
            {
            	ret = Strcmp(dirp->d_name,old);
            	if(ret < 0)    /*日期更老的文件夹的时候*/
                {
                	loop_node = del_old_date_list;
                	while (loop_node) 
                    {
                    	del_old_date_list = loop_node->next;
                    	Free(loop_node);
                    	loop_node = del_old_date_list;
                    }
                	del_old_date_list = NULL;
                    
                	new_node = (DEL_OLD_DATE *)Malloc(sizeof(DEL_OLD_DATE));
                	if(NULL == new_node)
                    {
                    	SVPrint("Malloc error:%s!\r\n",STRERROR_ERRNO);
                    	continue;
                    }    
                	new_node->next = NULL;
                    
                	strcpy(old,dirp->d_name);
                	sprintf(new_node->dirName,"%s/%s",pathname,old);
                	del_old_date_list = new_node;
                }
            	else if(ret == 0)/*不同分区中相同文件名的时候*/
                {
                	new_node = (DEL_OLD_DATE *)Malloc(sizeof(DEL_OLD_DATE));
                	if(new_node == NULL)
                    {
                    	SVPrint("Malloc error:%s!\r\n",STRERROR_ERRNO);
                    	continue;
                    }    
                	new_node->next = NULL;
                    
                	strcpy(old,dirp->d_name);
                	sprintf(new_node->dirName,"%s/%s",pathname,old);
                    
                	loop_node = del_old_date_list;
                	while(loop_node->next != NULL)
                    	loop_node = loop_node->next;
                	loop_node->next = new_node;
                }
            }
        } //while(NULL != (dirp=HddrwReaddir(dp)))        
    	HddrwClosedir(dp);   
    }
	return del_old_date_list;
}

/*
* 判断del是否被包含在为当前正在录像的文件名中,判断的条件很宽松,包括路径
*/
int IsRecordingFile(const char *checkName)
{    
	int i;
	int ch_num = REAL_CHANNEL_NUM;
	int flag = FI_FALSE;
	char recording_rec_name[RECORD_FILENAME_LEN] = {0};
	char recording_index_name[RECORD_FILENAME_LEN] = {0};
    
	if( NULL == checkName )
    {
    	SVPrint("error:NULL == filename!\r\n");
    	return FI_FALSE;        
    }
    
	for( i=0; i<ch_num; i++ )
    {
    	FiRecGetRecordingFilename(i,recording_rec_name,recording_index_name);
    	if( NULL != Strstr(recording_rec_name, (char *)checkName) 
            || NULL != Strstr(recording_index_name, (char *)checkName) )
        {
        	flag = FI_TRUE;
        	break;
        }
    }
    
	if(FI_TRUE == flag)
    {
    	return FI_TRUE;
    }
	else
    {
    	return FI_FALSE;
    }
}

/*
* 删除一整天的录像
*/
static int RmRecordDateDir(const char *pathname)
{
    char        	dirbuf[PATH_MAX]     = {0};
    DIR             *pdir                 = NULL;
    struct  dirent  *ep	                = NULL;
    struct  stat    buf                 = {0};

	if( (pdir = HddrwOpendir(pathname)) == NULL )
    {
    	SVPrint("HddrwOpendir(%s) error:%s!\r\n",pathname,STRERROR_ERRNO);
    	return(-1);
    }
    while( (ep = HddrwReaddir(pdir)) != NULL )
    {
        if(0 == Strcmp(ep->d_name, ".") || 0 == Strcmp(ep->d_name, ".."))
        {
        	continue;        
        }
    	snprintf( dirbuf, PATH_MAX, "%s/%s", pathname, ep->d_name );
        if( HddrwLstat(dirbuf, &buf) != 0 )
        {
        	HddrwClosedir( pdir );
        	SVPrint( "HddrwLstat(%s) error:%s!\r\n", dirbuf, STRERROR_ERRNO );
            return FI_FAILED;
        }
       	if( S_ISDIR(buf.st_mode) )
        {
            if( RmRecordDateDir(dirbuf) < 0 )
            {
            	HddrwClosedir( pdir );
            	return FI_FAILED;
            }
        }
        else
        {
        	if( FI_FALSE == IsRecordingFile(dirbuf) )
            {
            	HddrwChmod( dirbuf, 0777 );
            	Usleep( 100 );
                if( HddrwUnlink(dirbuf) < 0 )
                {
                	HddrwClosedir( pdir );
                	SVPrint( "HddrwUnlink(%s) error:%s!\r\n", dirbuf, STRERROR_ERRNO );
                	return FI_FAILED;
                }    
            	SVPrint( "del record file:%s!\r\n", dirbuf );
            	Usleep(100 * 1000);
            }
        }
    }
	if( HddrwClosedir(pdir) < 0 )
    {
    	SVPrint( "HddrwClosedir(%S) error:%s!\r\n", pdir, STRERROR_ERRNO );
    	return FI_FAILED;
    }
	if( FI_FALSE == IsRecordingFile(pathname) )
    {
    	if( HddrwRmdir(pathname) < 0 )
        {
        	SVPrint( "HddrwRmdir(%s) error:%s!\r\n", pathname, STRERROR_ERRNO );
        	return FI_FAILED;
        }
    }
    return FI_SUCCESSFUL;
}

static void *DelOldestRecFileThread(void *param)
{
	DEL_OLD_DATE *del_list = (DEL_OLD_DATE *)param,*list_bak = (DEL_OLD_DATE *)param;
	DEL_OLD_DATE *tmp_loop = NULL;

	ThreadDetach(ThreadSelf());
    
    // 为删除录像而停止录像服务
	RecordStopServiceForDel();    
	FiHddStopHotPlugForDelRecord();
    //Usleep(100000);
    
	SVPrint("DelOldestRecFileThread %d\r\n",ThreadSelf());
	g_startDelFlag = FI_TRUE;
    
	while(NULL != del_list)
    {            
    	Usleep(100*1000);
    	RmRecordDateDir(del_list->dirName);        
    	del_list = del_list->next;        
    }    
    
    /*要free参数传进来的节点*/
	while(list_bak)
    {
    	tmp_loop = list_bak->next;
    	Free(list_bak);
    	list_bak = tmp_loop;
    }
	g_delRecFileList     = NULL;
	g_startDelFlag         = FI_FALSE;

    // 为删除录像而重启录像服务	
	FiHddRestartHotPlugForDelRecord();
	RecordRestartServiceForDel();
    
	return(NULL);    
}

/*
* 是否支持循环录像
*/
static int g_loop_record = FI_FALSE;
static int IsSupportLoopRecord(void)
{    
	if(FI_TRUE == FiRecGetSupportLoopRecordFlag())
    {
    	return FI_TRUE;
    }
	else
    {
    	return FI_FALSE;
    }
}

/*
* 循环录像支持
* flag:0-不支持，1-支持
*/

int FiRecSetSupportLoopRecordFlag(int flag)
{
	g_recDelIfMutex.Lock();
	g_loop_record = flag;
	g_recDelIfMutex.Unlock();
	return FI_SUCCESSFUL;
}

int FiRecGetSupportLoopRecordFlag(void)
{
	return g_loop_record;
}

int FiRecInitLoopRecordFlag()
{
	int ret;
	PARAM_CONFIG_RECORD_PUBLIC param;
    
	ret = ParamGetRecordPublic( &param );
	if(0 == ret) 
    {
    	ret = FiRecSetSupportLoopRecordFlag( param.loopRecord );
    }
    
	return ret;
}


/*
* 剩余多少空间开始删除录像文件, 单位 M
*/
static int g_record_del_space = RECORD_DEL_SPACE;
int FiRecSetRecordDelSpace(int delSpace)
{
	int ret = -1;
	g_recDelIfMutex.Lock();
	if( delSpace >= 50 && delSpace <= 300 )
    {
    	g_record_del_space = delSpace;
    	ret = FI_SUCCESSFUL;
    }
	g_recDelIfMutex.Unlock();
	return ret;
}

int FiRecGetRecordDelSpace(void)
{
	return g_record_del_space;
}

/*
* 检查HDD是否满足删盘
* 返回:FI_FALSE:不满足,FI_TRUE:满足。
*/
static int CheckHddMeet(void)    
{
	int i,j;    
	int maxHddNum = FiHddGetMaxPartitionNum();
	int freeSize;

	if(FiHddGetTotalMounted() <= 0)
    {
    	return FI_FALSE;
    }
    
	for(i=0;i<5;i++)
    {        
    	for(j=0;j<maxHddNum;j++)
        {
        	freeSize = FiHddGetPartitionFreeSize(j);
        	if( freeSize > FiRecGetRecordDelSpace() )
            {                
            	return FI_FALSE;            
            }    
        }        
    	MessageSend( MSG_ID_CALCULATE_HDD_SIZE );        
    	sleep(2);        
    }    
    
	return FI_TRUE; 
}

static int CheckHddFull(void)    
{
	int j;    
	int maxHddNum = FiHddGetMaxPartitionNum();
	int freeSize;

	if(FiHddGetTotalMounted() <= 0)
    {
    	return FI_TRUE;
    }
    
    for(j=0;j<maxHddNum;j++)
    {
        freeSize = FiHddGetPartitionFreeSize(j);
        if( freeSize > MIN_PARTITION_SIZE )
        {                
            return FI_FALSE;            
        }    
    }        
    
	return FI_TRUE; 
}

void FiRecStartAutoDelOldFile(void)
{
	DEL_OLD_DATE *m_del_list, *toDel;        
	if(g_startDelFlag)    /*已经开始了删除线程*/
    {
    	return;
    }
	if(!IsSupportLoopRecord())    /*是否允许删除文件*/
    {
        if(!g_isHddFull)
        {
            // 检查是否硬盘满, 记录日志, 报警
            if(CheckHddFull())
            {
                g_isHddFull = 1;
                LogAdd(0xff, LOG_TYPE_STORAGE, LOG_LEVEL_CRITICAL, "Hdd if full.");
                FiHddHddStatErrReport(DCP_HDD_STAT_ERR_FULL);
            }
        }
        //FiPrint2("loop record:not support loop record!\r\n");
    	return;
    }    
	if(!CheckHddMeet())    
    {
        //FiPrint2("loop record:space enough!\r\n");
    	return;
    }
	sleep(1);
	g_delRecFileList = FindOldestRecordDate();    
	if(NULL == g_delRecFileList)
    {        
    	g_startDelFlag = 0;
    	return;
    }
	else
    {            
    	if( 0 != ThreadCreateCommonPriority( &g_delFileForLoopRecThreadId,
                            	DelOldestRecFileThread,
                                (void *)g_delRecFileList ))
        {
        	SVPrint( "ThreadCreateCommonPriority() error:%s!", STRERROR_ERRNO );
        	m_del_list = g_delRecFileList;
        	do
            {    
            	toDel = m_del_list;            
            	m_del_list = m_del_list->next;
            	Free(toDel);
            }while(NULL != m_del_list);
        	g_delRecFileList = NULL;
        	return;
        }
    }
	return;
}
