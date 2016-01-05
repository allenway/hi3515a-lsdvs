#if 0
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "debug.h"
#include "procon.h"
#include "malloc.h"
#include "thread.h"

static CProcon g_testProcon( BLOCK_NO, DATA_TYPE_NOMAL );

static void TestProconInit( int chNum, int dataNode )
{
	g_testProcon.Init( chNum, dataNode );
}

static int TestProconOpen( int channel, int flag )
{
	return g_testProcon.Open( channel, flag );
}

static void TestProconClose( uint fd )
{
	g_testProcon.Close( fd );
}

static int TestProconWrite( uint fd, DATA_PIECE_T proDataInfo )
{
	int ret;    
    
	ret = g_testProcon.Write( fd, proDataInfo );    

	return ret;
}

static PROCON_NODE_T *TestProconRead( uint fd )
{
	return g_testProcon.Read( fd );
}

static void TestProconFree( PROCON_NODE_T *pcpNode )
{
	g_testProcon.Tfree( pcpNode );
}


#define TEST_STREAM_SEND_BUF_SIZE_1 (1 * 1024)
#define TEST_STREAM_SEND_BUF_SIZE_2 (4 * 1024)
#define TEST_NUM 10

static void *TestProconWriteThread( void *args )
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
	fd = TestProconOpen( 0, OPEN_WRONLY );
	if( fd > 0 )
    {
    	while( 1 )
        {
        	ret = TestProconWrite( fd, proDataInfo );    
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

static void *TestProconRead1( void* args )
{
	uint fd;
	PROCON_NODE_T *proconNode;
	static uint totalReadSize = 0;

	fd = TestProconOpen( 0, OPEN_RDONLY );
	if( fd > 0 )
    {
    	while( 1 )
        {
        	proconNode = TestProconRead( fd );
        	if( NULL != proconNode )
            {    
            	totalReadSize += proconNode->proconHead.len;                    
                
            	FiPrint2( "%s:totalReadSize(%u)!\r\n", __FUNCTION__, totalReadSize );
            	TestProconFree( proconNode );
            }
        	else
            {
            	usleep( 40000 );
            }
        }

    	TestProconClose( fd );
    }
    
	return NULL;
}

static void *TestProconRead2( void* args )
{
	uint fd;
	PROCON_NODE_T *proconNode;
	static uint totalReadSize = 0;

	fd = TestProconOpen( 0, OPEN_RDONLY );
	if( fd > 0 )
    {
    	while( 1 )
        {
        	proconNode = TestProconRead( fd );
        	if( NULL != proconNode )
            {    
            	totalReadSize += proconNode->proconHead.len;                 
                
            	FiPrint2( "%s:totalReadSize(%u)!\r\n", __FUNCTION__, totalReadSize );
            	TestProconFree( proconNode );
            }
        	else
            {
            	usleep( 40000 );
            }
        }

    	TestProconClose( fd );
    }
    
	return NULL;
}

void TestProcon()
{
	pthread_t writeId;
	pthread_t readId1;
	pthread_t readId2;
    
	TestProconInit( 16, 64 );

	ThreadCreate( &readId1, TestProconRead1, NULL );
	ThreadCreate( &readId2, TestProconRead2, NULL );    

	sleep( 2 );
	ThreadCreate( &writeId, TestProconWriteThread, NULL );
}

#endif

