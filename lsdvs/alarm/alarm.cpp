/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 报警
********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "debug.h"
#include "const.h"
#include "thread.h"
#include "threadPool.h"
#include "linuxFile.h"
#include "timeExchange.h"
#include "message.h"
#include "timer.h"
#include "const.h"
#include "malloc.h"
#include "paramConfig.h"
#include "alarmLinkage.h"
#include "alarmList.h"
#include "alarm.h"
#include "alarmComm.h"
#include "alarmIo.h"
#include "alarmMd.h"
#include "alarmVloss.h"
#include "alarmShelter.h"
#include "log.h"
#include "dcpInsLocal.h"
#include "ttypes.h"
#include "dcpTypes.h"
#include "dcpTypes.h"
#include "sysRunTime.h"

static ClTimer s_AlarmRTimer;
static ClTimer s_AlarmVlossRTimer;
static ClTimer s_AlarmShelterRTimer;
ClThreadPool 	g_alarmThreadPool;
static const int ALARM_THREAD_POOL_NUM	= 10;

//记录上次报警记录， 防止某个瘫痪的报警被每次发送， 
//例如IO 1一直存在有报警，已经记录过一次，来了新的报警 IO 2   那么IO 1 不会再被记录一次
//除非IO 1有恢复过，再次故障则会上报
static uint ioFlagOld = 0;
static uint vlossFlagOld = 0;
static uint shelterFlagOld = 0;


static void * DealAlarmMsgProcess( void *args )
{
	ALARM_LINKAGE_NODE *alarmLinkage = (ALARM_LINKAGE_NODE *)args;
    
	if ( alarmLinkage != NULL )
    {
    	if ( alarmLinkage->fun != NULL )
        {
            ( *(alarmLinkage->fun) )( alarmLinkage->args );
        }
    }
	return NULL;
}

//
// 告警处理线程
// 根据告警消息的类型回调指定的告警处理函数
//
static void *AlarmWaitLinkageThread( void *args )
{
	ALARM_LINKAGE_NODE	alarmLinkage	= { 0 };

	CORRECTPRINT("##########start %s!\r\n", __FUNCTION__ );
	while ( 1 )
    {
    	if ( MessageRecv( MSG_ID_ALARM_WAIT_LINKAGE_THREAD ) >= 0 )
        {
        	break;
        }
        
    	if ( PopAlarmLinkageList( &alarmLinkage ) != -1 )
        {
        	g_alarmThreadPool.Add( DealAlarmMsgProcess, &alarmLinkage, sizeof(alarmLinkage) );
        }        
    	usleep( 10*1000 );
    }

	SVPrint("stop %s!\r\n", __FUNCTION__ );
	return NULL;
}

static void SendAlarmParmaMsgWhenStart()
{
	int i;

	AlarmSendMsgIoAlarmParamChange();
	for( i = 0; i < REAL_CHANNEL_NUM; i++ )
    {
    	AlarmSendMsgMdAlarmParamChange( i );
    	AlarmSendMsgVlossAlarmParamChange( i );
    	AlarmSendMsgShelterAlarmParamChange( i );
    }
}

