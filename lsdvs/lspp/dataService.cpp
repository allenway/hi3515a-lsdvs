/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此文件实现了对IO进行多路复用的基类。
**  date           :  2013.11.11
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <termios.h>
#include "debug.h"
#include "malloc.h"
#include "netSocket.h"
#include "dataService.h"
#include "linuxFile.h"

static const int DATA_SERVICE_INIT_NUM	    = 10;
static const int DATA_BUF_SIZE	            = 16 * 1024;    /* 数据缓冲区大小 */
static const int RS485_CONTROL_SET_GPIO45	= 3;
static const int RS485_CONTROL_CLR_GPIO45	= 4;
static const int MAX_HEART_BEAT_TIME	    = 180;            /* 心跳函数最大等待时间 */

CDataService::CDataService()
{
	InitSocket();
}
    
CDataService::~CDataService()
{
	CloseAll();
	if ( m_dataSocket ) 
    {
    	for ( int i = 0; i < m_socketNum; ++i )
        	Free( m_dataSocket[i].dataBuf );
    	Free( m_dataSocket );
    }
}

int CDataService::Init( int type, int port )
{
	int		nRet	        = -1;
	int		serviceSocket	= -1;
	int		serviceType	    = type & m_serviceType;
	switch ( serviceType )
    {
	case TCP_SERVICE:
    	nRet = SocketTcpListen( &serviceSocket, port, SOCKET_NOBLOCK );
    	break;
        
	case UDP_SERVICE:
    	nRet = SocketUdpListen( &serviceSocket, port );
    	if ( nRet != -1 ) SetSockOpt( serviceSocket );
    	break;
        
    	default:
    	FiPrint( "Please check serviceType!\r\n" );
    	break;
    }

	if ( nRet != -1 )
    {
    	AddSocket( serviceSocket, type );
    	m_serviceType &= ~type;
    }

	return nRet;
}

int CDataService::Send( void *data, int len )
{
	int nRet = -1;
    
	if ( m_dataSocket == NULL ) return nRet;

	int i = m_curSocket;
	if ( m_dataSocket[i].state )
    {
    	switch ( m_dataSocket[i].type )
        {
    	case TCP_SERVICE:
        	nRet = 0;
        	break;
            
    	case UDP_SERVICE:
        	nRet = sendto( m_dataSocket[i].socket, data, len, 0,
                    (struct sockaddr *)&m_clientAddr, sizeof(m_clientAddr) );
        	break;
            
    	case TCP_SOCKET:
        	nRet = Writen( m_dataSocket[i].socket, data, len );
        	break;
        }
    }

	return nRet;
}

// 将数据发送到所有的TCP客服端,仅用于TCP
int CDataService::SendtoAllTCPClient( void *data, int len )
{
	int nRet = -1;
	int i = 0;
    
	if ( m_dataSocket == NULL ) return nRet;

//	int i = m_curSocket;
	for( i = 0; i < m_socketNum; i++)
    {
//    	if ( m_dataSocket[i].state )
        {
        	if( m_dataSocket[i].type == TCP_SOCKET )
            {
FiPrint("socket:%d\n",m_dataSocket[i].socket);
            	nRet = Writen( m_dataSocket[i].socket, data, len );
            	if( nRet < 0 )
                {                
                	Close(m_dataSocket[i].socket);
                }
            }
        }
    }

	return nRet;
}


int CDataService::Recv( void *data, int len )
{
	int nRet	        = -1;
	int addrLen	        = 0;
    
	if ( m_dataSocket == NULL ) return nRet;

	for ( int i = 0; i < m_socketNum; ++i )
    {
    	if ( m_dataSocket[i].state )
        {            
        	switch ( m_dataSocket[i].type )
            {
        	case TCP_SERVICE:
            	nRet = 0;
            	break;
                
        	case UDP_SERVICE:
            	addrLen = sizeof( m_clientAddr );
            	nRet = recvfrom( m_dataSocket[i].socket, data, len, 0, 
                            (struct sockaddr *)&m_clientAddr, (socklen_t *)&addrLen );
            	break;    
                
        	case TCP_SOCKET:
            	nRet = recv( m_dataSocket[i].socket, data, len, 0 );
            	if ( nRet > 0 ) m_dataSocket[i].timeStamp = time( NULL );
            	break;
            }

        	m_curSocket = i;
        	break;
        }
    }

	return nRet;
}

