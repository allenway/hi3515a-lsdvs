#ifndef __DCPSTREAM_H__
#define __DCPSTREAM_H__

#include "const.h"
typedef enum _DcpStreamErr_
{
	DCP_STREAM_ERR_QUIT = 100,            // 推出线程
	DCP_STREAM_ERR_CHANNEL_IS_EXIST,    // 通道已经存在, 适应客户端对同一个通道请求多次
} DCP_STREAM_ERR_EN;

typedef struct _StreamCh_
{
	int         	ch;        // 通道号
	int         	socket;    // 发流的socket
	unsigned int 	fd;        // 取流的fd
} STREAM_CH_T;

typedef struct _DcpStream_
{
	int         	csnum;                        // 当前发送的通道数
	STREAM_CH_T 	streamCh[MAX_CHANNEL_NUM];  // 通道索引,存放
} DCP_STREAM_T;

int DcpCreateStreamThread( uint *threadId );
void DcpDestoryStreamThread( uint threadId );
int DcpStartChannelStream( uint threadId, int channel, int socket );
int DcpStopChannelStream( uint threadId, int channel );

#endif 

