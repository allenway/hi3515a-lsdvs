/*
*********************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.11.06
**  description  : 封装一下hisi 的mpi 接口, 给外界提供接口
*********************************************************************
*/

#include "mpi_venc.h"
#include "hi_common.h"
#include "hicomm.h"
#include "const.h"
#include "debug.h"
#include "mutex.h"
#include "vencParamEasy.h"
#include "mpiApp.h"

static CMutexLock g_mympiMutex;
/*
* fn: 强制i 帧
*/
int MympiForceIframe( int channel )
{
	return HI_MPI_VENC_RequestIDR( channel << 1 );
}

/*
* fn: 设置图像等级,[0,5],
* val: [0..5]. Valid when CBR/VBR.
 	With different RC mode, it means differ.
 	CBR: scope of bitrate fluctuate. 1-5: 10%-50%. 0: SDK optimized, recommended;
 	VBR: Quality of picture. 0: Best; 5: Worst.
*/
int MympiSetLevel( int channel, int val )
{
#if defined MCU_HI3515
	int ret = 0;
	VENC_CHN_ATTR_S stAttr;
	VENC_ATTR_H264_S stH264Attr;
	int vencCh = channel << 1;

	if( channel < 0 || channel > REAL_CHANNEL_NUM || val <  0 || val > 5 )
    {
    	SVPrint( "failed: %s( channel(%d) val(%d) )!\r\n",	__FUNCTION__, channel, val );
    	return -1;
    }

	g_mympiMutex.Lock();

	stAttr.enType = PT_H264;
	stAttr.pValue = &stH264Attr;
	memset( &stH264Attr, 0, sizeof(VENC_ATTR_H264_S) );
	ret = HI_MPI_VENC_GetChnAttr( vencCh, &stAttr );
	if (HI_SUCCESS == ret)
    {
    	stH264Attr.u32PicLevel = val;
    	ret = HI_MPI_VENC_SetChnAttr( vencCh, &stAttr );            
    }



	g_mympiMutex.Unlock();
    
	SVPrint( "ret(0x%X) = %s( channel(%d) val(%d) )!\r\n", ret, __FUNCTION__, channel, val );
    return ret;
#endif	

	return 0;
}

/*
* fn: 设置比特率类型
*/
int MympiSetBitrateType( int channel, int val )
{
#if defined MCU_HI3515

	int ret = 0;
	VENC_CHN_ATTR_S stAttr;
	VENC_ATTR_H264_S stH264Attr;
	int vencCh = channel << 1;

	if( channel < 0 || channel > REAL_CHANNEL_NUM || val <  RC_MODE_VBR || val > RC_MODE_FIXQP )
    {
    	SVPrint( "failed: %s( channel(%d) val(%d) )!\r\n",	__FUNCTION__, channel, val );
    	return -1;
    }

	g_mympiMutex.Lock();

	stAttr.enType = PT_H264;
	stAttr.pValue = &stH264Attr;
	memset( &stH264Attr, 0, sizeof(VENC_ATTR_H264_S) );
    
	ret = HI_MPI_VENC_GetChnAttr( vencCh, &stAttr );
	if( HI_SUCCESS == ret )
    {
    	stH264Attr.enRcMode = (RC_MODE_E)val;
    	ret = HI_MPI_VENC_SetChnAttr( vencCh, &stAttr );
    }
    
	g_mympiMutex.Unlock();
    
	SVPrint( "ret(0x%X) = %s( channel(%d) val(%d) )!\r\n", ret, __FUNCTION__, channel, val );
    
	return ret;
#endif
    return 0;
}

/*
* fn: 设置比特率
*/
int MympiSetBitrate( int channel, int val )
{
#if defined MCU_HI3515

	int ret = 0;
	VENC_CHN_ATTR_S stAttr;
	VENC_ATTR_H264_S stH264Attr;
	int vencCh = channel << 1;

	if( channel < 0 || channel > REAL_CHANNEL_NUM || val <  1 || val > 20000 )
    {
    	SVPrint( "failed: %s( channel(%d) val(%d) )!\r\n",	__FUNCTION__, channel, val );
    	return -1;
    }

	g_mympiMutex.Lock();

	stAttr.enType = PT_H264;
	stAttr.pValue = &stH264Attr;
	memset( &stH264Attr, 0, sizeof(VENC_ATTR_H264_S) );
    
	ret = HI_MPI_VENC_GetChnAttr( vencCh, &stAttr );
	if( HI_SUCCESS == ret )
    {
    	stH264Attr.u32Bitrate = val;
    	ret = HI_MPI_VENC_SetChnAttr( vencCh, &stAttr );
    }
    
	g_mympiMutex.Unlock();
    
	SVPrint( "ret(0x%X) = %s( channel(%d) val(%d) )!\r\n", ret, __FUNCTION__, channel, val );
    
	return ret;
#endif
    return 0;
}


