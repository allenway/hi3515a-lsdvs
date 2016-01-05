/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.11.06
**  description  : rtc时钟接口
*******************************************************************************
*/
#include <sys/ioctl.h>
#include "const.h"
#include "debug.h"
#include "mutex.h"
#include "timeExchange.h"
#include "linuxFile.h"

#if defined MCU_HI3515
#if 1 // 如果这里打开,那么rtc 时间设置是走标准流程的;否则,走非标准流程
#define RTC_DEVICE_PATH	    "/dev/rtc"

typedef struct linux_rtc_time 
{
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
} LINUX_RTC_TIME_T;

#define RTC_SET_TIME       	_IOW('p', 0x0a, struct linux_rtc_time) /* Set RTC time    */
#define RTC_RD_TIME        	_IOR('p', 0x09, struct linux_rtc_time)  /* Read RTC time   */
static CMutexLock g_rtcLock;

/*
* fn: 获取rtc 时间
*/
int RtcGetTime( int *pyear, int *pmonth, int *pday,
        	int *phour, int *pminute, int *psecond, int *pweekday )
{
	int rtcRet = FI_FAILED;
	int rtcHandle = -1;
	struct tm tm;

	memset( &tm, 0x00, sizeof(tm) );
	g_rtcLock.Lock();
	rtcHandle = Open( RTC_DEVICE_PATH, O_RDONLY );
	if( -1 == rtcHandle )
    {
    	SVPrint( "failed:can't Open rtc dev(%s)!\r\n", RTC_DEVICE_PATH );    
    	rtcRet = FI_FAILED;
    }
	else
    {        
    	if( 0 == ioctl( rtcHandle, RTC_RD_TIME, &tm) )
        {
        	tm.tm_year += 1900;
        	tm.tm_mon  += 1;
        	if( 1 == TimeIsValidDatetime(tm.tm_year, tm.tm_mon, tm.tm_mday, 
                	tm.tm_hour, tm.tm_min, tm.tm_sec) )
            {
            	if(NULL != pyear)        *pyear     = tm.tm_year;
            	if(NULL != pmonth)        *pmonth = tm.tm_mon;
            	if(NULL != pday)         *pday     = tm.tm_mday;
            	if(NULL != phour)         *phour     = tm.tm_hour;
            	if(NULL != pminute)     *pminute     = tm.tm_min;
            	if(NULL != psecond)     *psecond     = tm.tm_sec;    
            	if(NULL != pweekday)    *pweekday     = tm.tm_wday;
                
            	rtcRet = FI_SUCCESSFUL;
            }
        	else
            {
            	SVPrint( "TimeIsValidDatetime failed!\r\n" );
            }
        }    
    	else
        {
        	SVPrint( "ioctl error:%s!\r\n", STRERROR_ERRNO );
        }
    	Close( rtcHandle );
    }
	g_rtcLock.Unlock();

    
	SVPrint( "%s, rtcRet(%d): %04d-%02d-%02d %02d:%02d:%02d\r\n",
            	__FUNCTION__, rtcRet,
            	tm.tm_year, tm.tm_mon, tm.tm_mday, 
            	tm.tm_hour, tm.tm_min, tm.tm_sec );
    
	return rtcRet;
}

/*
* fn: 设置rtc 时间
*/
int RtcSetTime( int year, int month, int day,
            	int hour, int minute, int second )
{
	int rtcRet = FI_FAILED;
	time_t utcTime;    
	struct tm tm, *ptm;
	int rtcHandle;
    
	g_rtcLock.Lock();
	rtcHandle = Open( RTC_DEVICE_PATH, O_WRONLY );
	if(-1 == rtcHandle)
    {
    	SVPrint( "failed:can't Open rtc dev(%s)!\r\n", RTC_DEVICE_PATH );    
    }
	else
    {        
    	utcTime = FiTimeHumanToUtc( year, month, day, hour, minute, second );
    	ptm = gmtime_r( &utcTime, &tm );
    	if( NULL != ptm )
        {
        	tm.tm_isdst = 0;    
        	rtcRet = ioctl( rtcHandle, RTC_SET_TIME, &tm ); 
        	if( 0 != rtcRet )
            {
            	SVPrint( "%s, rtcRet(%d), ioctl error:%s\r\n", 
                    	__FUNCTION__, rtcRet, STRERROR_ERRNO );
            }
        }
                
    	Close ( rtcHandle );
    }
	g_rtcLock.Unlock();

	SVPrint( "%s, rtcRet(%d): %04d-%02d-%02d %02d:%02d:%02d!\r\n",
        	__FUNCTION__, rtcRet, 
        	year, month, day, 
        	hour, minute, second, rtcRet );
    
	return rtcRet;
}

