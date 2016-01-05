/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**	文件名: recordList.cpp
**  description  : 管理录像缓存列表
**  date           :  2013.10.18
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#include <stdio.h>
#include <sys/time.h>
#include <pthread.h>
#include "const.h"
#include "debug.h"
#include "thread.h"
#include "malloc.h"
#include "recordList.h" 
#include "hicomm.h" 

static	RECORD_LIST	*g_mem_list[] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
                    	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

static RECORD_MEM **g_ch_mb;
unsigned long *g_masterframeNo = NULL;
unsigned long *g_slaveframeNo = NULL;
static pthread_mutex_t *g_sendstream_mutex;

/*
* 初始化录像缓冲列表
*/
void FiRecMemListInit(int channel)
{
	int	i;
	RECORD_MEM	*mb;
	pthread_mutexattr_t mutexattr;

	g_mem_list[channel] = (RECORD_LIST *)Malloc(sizeof(RECORD_LIST));
	g_mem_list[channel]->freememHead = NULL;
	g_mem_list[channel]->freememTail = NULL;
	g_mem_list[channel]->usedmemHead = NULL;
	g_mem_list[channel]->usedmemTail = NULL;
	g_mem_list[channel]->usedListFrameCount = 0;
	for (i = 0; i < MAX_FRAME_NODE; i++)
    {
    	mb = (RECORD_MEM *)Malloc(sizeof(RECORD_MEM));
    	if (mb == NULL)
        {
        	perror("Mem_init() error");
        	continue;
        }
    	bzero(mb, sizeof(RECORD_MEM));
    	mb->next = NULL;
    	if (g_mem_list[channel]->freememHead == NULL)
        {
        	g_mem_list[channel]->freememHead = mb;
        }
    	else
        {
        	g_mem_list[channel]->freememTail->next = mb; 
        }
    	g_mem_list[channel]->freememTail = mb;
    }
	pthread_mutexattr_init(&mutexattr);
	pthread_mutexattr_settype(&mutexattr,PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init(&g_mem_list[channel]->freeListMutex,&mutexattr);
	pthread_mutex_init(&g_mem_list[channel]->usedListMutex,&mutexattr);
	pthread_mutexattr_destroy(&mutexattr);
	pthread_cond_init(&g_mem_list[channel]->condVar,NULL);    

}

/*
* 去初始化录像缓冲列表
*/
void FiRecMemListDestroy(int channel)
{
	RECORD_MEM	*tmp;

	if (g_mem_list[channel] == NULL)
    	return;
    
	tmp = g_mem_list[channel]->freememHead;
	while(tmp)
    {
    	g_mem_list[channel]->freememHead = tmp->next;
    	Free(tmp);
    	tmp = g_mem_list[channel]->freememHead;
    }
	g_mem_list[channel]->freememHead = NULL;
	g_mem_list[channel]->freememTail = NULL;
	tmp = g_mem_list[channel]->usedmemHead;
	while (tmp)
    {
    	g_mem_list[channel]->usedmemHead = tmp->next;
    	Free(tmp->fb.frameData);
    	Free(tmp);
    	tmp = g_mem_list[channel]->usedmemHead;
    }
	g_mem_list[channel]->usedmemHead = NULL;
	g_mem_list[channel]->usedmemTail = NULL;
	pthread_mutex_destroy(&g_mem_list[channel]->usedListMutex);
	pthread_mutex_destroy(&g_mem_list[channel]->freeListMutex);
	pthread_cond_destroy(&g_mem_list[channel]->condVar);

	Free(g_mem_list[channel]);
	g_mem_list[channel] = NULL;    
}

/*
* 从空闲缓冲列表中取出一个节点,缓冲一帧数据
* size:帧数据的大小
* 返回:该节点的地址
*/
RECORD_MEM *FiRecGetMbFromFreeList(int channel, int size)
{
	RECORD_MEM *mb;
	unsigned char *buf;
	int	i;
	int	frameSize = 0;

	if(size%sizeof(int) != 0)
    	frameSize = size + (4-size%sizeof(int));
	else
    	frameSize = size;
	while(1)
    {
    	buf = (unsigned char *)Malloc(frameSize);        
    	if(buf != NULL)
        {
        	pthread_mutex_lock(&g_mem_list[channel]->freeListMutex);
        	if( g_mem_list[channel]->freememHead != NULL)
            {
            	mb = g_mem_list[channel]->freememHead;
            	g_mem_list[channel]->freememHead = g_mem_list[channel]->freememHead->next;
            	if(g_mem_list[channel]->freememHead == NULL)
                	g_mem_list[channel]->freememTail = NULL;
            	mb->next = NULL;
            	mb->fb.frameData = buf;
            	pthread_mutex_unlock(&g_mem_list[channel]->freeListMutex);
            	return(mb);
            }
        	pthread_mutex_unlock(&g_mem_list[channel]->freeListMutex);
            
            //如果freemem_list为空,则从usedmem_list的头取Mb.
        	pthread_mutex_lock(&g_mem_list[channel]->usedListMutex);
        	if (g_mem_list[channel]->usedmemHead != NULL)
            {

            	mb = g_mem_list[channel]->usedmemHead;
            	Free(mb->fb.frameData);
            	g_mem_list[channel]->usedmemHead = g_mem_list[channel]->usedmemHead->next;
            	if(g_mem_list[channel]->usedmemHead == NULL)
                	g_mem_list[channel]->usedmemTail = NULL;
            	mb->next = NULL;
            	mb->fb.frameData = buf;
            	pthread_mutex_unlock(&g_mem_list[channel]->usedListMutex);
            	return(mb);
            }
        	else
            {
            	SVPrint("error:recordList get node failed!\r\n");
            	Free(buf);
            	pthread_mutex_unlock(&g_mem_list[channel]->usedListMutex);
            	break;
            }

        	pthread_mutex_unlock(&g_mem_list[channel]->usedListMutex);
        }
    	else
        {
        	for(i = 0; i< 5; i++)
            {
            	pthread_mutex_lock(&g_mem_list[channel]->usedListMutex);
            	if (g_mem_list[channel]->usedmemHead != NULL)
                {
                	mb = g_mem_list[channel]->usedmemHead;
                	g_mem_list[channel]->usedmemHead = g_mem_list[channel]->usedmemHead->next;
                	if(g_mem_list[channel]->usedmemHead == NULL)
                    	g_mem_list[channel]->usedmemTail = NULL;
                	mb->next = NULL;
                	pthread_mutex_unlock(&g_mem_list[channel]->usedListMutex);
                	FiRecInsertMbToFreeList(channel, mb);
                }
            	else
                {
                	pthread_mutex_unlock(&g_mem_list[channel]->usedListMutex);
                	return (NULL);
                }
            }        
        }
    } //end while(1)
	return(NULL);
}

/*
* 把用完的节点还给"未用缓冲列表",供下次使用
* mb: 要还得节点地址
*/
void FiRecInsertMbToFreeList(int channel, RECORD_MEM *mb)
{
	if(NULL == mb)
    {
    	SVPrint("error:NULL == mb!\r\n");
    	return;
    }
	pthread_mutex_lock(&g_mem_list[channel]->freeListMutex);
	Free(mb->fb.frameData);
	if(g_mem_list[channel]->freememHead == NULL)
    	g_mem_list[channel]->freememHead = mb;
	else
    	g_mem_list[channel]->freememTail->next = mb;
	g_mem_list[channel]->freememTail = mb;

	pthread_mutex_unlock(&g_mem_list[channel]->freeListMutex);
}

/*
* 从已经使用的缓冲列表中取出一个节点
* 返回:所取节点的地址
*/
RECORD_MEM *FiRecGetMbFromUsedList(int channel)
{
	RECORD_MEM	*mb = NULL;

	pthread_mutex_lock(&g_mem_list[channel]->usedListMutex);
	if (g_mem_list[channel]->usedmemHead != NULL)
    {
    	mb = g_mem_list[channel]->usedmemHead;
    	g_mem_list[channel]->usedmemHead = g_mem_list[channel]->usedmemHead->next;
    	if(g_mem_list[channel]->usedmemHead == NULL)
        	g_mem_list[channel]->usedmemTail = NULL;
    	mb->next = NULL;
    	g_mem_list[channel]->usedListFrameCount--; //qljt
    	pthread_mutex_unlock(&g_mem_list[channel]->usedListMutex);
    	return(mb);
    }
	pthread_mutex_unlock(&g_mem_list[channel]->usedListMutex);
	return(NULL);
}

/*
* 往已经使用的缓冲列表中插入一个节点
* mb:要插入的节点
*/
void FiRecInsertMbToUsedList(int channel, RECORD_MEM *mb)
{
	if(NULL == mb)
    {
    	SVPrint("error:NULL == mb!\r\n");
    	return;
    }
	mb->next = NULL;
	pthread_mutex_lock(&g_mem_list[channel]->usedListMutex);
	if (g_mem_list[channel]->usedmemHead == NULL)
    {
    	g_mem_list[channel]->usedmemHead = mb;
    }
	else
    	g_mem_list[channel]->usedmemTail->next = mb;
	g_mem_list[channel]->usedmemTail = mb;
	g_mem_list[channel]->usedListFrameCount++; 
	pthread_mutex_unlock(&g_mem_list[channel]->usedListMutex);
}

/*
* qljt 获取缓冲列表里面的帧数
* 用来支持预录了
*/
unsigned int FiRecGetUsedListFrameCount(int channel)
{
	unsigned int count = 0;
	pthread_mutex_lock(&g_mem_list[channel]->usedListMutex);
	count = g_mem_list[channel]->usedListFrameCount;
	pthread_mutex_unlock(&g_mem_list[channel]->usedListMutex);
	return count;
}

/*
* 判断空闲缓冲列表是否已经用完了
*/
int FiRecIsFreeListNULL(int channel)
{
	int flag = 0;
	pthread_mutex_lock(&g_mem_list[channel]->freeListMutex);
	if (g_mem_list[channel]->freememHead == NULL)
    {
    	flag = 1;
    }
	pthread_mutex_unlock(&g_mem_list[channel]->freeListMutex);
	return flag;
}


int FiRecInitRecordStream()
{
	int i;
	pthread_mutexattr_t mutexattr;
	int channelNum;

	channelNum = REAL_CHANNEL_NUM;
        
	g_ch_mb = (RECORD_MEM **)Malloc(sizeof(*g_ch_mb)* channelNum);
	bzero(g_ch_mb, sizeof(*g_ch_mb)*channelNum);

	g_masterframeNo = (unsigned long*)Malloc(sizeof(*g_masterframeNo) * channelNum);
	bzero(g_masterframeNo, sizeof(*g_masterframeNo)*channelNum);

	g_slaveframeNo = (unsigned long*)Malloc(sizeof(*g_slaveframeNo) * channelNum);
	bzero(g_slaveframeNo, sizeof(*g_slaveframeNo) * channelNum);

	g_sendstream_mutex = (pthread_mutex_t *)Malloc(sizeof(*g_sendstream_mutex) * channelNum);
	pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr,PTHREAD_MUTEX_TIMED_NP);
	for(i=0; i<channelNum; i++)
    {
    	FiRecMemListInit(i);
    	pthread_mutex_init(&g_sendstream_mutex[i],&mutexattr);
    }
    pthread_mutexattr_destroy(&mutexattr);

    

	SVPrint("record stream init finish\r\n");
    
	return 0;
}

