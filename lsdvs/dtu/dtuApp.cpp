/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.10.10
**  description  : 应用dtu 实现各种应用
*******************************************************************************
*/
#include <unistd.h>
#include <stdlib.h>
#include "debug.h"
#include "thread.h"
#include "message.h"
#include "condition.h"
#include "dtu.h"
#include "paramManage.h"
#include "dtuApp.h"
#include "sysRunTime.h"
#include "linuxFile.h"
#include "netSocket.h"
#include "public.h"

static ClCondition g_dtuWorkCondition;  

static void GetDtuWork( DTU_WORK_PARAM_T *dtuWorkParam )
{
	int ret;
	PARAM_CONFIG_DTU dtuConf;

	dtuWorkParam->enable = 0;
	ret = ParamGetDtu( &dtuConf );
	if( 0 == ret )
    {
    	strcpy( dtuWorkParam->severIp, dtuConf.severIp );        
    	strcpy( dtuWorkParam->heartbeatContent, dtuConf.heartbeatContent );
    	dtuWorkParam->enable         = dtuConf.enable;        
    	dtuWorkParam->interval	    = dtuConf.interval;
    	dtuWorkParam->transProtocol	= dtuConf.transProtocol;
    	dtuWorkParam->serverPort	= dtuConf.serverPort;
    }
}

static void ResetDtuWorkMaintain( DTU_WORK_MAINTAIN_T *dtuWorkMaintain )
{
	dtuWorkMaintain->pts         = -1000;
	dtuWorkMaintain->tcpMode     = 0;         // 0,长连接; 1,短链接
	if( dtuWorkMaintain->socket > 0 )
    {
    	Close( dtuWorkMaintain->socket );
    	dtuWorkMaintain->socket = -1;
    }
}

#if 0
/*
* fn: 刷新维护参数
*/
static void ReFreshDtuWorkMaintain( DTU_WORK_MAINTAIN_T *dtuWorkMaintain )
{
	dtuWorkMaintain->pts = -1000;
}
#endif

/*
* fn: 根据条件判断是否需要执行ping
* 返回: 0, 需要执行ping, 否则不需要执行
*/
static int ParseDtuWorkParamAndMaintain( DTU_WORK_MAINTAIN_T *dtuWorkMaintain,
                                            	DTU_WORK_PARAM_T dtuWorkParam )
{
	int ret         = -1;
	int curPts         = SysRunTimeGet();
	int interval     = curPts - dtuWorkMaintain->pts;    
    
	if( interval >= dtuWorkParam.interval )
    {        
    	dtuWorkMaintain->pts = curPts;
    	ret = 0;
    }    

	return ret;
}

/*
* fn: 发送dtu 心跳包,udp协议方式
*/
static int SendDtuHeartbeatOrHyalineUdp( DTU_WORK_PARAM_T dtuWorkParam, 
                            	DTU_WORK_MAINTAIN_T *dtuWorkMaintain )
{
	int ret;
	uint len;

	len = strlen( dtuWorkParam.heartbeatContent );
	len = len <= sizeof(dtuWorkParam.heartbeatContent)? len : sizeof(dtuWorkParam.heartbeatContent);
	if( dtuWorkMaintain->socket < 0 )
    {
    	ret = SocketUdpClient( &dtuWorkMaintain->socket );
    }
	else
    {
    	ret = 0;
    }
	if( 0 == ret )
    {
    	if( dtuWorkMaintain->hyalineLen > 0 )
        {
        	ret = UdpSendto( dtuWorkMaintain->socket, dtuWorkParam.severIp, dtuWorkParam.serverPort,
                        (unsigned char *)dtuWorkMaintain->hyalineBuf, dtuWorkMaintain->hyalineLen );
        	dtuWorkMaintain->hyalineLen = 0;
        }
    	else
        {
    	ret = UdpSendto( dtuWorkMaintain->socket, dtuWorkParam.severIp, dtuWorkParam.serverPort,
                                            (unsigned char *)dtuWorkParam.heartbeatContent, len );
        }
    	if( 0 != ret )
        {
        	Close( dtuWorkMaintain->socket );
        	dtuWorkMaintain->socket = -1;
        }
    }

	return ret;
}

/*
* fn: 发送dtu 心跳包,tcp协议短链接方式
*/
static int SendDtuHeartbeatTcpOrHyalineShort( DTU_WORK_PARAM_T dtuWorkParam, 
                            	DTU_WORK_MAINTAIN_T *dtuWorkMaintain )
{
	int ret;
	uint len;

	len = strlen( dtuWorkParam.heartbeatContent );
	len = len <= sizeof(dtuWorkParam.heartbeatContent)? len : sizeof(dtuWorkParam.heartbeatContent);
	if( dtuWorkMaintain->socket < 0 )
    {
    	ret = SocketTcpConnectTimtout( &dtuWorkMaintain->socket, dtuWorkParam.severIp,
                                                	dtuWorkParam.serverPort, 8000 );
    }
	else
    {
    	ret = 0;
    }
	if( 0 == ret )
    {
    	if( dtuWorkMaintain->hyalineLen > 0 )
        {
        	ret = Writen( dtuWorkMaintain->socket, dtuWorkMaintain->hyalineBuf, dtuWorkMaintain->hyalineLen );
        	ret = (unsigned int)ret == dtuWorkMaintain->hyalineLen ? 0 : -1;
        	dtuWorkMaintain->hyalineLen = 0;
        }
    	else
        {
        	ret = Writen( dtuWorkMaintain->socket, dtuWorkParam.heartbeatContent, len );
        	ret = (unsigned int)ret == len ? 0 : -1;
        }
    	Close( dtuWorkMaintain->socket );
    	dtuWorkMaintain->socket = -1;        
    }

	return ret;
}

