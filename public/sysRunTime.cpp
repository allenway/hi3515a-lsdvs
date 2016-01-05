/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.01.17
**  description  : 计算系统启动后所走过的时间
********************************************************************************/
#include <unistd.h>
#include <sys/time.h>
#include "timer.h"

/**************************************************************************
* func: 计算系统reset后所走过的时间
***************************************************************************/
static struct timeval g_sysRunTime;
static void *UpdateSysRunTime( void *arg )
{
	static struct timeval base;
	struct timeval now;
	struct timeval sub;

	gettimeofday( &now, NULL );
	if( timercmp(&now, &base, <) )
    {
    	base = now;
    	return NULL;
    }
	timersub( &now, &base, &sub );
	if( sub.tv_sec > 10 )
    {
    	base = now;
    	return NULL;
    }
	base = now;
	timeradd( &g_sysRunTime, &sub, &g_sysRunTime );
    
	return NULL;
}

/********************************************************************
* 返回: 系统reset后所走过的时间(秒)
**********************************************************************/
int SysRunTimeGet()
{    
	int runTime;

	runTime	= g_sysRunTime.tv_sec;

	return runTime;
}

//开一个定时器,计算系统走过的时间
void SysRunTimeAddTimer()
{
	AddRTimer( UpdateSysRunTime, NULL, 1 );
}

