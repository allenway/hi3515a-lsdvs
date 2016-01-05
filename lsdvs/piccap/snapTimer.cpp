/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 定时抓拍
********************************************************************************/


#include <stdio.h>
#include "const.h"
#include "debug.h"
#include "timer.h"
#include "public.h"
#include "snapApp.h"
#include "paramManage.h"

SNAP_WORK_PARAM_T g_snapWorkParam[MAX_JPG_CHN_NUM];
void InitSnapWorkParam()
{
	int i, ret;
	int hh, mm, ss;
	PARAM_CONFIG_SNAP_TIMER param;
	for( i = 0; i < MAX_JPG_CHN_NUM; ++i )
    {
    	ret = ParamGetSnapTimer( i, &param );
    	if( 0 == ret )
        {
        	g_snapWorkParam[i].enable     = param.enable;
        	g_snapWorkParam[i].interval = param.interval;
        	sscanf( param.startTime, "%d:%d:%d", &hh, &mm, &ss );
        	g_snapWorkParam[i].startSec = hh * 3600 + mm * 60 + ss;
        	sscanf( param.stopTime, "%d:%d:%d", &hh, &mm, &ss );
        	g_snapWorkParam[i].stopSec = hh * 3600 + mm * 60 + ss;            
        }
    }
}

void SnapTimerReleaseWorkParam()
{
	InitSnapWorkParam();
}

static void *SnapTimer( void *arg )
{
	int i;
	int curTime = time( NULL );
	int dateSec = curTime % 86400;
	static int elapse[MAX_JPG_CHN_NUM];
    
	for( i = 0; i < MAX_JPG_CHN_NUM; i++ )
    {
    	if( ++elapse[i] >= g_snapWorkParam[i].interval )
        {
        	if( 1 == g_snapWorkParam[i].enable )
            {
            	if( g_snapWorkParam[i].startSec <= dateSec 
                    && dateSec <= g_snapWorkParam[i].stopSec )
                    {                        
                    	SnapAppMessageSend( i, SNAP_TYPE_TIMER );
                    }
            }
        	elapse[i] = 0;
        }
    }

	return NULL;
}

void AddSnapTimer()
{
	AddRTimer( SnapTimer, NULL, 1 );
}