#else

#define RTC_DEVICE_PATH	    "/dev/hxrtc"

typedef struct 
{
    unsigned int  second;
    unsigned int  minute;
    unsigned int  hour;
    unsigned int  date;
    unsigned int  weekday;
    unsigned int  month;
    unsigned int  year;
} rtc_time_t_rt1307;

#define RTC_RD_TIME   _IOW( 'h', 4, rtc_time_t_rt1307 ) 
#define RTC_SET_TIME  _IOWR( 'h', 5, rtc_time_t_rt1307 ) 

static CMutexLock g_rtcLock;

/*
* fn: 获取rtc 时间
*/
int RtcGetTime( int *pyear, int *pmonth, int *pday,
        	int *phour, int *pminute, int *psecond, int *pweekday )
{
	int rtcRet = FI_FAILED;
	int rtcHandle = -1;
	rtc_time_t_rt1307 rtcTime;

	g_rtcLock.Lock();
	rtcHandle = Open( RTC_DEVICE_PATH, O_RDONLY );
	if( -1 == rtcHandle )
    {
    	SVPrint( "failed:can't Open rtc dev(%s)!\r\n", RTC_DEVICE_PATH );    
    	rtcRet = FI_FAILED;
    }
	else
    {        
    	if( 0 == ioctl( rtcHandle, RTC_RD_TIME, &rtcTime) )
        {            
        	if( 1 == TimeIsValidDatetime(rtcTime.year, rtcTime.month, rtcTime.date, 
                	rtcTime.hour, rtcTime.minute, rtcTime.second) )
            {
            	if(NULL != pyear)        *pyear     = rtcTime.year;
            	if(NULL != pmonth)        *pmonth = rtcTime.month;
            	if(NULL != pday)         *pday     = rtcTime.date;
            	if(NULL != phour)         *phour     = rtcTime.hour;
            	if(NULL != pminute)     *pminute     = rtcTime.minute;
            	if(NULL != psecond)     *psecond     = rtcTime.second;    
            	if(NULL != pweekday)    *pweekday     = rtcTime.weekday;
                
            	rtcRet = FI_SUCCESSFUL;
            }
        	else
            {
            	SVPrint( "TimeIsValidDatetime failed!\r\n" );
            }
        }    
    	else
        {
        	SVPrint( "ioctl error:%s!\r\n", STRERROR_ERRNO );
        }
    	Close( rtcHandle );
    }
	g_rtcLock.Unlock();
    
	SVPrint( "%s, rtcRet(%d): %04d-%02d-%02d %02d:%02d:%02d\r\n",
            	__FUNCTION__, rtcRet,
            	rtcTime.year, rtcTime.month, rtcTime.date, 
            	rtcTime.hour, rtcTime.minute, rtcTime.second );
    
	return rtcRet;
}

/*
* fn: 设置rtc 时间
*/
int RtcSetTime( int year, int month, int day,
            	int hour, int minute, int second )
{
	int rtcRet = FI_FAILED;
	time_t utcTime;    
	struct tm tm, *ptm;
	int rtcHandle;    
	rtc_time_t_rt1307 rtcTime;
    
	g_rtcLock.Lock();
	rtcHandle = Open( RTC_DEVICE_PATH, O_WRONLY );
	if(-1 == rtcHandle)
    {
    	SVPrint( "failed:can't Open rtc dev(%s)!\r\n", RTC_DEVICE_PATH );    
    }
	else
    {        
    	utcTime = FiTimeHumanToUtc( year, month, day, hour, minute, second );
    	ptm = gmtime_r( &utcTime, &tm );
    	if( NULL != ptm )
        {
        	rtcTime.year     = year;
        	rtcTime.month     = month;
        	rtcTime.date	= day;
        	rtcTime.hour	= hour;
        	rtcTime.minute	= minute;
        	rtcTime.second	= second;
        	rtcTime.weekday = tm.tm_wday;    
        	rtcRet = ioctl( rtcHandle, RTC_SET_TIME, &rtcTime ); 
        	if( 0 != rtcRet )
            {
            	SVPrint( "%s, rtcRet(%d), ioctl error:%s\r\n", 
                    	__FUNCTION__, rtcRet, STRERROR_ERRNO );
            }
        }
                
    	Close ( rtcHandle );
    }
	g_rtcLock.Unlock();

	SVPrint( "%s, rtcRet(%d): %04d-%02d-%02d %02d:%02d:%02d!\r\n",
        	__FUNCTION__, rtcRet, 
        	year, month, day, 
        	hour, minute, second, rtcRet );
    
	return rtcRet;
}

