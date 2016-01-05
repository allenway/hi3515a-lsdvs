/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此文件实现了对RTP/RTCP协议相关的类。
**  date           :  2013.11.12
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "debug.h"
#include "netSocket.h"
#include "const.h"
#include "malloc.h"
#include "rtpService.h"
#include "linuxFile.h"
#include "ttypes.h"
#include "timeExchange.h"

static const int RTP_SEND_BUF_MAX_SIZE	= 64 * 1024;
static const int MAX_HEART_BEAT_TIME	= 180;        /* 心跳函数最大等待时间 */
static const int RTP_DATA_BUF_SIZE	    = 1024;        /* 数据缓冲区大小 */
static const int RTP_DEFAULT_MAX_CLIENT	= 4;

CRtpService::CRtpService()
{
	InitSocket( RTP_DEFAULT_MAX_CLIENT );
}

CRtpService::CRtpService( int clientMax )
{
	InitSocket( clientMax );
}
    
CRtpService::~CRtpService()
{
	CloseAll();
	if ( m_ClientSocket )
    {
    	for ( int i = 0; i < m_ClientNum; ++i )
        	Free( m_ClientSocket[i].dataBuf );
    	Free( m_ClientSocket );
    }
}

int CRtpService::InitRtp( int rtpPort )
{
	int rtpSocket = -1;
	int nRet = SocketUdpListen( &rtpSocket, rtpPort );
	if ( nRet != -1 )
    {
    	int sendBufSize = RTP_SEND_BUF_MAX_SIZE;
    	setsockopt( rtpSocket, SOL_SOCKET, SO_SNDBUF,
                (char *)&sendBufSize, sizeof(sendBufSize) );
    	m_RtpSocket = rtpSocket;
    	m_RtpPort	= rtpPort;
    }
	return nRet;
}

int CRtpService::InitTcp( int tcpPort )
{
	int tcpSocket = -1;
	int nRet = SocketTcpListen( &tcpSocket, tcpPort, BLOCK_NO );
	if ( nRet != -1 )
    {
    	m_TcpSocket = tcpSocket;
    	m_TcpPort	= tcpPort;
    }
	return nRet;
}

int CRtpService::Accept()
{
	int nRet = -1;
	fd_set fd; 
	FD_ZERO( &fd );
  	FD_SET( m_TcpSocket, &fd );
	struct timeval timeout;
	timeout.tv_sec	= 0;
  	timeout.tv_usec	= 0;
	if ( select( m_TcpSocket+1, &fd, NULL, NULL, &timeout ) > 0 )
      {
    	struct sockaddr_in clientAddr = { 0 };
    	int len = sizeof( clientAddr );    
    	int clientSocket = accept( m_TcpSocket, (struct sockaddr *)&clientAddr, (socklen_t *)&len );
    	if ( clientSocket > 0 ) 
        {                
        	clientAddr.sin_port = htons( m_RtpPort );
        	nRet = AddSocket( clientSocket, clientAddr );
        	if ( nRet == -1 )
            {
            	close( clientSocket );
            }
        	else
            {
            	nRet = clientSocket;
            	FiPrint( "## Accept: IP = %s, PORT = %d, CLIENT = %d.\r\n",
                    	inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), ClientNum() );
            }
        }
    }
	return nRet;
}

void CRtpService::InitRtpHead( RTP_HEAD *pRtpHead, int seq, unsigned int time,
                        	int begin, int end,	int fType, int channel, int len )
{
	pRtpHead->ver	    = 2;
	pRtpHead->pad	    = 0;
	pRtpHead->ext	    = 1;
	pRtpHead->cc	    = 0;
	pRtpHead->mark	    = 0;
	pRtpHead->payLoad	= 0;
	pRtpHead->seq	    = htons( seq );
	pRtpHead->time	    = htonl( time );
	pRtpHead->ssrc	    = htonl( 0 );
	pRtpHead->sType	    = 1;
	pRtpHead->sFormat	= 1;
	pRtpHead->begin	    = begin;
	pRtpHead->end	    = end;
	pRtpHead->fType	    = fType;
	pRtpHead->channel	= channel;
	pRtpHead->len	    = htons( len );
}

