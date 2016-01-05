/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此文件提供了对对讲发送线程使用的相关结构体定义
**  date           :  2013.12.20
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#ifndef _TALKBACK_THREAD_H
#define _TALKBACK_THREAD_H

const unsigned int	MSG_TALKBACK_FLAG	    = 0xABBA5CC5;    // 协议标识
const unsigned char MSG_TALKBACK_TYPE         = 0x20;            // 协议消息类型

//
// 数据包类型
//
enum MsgTalkbackPackType
{
	MSG_TALKBACK_HEART_BEAT	        = 0x0000,        // 心跳
	MSG_TALKBACK_START_AUDIO	    = 0x0010,        // 开始音频发送
	MSG_TALKBACK_STOP_AUDIO	        = 0x0020,        // 停止音频发送
};

//
// 数据子类型
//
enum MsgAvsSubType
{
	MSG_TALKBACK_REQUEST	    = 0x01,        // 请求
	MSG_TALKBACK_RESPONSE	    = 0x02,        // 应答
	MSG_TALKBACK_ERROR	        = 0x03,        // 出错
};

#define PACK_ALIGN	__attribute__((packed))

//
// 数据头格式
//
typedef struct TalkbackPackHead
{
	unsigned int	msgFlag;        // 消息标识
	unsigned char	msgType;        // 消息类型
	unsigned short	packSn;            // 包序号
	unsigned int	packType;        // 包类型
	unsigned char	subType;        // 子类型
	unsigned short	len;            // 数据长度
    
} PACK_ALIGN TALKBACK_PACK_HEAD;

//
// 数据包格式
//
typedef struct TalkbackPack
{
	TALKBACK_PACK_HEAD head;        // 包头数据
	unsigned char data[1];            // 包含数据部分和2字节校验和
    
} PACK_ALIGN TALKBACK_PACK;

#undef PACK_ALIGN

void *DealTalkbackThread( void *args );

#endif  // _TALKBACK_THREAD_H

