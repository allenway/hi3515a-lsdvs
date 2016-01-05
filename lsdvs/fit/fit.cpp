/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 为了让这一套工程代码适应不同的硬件平台而建立这个文件
********************************************************************************/

#include "const.h"
#include "ttypes.h"
#include "mympi.h"
#include "mpiApp.h"
#include "snapMpi.h"
#include "osdApply.h"
#include "mpiViFrame.h"

#include "osdViVo.h"

void FitEncAddTimer()
{
	EncAddTimer();
}

int FitFiOsdInitOsdLib()
{
	return FiOsdInitOsdLib();
}

int FitFiOsdDeinitOsdLib()
{
	return FiOsdDeinitOsdLib();
}

int FitMpiServiceStart()
{
	return MpiServiceStart();
}

int FitMpiServiceStop()
{
	return MpiServiceStop();

}

int FitSnapMpiGetJpgAndToProcon( int snapCh, uint snapType )
{
#if defined MCU_HI3515
	return SnapMpiGetJpgAndToProcon( snapCh, snapType );
#else
	return 0;
#endif
}

int FitMympiForceIframe( int channel )
{
	return MympiForceIframe( channel );
}

/*
* fn: 设置图像等级,[0,5],
* val: [0..5]. Valid when CBR/VBR.
 	With different RC mode, it means differ.
 	CBR: scope of bitrate fluctuate. 1-5: 10%-50%. 0: SDK optimized, recommended;
 	VBR: Quality of picture. 0: Best; 5: Worst.
*/
int FitMympiSetLevel( int channel, int val )
{
#if defined MCU_HI3515
	return MympiSetLevel( channel, val );
#else
	return 0;
#endif
}

/*
* fn: 设置比特率类型
* val: 码率控制模式, 0,可变码率 VBR 模式; 1,固定码率 CBR 模式;
*/
int FitMympiSetBitrateType( int channel, int val )
{
#if defined MCU_HI3515
	return MympiSetBitrateType( channel, val );
#else
	return 0;
#endif
}


/*
* fn: 设置比特率
* val: ：[1, 20000], bit rate. valid when CBR/VBR/ABR.
      With different RC mode, it means differ.
      CBR/ABR: average bitrate; 
      VBR: Max bitrate;
*/
int FitMympiSetBitrate( int channel, int val )
{
#if defined MCU_HI3515
	return MympiSetBitrate( channel, val );
#else
	return 0;
#endif
}

/*
* fn: 设置帧率率
* val: 目标帧率,P 制(0, 25],N 制(0, 30]
*/
int FitMympiSetFramerate( int channel, int val )
{
#if defined MCU_HI3515
	return MympiSetFramerate( channel, val );
#else
	return 0;
#endif
}

/*
* fn: 设置I 帧间隔
* val: I 帧间隔,[0, 1000]
*/
int FitMympiSetIframeInterval( int channel, int val )
{
#if defined MCU_HI3515
	return MympiSetIframeInterval( channel, val );
#else
	return 0;
#endif
}

/*
* fn: 设置分辨率
      当设置视频参数涉及到视频分辨率时调用
*/
int FitMympiSetResolution()
{
#if defined MCU_HI3515
	return MympiSetResolution();
#else
	return 0;
#endif
}

int FitMympiSetVideoStandard()
{
#if defined MCU_HI3515A
	return MympiSetVideoStandard();
#else
	return 0;
#endif
}
                    
int FitFiOsdSetLogoOsdConfig( int channel, ConfigOsdLogo *setVal )
{
#if defined MCU_HI3515A
	return FiOsdSetLogoOsdConfig( channel, setVal );
#else
	return 0;
#endif
}

int FitFiOsdSetTimeOsdConfig( int channel, ConfigOsdTime *setVal )
{
#if defined MCU_HI3515A
	return FiOsdSetTimeOsdConfig( channel, setVal );
#else
	return 0;
#endif
}

/*
* fn: 设置伴音开关
* openFlag: 0,音视频; 其他值, 音频
*/
int FitMympiSetAvencAccompanyingAudio( int channel, int openFlag )
{
#if defined MCU_HI3515
	if( 0 == openFlag )
    {
    	return MympiSetAvencAccompanyingAudio( channel, 1 );
    }
	else
    {
    	return MympiSetAvencAccompanyingAudio( channel, 0 );
    }
#else
	return 0;
#endif
}

/*
* fn: 获取视频丢失状态
*/
int FitMympiGetShelterDetect( int channel )
{
#if defined MCU_HI3515A
	return VideoGetShelterDetect( channel );
#else
	return 0;
#endif
}

int FitMympiSetAudioEncType()
{
#if defined MCU_HI3515A
	return MympiSetAudioEncType();
#else
	return 0;
#endif
}