//
// 告警主线程
// 用于检测告警配置的改变，并由此修改告警定时器线程。
//
static int g_AlarmServiceFlag;
static void *AlarmServiceThread( void *args )
{
	int nRet = -1;
	int channel, i;
	PARAM_CONFIG_ALARM_IO	        	ioAlarmParam;
	PARAM_CONFIG_ALARM_VIDEO_LOSE		vlossAlarmParam;
	PARAM_CONFIG_ALARM_VIDEO_SHELTER	videoShelterAlarmParam;    
	PARAM_CONFIG_ALARM_MOVE_DETECT	mdAlarmParam;        
	ALARM_WORK_T         	alarmWorkVloss[REAL_CHANNEL_NUM]; 
	ALARM_WORK_T         	alarmWorkShelter[REAL_CHANNEL_NUM]; 
	ALARM_WORK_T         	alarmWorkIo;
	ALARM_WORK_T         	alarmWork[REAL_CHANNEL_NUM];
	unsigned int         	ioAlarmTimerId     = 0, mdAlarmTimerId[REAL_CHANNEL_NUM] = { 0 };    
	unsigned int vlossAlarmTierId[REAL_CHANNEL_NUM] = { 0 }; 
	unsigned int shelterAlarmTierId[REAL_CHANNEL_NUM] = { 0 };
	pthread_t AlarmWaitLinkageThreadId;

	SVPrint("start %s!\r\n", __FUNCTION__ );

	InitAlarmLinkageList();

	nRet = s_AlarmRTimer.Create( TIMER_PRECISION_MS );
	if ( nRet != 0 )
    {
    	SVPrint( "error:s_AlarmRTimer.Create()!\r\n" );
    	return NULL;
    }    
	nRet = g_alarmThreadPool.Create( ALARM_THREAD_POOL_NUM );
	if ( nRet != 0 )
    {
    	s_AlarmRTimer.Destroy();
    	SVPrint( "error:g_alarmThreadPool.Create()!\r\n" );
    	return NULL;
    }    

	nRet = s_AlarmVlossRTimer.Create( TIMER_PRECISION_MS );
	if ( nRet != 0 )
    {
    	SVPrint( "error:s_AlarmVlossRTimer.Create()!\r\n" );
    	return NULL;
    }    
    
	nRet = ThreadCreateCommonPriority( &AlarmWaitLinkageThreadId, AlarmWaitLinkageThread, NULL );
	if ( nRet != 0 )
    {
    	g_alarmThreadPool.Destroy();
    	s_AlarmRTimer.Destroy();
    	SVPrint( "error:ThreadCreateCommonPriority()!\r\n" );
    	return NULL;
    }    
	SendAlarmParmaMsgWhenStart();
    
	while( FI_TRUE == g_AlarmServiceFlag )
    {
    	if ( MessageRecv(MSG_ID_IO_ALARM_PARAM_CHANGE) >= 0 )
        {
        	s_AlarmRTimer.Delete( ioAlarmTimerId );    
        	nRet = ParamGetAlarmIo( &ioAlarmParam );
        	if( 0 == nRet )
            {
            	GetAlarmIoSt( ioAlarmParam, &alarmWorkIo );            
            	FiPrint2("handGuard(%d) timeNum(%d)!\r\n", alarmWorkIo.guard.guardHand.guardFlag, 
                                                	alarmWorkIo.guard.guardTimer.timeNum);
            	if ( FI_TRUE == alarmWorkIo.guard.guardHand.guardFlag
                    || alarmWorkIo.guard.guardTimer.timeNum > 0 )
                {                    
                	nRet = s_AlarmRTimer.Add( IoAlarmDetectTimer, &alarmWorkIo, 
                                    	alarmWorkIo.interval, &ioAlarmTimerId );
                	if ( nRet == -1 ) SVPrint( "Add detect alarm message timer failed !\r\n" );
                }
            }            
        }
    	if( MessageRecv(Msg_ID_MD_ALARM_PARAM_CHANGE, (char *)&channel, sizeof(channel) ) >= 0 )
        {
        	s_AlarmRTimer.Delete( mdAlarmTimerId[channel] );        
        	nRet = ParamGetAlarmMoveDetect( channel, &mdAlarmParam );
        	if( 0 == nRet )
            {
            	GetAlarmMdSt( channel, mdAlarmParam, &alarmWork[channel] );            
            	FiPrint2( "handGuard(%d) timeNum(%d)!\r\n", alarmWork[channel].guard.guardHand.guardFlag, 
                                                	alarmWork[channel].guard.guardTimer.timeNum );
                
            }
        }
  
    	for( i = 0; i < REAL_CHANNEL_NUM; i++ )
        {
        	DetectMdAlarm( i, &alarmWork[i] );
        }

#if 1	    
    	if ( MessageRecv(MSG_ID_VLOSS_ALARM_PARAM_CHANGE, (char *)&channel, sizeof(channel) ) >= 0 )
        {
        	s_AlarmVlossRTimer.Delete( vlossAlarmTierId[channel] );    
        	nRet = ParamGetAlarmVideoLose( channel, &vlossAlarmParam );
        	if( 0 == nRet )
            {
            	GetAlarmVlossSt( vlossAlarmParam, &alarmWorkVloss[channel] );    
            	FiPrint2("handGuard(%d) timeNum(%d)!\r\n", alarmWorkVloss[channel].guard.guardHand.guardFlag, 
                                                	alarmWorkVloss[channel].guard.guardTimer.timeNum);
            }
        }
    	for( i = 0; i < REAL_CHANNEL_NUM; i++ )
        {
        	DetectVlossAlarm( i, &alarmWorkVloss[i] );
        }
        
    	if ( MessageRecv(MSG_ID_SHELTER_ALARM_PARAM_CHANGE, (char *)&channel, sizeof(channel) ) >= 0 )
        {
        	s_AlarmShelterRTimer.Delete( shelterAlarmTierId[channel] );    
        	nRet = ParamGetAlarmVideoShelter( channel, &videoShelterAlarmParam );
        	if( 0 == nRet )
            {
            	GetAlarmShelterSt( videoShelterAlarmParam, &alarmWorkShelter[channel] );    
            	FiPrint2("handGuard(%d) timeNum(%d)!\r\n", alarmWorkShelter[channel].guard.guardHand.guardFlag, 
                                                	alarmWorkShelter[channel].guard.guardTimer.timeNum);
            }
        }
    	for( i = 0; i < REAL_CHANNEL_NUM; i++ )
        {
        	DetectVideoShelterAlarm( i, &alarmWorkShelter[i] );
        }
        
#endif	    
    	usleep( 500*1000 );
    }
	ThreadJoin( AlarmWaitLinkageThreadId, NULL );
	g_alarmThreadPool.Destroy();
	s_AlarmRTimer.Destroy();    

	SVPrint("stop %s!\r\n", __FUNCTION__ );
    
	return NULL;
}

