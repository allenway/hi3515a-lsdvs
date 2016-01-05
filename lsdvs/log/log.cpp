#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
// #include <malloc.h>
#include <pthread.h>
#include "const.h"
#include "linuxFile.h"
#include "debug.h"
#include "malloc.h"
#include "message.h"
#include "timer.h"
#include "flash.h"
#include "crc.h"
#include "log.h"
#include "timeExchange.h"

//#define USE_LOG_BACKUP

/*******************************************************************************
* fn: 得到当前系统时间,字符串格式，依次存放 年月日时分秒
* in:
* out: stime(保存在字符串中)
* ret:  0:success  other:failed
********************************************************************************/
static int GetCurrentTimeStr(char *stime)
{
    int ret = -1;
    time_t t;
    struct tm *tmp;

    t = Time();
    tmp = localtime(&t);

    if(tmp != NULL)
    {
        if(strftime(stime, TIME_STR_LEN, "%Y%m%d%H%M%S", tmp) > 0)
        {
            ret = 0;
        }
    }
    return ret;
}

class CLog
{
	pthread_mutex_t m_mutex;
	LP_LOG m_pLog;
public:
	CLog();
    ~CLog();
	int Add( unsigned char channel, unsigned char logType, unsigned char logLevel, char *pLogInformation);
	int Get( unsigned char logType, unsigned char logLevel, LP_LOG pLog);
    int SynToFlash();
	LP_LOG getLog(void) {return m_pLog;}
	int Clean(void);
    int Init(void);
    // int FixLog(LP_LOG pLog);
};
CLog::CLog()
{
	pthread_mutex_init(&m_mutex, NULL);
}
CLog::~CLog()
{
	pthread_mutex_lock(&m_mutex);
	if(m_pLog)
    {
    	Free(m_pLog);
    	m_pLog = NULL;
    }
	pthread_mutex_unlock(&m_mutex);
	pthread_mutex_destroy(&m_mutex);
}
int CLog::Init(void)
{
    unsigned long crcTemp;
    int readFlag;
    int ret = -1;
	m_pLog = (LP_LOG)Malloc(sizeof(LOG_T));
	pthread_mutex_lock(&m_mutex);
	if(m_pLog)
    {
    	Memset(m_pLog, 0, sizeof(LOG_T));
    	Strncpy(m_pLog->head.signature, LOG_SIGNATURE, sizeof(m_pLog->head.signature));
    	m_pLog->head.itemSize = sizeof(LOG_ITEM_T);
    	m_pLog->head.totalItemNum = 0;
    	m_pLog->head.writeItemIndex = 0;
    	LP_LOG pLog = (LP_LOG)Malloc(sizeof(LOG_T));
    	if(pLog)
        {
            readFlag = 0;
        	if(ReadLog((char*)pLog, sizeof(LOG_T)) == 0)
            {
            	if(Strncmp(pLog->head.signature, LOG_SIGNATURE, sizeof(pLog->head.signature)) == 0)
                {
                    crcTemp = CRC32((unsigned char*)pLog->itemArray, pLog->head.totalItemNum * pLog->head.itemSize);
                    if(crcTemp == pLog->head.crc)
                    {
                        readFlag = 1;
                    }
                    else
                    {
                        SVPrint("CLog Init ReadLog CRC Fail!\r\n");
                    }
                }
                else
                {
                    SVPrint("CLog Init ReadLog Signature Fail!\r\n");
                }
            }
            else
            {
                SVPrint("CLog Init ReadLog Fail!\r\n");
            }
#ifdef USE_LOG_BACKUP
            if(readFlag == 0)
            {
                if(ReadLogBackup((char *)pLog, sizeof(LOG_T)) == 0)
                {
                    if(Strncmp(pLog->head.signature, LOG_SIGNATURE, sizeof(pLog->head.signature)) == 0)
                    {
                        crcTemp = CRC32((unsigned char*)pLog->itemArray, pLog->head.totalItemNum * pLog->head.itemSize);
                        if(crcTemp == pLog->head.crc)
                        {
                            readFlag = 2;
                        }
                        else
                        {
                            SVPrint("CLog Init ReadLogBackup CRC Fail!\r\n");
                        }
                    }
                    else
                    {
                        SVPrint("CLog Init ReadLogBackup Signature Fail!\r\n");
                    }
                }
                else
                {
                    SVPrint("CLog Init ReadLogBackup Fail!\r\n");
                }
            }
#endif
            if(readFlag > 0)
            {
                ret = 0;
                SVPrint("CLog Init writeItemIndex:%d total:%d logsize:%d\r\n", pLog->head.writeItemIndex, pLog->head.totalItemNum, pLog->head.itemSize);
                if(pLog->head.writeItemIndex >= 0 && pLog->head.writeItemIndex < MAX_LOG_NUM)
                {
                    m_pLog->head.writeItemIndex = pLog->head.writeItemIndex;
                }
                else
                {
                    m_pLog->head.writeItemIndex = 0;
                }
                
                if(pLog->head.totalItemNum > 0 && pLog->head.totalItemNum <+ MAX_LOG_NUM)
                {
                    m_pLog->head.totalItemNum = pLog->head.totalItemNum;
                }
                else
                {
                    m_pLog->head.totalItemNum = MAX_LOG_NUM;
                }


                if(pLog->head.itemSize == sizeof(LOG_ITEM_T))
                {
                    m_pLog->head.crc = pLog->head.crc;
                    Memcpy(m_pLog->itemArray, pLog->itemArray, sizeof(LOG_ITEM_T)*pLog->head.totalItemNum);
                    readFlag = 3;
                }
                else
                {
                    int i;
                    int nLogSize = pLog->head.itemSize;
                    char* pSrc = (char*)pLog->itemArray;

                    if(nLogSize > (int)sizeof(LOG_T) || nLogSize < 1)
                    {
                        nLogSize = sizeof(LOG_T);
                    }
                    for(i = 0; i < pLog->head.totalItemNum; i++)
                    {
                        Memcpy(&m_pLog->itemArray[i], pSrc, nLogSize);
                        pSrc += pLog->head.itemSize;
                    }
                    m_pLog->head.crc = CRC32((unsigned char*)m_pLog->itemArray, m_pLog->head.totalItemNum * m_pLog->head.itemSize);
                }
#ifdef USE_LOG_BACKUP
                // 读出的flash 日志结构与程序日志不一样，需重写,程序升级时改变日志项大小会发生这种情况
                if(readFlag == 3)
                {
                    WriteLog( (char *)m_pLog, sizeof(LOG_T) );
                    WriteLogBackup( (char *)m_pLog, sizeof(LOG_T) );
                }
                // 读出主日志失败，读backup 成功，重写主日志
                else if(readFlag == 2)
                {
                    WriteLogBackup( (char *)m_pLog, sizeof(LOG_T) );
                }
                // 读主日志成功，需读出backup,查看是否有错，有错则重写backup
                else
                {
                    readFlag = 0;
                    if(ReadLogBackup((char *)pLog, sizeof(LOG_T)) == 0)
                    {
                        if(Strncmp(pLog->head.signature, LOG_SIGNATURE, sizeof(pLog->head.signature)) == 0)
                        {
                            crcTemp = CRC32((unsigned char*)pLog->itemArray, pLog->head.totalItemNum * pLog->head.itemSize);
                            if(crcTemp == pLog->head.crc)
                            {
                                readFlag = 2;
                            }
                        }
                    }
                    if(0 == readFlag)
                    {
                        WriteLog( (char *)m_pLog, sizeof(LOG_T) );
                    }
                }
#endif
                CORRECTPRINT("CLog Init OK!\r\n");
            }
        	Free(pLog);
        }
        else
        {
            ERRORPRINT("CLog Init Malloc temp Fail!\r\n");
        }
    }
    else
    {
        ERRORPRINT("CLog Init Malloc Fail!\r\n");
    }
	pthread_mutex_unlock(&m_mutex);
    return ret;
}
int CLog::Add( unsigned char channel, unsigned char logType, unsigned char logLevel, char * pLogInformation)
{
    int writepos;
	int ret = -1;

	pthread_mutex_lock(&m_mutex);
	if(m_pLog)
    {
        writepos = m_pLog->head.writeItemIndex;
        // if(m_pLog->head.totalItemNum) Memmove(&m_pLog->itemArray[1], &m_pLog->itemArray[0], sizeof(LOG_ITEM_T)*m_pLog->head.totalItemNum);

    	Memset(&m_pLog->itemArray[ writepos ], 0, sizeof(LOG_ITEM_T));
        GetCurrentTimeStr( m_pLog->itemArray[writepos].time);
    	m_pLog->itemArray[writepos].channel[0] = (channel <= REAL_CHANNEL_NUM) ? (channel + 0x30) : 'F';
    	m_pLog->itemArray[writepos].channel[1] = ' ';
    	m_pLog->itemArray[writepos].type[0] = ' ';
    	m_pLog->itemArray[writepos].type[1] = logType;
    	m_pLog->itemArray[writepos].level[0] = ' ';
    	m_pLog->itemArray[writepos].level[1] = logLevel;

    	if(pLogInformation)
        {
            Strncpy(m_pLog->itemArray[writepos].information, pLogInformation, LOG_INFORMATION_LEN);
        }

        m_pLog->head.writeItemIndex++;
    	if(m_pLog->head.writeItemIndex >= MAX_LOG_NUM)
        {
            m_pLog->head.writeItemIndex = 0;
        }
        if(m_pLog->head.totalItemNum < MAX_LOG_NUM)
        {
            m_pLog->head.totalItemNum++;
        }
        //ret = WriteLog((char*)m_pLog, sizeof(LOG_T));
    	ret = 0;
    }
	pthread_mutex_unlock(&m_mutex);
	return ret;
}
int CLog::Get(unsigned char logType, unsigned char logLevel, LP_LOG pLog)
{
	int i,n;
	if(pLog == NULL || m_pLog == NULL)
    {
        return 0;
    }
	pthread_mutex_lock(&m_mutex);
	Memcpy(&pLog->head, &m_pLog->head, sizeof(LOG_HEAD_T));
	pLog->head.totalItemNum = 0;
	pLog->head.writeItemIndex = 0;
    // for(i = 0; i < m_pLog->head.totalItemNum; i++) {

    n = m_pLog->head.totalItemNum;
    for(i = m_pLog->head.writeItemIndex-1; i >= 0 && n > 0; i--, n--)
    {
        if((logType == LOG_TYPE_ALL || m_pLog->itemArray[i].type[1] == logType) &&
           (LOG_LEVEL_ALL >= logLevel || m_pLog->itemArray[i].level[1] == logLevel) )
        {
            Memcpy(&pLog->itemArray[pLog->head.totalItemNum++], &m_pLog->itemArray[i], sizeof(LOG_ITEM_T));
        }
    }
    for(i = m_pLog->head.totalItemNum-1; i >= m_pLog->head.writeItemIndex && n >= 0; i--, n--)
    {
        if((logType == LOG_TYPE_ALL || m_pLog->itemArray[i].type[1] == logType) &&
           (LOG_LEVEL_ALL >= logLevel || m_pLog->itemArray[i].level[1] == logLevel) )
        {
        	Memcpy(&pLog->itemArray[pLog->head.totalItemNum++], &m_pLog->itemArray[i], sizeof(LOG_ITEM_T));
        }
    }
	pthread_mutex_unlock(&m_mutex);
	return pLog->head.totalItemNum;
}

