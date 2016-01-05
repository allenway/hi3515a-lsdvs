#ifndef __DCPRECDOWNLOAD_H__
#define __DCPRECDOWNLOAD_H__

#include "dcpTypes.h"

#define MAX_RECORD_DATA_SEND_PACKET_SIZE (8*1024) //每次发送8 k

typedef struct _RecSendPacket_
{
	STREAM_PACK_HEAD_T packHead;
	char dataBuf[MAX_RECORD_DATA_SEND_PACKET_SIZE]; // 存放从文件读到的录像内容
} REC_SEND_PACKET_T;

typedef struct _RecDownload_
{
	int socket;    // 发送的socket
	int fd;        // 录像文件的句柄
	int len;     // 发送的长度
	REC_SEND_PACKET_T sendPack;
} REC_DOWNLOAD_T;

int DcpStartRecDownload( int socket, int fd );

#endif 

