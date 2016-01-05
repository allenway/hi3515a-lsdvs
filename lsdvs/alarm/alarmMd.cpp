/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 移动侦测报警
********************************************************************************/

#include "const.h"
#include "debug.h"
#include "timeExchange.h"
#include "alarmMd.h"
#include "alarmLinkage.h"
#include "alarmList.h"
#include "alarmComm.h"
#include "malloc.h"
#include "public.h"
#include "md.h"
#include "proconMd.h"
#include "alarm.h"

static uint g_fdmd[REAL_CHANNEL_NUM];

static uint GetFdmd( int channel )
{
	return g_fdmd[channel];
}

void AlarmMdfdOpen()
{
	int i;
	for( i = 0; i < REAL_CHANNEL_NUM; i++ )
    {
    	g_fdmd[i] = ProconMdOpen( i, OPEN_RDONLY );
    }
}
//
// 检测告警输入, 判断是否触发告警.
//
int DetectMdAlarm( int channel, ALARM_WORK_T *pAlarmWork )
{
	int ret = -1;
	uint fdmd;
	PROCON_NODE_T *pProconMd;

	fdmd = GetFdmd( channel );
	pProconMd = ProconMdRead( fdmd );
	if( NULL != pProconMd )
    {
    	ret = AlarmThreadpoolAdd( MdAlarmDetectTimer, (void *)pAlarmWork, sizeof(ALARM_WORK_T) );
    	ProconMdFree( pProconMd );
    }

	return ret;
}

static int CheckIsMdAlarmOutPut( CONFIG_ALARM_LINKAGE_T	linkage, ALARM_LINKAGE_OUTPUT *pOutput )
{
	int i, ret = FI_FALSE;
	int	maxAlarmOutputNum = MAX_ALARM_OUTPUT_NUM;
    
	for( i = 0; i < maxAlarmOutputNum; i++ )
    {
    	if( linkage.linkageAlarmOut & (0x01 << i) )
        {
        	if( NULL != pOutput )
            {
            	pOutput->linkageAlarmOut     = linkage.linkageAlarmOut;
            	pOutput->alarmOutNum	    = maxAlarmOutputNum;
            	pOutput->alarmOutDuration	= linkage.alarmOutDuration;
            }

        	ret = FI_TRUE;
        	break;
        }
    }    

	return ret;
}

static int CheckIsMdAlarmCapture( CONFIG_ALARM_LINKAGE_T linkage, ALARM_LINKAGE_CAPTURE *pCapture )
{
	int i, ret = FI_FALSE;
	int	channelNum = REAL_CHANNEL_NUM;
    
	for( i = 0; i < channelNum; i++ )
    {
    	if( linkage.linkageCapture & (0x01 << i) )
        {
        	if( NULL != pCapture )
            {
            	pCapture->linkageCapture	= linkage.linkageCapture;
            	pCapture->captureType	    = SNAP_TYPE_ALARM_MD; 
            	pCapture->captureNum	    = linkage.captureNum;
            }

        	ret = FI_TRUE;
        	break;
        }
    }    

	return ret;
}

static int CheckIsMdAlarmRecord( CONFIG_ALARM_LINKAGE_T linkage, ALARM_LINKAGE_RECORD *pRecord )
{
	int i, ret = FI_FALSE;
	int	channelNum = REAL_CHANNEL_NUM;
    
	for( i = 0; i < channelNum; i++ )
    {
    	if( linkage.linkageRecord & (0x01 << i) )
        {
        	if( NULL != pRecord )
            {
            	pRecord->linkageRecord	= linkage.linkageRecord;
            	pRecord->recordType	    = RECORD_TYPE_ALARM_MD;
            	pRecord->recordDuration	= linkage.recordDuration;
            }

        	ret = FI_TRUE;
        	break;
        }
    }    

	return ret;
}

