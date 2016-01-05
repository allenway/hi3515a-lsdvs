/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此头文件提供了对RTP/RTCP协议相关的类的接口。
**  date           :  2013.11.12
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#ifndef _RTP_SERVICE_H
#define _RTP_SERVICE_H

#include "const.h"

const int RTP_PACK_MAX_SIZE	= 1348;//8 * 1024;

#define PACK_ALIGN	__attribute__((packed))

typedef struct rtpHead  /* 小端字节序包头结构	*/
{
	unsigned int	cc	    : 4;    /* CSRC计数	*/
	unsigned int	ext	    : 1;    /* 扩展	    */
	unsigned int	pad	    : 1;    /* 填充	    */
	unsigned int	ver	    : 2;    /* 版本	    */
	unsigned int	payLoad	: 7;    /* 负载类型	*/
	unsigned int	mark	: 1;    /* 标志	    */
	unsigned short	seq;            /* 序列号	*/
	unsigned int	time;            /* 时间戳	*/
	unsigned int	ssrc;            /* 同步源	*/
	unsigned int	end	    : 1;    /* 帧结束位	*/
	unsigned int	begin	: 1;    /* 帧开始位	*/
	unsigned int	sFormat	: 3;    /* 流的格式	*/
	unsigned int	sType	: 3;    /* 流的类型	*/
	unsigned int	channel	: 4;    /* 通道号	*/
	unsigned int	fType	: 4;    /* 帧的类型	*/
	unsigned short	len;            /* 包长度	*/
    
} PACK_ALIGN RTP_HEAD;

#undef	PACK_ALIGN

typedef struct rtpPack
{
	RTP_HEAD		rtpHead;
	unsigned char	rtpData[1];
    
} RTP_PACK;

typedef struct rtpSocket
{
	int socket;
	int state;                    // 写数据缓冲区中是否有数据
	time_t timeStamp;            // 时间戳

	unsigned char *dataBuf;        // 数据接收缓冲区
	int dataLen;                // 当前缓冲区数据长度
	int bufSize;                // 数据缓冲区大小
    
	int flag[MAX_CHANNEL_NUM];    // 是否发生RTP数据的标记
	unsigned short seq[MAX_CHANNEL_NUM];
	struct sockaddr_in addr;
    
} RTP_SOCKET;

class CRtpService
{
public:
	CRtpService();
	CRtpService( int clientMax );
    ~CRtpService();
    
	int InitRtp( int rtpPort );
	int InitTcp( int tcpPort );
	int RtpSend( int channel, void *data, int len,
                	int type, unsigned int time );
	int RtpRecv( void *data, int len, struct sockaddr_in *fromAddr );
	int TcpSend( void *data, int len );
	int TcpRecv( void *data, int len );  // 不带数据缓冲区的接收函数

    // 带接收缓冲区的TcpRecv()
    // 函数GetData从缓冲区中获取len字节的数据
    // 函数DelData从缓冲区中删除len字节的数据	
	int TcpRecv();
	int GetData( void *data, int len );
	int DelData( int len );    
	int Accept();
	int Select();
	int GetSocket();
	int CloseAll();
	int Close( int socket );
	void HeartBeat();
	int SetFlag( int channel );
	int ClearFlag( int channel );
	int SetPort( unsigned short port );    
	int SetAddr( void *ptrAddr );
	int ClientNum();    
	int GetChannelRequestFlag( int channel );    
	int GetCurChannelNum();    
    
private:
	void InitSocket( int clientMax );
	void InitRtpHead( RTP_HEAD *pRtpHead, int seq, unsigned int time,
            	int begin, int end,	int fType, int channel, int len );
	void GetRtpHead( RTP_HEAD *pRtpHead );
	int AddSocket( int socket, struct sockaddr_in &addr );
	int DelSocket( int socket );
	int Select( int socket, int maxMsec );
    
private:
	int m_RtpSocket;
	unsigned short m_RtpPort;
	int m_TcpSocket;
	unsigned short m_TcpPort;
	RTP_SOCKET *m_ClientSocket;    
	int m_ClientNum;
	int m_ClientMax;
	int m_CurSocket;
	int m_channelRequestFlag[REAL_CHANNEL_NUM]; // 该通道目前是否有请求
	int m_curChannelNum;
};

#endif  // _RTP_SERVICE_H