/*
* fn: 发送dtu 心跳包,tcp协议长链接方式
*/
static int SendDtuHeartbeatOrHyalineTcpLong( DTU_WORK_PARAM_T dtuWorkParam, 
                            	DTU_WORK_MAINTAIN_T *dtuWorkMaintain )
{
	int ret;
	uint len;

	len = strlen( dtuWorkParam.heartbeatContent );
	len = len <= sizeof(dtuWorkParam.heartbeatContent)? len : sizeof(dtuWorkParam.heartbeatContent);

	if( dtuWorkMaintain->socket < 0 )
    {
    	ret = SocketTcpConnectTimtout( &dtuWorkMaintain->socket, dtuWorkParam.severIp,
                                                	dtuWorkParam.serverPort, 8000 );
    }
	else
    {
    	ret = 0;
    }
	if( 0 == ret )
    {
    	if( dtuWorkMaintain->hyalineLen > 0 )
        {
        	ret = Writen( dtuWorkMaintain->socket, dtuWorkMaintain->hyalineBuf, dtuWorkMaintain->hyalineLen );
        	ret = (unsigned int)ret == dtuWorkMaintain->hyalineLen ? 0 : -1;
        	dtuWorkMaintain->hyalineLen = 0;
        }
    	else
        {
        	ret = Writen( dtuWorkMaintain->socket, dtuWorkParam.heartbeatContent, len );
        	ret = (unsigned int)ret == len ? 0 : -1;
        }
    	if( 0 != ret )
        {
        	Close( dtuWorkMaintain->socket );
        	dtuWorkMaintain->socket = -1;    
        }
    }

	return ret;
}


/*
* fn: 发送dtu 心跳包
*/
static int SendDtuHeartbeatOrHyaline( DTU_WORK_PARAM_T dtuWorkParam, 
                            	DTU_WORK_MAINTAIN_T *dtuWorkMaintain )
{
	int ret;
	if( 1 == dtuWorkParam.transProtocol ) // udp
    {
    	ret = SendDtuHeartbeatOrHyalineUdp( dtuWorkParam, dtuWorkMaintain );
    }
	else // tcp
    {
    	if( 1 == dtuWorkMaintain->tcpMode ) // 短连接
        {
        	ret = SendDtuHeartbeatTcpOrHyalineShort( dtuWorkParam, dtuWorkMaintain );
        }
    	else  // 长连接
        {
        	ret = SendDtuHeartbeatOrHyalineTcpLong( dtuWorkParam, dtuWorkMaintain );
        }
    }

	return ret;
}

static THREAD_MAINTAIN_T g_dtuAppTm;
static void *DtuAppThread( void *arg )
{
	int ret;
    
	DTU_WORK_PARAM_T dtuWorkParam;
	DTU_WORK_MAINTAIN_T dtuWorkMaintain;
    
	dtuWorkMaintain.socket = -1;
	ResetDtuWorkMaintain( &dtuWorkMaintain );
	GetDtuWork( &dtuWorkParam );
	SVPrint( "%s start!\r\n", __FUNCTION__ );
	while( g_dtuAppTm.runFlag )
    {
    	if( 0 == dtuWorkParam.enable )
        {
        	g_dtuWorkCondition.Wait();
        	GetDtuWork( &dtuWorkParam );
        	ResetDtuWorkMaintain( &dtuWorkMaintain );
        	continue;
        }
    	else
        {
        	if( MessageRecv(MSG_ID_DTU_APP) >= 0 )
            {
            	GetDtuWork( &dtuWorkParam );
            	ResetDtuWorkMaintain( &dtuWorkMaintain );                
            	continue;
            }
        	if( (ret = MessageRecv(MSG_ID_DTU_HYALINE, dtuWorkMaintain.hyalineBuf, sizeof(dtuWorkMaintain.hyalineBuf))) > 0 )
            {
            	dtuWorkMaintain.hyalineLen     = ret;
            	dtuWorkMaintain.pts	        = -1000;
            }
        	ret = ParseDtuWorkParamAndMaintain( &dtuWorkMaintain, dtuWorkParam ); // 检查是否满足发送心跳条件
        	if( ret == 0 )
            {
            	ret = SendDtuHeartbeatOrHyaline( dtuWorkParam, &dtuWorkMaintain );
            }

        	usleep( 500000 );
        }
    }
    
	SVPrint( "%s stop!\r\n", __FUNCTION__ );
	return NULL;
}

void StartDtuAppThread()
{
	int ret;
	g_dtuAppTm.runFlag = 1;
	ret = ThreadCreate( &g_dtuAppTm.id, DtuAppThread, NULL );
	if( 0!= ret )
    {        
    	g_dtuAppTm.runFlag = 0;
    	SVPrint( "error:ThreadCreate:%s\r\n", STRERROR_ERRNO );
    }
}

void StopDtuAppThread()
{
	int ret;
	g_dtuAppTm.runFlag = 0;
	g_dtuWorkCondition.Signal();
	ret = ThreadJoin( g_dtuAppTm.id, NULL );
	if( 0 != ret )
    {
    	SVPrint( "error:ThreadJoin:%s\r\n", STRERROR_ERRNO );
    }
}

/*
* fn: 当dtu 参数被修改后,通过此函数通知
*/
void DtuSendParamChangeMessage()
{
	g_dtuWorkCondition.Signal();
	MessageSend( MSG_ID_DTU_APP );
}

