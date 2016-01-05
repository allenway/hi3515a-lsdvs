#if 0
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include "debug.h"
#include "thread.h"
#include "message.h"
#include "testMessage.h"
#include "Rand.h"
#include "LinuxFile.h"
#include "test.h"

#define MSG_NUM 3000

typedef struct{
	int nCreate;    // 产生消息个数
	int nConsume;   // 消耗消息个数
	int nRemainder; // 剩余消息个数
} MY_TEST;

static MY_TEST s_test[5] = {0};
static THREAD_MAINTAIN_T s_handle;

// 产生消息
static void *TestThreadCreateMsg( void *arg )
{
	int i = 0;
	int msgId = 0;
	int ret = 0;
	char msgBuf[] = "good";
	int strlen = Strlen( msgBuf ) + 1;
    
	for (i=0; i<MSG_NUM; i++)
    {
    	msgId = RandAb( MSG_ID_0, MSG_ID_4 );
    	ret = MessageSend( msgId, msgBuf, strlen );
    	if (0 <= ret)
        {
        	s_test[msgId].nCreate++;
        }
    	usleep( 1 );
    }
	MessageSend( MSG_ID_MAX, NULL, 0 );
}

static void printfTest( int index )
{
	SVPrint( "\n%d, %d, %d, %d\n", index, s_test[index].nCreate, s_test[index].nConsume, s_test[index].nRemainder );
}

// 消耗消息0的线程
static void *TestThreadConMsg0( void *arg )
{
	int ret = 0;
	while ( s_handle.runFlag )
    {
    	ret = MessageRecv( MSG_ID_MAX, NULL, 0 );
    	if ( 0 <= ret )
        {
        	s_handle.runFlag = 0;
        }

    	if (0 == s_handle.runFlag)
        	break;
        
    	ret = MessageRecv( MSG_ID_0, NULL, 0 );
    	if ( 0 <= ret )
        {
        	s_test[MSG_ID_0].nConsume++;
        }
    	usleep( 4000 );
    }
	s_test[MSG_ID_0].nRemainder = MessageGetNum( MSG_ID_0 );
	printfTest( MSG_ID_0 );
}

// 消耗消息1的线程
static void *TestThreadConMsg1( void *arg )
{
	int ret = 0;
	while ( s_handle.runFlag )
    {
    	ret = MessageRecv( MSG_ID_MAX, NULL, 0 );
    	if ( 0 <= ret )
        {
        	s_handle.runFlag = 0;
        }

    	if (0 == s_handle.runFlag)
        	break;
        
    	ret = MessageRecv( MSG_ID_1, NULL, 0 );
    	if ( 0 <= ret )
        {
        	s_test[MSG_ID_1].nConsume++;
        }
    	usleep( 5000 );
    }
	s_test[MSG_ID_1].nRemainder = MessageGetNum( MSG_ID_1 );
	printfTest( MSG_ID_1 );
}

// 消耗消息2的线程
static void *TestThreadConMsg2( void *arg )
{
	int ret = 0;
	while ( s_handle.runFlag )
    {
    	ret = MessageRecv( MSG_ID_MAX, NULL, 0 );
    	if ( 0 <= ret )
        {
        	s_handle.runFlag = 0;
        }
        
    	if (0 == s_handle.runFlag)
        	break;

    	ret = MessageRecv( MSG_ID_2, NULL, 0 );
    	if ( 0 <= ret )
        {
        	s_test[MSG_ID_2].nConsume++;
        }
    	usleep( 3500 );
    }
	s_test[MSG_ID_2].nRemainder = MessageGetNum( MSG_ID_2 );
	printfTest( MSG_ID_2 );
}

// 消耗消息3的线程
static void *TestThreadConMsg3( void *arg )
{
	int ret = 0;
	while ( s_handle.runFlag )
    {
    	ret = MessageRecv( MSG_ID_MAX, NULL, 0 );
    	if ( 0 <= ret )
        {
        	s_handle.runFlag = 0;
        }
        
    	if (0 == s_handle.runFlag)
        	break;

    	ret = MessageRecv( MSG_ID_3, NULL, 0 );
    	if ( 0 <= ret )
        {
        	s_test[MSG_ID_3].nConsume++;
        }
    	usleep( 2800 );
    }
	s_test[MSG_ID_3].nRemainder = MessageGetNum( MSG_ID_3 );
	printfTest( MSG_ID_3 );
}

// 消耗消息4的线程
static void *TestThreadConMsg4( void *arg )
{
	int ret = 0;
	while ( s_handle.runFlag )
    {
    	ret = MessageRecv( MSG_ID_MAX, NULL, 0 );
    	if ( 0 <= ret )
        {
        	s_handle.runFlag = 0;
        }
        
    	if (0 == s_handle.runFlag)
        	break;

    	ret = MessageRecv( MSG_ID_4, NULL, 0 );
    	if ( 0 <= ret )
        {
        	s_test[MSG_ID_4].nConsume++;
        }
    	usleep( 5600 );
    }
	s_test[MSG_ID_4].nRemainder = MessageGetNum( MSG_ID_4 );
	printfTest( MSG_ID_4 );
}

// 随机消耗消息
static void *TestThreadRandConMsg( void *arg )
{
	int msgId = 0;
	int ret = 0;

	while ( s_handle.runFlag )
    {
    	msgId = RandAb( MSG_ID_0, MSG_ID_4 );
    	ret = MessageRecv( MSG_ID_MAX, NULL, 0 );
    	if ( 0 <= ret )
        {
        	s_handle.runFlag = 0;
        }
        
    	if (0 == s_handle.runFlag)
        	break;

    	ret = MessageRecv( msgId, NULL, 0 );
    	if ( 0 <= ret )
        {
        	s_test[msgId].nConsume++;
        }
    	usleep( 10000 );
    }
}

// message的测试函数
void testMessage()
{    
	pthread_t handle1;
	pthread_t handle2;
	pthread_t handle3;
	pthread_t handle4;
	pthread_t handle5;
	pthread_t handle6;
	pthread_t handle7;
	clock_t start = 0, finish = 0;
	double totalTime = 0;
	start = clock();
	s_handle.runFlag = 1;
    
	ThreadCreate( &handle1, TestThreadCreateMsg, NULL );
	ThreadCreate( &handle2, TestThreadConMsg0, NULL );
	ThreadCreate( &handle3, TestThreadConMsg1, NULL );
	ThreadCreate( &handle4, TestThreadConMsg2, NULL );
	ThreadCreate( &handle5, TestThreadConMsg3, NULL );
	ThreadCreate( &handle6, TestThreadConMsg4, NULL );
	ThreadCreate( &handle7, TestThreadRandConMsg, NULL );

	pthread_join( handle1, NULL );
	pthread_join( handle2, NULL );
	pthread_join( handle3, NULL );
	pthread_join( handle4, NULL );
	pthread_join( handle5, NULL );
	pthread_join( handle6, NULL );
	pthread_join( handle7, NULL );
	finish = clock();
	totalTime = (double)(finish - start);
    
	SVPrint("time: total = %f, create = %f, consume = %f\n", totalTime, MessageGetCreate(), MessageGetConsume());
}
#endif 