static pthread_t g_AlarmServiceId;
int AlarmStartService()
{
	int ret;
	AlarmMdfdOpen();
    
	g_AlarmServiceFlag = FI_TRUE;
	ret = ThreadCreateCommonPriority( &g_AlarmServiceId, AlarmServiceThread, NULL );
	if( ret != FI_SUCCESS )
    {
    	g_AlarmServiceFlag = FI_FALSE;
    	SVPrint("failed: create AlarmServiceThread!\r\n");
    }

	return ret;
}

int AlarmStopService()
{
	MessageSend( MSG_ID_ALARM_WAIT_LINKAGE_THREAD );
	SignalAlarmLinkageList();    
    
	g_AlarmServiceFlag = FI_FALSE;
	ThreadJoin(g_AlarmServiceId, NULL);
    
	return 0;
}

void AlarmSendMsgIoAlarmParamChange()
{
	MessageSend( MSG_ID_IO_ALARM_PARAM_CHANGE );
}

void AlarmSendMsgMdAlarmParamChange( int channel )
{
	int ch = channel;
	MessageSend( Msg_ID_MD_ALARM_PARAM_CHANGE, (char *)&ch, sizeof(ch)  );
}

void AlarmSendMsgVlossAlarmParamChange( int channel )
{
	int ch = channel;
	MessageSend( MSG_ID_VLOSS_ALARM_PARAM_CHANGE, (char *)&ch, sizeof(ch) );
}

void AlarmSendMsgShelterAlarmParamChange( int channel )
{
	int ch = channel;
	MessageSend( MSG_ID_SHELTER_ALARM_PARAM_CHANGE, (char *)&ch, sizeof(ch) );
}

