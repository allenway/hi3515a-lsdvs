/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : AT打电话接口
**  date           :  2014.9.25
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "debug.h"
#include "atCom.h"

/*
* pPhone:     	电话号码
* networkType:	不同的3G模块,命令可能不一样,暂时填0。
* 返回:     	少于0,失败; 其他值,成功
*/
int FiAtSendCall( char *pPhone )
{
	int ret = -1;    
	AT_CMD_ST *pCmd = &g_atCmd;

	if( NULL == pPhone )
    {
    	SVPrint( "error:NULL == pPhone!\r\n" );
    	return -1;
    }

	pthread_mutex_lock( &pCmd->lock );
    
	sprintf(pCmd->req, "%s%s%sI;%s", ATCMD_PRE, "D", pPhone, ATCMD_CR_LF);    
	pCmd->reqSize = strlen( pCmd->req );
    
	ret = AtCmdSend( pCmd );

	if( ret >= 0 ) ret = AtCmdSendSync( pCmd, (char *)AT_CHARS_OK );
    
	pthread_mutex_unlock(&(pCmd->lock));
    
	return ret;
}