//ql add
int CLog::Clean(void)
{
	pthread_mutex_lock(&m_mutex);
	if(m_pLog != NULL)
    {
        //bzero(pLog,sizeof(LOG_T));
        //strcpy(m_pLog->head.signature, LOG_SIGNATURE);
        // ret = WriteLogMsg((char *)m_pLog,logLen);
        // ret = SynToFlash();
        Free(m_pLog);
    }
	pthread_mutex_unlock(&m_mutex);
	return 0;
}
/*
*fn: 把日志写入Flash
return: 0, 写Flash 成功; -1, 写日志备份出错; -2, 写主日志出错; -3, 写主日志和备份日志都出错. 
*/
int CLog::SynToFlash(void)
{
    int ret = -1;
    int logLen;
    pthread_mutex_lock(&m_mutex);
    if(m_pLog)
    {
        m_pLog->head.crc = CRC32((unsigned char*)m_pLog->itemArray, m_pLog->head.totalItemNum * m_pLog->head.itemSize);
        logLen = sizeof(LOG_HEAD_T) + m_pLog->head.totalItemNum*m_pLog->head.itemSize;
        // ret = WriteLogMsg((char *)m_pLog,logLen);
        ret = WriteLog((char *)m_pLog,logLen);
        if(ret == -1)
        {
            ret = -2;
        }
#ifdef USE_LOG_BACKUP
        ret += WriteLogBackup((char *)m_pLog,logLen);
#endif
    }
    pthread_mutex_unlock(&m_mutex);
    return ret;

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CLog s_log;

int LogInit(void)
{
    int ret = -1;
    ret = s_log.Init();
    if(ret == 0)
    {
        // LogPrint(LOG_TYPE_ALL, LOG_LEVEL_DEBUG);
        // LogAdd(0xff, LOG_TYPE_SYSTEM, LOG_LEVEL_INFO, "bhdvs start, LogInit ok.");
        CORRECTPRINT("LogInit OK!!!\r\n");
    }
    return ret;
}
/********************************************************************************
* 添加log, 不会马上写FLASH, 发出需写LOG FLASH 的消息, 由定时器接到消息同一写FLASH.
*********************************************************************************/
int LogAdd( unsigned char channel, unsigned char logType, unsigned char logLevel, char *pLogFmt, ... )
{
	int ret;
    char buf[LOG_INFORMATION_LEN];
	va_list ap;

    Memset(buf, 0, sizeof(buf));
    if(pLogFmt)
    {
        va_start( ap, pLogFmt );
        vsnprintf( buf, sizeof(buf) - 3, pLogFmt, ap );
        va_end( ap );
    }
    Strcat(buf, "\r\n");
	ret = s_log.Add( channel, logType,  logLevel, buf );
	if(ret == 0)
    {
        MessageSend(MSG_ID_TIMER_WRITE_SYS_LOG);
    }
	return ret;
}

/*******************************************************************************
* 添加log, 马上写入FLASH.
********************************************************************************/
int LogAddAndWriteFlash( unsigned char channel, unsigned char logType, unsigned char logLevel, char *pLogFmt, ... )
{
	int ret;
    char buf[LOG_INFORMATION_LEN];
	va_list ap;

    Memset(buf, 0, sizeof(buf));
    if(pLogFmt)
    {
        va_start( ap, pLogFmt );
        vsnprintf( buf, sizeof(buf) - 3, pLogFmt, ap );
        va_end( ap );
    }
    Strcat(buf, "\r\n");
	ret = s_log.Add( channel, logType,  logLevel, buf );
	if(ret == 0)
    {
        s_log.SynToFlash();
    }
	return ret;
}

/******************************************************************************
* 获取指定类型和级别的日志, 存放到传入的pLog, pLog 必须已经分配好.
******************************************************************************/
int LogGet( unsigned char logType, unsigned char logLevel, LP_LOG pLog)
{
	return s_log.Get( logType,  logLevel, pLog );
}

/********************************************************************************
* 将指定类型和级别的日志打印到终端,用于调试.
********************************************************************************/
int LogPrint( unsigned char logType, unsigned char logLevel)
{
    int i;
    int logNum;
    LP_LOG pLog;
    pLog = (LP_LOG)Malloc(sizeof(LOG_T));
	if(pLog == NULL)
    {
        return -1;
    }
    logNum = s_log.Get( logType, logLevel, pLog);

    if(logNum > 0)
    {
        SVPrint("printLog : (number = %d)\r\n", logNum);
        for(i = 0; i < logNum; i++)
        {
            SVPrint("%s", (char *)&pLog->itemArray[i]);
        }
    }
    Free(pLog);
    return 0;
}

/***************************************************************************
* 释放日志缓冲区, 不会刷新和清空flash.
*****************************************************************************/
int LogClean(void)
{
    return s_log.Clean();
}
#if 0
int LogSizeOfLog(LP_LOG pLog)
{
    return sizeof(LOG_T);
}
#endif
/***************************************************************************
* fn: 告诉定时器,要保存日志到flash
****************************************************************************/
int SaveLogToFlash()
{
	return MessageSend( MSG_ID_TIMER_WRITE_SYS_LOG );
}

/****************************************************************************
* fn: 马上把配置写进flash
*****************************************************************************/

static void *WriteLogTimer( void *args )
{
	if( MessageFind( MSG_ID_TIMER_WRITE_SYS_LOG ) )
    {
    	while( MessageRecv( MSG_ID_TIMER_WRITE_SYS_LOG ) >= 0 )    { ; }
        s_log.SynToFlash();
    }
	return NULL;
}

/*****************************************************************************
* 写flash 定时器
*******************************************************************************/
void SysLogAddTimer()
{
	unsigned int writeSysLogTimerInterval	= 3;
	AddRTimer( WriteLogTimer, NULL, writeSysLogTimerInterval );
}

