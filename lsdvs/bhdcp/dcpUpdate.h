/********************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此头文件提供了DVS升级数据传输协议的包头数据结构定义
**
**  version       :  1.0
**  author        :  SVEN
********************************************************************************/

#ifndef __DCPUPDATE_H__
#define __DCPUPDATE_H__

// 升级请求发送文件包类型
typedef enum _UpdateFileSubtype_
{
    UPDATE_FILE_DATA_SEGMENT = 0,   // 传送文件数据包
    UPDATE_FILE_LAST_SEGMENT,       // 最后一个数据包
} UPDATE_FILE_SUBTYPE_EN;

// 处理接收文件时返回的状态
typedef enum _DcpFileCode_
{
    DCP_FILE_COMPLETE = 0,  // 完成接收
    DCP_FILE_RECEIVE,       // 正在接收
    DCP_FILE_ERROR,         // 接收出错
} DCP_FILE_CODE_EN;

// 写FLASH进度结构体
typedef struct _WriteFlashProgress_
{
    int percent;
    char reserved[20];
} WRITE_FLASH_PROGRESS_T;

// 创建升级线程需要的参数 
typedef struct _UpdateParam_
{
	unsigned int 	fileSize;        // 升级文件大小
	int         	socket;            // 升级交互的socket
} UPDATE_PARAM_T;

// 维护升级交互通信结构
typedef struct _UpdateCom_
{
	int 	socket;                // 通信socket
	char     *readBuf;            // 接收缓冲区	
	uint 	readBufSize;        // 接收缓冲区的大小
	uint 	readBufDataSize;    // 接收缓冲区里面当前的数据大小
	char     *writeBuf;            // 发送缓冲区
	uint	writeBufSize;        // 发送缓冲区的大小
	uint	writeBufDataSize;    // 发送数据buffer里面的数据大小
} UPDATE_COM_T;

int DcpCreateUpdateThread(uint fileSize, int socket );
int IsDcpUpdateThreadRun();

void StartUpdateFlashThread();
void StopUpdateFlashThread();

#endif

