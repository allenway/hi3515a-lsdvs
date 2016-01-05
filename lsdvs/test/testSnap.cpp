#if 0
#include <stdio.h>
#include <unistd.h>
#include "const.h"
#include "debug.h"
#include "snapMpi.h"
#include "snapSearch.h"
#include "timeExchange.h"

static void TestSaveJpg( char *fileName, char *pData0, int len0 )
{
	FILE *ffd = NULL;
    
	if( NULL == ffd )
    {
    	ffd = fopen( fileName, "w+b" );
    	if(ffd == NULL)
        {
        	SVPrint("fopen failed:%s!\r\n",STRERROR_ERRNO);
        	return;
        }
    }                            

	if( len0 > 0 )
    {
    	if( fwrite( pData0, len0, 1, ffd) != 1 )
        {
        	perror( "fi_record write1" );
        }
    }

	fclose( ffd );
}

#define TEST_JPG_LEN (1024*1024)
void TestSnap()
{
	int ret, i;
	uint len;
	static char buf[TEST_JPG_LEN];
	char filename[128];
	static int count;

	for( i = 0; i < REAL_CHANNEL_NUM; ++i )
    {
    	if( i != 2 ) continue;
    	sprintf( filename, "./h3515snap_ch%02d_no%d.jpg", i, count++ );
    	ret = SnapMpiGetJpg( i, buf, &len );
    	if( 0 == ret )
        {
        	TestSaveJpg( filename, buf, len );
        }
    }
}

void TestSnapSearch()
{
	SNAP_INQUIRE_LIST *snap_inquire_ret=NULL;    
	int	debug_count = 0, i;

	char startTimeHuman[] = "2012-11-28 00:00:00";    
	char stopTimeHuman[]  = "2012-11-28 23:59:59";
	int YY, MM, DD, hh, mm, ss;
	int startTime, stopTime;

	sscanf( startTimeHuman, "%04d-%02d-%02d %02d:%02d:%02d",
                            &YY, &MM, &DD, &hh, &mm, &ss );
	startTime = FiTimeHumanToUtc( YY, MM, DD, hh, mm, ss );    
	sscanf( stopTimeHuman, "%04d-%02d-%02d %02d:%02d:%02d",
                                &YY, &MM, &DD, &hh, &mm, &ss );
	stopTime = FiTimeHumanToUtc( YY, MM, DD, hh, mm, ss );    

	for( i = 0; i < REAL_CHANNEL_NUM; ++i )
    {
    	snap_inquire_ret = FiSnapInquireSnapFile( i, SNAP_TYPE_ALL, startTime,stopTime );    
    	if(NULL == snap_inquire_ret)
        {
        	FiPrint2("inquire no snap file:NULL == snap_inquire_ret!\r\n");
        	return;
        }
    	if(NULL == snap_inquire_ret->head)
        {
        	FiSnapFreeInquireSnap(snap_inquire_ret);
        	FiPrint2("inquire no snap file:NULL == snap_inquire_ret!\r\n");
        	return;
        }
        
    	snap_inquire_ret->cur = snap_inquire_ret->head;
    	while(1)
        {
        	if(NULL == snap_inquire_ret->cur)
            	break;

        	SVPrint("inqiure snap file(%d):%s,size=%d\r\n",
                ++debug_count,snap_inquire_ret->cur->snapName,snap_inquire_ret->cur->snapLen);
        	snap_inquire_ret->cur = snap_inquire_ret->cur->next;
        }
    	FiSnapFreeInquireSnap(snap_inquire_ret);
    }
}

#endif

