/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 这里的接口主要给mpi 视频编码取视频帧的时候填充帧头使用的
        	这些接口其实可以用 paramManage.h 里面的接口实现,但是由于使用
        	频率非常高,每个通道的每一帧都用到,paramManage.h 里面的接口都
        	涉及到加锁解锁动作,效率有点低,所以专门使用被文档的接口
********************************************************************************/

#include "paramManage.h"
#include "vencParamEasy.h"

/* ===== video ===== */
static VIDEO_HEAD_LOCAL_T g_videoHeadLocal;

uchar VencParamEasyGetVideoStandard()
{
	return g_videoHeadLocal.videoStandard;
}

uchar VencParamEasyGetResolution( int channel )
{
	return g_videoHeadLocal.resolution[channel];
}

uchar VencParamEasyGetFramerate( int channel )
{
	return g_videoHeadLocal.frameRate[channel];
}

void VencParamEasySetVideoStandard( uchar val )
{
	g_videoHeadLocal.videoStandard = val;
}

void VencParamEasySetResolution( int channel, uchar val )
{
	g_videoHeadLocal.resolution[channel] = val;
}

void VencParamEasySetFramerate( int channel, uchar val )
{
	g_videoHeadLocal.frameRate[channel] = val;
}

static void VencParamEasyInit()
{
	int ret, i;
	PARAM_CONFIG_VIDEO_ENCODE_PUBLIC vepp;
	PARAM_CONFIG_VIDEO_ENCODE vep;
    
	ret = ParamGetVideoEncodePublic( &vepp );
	if( 0 == ret )
    {
    	g_videoHeadLocal.videoStandard = vepp.videoStandard;
    }
	for( i = 0; i < REAL_CHANNEL_NUM; ++i )
    {
    	ret = ParamGetVideoEncode( i, &vep );
    	if( 0 == ret )
        {
        	g_videoHeadLocal.resolution[i]    = vep.resolution;        
        	g_videoHeadLocal.frameRate[i]     = vep.frameRate;
        }
    }
}

/* ===== audio ===== */

static AUDIO_HEAD_LOCAL_T g_audioHeadLocal;

uchar VencParamEasyGetAudioSampleRate()
{
	return g_audioHeadLocal.sampleRate;
}

uchar VencParamEasyGetAudioBitWidth()
{
	return g_audioHeadLocal.bitWidth;
}

uchar VencParamEasyGetAudioEncodeType()
{
	return g_audioHeadLocal.encodeType;
}

uchar VencParamEasyGetAudioChMode()
{
	return g_audioHeadLocal.chMode;
}

void VencParamEasySetAudioSampleRate( uchar val )
{
	g_audioHeadLocal.sampleRate = val;
}

void VencParamEasySetAudioBitWidth( uchar val )
{
	g_audioHeadLocal.bitWidth = val;
}

void VencParamEasySetAudioEncodeType( uchar val )
{
	g_audioHeadLocal.encodeType = val;
}

void VencParamEasySetAudioChMode( uchar val )
{
	g_audioHeadLocal.chMode = val;
}

static void AudioParamEasyInit()
{
	int ret;
	PARAM_CONFIG_AUDIO_ENCODE param;
	ret = ParamGetAudio( &param );
	if( 0 == ret )
    {
    	VencParamEasySetAudioSampleRate( param.sampleRate );
    	VencParamEasySetAudioBitWidth( param.bitWidth );
    	VencParamEasySetAudioEncodeType( param.encodeType );
    	VencParamEasySetAudioChMode( param.chMode );
    }
}

void ParamEasyInit()
{
	VencParamEasyInit();
	AudioParamEasyInit();
}

