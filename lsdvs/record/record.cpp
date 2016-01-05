/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**	文件名: record.cpp
**  description  : 录像,取视频帧和写文件,切换文件
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
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "const.h"
#include "debug.h"
#include "proconApp.h"
#include "malloc.h"
#include "linuxFile.h"
#include "recordPool.h"
#include "hdd.h"
#include "record.h"
#include "timeExchange.h"
#include "thread.h"
#include "const.h"
#include "public.h"
#include "recordDel.h"
#include "recordGetH264.h"
#include "recordFit.h"
#include "recordPool.h"
#include "sysRunTime.h"
#include "hddrw.h"
#include "gpio.h"

static const uint all_rec_type[MAX_SUPPORT_RECORD_TYPE_NUM] = 
{
	RECORD_TYPE_HAND,
	RECORD_TYPE_TIMER,
	RECORD_TYPE_ALARM_IO,
	RECORD_TYPE_ALARM_MD,
	RECORD_TYPE_ALARM_LOST,
	RECORD_TYPE_ALARM_SHELTER,
};

static RECORD_THREAD rec_thread[MAX_CHANNEL_NUM];    //各通道的录像线程相关
static RECORD_MANAGE rec_manage;                    //所有录像通道共用一个监视线程
static int g_rec_handle[MAX_CHANNEL_NUM] ;            //各通道h264录像文件操作句柄
static int g_index_handle[MAX_CHANNEL_NUM] ;        //各通道录像检索文件操作句柄
static RECORD_INDEX rec_index[MAX_CHANNEL_NUM];     //各通道的检索信息
static unsigned int rec_count = 0;                    //录像文件计数
static int rec_file_switch_time = 60*RECORD_TIMER_SWITCH_TIME;    //录像文件切换时间间隔
static int rec_file_switch_time_alarm = 60*RECORD_TIMER_SWITCH_TIME_ALARM;    //报警录像文件切换时间间隔
static uint recType[MAX_CHANNEL_NUM] = {0}; //各通道的录像类型
static time_t alarm_cursor_time[REAL_CHANNEL_NUM];    //报警游标时间
static int manage_init_flag = -1;    //监视线程初始化标志 -1:还没有初始化
static pthread_mutex_t record_control_lock; //录像控制模块公共锁
static RECORD_TIMER g_rec_timer[MAX_CHANNEL_NUM];
//static RECORD_PUBLIC g_rec_public;
static RECORD_HAND g_rec_hand[MAX_CHANNEL_NUM];
static int g_videoEncodeOpenFlag[MAX_CHANNEL_NUM];

// static char g_recFileErrFileName[RECORD_FILENAME_LEN];
/*
* 设置录像间隔时间
*/
int FiRecSetRecFileSwitchTime(int switchTime)
{
	int ret = -1;
	pthread_mutex_lock(&record_control_lock);
	if( switchTime >= 10 && switchTime <= 30 )
    {
    	rec_file_switch_time = switchTime * 60;
    	ret = 0;
    }
	pthread_mutex_unlock(&record_control_lock);
    return ret;
}
int FiRecGetRecFileSwitchTime()
{
    int switchTime;
	pthread_mutex_lock(&record_control_lock);
    switchTime = rec_file_switch_time;
	pthread_mutex_unlock(&record_control_lock);
    return switchTime;
}
int FiRecSetRecFileSwitchTimeAlarm(int switchTime)
{
	pthread_mutex_lock(&record_control_lock);
    rec_file_switch_time_alarm = switchTime * 60;
	pthread_mutex_unlock(&record_control_lock);
    return FI_SUCCESSFUL;
}
int FiRecGetRecFileSwitchTimeAlarm()
{
    int switchTime;
	pthread_mutex_lock(&record_control_lock);
    switchTime = rec_file_switch_time_alarm;
	pthread_mutex_unlock(&record_control_lock);
    return switchTime;
}

void FiRecGetHandRecordParam(int channel,RECORD_HAND *recHand)
{
	if(NULL == recHand)
    {
    	SVPrint("error:NULL == recHand!\r\n");
    	return;
    }
    *recHand = g_rec_hand[channel];
}

static void FiRecStartHandRecord(int channel)
{    
	pthread_mutex_lock(&rec_thread[channel].recordLock);
	g_rec_hand[channel].recHand = FI_TRUE;    
	pthread_mutex_unlock(&rec_thread[channel].recordLock);
}

static void FiRecStopHandRecord(int channel)
{
	pthread_mutex_lock(&rec_thread[channel].recordLock);
	g_rec_hand[channel].recHand = FI_FALSE;
	pthread_mutex_unlock(&rec_thread[channel].recordLock);
}

int FiRecSetHandRecord( int channel, PARAM_CONFIG_RECORD_PARAM *pRecordParam )
{
	if( NULL==pRecordParam || channel>REAL_CHANNEL_NUM )
    {
    	SVPrint("error:NULL==pRecordParam || channel(%d)>REAL_CHANNEL_NUM!\r\n",channel);
    	return FI_FAIL;
    }
    
	if( 1 == pRecordParam->recHand.recFlag )
    	FiRecStartHandRecord(channel);
	else
    	FiRecStopHandRecord(channel);

	SVPrint("channel(%d) hand record:enable flag(%d)\r\n",channel, pRecordParam->recHand.recFlag);

	return FI_SUCCESS;
}

static uint GetRecordType(int channel)
{    
	uint ret_rec_type;
	pthread_mutex_lock(&rec_thread[channel].recordLock);
	ret_rec_type = recType[channel];
	pthread_mutex_unlock(&rec_thread[channel].recordLock);
	return ret_rec_type;
}

static void SetRecordType(int channel,uint val)
{
	pthread_mutex_lock(&rec_thread[channel].recordLock);
	recType[channel] = val;
	pthread_mutex_unlock(&rec_thread[channel].recordLock);
}

static int CloseRecordAndIndexFile(int channel)
{
	if(-1 != g_rec_handle[channel])
    {
    	HddrwClose(g_rec_handle[channel]);
    	g_rec_handle[channel] = -1;
    }
	if(-1 != g_index_handle[channel])
    {
    	HddrwClose(g_index_handle[channel]);
    	g_index_handle[channel] = -1;
    }
    
	FiRecSetRecordingFilename(channel, (char *)"0", (char *)"0");
    
	return FI_SUCCESS;
}

static void ResetIndexStruct(int channel)
{
	rec_index[channel].indexVersion = RECORD_INDEX_VERSION;
	rec_index[channel].recordType = 0;
	rec_index[channel].fileLen = 0;
	rec_index[channel].videoFrameSum = 0;
	rec_index[channel].firstFrameTimestamp = 0;
	rec_index[channel].lastFrameTimestamp = 0;    
}

