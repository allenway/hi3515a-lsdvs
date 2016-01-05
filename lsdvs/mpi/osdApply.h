/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**	文件名: osdApply.h
**  description  : for osdApply.cpp
**  date           :  2013.10.18
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/

#ifndef __OSD_APPLY__
#define __OSD_APPLY__

#include "ttypes.h"

#define MAX_OSD_CHN		REAL_CHANNEL_NUM

#define MAX_ASC_NUM		255

#define MAX_LOGO_LEN    64

enum 
{
	OSDEVENT_VECREATE=1, 
	OSDEVENT_VEUPDATE, 
	OSDEVENT_VEDESTROY, 
	OSDEVENT_TIMEUPDATE, 
	OSDEVENT_LOGOUPDATE
};

typedef struct OsdLogo
{
	unsigned char enable;    /* 1: enable, 0: disable */
	unsigned char colorRed;/* 0: white, 1: red, 2: blue */
	unsigned char colorGreen;    
	unsigned char colorBlue;    
	unsigned short xPos;
	unsigned short yPos;
	char logo[MAX_LOGO_LEN];
	unsigned char  bgTransparence;        // 背景透明度 0~100,标示0%~100%
	char	reserved[7];
}OSD_LOGO;

typedef struct OsdTime
{
	unsigned char enable;    /* 1: enable, 0: disable */
	unsigned char colorRed;    /* 0: white, 1: red, 2: blue */
	unsigned char colorGreen;    
	unsigned char colorBlue;    
	unsigned short xPos;
	unsigned short yPos;
	unsigned char  bgTransparence;        // 背景透明度 0~100,标示0%~100%
	char	reserved[7];
}OSD_TIME;

int FiOsdDoKindOfEvent(int event,int channel);
int FiOsdSetTimeOsdStruct(int channel,OSD_TIME *timeOsd);
int FiOsdGetTimeOsdStruct(int channel,OSD_TIME *timeOsd);

void FiOsdInitLogoOsdStruct(void);
void FiOsdInitTimeOsdStruct(void);

int FiOsdSetLogoOsdConfig(int channel,CONFIG_OSD_LOGO *logoOsd);
int FiOsdSetTimeOsdConfig(int channel,CONFIG_OSD_TIME *timeOsd);

void FiRefreshOsdTime(void);
int FiOsdStartTimeOsdThread(void);
int FiOsdStopTimeOsdThread(void);


void FiOsdInitTimeOsdStruct(void);

#endif //__OSD_APPLY__

