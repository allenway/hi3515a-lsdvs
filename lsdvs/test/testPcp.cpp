#if 1
#include <unistd.h>
#include <signal.h>
#include <time.h>
/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : ipc 抓拍生产者消费者
*******************************************************************************
*/
#include "debug.h"
#include "pcp.h"
#include "malloc.h"
#include "thread.h"
#include "pcp.h"

static CPcp g_testPcp;

static void TestPcpInit( int chNum, int dataNode, uint maxSize )
{
	g_testPcp.Init( chNum, dataNode, maxSize );
}

static int TestPcpOpen( int channel, int flag )
{
	return g_testPcp.Open( channel, flag );
}

static void TestPcpClose( uint fd )
{
	g_testPcp.Close( fd );
}

static int TestPcpWrite( uint fd, DATA_PIECE_T proDataInfo )
{
	int ret;    
    
	ret = g_testPcp.Write( fd, proDataInfo );    

	return ret;
}

static PCP_NODE_T *TestPcpRead( uint fd )
{
	return g_testPcp.Read( fd );
}

static void TestPcpFree( PCP_NODE_T *pcpNode )
{
	g_testPcp.Tfree( pcpNode );
}

#define TEST_STREAM_SEND_BUF_SIZE_1 (1 * 1024)
#define TEST_STREAM_SEND_BUF_SIZE_2 (4 * 1024)
#define TEST_NUM 10

static void *TestPcpWriteThread( void *args )
{
	uint fd;
	int ret, i;
	static uint totalWriteSize = 0;
	int count = 0;
	static char sendBuf1[TEST_STREAM_SEND_BUF_SIZE_1];
	static char sendBuf2[TEST_STREAM_SEND_BUF_SIZE_2];
	DATA_PIECE_T proDataInfo;
	proDataInfo.count = 2;
	proDataInfo.buf[0] = sendBuf1;
	proDataInfo.len[0] = TEST_STREAM_SEND_BUF_SIZE_1;
	proDataInfo.buf[1] = sendBuf2;
	proDataInfo.len[1] = TEST_STREAM_SEND_BUF_SIZE_2;

	fd = TestPcpOpen( 2, OPEN_WRONLY );
	if( fd > 0 )
    {
    	while( 1 )
        {
        	ret = TestPcpWrite( fd, proDataInfo );    
        	if( 0 == ret )
            {
            	for( i = 0; i < proDataInfo.count; ++i )
                {
                	totalWriteSize += proDataInfo.len[i];
                }
            }
        	FiPrint2( "%s:totalWriteSize(%u)!\r\n", __FUNCTION__, totalWriteSize );
        	usleep( 40000 );
        	if( ++count == 10 )
            {            
                //break;
            }
        }
    }
    
	FiPrint2( "quit %s!\r\n", __FUNCTION__ );

	return NULL;
}

static void *TestPcpRead1( void* args )
{
	uint fd;
	PCP_NODE_T *pcpNode;
	static uint totalReadSize = 0;

	fd = TestPcpOpen( 2, OPEN_RDONLY );
	if( fd > 0 )
    {
    	while( 1 )
        {
        	pcpNode = TestPcpRead( fd );
        	if( NULL != pcpNode )
            {    
            	totalReadSize += pcpNode->pcpHead.len;                    
                
            	FiPrint2( "%s:totalReadSize(%u)!\r\n", __FUNCTION__, totalReadSize );
            	TestPcpFree( pcpNode );
            }
        	else
            {
            	usleep( 40000 );
            }
        }

    	TestPcpClose( fd );
    }
    
	return NULL;
}

static void *TestPcpRead2( void* args )
{
	uint fd;
	PCP_NODE_T *pcpNode;
	static uint totalReadSize = 0;

	fd = TestPcpOpen( 8, OPEN_RDONLY );
	if( fd > 0 )
    {
    	while( 1 )
        {
        	pcpNode = TestPcpRead( fd );
        	if( NULL != pcpNode )
            {    
            	totalReadSize += pcpNode->pcpHead.len;                 
                
            	FiPrint2( "%s:totalReadSize(%u)!\r\n", __FUNCTION__, totalReadSize );
            	TestPcpFree( pcpNode );
            }
        	else
            {
            	usleep( 40000 );
            }
        }

    	TestPcpClose( fd );
    }
    
	return NULL;
}

void TestPcp()
{
	pthread_t writeId;
	pthread_t readId1;
	pthread_t readId2;

	TestPcpInit( 16, 5, 704 * 576 * 3 / 2 );

	ThreadCreate( &readId1, TestPcpRead1, NULL );
	ThreadCreate( &readId2, TestPcpRead2, NULL );    

	sleep( 2 );
	ThreadCreate( &writeId, TestPcpWriteThread, NULL );
}

#endif