/*
* 创建录像文件和对应的索引文件
* 返回:0-成功,其他-失败
*/
static int OpenRecordAndIndexFile(int channel,char *recFilename,char *index_filename)
{
	if(-1 != g_rec_handle[channel])
    {
    	HddrwClose(g_rec_handle[channel]);
    	g_rec_handle[channel] = -1;
    }
	g_rec_handle[channel] = HddrwOpen(recFilename, O_RDWR|O_CREAT|O_TRUNC );
	if(-1 == g_rec_handle[channel])
    {
    	SVPrint("HddrwOpen(%s) error:%s!\r\n",recFilename,STRERROR_ERRNO);
    	return FI_FAIL;
    }    
	HddrwChmod(recFilename, 0777);
	SVPrint("ch(%d)rec file:%s,handle(%d)\r\n", channel,recFilename, g_rec_handle[channel]);

	if(-1 != g_index_handle[channel])
    {
    	HddrwClose(g_index_handle[channel]);
    	g_index_handle[channel] = -1;
    }
	g_index_handle[channel] = HddrwOpen(index_filename, O_WRONLY|O_CREAT|O_TRUNC);
	if(-1 == g_index_handle[channel])
    {
    	SVPrint("HddrwOpen(%s) error:%s!\r\n", index_filename, STRERROR_ERRNO);
    	return FI_FAIL;
    }    
    
	HddrwChmod(index_filename, 0777);
	SVPrint("ch(%d)index file:%s,handle(%d)\r\n", channel, index_filename, g_index_handle[channel]);

	return FI_SUCCESS;
}

static int GenRecFilename(int channel,time_t utc,char *recFilename,char *index_filename)
{
	int ret;
	char hddPath[32] = {0};
	char filepath[RECORD_FILENAME_LEN] ={0};
	char datePath[RECORD_FILENAME_LEN] ={0};
	char channelPath[RECORD_FILENAME_LEN] ={0};
	int year,month,day,hour,minute,second;

	if(NULL==recFilename || NULL==index_filename)
    {
    	SVPrint("error:NULL==recFilename || NULL==index_filename!\r\n");
    	return FI_FAIL;
    }
    
	if(FI_FAIL == FiHddGetUsingPartition(hddPath))
    {
    	FiPrint2("FI_FAIL == FiHddGetUsingPartition!\r\n");
    	return FI_FAIL;
    }
	FiTimeUtcToHuman(utc,&year,&month,&day,&hour,&minute,&second);    
	FiPrint2("utc=%d,%d-%d-%d %d:%d:%d\r\n",utc,year,month,day,hour,minute,second);
    
	snprintf(datePath,sizeof(filepath),"%s/%04d-%02d-%02d",hddPath,year,month,day);
	ret = HddrwMkdir( datePath, 0777 );
	if( 0 == ret || errno == EEXIST )
    {        
    	HddrwChmod( datePath, 0777 );
    	ret = 0;
    }
	else
    {
    	SVPrint( "error:HddrwMkdir(%s):%s!\r\n", datePath, STRERROR_ERRNO );
    }

	FiPrint2("datePath = %s\r\n",datePath);
    
	snprintf(channelPath,sizeof(channelPath),"%s/ch%02d",datePath,channel);
	HddrwMkdir( channelPath, 0777 );
	if( 0 == ret || errno == EEXIST )
    {        
    	HddrwChmod( channelPath, 0777 );
    	ret = 0;
    }
	else
    {
    	SVPrint( "error:HddrwMkdir(%s):%s!\r\n", channelPath, STRERROR_ERRNO );
    }

	FiPrint2("channelPath = %s\r\n",channelPath);
    /* /record/hd00 /10-10-28 /ch00 /brec_11h45m50s_000.dat */
	snprintf(filepath,sizeof(filepath),"%s/%s%02dh%02dm%02ds_%03d",
            	channelPath, RECORD_FILENAME_PREFIX,
            	hour, minute, second, rec_count++ ); 
                
	snprintf(recFilename,sizeof(filepath),"%s.dat",filepath);
	snprintf(index_filename,sizeof(filepath),"%s.index",filepath);
//	FiPrint2("record filename:%s\r\n",recFilename);
//	FiPrint2("record filename:%s\r\n",index_filename);

	return ret;
}

static char g_recordingRecFilename[REAL_CHANNEL_NUM][RECORD_FILENAME_LEN];
static char g_recordingIndexFilename[REAL_CHANNEL_NUM][RECORD_FILENAME_LEN];
/*
* 把当前的录像文件全名缓存起来,供其他模块使用
*/
int FiRecSetRecordingFilename(int channel,char *rec_name,char *index_name)
{
	if(NULL == rec_name || NULL == index_name)
    {
    	SVPrint("error:NULL == rec_name || NULL == index_name!\r\n");
    	return FI_FAIL;
    }
    
	pthread_mutex_lock(&rec_thread[channel].recordLock);
	strcpy(g_recordingRecFilename[channel],rec_name);
	strcpy(g_recordingIndexFilename[channel],index_name);    
	pthread_mutex_unlock(&rec_thread[channel].recordLock);
    
	return FI_SUCCESS;
}

int FiRecGetRecordingFilename(int channel,char *rec_name,char *index_name)
{
	if(NULL == rec_name || NULL == index_name)
    {
    	SVPrint("error:NULL == rec_name || NULL == index_name!\r\n");
    	return FI_FAIL;
    }
    
	pthread_mutex_lock(&rec_thread[channel].recordLock);
	strcpy(rec_name,g_recordingRecFilename[channel]);
	strcpy(index_name,g_recordingIndexFilename[channel]);    
	pthread_mutex_unlock(&rec_thread[channel].recordLock);    
    
	return FI_SUCCESS;
}

/*
* 创建录像文件和索引文件
* 返回:0-成功,小于0-失败
*/
static int CreateRecordAndIndexFile( int channel, time_t utc )
{    
	char rec_file[RECORD_FILENAME_LEN] = {0};
	char index_file[RECORD_FILENAME_LEN] = {0};        
    
	if(FI_FAIL == GenRecFilename(channel,utc,rec_file,index_file))
    {
    	FiPrint2("FI_FAIL == GenRecFilename!\r\n");
    	return FI_FAIL;
    }
	if(FI_SUCCESS == OpenRecordAndIndexFile(channel,rec_file,index_file))
    {
    	FiRecSetRecordingFilename(channel,rec_file,index_file);        

    	return FI_SUCCESS;
    }
	else
    {
    	FiPrint2("failed:OpenRecordAndIndexFile()\r\n");
    	return FI_FAIL;
    }    
}

