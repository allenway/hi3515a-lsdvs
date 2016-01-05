/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 从生产者消费者队列中获取h264, 并写入录像队列
*******************************************************************************
*/
#include <stdio.h>
#include <unistd.h>
#include "debug.h"
#include "const.h"
#include "thread.h"
#include "ttypes.h"
#include "proconH264.h"
#include "recordPool.h"

static THREAD_MAINTAIN_T g_recordGetH264Tm;
static void *RecordGetH264Thread( void *arg )
{
	int i, ret;
	static uint nullCount;
	uint proconH264Fd[REAL_CHANNEL_NUM];
	PROCON_NODE_T *pProconH264;

	for( i = 0; i < REAL_CHANNEL_NUM; ++i )
    {
    	proconH264Fd[i] = ProconH264Open( i, OPEN_RDONLY );
    	if( 0 == proconH264Fd[i] )
        {
        	SVPrint( "failed:ProconH264Open( %d, OPEN_RDONLY )!\r\n", i );
        }
    }
	SVPrint( "%s start!\r\n", __FUNCTION__ );
	while( g_recordGetH264Tm.runFlag )
    {        
    	for( i = 0; i < REAL_CHANNEL_NUM; ++i )
        {
        	pProconH264 = ProconH264Read( proconH264Fd[i] );
        	if( NULL == pProconH264 )
            {
            	nullCount++;
            }
        	else
            {
            	nullCount = 0;
            	ret = RecordPoolWrite( i, pProconH264 );
                
            	ProconH264Free( pProconH264 );
            }
        }
    	if( nullCount >= REAL_CHANNEL_NUM )
        {
        	usleep( 20000 );
        }
    }
 
	for( i = 0; i < REAL_CHANNEL_NUM; ++i )
    {
    	if( 0 != proconH264Fd[i] )
        {
        	ProconH264Close( proconH264Fd[i] );
        }
    }
	SVPrint( "%s stop!\r\n", __FUNCTION__ );
	return NULL;
}

void StartRecordGetH264Thread()
{
	int ret;
	g_recordGetH264Tm.runFlag = 1;
	ret = ThreadCreate( &g_recordGetH264Tm.id, RecordGetH264Thread, NULL );
	if( 0!= ret )
    {        
    	g_recordGetH264Tm.runFlag = 0;
    	SVPrint( "error:ThreadCreate:%s\r\n", STRERROR_ERRNO );
    }
}

void StopRecordGetH264Thread()
{
	int ret;
	g_recordGetH264Tm.runFlag = 0;
	ret = ThreadJoin( g_recordGetH264Tm.id, NULL );
	if( 0 != ret )
    {
    	SVPrint( "error:ThreadJoin:%s\r\n", STRERROR_ERRNO );
    }
}

