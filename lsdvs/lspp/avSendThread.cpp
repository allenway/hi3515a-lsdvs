/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此文件实现了流媒体数据发送模块，主要将流媒体数据通过RTP/RTCP协议
**            发送到客户端。
**  date           :  2013.11.11
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
#include <sys/socket.h>
#include <arpa/inet.h>

#include "debug.h"
#include "const.h"
#include "malloc.h"
#include "netSocket.h"
#include "dataService.h"
#include "thread.h"
#include "mutex.h"
#include "condition.h"
#include "message.h"
#include "packData.h"
#include "rtpService.h"
#include "avSendList.h"
#include "avSendThread.h"
#include "proconH264.h"
#include "hton.h"

#include "fitLspp.h"
#include "fit.h"

static const int TCP_BUF_MAX_SIZE	    = 1024;
static const int RTP_PORT	            = 6020;
static const int TCP_PORT	            = 6021;
static const int RTP_AV_MAX_CLIENT	    = 4;        /* 最大支持客户端连接数 */

static CMutexLock	s_RtpMutex;
static CCondition	s_RtpCondition( s_RtpMutex );
static CRtpService	s_RtpService( RTP_AV_MAX_CLIENT );

static void PackAvsDataHead( AVS_PACK_HEAD &head, unsigned short packSn,
                        	unsigned int packType, unsigned char subType,
                        	unsigned short dataLen )
{
	head.msgFlag	= htonl( MSG_AVS_DATA_FLAG );
	head.msgType	= MSG_AVS_DATA_TYPE;
	head.packSn	    = htons( packSn );
	head.packType	= htonl( packType );
	head.subType	= subType;
	head.len	    = htons( dataLen );
}

static void GetAvsDataHead( AVS_PACK_HEAD &head )
{
	head.msgFlag	= ntohl( head.msgFlag );
	head.packSn	    = ntohs( head.packSn );
	head.packType	= ntohl( head.packType );
	head.len	    = ntohs( head.len );
}

static int GetAvsPackLen( AVS_PACK_DATA *pPackData )
{
	unsigned short check = 0;
	return sizeof(pPackData->head) + ntohs(pPackData->head.len) + sizeof(check);
}

static void PackAvsDataPack(	unsigned int	packType,
                            	unsigned char	subType,
                            	unsigned char *	dataBuf,
                            	unsigned short	dataLen,
                            	unsigned char *	packBuf,
                            	int &        	packLen	)
{
	AVS_PACK_DATA *pAvsPackData = ( AVS_PACK_DATA * )packBuf;
	GetAvsDataHead( pAvsPackData->head );
    
	unsigned short packSn = pAvsPackData->head.packSn;
	PackAvsDataHead( pAvsPackData->head, packSn, packType, subType, dataLen );

	packLen = sizeof( pAvsPackData->head );
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
	PackAvsDataPack( MSG_AVS_HEART_BEAT, MSG_AVS_DATA_RESPONSE, NULL, 0, dataBuf, dataLen );
	return 0;    
}

