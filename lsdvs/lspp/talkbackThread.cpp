/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此文件实现了对讲发送线程
**  date           :  2013.12.20
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "const.h"
#include "debug.h"
#include "malloc.h"
#include "netSocket.h"
#include "dataService.h"
#include "thread.h"
#include "mutex.h"
#include "condition.h"
#include "message.h"
#include "rtpService.h"
#include "packData.h"
#include "talkbackList.h"
#include "talkbackThread.h"
#include "fitLspp.h"

static const unsigned	TCP_BUF_MAX_SIZE	        = 2 * 1024;
static const unsigned	RECV_BUF_MAX_SIZE	        = 9 * 1024;
static const int		TALKBACK_RTP_PORT	        = 6030;
static const int		TALKBACK_TCP_PORT	        = 6031;
static const int		AUDIO_STREAM_TYPE	        = 5;
static const int     	RTP_TALKBACK_MAX_CLIENT	    = 1;    /* 最大支持客户端连接数 */

static CMutexLock		s_TalkbackMutex;
static CCondition		s_TalkbackCondition( s_TalkbackMutex );
static CRtpService		s_TalkbackService( RTP_TALKBACK_MAX_CLIENT );

static void PackTalkbackHead(	TALKBACK_PACK_HEAD &head,
                            	unsigned short		packSn,
                            	unsigned int		packType,
                            	unsigned char		subType,
                            	unsigned short		dataLen	)
{
	head.msgFlag	= htonl( MSG_TALKBACK_FLAG );
	head.msgType	= MSG_TALKBACK_TYPE;
	head.packSn	    = htons( packSn );
	head.packType	= htonl( packType );
	head.subType	= subType;
	head.len	    = htons( dataLen );
}

static void GetTalkbackHead( TALKBACK_PACK_HEAD &head )
{
	head.msgFlag	= ntohl( head.msgFlag );
	head.packSn	    = ntohs( head.packSn );
	head.packType	= ntohl( head.packType );
	head.len	    = ntohs( head.len );
}

static int GetTalkbackPackLen( TALKBACK_PACK *pPackData )
{
	unsigned short check = 0;
	return sizeof(pPackData->head) + ntohs(pPackData->head.len) + sizeof(check);
}

static void PackTalkbackPack(	unsigned int	packType,
                            	unsigned char	subType,
                            	unsigned char *	dataBuf,
                            	unsigned short	dataLen,
                            	unsigned char *	packBuf, 
                            	int &        	packLen )
{
	TALKBACK_PACK *pTalkbackPack = ( TALKBACK_PACK * )packBuf;
	GetTalkbackHead( pTalkbackPack->head );
    
	unsigned short packSn = pTalkbackPack->head.packSn;
	PackTalkbackHead( pTalkbackPack->head, packSn, packType, subType, dataLen );

	packLen = sizeof( pTalkbackPack->head );
	if ( dataBuf != NULL && dataLen > 0 )
    {
    	memcpy( (char *)packBuf + packLen, dataBuf, dataLen );
    	packLen += dataLen;
    }
	unsigned short check = htons( GetCheckSum( packBuf, packLen ) );
	memcpy( (char *)packBuf + packLen, (char *)&check, sizeof(check) );
	packLen += sizeof(check);
}

static int DealHeartBeat( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	PackTalkbackPack( MSG_TALKBACK_HEART_BEAT, MSG_TALKBACK_RESPONSE, NULL, 0, dataBuf, dataLen );
	return 0;    
}