static int CheckipIsNat( char *ipAddr )
{
	if( !strncmp(ipAddr,"10", 2) 
        || !strncmp(ipAddr, "192.168", 7) 
        || (strncmp(ipAddr, "172.16", 6) > 0 && strncmp(ipAddr, "172.31", 6) < 0) )
    {
        	return 1;
    }
	else 
    {
    	return 0;
    }
}

int CRtpService::RtpSend( int channel, void *data, int len,
                        	int type, unsigned int time )
{
	int	    	nRet	    = -1;
	int	    	packLen	    = sizeof( RTP_PACK ) + len - 1;

	RTP_PACK *	pRtpPack	= ( RTP_PACK * )Malloc( packLen );
	if ( pRtpPack == NULL ) return nRet;
    
	for ( int i = 0; i < m_ClientNum; ++i )
    {
    	if ( m_ClientSocket[i].flag[channel] == 0 ) continue;
        
    	char *	sendData	= (char *)data;
    	int		sendLen	    = len;
    	int		dataLen     = len;
    	int		begin	    = 1;    // 包开始标记
    	int		end	        = 0;    // 包结束标记
    	int		beginSeq	= m_ClientSocket[i].seq[channel];
    	char	ipAddr[20];

    	while ( dataLen > 0 )  // 对于比较大的帧进行分包发送
        {
        	if ( beginSeq != m_ClientSocket[i].seq[channel] ) begin = 0;
        	if ( dataLen < RTP_PACK_MAX_SIZE )
            {
            	sendLen = dataLen;
            	end = 1;
            }
        	else sendLen = RTP_PACK_MAX_SIZE;

        	InitRtpHead( &pRtpPack->rtpHead, ++(m_ClientSocket[i].seq[channel]),
                        	time, begin, end, type, channel, sendLen );
        	memcpy( (char *)(pRtpPack->rtpData), (char *)sendData, sendLen );
        	packLen = sizeof( pRtpPack->rtpHead ) + sendLen;
        	nRet = SelectWrite( m_RtpSocket, 1 );
        	if( nRet > 0 )
            {
            	nRet = sendto( m_RtpSocket, pRtpPack, packLen, 0,
                        (struct sockaddr *)&(m_ClientSocket[i].addr),
                        	sizeof(m_ClientSocket[i].addr) );    
            	Ntop( m_ClientSocket[i].addr, ipAddr, NULL );
            	if( !CheckipIsNat(ipAddr) )
                {                    
                	usleep(1000); // ql add for 为了适应像evdo 3G 这种网络而添加的
                }
                // NanosleepZero();
            }                                            
        	if ( nRet < 0 ) break;
            
        	dataLen -= sendLen;
        	sendData += sendLen;
        }
    }    
	Free( pRtpPack );
	return nRet;
}

void CRtpService::GetRtpHead( RTP_HEAD *pRtpHead )
{
	pRtpHead->seq	= ntohs( pRtpHead->seq );
	pRtpHead->time	= ntohl( pRtpHead->time );
	pRtpHead->len	= ntohs( pRtpHead->len );
}

int CRtpService::RtpRecv( void *data, int len, struct sockaddr_in *fromAddr )
{
	int nRet = Select( m_RtpSocket, 100 );
	if ( nRet > 0 )
    {
    	struct sockaddr_in addrRecv = { 0 }; //m_ClientSocket[m_CurSocket].addr;
    	socklen_t addrLen = sizeof( addrRecv );
    	nRet = recvfrom( m_RtpSocket, (char *)data, len, 0, 
                        (struct sockaddr *)&addrRecv, &addrLen );    
    	if ( nRet > ( int )sizeof(RTP_HEAD) )
        {
        	RTP_PACK *pRtpPack = ( RTP_PACK * )data;
        	GetRtpHead( &(pRtpPack->rtpHead) );
        	if( NULL != fromAddr )
            {
                *fromAddr = addrRecv;
            }
        }        
    	else
        {
        	nRet = -1;
        }
    }
	return nRet;
}