void FiRecDestroyRecordStream(void)
{
	int i;
	int channelNum;

	channelNum = REAL_CHANNEL_NUM;
    
	for(i=0; i<channelNum; i++)
    {        
    	FiRecMemListDestroy(i);        
    	pthread_mutex_destroy(&g_sendstream_mutex[i]);
    }
    
	Free(g_ch_mb);

	return;
}

/*
* fn: 为了使录像中每个文件开头的第一帧都是I 帧, 需要对写入模块的帧做适配
* 返回: 1, 该帧适配; 0, 该帧不适配
*/
static int IsNeedFrame( int curState, uint totalSize, int frameType )
{
	if( 1 == curState ) // 需要的是p 帧
    {
    	return 1;
    }
	else // 需要的是I 帧
    {
    	if( totalSize != 0 ) // 已经请求过I 帧了
        {
        	return 1;
        }
    	else // 还没有请求过I 帧
        {
        	if( H264E_NALU_PSLICE == frameType ) // 本帧是p 帧
            {
            	return 0;
            }
        	else // 本帧是i 帧
            {
            	return 1;
            }
        }
    }
}

/*
* 存放录像缓冲池
* channel:通道
* ch_type:0-主码流;1-子码流
* pFrame:录像数据
*/
int FiRecSendFrameToRecordStreamListEx(int channel, int ch_type, void *pRecordFrame)
{        
	static RECORD_FRAME_HEAD vframeHead[REAL_CHANNEL_NUM];            
	unsigned int	size = 0;
	struct	timeval tv; 
	int ret = FI_FAILED;
	int realFrameType;
	const unsigned int RECORD_LIST_MIX_FRAME_LEN = 256 * 1024;    
	const unsigned int curFrameLen = RECORD_LIST_MIX_FRAME_LEN + sizeof(vframeHead[channel]);    
	RECORD_LIST_FRAME *pStream = (RECORD_LIST_FRAME*)pRecordFrame;    
	static NEED_FRAME_STATUS_T needFrameStatus[REAL_CHANNEL_NUM];

	if(NULL == pRecordFrame)
    {
    	SVPrint("error:NULL == pRecordFrame!\r\n");
    	return FI_FAILED;
    }
	pthread_mutex_lock(&g_sendstream_mutex[channel]);
	if(pStream->frameLen > 0)
    {            
    	gettimeofday(&tv, NULL);
            
    	if(pStream->frameLen >= MAX_FRAME_SIZE)
        {
        	SVPrint("video frame is %dk too len, and lost it!\r\n", curFrameLen/1024);
        	ret = FI_FAILED;
        	goto _errReturn;
        }        

    	if( NULL == g_ch_mb[channel] )
        {                    
        	FiPrint2( "send record pool no NULL == g_ch_mb[%d]!\r\n", channel );
        	memset( &vframeHead[channel], 0x00, sizeof(vframeHead[channel]) );
        	g_ch_mb[channel] = FiRecGetMbFromFreeList(channel, curFrameLen);
        	if(NULL == g_ch_mb[channel])
            {
            	SVPrint("send record pool no free mb drop frame!\r\n");
            	ret = FI_FAILED;
            	goto _errReturn;
            }
        }
    	else
        {    
        	if( 0 == needFrameStatus[channel].state )
            {
            	if( (vframeHead[channel].frameSize + pStream->frameLen) > RECORD_LIST_MIX_FRAME_LEN )
                {
                	FiRecInsertMbToUsedList(channel, g_ch_mb[channel]); 
                	FiRecRecordStreamBroadcast(channel);
                    
                	memset( &vframeHead[channel], 0x00, sizeof(vframeHead[channel]) );
                	g_ch_mb[channel] = FiRecGetMbFromFreeList(channel, curFrameLen);
                	needFrameStatus[channel].state = 1;
                }
            }
        	else
            {
            	if( (vframeHead[channel].frameSize + pStream->frameLen) > (RECORD_LIST_MIX_FRAME_LEN >> 1)
                    && pStream->frameType != H264E_NALU_PSLICE ) // TODO 注意: 这里有一个潜在的风险
                {
                	FiRecInsertMbToUsedList(channel, g_ch_mb[channel]); 
                	FiRecRecordStreamBroadcast(channel);
                    
                	memset( &vframeHead[channel], 0x00, sizeof(vframeHead[channel]) );
                	g_ch_mb[channel] = FiRecGetMbFromFreeList(channel, curFrameLen);
                	needFrameStatus[channel].state = 0;
                }                        
            }

        	if(NULL == g_ch_mb[channel])
            {
            	SVPrint("get mb is NULL\r\n");
            	ret = FI_FAILED;
            	goto _errReturn;
            }            
        }    
        
    	ret = IsNeedFrame( needFrameStatus[channel].state, 
            	vframeHead[channel].frameSize, pStream->frameType );
    	if( 1 != ret )
        {
        	ret = FI_FAILED;
        	goto _errReturn;
        }
        
    	size = vframeHead[channel].frameSize + sizeof(vframeHead[channel]);     
    	memcpy(&g_ch_mb[channel]->fb.frameData[size],pStream->frameData,pStream->frameLen);                    
    
    	vframeHead[channel].frameIdentify = FIRSCOM_FRAME_IDENTIFY;
    	vframeHead[channel].FrameHeadSize = sizeof(RECORD_FRAME_HEAD);
    	vframeHead[channel].frameSize += pStream->frameLen;
    	if(0 == ch_type)
        	vframeHead[channel].frameNo = g_masterframeNo[channel]++;
    	else
        	vframeHead[channel].frameNo = g_slaveframeNo[channel]++;
        
    	vframeHead[channel].videoResolution = pStream->videoResolution;
    	realFrameType = needFrameStatus[channel].state == 1? H264E_NALU_PSLICE : H264E_NALU_ISLICE;
    	vframeHead[channel].frameType = realFrameType;
    	vframeHead[channel].frameRate = pStream->frameRate;
    	vframeHead[channel].videoStandard = pStream->videoStandard;
    	vframeHead[channel].sec = tv.tv_sec;
    	vframeHead[channel].usec = tv.tv_usec;    
    	vframeHead[channel].pts = pStream->pts;         
        //memcpy(&g_ch_mb[channel]->fb.frameData[0], &vframeHead[channel], sizeof(vframeHead[channel]));
    	memcpy(&g_ch_mb[channel]->fb.frameHead, &vframeHead[channel], sizeof(vframeHead[channel]));

    	ret = FI_SUCCESSFUL;
    }

_errReturn:
        
	pthread_mutex_unlock(&g_sendstream_mutex[channel]);
    
	return ret;
}

