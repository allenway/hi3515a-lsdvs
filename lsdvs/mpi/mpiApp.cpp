/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.10.10
**  description  : 调用mpp 的接口, 封装, 给外界提供接口
********************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include "const.h"
#include "debug.h"
#include "ttypes.h"
#include "mpiApp.h"
#include "message.h"
#include "timer.h"
#include "hton.h"
#include "showFps.h"
#include "sysRunTime.h"
#include "proconH264.h"
#include "mpiVenc.h"
#include "osdApply.h"
#include "fitMpi.h"
#include "osdViVo.h"
#include "vencParamEasy.h"
#include "hi_comm_venc.h"
#include "snapMpi.h"
#include "md.h"
#include "timeExchange.h"
#include "mympi.h"

#include "tw2866Init.h"
#include "cx26828init.h"
#include "hi_comm_aio.h"
#include "mpiViFrame.h"
/*************************************************************************
* fn: 发送消息告诉定时器,执行重启视频编码线程的动作
*************************************************************************/
void FiEncSendRestartMessage()
{
	MessageSend( MSG_ID_RESTART_ENC_MODEL );
}

static void *MoniterEncRestart( void *arg )
{
	if( MessageFind(MSG_ID_RESTART_ENC_MODEL) > 0 )
    {
    	while( MessageRecv(MSG_ID_RESTART_ENC_MODEL) >= 0 );
    	MpiServiceStop();
    	usleep( 20 );
    	MpiServiceStart();
    	SVPrint( "Note: MPI Service Restart!!!\r\n" );
    }

	return NULL;
}

static uint GetFrameSeq( int channel )
{
	static uint seq[MAX_CHANNEL_NUM];
	return seq[channel]++;
}

void EncAddTimer()
{
	AddRTimer( MoniterEncRestart, NULL, 1 );
}


static uint g_proconH264Fd[REAL_CHANNEL_NUM] = {0};
static int GetChnProconH264Fd( int channel )
{
	return g_proconH264Fd[channel];
}

//获取各个通道的H264的生产者消费者对象的句柄
static void InitProconH264Fd()
{
	int i;
	for( i = 0; i < REAL_CHANNEL_NUM; ++i )
    {
    	g_proconH264Fd[i] = ProconH264Open( i, OPEN_WRONLY );
    	if( 0 == g_proconH264Fd[i] )
        {
        	ERRORPRINT( "ch(%d) ProconH264Open failed!\r\n", i );
        }
    }
}

static void DeinitProconH264Fd()
{
	int i;
	for( i = 0; i < REAL_CHANNEL_NUM; ++i )
    {
    	ProconH264Close( g_proconH264Fd[i] );
    	g_proconH264Fd[i] = 0;
    }
}

/*******************************************************************************
* fn: 把H264 从海思底层的地址拷贝到应用层地址,并组成一帧
* channel: 通道
* chType: 0, 主码流; 1, 从码流
* hisiH264: 从海思底层中获取到的h264 指针, 类型为 VENC_STREAM_S
* hisiAudio: 从海思底层中获取到的音频 指针, 类型为 AUDIO_STREAM_S
*******************************************************************************/
#if 0

