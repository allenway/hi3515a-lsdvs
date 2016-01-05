/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 管理wifi fifo,主要是和wifi.sh脚本通信
**  date           :  2013.10.21
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "debug.h"
#include "linuxFile.h"
#include "wifiCom.h"
#include "wifiFifo.h"
#include "paramManage.h"

pthread_mutex_t g_wifiFifoLock = PTHREAD_MUTEX_INITIALIZER;

int WifiFifoCmd( WIFI_FIFO_CMD_EN cmd )
{
	int ret, fd;
	int writeLen;
	char writeBuf[4] = { 0 };
    
	pthread_mutex_lock( &g_wifiFifoLock );
    
	fd = open( WIFI_FIFO_DEVICE, O_RDWR );
	if( fd != -1 )
    {
    	sprintf( writeBuf, "%d", cmd);    
    	writeLen = strlen(writeBuf);    
    	SVPrint( "Writen wifi cmd(%s) to /tmp/wifi.fifo\r\n", writeBuf );
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
    	close(fd);
    }
	else
    {
    	SVPrint( "open(%s) error:%s!\r\n", WIFI_FIFO_DEVICE, STRERROR_ERRNO );
    	ret = -1;
    }
	pthread_mutex_unlock( &g_wifiFifoLock );
    
	return ret;
}

/*
*	fn : 重启wifi
*    
*/
int WifiFifoReStart()
{
	int ret = -1;
	PARAM_CONFIG_NETWORK network;

	if( (ret = ParamGetNetwork( &network )) == 0 )
    {
    	if( 1 == network.wifi.dhcpEnable ) // dchp
        {
        	ret = WifiFifoCmd( WIFI_FIFO_CMD_RESTART_DHCP );
        }
    	else if( 0 == network.wifi.dhcpEnable ) // static
        {
        	ret = WifiFifoCmd( WIFI_FIFO_CMD_RESTART );
        }
    }

	return ret;
}


