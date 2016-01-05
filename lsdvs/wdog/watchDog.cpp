/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.23
**  description  : watchdog
********************************************************************************/


#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/time.h>
#include "debug.h"
#include "const.h"
#include "thread.h"
#include "timer.h"
#include "watchDog.h"


#define WATCHDOG_TIMEOUT		20   //  second
#define WATCHDOG_DEVICE_PATH	"/dev/watchdog"

static signed int g_wdt_fd = -1;
static THREAD_MAINTAIN_T g_WdtTm = {0};

/********************************************
* fn: GPIO 初始化，打开GPIO设备
* return : 0:成功    -1:失败
*********************************************/
static int WatchDogInit(void)
{
	int ret = FI_FAIL;
	int wdt_ret;
	unsigned long timeout;

    if(g_wdt_fd <= 0)
    {
    	g_wdt_fd = open( WATCHDOG_DEVICE_PATH, O_RDWR );
    	if(g_wdt_fd <= 0)
        {
        	ERRORPRINT("failed: WatchDogInit!\r\n");
        	ret = FI_FAIL;
        }
    	else 
        {
            // 设置复位时间 
        	timeout = WATCHDOG_TIMEOUT/2;
        	wdt_ret = ioctl( g_wdt_fd, WDIOC_SETTIMEOUT, &timeout);
        	if(wdt_ret != 0)
            {
            	ERRORPRINT("failed: watch dog SET TIMEOUT ioctl() !\r\n");
            }                        
        	ret = FI_SUCCESS;
        }
    }
    
    return ret;
}

//feed dog
static void *WdtThread( void *arg )
{
	int wdt_ret = 1;

	SVPrint( "%s start!\r\n", __FUNCTION__ );

	while( g_WdtTm.runFlag )
    {
        if(g_wdt_fd <= 0)
        {
        	ERRORPRINT("failed: watch dog feed fd<0 !\r\n");
        }
    	else 
        {    
        	wdt_ret = ioctl( g_wdt_fd, WDIOC_KEEPALIVE, (unsigned long)NULL );
        	if(wdt_ret != 0)
            {
            	ERRORPRINT("failed: watch dog feed ioctl() !\r\n");
            }                        
        }
    	usleep( 5000*1000 );
    }
    
	ERRORPRINT( "%s stop!\r\n", __FUNCTION__ );
	return NULL;
}




void WdtServiceStart()
{
	int ret;
    
	WatchDogInit();
    
	g_WdtTm.runFlag = 1;
	ret = ThreadCreateSched( &g_WdtTm.id, WdtThread, NULL, 
                             THREAD_SCHED_POLICY, THREAD_PRIORITY_WDT );
	if( ret )
    {        
    	g_WdtTm.runFlag = 0;
    	ERRORPRINT( "error:ThreadCreateCommonPriority:%s\r\n", STRERROR_ERRNO );
    }
}

void WdtServiceStop()
{
	int ret;
    
	g_WdtTm.runFlag = 0;
    
	ret = ThreadJoin( g_WdtTm.id, NULL );
	if( 0 != ret )
    {
    	ERRORPRINT( "error:ThreadJoin:%s\r\n", STRERROR_ERRNO );
    }

    if(g_wdt_fd > 0)
    {
        ret = close(g_wdt_fd);
        if(ret < 0)
        {
            ERRORPRINT("watch dog driver close failed!\n");
        }
        else
        {
            g_wdt_fd = -1;
        }
    }
}



