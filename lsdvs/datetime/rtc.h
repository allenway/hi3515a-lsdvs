#ifndef __RTC_H__
#define __RTC_H__

int RtcGetTime( int *pyear, int *pmonth, int *pday,
        	int *phour, int *pminute, int *psecond, int *pweekday );
int RtcSetTime( int year, int month, int day,
            	int hour, int minute, int second );

#endif // __RTC_H__

