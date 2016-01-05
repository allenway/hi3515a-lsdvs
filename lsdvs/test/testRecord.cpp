#if 1
#include <sys/time.h>
#include "debug.h"
#include "record.h"
#include "timeExchange.h"
#include "public.h"

static void PrintResultInquireResult( RECORD_INQUIRE *pResult )
{
	int YY, MM, DD, hh, mm, ss;
	Print( " ===== name(%s) =====\r\n", pResult->recFilename );    
	Print( "size=%d, type=%d!\r\n", pResult->recLen, pResult->recType );
	FiTimeUtcToHuman( pResult->startTime, &YY, &MM, &DD, &hh, &mm, &ss );
	Print( "start time(%04d-%02d-%02d %02d:%02d:%02d)!\r\n", YY, MM, DD, hh, mm, ss );        
	FiTimeUtcToHuman( pResult->stopTime, &YY, &MM, &DD, &hh, &mm, &ss );
	Print( "start time(%04d-%02d-%02d %02d:%02d:%02d)!\r\n", YY, MM, DD, hh, mm, ss );    
}

void FiRecTestInquireRecord()
{
	RECORD_INQUIRE_LIST *recInquireRret=NULL;
    
	char startTimeHuman[] = "2013-05-22 00:00:00";    
	char stopTimeHuman[]  = "2013-05-22 23:59:59";
	int YY, MM, DD, hh, mm, ss;
	int startTime, stopTime;

	sscanf( startTimeHuman, "%04d-%02d-%02d %02d:%02d:%02d",
                            &YY, &MM, &DD, &hh, &mm, &ss );
	startTime = FiTimeHumanToUtc( YY, MM, DD, hh, mm, ss );    
	sscanf( stopTimeHuman, "%04d-%02d-%02d %02d:%02d:%02d",
                                &YY, &MM, &DD, &hh, &mm, &ss );
	stopTime = FiTimeHumanToUtc( YY, MM, DD, hh, mm, ss );
    
	recInquireRret = FiRecInquireRecordFile( 0, RECORD_TYPE_TIMER, startTime, stopTime );    
	if( NULL == recInquireRret )
    {
    	FiPrint2("inquire no record file:NULL == recInquireRret!\r\n");
    }
	else if( NULL == recInquireRret->head )
    {
    	FiPrint2("inquire no record file:NULL == recInquireRret!\r\n");
    }
	else
    {
    	recInquireRret->cur = recInquireRret->head;
    	while(1)
        {
        	if( NULL == recInquireRret->cur )
            {
            	break;
            }
        	PrintResultInquireResult( recInquireRret->cur );
        	recInquireRret->cur = recInquireRret->cur->next;
        }
    }
	FiRecFreeInquireRecord( recInquireRret );    
	return;
}

#if 0  // ÔÝÊ±¼Ä´æ
static struct timeval g_system_runtime;
static void RefrechSysRunTime(void)
{
	static struct timeval base;
	struct timeval now;
	struct timeval sub;

	gettimeofday(&now,NULL);
	if(timercmp(&now,&base,<))
    {
    	base = now;
    	return;
    }
	timersub(&now,&base,&sub);
	if(sub.tv_sec > 10)
    {
    	base = now;
    	return;
    }
	base = now;
	timeradd(&g_system_runtime,&sub,&g_system_runtime);
}

time_t FiAppGetSysRunTime(void)
{
	struct timeval tv,tv1;
	while(1)
    {
        /*simple mutex!*/
    	tv=g_system_runtime;
    	tv1=g_system_runtime;
    	if(tv.tv_sec == tv1.tv_sec && tv.tv_usec == tv1.tv_usec) break;
    	Usleep(100);
    }
	return tv.tv_sec;
}
#endif


#endif 

