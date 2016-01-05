/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 管理3G的AT业务
**  date           :  2014.9.25
**
**  version       :  1.0
**  author        :  sven
********************************************************************************/
#ifndef __THREEGAT_H__
#define __THREEGAT_H__

typedef struct _ThreegAtCommunicate_
{
	int         	socket;            //通信socket
	char             *sockBuf;        //socket buffer	
	unsigned int 	sockDataSize;    //接收缓冲区里面当前的数据大小
	char             *anaBuf;        //分析数据buffer
	unsigned int	anaDataSize;    //分析数据buffer里面的数据大小
}THREEG_AT_COMMUNICATE;

typedef int (*THREEG_AT_FUNC)( THREEG_AT_COMMUNICATE *pThreegAt, char *pLine );

typedef struct _ThreegAtMsgFunc_
{
	char         	msgKey[32];    // 消息关键字
	THREEG_AT_FUNC	func;        // 对应改消息处理的函数	
} THREEG_AT_MSG_FUNC;

int ThreegAtStartService();
int ThreegAtStopService();
int ThreegAtInit();

#endif 

