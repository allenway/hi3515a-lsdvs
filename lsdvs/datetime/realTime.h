#ifndef __REALTIME_H__
#define __REALTIME_H__

int RealTimeSetDatetime( int year, int month, int day,
                   	int hour, int minute, int second );

void RealTimeSyncRtcWhenStartSys();
void RealTimeAddTimer();


#ifdef USE_SVEN_ST_ //±ãÓÚ´úÂëÔÄ¶Á
struct tm 
{
	int tm_sec;         /* seconds */
	int tm_min;         /* minutes */
	int tm_hour;        /* hours */
	int tm_mday;        /* day of the month */
	int tm_mon;         /* month */
	int tm_year;        /* year */
	int tm_wday;        /* day of the week */
	int tm_yday;        /* day in the year */
	int tm_isdst;       /* daylight saving time */
};
#endif 

#endif  

