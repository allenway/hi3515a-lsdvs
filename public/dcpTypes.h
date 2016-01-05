#ifndef __DCPTYPES_H__
#define __DCPTYPES_H__

#include "ptypes.h"

#define PACK_ALIGN	__attribute__((packed))

typedef struct _ClientMsgHeadSt_
{
	unsigned int	mark;                // 一条消息开始的标识，=CLIENT_MSG_MARK
	unsigned short	seq;                // 消息队列
	unsigned short 	msgType;             // 消息类型
	unsigned short 	subType;             // 消息子类型
	unsigned short 	ackResult;             // 返回结果
	unsigned int 	len;                 // 消息体长度	    
	unsigned char 	unused[4];
}PACK_ALIGN DCP_HEAD_T;

typedef PARAM_CONFIG_BASE_INFO             	CONFIG_BASE_INFO;
typedef	PARAM_CONFIG_NETWORK	        	CONFIG_NETWORK;              
typedef	PARAM_CONFIG_CLIENT_USER          	CONFIG_CLIENT_USER;                                                                           
typedef	PARAM_CONFIG_VIDEO_ENCODE_PUBLIC	CONFIG_VIDEO_ENCODE_PUBLIC;                                                                           
typedef	PARAM_CONFIG_VIDEO_ENCODE           CONFIG_VIDEO_ENCODE;                                                                           
typedef	PARAM_CONFIG_VIDEO_BASE_PARAM       CONFIG_VIDEO_BASE_PARAM;                                                                           
typedef	PARAM_CONFIG_OSD_LOGO               CONFIG_OSD_LOGO_T;                                                                           
typedef	PARAM_CONFIG_OSD_TIME               CONFIG_OSD_TIME_T;             
typedef	PARAM_CONFIG_AUDIO_ENCODE	        CONFIG_AUDIO;
typedef	PARAM_CONFIG_AUTO_MAINTAIN          CONFIG_AUTO_MAINTAIN;         
typedef	PARAM_CONFIG_RECORD_PUBLIC          CONFIG_RECORD_PUBLIC;        
typedef	PARAM_CONFIG_RECORD_PARAM           CONFIG_RECORD_PARAM;          
typedef	PARAM_CONFIG_NTP                    CONFIG_NTP;                   
typedef	PARAM_CONFIG_EMAIL                  CONFIG_EMAIL; 
typedef PARAM_CONFIG_ALARM_IO	        	CONFIG_ALARM_IO;
typedef PARAM_CONFIG_ALARM_VIDEO_LOSE		CONFIG_ALARM_VIDEO_LOSE;
typedef PARAM_CONFIG_ALARM_VIDEO_SHELTER	CONFIG_ALARM_VIDEO_SHELTER;
typedef PARAM_CONFIG_ALARM_MOVE_DETECT		CONFIG_ALARM_MOVE_DETECT;


typedef struct _ConfigOsd_
{
	CONFIG_OSD_LOGO_T 	osdLogo;
	CONFIG_OSD_TIME_T 	osdTime;
} PACK_ALIGN CONFIG_OSD;

//
// 客户端登录到dvs请求, pc->pu
typedef struct _LoginDvsReq_
{
	char user[NORMAL_USER_NAME_LEN];        // 用户名
	char passwd[NORMAL_PASSWD_LEN];    // 密码
	char reserved[32];    // 保留
} PACK_ALIGN LOGIN_DVS_REQ_T;
// 登录回复, pu->pc
typedef struct _LoginDvsRes_
{
	uint	permission;        // 每1 bit标识一种权限;
	uint	userStreamMark;    // 该用户的流标识,客户端在申请音视频的时候要带上这个mark
	uchar	level;            // 用户的级别:0,NONE; 1,Admin; 2,浏览用户;
	char	reserved[7];    
} LOGIN_DVS_RES_T;

