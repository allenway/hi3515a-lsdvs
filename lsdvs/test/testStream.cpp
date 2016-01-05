#if 0

#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "debug.h"
#include "netStm.h"
#include "malloc.h"

void TestStreamGet()
{
	FRAME_T* pFrame;
	static READ_POS_T readPos = { 0 };
	static int bareDataTotal = 0;

	FiPrint2( "testStreamGet\n" );
	pFrame = LqjStreamGetFrameFromNetPool( 0, &readPos );
	if( NULL != pFrame )
    {        
    	bareDataTotal += pFrame->streamHead.frameHead.frameLen;
    	FiPrint2( "frameLen(%u), get bareDataTotal(%d)!\r\n", 
                        	pFrame->streamHead.frameHead.frameLen, bareDataTotal );
    }
}

#define TEST_STREAM_SEND_BUF_SIZE_1 (1 * 1024)
#define TEST_STREAM_SEND_BUF_SIZE_2 (4 * 1024)
#define TEST_NUM 10

void TestStreamSend()
{
	int ret;
	static char sendBuf1[TEST_STREAM_SEND_BUF_SIZE_1];
	static char sendBuf2[TEST_STREAM_SEND_BUF_SIZE_2];
	STREAM_HEAD_T streamHead;
	FRAME_DATA_INFO_T frameDataInfo;
	frameDataInfo.count = 2;
	frameDataInfo.buf[0] = sendBuf1;
	frameDataInfo.len[0] = TEST_STREAM_SEND_BUF_SIZE_1;
	frameDataInfo.buf[1] = sendBuf2;
	frameDataInfo.len[1] = TEST_STREAM_SEND_BUF_SIZE_2;
	ret = LqjStreamSendFrameToNetPool( 1, &streamHead, frameDataInfo );    
}

void* StreamWrite( void* args )
{
	int ret;
	static char sendBuf1[TEST_STREAM_SEND_BUF_SIZE_1];
	static char sendBuf2[TEST_STREAM_SEND_BUF_SIZE_2];
	STREAM_HEAD_T streamHead;
	FRAME_DATA_INFO_T frameDataInfo;
	frameDataInfo.count = 2;
	frameDataInfo.buf[0] = sendBuf1;
	frameDataInfo.len[0] = TEST_STREAM_SEND_BUF_SIZE_1;
	frameDataInfo.buf[1] = sendBuf2;
	frameDataInfo.len[1] = TEST_STREAM_SEND_BUF_SIZE_2;

	SVPrint( "StreamWrite\n" );
	for ( int i=0; i<TEST_NUM; i++ )
    {
    	ret = LqjStreamSendFrameToNetPool( 1, &streamHead, frameDataInfo );
    	if (0 != ret)
        {
        	FiPrint2( "LqjStreamSendFrameToNetPool failed!\n" );
        }
    	SVPrint( "StreamWrite i = %d\n",i );
        //usleep( 10000 );
    }

	return NULL;
}

void* StreamRead1( void* args )
{
	FRAME_T* pFrame = NULL;
	static READ_POS_T readPos = { 0 };
	static int bareDataTotal = 0;

	for ( int i=0; i<TEST_NUM; i++ )
    {
    	SVPrint( "StreamRead1 i = %d\n",i );
    	pFrame = LqjStreamGetFrameFromNetPool( 1, &readPos );
    	if( NULL != pFrame )
        {        
        	bareDataTotal += pFrame->streamHead.frameHead.frameLen;

        	ShareFree( pFrame );
        }
    	else
        {
        	SVPrint( "StreamRead1 lost i = %d\n",i );
        }
        //usleep( 10000 );
    }
	FiPrint2( "frameLen1(), get bareDataTotal(%d)!\r\n", 
            /*pFrame->streamHead.frameHead.frameLen,*/ bareDataTotal );

	return NULL;
}

void* StreamRead2( void* args )
{
	FRAME_T* pFrame;
	static READ_POS_T readPos = { 0 };
	static int bareDataTotal = 0;

	for ( int i=0; i<TEST_NUM; i++ )
    {
    	SVPrint( "StreamRead2 i = %d\n",i );
    	pFrame = LqjStreamGetFrameFromNetPool( 1, &readPos );
    	if( NULL != pFrame )
        {        
        	bareDataTotal += pFrame->streamHead.frameHead.frameLen;

        	ShareFree( pFrame );
        }
    	else
        {
        	SVPrint( "StreamRead2 lost i = %d\n",i );
        }
        //usleep( 10000 );
    }
	FiPrint2( "frameLen2(), get bareDataTotal(%d)!\r\n", 
            /*pFrame->streamHead.frameHead.frameLen,*/ bareDataTotal );
}

void testStream()
{
	pthread_t writeHandle;
	pthread_t readHandle1;
	pthread_t readHandle2;

	clock_t start = 0, finish = 0;
	double totalTime = 0;
	start = clock();

	pthread_create( &writeHandle, NULL, StreamWrite, NULL );
	pthread_create( &readHandle1, NULL, StreamRead1, NULL );
	pthread_create( &readHandle2, NULL, StreamRead2, NULL );
    
	pthread_join( writeHandle, NULL );
	pthread_join( readHandle1, NULL );
	pthread_join( readHandle2, NULL );

	finish = clock();
	totalTime = (double)(finish - start);
	SVPrint( "totalTime = %f\n",totalTime );

}

#endif