int AlarmThreadpoolAdd( THREAD_FUN fun, void *args, int argsSize ) 
{
	return g_alarmThreadPool.Add( fun, args, argsSize );    
}

/*
* fun:  报警转发处理函数 包括 记录日志  传到客户端
* type : 报警类型
* typeSubFlag:  每一位标识一个报警状态   H:有报警  L:无报警
*/
int AlarmTransmit( uint type, uint typeSubFlag )
{
	MSG_CMD_T msgCmd;
	DCP_ALARM_RES_T alarm;
	int i;
	int ret;

    // shelter 两次写log 需隔一段时间,
    static int lastLogShelterTime = -1000;
    int curTime;
    
	for ( i=0; i<32; i++ )
    {
    	if ( typeSubFlag&(1<<i) )
        {
            //日志记录
        	if ( RES_ALARM_TYPE_IO==type )
            {
                //判断是否曾经发送过
            	if ( ioFlagOld&(1<<i) ) continue;
            	ioFlagOld |= (1<<i);
                
            	ret = LogAdd( 0xff, LOG_TYPE_MODULEx, LOG_LEVEL_INFO, "IO:%d Alarm", i );
            	if ( FI_SUCCESS == ret ) 
                {
                	SVPrint( "sys log IO index: %d  success.\n", i );
                }
            	else 
                {
                	SVPrint( "sys log IO index: %d  fail.\n", i );
                }
            }
        	else if ( RES_ALARM_TYPE_VLOSS==type )
            {
                //判断是否曾经发送过
            	if ( vlossFlagOld&(1<<i) ) continue;
            	vlossFlagOld |= (1<<i);                
                
            	ret = LogAdd( i, LOG_TYPE_MODULEx, LOG_LEVEL_INFO, "Vloss:%d Alarm", i );
            	if ( FI_SUCCESS == ret ) 
                {
                	SVPrint( "sys log Vedio loss index: %d  success.\n", i );
                }
            	else 
                {
                	SVPrint( "sys log Vedio loss index: %d  fail.\n", i );
                }
            }
        	else if ( RES_ALARM_TYPE_SHELTER==type )
            {
                //判断是否曾经发送过
            	if ( shelterFlagOld&(1<<i) ) continue;
            	shelterFlagOld |= (1<<i);                
                
                curTime = SysRunTimeGet();
                if(curTime - lastLogShelterTime > 300)
                {
                    lastLogShelterTime = curTime;
                    ret = LogAdd( i, LOG_TYPE_MODULEx, LOG_LEVEL_INFO, "Shelter:%d Alarm", i );
                    if ( FI_SUCCESS == ret ) 
                    {
                        SVPrint( "sys log Vedio shelter index: %d  success.\n", i );
                    }
                    else 
                    {
                        SVPrint( "sys log Vedio shelter index: %d  fail.\n", i );
                    }
                }
            }
        	else 
            {
            	return -1;
            }

            //发送到客户端
        	msgCmd.cmd = DIL_ALARM_UPLOAD;
        	alarm.alarmType = type;
        	alarm.alarmTypeSub = i;
        	msgCmd.pData = (char *)Malloc( sizeof(DCP_ALARM_RES_T) );
        	if ( NULL != msgCmd.pData )
            {
            	Memcpy( msgCmd.pData, ( char* )&alarm, sizeof(DCP_ALARM_RES_T) );
            	MessageSend( MSG_ID_DCP_SIGNAL, (char *)&msgCmd, sizeof(msgCmd) );
            }            
        }

    }

	return 0;
}