/*
* fn: 向文件中写入所有字节才返回
* 返回: -1, 失败,需要写入的字节数
*/
static int writeRecordDataComplete( int fd, const void *vptr, int n )
{
	int nleft;
	int nwritten;
	const char *ptr;

	ptr = (char *)vptr;
	nleft = n;
	while (nleft > 0) 
    {
    	if ( (nwritten = HddrwWrite(fd, ptr, nleft)) <= 0) 
        {
        	if (errno == EINTR)
            	nwritten = 0; /* and call write() again */
        	else
            	return(-1); /* error */
        }
    	if (errno == 5)
        	return(-1);
    	nleft -= nwritten;
    	ptr += nwritten;
    }
    
	return(n);
}


static int WriteIndexDataToIndexFile( int channel )
{
	int writeSize = 0;
	int size;
	int ret = FI_FAIL;
    
	if( -1 != g_index_handle[channel] )
    {    
    	lseek(g_index_handle[channel],0,SEEK_SET);
    	size = sizeof(rec_index[channel]);
    	writeSize = writeRecordDataComplete(g_index_handle[channel],
                                (char *)&rec_index[channel], size );
    	if( writeSize != size )
        {
        	SVPrint( "writeSize(%d) = Writen(%u) error:%s!\r\n", 
                        	writeSize, size, STRERROR_ERRNO );
        	ret = FI_FAIL;
        }
    	else
        {
        	ret = FI_SUCCESS;
        }    
    }
    
	return ret;    
}

static void RefreshIndexStruct(int channel,uint recordType,uint frameSize,time_t utc)
{
	rec_index[channel].indexVersion = RECORD_INDEX_VERSION;
	rec_index[channel].recordType = recordType;
	rec_index[channel].fileLen += frameSize;
	rec_index[channel].videoFrameSum ++;
	if(1 == rec_index[channel].videoFrameSum)
    	rec_index[channel].firstFrameTimestamp = utc;
	rec_index[channel].lastFrameTimestamp = utc;    
}

/*
* 把帧头和帧数据写进硬盘里
* pRecPcp:从录像缓冲池中取出来的一帧(也可以说是一个节点)的数据
*/
static long WriteFrameDataToRecordFile( int channel, PCP_NODE_T *pRecPcp, uint size )
{
	int writedSize = 0;
	int ret = FI_FAIL;
    
	if( NULL == pRecPcp )
    {
    	SVPrint("error:NULL == pRecPcp!\r\n");
    	return FI_FAIL;
    }
    
	if( -1 != g_rec_handle[channel] )
    {            
    	writedSize = writeRecordDataComplete( g_rec_handle[channel], pRecPcp->data, size );
    	if( size != (uint)writedSize )
        {
        	SVPrint( "writedSize(%d) = Writen(%u) error:%s!\r\n", 
                            	writedSize, size, STRERROR_ERRNO );
        	ret =  FI_FAIL;
        }
    	else
        {
        	ret = FI_SUCCESS;
        }
    }    
    
	return ret;
}

/*每个通道写满2Mbytes数据就fsync
* size一次写入的大小
*
*/

static uint g_writedSize[MAX_CHANNEL_NUM] = {0};
static const uint RECORD_FSYNC_SIZE = 200 * 1024;
static int FsyncRecordData( int channel )
{    
	int ret = 0;
	if( g_writedSize[channel] >= RECORD_FSYNC_SIZE )
    {
    	ret = HddrwFsync( g_rec_handle[channel] );
        if(ret == -1 && EINTR != errno) // && errno == EIO)
        {
            FiHddSetHddPartitionFault(-1);
            SVPrint("%s error:%d:%s.\r\n", __FUNCTION__, errno, STRERROR_ERRNO);
        }
    	g_writedSize[channel] = 0;
    }    

	return ret;
}

static int WriteDataToDisk( int channel, PCP_NODE_T *pRecPcp, uint size,
                                        	uint recordType, time_t utc )
{
	int ret = FI_FAIL;
    
	ret = WriteFrameDataToRecordFile( channel, pRecPcp, size );
	if(FI_FAIL != ret)
    {
    	RefreshIndexStruct( channel, recordType, size, utc );
    	ret = WriteIndexDataToIndexFile( channel );
    }
    // sven add for FsyncRecordData
	g_writedSize[channel] += size;
	if(FI_FAIL != ret)
    {
    	ret = FsyncRecordData( channel );
    }
	return ret;
}

/************************************************************
* 从缓冲池中拿到一帧以后取出我们需要的数据
* pRecPcp:从缓冲池中取出的帧
* utc:out,时间戳
* recordType:out,录像类型(定时录像、报警录像等等)
* writeSize:out,要写进硬盘的数据大小
* frameType:out,帧类型(I、P、智能等等)
******************************************************************/
static int AnalysisFrameData( int channel, PCP_NODE_T *pRecPcp, time_t *utc,
                              uint *recordType, uint *writeSize,
                              uchar *frameType )
{
	uint curRecordType = *recordType;
    
	if( NULL==pRecPcp || NULL==utc || NULL==recordType || NULL==writeSize || NULL==frameType )
    {
    	SVPrint("error:NULL==pRecPcp || NULL==utc || NULL==recordType || NULL==writeSize || NULL==frameType!\r\n");
    	return FI_FAIL;
    }
    
    *frameType = pRecPcp->pcpHead.type;
    *writeSize = pRecPcp->pcpHead.len;
    *utc = time(NULL);
	curRecordType = GetRecordType(channel);    
    *recordType |= curRecordType;
    
	return FI_SUCCESS;
}

static int GetRecordCtlFlag(int channel)
{
	int flag;
	pthread_mutex_lock(&rec_thread[channel].recordLock);
	flag = rec_thread[channel].recordCtlFlag;
	pthread_mutex_unlock(&rec_thread[channel].recordLock);
	return flag;
}

static void SetRecordCtlFlag(int channel,int recordCtlFlag)
{
	pthread_mutex_lock(&rec_thread[channel].recordLock);
	rec_thread[channel].recordCtlFlag = recordCtlFlag;
	pthread_mutex_unlock(&rec_thread[channel].recordLock);
}

/*
* 更新定时录像时间基游标
*/
static int RefrechTimerBaseTimeCursor(int *realTime,int *realYear,int *realMonth,int *realDay,
                                      int *baseTime,int *baseYear,int *baseMonth,int *baseDay)
{
	time_t t = time(NULL);
    
	if(realTime)    *realTime = (int)t;    
	if(baseTime)    *baseTime = (int)t;    
	FiTimeUtcToHuman(t,realYear,realMonth,realDay,NULL,NULL,NULL);
	FiTimeUtcToHuman(t,baseYear,baseMonth,baseDay,NULL,NULL,NULL);    
    
	return 0;
}

