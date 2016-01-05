/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.10.10
**  description  : 调用海思接口, 实现音视频同步取流
*******************************************************************************
*/
#if defined MCU_HI3515//do  not use these func


#include <unistd.h>
#if defined MCU_HI3515
#include "mpi_avenc.h"
#endif
#include "hi_common.h"

#include "mympiAvenc.h"
#include "const.h"
#include "debug.h"
#include "malloc.h"
#include "thread.h"
#include "mympiAvenc.h"
#include "mpiApp.h"
#include "paramManage.h"
#include "mpiViFrame.h"

static const int g_avChNum = REAL_CHANNEL_NUM;
static AVENC_ST g_avencSt[g_avChNum];

static void InitAvencSt()
{
	int i, ret;
	PARAM_CONFIG_VIDEO_ENCODE param;

	for( i = 0; i < g_avChNum; ++i )
    {        
    	ret = ParamGetVideoEncode( i, &param );
    	if( 0 == ret )
        {
        	if( 0 != param.encodeType ) // 视频流
            {
            	g_avencSt[i].enObject = AVENC_OPERATION_VIDEO;
            }
        	else // 音视频流
            {
            	g_avencSt[i].enObject = AVENC_OPERATION_BOTH;
            }
        }
    }
}

static void ChangeAv( AVENC_ST *pAvencSt )
{
	int ret;
	if( 0 == pAvencSt->avChange.val )
    {
    	ret = HI_MPI_AVENC_StopCH( pAvencSt->AVChnId, AVENC_OPERATION_AUDIO );
    	if( 0 != ret )
        {
        	SVPrint( "error:HI_MPI_AVENC_StopCH():0x%X!\r\n", ret );
        	return;
        }
    }
	else
    {
    	ret = HI_MPI_AVENC_StartCH( pAvencSt->AVChnId, AVENC_OPERATION_AUDIO, 0 );
    	if( 0 != ret )
        {
        	SVPrint( "error:HI_MPI_AVENC_StartCH():0x%X!\r\n", ret );
        	return;
        }
    }
}

static void *AvencThread( void *args )
{
	HI_S32 ret;
	int channel;
	AVENC_ST *pAvencSt;
	AVENC_STREAM_S avStream;
	AVENC_LIST_S *pVideo, *pAudio;
	void *pVideoData, *pAudioData;

	channel = *(int *)args;
	Free( args );
    
	pAvencSt = &g_avencSt[channel];

	ret = HI_MPI_AVENC_StartCH( pAvencSt->AVChnId, pAvencSt->enObject, 0 );
	if( 0 != ret )
    {
    	SVPrint( "error:ch(%d) HI_MPI_AVENC_StartCH():0x%X!\r\n", channel, ret );
                
    	return NULL;
    }
	SVPrint( "channel(%d) %s start!\r\n", channel, __FUNCTION__ );
	while( pAvencSt->runFlag )
    {
    	if( 1 == pAvencSt->avChange.operateFlag )
        {
        	ChangeAv( pAvencSt );
        	pAvencSt->avChange.operateFlag = 0;
        }
        
    	ret = HI_MPI_AVENC_GetStream( pAvencSt->AVChnId, &avStream, HI_IO_BLOCK );        
    	if( 0 == ret )
        {    
        	pVideo = avStream.pVideoListHead;
        	pAudio = avStream.pAudioListHead;
        	while( NULL != pVideo || NULL != pAudio )
            {                
            	pVideoData = NULL;
            	if( NULL != pVideo )
                {
                	pVideoData = pVideo->pData;
                	pVideo = pVideo->pNext;
                }
            	pAudioData = NULL;
            	if( NULL != pAudio )
                {
                	pAudioData = pAudio->pData;
                	pAudio = pAudio->pNext;
                }

            	H264FromHisiAddrToMyAddrAv( channel, pVideoData, pAudioData );
            }
        	HI_MPI_AVENC_ReleaseStream( pAvencSt->AVChnId, &avStream );            
        }
    	else
        {
        	SVPrint( "error:ch(%d) HI_MPI_AVENC_GetStream():0x%X!\r\n", channel, ret );
        }
    }    
    
	HI_MPI_AVENC_StopCH( pAvencSt->AVChnId, AVENC_OPERATION_BOTH );
    
	SVPrint( "channel(%d) %s stop!\r\n", channel, __FUNCTION__ );
	return NULL;
}

