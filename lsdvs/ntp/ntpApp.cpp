/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : ntp 应用
********************************************************************************/

#include <unistd.h>
#include "debug.h"
#include "thread.h"
#include "ntpApp.h"
#include "ntpclient.h"
#include "message.h"
#include "message.h"
#include "paramManage.h"
#include "timeExchange.h"
#include "rtc.h"
#include "realTime.h"

static NTP_APP_T g_ntpApp;
static THREAD_MAINTAIN_T g_ntpAppTh;

void StartNtpAppService()
{
	g_ntpAppTh.runFlag = 1;
	ThreadCreate( &g_ntpAppTh.id, NtpThread, NULL );    
}

void StopNtpAppService()
{
	NtpAppOutMaintain();
	g_ntpAppTh.runFlag = 0;
	ThreadJoin( g_ntpAppTh.id, NULL );
}

/*****************************************************
* fn: ntpclient 线程工作标志
******************************************************/
int NtpAppGetRunFlag()
{
	return g_ntpAppTh.runFlag;
}

/*********************************************************
* fn: 获取使能标识
*********************************************************/
static unsigned char NtpAppGetEnableFlag()
{
	return g_ntpApp.enable;
}

/*********************************************************
* fn: 维护ntpclient 的工作状态
**********************************************************/
int NtpAppMaintain()
{
	if( MessageRecv(MSG_ID_RESTART_NTP) >= 0 )
    {
    	return NTP_MAINTAIN_RESTART;
    }
	else if( 1 != NtpAppGetEnableFlag() )
    {
    	sleep( 2 );
    	return NTP_MAINTAIN_NOT_ENABLE;
    }

	return NTP_MAINTAIN_WORK;
}

/***********************************************************************
* fn: 改变ntpclient 的工作状态, 进行一些重新初始化的动作
      该函数一般在修改ntp 参数修改后被调用
*************************************************************************/
void NtpAppOutMaintain()
{
	MessageSend(MSG_ID_RESTART_NTP);
}

/**************************************************************************
* fn: 从参数模块获取ntpclient 的工作参数
* pInterval: out, 对时间隔
* pHost: 对时的主机
***************************************************************************/
int NtpAppGetParam( int *pInterval, char *pHost )
{
	int ret;
	PARAM_CONFIG_NTP ntpParam;
    
	if( NULL == pInterval || NULL == pHost )
    {
    	SVPrint( "error:NULL == pInterval || NULL == pHost!\r\n" );
    	return -1;
    }
    
	ret = ParamGetNtp( &ntpParam );
	if( 0 == ret )
    {
    	g_ntpApp.enable     = ntpParam.enable;
    	if( 0 == ntpParam.interval )
        {
        	ntpParam.interval = 600;
        }
    	g_ntpApp.interval     = ntpParam.interval;
        *pInterval             = ntpParam.interval;
    	if( 0 == ntpParam.zone )
        {
        	ntpParam.zone = 8;
        }
    	g_ntpApp.zone         = ntpParam.zone;
    	strncpy( g_ntpApp.host, ntpParam.host, sizeof(g_ntpApp.host) );
    	strncpy( pHost, ntpParam.host, sizeof(g_ntpApp.host) );
    	g_ntpApp.count = 0;
        
    }
    
	return ret;
}

/*************************************************************************************
* fn: 保存ntp 对时成功后的时间
      如果是第一次对时成功,或者对时失败,则把时间直接设到系统
* second: 对时后的时间, utc
*************************************************************************************/
int NtpAppSaveTime( int ntpSec )
{    
	int ret = -1;
	int year, month, day, hour, minute, second;
	int localSec = ntpSec + 3600 * g_ntpApp.zone;

	FiTimeUtcToHuman( localSec, &year, &month, &day, &hour, &minute, &second );
	if( 1 == TimeIsValidDatetime(year, month, day, hour, minute, second) )
    {
    	ret = RtcSetTime( year, month, day, hour, minute, second );
    	if( 0 == g_ntpApp.count || -1 == ret )
        {
        	g_ntpApp.count++;
        	RealTimeSetDatetime( year, month, day, hour, minute, second );
        }
    }

	return ret;
}

