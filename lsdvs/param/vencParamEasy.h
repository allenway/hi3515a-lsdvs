#ifndef __VENCPARAMEASY_H__
#define __VENCPARAMEASY_H__

#include "const.h"

typedef struct _VideoHeadLocal_
{
	uchar videoStandard;
	uchar resolution[REAL_CHANNEL_NUM];
	uchar frameRate[REAL_CHANNEL_NUM];
} VIDEO_HEAD_LOCAL_T;

typedef struct _AudioHeadLocal_
{
	uchar	sampleRate;            // ²ÉÑùÂÊ, 0:8k, 1:16k
	uchar	bitWidth;            // Î»¿í,0:8bits, 1:16bits
	uchar	encodeType;            // ±àÂë·½Ê½,0:adpcm, 1:g711, 2:g726
	uchar	chMode;                // Í¨µÀÄ£Ê½, 0:µ¥Í¨µÀ, 1:Á¢ÌåÉù
} AUDIO_HEAD_LOCAL_T;

uchar VencParamEasyGetVideoStandard();
uchar VencParamEasyGetResolution( int channel );
uchar VencParamEasyGetFramerate( int channel );

void VencParamEasySetVideoStandard( uchar val );
void VencParamEasySetResolution( int channel, uchar val );
void VencParamEasySetFramerate( int channel, uchar val );

uchar VencParamEasyGetAudioSampleRate();
uchar VencParamEasyGetAudioBitWidth();
uchar VencParamEasyGetAudioEncodeType();
uchar VencParamEasyGetAudioChMode();
void VencParamEasySetAudioSampleRate( uchar val );
void VencParamEasySetAudioBitWidth( uchar val );
void VencParamEasySetAudioEncodeType( uchar val );
void VencParamEasySetAudioChMode( uchar val );

void ParamEasyInit();

#endif 

