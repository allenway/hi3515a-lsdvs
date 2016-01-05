/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : AT GPS控制话接口
**  date           :  2014.9.25
**
**  version       :  1.0
**  author        :  sven
********************************************************************************/

#include <stdio.h>
#include <string.h>
#include <pthread.h>
    
#include "debug.h"
#include "atCom.h"
#include "atGps.h"

int FiAtGpsCtl( AT_GPS_EN flag )
{
	int ret = -1;    
	AT_CMD_ST *pCmd;    
    
	pCmd = &g_atCmd;    
	pthread_mutex_lock( &pCmd->lock );
    
	if( AT_GPS_OFF == flag )
    {
    	sprintf( pCmd->req, "%s%s\r", ATCMD_PRE, ATCMD_GSP_STOP_KEY );
    }
	else if( AT_GPS_ON == flag )
    {
    	sprintf( pCmd->req, "%s%s\r", ATCMD_PRE, ATCMD_GSP_START_KEY );
    }
    
	pCmd->reqSize = strlen( pCmd->req );
    
	ret = AtCmdSend( pCmd );

	if( ret >= 0 ) ret = AtCmdSendSync( pCmd, (char *)AT_CHARS_OK );
    
	pthread_mutex_unlock( &(pCmd->lock) );
    
	return ret;
}