static int DealStartVideo( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	AVS_PACK_DATA *pAvsPackData = ( AVS_PACK_DATA * )dataBuf;
	int channel = pAvsPackData->data[0];
	int nRet = s_RtpService.SetFlag( channel );
	if ( nRet == 0 )
    {
    	FitMympiForceIframe( channel );
    }
	else
    {
    	FiPrint( "DealStartVideo Error, channel %d.\r\n", channel );
    }
	unsigned char subType = ( nRet != -1 ) ? MSG_AVS_DATA_RESPONSE : MSG_AVS_DATA_ERROR;
	PackAvsDataPack( MSG_AVS_START_VIDEO, subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

static int DealStopVideo( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	AVS_PACK_DATA *pAvsPackData = ( AVS_PACK_DATA * )dataBuf;
	int channel = pAvsPackData->data[0];
	int nRet = s_RtpService.ClearFlag( channel );
	unsigned char subType = ( nRet != -1 ) ? MSG_AVS_DATA_RESPONSE : MSG_AVS_DATA_ERROR;
	PackAvsDataPack( MSG_AVS_STOP_VIDEO, subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

#if 1
static int RtpRecvNat( struct sockaddr_in *pFromAddr )
{
	int i, ret;
	char buf[1024];
	struct sockaddr_in fromAddr;

	for( i = 0; i < 10; ++i )
    {
    	ret = s_RtpService.RtpRecv( buf, sizeof(buf), &fromAddr );
    	if( ret > 0 )
        {
        	if( NULL != pFromAddr )
            {
                *pFromAddr = fromAddr;
            	ret = 0;
            }
        	break;
        }
    	else
        {
        	ret = -1;
        }
    }

	return ret;
}

static int DealSetVideoPort( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	unsigned int port = 0;
	ushort myPort;
	int nRet;
	char addr[NET_ADDRSIZE];    
	struct sockaddr_in fromAddr;
    
	AVS_PACK_DATA *pAvsPackData = ( AVS_PACK_DATA * )dataBuf;
	memcpy( &port, pAvsPackData->data, sizeof(port) );
	port = ntohs(port);

	nRet = RtpRecvNat( &fromAddr );
	if( 0 == nRet )
    {        
    	nRet = s_RtpService.SetAddr( (void *)&fromAddr );

    	Ntop( fromAddr, addr, &myPort );
    	FiPrint( "nRet(%d) = RtpRecvNat(%s:%u)!\r\n", nRet, addr, myPort );    
        //s_RtpService.AddNatSocket( addr, myPort );
    }
	else
    {
    	nRet = -1;
    }    
    
	unsigned char subType = ( nRet != -1 ) ? MSG_AVS_DATA_RESPONSE : MSG_AVS_DATA_ERROR;
	PackAvsDataPack( MSG_AVS_SET_VIDEO_PORT, subType, NULL, 0, dataBuf, dataLen );
    
	return nRet;
}

#else
static int DealSetVideoPort( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	unsigned int port = 0;
	AVS_PACK_DATA *pAvsPackData = ( AVS_PACK_DATA * )dataBuf;
	memcpy( &port, pAvsPackData->data, sizeof(port) );
	FiPrint( "Set Rtp Video Port %d !\r\n", ntohs(port) );    
	int nRet = s_RtpService.SetPort( ntohs( port ) );
	unsigned char subType = ( nRet != -1 ) ? MSG_AVS_DATA_RESPONSE : MSG_AVS_DATA_ERROR;
	PackAvsDataPack( MSG_AVS_SET_VIDEO_PORT, subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

#endif

//
// 命令处理函数列表
//
static ID_CMD s_AvsDataIdCmd[] = 
{
    { MSG_AVS_HEART_BEAT,    	DealHeartBeat	    },
    { MSG_AVS_START_VIDEO,    	DealStartVideo	    },
    { MSG_AVS_STOP_VIDEO,    	DealStopVideo	    },
    { MSG_AVS_SET_VIDEO_PORT,	DealSetVideoPort	},
};

static int DealAvsDataProcess( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int nRet = -1;
	AVS_PACK_DATA *pAvsPackData = ( AVS_PACK_DATA * )dataBuf;
	CIdCmd dealAvsData;
	dealAvsData.Init( s_AvsDataIdCmd, sizeof(s_AvsDataIdCmd)/sizeof(ID_CMD) );

	int cmdId = ntohl( pAvsPackData->head.packType );
	ID_CMD_ACTION DealAvsDataCmd = dealAvsData.GetCmd( cmdId );
	if ( DealAvsDataCmd != NULL )
    	nRet = DealAvsDataCmd( dataBuf, dataLen, bufSize );

	if ( nRet < 0 ) FiPrint( "DealAvsDataProcess Error !\r\n" );
	return nRet;
}

static int CheckAvsDataProcess( unsigned char *dataBuf, int dataLen, int bufSize, int &offset )
{
	int nRet = -1;
	if ( dataBuf != NULL && dataLen > 0 && bufSize >= dataLen )
    {
    	offset = GetFlagOffset( dataBuf, dataLen, htonl(MSG_AVS_DATA_FLAG) );
    	unsigned char *	data	    = dataBuf + offset;
    	int	        	len	        = dataLen - offset;
    	int	        	size	    = bufSize - offset;
    	AVS_PACK_DATA *	pAvsPack	= (AVS_PACK_DATA *)data;
    	int	        	packLen	    = GetAvsPackLen( pAvsPack );
        
    	if ( packLen > size )
        {
        	offset = sizeof( AVS_PACK_HEAD );  // 包长出错, 丢弃包头.
        	FiPrint( "Check Avs Pack Length Error !\r\n" );
        }
    	else	    
    	if ( packLen <= len )
        {
        	if ( IsCheckSumOK(data, packLen) )
            {
            	if ( pAvsPack->head.msgType == MSG_AVS_DATA_TYPE )
                {
                	nRet = packLen;
                }
            	else
                {
                	offset += packLen;
                	FiPrint( "Check Avs Type Error !\r\n" );
                }
            }
        	else
            {
            	offset += packLen;
            	FiPrint( "Check Sum Error !\r\n" );
            }
        }
    }
	return nRet;
}

#if 0 // 
#define HI3512_RECORD_FILE_1 "/tmp/ch0_1.h264"
static void SaveH2641( char *pData0, int len0 )
{
	static FILE	*fp_record = NULL;
    
	if( NULL == fp_record )
    {
    	fp_record = fopen( HI3512_RECORD_FILE_1, "w+b" );
    	if(fp_record == NULL)
        {
        	FiPrint("fopen failed:%s!\r\n",STRERROR_ERRNO);
        	return;
        }
    }                            

	if( len0 > 0 )
    {
    	if( fwrite( pData0, len0, 1, fp_record) != 1 )
        {
        	perror( "fi_record write1" );
        }
    }
}

#endif

static void *DealRtpSendThread( void *args )
{
    
	bool quit = false;
	VIDEO_SEND_NODE vSendNode	= { 0 };
	int i, ret;
	int count = 0, countGetChannelRequestFlag = 0;
	unsigned int ProconH264Fd[REAL_CHANNEL_NUM];
	PROCON_NODE_T *proconH264;

	for( i = 0; i < REAL_CHANNEL_NUM; ++i )
    {
    	ProconH264Fd[i] = ProconH264Open( i, OPEN_RDONLY );
    }
	while ( ! quit )
    {
    	if ( MessageRecv( MSG_ID_DEAL_RTP_SEND_THREAD ) >= 0 )
        {
        	FiPrint( "DealAVSendThread will quit ...\r\n" );
        	quit = true;
        	break;
        }

        // 获取音视频媒体数据并进行发送处理...
    	if ( s_RtpService.ClientNum() > 0 )
        {
        	for ( int channel = 0; channel < REAL_CHANNEL_NUM; ++channel )
            {
            	if ( MessageRecv( MSG_ID_DEAL_RTP_SEND_THREAD ) >= 0 )
                {
                	FiPrint( "DealAVSendThread will quit ...\r\n" );
                	quit = true;
                	break;
                }
                
            	ret = s_RtpService.GetChannelRequestFlag(channel);
                
                //FiPrint( "ret(%d) = s_RtpService.GetChannelRequestFlag(ch%02d)!\r\n", ret, channel );
            	if( ret <= 0 )
                {    
                	if( ++countGetChannelRequestFlag >= REAL_CHANNEL_NUM )
                    {
                    	countGetChannelRequestFlag = 0;
                    	usleep( 40000 );
                    }
                	continue;
                }
            	countGetChannelRequestFlag = 0;
            	proconH264 = ProconH264Read( ProconH264Fd[channel] );
            	if( NULL == proconH264 )
                {
                	if( ++count >= s_RtpService.GetCurChannelNum() )
                    {
                    	count = 0;
                    	usleep( 20000 );
                    }
                	continue;
                }
            	else 
                {                    
                	count = 0;                    
                	ProconH264ToVideoSendNode( proconH264, &vSendNode );
                	s_RtpService.RtpSend( channel, vSendNode.frameData, vSendNode.frameLen, 
                                        	vSendNode.frameType, vSendNode.timeStamp );    
                	ProconH264Free( proconH264 );
                }                
            }
        }
    	else
        {
            // 等待客户端连接
        	s_RtpMutex.Lock();
        	if ( s_RtpService.ClientNum() == 0 )
            	s_RtpCondition.Wait();
        	s_RtpMutex.Unlock();
        }
    }
	for( i = 0; i < REAL_CHANNEL_NUM; ++i )
    {
    	ProconH264Close( ProconH264Fd[i] );
    }
	return NULL;
}

void *DealAVSendThread( void *args )
{
	int	        	nRet	    = -1;
	unsigned char * dataBuf	    = NULL;
	int	        	bufSize	    = 0;
	int	        	dataLen	    = 0;
	char	    	udpBuf[1024];
	struct sockaddr_in rtpFromAddr;
    
	dataBuf = ( unsigned char * )Malloc( sizeof(unsigned char)*TCP_BUF_MAX_SIZE );
	if ( dataBuf == NULL ) return NULL;
	else bufSize = TCP_BUF_MAX_SIZE;
        
	nRet = s_RtpService.InitTcp( TCP_PORT );    
	if ( nRet == -1 )
    {
    	FiPrint( "Init Tcp Listen Error .\r\n" );
    	s_RtpService.CloseAll();
    	Free( dataBuf );
    	return NULL;
    }
	nRet = s_RtpService.InitRtp( RTP_PORT );    
	if ( nRet == -1 )
    {
    	FiPrint( "Init Rtp Listen Error .\r\n" );
    	s_RtpService.CloseAll();
    	Free( dataBuf );
    	return NULL;
    }

	pthread_t dealRtpSendThreadID;
	nRet = ThreadCreateCommonPriority( &dealRtpSendThreadID, DealRtpSendThread, NULL );
	if ( nRet != 0 )
    {
    	FiPrint( "Create Rtp Thread Error !\r\n" );
    	s_RtpService.CloseAll();
    	Free( dataBuf );
    	return NULL;
    }

	while(  (nRet = s_RtpService.RtpRecv(udpBuf, sizeof(udpBuf), &rtpFromAddr)) > 0 )
    {
        ;
    }
    
	while ( 1 )
    {    
    	if ( MessageRecv( MSG_ID_DEAL_AV_SEND_THREAD ) >= 0 )
        {
        	FiPrint( "DealAVSendThread will quit ...\r\n" );
        	break;
        }
        
        // 接收连接和数据并进行接收处理...
    	nRet = s_RtpService.Select();
    	if ( nRet < 0 )
        {
        	FiPrint( "DealMessageDataThread Select Error !\r\n" );
        	break;
        }
    	else if ( nRet > 0 )
        {    
        	s_RtpMutex.Lock();
        	int nAccept = s_RtpService.Accept();
        	if ( nAccept != -1 )
            {
            	if ( s_RtpService.ClientNum() == 1 )
                	s_RtpCondition.Signal();
            }
        	s_RtpMutex.Unlock();
        	if ( nAccept != -1 ) continue;

            // 接收数据并将数据放入接收缓冲区	
        	int recvLen = s_RtpService.TcpRecv();
        	if ( recvLen <= 0 ) s_RtpService.Close( s_RtpService.GetSocket() );

            // 循环从缓冲区中读取数据进行处理 !
            // 处理过的数据从缓冲区中删除, 没有处理的数据则继续循环处理. 
            // 首先对数据进行校验, 通过数据校验后再进行数据处理; 处理完毕后进行发送.
        	while ( recvLen > 0 )
            {
                // 读取数据
            	dataLen = s_RtpService.GetData( dataBuf, bufSize );
            	if ( dataLen == -1 ) break;
                
                // 校验数据
            	int offset = 0;
            	nRet = CheckAvsDataProcess( dataBuf, dataLen, bufSize, offset );
            	recvLen -= s_RtpService.DelData( offset );
            	if ( nRet == -1 ) break;
            	else recvLen -= s_RtpService.DelData( nRet );
                
                // 处理数据
            	unsigned char *	packBuf = dataBuf + offset;
            	int         	packLen = dataLen - offset;
            	nRet = DealAvsDataProcess( packBuf, packLen, bufSize-offset );
            	if ( nRet == -1 ) break;
                
                // 发送数据
            	nRet = s_RtpService.TcpSend( packBuf, packLen );
            	if ( nRet < 0 )
                {
                	s_RtpService.Close( s_RtpService.GetSocket() );
                	break;
                }
            }
        }
        // 目的是接收nat 的心跳包
    	if(  (nRet = s_RtpService.RtpRecv(udpBuf, sizeof(udpBuf), &rtpFromAddr)) > 0 )
        {            
        	inet_ntop( AF_INET, &rtpFromAddr.sin_addr, udpBuf, 20 );
            //int udpFromPort = Ntohs( rtpFromAddr.sin_port );
        }
    	s_RtpService.HeartBeat();
    	usleep( 10*1000 );
    }
    
	ThreadJoin( dealRtpSendThreadID, NULL );
	s_RtpService.CloseAll();
	Free( dataBuf );
	return NULL;
}

int QuitAVSendThread()
{
	s_RtpService.CloseAll();
	int nRet = MessageSend( MSG_ID_DEAL_RTP_SEND_THREAD );
	if ( nRet != -1 )
    {
    	SignalVideoSendList();
    	s_RtpCondition.Signal();
    	nRet = MessageSend( MSG_ID_DEAL_AV_SEND_THREAD );
    }
	return nRet;
}