/*
* 更新报警录像时间基游标
*/
static void RefrechAlarmBaseTimeCursor(int channel)
{
	pthread_mutex_lock(&record_control_lock);
	alarm_cursor_time[channel] = SysRunTimeGet();
	pthread_mutex_unlock(&record_control_lock);
}
/*
* 报警录像时间基游标
*/
static int getAlarmBaseTimeCursor(int channel)
{
	int ret;
	pthread_mutex_lock(&record_control_lock);
	ret = alarm_cursor_time[channel];
	pthread_mutex_unlock(&record_control_lock);
	return ret;
}

static RECORD_LED_STATUS g_recLed;
/*
* 打开录像指示灯
*/
static int OpenRecLed(int channel)
{
	int ret = FI_FAIL;
	FiPrint2("ch(%d) OpenRecLed\r\n",channel);
	if(channel>=0 && channel<=REAL_CHANNEL_NUM)
    {
    	FiPrint2("g_recLed.openFlag=0x%X\r\n",g_recLed.openFlag);
    	g_recLed.recStatus[channel] = 1;
    	if(FI_FALSE == g_recLed.openFlag)
        {            
            //
            // TODO !!!
            // 
        	GpioRecordLedEnable();
        	ret = FI_SUCCESS;///OpenLedRecord();    
        	if(FI_SUCCESS == ret) g_recLed.openFlag = FI_TRUE;
        }
    }

	return ret;
}

/*
* 关闭录像指示灯
*/
static int CloseRecLed(int channel)
{
	int i;
	int ret = FI_FAIL;
	int closeFlag = 1;
    
	if(channel>=0 && channel<=REAL_CHANNEL_NUM)
    {
    	g_recLed.recStatus[channel] = 0;
        
    }
	for(i=0;i<REAL_CHANNEL_NUM;i++)
    {
    	if(g_recLed.recStatus[i] != 0)
        {
        	closeFlag = 0;
        	break;
        }
    }
	if(1==closeFlag && FI_TRUE==g_recLed.openFlag)
    {
    //
    // TODO !!!
    //
    ///	ret = CloseLedRecord();
    	GpioRecordLedDisable();
    	g_recLed.openFlag = FI_FALSE; 
    }

	return ret;
}

static int StartH264Stream(int channel)
{
	int ret = FI_SUCCESS;
    
	if(FI_FALSE == g_videoEncodeOpenFlag[channel] )
    {
    	FiPrint2( "StartH264Stream channel(%d) = %d!\r\n", channel, ret );
    	if(ret > 0 ) g_videoEncodeOpenFlag[channel] = FI_TRUE;
    }

	return ret;
}

static void CloseH264Stream(int channel)
{
	if( FI_TRUE == g_videoEncodeOpenFlag[channel])
    {
    	g_videoEncodeOpenFlag[channel] = FI_FALSE;
    }
}

static void *WriteRecordFileThread(void *arg)
{
	static unsigned int stopCount = 0;
	PCP_NODE_T *pRecPcp = NULL;
	int channel = (int)arg;
	time_t utc = 0;
	uint recordType = 0;
	uint writeSize = 0;
	uchar frameType = 0;
	RECORD_THREAD *p_rec = NULL;
	int recordCtlFlag = REC_SLEEP;
	int ret = 0;
    
	SVPrint("ch(%d) WriteRecordFileThread:%d!\r\n",channel,ThreadSelf());
	p_rec = &rec_thread[channel];
	while(p_rec->recordThreadRunFlag) 
    {    
    	recordCtlFlag = GetRecordCtlFlag(channel);
                    
    	switch(recordCtlFlag)
        {
        	case REC_START:    
            	stopCount = 0;
            	recordType = 0;
            	CloseRecLed(channel);
            	StartH264Stream(channel);
            	pRecPcp = RecordPoolRead( channel );
            	if( NULL == pRecPcp )
                {
                	RecordPoolWait( channel );
                }
            	else
                {
                	AnalysisFrameData( channel, pRecPcp, &utc, &recordType, 
                                                    &writeSize, &frameType );

                	FiPrint2( "wait I frame,frameType(%d)!\r\n",
                            	pRecPcp->pcpHead.type);
                	if( FI_FRAME_TYPE_VIDEO_I != pRecPcp->pcpHead.type )
                    {
                    	FiPrint2( "start record but no I frame(%d)!\r\n",
                                        	pRecPcp->pcpHead.type);
                    	RecordPoolFree( pRecPcp );
                    	continue;
                    }    
                	ret = CreateRecordAndIndexFile( channel, utc );
                	if(FI_SUCCESS == ret)
                    {
                    	ResetIndexStruct( channel );
                    	ret = WriteDataToDisk( channel, pRecPcp, writeSize, recordType, utc );
                    	if(FI_SUCCESS == ret)
                        {
                        	SetRecordCtlFlag( channel, REC_WRITE );
                        	OpenRecLed( channel );
                        }    
                    }
                	RecordPoolFree( pRecPcp );                    
                	if(FI_SUCCESS != ret)
                    {
                    	sleep(1);
                    }    
                }
            	break;
        	case REC_WRITE:    
            	RecordPoolWait( channel );
            	while( 1 )
                {
                	if( NULL == (pRecPcp=RecordPoolRead(channel)) )
                    {
                    	break;
                    }                    
                	AnalysisFrameData( channel, pRecPcp, &utc, &recordType, 
                                                    &writeSize, &frameType );
                	ret = WriteDataToDisk( channel, pRecPcp, writeSize, recordType, utc );
                	if( FI_SUCCESS != ret )
                    {
                    	SetRecordCtlFlag( channel, REC_START );
                    }
                	RecordPoolFree( pRecPcp );
                }                
            	break;
        	case REC_SWITCH:
            	stopCount = 0;
            	RecordPoolWait( channel );
            	while( 1 )
                {
                	if( NULL == (pRecPcp=RecordPoolRead(channel)) )
                    {
                    	break;
                    }
                	AnalysisFrameData( channel, pRecPcp, &utc, &recordType,
                                                &writeSize, &frameType );
                    
                	if( FI_FRAME_TYPE_VIDEO_I != pRecPcp->pcpHead.type )
                    {
                    	ret = WriteDataToDisk( channel, pRecPcp, writeSize, recordType, utc );
                    	if( FI_SUCCESS != ret )
                        {
                        	SetRecordCtlFlag( channel, REC_START );
                        }
                    }
                	else 
                    {    
                    	CloseRecordAndIndexFile( channel );
                    	utc = time( NULL );
                    	ret = CreateRecordAndIndexFile( channel, utc );                        
                    	if( FI_SUCCESS == ret )
                        {
                        	ResetIndexStruct( channel );
                        	ret = WriteDataToDisk( channel, pRecPcp, 
                                    	writeSize, recordType, utc );
                        	if( FI_SUCCESS == ret )
                            {
                            	SetRecordCtlFlag( channel, REC_WRITE );
                            }                            
                        }
                    	else //if(FI_SUCCESS != ret)
                        {
                        	SetRecordCtlFlag( channel, REC_START );
                        	Usleep(1000);
                        }    
                    }    
                	RecordPoolFree( pRecPcp );
                }                
            	break;
        	case REC_STOP:
            	while( 1 )
                {
                	pRecPcp = RecordPoolRead( channel );
                	SVPrint( "ql debug -ch(%d), pRecPcp(0x%X), stopCount(%u)!\r\n",
                        	channel, pRecPcp, stopCount );
                	if( NULL == pRecPcp ||
                        ++stopCount >= (MAX_RECORD_DATA_NODE << 1) )
                    {
                    	break; // while( 1 )
                    }
                	AnalysisFrameData( channel, pRecPcp, &utc, &recordType, 
                                                &writeSize, &frameType );
                	ret = WriteDataToDisk( channel, pRecPcp, writeSize, recordType, utc );
                	RecordPoolFree( pRecPcp );
                	if( FI_SUCCESS != ret )
                    {
                    	break; // while( 1 )
                    }                        
                }                
                
            	CloseRecordAndIndexFile( channel );
            	SetRecordCtlFlag( channel, REC_SLEEP );
            	CloseH264Stream( channel );
            	CloseRecLed( channel );
            	break;
        	case REC_SLEEP:
        	default:
            	Usleep( 500000 );
            	break;
        } //switch	        
    }    //while(1)    
	SVPrint("quit ch(%d) WriteRecordFileThread!\r\n",channel);
	return NULL;
}

