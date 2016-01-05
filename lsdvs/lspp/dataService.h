/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此文件提供了对IO进行多路复用的基类的接口。
**  date           :  2013.11.11
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#ifndef _DATA_SERVICE_H
#define _DATA_SERVICE_H

#include <netinet/in.h>

static const int TCP_SERVICE	            = 0x01;
static const int UDP_SERVICE	            = 0x02;
static const int RS232_SERVICE	            = 0x04;
static const int RS485_SERVICE	            = 0x08;
static const int TCP_SOCKET	                = 0x10;

typedef struct dataSocket
{
	int socket;
	int type;            // socket类型:tcp, udp, rs485, rs232 
	int state;            // 接收缓冲区状态: 1:有数据,0:无数据.
	time_t timeStamp;    // 时间戳
    
	unsigned char *dataBuf;
	int dataLen;
	int bufSize;

} DATA_SOCKET;

class CDataService
{
public:
	CDataService();
    ~CDataService();
	int Init( int type, int port );    
	int Send( void *data, int len );
	int Recv( void *data, int len );
	int Recvn( void *data, int len );

    // 带接收缓冲区的Recv()
    // 函数GetData从缓冲区中获取len字节的数据
    // 函数DelData从缓冲区中删除len字节的数据
	int Recv();
	int GetData( void *data, int len );
	int DelData( int len );
	int ClearRecvBuf( int len );
	int Accept();
	int Select();
	int CloseAll();
	int GetSocket();
	int Close( int socket );
	void HeartBeat();
	int SendtoAllTCPClient( void *data, int len );

private:
	int SetSockOpt( int socket );
	void InitSocket();
	int AddSocket( int socket, int type );
	int DelSocket( int socket );

private:
	DATA_SOCKET *m_dataSocket;
	int m_socketNum;
	int m_socketMax;
	int m_serviceType;
	int m_curSocket;
	int m_baud;
public:
	int m_serviceSocket;    // 供rs232设置波特率时使用
    

	struct sockaddr_in m_clientAddr;
};

typedef int (*ID_CMD_ACTION)( unsigned char *dataBuf, int &dataLen, const int bufSize );

typedef struct idCmd
{
	unsigned int	id;
	ID_CMD_ACTION	cmd;
    
} ID_CMD;

class CIdCmd 
{
public:    
	CIdCmd(): m_IdCmd(NULL), m_Size(0) {}
    ~CIdCmd() {}

	void Init( ID_CMD *pIdCmd, int size )
    {
    	m_IdCmd = pIdCmd;
    	m_Size = size;
    }    

	ID_CMD_ACTION GetCmd( unsigned id )
    {
    	for ( int i = 0; i < m_Size; ++i )
        {
        	if ( id == m_IdCmd[i].id )
            {
            	return m_IdCmd[i].cmd;
            }
        }
    	return NULL;
    }

private:
	ID_CMD *m_IdCmd;
	int m_Size;
};

#endif // _DATA_SERVICE_H


