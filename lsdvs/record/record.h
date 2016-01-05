/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**	文件名: record.h
**  description  : for record.cpp
**  date           :  2013.10.18
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/

#ifndef __RECORD_H__
#define __RECORD_H__

#include <pthread.h>
#include <sys/time.h>
#include "ttypes.h"
#include "paramManage.h"
#include "recordSearch.h"
#include "recordFit.h"

#define RECORD_BARE_STREAM	//使用裸流录像

#define RECORD_FILENAME_PREFIX	"brec_"

#define MAX_WEEK_DAY		7
#define MAX_TIME_SEG_NUM 	4

#define RECORD_TIME_ERROR_RANGE	    	30	//second
#define RECORD_TIMER_SWITCH_TIME		20	//minute
#define RECORD_TIMER_SWITCH_TIME_ALARM	3	//minute

#define RECORD_INDEX_VERSION	0x1FDF0301	/*用于日后检索时候的扩展,高2字节固定,低2字节可变*/

typedef struct _RecordThread
{
	int channel;
	pthread_t recordThreadId;    
	pthread_mutex_t recordLock;
	int	recordThreadRunFlag;    
	int recordCtlFlag;
}RECORD_THREAD;

typedef struct _RecordManage
{
	pthread_t recordManageId;
	int recordManageRunFlag;
}RECORD_MANAGE;

enum _RecordCtl
{
	REC_START = 0,    //开始录像
	REC_STOP,        //停止录像
	REC_SWITCH,        //切换录像
	REC_WRITE,        //写录像文件
	REC_SLEEP	    //录像模块休眠
};

#ifndef PACK_ALIGN
#define PACK_ALIGN __attribute__((packed))
#endif

/*录像共用结构体*/
typedef struct _RecordPublic
{    
	uint delSpace;            //硬盘录像空间报警值  >= 100M  <= 10000M
	uint delSize;            //reserve,删除录像的大小(固定为按天删除)
	uchar loopRecord;        //循环录像,0:不支持,1:支持.
	uchar preRecord;        //预录,0:停止,1:开启. 
	uchar switchFileTime;    //reserve,录像文件切换时间 10~30分钟(固定为20分钟)        
	char reserve[13];                
} PACK_ALIGN RECORD_PUBLIC;

/*手动录像*/
typedef struct _RecordHand
{
	uchar recHand;    //0:停止手动录像,1:启动手动录像
	char reserve[3];
} PACK_ALIGN RECORD_HAND;

/*定时录像结构体*/
typedef struct _RecTimeSeg
{
	uchar	startHour;
	uchar	startMinute;
	uchar	startSecond;
	uchar	stopHour;
	uchar	stopMinute;
	uchar	stopSecond;
	uchar 	reserve[2];
} PACK_ALIGN REC_TIME_SEG;

typedef struct _TimeWeek
{
	REC_TIME_SEG	timeSeg[MAX_TIME_SEG_NUM];
	char	    	enableFlag;    //1: enable, 0: disable
	char	    	reserve[3];
} PACK_ALIGN TIME_WEEK;

typedef struct _RecordTimer
{
	TIME_WEEK timerWeek[MAX_WEEK_DAY];
} PACK_ALIGN RECORD_TIMER;

#undef PACK_ALIGN



typedef struct _RecordPublicConf
{
	char loopRecordFlag;
	char reserve[15];
}RECORD_PUBLIC_CONF;



typedef struct _RecordLedStatus
{
	int openFlag;
	int recStatus[REAL_CHANNEL_NUM];
}RECORD_LED_STATUS;

int FiRecSetRecFileSwitchTime(int switchTime);
int FiRecGetRecFileSwitchTime();
int FiRecSetRecFileSwitchTimeAlarm(int switchTime);
int FiRecGetRecFileSwitchTimeAlarm();

void FiRecStartRecordService(void);
void FiRecStopRecordService(void);
void FiRecRecordInit(void);
void FiRecInitPublic(void);
void FiRecDelRecordInit(void);
int FiRecStartRecord(int channel,uint recType);
int FiRecStopRecord(int channel,uint recType);
int FiRecSetRecordingFilename(int channel,char *rec_name,char *index_name);
int FiRecGetRecordingFilename(int channel,char *rec_name,char *index_name);

int FiRecSetHandRecord( int channel, PARAM_CONFIG_RECORD_PARAM *pRecordParam );

void FiRecGetTimerRecordPolicy(int channel,int weekDay,TIME_WEEK *recTimerPolicy);
int FiRecSetTimerRecordPolicy( int channel, int weekDay, PARAM_CONFIG_RECORD_PARAM *pRecordParam );

time_t FiAppGetSysRunTime(void);
int FiRecIsRecording();
int FiRecIsHandLeRecording(int channel);

// 为删除录像而停止录像服务
void RecordStopServiceForDel();
// 为删除录像而重启录像服务
void RecordRestartServiceForDel();


#endif //__RECORD_H__