/*
* 控制录像线程启动录像
*/
static int ControlRecordThreadStartRecord(int channel)
{
	int rec_ctl_flag;
	int ret	= FI_FALSE;
    
	rec_ctl_flag = GetRecordCtlFlag(channel);
	if((REC_START != rec_ctl_flag) && (REC_WRITE != rec_ctl_flag) && (REC_SWITCH != rec_ctl_flag))
    {
    	rec_ctl_flag = REC_START;
    	SetRecordCtlFlag(channel,rec_ctl_flag);
    	ret = FI_TRUE;
    }    

	return ret;
}

/*
* 控制录像线程停止录像
*/
static int ControlRecordThreadStopRecord(int channel)
{
	int rec_ctl_flag = REC_SLEEP;
	int ret	= FI_FALSE;
    
	rec_ctl_flag = GetRecordCtlFlag(channel);
	if((REC_START == rec_ctl_flag) || (REC_WRITE == rec_ctl_flag) || (REC_SWITCH == rec_ctl_flag))
    {
    	rec_ctl_flag = REC_STOP;
    	SetRecordCtlFlag(channel,rec_ctl_flag);
    	ret = FI_TRUE;
    }    

	return ret;
}

static void ManageHandRecord(int channel)
{
	RECORD_HAND record_hand;    
	FiRecGetHandRecordParam(channel,&record_hand);
	if(FI_TRUE == record_hand.recHand)
    {
    	FiRecStartRecord(channel,RECORD_TYPE_HAND);
    }
	else
    {
    	FiRecStopRecord(channel,RECORD_TYPE_HAND);
    }
}

void FiRecGetTimerRecordPolicy(int channel,int weekDay,TIME_WEEK *recTimerPolicy)
{
	if(NULL == recTimerPolicy)
    {
    	SVPrint("error:NULL == recTimerPolicy!\r\n");
    	return;
    }
    *recTimerPolicy = g_rec_timer[channel].timerWeek[weekDay];
}

static int PrintfRecordPolicy(int channel,int weekDay)
{
	int i;
	char buf[MAX_TIME_SEG_NUM][32];
	char displayBuf[128] = {0};
	int len = 0;    

	memset( buf, 0x00, sizeof(buf) );
	for(i=0;i<MAX_TIME_SEG_NUM;i++)
    {
    	snprintf(buf[i],32-1,"%u:%u:%u-%u:%u:%u",
                        	g_rec_timer[channel].timerWeek[weekDay].timeSeg[i].startHour,
                        	g_rec_timer[channel].timerWeek[weekDay].timeSeg[i].startMinute,
                        	g_rec_timer[channel].timerWeek[weekDay].timeSeg[i].startSecond,
                        	g_rec_timer[channel].timerWeek[weekDay].timeSeg[i].stopHour,
                        	g_rec_timer[channel].timerWeek[weekDay].timeSeg[i].stopMinute,
                        	g_rec_timer[channel].timerWeek[weekDay].timeSeg[i].stopSecond);
    	snprintf(&displayBuf[len],128-1,"%s ",buf[i]);
    	len += strlen(buf[i])+1;
    }
	SVPrint("record policy:channel(%d),weekDay(%d),enable(%d),policy( %s)!\r\n",channel,weekDay
            ,g_rec_timer[channel].timerWeek[weekDay].enableFlag,displayBuf);

	return 0;
}

int FiRecSetTimerRecordPolicy( int channel, int weekDay, PARAM_CONFIG_RECORD_PARAM *pRecordParam )
{
	int i;
	int recWeekDay;
	int timeSeg;
    
	if( NULL == pRecordParam )
    {
    	SVPrint("error:NULL == pRecordParam!\r\n");
    	return FI_FAIL;
    }
    
	pthread_mutex_lock(&rec_thread[channel].recordLock);

	recWeekDay = weekDay;//(weekDay+6)%7;
	g_rec_timer[channel].timerWeek[recWeekDay].enableFlag = pRecordParam->recTimer.day[recWeekDay].enableFlag; 
	timeSeg = MAX_DAY_TIME_SEG > MAX_TIME_SEG_NUM ? MAX_TIME_SEG_NUM : MAX_DAY_TIME_SEG;
	for( i = 0; i < timeSeg; i++ )
    {
    	g_rec_timer[channel].timerWeek[recWeekDay].timeSeg[i].startHour     = pRecordParam->recTimer.day[recWeekDay].timeSeg[i].timeStart.hour;
    	g_rec_timer[channel].timerWeek[recWeekDay].timeSeg[i].startMinute	= pRecordParam->recTimer.day[recWeekDay].timeSeg[i].timeStart.minute;
    	g_rec_timer[channel].timerWeek[recWeekDay].timeSeg[i].startSecond	= pRecordParam->recTimer.day[recWeekDay].timeSeg[i].timeStart.second;
    	g_rec_timer[channel].timerWeek[recWeekDay].timeSeg[i].stopHour         = pRecordParam->recTimer.day[recWeekDay].timeSeg[i].timeEnd.hour;
    	g_rec_timer[channel].timerWeek[recWeekDay].timeSeg[i].stopMinute	= pRecordParam->recTimer.day[recWeekDay].timeSeg[i].timeEnd.minute;
    	g_rec_timer[channel].timerWeek[recWeekDay].timeSeg[i].stopSecond	= pRecordParam->recTimer.day[recWeekDay].timeSeg[i].timeEnd.second;
    }    
    
	PrintfRecordPolicy( channel, recWeekDay );
    
	pthread_mutex_unlock( &rec_thread[channel].recordLock );
    
	return FI_SUCCESS;
}