//
// 请求视频流 pc->pu
typedef struct _StreamReq_
{
	uint 	userStreamMark; // 在登录的时候由 pu 返回
	int		channel;
} STREAM_REQ_T;
// 发送视频流分包 pu->pc
typedef struct _StreamPackHead_
{
	ushort 	len;    // 本包长度	
	uchar 	seq;    // 包序号, 在使用tcp 传输的时候这个不再重要了
	uchar 	mark;    // 包标识, 0为一帧只有一个包,1为第一包,2为结束包,3为中间包
} STREAM_PACK_HEAD_T;



//
// 时间相关
typedef struct _DvsTime_
{
	char timeVal[DATETIME_LEN];
} DVS_TIME_T;

//
// 录像搜索条件 pc->pu
typedef struct _DcpSearchRecordCond_
{
	int  channel;        // 通道号
	uint type;            // 录像类型
	char startTime[DATETIME_LEN];     // 开始时间
	char endTime[DATETIME_LEN];     // 结束时间
} DCP_SEARCH_RECORD_COND_T;

//
// 搜索录像的结果 pu->pc
typedef struct _DcpSearchRecordResult_
{
	char	recordName[NORMAL_FILENAME_LEN];    // 录像文件名
	char	startTime[DATETIME_LEN];    // 开始时间(YYYY-MM-DD hh:mm:ss)
	char	endTime[DATETIME_LEN];        // 结束时间(YYYY-MM-DD hh:mm:ss)
	uint	recordLen;                    // 录像长度
	uint	recordType;                    // 录像类型	
} DCP_SEARCH_RECORD_RESULT_T;

// 
// 请求升级 pc->pu 
typedef struct _UpdateReq_ 
{ 
   uint   fileSize;     // 升级文件大小
   char   reserved[64]; 
} DCP_UPDATE_REQ_T;

// 
// 录像下载请求 pc->pu
typedef struct _DcpRecordReq_
{
	char recFileName[NORMAL_FILENAME_LEN]; // 包含绝对路径的录像文件名
} DCP_RECORD_REQ_T;

//
// 日志下载请求
typedef struct _DcpGetLog_
{
    unsigned char logType;      // 需要下载的日志类型
    unsigned char logLevel;  // 需要下载人日志最低级别
    char reserved[6];
} DCP_GET_LOG_T;

// 
// 报警类型
/*
类型子参数根据报警类型决定： 
例如 视频丢失报警  ， 
0表示 通道1 
1表示 通道2 
2表示 通道3 

如果IO报警： 
0：表示第1个IO报警 
1：表示第2个IO报警
*/
typedef struct _DcpAlarmRes_
{
	uint alarmType;        // 报警类型   
	uint alarmTypeSub;   // 类型子参数   下标从0 开始
} DCP_ALARM_RES_T;

//
// 获取存储信息的返回结果，每个存储一个结构体
typedef struct _DcpStorageInfo_
{
    int  type;                 // 存储器类型, 参见 HDD_DEV_TYPE_ST
    uint totalSize;            // 分区总大小 单位MBytes
    uint freeSize;             // 分区剩余空间 单位MBytes
} DCP_STORAGE_INFO_T;

//
//日期时间
typedef struct _DcpDateTime_
{
	char dateTime[20];   //2013-09-23 09:05:29
	char reserved[8];
} DCP_DATE_TIME_T;

// 硬盘状态异常
typedef enum _DcpHddStatErrCode_
{
    DCP_HDD_STAT_ERR_FULL = 0,
    DCP_HDD_STAT_ERR_REMOVED,
    DCP_HDD_STAT_ERR_OTHER
} DCP_HDD_STAT_ERR_CODE_EN;

typedef struct _DcpHddStatErr_
{
	int errCode;        // 0. 硬盘满 1.硬盘拔出 2.其他   
} DCP_HDD_STAT_ERR_T;

#undef  PACK_ALIGN

#endif // __DCPTYPES_H__