/*
* fn: 设置帧率率
*/
int MympiSetFramerate( int channel, int val )
{
#if defined MCU_HI3515

	int ret = 0;
	VENC_CHN_ATTR_S stAttr;
	VENC_ATTR_H264_S stH264Attr;
	int vencCh = channel << 1;

	if( channel < 0 || channel > REAL_CHANNEL_NUM || val <	1 || val > 30 )
    {
    	SVPrint( "failed: %s( channel(%d) val(%d) )!\r\n",	__FUNCTION__, channel, val );
    	return -1;
    }
	if( VIDEO_ENCODING_MODE_PAL == VencParamEasyGetVideoStandard() )
    {
    	if( val > 25 )
        {
        	val = 25;
        }
    }

	g_mympiMutex.Lock();

	stAttr.enType = PT_H264;
	stAttr.pValue = &stH264Attr;
	memset( &stH264Attr, 0, sizeof(VENC_ATTR_H264_S) );
    
	ret = HI_MPI_VENC_GetChnAttr( vencCh, &stAttr );
	if( HI_SUCCESS == ret )
    {
    	stH264Attr.u32TargetFramerate     = val;
    	ret = HI_MPI_VENC_SetChnAttr( vencCh, &stAttr );
    }
    
	g_mympiMutex.Unlock();
    
	SVPrint( "ret(0x%X) = %s( channel(%d) val(%d) )!\r\n", ret, __FUNCTION__, channel, val );
    
	return ret;
#endif
    return 0;
}

/*
* fn: 设置I 帧间隔
*/
int MympiSetIframeInterval( int channel, int val )
{
#if defined MCU_HI3515    
	int ret = 0;
	VENC_CHN_ATTR_S stAttr;
	VENC_ATTR_H264_S stH264Attr;
	int vencCh = channel << 1;

	if( channel < 0 || channel > REAL_CHANNEL_NUM || val <	1 || val > 1000 )
    {
    	SVPrint( "failed: %s( channel(%d) val(%d) )!\r\n",	__FUNCTION__, channel, val );
    	return -1;
    }
    
	g_mympiMutex.Lock();

	stAttr.enType = PT_H264;
	stAttr.pValue = &stH264Attr;
	memset( &stH264Attr, 0, sizeof(VENC_ATTR_H264_S) );
    
	ret = HI_MPI_VENC_GetChnAttr( vencCh, &stAttr );
	if( HI_SUCCESS == ret )
    {
    	stH264Attr.u32Gop	= val;
    	ret = HI_MPI_VENC_SetChnAttr( vencCh, &stAttr );
    }
    
	g_mympiMutex.Unlock();
    
	SVPrint( "ret(0x%X) = %s( channel(%d) val(%d) )!\r\n", ret, __FUNCTION__, channel, val );
    
	return ret;
#endif
    return 0;
}

/*
* fn: 设置分辨率
	1. 当设置视频参数涉及到视频分辨率时调用
	2. 发送消息告诉定时器,执行重启视频编码线程的动作
	3. 设置分辨率涉及到资源的重新分配,所以发消息重启编码线程
*/
int MympiSetResolution()
{
	FiEncSendRestartMessage();
	return 0;
}

/*
* fn: 设置视频标准
     1. 当设置视频参数涉及到视频制式时调用
     2. 发送消息告诉定时器,执行重启视频编码线程的动作
     3. 设置视频制式涉及到资源的重新分配,所以发消息重启编码线程
*/
int MympiSetVideoStandard()
{
	FiEncSendRestartMessage();
	return 0;
}

/*
* fn: 设置音频编码类型
     1. 当设置音频参数涉及到编码类型时调用
     2. 发送消息告诉定时器,执行重启视频编码线程的动作
     3. 如果仅仅重启音频编码部分,可能会影响到音视频同步通道,所以整个编码库重启
*/
int MympiSetAudioEncType()
{
	FiEncSendRestartMessage();
	return 0;
}