/**************************************************
 对定时录像时间管理
***************************************************/
static void ManageTimerRecordTime(int channel)
{
	int i;
	int rec_flag = FI_FALSE;
	TIME_WEEK week_timer;
	time_t real_time;
	int real_wday,real_hour,real_minute,real_second;
    //int startHour,startMinute,startSecond;
	int cur_day_second,start_day_second,stop_day_second;
    
	real_time = time(NULL);
	FiTimeUtcToWeekDay( real_time, &real_wday );
	FiTimeUtcToHuman( real_time ,NULL, NULL, NULL, &real_hour, &real_minute, &real_second );    
	FiRecGetTimerRecordPolicy(channel,real_wday,&week_timer);
	cur_day_second = real_hour*60*60 + real_minute*60 + real_second;

	if(0 == week_timer.enableFlag)
    {
    	rec_flag = FI_FALSE;
    }
	else
    {        
    	for(i=0;i<MAX_TIME_SEG_NUM;i++)
        {
        	start_day_second = week_timer.timeSeg[i].startHour*60*60 +
                         week_timer.timeSeg[i].startMinute*60 +
                         week_timer.timeSeg[i].startSecond;
        	stop_day_second = week_timer.timeSeg[i].stopHour*60*60 +
                         week_timer.timeSeg[i].stopMinute*60 +
                         week_timer.timeSeg[i].stopSecond;
    //    	FiPrint2("wday=%d:%02dh%02dm%02ds-%02dh%02dm%02ds\r\n",real_wday,
    //	week_timer.timeSeg[i].startHour,week_timer.timeSeg[i].startMinute,week_timer.timeSeg[i].startSecond,
    //	week_timer.timeSeg[i].stopHour,week_timer.timeSeg[i].stopMinute,week_timer.timeSeg[i].stopSecond);
        	if(start_day_second < stop_day_second && start_day_second <= cur_day_second && cur_day_second <= stop_day_second)
            {                
            	rec_flag = FI_TRUE;
            	break;
            }
        }
    }
	if(FI_TRUE == rec_flag)
    {        
    	FiRecStartRecord(channel,RECORD_TYPE_TIMER);
    }    
	else
    {
    	FiRecStopRecord(channel,RECORD_TYPE_TIMER);
    }
}

/*********************************************************
  录像监视线程,该线程与系统时间有关,
  请在启动应用线程后再启动该线程
  并在启动此次线程再启动录像线程
***********************************************************/
static void *ManageRecordThread(void *arg)
{
	int chNum = REAL_CHANNEL_NUM;
	int i,j;
	int real_time[chNum];
	int base_time[REAL_CHANNEL_NUM] = {-1};
	int real_year[chNum],real_month[chNum],real_day[chNum];
	int base_year[chNum],base_month[chNum],base_day[chNum];
	uint base_rec_type[REAL_CHANNEL_NUM];
	uint real_rec_type[REAL_CHANNEL_NUM];    
	int alarm_real_time[REAL_CHANNEL_NUM];
	int alarm_base_time[REAL_CHANNEL_NUM];    
	char real_parh[RECORD_FILENAME_LEN] = {0};
	char base_path[RECORD_FILENAME_LEN] = {0};

	SVPrint("start ManageRecordThread:%d!\r\n",ThreadSelf());
    
	while(rec_manage.recordManageRunFlag)
    {
    	FiRecStartAutoDelOldFile();
        /*录像分区改变切换文件,优先级最高*/
    	if(FI_FAIL == FiHddGetUsingPartition(real_parh))
        {
        	for(i=0;i<chNum;i++)
            {
            	if(REC_SLEEP != GetRecordCtlFlag(i))
                	SetRecordCtlFlag(i,REC_STOP);
            }
        	sleep(1);
        	continue;
        }
    	else
        {
        	if(0 != strcmp(base_path,real_parh))
            {
            	strcpy(base_path,real_parh);
            	for(i=0;i<chNum;i++)
                {
                	if(REC_WRITE == GetRecordCtlFlag(i))
                    {
                    	SVPrint("record switch partition.\r\n");    
                    	RefrechAlarmBaseTimeCursor(i);
                    	RefrechTimerBaseTimeCursor(&real_time[i],&real_year[i],&real_month[i],&real_day[i],
                                        &base_time[i],&base_year[i],&base_month[i],&base_day[i]);
                    	SetRecordCtlFlag(i,REC_SWITCH);    //仅仅录像分区,不做任何时间检测
                    }
                }
            }
        }
        
    	if(-1 == manage_init_flag)
        {            
            
        	for(j=0;j<chNum;j++)
            {
                /*初始化定时录像监测*/
            	real_time[j] = time(NULL);
            	base_time[j] = real_time[j];
            	FiTimeUtcToHuman(base_time[j],&base_year[j],&base_month[j],&base_day[j],NULL,NULL,NULL);

                /*初始化录像类型切换监测*/            
            	real_rec_type[j] = GetRecordType(j);
            	base_rec_type[j] = real_rec_type[j];

            	RefrechAlarmBaseTimeCursor(j);
            	alarm_real_time[j] = SysRunTimeGet();
            }
            
        	manage_init_flag = 0;
        }        
        
        /*监测手动和定时录像*/
    	for(i=0;i<chNum;i++)
        {
        	ManageHandRecord(i);
        	ManageTimerRecordTime(i);            
        }
        
        /*定时录像切文件*/
    	for(i=0;i<chNum;i++)
        {            
        	real_time[i] = time(NULL);
        	FiTimeUtcToHuman(real_time[i],&real_year[i],&real_month[i],&real_day[i],NULL,NULL,NULL);
        	if(( real_time[i] - base_time[i] >= FiRecGetRecFileSwitchTime() ) || 
                (real_time[i] - base_time[i] < 0 && base_time[i] - real_time[i] > RECORD_TIME_ERROR_RANGE)    || 
                (real_year[i] != base_year[i]) || (real_month[i] != base_month[i]) || (real_day[i] != base_day[i]))
            {
            	SVPrint("ch(%d),%d-%d=%d\r\n",i,real_time[i],base_time[i],real_time[i]-base_time[i]);    
            	RefrechAlarmBaseTimeCursor(i);
            	RefrechTimerBaseTimeCursor(&real_time[i],&real_year[i],&real_month[i],&real_day[i],
                                    &base_time[i],&base_year[i],&base_month[i],&base_day[i]);
            	if(REC_WRITE == GetRecordCtlFlag(i))
                {                        
                	SetRecordCtlFlag(i,REC_SWITCH);                    
                }    
            }
        } 
        /*报警录像情况的录像类型改变从而切换文件*/
    	for(i=0;i<chNum;i++)
        {
        	real_rec_type[i] = GetRecordType(i);            
        	if(real_rec_type[i] & RECORD_TYPE_ALARM_ALL)
            {                
            	alarm_base_time[i] = getAlarmBaseTimeCursor(i);
            	alarm_real_time[i] = SysRunTimeGet();
            	if( alarm_real_time[i] - alarm_base_time[i] >= FiRecGetRecFileSwitchTimeAlarm() )
                {
                	SVPrint("%d-%d=%d\r\n",alarm_real_time[i] , alarm_base_time[i],alarm_real_time[i] - alarm_base_time[i]);
                	FiRecStopRecord(i,RECORD_TYPE_ALARM_ALL);
                	RefrechAlarmBaseTimeCursor(i);                        
                	RefrechTimerBaseTimeCursor(&real_time[i],&real_year[i],&real_month[i],&real_day[i],
                                    &base_time[i],&base_year[i],&base_month[i],&base_day[i]);                
                	if(real_rec_type[i] & (RECORD_TYPE_ALL&(~RECORD_TYPE_ALARM_ALL)))                    
                    {
                    	SetRecordCtlFlag(i,REC_SWITCH);                            
                    }    
                }
            }            
        }	

        /* 控制录像线程启动录像 ,是否启动录像*/
    	for(i=0;i<chNum;i++)
        {
        	real_rec_type[i] = GetRecordType(i);
        	if(real_rec_type[i] & RECORD_TYPE_ALL)
            {
            	if(FI_TRUE == ControlRecordThreadStartRecord(i))
                {
                	RefrechAlarmBaseTimeCursor(i);                        
                	RefrechTimerBaseTimeCursor(&real_time[i],&real_year[i],&real_month[i],&real_day[i],
                                    &base_time[i],&base_year[i],&base_month[i],&base_day[i]);
                }
            }
        	else
            {
            	ControlRecordThreadStopRecord(i);
            }
        }    
        
    	Usleep(500000); //500ms
    }    //while(rec_manage.recordManageRunFlag)
    
	SVPrint("quit ManageRecordThread!\r\n");
    
	return NULL;
}

