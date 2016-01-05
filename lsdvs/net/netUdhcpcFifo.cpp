/********************************************************************************
**	Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**	author        :  sven
**	version       :  v1.0
**	date           :  2013.10.10
**	description  : 管理和udhcpc 进程交互的fifo
********************************************************************************/

#include "linuxFile.h"
#include "debug.h"
#include "mutex.h"
#include "netUdhcpcFifo.h"

static ClMutexLock g_udhcpcMutex;
/*************************************************************
*  func: 	与ppp.sh同步, 控制拨号的开关
*  cmd:    	参见 UDHCPC_FIFO_CMD
*  返回:	0,成功; 其他,失败
**************************************************************/
int UdhcpcForkCtrl( UDHCPC_FIFO_CMD cmd )
{
	int ret, fd = -1;
	int writeLen;
	char writeBuf[4] = { 0 };    

	g_udhcpcMutex.Lock();
    
	fd = Open( UDHCPC_FIFO, O_RDWR );
	if( fd != -1 )
    {
    	sprintf( writeBuf, "%d", cmd);    
    	writeLen = Strlen(writeBuf);    
    	ret = Writen( fd, writeBuf, writeLen );
    	if( ret != writeLen )
        {
        	SVPrint( "Writen error:%s!\r\n", STRERROR_ERRNO );
        	ret = -1;
        }
    	else
        {
        	ret = 0;
        }
    	Close(fd);
    }
	else
    {
    	SVPrint( "open(%s) error:%s!\r\n", UDHCPC_FIFO, STRERROR_ERRNO );
    	ret = -1;
    }
    
	g_udhcpcMutex.Unlock();
    
	return ret;
}