int CRtpService::TcpSend( void *data, int len )
{
	int nRet = -1;
    
	if ( m_ClientSocket == NULL ) return nRet;

	if ( m_ClientSocket[m_CurSocket].state )
    	nRet = Writen( m_ClientSocket[m_CurSocket].socket, data, len );

	return nRet;
}

int CRtpService::TcpRecv( void *data, int len )
{
	int nRet = -1;
    
	if ( m_ClientSocket == NULL ) return nRet;

	for ( int i = 0; i < m_ClientNum; ++i )
    {
    	if ( m_ClientSocket[i].state )
        {
        	m_CurSocket = i;
        	nRet = recv( m_ClientSocket[i].socket, data, len, 0 );
        	if ( nRet > 0 ) m_ClientSocket[i].timeStamp = time( NULL );
        	break;
        }
    }
	return nRet;
}

int CRtpService::TcpRecv()
{
	int nRet = -1;    
	if ( m_ClientSocket == NULL ) return nRet;
    
	for ( int i = 0; i < m_ClientNum; ++i )
    {
    	if ( m_ClientSocket[i].state && m_ClientSocket[i].dataBuf )
        {            
        	m_CurSocket = i;
        	unsigned int nDataLen = m_ClientSocket[i].bufSize;
        	unsigned char *pDataBuf = m_ClientSocket[i].dataBuf;
        	pDataBuf += m_ClientSocket[i].dataLen;
        	nDataLen -= m_ClientSocket[i].dataLen;

        	nRet = recv( m_ClientSocket[i].socket, pDataBuf, nDataLen, 0 );
        	if ( nRet > 0 ) 
            {    
            	m_ClientSocket[i].timeStamp = time( NULL );
            	m_ClientSocket[i].dataLen += nRet;
            	nRet = m_ClientSocket[i].dataLen;
            }
        }
    }
	return nRet;
}

int CRtpService::GetData( void *data, int len )
{
	if ( m_ClientSocket == NULL ) return -1;
	int i = m_CurSocket;
	int nDataLen = m_ClientSocket[i].dataLen;
	if ( len < nDataLen ) nDataLen = len;     
	if ( nDataLen > 0 ) memcpy( data, m_ClientSocket[i].dataBuf, nDataLen );
	return nDataLen;
}

int CRtpService::DelData( int len )
{
	int nRet = 0;
	if ( m_ClientSocket == NULL ) return nRet;
	int i = m_CurSocket;
	if ( len > 0 )
    {
    	unsigned char *pDataBuf = m_ClientSocket[i].dataBuf;
    	if ( pDataBuf != NULL )
        {
        	if ( len >= m_ClientSocket[i].dataLen ) 
            {
            	nRet = m_ClientSocket[i].dataLen;
            	m_ClientSocket[i].dataLen = 0;
            }
        	else
            {
            	m_ClientSocket[i].dataLen -= len;
            	memmove( pDataBuf, pDataBuf+len, m_ClientSocket[i].dataLen );
            	nRet = len;
            }
        }
    }    
	return nRet;
}

int CRtpService::Select()
{
	int nRet = -1;
	if ( m_ClientSocket == NULL ) return nRet;

	fd_set fd; 
	FD_ZERO( &fd );
  	FD_SET( m_TcpSocket, &fd );

  	int maxfd = m_TcpSocket;
    for ( int i = 0; i < m_ClientNum; ++i )
      {
      	FD_SET( m_ClientSocket[i].socket, &fd );
      	if ( m_ClientSocket[i].socket > maxfd )
          	maxfd = m_ClientSocket[i].socket;
      }
	struct timeval timeout;
	timeout.tv_sec	= 0;
  	timeout.tv_usec	= 500000;
    
  	nRet = select( maxfd + 1, &fd, NULL, NULL, &timeout );
	if ( nRet > 0 )
      {
      	for ( int i = 0; i < m_ClientNum; ++i )
          {
          	if ( FD_ISSET(m_ClientSocket[i].socket, &fd) )
              	m_ClientSocket[i].state = 1;
          	else m_ClientSocket[i].state = 0;
          }
      }
  	return nRet;
}