int H264FromHisiAddrToMyAddrAv( int channel, void *hisiH264, void *hisiAudio )
{
	uint             	i;
	int	            	len, index = 0;
	unsigned long     	frameLen = 0;
	uint             	proconH264Fd;
	STREAM_HEAD_T     	vFrameHead;     // 视频帧头
	STREAM_HEAD_T     	aFrameHead;     // 音频帧头
	DATA_PIECE_T             	proDataInfo;
	VENC_STREAM_S     *vEncStream = (VENC_STREAM_S *)hisiH264;
	AUDIO_STREAM_S     *aEncStream = (AUDIO_STREAM_S *)hisiAudio; 
	uchar frameType;
	struct timeval tv;
	char c;
    //ShowFps( "FiStreamSendFrameToStreamBuf", channel );

	proconH264Fd = GetChnProconH264Fd( channel );
	if( 0 == proconH264Fd )
    {
    	return -1;
    }

	if( NULL != vEncStream )
    {
    	proDataInfo.count         = index + 1;
    	proDataInfo.buf[index]     = (char *)&vFrameHead;
    	proDataInfo.len[index]    = sizeof(vFrameHead);    
    	index++;
    	proDataInfo.nalInfo.nalNum         = vEncStream->u32PackCount;
    	proDataInfo.nalInfo.nalStartOff = sizeof(vFrameHead);
    	for( i = 0; i < vEncStream->u32PackCount; ++i )
        {
        	proDataInfo.nalInfo.nalSize[i] = vEncStream->pstPack[i].u32Len[0] 
                                            + vEncStream->pstPack[i].u32Len[1];

        	c = vEncStream->pstPack[i].pu8Addr[0][4];    
        	if( (0x1F & c) == 0x07 )
            {
            	set_h264SeqParam(channel, 0, (char *)(vEncStream->pstPack[i].pu8Addr[0]+4),
                                	vEncStream->pstPack[i].u32Len[0]-4, 0);            
            }
        	else if( (0x1F & c) == 0x08 )
            {
            	set_h264PicParam(channel, 0, (char *)(vEncStream->pstPack[i].pu8Addr[0]+4),
                                	vEncStream->pstPack[i].u32Len[0]-4, 0);    
            }
            
        	len = vEncStream->pstPack[i].u32Len[0];
        	if( len > 0 )
            {
            	proDataInfo.count         = index + 1;
            	proDataInfo.buf[index]     = (char *)vEncStream->pstPack[i].pu8Addr[0];
            	proDataInfo.len[index]    = len;    
            	frameLen += len;
            	index++;
            }
        	len = vEncStream->pstPack[i].u32Len[1];
        	if( len > 0 )
            {
            	proDataInfo.count         = index + 1;
            	proDataInfo.buf[index]     = (char *)vEncStream->pstPack[i].pu8Addr[1];
            	proDataInfo.len[index]    = len;    
            	frameLen += len;
            	index++;
            }
        	if( index >= MAX_DATA_PIECE_SIZE )
            {
            	break;
            }
        } // for( i = 0; i < vEncStream->u32PackCount; ++i

    	vFrameHead.packHead.packType	    = PACK_TYPE_VIDEO;
    	vFrameHead.packHead.frameHeadLen	= sizeof( vFrameHead.frameHead );
        
    	vFrameHead.frameHead.frameLen	        = frameLen;
    	vFrameHead.frameHead.frameNo	        = GetFrameSeq( channel );
    	vFrameHead.frameHead.videoStandard	    = (uchar)VencParamEasyGetVideoStandard();
    	vFrameHead.frameHead.videoResolution	= (uchar)VencParamEasyGetResolution( channel );
    	vFrameHead.frameHead.frameRate	        = (uchar)VencParamEasyGetFramerate( channel );
    	frameType = (uchar)vEncStream->pstPack[0].DataType.enH264EType;
    	frameType = H264E_NALU_PSLICE == frameType ?  H264E_NALU_PSLICE: H264E_NALU_ISLICE;
    	vFrameHead.frameHead.frameType	        = frameType;
    	Gettimeofday( &tv, NULL );
    	vFrameHead.frameHead.sec	            = tv.tv_sec;
    	vFrameHead.frameHead.usec	            = tv.tv_usec;

    	ProconH264Write( proconH264Fd, proDataInfo );
    }

	if( NULL != aEncStream ) // audio
    {        
    	index = 0;
    	proDataInfo.count         = index + 1;
    	proDataInfo.buf[index]     = (char *)&aFrameHead;
    	proDataInfo.len[index]    = sizeof(aFrameHead);    
    	index++;

    	proDataInfo.count         = index + 1;
    	proDataInfo.buf[index]     = (char *)aEncStream->pStream;
    	proDataInfo.len[index]    = aEncStream->u32Len;    
    	frameLen	            = aEncStream->u32Len;
    	index++;

    	aFrameHead.packHead.packType	        = PACK_TYPE_AUDIO;
    	aFrameHead.packHead.frameHeadLen	    = sizeof( aFrameHead.frameHead );
        
    	aFrameHead.frameHead.frameLen	        = frameLen;
    	aFrameHead.frameHead.frameNo	        = GetFrameSeq( channel );
    	aFrameHead.frameHead.videoStandard	    = VencParamEasyGetAudioChMode();
    	aFrameHead.frameHead.videoResolution	= VencParamEasyGetAudioSampleRate();
    	aFrameHead.frameHead.frameRate	        = VencParamEasyGetAudioBitWidth();
    	aFrameHead.frameHead.frameType	        = VencParamEasyGetAudioEncodeType();
    	aFrameHead.frameHead.sec	            = tv.tv_sec;
    	aFrameHead.frameHead.usec	            = tv.tv_usec;
    	aFrameHead.frameHead.pts	            = aEncStream->u64TimeStamp;

    	ProconH264Write( proconH264Fd, proDataInfo );
    }
    #if 0
	if( 0 == channel )
    {        
    	if( NULL != vEncStream )
        {
        	SaveH264( (uchar *)&vFrameHead,
                          sizeof(vFrameHead),
                          NULL,
                          0,
                          NULL,
                          0 );
        	for( i = 0; i < vEncStream->u32PackCount; ++i )
            {
            	SaveH264( NULL,
                          0,
                          vEncStream->pstPack[i].pu8Addr[0],
                          vEncStream->pstPack[i].u32Len[0],
                          vEncStream->pstPack[i].pu8Addr[1],
                          vEncStream->pstPack[i].u32Len[1]);
            }    
        }
    } // if( 0 == channel
    #endif
	return 0;
}
#endif
/******************************************************************************
* fn: 把H264 从海思底层的地址拷贝到应用层地址,并组成一帧
* channel: 通道
* chType: 0, 主码流; 1, 从码流
* hisiH264: 从海思底层中获取到的h264 指针, 类型为 VENC_STREAM_S
*******************************************************************************/
int H264FromHisiAddrToMyAddr( int channel, void *hisiH264 )
{
	uint             	i,ret = -1;
	int	            	len, index = 0;
	unsigned long     	frameLen = 0;
	uint             	proconH264Fd;
	STREAM_HEAD_T     	vFrameHead; // 视频帧头
	DATA_PIECE_T     	proDataInfo;
	VENC_STREAM_S *vEncStream = (VENC_STREAM_S *)hisiH264;
	uchar frameType;
	struct timeval tv;
    //ShowFps( "FiStreamSendFrameToStreamBuf", channel );
    char c;

	proconH264Fd = GetChnProconH264Fd( channel );
	if( 0 == proconH264Fd )
    {
    	return -1;
    }

	if( NULL != vEncStream )
    {
    	proDataInfo.count         = index + 1;
    	proDataInfo.buf[index]    = (char *)&vFrameHead;
    	proDataInfo.len[index]    = sizeof(vFrameHead);    
    	index++;
        /////////
        proDataInfo.nalInfo.nalNum 		= vEncStream->u32PackCount;
		proDataInfo.nalInfo.nalStartOff = sizeof(vFrameHead);
        ////////
    	for( i = 0; i < vEncStream->u32PackCount; ++i )
        {
            ////////////////////////
            proDataInfo.nalInfo.nalSize[i] = vEncStream->pstPack[i].u32Len[0] 
											+ vEncStream->pstPack[i].u32Len[1];

			c = vEncStream->pstPack[i].pu8Addr[0][4];	
			if( (0x1F & c) == 0x07 )
			{
				set_h264SeqParam(channel, 0, (char *)(vEncStream->pstPack[i].pu8Addr[0]+4),
									vEncStream->pstPack[i].u32Len[0]-4, 0);			
			}
			else if( (0x1F & c) == 0x08 )
			{
				set_h264PicParam(channel, 0, (char *)(vEncStream->pstPack[i].pu8Addr[0]+4),
									vEncStream->pstPack[i].u32Len[0]-4, 0);	
			}
			
            //////////////////////////
            
        	len = vEncStream->pstPack[i].u32Len[0];
        	if( len > 0 )
            {
            	proDataInfo.count         = index + 1;
            	proDataInfo.buf[index]     = (char *)vEncStream->pstPack[i].pu8Addr[0];
            	proDataInfo.len[index]    = len;    
            	frameLen += len;
            	index++;
            }
        	len = vEncStream->pstPack[i].u32Len[1];
        	if( len > 0 )
            {
            	proDataInfo.count         = index + 1;
            	proDataInfo.buf[index]     = (char *)vEncStream->pstPack[i].pu8Addr[1];
            	proDataInfo.len[index]    = len;    
            	frameLen += len;
            	index++;
            }
        	if( index >= MAX_DATA_PIECE_SIZE )
            {
            	break;
            }
        }

                
    	vFrameHead.packHead.packType	    = PACK_TYPE_VIDEO;
    	vFrameHead.packHead.frameHeadLen	= sizeof( vFrameHead.frameHead );
        
    	vFrameHead.frameHead.frameLen	        = frameLen;
    	vFrameHead.frameHead.frameNo	        = GetFrameSeq( channel );
        //printf("frame no is %d\n",vFrameHead.frameHead.frameNo);
    	vFrameHead.frameHead.videoStandard	    = (uchar)VencParamEasyGetVideoStandard();
    	vFrameHead.frameHead.videoResolution	= (uchar)VencParamEasyGetResolution( channel );
    	vFrameHead.frameHead.frameRate	        = (uchar)VencParamEasyGetFramerate( channel );
    	frameType = (uchar)vEncStream->pstPack[0].DataType.enH264EType;
    	frameType = H264E_NALU_PSLICE == frameType ?  H264E_NALU_PSLICE: H264E_NALU_ISLICE;
    	vFrameHead.frameHead.frameType	        = frameType;
    	Gettimeofday( &tv, NULL );
    	vFrameHead.frameHead.sec	            = tv.tv_sec;
    	vFrameHead.frameHead.usec	            = tv.tv_usec;

    	ret = ProconH264Write( proconH264Fd, proDataInfo );
    }    
    
#if 0
	if( 0 == channel)
    {
    	if( NULL != vEncStream )
        {
        	SaveH264( (uchar *)&vFrameHead,
                          sizeof(vFrameHead),
                          NULL,
                          0,
                          NULL,
                          0 );
        	for( i = 0; i < vEncStream->u32PackCount; ++i )
            {
            	SaveH264( NULL,
                          0,
                          vEncStream->pstPack[i].pu8Addr[0],
                          vEncStream->pstPack[i].u32Len[0],
                          vEncStream->pstPack[i].pu8Addr[1],
                          vEncStream->pstPack[i].u32Len[1]);
            }
        }
    } // if( 0 == channel
#endif
    
	return ret;
}