#endif//IF 1
#endif // if defiened MCU_HI3515


#if defined MCU_HI3515A
#define RTC_DEVICE_PATH	    "/dev/hi_rtc"

typedef struct 
{
    unsigned int  year;
    unsigned int  month;
    unsigned int  date;
    unsigned int  hour;
    unsigned int  minute;
    unsigned int  second;
    unsigned int  weekday;
} rtc_time_t;



#define RTC_RD_TIME		_IOR('p', 0x09,  rtc_time_t)
#define RTC_SET_TIME		_IOW('p', 0x0a,  rtc_time_t)

static CMutexLock g_rtcLock;

/*
* fn: 获取rtc 时间
*/
int RtcGetTime( int *pyear, int *pmonth, int *pday,
        	int *phour, int *pminute, int *psecond, int *pweekday )
{
	int rtcRet = FI_FAILED;
	int rtcHandle = -1;
	rtc_time_t tm;

	memset( &tm, 0x00, sizeof(tm) );
	g_rtcLock.Lock();
	rtcHandle = Open( RTC_DEVICE_PATH, O_RDONLY );
	if( -1 == rtcHandle )
    {
    	ERRORPRINT( "failed:can't Open rtc dev(%s)!\r\n", RTC_DEVICE_PATH );    
    	rtcRet = FI_FAILED;
    }
	else
    {        
    	if( 0 == ioctl( rtcHandle, RTC_RD_TIME, &tm) )
        {
        	if( 1 == TimeIsValidDatetime(tm.year, tm.month, tm.date, 
                	tm.hour, tm.minute, tm.second) )
            {
            	if(NULL != pyear)        *pyear     = tm.year;
            	if(NULL != pmonth)        *pmonth = tm.month;
            	if(NULL != pday)         *pday     = tm.date;
            	if(NULL != phour)         *phour     = tm.hour;
            	if(NULL != pminute)     *pminute     = tm.minute;
            	if(NULL != psecond)     *psecond     = tm.second;    
            	if(NULL != pweekday)    *pweekday     = tm.weekday;
                
            	rtcRet = FI_SUCCESSFUL;
            }
        	else
            {
            	ERRORPRINT( "TimeIsValidDatetime failed!\r\n" );
            }
        }    
    	else
        {
        	ERRORPRINT( "ioctl error:%s!\r\n", STRERROR_ERRNO );
        }
    	Close( rtcHandle );
    }
	g_rtcLock.Unlock();

    
	SVPrint( "%s, rtcRet(%d): %04d-%02d-%02d %02d:%02d:%02d\r\n",
            	__FUNCTION__, rtcRet,
            	tm.year, tm.month, tm.date, 
            	tm.hour, tm.minute, tm.second );
    
	return rtcRet;
}

/*
* fn: 设置rtc 时间
*/
int RtcSetTime( int year, int month, int day,
            	int hour, int minute, int second )
{
	int rtcRet = FI_FAILED;
	rtc_time_t tm;
	int rtcHandle;
    
	g_rtcLock.Lock();
	rtcHandle = Open( RTC_DEVICE_PATH, O_WRONLY );
	if(-1 == rtcHandle)
    {
    	ERRORPRINT( "failed:can't Open rtc dev(%s)!\r\n", RTC_DEVICE_PATH );    
    }
	else
    {    
        tm.year   = year;
        tm.month  = month;
        tm.date   = day;
        tm.hour   = hour;
        tm.minute = minute;
        tm.second = second;
        
        rtcRet = ioctl( rtcHandle, RTC_SET_TIME, &tm ); 
        if(rtcRet < 0)
        {
            ERRORPRINT("RTC ioctl SET time failed.return:%d\n!!!,rtcRet");
        }
    	Close ( rtcHandle );
    }
	g_rtcLock.Unlock();

	SVPrint( "%s, rtcRet(%d): %04d-%02d-%02d %02d:%02d:%02d!\r\n",
        	__FUNCTION__, rtcRet, 
        	year, month, day, 
        	hour, minute, second, rtcRet );
    
	return rtcRet;
}

#endif
