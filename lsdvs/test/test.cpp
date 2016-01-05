#if 1
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "debug.h"
#include "thread.h"
#include "test.h"
#include "testMessage.h"
#include "testStream.h"
#include "testosd.h"
#include "fit.h"
#include "gpio.h"
#include "driver.h"

/* ======== 下面是创建一个线程和销毁一个线程的典型代码,可以拷贝使用 ======== */
static THREAD_MAINTAIN_T g_testTm;
static void *TestThread( void *arg )
{
	static int count = 0;
	SVPrint( "%s start!\r\n", __FUNCTION__ );
//	TestXmlMakeTermRegisterPack();
//	TestXmlMakeTermRegisterParse();
//	TestXmlMakeTermRegisterParseLoadHead();
//	TestProcon();
//	TestPcp();
//	TestRtcGetTime();
//	TestReadMtd1();
    // TestSetOsd();
#if 0
	FitMympiSetResolution();
    sleep( 10 );
	DriverInit();
#endif
	while( g_testTm.runFlag )
    {
    	sleep( 1 );
        //TestProconH264Read();
        //TestSnapSearch();
        //TestSnap();
        //FiRecTestInquireRecord();
        //TestIpcHttpSnap();
    	TestTw2867();
    	count++;
#if 0
    	if( count == 2 )
        {
        	FitMympiSetAvencAccompanyingAudio( 0, 0 );
        	SVPrint( "sven debug---1\r\n" );
        }
    	else if( count == 5 )
        {
        	FitMympiSetAvencAccompanyingAudio( 0, 1 );
            
        	SVPrint( "sven debug---2\r\n" );
        }
    	else if( count == 8 )
        {
        	FitMympiSetAvencAccompanyingAudio( 0, 0 );
            
        	SVPrint( "sven debug---3\r\n" );
        }
        #endif
    }
    
	SVPrint( "%s stop!\r\n", __FUNCTION__ );
	return NULL;
}

void StartTestThread()
{
	int ret;
	g_testTm.runFlag = 1;
	ret = ThreadCreate( &g_testTm.id, TestThread, NULL );
	if( 0!= ret )
    {        
    	g_testTm.runFlag = 0;
    	SVPrint( "error:ThreadCreate:%s\r\n", STRERROR_ERRNO );
    }
}

void StopTestThread()
{
	int ret;
	g_testTm.runFlag = 0;
	ret = ThreadJoin( g_testTm.id, NULL );
	if( 0 != ret )
    {
    	SVPrint( "error:ThreadJoin:%s\r\n", STRERROR_ERRNO );
    }
}
/* ======== 上面是创建一个线程和销毁一个线程的典型代码 ======== */

void TestServiceStart()
{
     StartTestThread();
    // StartTestEpollwThread();
    // StartTestMutexwThread();
}

void TestServiceStop()
{
    // StopTestThread();
    // StopTestEpollwThread();
    // StopTestMutexwThread();
}

#endif //end 1

