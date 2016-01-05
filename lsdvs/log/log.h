#ifndef __LOG_H__
#define __LOG_H__

#include <time.h>

#define LOG_SIGNATURE	        "FirsLogEvt"
#define MAX_LOG_NUM	        	1000
// #define LOG_DESCRIPT_LEN		15
#define LOG_INFORMATION_LEN		108
#define LOG_TYPE_ALL	        '0'
#define LOG_TYPE_SYSTEM	        '1'      /* 系统事件*/
#define LOG_TYPE_DATABASE	    '2'      /* 记录事件*/
#define LOG_TYPE_STORAGE	    '3'   /* FLASH 或SD卡等存贮介质事件 */
#define LOG_TYPE_MODULEx	    '4'   /* 功能模块扩展*/

#define LOG_LEVEL_ALL	        '0'
#define LOG_LEVEL_DEBUG         '1'
#define LOG_LEVEL_INFO	        '2'
#define LOG_LEVEL_WARN	        '3'
#define LOG_LEVEL_ERROR	        '4'
#define LOG_LEVEL_CRITICAL	    '5'
#define TIME_STR_LEN            14

#ifdef WIN32
#pragma pack(1)
#define PACK_ALIGN
#else
#define PACK_ALIGN __attribute__((packed))
#endif

// 日志头
typedef struct _LOG_HEAD_
{
	char signature[8];  // 日志签名
	int totalItemNum;   // 总日志条数
    int writeItemIndex; // 当前写日志偏移
    int itemSize;       // 日志项大小
    unsigned long crc;
} PACK_ALIGN LOG_HEAD_T, *LP_LOG_HEAD;

// 日志内容, 例: "1 20131212121212 1 1视频丢失“
typedef struct _LOG_ITEM_
{
	unsigned char channel[2];                    /* 事件通道 1 */
 	char time[14];                                /* 事件时间 20131212121212 */
	unsigned char type[2];                        /* 事件类型  1*/
 	unsigned char level[2];                        /* 事件级别  1*/
	char information[LOG_INFORMATION_LEN];        /* 事件内容 */
} PACK_ALIGN LOG_ITEM_T, *LP_LOG_ITEM;

// 日志
typedef struct _LOG_
{
	LOG_HEAD_T head;
	LOG_ITEM_T itemArray[MAX_LOG_NUM];
} LOG_T, *LP_LOG;

#ifdef WIN32
#pragma pack()
#endif

#undef PACK_ALIGN
#ifdef __cplusplus
extern "C" {
#endif

void SysLogAddTimer();
int LogInit(void);
int LogAdd( unsigned char channel, unsigned char logType, unsigned char logLevel, char *pLogFmt, ...);
int LogAddAndWriteFlash( unsigned char channel, unsigned char logType, unsigned char logLevel, char *pLogFmt, ... );
int LogGet( unsigned char logType, unsigned char logLevel, LP_LOG pLog);
int LogPrint( unsigned char logType, unsigned char logLevel);
int LogClean(void);

#ifdef __cplusplus
}
#endif

#endif 