void FiRecRecordStreamBroadcast(int channel)
{
	RECORD_LIST *record_stream = g_mem_list[channel];

	pthread_mutex_lock(&record_stream->usedListMutex);
	pthread_cond_broadcast(&record_stream->condVar);
	pthread_mutex_unlock(&record_stream->usedListMutex);
}

typedef void (*pthread_cleanup_push_routine_t)(void*);
void FiRecRecordStreamWaiting(int channel)
{
	RECORD_LIST *record_stream = g_mem_list[channel];
	struct timespec timeout;
	struct timeval curtime;
    
	if(gettimeofday( &curtime, NULL) !=0 ) 
    {
    	SVPrint("Fail to get current time\n");
    	return;
    }
	timeout.tv_sec = curtime.tv_sec;
	timeout.tv_nsec = curtime.tv_usec * 1000 + 50*1000*1000;

	if(timeout.tv_nsec >= 1000*1000*1000)
    {
    	timeout.tv_sec += 1;
    	timeout.tv_nsec %= 1000*1000*1000;
    }    
	pthread_cleanup_push((pthread_cleanup_push_routine_t)pthread_mutex_unlock, (void *)&(record_stream->usedListMutex));
	pthread_mutex_lock(&record_stream->usedListMutex);
    // pthread_cond_timedwait(&record_stream->condVar,&record_stream->usedListMutex,&timeout);    
	pthread_cond_wait( &record_stream->condVar, &record_stream->usedListMutex );    
	pthread_mutex_unlock(&record_stream->usedListMutex);
	pthread_cleanup_pop(0);    
}

RECORD_MEM* FiRecGetFrameFromRecordStreamList(int channel)
{
	return FiRecGetMbFromUsedList(channel);
}