int CRtpService::Select( int socket, int maxMsec )
{
	int nRet = -1;
	fd_set fd; 
	FD_ZERO( &fd );
  	FD_SET( socket, &fd );

	struct timeval timeout;
	timeout.tv_sec	= maxMsec / 1000;
  	timeout.tv_usec	= (maxMsec % 1000) * 1000;
    
  	nRet = select( socket + 1, &fd, NULL, NULL, &timeout );
	if ( nRet > 0 )
      {
      	if ( FD_ISSET(socket, &fd) ) nRet = 1;
      	else nRet = -1;
      }
  	return nRet;
}

int CRtpService::GetSocket()
{
	if ( m_ClientSocket == NULL ) return -1;
	return m_ClientSocket[m_CurSocket].socket;
}

int CRtpService::CloseAll()
{
	if ( m_ClientSocket == NULL ) return -1;
	for ( int i = 0; i < m_ClientNum; ++i )
    	Close( m_ClientSocket[i].socket );
	if ( m_TcpSocket != -1 ) close( m_TcpSocket );
	if ( m_RtpSocket != -1 ) close( m_RtpSocket );
	return 0;
}

int CRtpService::Close( int socket )
{
	int nRet = -1;
	if ( socket != -1 )
    {
    	nRet = close( socket );
    	DelSocket( socket );
    }
	return nRet;
}

void CRtpService::HeartBeat()
{
	time_t curTime = time( NULL );
	for ( int i = 0; i < m_ClientNum; ++i )
    {
    	int difTime = curTime - m_ClientSocket[i].timeStamp;
    	if ( difTime < 0 || difTime > MAX_HEART_BEAT_TIME+10 )
        {
        	difTime = 0;
        	m_ClientSocket[i].timeStamp = curTime;
        }
    	if ( difTime > MAX_HEART_BEAT_TIME )
        {
        	FiPrint( "HeartBeat: Client Socket %d Timeout .\r\n", m_ClientSocket[i].socket );
        	Close( m_ClientSocket[i].socket );
        }
    }
}

int CRtpService::SetFlag( int channel )
{
	if ( 0 <= channel && channel < REAL_CHANNEL_NUM )
    {
    	m_ClientSocket[m_CurSocket].flag[channel] = 1;
    	m_channelRequestFlag[channel]++;
    	if( 1 == m_channelRequestFlag[channel] )
        {
        	m_curChannelNum++;
        }
    	return 0;
    }
	return -1;
}

int CRtpService::ClearFlag( int channel )
{
	if ( 0 <= channel && channel < REAL_CHANNEL_NUM )
    {
    	m_ClientSocket[m_CurSocket].flag[channel] = 0;            
    	m_channelRequestFlag[channel]--;
    	if( 0 == m_channelRequestFlag[channel] )
        {
        	m_curChannelNum--;
        }
    	return 0;
    }
	return -1;
}

int CRtpService::SetPort( unsigned short port )
{
	int	nRet	    = -1;
	int	startFlag	= 0;
	for ( int channel = 0; channel < REAL_CHANNEL_NUM; ++channel )
    {
    	if ( m_ClientSocket[m_CurSocket].flag[channel] )
        {
        	startFlag = 1;
        	break;
        }
    }    
	if ( startFlag == 0 )
    {
    	m_ClientSocket[m_CurSocket].addr.sin_port = htons( port );
    	nRet = 0;
    }
	return nRet;
}

int CRtpService::SetAddr( void *ptrAddr )
{
	int	nRet	    = -1;
	int	startFlag	= 0;
	for ( int channel = 0; channel < REAL_CHANNEL_NUM; ++channel )
    {
    	if ( m_ClientSocket[m_CurSocket].flag[channel] )
        {
        	startFlag = 1;
        	break;
        }
    }    
	if ( startFlag == 0 )
    {
    	m_ClientSocket[m_CurSocket].addr = *((struct sockaddr_in *)ptrAddr);
    	nRet = 0;
    }
	return nRet;
}


