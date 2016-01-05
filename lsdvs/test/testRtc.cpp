#if 0
#include <unistd.h>
#include "rtc.h"
#include "debug.h"

void TestRtcGetTime()
{
//	int ret;
//	int year, month, day, hour, minute, second, weekday;
//	ret = RtcGetTime( &year, &month, &day,
//            &hour, &minute, &second, &weekday );

//	SVPrint( "ret(%d), %04d-%02d-%02d %02d:%02d:%02d %d!\r\n",
//        	ret, year, month, day, hour, minute, second, weekday);
//	sleep( 1 );
	RtcSetTime( 2012, 8, 18, 11, 18, 18);

//	ret = RtcGetTime( &year, &month, &day,
//            &hour, &minute, &second, &weekday );
            
//	sleep( 1 );

//	SVPrint( "ret(%d), %04d-%02d-%02d %02d:%02d:%02d %d!\r\n",
//        	ret, year, month, day, hour, minute, second, weekday);
}
#endif