/**************************************************
* 启动录像模块的所有线程
***************************************************/
void FiRecStartRecordService(void)
{
	int chNum = REAL_CHANNEL_NUM;
	int i;
    
	FiRecRecordInit();
	StartRecordGetH264Thread(); 
    
	for(i=0;i<chNum;i++)
    {
    	rec_thread[i].channel = i;
    	rec_thread[i].recordThreadRunFlag = 1;
    	SetRecordCtlFlag(i,REC_SLEEP); //由监视线程启动录像
        
    	ThreadCreateCommonPriority(&rec_thread[i].recordThreadId,
                        	WriteRecordFileThread,
                            (void *)rec_thread[i].channel );
    }    
    
	rec_manage.recordManageRunFlag = 1;    
	ThreadCreateCommonPriority(    &rec_manage.recordManageId,
                            	ManageRecordThread,
                            	NULL );

    //StartRecordGetH264Thread();
    
	return;
}

/**********************************************************
* 停止录像模块的所有线程
***********************************************************/
void FiRecStopRecordService(void)
{
	int chNum = REAL_CHANNEL_NUM;
	int i;    
    
	rec_manage.recordManageRunFlag = 0;    
	if(0 != ThreadJoin(rec_manage.recordManageId,NULL))
    {
    	SVPrint("error:ThreadJoin(rec_manage.recordManageId,NULL)\r\n");
    }

	for(i=0;i<chNum;i++)
    {
    	if(REC_SLEEP != GetRecordCtlFlag(i))
        {
        	SetRecordCtlFlag(i,REC_STOP);            
        	RecordPoolSignal( i );
        }
    }
	for( i=0; i<chNum; i++ )
    {
    	while(REC_SLEEP != GetRecordCtlFlag(i))
        {
        	Usleep(50);
        }
    }
    
	for( i=0; i<chNum; i++ )
    {
    	rec_thread[i].recordThreadRunFlag = 0;            
    }
    
	for( i=0; i<chNum; i++ )
    {
    	if( 0 != ThreadJoin( rec_thread[i].recordThreadId, NULL) )
        {
        	SVPrint("ThreadJoin(rec_thread[%d].recordThreadId,NULL)\r\n",i);
        }
    }
    
	StopRecordGetH264Thread();
	FiRecDelRecordInit();
	return;
}



// 为删除录像而停止录像服务
void RecordStopServiceForDel()
{
	int chNum = REAL_CHANNEL_NUM;
	int i;
	rec_manage.recordManageRunFlag = 0;    
	if(0 != ThreadJoin(rec_manage.recordManageId,NULL))
    {
    	SVPrint("error:ThreadJoin(rec_manage.recordManageId,NULL)\r\n");
    }

	for(i=0;i<chNum;i++)
    {
    	if(REC_SLEEP != GetRecordCtlFlag(i))
        {
        	SetRecordCtlFlag( i, REC_STOP );
        	RecordPoolSignal( i );
        }
    }
    
	for(i=0;i<chNum;i++)
    {
    	while(REC_SLEEP != GetRecordCtlFlag(i))
        {
        	Usleep(50);
        }
    }
    
	sync();    
}

// 为删除录像而重启录像服务
void RecordRestartServiceForDel()
{
	rec_manage.recordManageRunFlag = 1;    
	ThreadCreateCommonPriority( &rec_manage.recordManageId,
                	ManageRecordThread,
                	NULL );
}

