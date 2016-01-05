/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此头文件提供了DVS传输协议的包头数据结构定义
**  参考文档: <<DVS传输协议.doc>> V1.0
**  date           :  2013.11.11
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#ifndef _MESSAGE_PROTOCOL_H
#define _MESSAGE_PROTOCOL_H

const unsigned int	MSG_DATA_FLAG	    = 0xABBA3AA3;    // 协议标识
const int	    	MSG_CFG_DATA_PORT	= 6010;            // 端口号
const int	    	MSG_CFG_DATA_BAUD	= 9600;            // 波特率

enum MsgDataType
{
	MSG_CFG_DATA_TYPE	        = 0x01,        // 配置协议消息类型
	MSG_AVS_DATA_TYPE	        = 0x02,        // 流媒体控制命令
	MSG_CTL_DATA_TYPE	        = 0x03,        // 控制命令
	MSG_UPL_DATA_TYPE	        = 0x04,        // 事件上报
	MSG_DEV_SEARCH_TYPE	        = 0x05,        // 设备搜索
	MSG_DATA_TRANSMIT_TYPE	    = 0x06,        // 传输协议
	MSG_DATA_REPORT_TYPE	    = 0x07,        // 报表协议
};

#define PACK_ALIGN	__attribute__((packed))

typedef struct messageData
{
	unsigned int	msgFlag;        // 消息标识
	unsigned char	msgType;        // 消息类型
	unsigned char 	msgData[1];        // 消息内容
    
} PACK_ALIGN MESSAGE_DATA;

#undef PACK_ALIGN

const int MAX_DATA_BUF_SIZE = 16 * 1024;

#endif  // _MESSAGE_PROTOCOL_H

