/********************************************************************************
**	Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**	author        :  sven
**	version       :  v1.0
**	date           :  2013.10.10
**	description  : 专门为流处理线程建立独立的消息队列,防止冲突
********************************************************************************/
#include "message.h"
#include "debug.h"

/* ------------------------------------------------------------------------- */

static CMessage	g_dsmMessage;

/************************************************************************
* fn: 发送消息
* 返回: 大于0, 表示成功发送消息, 而且有消息内容; 
    	等于0, 表示成功发送消息, 但没有消息内容; 
    	小于0,表示发送消息失败
*************************************************************************/
int DsmMessageSend( int nMsgId, char* pMsgBuf, int nMsgLen )
{
	return g_dsmMessage.Send( nMsgId, pMsgBuf, nMsgLen );
}

/***************************************************************************
* fn: 接收消息
* 返回: 大于0, 表示有接收到消息, 而且有消息内容; 
    	等于0, 表示有接收到消息, 但没有消息内容; 
    	小于0,表示没有接收到消息
****************************************************************************/
int DsmMessageRecv( int nMsgId, char* pMsgBuf, int nMsgLen )
{
	int ret;
	ret = g_dsmMessage.Recv( nMsgId, pMsgBuf, nMsgLen );
	return ret;
}

bool DsmMessageFind( int nMsgId )
{
	return g_dsmMessage.Find( nMsgId );
}