/***************************************************
* 录像初始化
**************************************************/
void FiRecRecordInit(void)
{
	int chNum = REAL_CHANNEL_NUM;
	int i, j, ret;
	PARAM_CONFIG_RECORD_PARAM recParam;
    
	pthread_mutexattr_t mutexattr;
    
	pthread_mutexattr_init( &mutexattr );
	pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_RECURSIVE_NP );        
	for( i=0; i<chNum; i++ )
    {
    	pthread_mutex_init( &rec_thread[i].recordLock, &mutexattr );        
    }
	pthread_mutex_init( &record_control_lock, &mutexattr );

	for(i=0;i<MAX_CHANNEL_NUM;i++)
    {
    	g_rec_handle[i] = -1;
    	g_index_handle[i] = -1;
    }
    
	for( i=0; i<REAL_CHANNEL_NUM; i++ )
    {            
    	ret = ParamGetRecordParam( i, &recParam );
    	if( 0 == ret )
        {
        	FiRecSetHandRecord( i, &recParam );
        	for( j = 0; j < 7; j++ )
            {
            	FiRecSetTimerRecordPolicy( i, j, &recParam );
            }
        }
    }    
    
	manage_init_flag = -1 ; //让线程支持重入	
	memset(g_videoEncodeOpenFlag,0x00,sizeof(g_videoEncodeOpenFlag));

    // FiRecInitLoopRecordFlag();
    FiRecInitPublic();
    
	pthread_mutexattr_destroy(&mutexattr);
}

void FiRecInitPublic(void)
{
	int ret;
	PARAM_CONFIG_RECORD_PUBLIC param;
    
	ret = ParamGetRecordPublic( &param );
	if(0 == ret) 
    {
    	ret = FiRecSetSupportLoopRecordFlag( param.loopRecord );
    	ret = FiRecSetRecordDelSpace( param.delSpace );
        ret = FiRecSetRecFileSwitchTime( param.switchFileTime );
        SVPrint(" record switchFileTime(%u), delSpace(%u), loopRecord(%u)!\r\n",
                param.switchFileTime, param.delSpace, param.loopRecord);
    }
}

/*
* 录像去初始化
*/
void FiRecDelRecordInit(void)
{
	int chNum = REAL_CHANNEL_NUM;
	int i;
    
	for(i=0;i<chNum;i++)
    {
    	pthread_mutex_destroy(&rec_thread[i].recordLock);
    	CloseRecLed(i);
    }
	pthread_mutex_destroy(&record_control_lock);

	FiPrint2("FiRecDelRecordInit() end!\r\n");
}

/*
* 检测录像类型,为startOneKindRecord和stopOneKindRecord服务
*/
static int CheckRecordStartUpType(uint recType)
{
	int count = 0;
	int i;

	for(i=0;i<MAX_SUPPORT_RECORD_TYPE_NUM;i++)
    {
    	if(recType & 0x01)
        	count++;
    	recType >>= 1;
    }
	if(1 == count)
    	return FI_SUCCESS;
	else
    	return FI_FAIL;
}

/*
* 启动录像
* recType:录像类型,支持所有录像的中的其中一种
*/
static void StartOneKindRecord(int channel,uint recType)
{
	uint recordType;
	if(FI_FAIL == CheckRecordStartUpType(recType))
    {
    	SVPrint("failed ch(%d) FI_FAIL == CheckRecordStartUpType(recType)!\r\n",channel);
    	return;
    }
	recordType = GetRecordType(channel);
	if(!(recordType & recType)) //原来没有那种录像	
    {
    	recordType |= recType;
    	SetRecordType(channel,recordType);
    }    
    //刷新时间游标
	if(recType & RECORD_TYPE_ALARM_ALL)
    {
    	RefrechAlarmBaseTimeCursor(channel);
    }
    //Usleep(1); //just for test 有必要添加吗?
}

/*
* 启动录像
* recType:录像类型(以RECORD_TYPE_为前缀)的一种或多种的或组合
*/
int FiRecStartRecord(int channel,uint recType)
{
	int recordType = recType & RECORD_TYPE_ALL;
	int i,ret = FI_SUCCESS;
	char hddPath[128];

	if(FI_SUCCESS != FiHddGetUsingPartition(hddPath))
    {
    	ret = FI_FAIL;
    }
	if(FI_SUCCESS == ret)
    {
    	if( RECORD_TYPE_HAND == recType) FiRecStartHandRecord(channel);
    	for(i=0; i < MAX_SUPPORT_RECORD_TYPE_NUM; i++)
        {
        	if(all_rec_type[i] & recordType)
            {
            	StartOneKindRecord(channel,all_rec_type[i]);
            }
        }
    }

	return ret;
}

/*
* 停止录像
* recType:录像类型,支持所有录像的中的其中一种
*/
static void StopOneKindRecord(int channel,uint recType)
{
	uint recordType;
	if(FI_FAIL == CheckRecordStartUpType(recType))
    {
    	SVPrint("failed ch(%d) FI_FAIL == CheckRecordStartUpType(recType)!\r\n",channel);
    	return;
    }
	recordType = GetRecordType(channel);
	if(recordType & recType) //原来有那种录像	
    {
    	recordType &= (~recType);
    	SetRecordType(channel,recordType);
    }
}

/*
* 停止录像,可重入,对同一种录像类型可以多次调用该函数,不影响录像结果
* recType:录像类型的一种或多种的或组合
*/
int FiRecStopRecord(int channel,uint recType)
{
	int recordType = recType & RECORD_TYPE_ALL;
	int i;

	if( RECORD_TYPE_HAND == recType) //红外手动录像
    {
    	FiRecStopHandRecord(channel);
    }

	for(i=0;i<MAX_SUPPORT_RECORD_TYPE_NUM;i++)
    {
    	if(all_rec_type[i] & recordType)
        {
        	StopOneKindRecord(channel,all_rec_type[i]);
        }
    }    

	return FI_SUCCESS;
}

int FiRecIsRecording()
{
	int i;
	int recFlag = FI_FALSE;
	int ctlFlag = REC_SLEEP;
    
	for(i=0;i<REAL_CHANNEL_NUM;i++)
    {
    	ctlFlag = GetRecordCtlFlag(i);
    	if(REC_SLEEP != ctlFlag)
        {
        	recFlag = FI_TRUE;
        	break;
        }
    }

	return recFlag;
}

int FiRecIsHandLeRecording(int channel)
{
	uint recType = GetRecordType(channel);
	int ctlFlag = GetRecordCtlFlag(channel);

	SVPrint("recType(0x%X) ,ctlFlag(%d)\r\n", recType, ctlFlag);
	if( REC_SLEEP != ctlFlag && (RECORD_TYPE_HAND & recType) )
    	return FI_TRUE;
	else
    	return FI_FALSE;    
}