/*
* fun:  取消报警转发处理函数 包括 记录日志  传到客户端
* type : 报警类型
* typeSub:  每种类型报警的某一路
*/
int AlarmTransmitCancel( uint type, uint typeSub )
{
	MSG_CMD_T msgCmd;
	DCP_ALARM_RES_T alarm;
	int ret;

    // shelter 两次写log 需隔一段时间,
    static int lastLogShelterTime = 0;
    int curTime;

    //日志记录
	if ( RES_ALARM_TYPE_IO_CANCEL==type )
    {            
    	ret = LogAdd( 0xff, LOG_TYPE_MODULEx, LOG_LEVEL_INFO, "IO:%d Alarm cancel", typeSub );
    	if ( FI_SUCCESS == ret ) 
        {
        	SVPrint( "sys log IO cancel index: %d  success.\n", typeSub );
        }
    	else 
        {
        	SVPrint( "sys log IO cancel index: %d  fail.\n", typeSub );
        }
    }
	else if ( RES_ALARM_TYPE_VLOSS_CANCEL==type )
    {            
    	ret = LogAdd( typeSub, LOG_TYPE_MODULEx, LOG_LEVEL_INFO, "Vloss:%d Alarm cancel", typeSub );
    	if ( FI_SUCCESS == ret ) 
        {
        	SVPrint( "sys log Vedio loss cancel index: %d  success.\n", typeSub );
        }
    	else 
        {
        	SVPrint( "sys log Vedio loss cancel index: %d  fail.\n", typeSub );
        }
    }
	else if ( RES_ALARM_TYPE_SHELTER_CANCEL==type )
    {
        curTime = SysRunTimeGet();
        if(curTime - lastLogShelterTime > 600)
        {
            lastLogShelterTime = curTime;
            ret = LogAdd( typeSub, LOG_TYPE_MODULEx, LOG_LEVEL_INFO, "Shelter:%d Alarm cancel", typeSub );
            if ( FI_SUCCESS == ret ) 
            {
                SVPrint( "sys log Vedio shelter cancel index: %d  success.\n", typeSub );
            }
            else 
            {
                SVPrint( "sys log Vedio shelter cancel index: %d  fail.\n", typeSub );
            }
        }
    }
	else 
    {
    	return -1;
    }

    //发送到客户端
	msgCmd.cmd = DIL_ALARM_UPLOAD;
	alarm.alarmType = type;
	alarm.alarmTypeSub = typeSub;
	msgCmd.pData = (char *)Malloc( sizeof(DCP_ALARM_RES_T) );
	if ( NULL != msgCmd.pData )
    {
    	Memcpy( msgCmd.pData, ( char* )&alarm, sizeof(DCP_ALARM_RES_T) );
    	MessageSend( MSG_ID_DCP_SIGNAL, (char *)&msgCmd, sizeof(msgCmd) );
    }            
	return 0;
}

/*
* fun:  清楚报警标志，并且将报警由有到无的变化 上传到客户端
* type : 报警类型
* typeSub:  每一位标识一个报警状态   H:有报警  L:无报警
*/
void AlarmUpdateFlag( uint type, uint typeSubFlag )
{
	int i;
	for ( i=0;i<32;i++ ) 
    {
    	if ( RES_ALARM_TYPE_IO==type )
        {
        	if ( (ioFlagOld&(1<<i)) && (!(typeSubFlag&(1<<i))) )
            {
            	ioFlagOld &= ~(1<<i);
            	AlarmTransmitCancel( RES_ALARM_TYPE_IO_CANCEL, i );
            }
        }
    	else if ( RES_ALARM_TYPE_VLOSS==type )
        {
        	if ( (vlossFlagOld&(1<<i)) && (!(typeSubFlag&(1<<i))) )
            {
            	vlossFlagOld &= ~(1<<i);
            	AlarmTransmitCancel( RES_ALARM_TYPE_VLOSS_CANCEL, i );
            }
        }
    	else if ( RES_ALARM_TYPE_SHELTER==type )
        {
        	if ( (shelterFlagOld&(1<<i)) && (!(typeSubFlag&(1<<i))) )
            {
            	shelterFlagOld &= ~(1<<i);
            	AlarmTransmitCancel( RES_ALARM_TYPE_SHELTER_CANCEL, i );
            }
        }    
    }
}



