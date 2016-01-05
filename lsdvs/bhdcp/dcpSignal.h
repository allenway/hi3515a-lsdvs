#ifndef __DCPSIGNAL_H__
#define __DCPSIGNAL_H__

#include "const.h"
#include "dcpErr.h"
#include "dcpCom.h"

typedef struct _SelectClientData_
{
	ushort num;        // 检测到多少个客户端
	ushort index[MAX_CLIENT_SOCKET_NUM]; // 有数据各个客户端下标的值
} SELECT_CLIENT_DATA_T;

typedef struct _ClientCommunicate_
{
	int 	socket;                // 通信socket
	int		socketState;        // socket状态,CLIENT_SOCKET_STATE_***
	char     *readBuf;            // 接收缓冲区	
	uint 	readBufSize;        // 接收缓冲区的大小
	uint 	readBufDataSize;    // 接收缓冲区里面当前的数据大小
	char     *writeBuf;            // 发送缓冲区
	uint	writeBufSize;        // 发送缓冲区的大小
	uint	writeBufDataSize;    // 发送数据buffer里面的数据大小
	uint	timestamp;            // 时间戳,用来判断用户心跳时间
	uint	threadId;            // 对每个登录的客户建立一个线程,来管理流发送, 在dcpss.cpp 中用来决定请求的视频socket 由那个线程来管理
	ushort	index;                // 该客户端对应连接在整个对象中的索引,这个在本地线程间通信会用到
} CLIENT_COMMUNICATE_T;

typedef struct _ClientConnect_
{
	ushort num;                                            // 目前有多少个连接
	CLIENT_COMMUNICATE_T client[MAX_CLIENT_SOCKET_NUM];    // 最多支持16个用户同时连接
} CLIENT_CONNECT_T;


void StartDcpSignalThread();
void StopDcpSignalThread();
ushort DcpSignalConnectedUserNum(); 

#endif 

