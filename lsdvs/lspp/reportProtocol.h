/*
*******************************************************************************
**  Copyright (c) 2011, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**	文件名: 
**  description  : 
**  功能:    	用于与DVSClient通讯返回报表数据
**  date           :  2011.08.18
**
**  version       :  1.0
**  author        :  ronat
*******************************************************************************
*/

#ifndef _REPORT_PROTOCOL_H_2011_0818_
#define _REPORT_PROTOCOL_H_2011_0818_

enum MsgReportDataPackType
{
	MSG_REPORT_PC_GETDAILYDATA_REQUEST	= 0x0100,        // 取数人日报表统计数据
	MSG_REPORT_PC_GETMONTHDATA_REQUEST	= 0x0200,        // 取数人月报表统计数据
};

enum MsgReportDataSubType
{
	MSG_REPORT_REQUEST	                = 0x01,        // 请求
	MSG_REPORT_RESPONSE	                = 0x02,        // 应答
	MSG_REPORT_ERROR	                = 0x03,        // 出错
};

#ifdef WIN32
#define PACK_ALIGN
#pragma pack(push,1)
#else
#define PACK_ALIGN	__attribute__((packed))
#endif //~

typedef struct ReportPcDataRequest
{
	unsigned char 	channel;                // 通道号
	unsigned char 	startTime[20];            // 开始时间 形如2011-08-18 00:00:00
	unsigned char 	endTime[20];            // 结束时间
	unsigned int  	interval;                // 时间间隔，秒
	char	    	reserved[32];            // 保留
    
} PACK_ALIGN REPORT_PC_DATA_REQUEST;




#ifdef WIN32
#pragma pack(pop,1)
#else
#undef PACK_ALIGN
#endif //~


int DealReportDataProcess( unsigned char *dataBuf, int &dataLen, const int bufSize );
int CheckReportDataProcess( unsigned char *dataBuf, int dataLen, int bufSize, int &offset );

#endif  //~_REPORT_PROTOCOL_H_2011_0818_