/*
* fn: ql add,和Recv 的区别是他会调用readn,而不是read
*/
int CDataService::Recvn( void *data, int len )
{
	int nRet	        = -1;
	int addrLen	        = 0;
    
	if ( m_dataSocket == NULL ) return nRet;

	for ( int i = 0; i < m_socketNum; ++i )
    {
    	if ( m_dataSocket[i].state )
        {            
        	switch ( m_dataSocket[i].type )
            {
        	case TCP_SERVICE:
            	nRet = 0;
            	break;
                
        	case UDP_SERVICE:
            	addrLen = sizeof( m_clientAddr );
            	nRet = recvfrom( m_dataSocket[i].socket, data, len, 0, 
                            (struct sockaddr *)&m_clientAddr, (socklen_t *)&addrLen );
            	break;
                
        	case TCP_SOCKET:
            	nRet = recv( m_dataSocket[i].socket, data, len, 0 );
            	if ( nRet > 0 ) m_dataSocket[i].timeStamp = time( NULL );
            	break;
            }

        	m_curSocket = i;
        	break;
        }
    }

	return nRet;
}

int CDataService::Recv()
{
	int nRet	        = -1;
	int addrLen	        = 0;
    
	if ( m_dataSocket == NULL ) return nRet;
    
	for ( int i = 0; i < m_socketNum; ++i )
    {
    	if ( m_dataSocket[i].state && m_dataSocket[i].dataBuf )
        {            
        	unsigned int	nDataLen = m_dataSocket[i].bufSize;
        	unsigned char *	pDataBuf = m_dataSocket[i].dataBuf;
        	pDataBuf += m_dataSocket[i].dataLen;
        	nDataLen -= m_dataSocket[i].dataLen;

        	switch ( m_dataSocket[i].type )
            {
        	case TCP_SERVICE:
            	nRet = 0;
            	break;
                
        	case UDP_SERVICE:
            	addrLen = sizeof( m_clientAddr );
            	nRet = recvfrom( m_dataSocket[i].socket, pDataBuf, nDataLen, 0, 
                            (struct sockaddr *)&m_clientAddr, (socklen_t *)&addrLen );
            	break;
                
        	case TCP_SOCKET:
            	nRet = recv( m_dataSocket[i].socket, pDataBuf, nDataLen, 0 );
            	if ( nRet > 0 ) m_dataSocket[i].timeStamp = time( NULL );
            	break;
            }

        	if ( nRet > 0 ) 
            {    
            	m_dataSocket[i].dataLen += nRet;
            	nRet = m_dataSocket[i].dataLen;
            }
            
        	m_curSocket = i;
        	break;
        }
    }
	return nRet;
}

int CDataService::GetData( void *data, int len )
{
	if ( m_dataSocket == NULL ) return -1;
	int i = m_curSocket;
	int nDataLen = m_dataSocket[i].dataLen;
	if ( len < nDataLen ) nDataLen = len;
	if ( nDataLen > 0 ) memcpy( data, m_dataSocket[i].dataBuf, nDataLen );

	return nDataLen;
}

int CDataService::DelData( int len )
{
	int nRet = 0;
	if ( m_dataSocket == NULL ) return nRet;
	int i = m_curSocket;
	if ( len > 0 )
    {
    	unsigned char *pDataBuf = m_dataSocket[i].dataBuf;
    	if ( pDataBuf != NULL )
        {
        	if ( len >= m_dataSocket[i].dataLen ) 
            {
            	nRet = m_dataSocket[i].dataLen;
            	m_dataSocket[i].dataLen = 0;
            }
        	else
            {
            	m_dataSocket[i].dataLen -= len;
            	memmove( pDataBuf, pDataBuf+len, m_dataSocket[i].dataLen );
            	nRet = len;
            }
        }
    }    
	return nRet;
}