int CRtpService::ClientNum()
{
	return m_ClientNum;
}

void CRtpService::InitSocket( int clientMax )
{
	m_RtpSocket	    = -1;
	m_TcpSocket	    = -1;
	m_ClientNum	    = 0;
	m_ClientMax	    = 0;
	m_CurSocket	    = -1;
	m_RtpPort	    = 0;
	m_TcpPort	    = 0;

	if ( m_ClientSocket == NULL )
    {
    	m_ClientSocket = ( RTP_SOCKET * )Calloc( 1, sizeof(RTP_SOCKET)*clientMax );
    	if ( m_ClientSocket != NULL )
        {
        	m_ClientMax = clientMax;
        	for ( int i = 0; i < m_ClientMax; ++i )
            	m_ClientSocket[i].socket = -1;
        }
    }

	memset( m_channelRequestFlag, 0x00, sizeof(m_channelRequestFlag) );
	m_curChannelNum = 0;
}

int CRtpService::AddSocket( int socket, struct sockaddr_in &addr )
{    
	if ( m_ClientSocket == NULL ) InitSocket( RTP_DEFAULT_MAX_CLIENT );
	if ( m_ClientSocket == NULL ) return -1;

	if ( m_ClientNum < m_ClientMax )
    {
    	m_ClientSocket[m_ClientNum].socket	    = socket;
    	m_ClientSocket[m_ClientNum].state	    = 0;
    	m_ClientSocket[m_ClientNum].timeStamp	= 0;
    	m_ClientSocket[m_ClientNum].dataLen	    = 0;
    	m_ClientSocket[m_ClientNum].dataBuf	    = ( unsigned char * )Malloc( RTP_DATA_BUF_SIZE );
    	m_ClientSocket[m_ClientNum].addr	    = addr;

    	if ( m_ClientSocket[m_ClientNum].dataBuf != NULL )
        	m_ClientSocket[m_ClientNum].bufSize = RTP_DATA_BUF_SIZE;
    	else m_ClientSocket[m_ClientNum].bufSize = 0;
        
    	memset( m_ClientSocket[m_ClientNum].seq, 0,
            	sizeof(m_ClientSocket[m_ClientNum].seq) );            
    	memset( m_ClientSocket[m_ClientNum].flag, 0,
            	sizeof(m_ClientSocket[m_ClientNum].flag) );
    	m_ClientNum++;
    	return 0;
    }

	return -1;
}

int CRtpService::DelSocket( int socket )
{
	if ( m_ClientSocket == NULL ) return -1;

	for ( int i = 0; i < m_ClientNum; ++i )
    {
    	if ( m_ClientSocket[i].socket == socket )
        {
        	for( int j = 0; j < REAL_CHANNEL_NUM; ++j )
            {
            	if( 1 == m_ClientSocket[i].flag[j] )
                {                    
                	m_channelRequestFlag[j]--;
                	if( 0 == m_channelRequestFlag[j] )
                    {
                    	m_curChannelNum--;
                    }
                }
            }
        	if ( i != m_ClientNum-1 )
            {
            	do {
                	m_ClientSocket[i] = m_ClientSocket[i+1];
                } while ( ++i < m_ClientNum-1 );
            }
            
        	memset( &m_ClientSocket[i], 0, sizeof(m_ClientSocket[i]) );
        	m_ClientSocket[i].socket = -1;
        	m_ClientNum--;
        	return 0;
        }
    }
	return -1;
}

/*
* fn: 获取某个通道是否有用户正在请求视频
* 返回: > 0, 表示有一个以上的用户正在请求视频; 否则,没有用户在请求该通道
*/
int CRtpService::GetChannelRequestFlag( int channel )
{
	return m_channelRequestFlag[channel];
}

/*
* fn: 获取当前有多少个通道被请求视频流
*/
int CRtpService::GetCurChannelNum()
{
	return m_curChannelNum;
}

