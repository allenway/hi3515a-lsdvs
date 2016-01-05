/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 信号格数
**  date           :  2013.9.25
**  version       :  1.0
**  author        :  sven
********************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "debug.h"
#include "atCom.h"
#include "atEcho.h"
#include "atCsqlvl.h"

#define MIN_SIGNAL_VALUE	0
#define MAX_SIGNAL_VALUE	5

/******************************************************************
*  	func        : 从3G模块返回的信息中获取信号格数
*	pRspStr	: 从3G模块返回的信息
*	pSignalValue	: out,获取到的信号格数
*	return	: 0-pSignalValue有效;反之-无效
********************************************************************/
static int AtGetSignalValue( char *pRspStr, int *pSignalValue )
{
	int ret = -1;
	char *pResult;

	if( NULL != pRspStr && NULL != pSignalValue )
    {
    	pResult = strstr( pRspStr, "," );
    	if( pResult != NULL && pResult != pRspStr )
        {
            *pSignalValue = atoi( pResult - 1 );
            
        	if( *pSignalValue >= MIN_SIGNAL_VALUE && *pSignalValue <= MAX_SIGNAL_VALUE )
            {
            	ret = 0;
            }
        }
    }

	return ret;
}

/********************************************************
*   func	             : 获取当前信号格数
*   pSignalValue	: out,信号格数
*   return              : 0-pSignalValue 有效, 反之无效
*********************************************************/
int FiAtCsqlvlZx( int *pSignalValue )
{
	int ret = -1;    
	AT_CMD_ST *pCmd;    
    
	pCmd = &g_atCmd;    
	pthread_mutex_lock( &pCmd->lock );

	sprintf(pCmd->req, "%s%s\r", ATCMD_PRE, AT_KEY_SIGNAL_VALUE );    
	pCmd->reqSize = strlen( pCmd->req );
    
	ret = AtCmdSend( pCmd );
	FiPrint2( "------1ret(%d)!\r\n", ret );
	if( ret >= 0 ) ret = AtCmdSendSync( pCmd, (char *)AT_KEY_SIGNAL_VALUE );    
    
	FiPrint2( "------1ret(%d)!\r\n", ret );
	if( 0 == ret ) ret = AtGetSignalValue( pCmd->rsp, pSignalValue );
    
	FiPrint2( "------1ret(%d),pCmd->rsp(%s)!\r\n", ret, pCmd->rsp );
	pthread_mutex_unlock( &(pCmd->lock) );
    
	return ret;
}

/* ===== 上面的代码用于thinkwill的wcdma | 下面的代码用华为wcdma===== */

/**************************************************************************************
*  	func     : 从3G模块返回的信息中获取信号格数
*	pRspStr	: 从3G模块返回的信息
*	pSignalValue	: out,获取到的信号格数
*	返回	: 0-pSignalValue有效;反之-无效
**************************************************************************************/
static int AtGetSignalValueHw( char *pRspStr, int *pSignalValue )
{
	int ret = -1;
	int rssi = 0, ber = 0;
	char *pResult;
	int sl = 0;

	if( NULL != pRspStr && NULL != pSignalValue )
    {
    	pResult = strstr( pRspStr, AT_KEY_SIGNAL_VALUE_HW );
    	if( pResult != NULL )
        {            
        	sscanf( pResult, "+CSQ: %u,%u", &rssi, &ber);
        	if( (rssi <= 0) || (rssi == 99) )
            {
            	sl = 0;
            }
        	else if( (rssi >= 1) && (rssi < 8) )
            {
            	sl = 1;
            }
        	else if( (rssi >= 8) && (rssi < 16) )
            {
            	sl = 2;
            }
        	else if((rssi >= 16) && (rssi < 24))
            {    
            	sl = 3;
            }
        	else if((rssi >= 24) && (rssi < 31))
            {
            	sl = 4;
            }
        	else if(rssi >= 31)
            {
            	sl = 5;
            }
        	else
            {
            	sl = 0;
            }

        	ret = 0;
        }
        *pSignalValue = sl;
    }    

	return ret;
}

int FiAtCsqlvlHw( int *pSignalValue )
{
	int ret = -1;    
	AT_CMD_ST *pCmd;    
    
	pCmd = &g_atCmd;    
	pthread_mutex_lock( &pCmd->lock );

	sprintf(pCmd->req, "%s%s\r", ATCMD_PRE, AT_KEY_SIGNAL_VALUE_HW );    
	pCmd->reqSize = strlen( pCmd->req );
    
	ret = AtCmdSend( pCmd );
	if( ret >= 0 ) ret = AtCmdSendSync( pCmd, (char *)AT_KEY_SIGNAL_VALUE_HW );    
    
	if( 0 == ret ) ret = AtGetSignalValueHw( pCmd->rsp, pSignalValue );
    
	pthread_mutex_unlock( &(pCmd->lock) );
    
	return ret;
}

int FiAtCsqlvl( int *pSignalValue )
{
	int ret = -1;
    
//#if ( BOARD_TYPE == BOARD_CAR ) // 信可模块
	ret = FiAtCsqlvlZx(pSignalValue);
//#elif ( BOARD_TYPE == BOARD_NCIM ) // // 华为信可模块
//	ret = FiAtCsqlvlHw(pSignalValue);
//#endif
	return ret;
}



