/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 管理3G拨号服务
**  date           :  2014.9.25
**
**  version       :  1.0
**  author        :  sven
********************************************************************************/
#ifndef __THREEGPPP_H__
#define __THREEGPPP_H__

#include <pthread.h>

#define THREEG_PPPSH_FIFO	"/tmp/ppp.fifo" // 与 ppp.sh 同步的fifo
typedef enum _ThreegPppFifoCmd_
{
	THREEG_PPP_FIFO_CMD_START = 1,
	THREEG_PPP_FIFO_CMD_STOP,
	THREEG_PPP_FIFO_CMD_RESTART
} THREEG_PPP_FIFO_CMD;

typedef struct _ThreegPppSt_
{
	pthread_mutex_t lock;
	int	        	fifoFd;    // 与 ppp.sh 同步的fifo句柄	
} THREEG_PPP_ST;

int ThreegPppInit();
int ThreegPppDeInit();
int ThreegPppControl( THREEG_PPP_FIFO_CMD cmd );

#endif