void *MdAlarmDetectTimer( void *args )
{    
	ALARM_WORK_T         *pAlarmMd	    = (ALARM_WORK_T *)args;
	ALARM_LINKAGE_NODE	alarmNode	    = { 0 };
    
	if( CheckIsGuard( pAlarmMd->guard ) )
    {    // 触发告警则将告警处理函数插入告警列表。	    
    	ALARM_LINKAGE_OUTPUT *pOutput = (ALARM_LINKAGE_OUTPUT *)Calloc( 1, sizeof(ALARM_LINKAGE_OUTPUT) );
    	if( FI_TRUE == CheckIsMdAlarmOutPut(pAlarmMd->linkage, pOutput) )
        {
        	alarmNode.fun	= AlarmLinkageOutput;
        	alarmNode.args	= pOutput;
        	PutAlarmLinkageList( &alarmNode );
        }
    	else Free(pOutput);    
        
    	ALARM_LINKAGE_CAPTURE *pCapture = (ALARM_LINKAGE_CAPTURE *)Calloc( 1, sizeof(ALARM_LINKAGE_CAPTURE) );
    	if( FI_TRUE == CheckIsMdAlarmCapture(pAlarmMd->linkage, pCapture) )
        {
        	alarmNode.fun	= AlarmLinkageCapture;
        	alarmNode.args	= pCapture;                
        	PutAlarmLinkageList( &alarmNode );
        }
    	else Free(pCapture);
        
    	ALARM_LINKAGE_RECORD *pRecord = (ALARM_LINKAGE_RECORD *)Calloc( 1, sizeof(ALARM_LINKAGE_RECORD) );
    	if( FI_TRUE == CheckIsMdAlarmRecord(pAlarmMd->linkage, pRecord) )
        {
        	alarmNode.fun	= AlarmLinkageRecord;
        	alarmNode.args	= pRecord;
        	PutAlarmLinkageList( &alarmNode );
        }
    	else Free(pRecord);            
        
    }
	return NULL;
}

void GetAlarmMdSt( int channel, PARAM_CONFIG_ALARM_MOVE_DETECT pcamd, ALARM_WORK_T *pAlarmMd )
{    
	int i, n;
	TIME_SEG_ST *pTimeSeg;
	int timeStart;
	int timeStop;
	int seg;
	WEEK_WORK_TIME     *pTimerWeek	= &(pcamd.armTimer);
    
	pAlarmMd->guard.guardTimer.timeNum = 0;

	pAlarmMd->guard.guardHand.guardFlag = pcamd.armFlag;
	pAlarmMd->linkage	                = pcamd.linkage;    
	pAlarmMd->channel = channel;
	for ( i = 0; i < MAX_WEEK_DAY; ++i )
    {
    	DAY_WORK_TIME *pTimeDay = &pTimerWeek->day[i];
    	if( FI_TRUE ==  pTimeDay->enableFlag )
        {
        	for ( seg = 0; seg < MAX_DAY_TIME_SEG; ++seg )
            {        
            	pTimeSeg = pTimeDay->timeSeg;
            	timeStart	= i * ONE_DAY_SECOND
                                + pTimeSeg[seg].timeStart.hour * 3600
                                + pTimeSeg[seg].timeStart.minute * 60
                                + pTimeSeg[seg].timeStart.second;
                
            	timeStop	= i * ONE_DAY_SECOND
                                + pTimeSeg[seg].timeEnd.hour * 3600
                                + pTimeSeg[seg].timeEnd.minute * 60
                                + pTimeSeg[seg].timeEnd.second;
                                
            	if ( timeStart < timeStop )
                {            
                	n = pAlarmMd->guard.guardTimer.timeNum;
                	pAlarmMd->guard.guardTimer.timeSeg[n].timeStart = timeStart;
                	pAlarmMd->guard.guardTimer.timeSeg[n].timeStop  = timeStop; 
                	pAlarmMd->guard.guardTimer.timeNum++;
                }
            } 
        } 
    }

	pAlarmMd->interval = pcamd.scoutInterval;
}