static int DealStartTalkback( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int nRet = s_TalkbackService.SetFlag( 0 );
	unsigned char subType = ( nRet != -1 ) ? MSG_TALKBACK_RESPONSE : MSG_TALKBACK_ERROR;
	PackTalkbackPack( MSG_TALKBACK_START_AUDIO, subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

static int DealStopTalkback( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int nRet = s_TalkbackService.ClearFlag( 0 );
	unsigned char subType = ( nRet != -1 ) ? MSG_TALKBACK_RESPONSE : MSG_TALKBACK_ERROR;
	PackTalkbackPack( MSG_TALKBACK_STOP_AUDIO, subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 命令处理函数列表
//
static ID_CMD s_TalkbackIdCmd[] = 
{
    { MSG_TALKBACK_HEART_BEAT,    	DealHeartBeat	    },
    { MSG_TALKBACK_START_AUDIO,    	DealStartTalkback	},
    { MSG_TALKBACK_STOP_AUDIO,    	DealStopTalkback	},
};

static int DealTalkbackProcess( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int nRet = -1;
	TALKBACK_PACK *pTalkbackPack = ( TALKBACK_PACK * )dataBuf;
	CIdCmd dealTalkback;
	dealTalkback.Init( s_TalkbackIdCmd, sizeof(s_TalkbackIdCmd)/sizeof(ID_CMD) );

	int cmdId = ntohl( pTalkbackPack->head.packType );
	ID_CMD_ACTION DealTalkbackCmd = dealTalkback.GetCmd( cmdId );
	if ( DealTalkbackCmd != NULL )
    	nRet = DealTalkbackCmd( dataBuf, dataLen, bufSize );

	if ( nRet < 0 ) FiPrint( "DealTalkbackProcess Error !\r\n" );
	return nRet;
}

static int CheckTalkbackProcess( unsigned char *dataBuf, int dataLen, int bufSize, int &offset )
{
	int nRet = -1;
	if ( dataBuf != NULL && dataLen > 0 && bufSize >= dataLen )
    {
    	offset = GetFlagOffset( dataBuf, dataLen, htonl(MSG_TALKBACK_FLAG) );
    	unsigned char *	data	        = dataBuf + offset;
    	int	        	len	            = dataLen - offset;
    	int	        	size	        = bufSize - offset;
    	TALKBACK_PACK *	pTalkbackPack	= (TALKBACK_PACK *)dataBuf;
    	int	        	packLen	        = GetTalkbackPackLen( pTalkbackPack );
        
    	if ( packLen > size )
        {
        	offset = sizeof( TALKBACK_PACK_HEAD );  // 包长出错, 丢弃包头.
        	FiPrint( "Check Talkback Pack Length Error !\r\n" );
        }
    	else	
    	if ( packLen <= len )
        {
        	if ( IsCheckSumOK(data, packLen) )
            {
            	if ( pTalkbackPack->head.msgType == MSG_TALKBACK_TYPE )
                {
                	nRet = packLen;
                }
            	else
                {
                	offset += packLen;
                	FiPrint( "Check Talkback Type Error !\r\n" );
                }
            }
        	else
            {
            	offset += packLen;
            	FiPrint( "CheckTalkbackProcess Check Sum Error !\r\n" );
            }
        }
    }
	return nRet;
}

void WaitTalkbackClient()
{
	if ( s_TalkbackService.ClientNum() == 0 )
    {
    	s_TalkbackMutex.Lock();
    	if ( s_TalkbackService.ClientNum() == 0 )
        	s_TalkbackCondition.Wait();
    	s_TalkbackMutex.Unlock();
    }
}

void BoardcastTalkbackClient()
{
	s_TalkbackCondition.Broadcast();
}

void *DealTalkbackSendThread( void *args )
{
	TALKBACK_NODE talkbackNode = { 0 };
    
	while ( 1 )
    {
    	if ( MessageRecv( MSG_ID_DEAL_TALKBACK_SEND_THREAD ) >= 0 )
        {
        	FiPrint( "DealTalkbackSendThread will exit ...\r\n" );
        	break;
        }

        // 获取音视频媒体数据并进行发送处理...
    	if ( s_TalkbackService.ClientNum() > 0 )
        {
        	if ( PopTalkbackSendList( &talkbackNode ) != -1 )
            {            
            	s_TalkbackService.RtpSend( 0, talkbackNode.frameData, talkbackNode.frameLen, 
                                        	AUDIO_STREAM_TYPE, talkbackNode.timeStamp );
            	ShareFree( talkbackNode.frameData );
            }
        }
    	else
        {
            // 等待客户端连接...
        	WaitTalkbackClient();
        }
    }
	return NULL;
}

void *DealTalkbackRecvThread( void *args )
{
	unsigned char *	recvBuf = NULL;
	int	        	bufSize = RECV_BUF_MAX_SIZE;
    
	recvBuf = ( unsigned char * )Malloc( sizeof(unsigned char)*RECV_BUF_MAX_SIZE );
	if ( recvBuf == NULL ) return NULL;

	int nRet = -1;
	TALKBACK_NODE talkbackNode = { 0 };
	while ( 1 )
    {
    	if ( MessageRecv( MSG_ID_DEAL_TALKBACK_RECV_THREAD ) >= 0 )
        {
        	FiPrint( "DealTalkbackRecvThread will quit ...\r\n" );
        	break;
        }

        // 获取音视频媒体数据并进行发送处理...
    	if ( s_TalkbackService.ClientNum() > 0 )
        {
            //
            // TODO !!! 对接收的RTP包进行排序
            //
        	nRet = s_TalkbackService.RtpRecv( recvBuf, bufSize, NULL );
        	if ( nRet != -1 )
            {
            	RTP_PACK *	pRtpPack	= ( RTP_PACK * )recvBuf;
            	talkbackNode.timeStamp	= pRtpPack->rtpHead.time;
            	talkbackNode.frameSeq	= pRtpPack->rtpHead.seq;
            	talkbackNode.frameLen	= pRtpPack->rtpHead.len;

            	if ( talkbackNode.frameLen <= nRet - sizeof(RTP_HEAD) )
                {
                	talkbackNode.frameData = ( unsigned char * )ShareMalloc( talkbackNode.frameLen );
                	if ( talkbackNode.frameData != NULL )
                    {
                    	memcpy( (char *)talkbackNode.frameData,
                                (char *)pRtpPack->rtpData, talkbackNode.frameLen );                
                    	PutTalkbackRecvList( &talkbackNode );
                    }
                }
            }
        }
    	else
        {    // 等待客户端连接...
        	WaitTalkbackClient();
        }
    }

	Free( recvBuf );
	return NULL;
}

void *DealTalkbackThread( void *args )
{
	int	        	nRet	    = -1;
	unsigned char * dataBuf	    = NULL;
	int	        	bufSize	    = 0;
	int	        	dataLen	    = 0;
	int	        	channel	    = 0;
	bool	    	bRun	    = true;

	dataBuf = ( unsigned char * )Malloc( sizeof(unsigned char)*TCP_BUF_MAX_SIZE );
	if ( dataBuf == NULL ) return NULL;
	else bufSize = TCP_BUF_MAX_SIZE;
    
	nRet = s_TalkbackService.InitTcp( TALKBACK_TCP_PORT );    
	if ( nRet == -1 )
    {
    	FiPrint( "Init Tcp Listen Error .\r\n" );
    	s_TalkbackService.CloseAll();
    	Free( dataBuf );
    	return NULL;
    }
    
	nRet = s_TalkbackService.InitRtp( TALKBACK_RTP_PORT );    
	if ( nRet == -1 )
    {
    	FiPrint( "Init Rtp Listen Error .\r\n" );
    	s_TalkbackService.CloseAll();
    	Free( dataBuf );
    	return NULL;
    }
    
	pthread_t dealTalkbackSendThreadID;
	pthread_t dealTalkbackRecvThreadID;
    
	if ( bRun )
    {
    	nRet = ThreadCreateCommonPriority( &dealTalkbackSendThreadID, DealTalkbackSendThread, NULL );
    	if ( nRet != 0 )
        {
        	FiPrint( "Create Talkback Send Thread Error !\r\n" );
        	bRun = false;
        }
    }
    
	if ( bRun )
    {
    	nRet = ThreadCreateCommonPriority( &dealTalkbackRecvThreadID, DealTalkbackRecvThread, NULL );
    	if ( nRet != 0 )
        {
        	FiPrint( "Create Talkback Recv Thread Error !\r\n" );
        	bRun = false;
        }
    }

	while ( bRun )
    {    
    	if ( MessageRecv( MSG_ID_DEAL_TALKBACK_THREAD ) >= 0 )
        {
        	FiPrint( "DealTalkbackThread channel %d will quit ...\r\n", channel );
        	break;
        }        
        // 接收连接和数据并进行接收处理...
    	nRet = s_TalkbackService.Select();
    	if ( nRet < 0 )
        {
        	FiPrint( "DealTalkbackThread Select Error !\r\n" );
        	break;
        }
        
    	if ( nRet > 0 )
        {
        	s_TalkbackMutex.Lock();
        	int nAccept = s_TalkbackService.Accept();
        	if ( nAccept != -1 )
            {
            	if ( s_TalkbackService.ClientNum() == 1 )
                	BoardcastTalkbackClient();
            }
        	s_TalkbackMutex.Unlock();
        	if ( nAccept != -1 ) continue;
                
            // 接收数据
        	int recvLen = s_TalkbackService.TcpRecv();    // 接收的数据放入接收缓冲区	        
        	if ( recvLen <= 0 ) s_TalkbackService.Close( s_TalkbackService.GetSocket() );

            // 循环从缓冲区中读取数据进行处理 !
            // 处理过的数据从缓冲区中删除, 没有处理的数据则继续循环处理. 
            // 首先对数据进行校验, 通过数据校验后再进行数据处理; 处理完毕后进行发送.
        	while ( recvLen > 0 )
            {
                // 读取数据
            	dataLen = s_TalkbackService.GetData( dataBuf, bufSize );
            	if ( dataLen == -1 ) break;

                // 校验数据
            	int offset = 0;
            	nRet = CheckTalkbackProcess( dataBuf, dataLen, bufSize, offset );
            	recvLen -= s_TalkbackService.DelData( offset );
            	if ( nRet == -1 ) break;
            	else recvLen -= s_TalkbackService.DelData( nRet );

                // 处理数据
            	unsigned char *	packBuf = dataBuf + offset;
            	int         	packLen = dataLen - offset;                
            	nRet = DealTalkbackProcess( packBuf, packLen, bufSize-offset );
            	if ( nRet == -1 ) break;
                
                // 发送数据
            	nRet = s_TalkbackService.TcpSend( packBuf, packLen );
            	if ( nRet < 0 )
                {
                	s_TalkbackService.Close( s_TalkbackService.GetSocket() );
                	break;
                }
            }
        }
    	s_TalkbackService.HeartBeat();
    	usleep( 10*1000 );
    }

	ThreadJoin( dealTalkbackSendThreadID, NULL );
	ThreadJoin( dealTalkbackRecvThreadID, NULL );
	s_TalkbackService.CloseAll();
	Free( dataBuf );

	return NULL;
}

int QuitTalkbackThread()
{
	int nRet = 0;
	if ( nRet != -1 ) nRet = MessageSend( MSG_ID_DEAL_TALKBACK_SEND_THREAD );
	if ( nRet != -1 ) SignalTalkbackSendList();
	if ( nRet != -1 ) nRet = MessageSend( MSG_ID_DEAL_TALKBACK_RECV_THREAD );
	if ( nRet != -1 ) SignalTalkbackRecvList();
	if ( nRet != -1 )
    {
    	BoardcastTalkbackClient();
    	nRet = MessageSend( MSG_ID_DEAL_TALKBACK_THREAD );
    }
	FiPrint( "Quit Talkback Thread %s !\r\n", (nRet != -1) ? "OK" : "Failed"  );
	return nRet;
}