static void StartAvencThread()
{
	int ret, i;
	int *pCh;
    
	for( i = 0; i < g_avChNum; ++i )
    {
    	g_avencSt[i].runFlag = 1;
    	pCh = (int *)Malloc( sizeof(int) );
    	if( NULL != pCh )
        {
            *pCh = i;
        	ret = ThreadCreate( &g_avencSt[i].threadId, AvencThread, (void *)pCh );
        	if( 0!= ret )
            {        
            	g_avencSt[i].runFlag = 0;
            	SVPrint( "error:ThreadCreate:%s!\r\n", STRERROR_ERRNO );
            }
        }
    }
}

static void StopAvencThread()
{
	int ret, i;
    
	for( i = 0; i < g_avChNum; ++i )
    {
    	g_avencSt[i].runFlag = 0;
    	ret = ThreadJoin( g_avencSt[i].threadId, NULL );
    	if( 0 != ret )
        {
        	SVPrint( "error:ThreadJoin:%s\r\n", STRERROR_ERRNO );
        }
    }
}

static int CreateAvencChannel()
{
	int ret = -1, i;
	VENC_CHN VChnId;
	AENC_CHN AChnId;
    
	for( i = 0; i < g_avChNum; ++i )
    {
    	VChnId = i << 1;
    	AChnId = i;
    	ret = HI_MPI_AVENC_CreatCH( VChnId, AChnId, &g_avencSt[i].AVChnId );
    	if( HI_SUCCESS != ret )
        {
        	SVPrint( "error: VChnId(%d), AChnId(%d) HI_MPI_AVENC_CreatCH(): 0x%X\n", VChnId, AChnId, ret );
        	break;
        }
    }

	return ret;
}

static int DestoryAvencChannel()
{
	int ret = -1, i;    
    
	for( i = 0; i < g_avChNum; ++i )
    {
    	ret = HI_MPI_AVENC_DestroyCH(g_avencSt[i].AVChnId, HI_FALSE);
    	if( HI_ERR_AVENC_BUFFER_NOTFREE == ret )
        {
            SVPrint( "Some AVENC buffer may be hold by user! Be sure to release all"\
               "buffer! Otherwise some exception may arose!\n" );
            /* 如果确信不会再操作复合编码通道，可以强制销毁编码通道. */
            ret = HI_MPI_AVENC_DestroyCH( g_avencSt[i].AVChnId, HI_TRUE );
        	if( HI_SUCCESS != ret )
            {
            	SVPrint( "error: ch(%d) HI_MPI_AVENC_DestroyCH(): 0x%X\n", i, ret );
            	break;
            }
        }    
    }

	return ret;
}


int MympiAvencStart()
{
	InitAvencSt();
	CreateAvencChannel();
	StartAvencThread();
	StartViFrameThread();
	return 0;
}

int MympiAvencStop()
{    
	StopViFrameThread();
	StopAvencThread();
	DestoryAvencChannel();
	return 0;
}

/*
* fn: 设置伴音开关
* openFlag: 0,关闭伴音音频; 1,打开伴音音频
*/
int MympiSetAvencAccompanyingAudio( int channel, int openFlag )
{
	int ret = -1;
	if( 0 == openFlag || 1 == openFlag )
    {
    	g_avencSt[channel].avChange.val             = openFlag;
    	g_avencSt[channel].avChange.operateFlag     = 1;

    	ret = 0;
    }

	return ret;
}


#endif
