/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此文件实现了对对讲发送缓冲区进行操作的相关函数
**  date           :  2013.12.20
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#include <stdio.h>
#include "debug.h"
#include "malloc.h"
#include "mutex.h"
#include "condition.h"
#include "ringList.h"
#include "talkbackList.h"

static CRingList s_TalkbackSendList;
static CMutexLock s_TalkbackSendMutex;
static CCondition s_TalkbackSendCondition( s_TalkbackSendMutex );

static CRingList s_TalkbackRecvList;
static CMutexLock s_TalkbackRecvMutex;
static CCondition s_TalkbackRecvCondition( s_TalkbackRecvMutex );

void WaitTalkbackSendList()
{
	s_TalkbackSendMutex.Lock();
	if ( s_TalkbackSendList.Size() == 0 )
    	s_TalkbackSendCondition.Wait();
	s_TalkbackSendMutex.Unlock();
}

void SignalTalkbackSendList()
{
	s_TalkbackSendCondition.Signal();    
}

int PopTalkbackSendList( TALKBACK_NODE *tFrame )
{
	WaitTalkbackSendList();
	return s_TalkbackSendList.Pop( tFrame, NULL );
}

int PutTalkbackSendList( TALKBACK_NODE *tFrame )
{
	int	        	nRet	= 0;
	TALKBACK_NODE	tNode	= { 0 };
    
	if ( s_TalkbackSendList.IsFull() )
    {
    	nRet = s_TalkbackSendList.Pop( &tNode, NULL );
    	if ( nRet == 0 ) ShareFree( tNode.frameData );
    //	FiPrint( "Put talkback send list is full, pop one node !\r\n" );
    }
	if ( nRet == 0 )
    {
    	s_TalkbackSendMutex.Lock();
    	nRet = s_TalkbackSendList.Put( tFrame, sizeof(*tFrame) );
    	if ( nRet == 0 && s_TalkbackSendList.Size() == 1 ) SignalTalkbackSendList();
    	s_TalkbackSendMutex.Unlock();
    }
	return nRet;
}

void ClearTalkbackSendList()
{
	int	        	nRet	= -1;
	TALKBACK_NODE	tNode	= { 0 };
	while ( ! s_TalkbackSendList.IsEmpty() )
    {
    	nRet = s_TalkbackSendList.Pop( &tNode, NULL );
    	if ( nRet == 0 ) ShareFree( tNode.frameData );
    }
}

void WaitTalkbackRecvList()
{
	s_TalkbackRecvMutex.Lock();
	if ( s_TalkbackRecvList.Size() == 0 )
    	s_TalkbackRecvCondition.Wait();
	s_TalkbackRecvMutex.Unlock();
}

void SignalTalkbackRecvList()
{
	s_TalkbackRecvCondition.Signal();    
}

int PopTalkbackRecvList( TALKBACK_NODE *tFrame )
{
	WaitTalkbackRecvList();
	return s_TalkbackRecvList.Pop( tFrame, NULL );
}

int PutTalkbackRecvList( TALKBACK_NODE *tFrame )
{
	int	        	nRet	= 0;
	TALKBACK_NODE	tNode	= { 0 };
    
	if ( s_TalkbackRecvList.IsFull() )
    {
    	nRet = s_TalkbackRecvList.Pop( &tNode, NULL );
    	if ( nRet == 0 ) ShareFree( tNode.frameData );
    //	FiPrint( "Put talkback recv list is full, pop one node !\r\n" );
    }
	if ( nRet == 0 )
    {
    	s_TalkbackRecvMutex.Lock();
    	nRet = s_TalkbackRecvList.Put( tFrame, sizeof(*tFrame) );
    	if ( nRet == 0 && s_TalkbackRecvList.Size() == 1 ) SignalTalkbackRecvList();
    	s_TalkbackRecvMutex.Unlock();
    }
	return nRet;
}

void ClearTalkbackRecvList()
{
	int	        	nRet	= -1;
	TALKBACK_NODE	tNode	= { 0 };
    
	while ( ! s_TalkbackRecvList.IsEmpty() )
    {
    	nRet = s_TalkbackRecvList.Pop( &tNode, NULL );
    	if ( nRet == 0 ) ShareFree( tNode.frameData );
    }
}

