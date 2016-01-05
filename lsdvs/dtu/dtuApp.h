#ifndef __DTUAPP_H__
#define __DTUAPP_H__

typedef struct _DtuWorkParam_
{
	uchar 	enable;                        // 0,非使能; 1,使能
	uchar 	transProtocol;                // 传输协议, 0:tcp; 1,udp
	ushort	serverPort;                    // 服务器端口
	char	severIp[NET_ADDRSIZE];        // 服务器IP 地址
	char	heartbeatContent[76];        // 心跳包的内容,最多32 个字节,可包含文字、数字、英文大小写字母等
	ushort	interval;                    // 心跳包的发送间隔1～65535s
} DTU_WORK_PARAM_T;

typedef struct _DtuWorkMaintain_
{
	int	pts;            // 时间戳,相对的
	int tcpMode;        // 0,长连接; 1,短链接
	int socket;            // tcp 通信的socket, -1 表示没有连接, > 0 表示有连接
	char hyalineBuf[1024];  // 透明传输内容
	uint hyalineLen;        // 透明传输长度
} DTU_WORK_MAINTAIN_T;

void StartDtuAppThread();
void StopDtuAppThread();
void DtuSendParamChangeMessage();

#endif //__DTUAPP_H__

