/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此头文件提供了DVS数据传输协议的包头数据结构定义
**  参考文档: <<DVS传输协议.doc>> V1.0
**  date           :  2013.12.07
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#ifndef _TRANSMIT_PROTOCOL_H
#define _TRANSMIT_PROTOCOL_H

enum MsgDataTransmitPackType
{
	MSG_TRANSMIT_UPGRADE_REQUEST	= 0x0100,        // 请求系统升级
	MSG_TRANSMIT_UPGRADE_FILE	    = 0x0200,        // 接收升级文件
	MSG_TRANSMIT_GET_SYSCONFIGURE	= 0x0300,        // 发送系统配置
};

enum MsgDataTransmitSubType
{
	MSG_TRANSMIT_REQUEST	        = 0x01,        // 请求
	MSG_TRANSMIT_RESPONSE	        = 0x02,        // 应答
	MSG_TRANSMIT_ERROR	            = 0x03,        // 出错
	MSG_TRANSMIT_DATA_SEGMENT	    = 0x04,        // 数据分片
	MSG_TRANSMIT_LAST_SEGMENT	    = 0x05,        // 最后一个数据分片
};

int DealDataTransmitProcess( unsigned char *dataBuf, int &dataLen, const int bufSize );
int CheckDataTransmitProcess( unsigned char *dataBuf, int dataLen, int bufSize, int &offset );

#endif  // _TRANSMIT_PROTOCOL_H

