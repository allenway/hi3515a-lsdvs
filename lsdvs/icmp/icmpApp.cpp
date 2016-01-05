/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.10.10
**  description  : 应用icmp 实现各种应用
*******************************************************************************
*/
#include <unistd.h>
#include <stdlib.h>
#include "debug.h"
#include "thread.h"
#include "message.h"
#include "condition.h"
#include "icmp.h"
#include "paramManage.h"
#include "icmpApp.h"
#include "sysRunTime.h"

static ClCondition g_icmpWorkCondition;  
static int g_curState;
static THREAD_MAINTAIN_T g_icmpAppTm;

static void SetNetworkStatus( int state )
{
	g_curState = state;
}
/*
* fn: 通过icmp 的状态获取当前的网络状态
* 返回: 1,能ping 通; 0,ping 不通
*/
int IcmpGetNetworkState()
{
	return g_curState;
}

static void GetIcmpWork( ICMP_WORK_PARAM_T *icmpWorkParam )
{
	int ret;
	PARAM_CONFIG_ICMP icmpConf;
    
	icmpWorkParam->enable = 0;
	ret = ParamGetIcmp( 0, &icmpConf );
	if( 0 == ret )
    {
    	strcpy( icmpWorkParam->ipAddr, icmpConf.ipAddr );
    	icmpWorkParam->enable         = icmpConf.enable;        
    	icmpWorkParam->interval	    = icmpConf.interval;
    	icmpWorkParam->timeOut	    = icmpConf.timeOut;
    	icmpWorkParam->fcount	    = icmpConf.fcount;
    	icmpWorkParam->finterval	= icmpConf.finterval;
    }
}

static void ResetIcmpWorkMaintain( ICMP_WORK_MAINTAIN_T *icmpWorkMaintain )
{
	icmpWorkMaintain->curState     = ICMP_CUR_STATE_UNKNOW;
	icmpWorkMaintain->fcount	= 0;
	icmpWorkMaintain->pts	    = -100;
}

/*
* fn: 根据条件判断是否需要执行ping
* 返回: 0, 需要执行ping, 否则不需要执行
*/
static int ParseIcmpWorkParamAndMaintain( ICMP_WORK_MAINTAIN_T *icmpWorkMaintain,
                                            	ICMP_WORK_PARAM_T icmpWorkParam )
{
	int ret = -1;
	int curPts = SysRunTimeGet();
	int interval = curPts - icmpWorkMaintain->pts;
    
	if( ICMP_CUR_STATE_UNKNOW == icmpWorkMaintain->curState
        || ICMP_CUR_STATE_PING_SUCCESS == icmpWorkMaintain->curState )
    {        
    	if( interval >= icmpWorkParam.interval )
        {
        	icmpWorkMaintain->pts = curPts;
        	ret = 0;
        }
    }
	else if( ICMP_CUR_STATE_PING_FAILED == icmpWorkMaintain->curState )
    {        
    	ret = 0;
    }
	else if( ICMP_CUR_STATE_PING_N_FAILED == icmpWorkMaintain->curState ) 
    {
    	if( interval >= icmpWorkParam.finterval )
        {
        	icmpWorkMaintain->pts = curPts;
        	ret = 0;
        }
    }

	return ret;
}

/*
* fn: 分析ping 的结果
* icmpWorkMaintain: out, 根据ping 的结果对 维系变量做修改
*/
static void ParsePingResult( int ret, ICMP_WORK_PARAM_T icmpWorkParam, 
                            	ICMP_WORK_MAINTAIN_T *icmpWorkMaintain )
{
	if( 0 == ret )
    {
    	icmpWorkMaintain->curState     = ICMP_CUR_STATE_PING_SUCCESS;
    	icmpWorkMaintain->fcount	= 0;
    }
	else
    {
    	icmpWorkMaintain->fcount++;
    	if( ICMP_CUR_STATE_PING_FAILED == icmpWorkMaintain->curState )
        {       
        	if( icmpWorkMaintain->fcount >= icmpWorkParam.fcount )
            {
            	icmpWorkMaintain->curState     = ICMP_CUR_STATE_PING_N_FAILED;
            	icmpWorkMaintain->fcount     = 0;
            }
        	else
            {
            	icmpWorkMaintain->curState     = ICMP_CUR_STATE_PING_FAILED;
            }
        }
    	else if( ICMP_CUR_STATE_PING_N_FAILED == icmpWorkMaintain->curState )
        {
        	icmpWorkMaintain->curState = ICMP_CUR_STATE_PING_FAILED;
        }
    	else
        {
        	icmpWorkMaintain->curState = ICMP_CUR_STATE_PING_FAILED;
        }
    }

	if( ICMP_CUR_STATE_PING_SUCCESS == icmpWorkMaintain->curState )
    {
    	SetNetworkStatus( 1 );
    }
	else
    {
    	SetNetworkStatus( 0 );
    }
    
}

static void *IcmpAppThread( void *arg )
{
	int ret;
    
	ICMP_WORK_PARAM_T icmpWorkParam;
	ICMP_WORK_MAINTAIN_T icmpWorkMaintain;

	ResetIcmpWorkMaintain( &icmpWorkMaintain );
	GetIcmpWork( &icmpWorkParam );
	SVPrint( "%s start!\r\n", __FUNCTION__ );
	while( g_icmpAppTm.runFlag )
    {
    	if( 0 == icmpWorkParam.enable )
        {
        	g_icmpWorkCondition.Wait();
        	GetIcmpWork( &icmpWorkParam );
        	ResetIcmpWorkMaintain( &icmpWorkMaintain );
        	continue;
        }
    	else
        {
        	if( MessageRecv(MSG_ID_ICMP_APP) >= 0 )
            {
            	GetIcmpWork( &icmpWorkParam );
            	ResetIcmpWorkMaintain( &icmpWorkMaintain );                
            	continue;
            }
        	ret = ParseIcmpWorkParamAndMaintain( &icmpWorkMaintain, icmpWorkParam );
        	if( ret == 0 )
            {
            	ret = MyPing( icmpWorkParam.ipAddr, icmpWorkParam.timeOut * 1000 );
            	ParsePingResult( ret, icmpWorkParam, &icmpWorkMaintain );
            }

        	sleep( 1 );
        }
    }
    
	SVPrint( "%s stop!\r\n", __FUNCTION__ );
	return NULL;
}

void StartIcmpAppThread()
{
	int ret;
	g_icmpAppTm.runFlag = 1;
	ret = ThreadCreate( &g_icmpAppTm.id, IcmpAppThread, NULL );
	if( 0!= ret )
    {        
    	g_icmpAppTm.runFlag = 0;
    	SVPrint( "error:ThreadCreate:%s\r\n", STRERROR_ERRNO );
    }
}

void StopIcmpAppThread()
{
	int ret;
	g_icmpAppTm.runFlag = 0;
	g_icmpWorkCondition.Signal();
	ret = ThreadJoin( g_icmpAppTm.id, NULL );
	if( 0 != ret )
    {
    	SVPrint( "error:ThreadJoin:%s\r\n", STRERROR_ERRNO );
    }
}

/*
* fn: 当icmp 参数被修改后,通过此函数通知
*/
void IcmpSendParamChangeMessage()
{
	g_icmpWorkCondition.Signal();
	MessageSend( MSG_ID_ICMP_APP );
}

