/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 管理3G的GPS业务
**  date           :  2014.9.25
**
**  version       :  1.0
**  author        :  sven
********************************************************************************/
#ifndef __THREEGGPS_H__
#define __THREEGGPS_H__

#define GPS_LINE_SIZE 1024
#define GPS_MSG_START	"$"
#define GPS_MSG_STOP	"\r\n"
typedef struct _ThreegGpsCommunicate_
{
	int         	socket;            //通信socket
	char             *sockBuf;        //socket buffer	
	unsigned int 	sockDataSize;    //接收缓冲区里面当前的数据大小
	char             *anaBuf;        //分析数据buffer
	unsigned int	anaDataSize;    //分析数据buffer里面的数据大小
}THREEG_GPS_COMMUNICATE;

typedef int (*THREEG_GPS_FUNC)( THREEG_GPS_COMMUNICATE *pThreegGps, char *pLine );
typedef struct _ThreegGpsMsgFunc_
{
	char         	msgKey[32];    // 消息关键字
	THREEG_GPS_FUNC	func;        // 对应改消息处理的函数	
} THREEG_GPS_MSG_FUNC;

int ThreegGpsStartService();
int ThreegGpsStopService();

#endif