static uint GetAudioFrameSeq( int channel )
{
	static uint aseq[MAX_CHANNEL_NUM];
	return aseq[channel]++;
}

int AudioFromHisiAddrToMyAddr( int channel, void *hisiAudio )
{
	int					index = 0;
	unsigned long 		frameLen = 0;
	uint 				proconH264Fd;
	STREAM_HEAD_T 		aFrameHead; 	// 音频帧头
	DATA_PIECE_T 		proDataInfo;
	AUDIO_STREAM_S 	*   aEncStream = (AUDIO_STREAM_S *)hisiAudio; 
	struct timeval tv;


    proconH264Fd = GetChnProconH264Fd( channel );
	if( 0 == proconH264Fd )
	{
		return -1;
	}

    if( NULL != aEncStream )
	{		
		index = 0;
		proDataInfo.count 		= index + 1;
		proDataInfo.buf[index] 	= (char *)&aFrameHead;
		proDataInfo.len[index]	= sizeof(aFrameHead);	
		index++;

		proDataInfo.count 		= index + 1;
		proDataInfo.buf[index] 	= (char *)aEncStream->pStream;
		proDataInfo.len[index]	= aEncStream->u32Len;	
		frameLen				= aEncStream->u32Len;
		index++;

		aFrameHead.packHead.packType			= PACK_TYPE_AUDIO;
		aFrameHead.packHead.frameHeadLen		= sizeof( aFrameHead.frameHead );
		
		aFrameHead.frameHead.frameLen			= frameLen;
		aFrameHead.frameHead.frameNo			= GetAudioFrameSeq( channel );
		aFrameHead.frameHead.videoStandard		= VencParamEasyGetAudioChMode();
		aFrameHead.frameHead.videoResolution	= VencParamEasyGetAudioSampleRate();
		aFrameHead.frameHead.frameRate			= VencParamEasyGetAudioBitWidth();
		aFrameHead.frameHead.frameType			= VencParamEasyGetAudioEncodeType();
        Gettimeofday( &tv, NULL );
		aFrameHead.frameHead.sec				= tv.tv_sec;
		aFrameHead.frameHead.usec				= tv.tv_usec;
		aFrameHead.frameHead.pts				= aEncStream->u64TimeStamp;

		ProconH264Write( proconH264Fd, proDataInfo );
	}


    return 0;
}

#if defined MCU_HI3515
static void InitTw2867()
{
	Tw2866InitVideoMode();
	Tw2866InitVideoBaseParam();
}    
#endif


#if defined MCU_HI3515A
static void init_adc()
{
    cx2866_init_videobaseparam();//use nvp1914
    //cx2866_Init_videomode();//do not use cx
}
#endif



int MpiServiceStart()
{    
	ParamEasyInit();
    //init_adc();//InitTw2867();
	InitProconH264Fd();        
    //InitProconJpgFd();
	MpiVencStart();
	FitMpiInitOsdChannel();
    //SnapMpiInit();
    
	FiOsdStartTimeOsdThread();
    //StartMdThread();
    //StartOdThread();
	return 0;
}

int MpiServiceStop()
{ 
    //StopOdThread();
    //StopMdThread();
	FiOsdStopTimeOsdThread();
    //SnapMpiDeinit();
    
	FitMpiDeinitOsdChannel();
    MympiStopAudioEnc();//
	MpiVencStop();        
        
    //DeinitProconJpgFd();//move at 20130926
	DeinitProconH264Fd();    

	return 0;
}



