/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此头文件提供了对音视频发送缓冲区进行操作的相关函数的接口
**  date           :  2013.11.11
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#ifndef _AV_SEND_LIST_H
#define _AV_SEND_LIST_H

typedef struct videoSendNode
{
	unsigned int	timeStamp;        // 时间戳
	unsigned char	frameType;        // 帧类型
	unsigned int	frameLen;        // 帧长度
	unsigned char *	frameData;        // 帧内容
} VIDEO_SEND_NODE;

void WaitVideoSendList();
void SignalVideoSendList();
int PopVideoSendList( int channel, VIDEO_SEND_NODE *vFrame );
int PutVideoSendList( int channel, VIDEO_SEND_NODE *vFrame );
void ClearVideoSendList();

#endif  // _AV_SEND_LIST_H