int CDataService::Accept()
{
	int nRet = -1;
	if ( m_dataSocket == NULL ) return nRet;
	for ( int i = 0; i < m_socketNum; ++i )
    {
    	if ( m_dataSocket[i].type == TCP_SERVICE )
        {
        	if ( m_dataSocket[i].state == 0 ) break;
        	else m_curSocket = i;
            
        	struct sockaddr_in clientAddr;
        	int len = sizeof( clientAddr );    
        	int clientSocket = accept( m_dataSocket[i].socket, 
                                    (struct sockaddr *)&clientAddr, (socklen_t *)&len );
        	if ( clientSocket > 0 ) 
            {    
            	FiPrint( "## Accept: IP = %s, PORT = %d.\r\n",
                    	inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port) );
            	nRet = AddSocket( clientSocket, TCP_SOCKET );
            	if ( nRet == -1 ) 
                {
                	FiPrint( "AddSocket(%d) failed!\r\n", clientSocket );
                	close( clientSocket );
                	nRet = -2;
                }
            	else nRet = clientSocket;
            }
        	else
            {
            	nRet = -2;
            	FiPrint( "Accept Error:%s!\r\n", STRERROR_ERRNO );
            }
        }        
    }
	return nRet;
}

int CDataService::Select()
{
	if ( m_dataSocket == NULL ) return -1;
    
	fd_set fd; 
	FD_ZERO( &fd );
  	int maxfd = 0;
    for ( int i = 0; i < m_socketNum; ++i )
      {
      	FD_SET( m_dataSocket[i].socket, &fd );
      	if ( m_dataSocket[i].socket > maxfd )
          {
          	maxfd = m_dataSocket[i].socket;
          }
      }
	struct timeval timeout;
	timeout.tv_sec	= 1;
  	timeout.tv_usec	= 0;
  	int nRet = select( maxfd + 1, &fd, NULL, NULL, &timeout );
	if ( nRet > 0 )
      {
      	for ( int i = 0; i < m_socketNum; ++i )
          {
          	if ( FD_ISSET(m_dataSocket[i].socket, &fd) ) m_dataSocket[i].state = 1;
        	else m_dataSocket[i].state = 0;
          }
      }
  	return nRet;
}

int CDataService::CloseAll()
{
	if ( m_dataSocket == NULL ) return -1;
	for ( int i = 0; i < m_socketNum; ++i )
    {
    	Close( m_dataSocket[i].socket );
    }
	return 0;
}

int CDataService::GetSocket()
{
	if ( m_dataSocket == NULL ) return -1;
	return m_dataSocket[m_curSocket].socket;
}

int CDataService::Close( int socket )
{
	int nRet = -1;
	if ( socket != -1 )
    {
    	nRet = close( socket );
    	DelSocket( socket );
    }
	return nRet;
}

void CDataService::HeartBeat()
{
	time_t curTime = time( NULL );
	for ( int i = 0; i < m_socketNum; ++i )
    {
    	if ( m_dataSocket[i].type == TCP_SOCKET )
        {
        	int difTime = curTime - m_dataSocket[i].timeStamp;        
        	if ( difTime < 0 || difTime > MAX_HEART_BEAT_TIME+10 )
            {
            	difTime = 0;
            	m_dataSocket[i].timeStamp = curTime;
            }
        	if ( difTime > MAX_HEART_BEAT_TIME )
            {
            	FiPrint( "HeartBeat: Client Socket %d Timeout .\r\n", m_dataSocket[i].socket );
            	Close( m_dataSocket[i].socket );
            }
        }
    }
}

#define MULTICAST_SERVER         "234.168.18.10"
#define MULTICAST_CLIENT         "234.168.18.11"

int CDataService::SetSockOpt( int socket )
{
    // 设置发送缓冲区64K, 接收缓冲区16K
	int sendBufSize = 64*1024;
	int recvBufSize = 16*1024;
	setsockopt( socket, SOL_SOCKET, SO_SNDBUF, (char *)&sendBufSize, sizeof(sendBufSize) );
	setsockopt( socket, SOL_SOCKET, SO_RCVBUF, (char *)&recvBufSize, sizeof(recvBufSize) );

    // 设置支持广播和组播
	int bBroadcast = 1, nTtl = 10;
	setsockopt( socket, SOL_SOCKET, SO_BROADCAST, &bBroadcast, sizeof(int) );
	setsockopt( socket, IPPROTO_IP, IP_MULTICAST_TTL, &nTtl, sizeof(int) );

	struct ip_mreq mreq;
	memset( &mreq, 0, sizeof(mreq) );
	mreq.imr_multiaddr.s_addr = inet_addr( MULTICAST_SERVER );
	mreq.imr_interface.s_addr = htonl( INADDR_ANY );
	int nRet = setsockopt( socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq) );
	if ( nRet == -1 ) FiPrint( "Don't Support Multicast.\r\n" );
	return nRet;
}

