/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**	文件名: encComm.h
**  description  : 定义一些编码库公用的宏和常量
**  date           :  2013.10.18
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#ifndef __ENC_COMM_H__
#define __ENC_COMM_H__

#include "const.h"

/******AD芯片类型******/
#define AD_NOCHIP		0
#define AD_TVP5150		1
#define AD_TW2815A     	2
#define AD_TW2815AB		3
#define AD_TW2815ABCD	4
#define	AD_CMOS_MT9D131	5
#define AD_TW2865A      6
/******视频制式******/
typedef enum VideoStandard
{
	HI_PAL = 0,
	HI_NTSC = 1
}VIDEO_STANDARD_E;

/******视频分辨率******/
typedef enum PictureFormat 
{
	HI_QCIF = 0,
	HI_CIF,
	HI_2CIF,
	HI_HD1,
	HI_D1,
	HI_MD1,
	HI_QVGA,
	HI_VGA,
	HI_SXGA
}PICTURE_FORMAT_E;

/******视频流控类型******/
typedef enum VideoBitrateMode
{
	HI_CBR = 0,
	HI_VBR = 1
}VIDEO_BITRATE_MODE;

/******编码码流种类******/
typedef enum
{
	HI_AUDIO = 0,
	HI_VIDEO = 1,
	HI_AUDIO_VIDEO = 2
}VideoEncMode;

#define DEFAULT_ENCODE_STANDARD	    	HI_PAL         //PAL
#define DEFAULT_ENCODE_RESOLUTION		HI_D1	    //cif
#define DEFAULT_ENCODE_SUB_RESOLUTION	HI_CIF         //cif
#define DEFAULT_ENCODE_BITRATE_TYPE		HI_CBR         //CBR
#define DEFAULT_ENCODE_LEVEL	    	128	        //画质(即码率32~4000 kbits) 
#define DEFAULT_ENCODE_FRAME_RATE		25	        //帧率(PAL:1~25,NTSC:1~30)
#define DEFAULT_ENCODE_FRAME_INTERVAL	50	        //I帧间隔(1~149)
#define DEFAULT_ENCODE_PREFER_FRAME		1              //0 画质优先, 1 贞率优先 ,3512已经不支持
#define DEFAULT_ENCODE_MAX_QP	    	32	        //QP,3512已经不支持
#define DEFAULT_ENCODE_TYPE	        	HI_VIDEO    //0: 音视频流, 1: 视频流, 2: 音频流
#define DEFAULT_AUDIO_SAMPLE_RATE		0	        //8K	
#define DEFAULT_AUDIO_BIT_WIDTH	    	1	        //16bit
#define DEFAULT_AUDIO_ENCODE_TYPE		5	        //g726_24k
#define DEFAULT_AMR_MODE	        	0	        //MIME
#define DEFAULT_AMR_FORMAT	        	0	        //4.75K

#define MAX_CHANNEL_PER_DEV	        	4	        // 一个videv 支持4个通道时分复用
#define MAX_JPEG_SIZE                     (512*1024)        /*抓拍最大支持512KbytesJPEG*/

#endif //__ENC_COMM_H__

