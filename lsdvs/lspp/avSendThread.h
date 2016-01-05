/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此头文件提供了流媒体数据发送模块的接口。
**  date           :  2013.11.11
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#ifndef _AV_SEND_THREAD_H
#define _AV_SEND_THREAD_H

const unsigned int	MSG_AVS_DATA_FLAG	    = 0xABBA5CC5;    // 协议标识
const unsigned char MSG_AVS_DATA_TYPE         = 0x10;            // 协议消息类型

//
// 数据包类型
//
enum MsgAvsPackType
{
	MSG_AVS_HEART_BEAT	        = 0x0000,        // 心跳
	MSG_AVS_START_VIDEO	        = 0x0010,        // 开始视频发送
	MSG_AVS_STOP_VIDEO	        = 0x0020,        // 停止视频发送
	MSG_AVS_SET_VIDEO_PORT	    = 0x0030,        // 设置视频发送端口
};

//
// 数据子类型
//
enum MsgAvsSubType
{
	MSG_AVS_DATA_REQUEST	    = 0x01,        // 请求
	MSG_AVS_DATA_RESPONSE	    = 0x02,        // 应答
	MSG_AVS_DATA_ERROR	        = 0x03,        // 出错
};

#define PACK_ALIGN	__attribute__((packed))

//
// 数据头格式
//
typedef struct AvsPackHead
{
	unsigned int	msgFlag;        // 消息标识
	unsigned char	msgType;        // 消息类型
	unsigned short	packSn;            // 包序号
	unsigned int	packType;        // 包类型
	unsigned char	subType;        // 子类型
	unsigned short	len;            // 数据长度
    
} PACK_ALIGN AVS_PACK_HEAD;

//
// 数据包格式
//
typedef struct AvsPackData
{
	AVS_PACK_HEAD head;                // 包头数据
	unsigned char data[1];            // 包含数据部分和2字节校验和
    
} PACK_ALIGN AVS_PACK_DATA;

#undef PACK_ALIGN

void *DealAVSendThread( void *args );

#endif  // _AV_SEND_THREAD_H