void CDataService::InitSocket()
{
	m_socketNum	= 0;
	m_curSocket	= -1;
	m_baud	    = 9600;

	if ( m_dataSocket == NULL )
    {
    	memset( &m_clientAddr, 0, sizeof(m_clientAddr) );
    	m_serviceType = TCP_SERVICE | UDP_SERVICE | RS232_SERVICE | RS485_SERVICE;
    	m_dataSocket = ( DATA_SOCKET * )Malloc( sizeof(DATA_SOCKET) * DATA_SERVICE_INIT_NUM );
    	if ( m_dataSocket != NULL )
        {
        	m_socketMax = DATA_SERVICE_INIT_NUM;
        	for ( int i = 0; i < m_socketMax; ++i )
            {
            	m_dataSocket[i].socket	    = -1;
            	m_dataSocket[i].type	    = 0;
            	m_dataSocket[i].state	    = 0;
            	m_dataSocket[i].timeStamp	= 0;
            	m_dataSocket[i].dataBuf	    = NULL;
            	m_dataSocket[i].dataLen	    = 0;
            	m_dataSocket[i].bufSize	    = 0;
            }
        }
    	else
        {
        	m_socketMax = 0;
        }
    }
}

int CDataService::AddSocket( int socket, int type )
{    
	if ( m_dataSocket == NULL ) InitSocket();
    
	if ( m_dataSocket != NULL )
    {
    	if ( m_socketNum == m_socketMax )
        {
        	DATA_SOCKET *dataSocket = ( DATA_SOCKET * )Realloc( m_dataSocket,
                                            	sizeof(DATA_SOCKET) * m_socketMax * 2 );
        	if ( dataSocket != NULL )
            {
            	m_dataSocket = dataSocket;
            	memset( m_dataSocket + m_socketMax, 0,
                    	sizeof(DATA_SOCKET) * m_socketMax );
            	m_socketMax *= 2;
            }
        }

    	if ( m_socketNum < m_socketMax )
        {
        	m_dataSocket[m_socketNum].socket	= socket;
        	m_dataSocket[m_socketNum].type	    = type;
        	m_dataSocket[m_socketNum].state	    = 0;
        	m_dataSocket[m_socketNum].timeStamp	= 0;
        	m_dataSocket[m_socketNum].dataLen	= 0;
        	m_dataSocket[m_socketNum].dataBuf	= ( unsigned char * )Malloc( DATA_BUF_SIZE );

        	if ( m_dataSocket[m_socketNum].dataBuf ) 
            	m_dataSocket[m_socketNum].bufSize = DATA_BUF_SIZE;
        	else m_dataSocket[m_socketNum].bufSize = 0;
        	m_socketNum++;
        	return 0;
        }
    }
	return -1;
}

int CDataService::DelSocket( int socket )
{
	if ( m_dataSocket != NULL )
    {
    	for ( int i = 0; i < m_socketNum; ++i )
        {
        	if ( m_dataSocket[i].socket == socket )
            {
            	Free( m_dataSocket[i].dataBuf );

            	if ( i != m_socketNum-1 )
                {
                	do
                    {
                    	m_dataSocket[i] = m_dataSocket[i+1];
                        
                    } while ( ++i < m_socketNum-1 );
                }
                
            	m_dataSocket[i].socket	    = -1;
            	m_dataSocket[i].type	    = 0;
            	m_dataSocket[i].state	    = 0;
            	m_dataSocket[i].timeStamp	= 0;
            	m_dataSocket[i].dataBuf	    = NULL;
            	m_dataSocket[i].dataLen	    = 0;
            	m_dataSocket[i].bufSize	    = 0;
            	m_socketNum--;
            	return 0;
            }
        }
    }
	return -1;
}
    
