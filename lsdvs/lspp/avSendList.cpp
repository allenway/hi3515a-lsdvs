/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此文件实现了对音视频发送缓冲区进行操作的相关函数
**  date           :  2013.11.11
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#include "debug.h"
#include "const.h"
#include "malloc.h"
#include "mutex.h"
#include "condition.h"
#include "ringList.h"
#include "avSendList.h"

static CRingList s_VideoSendList[MAX_CHANNEL_NUM];
static CMutexLock s_VideoSendMutex;
static CCondition s_VideoSendCondition( s_VideoSendMutex );

void WaitVideoSendList()
{
	s_VideoSendMutex.Lock();
	bool waitFlag = true;
	for ( int channel = 0; channel < REAL_CHANNEL_NUM; ++channel )
    {
    	if ( s_VideoSendList[channel].Size() > 0 )
        {
        	waitFlag = false;
        	break;
        }
    }
	if ( waitFlag ) s_VideoSendCondition.Wait();
	s_VideoSendMutex.Unlock();
}

void SignalVideoSendList()
{
	s_VideoSendCondition.Signal();    
}

int PopVideoSendList( int channel, VIDEO_SEND_NODE *vFrame )
{
	int nRet = -1;
	WaitVideoSendList();    
	if ( 0 <= channel && channel < REAL_CHANNEL_NUM )
    	nRet = s_VideoSendList[channel].Pop( vFrame, NULL );
	return nRet;
}

int PutVideoSendList( int channel, VIDEO_SEND_NODE *vFrame )
{
	int nRet = 0;

	if ( 0 <= channel && channel < REAL_CHANNEL_NUM )
    {        
    	if ( s_VideoSendList[channel].IsFull() )
        {
        	VIDEO_SEND_NODE vNode = { 0 };
        	nRet = s_VideoSendList[channel].Pop( &vNode, NULL );
        	if ( nRet == 0 ) ShareFree( vNode.frameData );
        }
        
    	if ( nRet == 0 )
        {
        	s_VideoSendMutex.Lock();
        	bool signalFlag = true;
        	for ( int i = 0; i < REAL_CHANNEL_NUM; ++i )
            {
            	if ( s_VideoSendList[channel].Size() > 0 )
                	signalFlag = false;
            }            
        	nRet = s_VideoSendList[channel].Put( vFrame, sizeof(*vFrame) );                
        	if ( nRet == 0 && signalFlag ) SignalVideoSendList();
        	s_VideoSendMutex.Unlock();
        }
    }
	return nRet;
}

void ClearVideoSendList()
{
	int	        	nRet	= -1;
	VIDEO_SEND_NODE	vNode	= { 0 };
    
	for ( int channel = 0; channel < REAL_CHANNEL_NUM; ++channel )
    {
    	while ( ! s_VideoSendList[channel].IsEmpty() )
        {
        	nRet = s_VideoSendList[channel].Pop( &vNode, NULL );
        	if ( nRet == 0 ) ShareFree( vNode.frameData );
        }
    }
}

