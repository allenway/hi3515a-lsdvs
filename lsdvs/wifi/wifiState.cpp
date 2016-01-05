/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 获取wifi的状态
**  date           :  2011.11.10
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "debug.h"
#include "linuxFile.h"
#include "wifiCom.h"
#include "wifiState.h"

pthread_mutex_t g_wifiStateLock = PTHREAD_MUTEX_INITIALIZER;

static int GetWifiSateFlag( char *pLine )
{
	int connectState = 0;
	if( NULL != pLine )
    {
    	sscanf( pLine, WIFI_STATE_MARK_FLAG"%d", &connectState );
    }

	return connectState;
}

static int GetWifiSateSinal( char *pLine )
{
	int dbm, signalLevel = 0;
	if( NULL != pLine )
    {
    	sscanf( pLine, WIFI_STATE_MARK_SIGNAL"%d", &dbm );
    	signalLevel = WifiSignalDbmToLevel( dbm );
    }

	return signalLevel;
}

int WifiStateGet( unsigned char *pConnectState, unsigned char *pSignalLevel )
{
	int ret = -1, fd;
	int connectState = 0, signalLevel = 0;
	char line[64];
	if( NULL == pConnectState || NULL == pSignalLevel )
    {
    	SVPrint( "NULL == pConnectState || NULL == pSignalLevel!\r\n" );
    	return -1;
    }

	pthread_mutex_lock( &g_wifiStateLock );
    
	fd = open( WIFI_STATE_FILE, O_RDONLY );
	if( -1 != fd )
    {
    	ret = Readline( fd, line, sizeof(line) );
    	if( ret > 0 ) connectState = GetWifiSateFlag( line );
    	ret = Readline( fd, line, sizeof(line) );
    	if( ret > 0 ) signalLevel =	GetWifiSateSinal( line );
        
    	close( fd );
    	ret = 0;
    }
    
	pthread_mutex_unlock( &g_wifiStateLock );

    *pConnectState     = (unsigned char)connectState;
    *pSignalLevel     = (unsigned char)signalLevel;
    
	return ret;        
}

