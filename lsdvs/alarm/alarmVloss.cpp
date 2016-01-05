/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.9.26
**  description  : 视频丢失报警
********************************************************************************/
#include "const.h"
#include "debug.h"
#include "timeExchange.h"
#include "alarmIo.h"
#include "alarmLinkage.h"
#include "alarmList.h"
#include "alarmComm.h"
#include "malloc.h"
#include "public.h"
#include "gpio.h"
#include "driver.h"
#include "ttypes.h"


//每一位标识一个视频丢失报警状态
static uint gAlarmVlossFlag;

/******************************************************************************
* fn: 检测告警输入, 判断是否触发告警.
* 返回: FI_TRUE,检测到报警; FI_FALSE,没检测到报警
******************************************************************************/
static int GetVlossAlarmSt()
{
	int value	= FI_TRUE;
	int nRet = FI_SUCCESS;///GetAlarmIn( &value );
	int i;

	for ( i=0; i<REAL_CHANNEL_NUM; i++ )
    {
    	if ( DriverGetAdVideoLoss( i )) 
        {
        	gAlarmVlossFlag |= 1<<i;
        }
    	else 
        {    
        	gAlarmVlossFlag &= ~(1<<i);    
        }
    }
	if ( gAlarmVlossFlag>0 ) 
    {
    	nRet = FI_SUCCESS;
    }    
	else {
    	nRet = FI_FAIL;
    }

	if ( FI_SUCCESS == nRet ) return value ? FI_TRUE : FI_FALSE;
	else return FI_FALSE;
}



static int CheckIsVlossAlarmOutPut( CONFIG_ALARM_LINKAGE_T	linkage, ALARM_LINKAGE_OUTPUT *pOutput )
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

static int CheckIsVlossAlarmCapture( CONFIG_ALARM_LINKAGE_T linkage, ALARM_LINKAGE_CAPTURE *pCapture )
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
            	pCapture->captureType	    = SNAP_TYPE_ALARM_LOST; 
            	pCapture->captureNum	    = linkage.captureNum;
            }

        	ret = FI_TRUE;
        	break;
        }
    }    

	return ret;
}

static int CheckIsVlossAlarmRecord( CONFIG_ALARM_LINKAGE_T linkage, ALARM_LINKAGE_RECORD *pRecord )
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
            	pRecord->recordType	    = RECORD_TYPE_ALARM_LOST;
            	pRecord->recordDuration	= linkage.recordDuration;
            }

        	ret = FI_TRUE;
        	break;
        }
    }    

	return ret;
}


static int CheckIsVlossAlarmBuzzer( CONFIG_ALARM_LINKAGE_T linkage, ALARM_LINKAGE_BUZZER *pBuzzer )
{
	int i, ret = FI_FALSE;
	int	channelNum = REAL_CHANNEL_NUM;
    
	for( i = 0; i < channelNum; i++ )
    {
    	if( linkage.linkageBuzzer & (0x01 << i) )
        {
        	if( NULL != pBuzzer )
            {
            	pBuzzer->buzzerDuration	= linkage.buzzerDuration;
            }
        	ret = FI_TRUE;
        	break;
        }
    }    

	return ret;
}

void VlossAlarmDetectLinkage( ALARM_WORK_T pAlarmVloss )
{    
	ALARM_LINKAGE_NODE	alarmNode	    = { 0 };

    // 触发告警则将告警处理函数插入告警列表。
	if ( CheckIsGuard( pAlarmVloss.guard ) )
    {            
    	AlarmTransmit( RES_ALARM_TYPE_VLOSS, gAlarmVlossFlag );
    	ALARM_LINKAGE_OUTPUT *pOutput = (ALARM_LINKAGE_OUTPUT *)Calloc( 1, sizeof(ALARM_LINKAGE_OUTPUT) );
    	if(FI_TRUE == CheckIsVlossAlarmOutPut(pAlarmVloss.linkage, pOutput))
        {
        	alarmNode.fun	= AlarmLinkageOutput;
        	alarmNode.args	= pOutput;
        	PutAlarmLinkageList( &alarmNode );
        }
    	else Free(pOutput);    
        
    	ALARM_LINKAGE_CAPTURE *pCapture = (ALARM_LINKAGE_CAPTURE *)Calloc( 1, sizeof(ALARM_LINKAGE_CAPTURE) );
    	if(FI_TRUE == CheckIsVlossAlarmCapture(pAlarmVloss.linkage, pCapture))
        {
        	alarmNode.fun	= AlarmLinkageCapture;
        	alarmNode.args	= pCapture;                
        	PutAlarmLinkageList( &alarmNode );
        }
    	else Free(pCapture);
        
    	ALARM_LINKAGE_RECORD *pRecord = (ALARM_LINKAGE_RECORD *)Calloc( 1, sizeof(ALARM_LINKAGE_RECORD) );
    	if(FI_TRUE == CheckIsVlossAlarmRecord(pAlarmVloss.linkage, pRecord))
        {
        	alarmNode.fun	= AlarmLinkageRecord;
        	alarmNode.args	= pRecord;
        	PutAlarmLinkageList( &alarmNode );
        }
    	else Free(pRecord);            

    	ALARM_LINKAGE_BUZZER *pBuzzer = (ALARM_LINKAGE_BUZZER *)Calloc( 1, sizeof(ALARM_LINKAGE_BUZZER) );
    	if(FI_TRUE == CheckIsVlossAlarmBuzzer(pAlarmVloss.linkage, pBuzzer))
        {
        	alarmNode.fun	= AlarmLinkageBuzzer;
        	alarmNode.args	= pBuzzer;
        	PutAlarmLinkageList( &alarmNode );
        }
    	else Free(pBuzzer);            
                
    } 
    
}


//
// 检测告警输入, 判断是否触发告警.
//
int DetectVlossAlarm( int channel, ALARM_WORK_T *pAlarmWork )
{
	static uint		detectFlag             = 0;
	int	        	linkageTriggerFlag	    = 0;

	if ( FI_TRUE == GetVlossAlarmSt() )
    {
    	if ( (gAlarmVlossFlag != detectFlag) || (detectFlag == 0) )
        {
        	FiPrint2("alarm debug: had DetectVlossAlarm!\r\n");
        	detectFlag             = gAlarmVlossFlag;
        	linkageTriggerFlag     = 1;
        }
    }
	else
    {
    	detectFlag = gAlarmVlossFlag;
    }
	AlarmUpdateFlag( RES_ALARM_TYPE_VLOSS, gAlarmVlossFlag ); 
    
	if ( linkageTriggerFlag )
    {
    	VlossAlarmDetectLinkage( *pAlarmWork );
    }
	return 0;
}


void GetAlarmVlossSt( PARAM_CONFIG_ALARM_VIDEO_LOSE vlossAlarmParam, ALARM_WORK_T *pAlarmVloss )
{

	gAlarmVlossFlag = 0;

	pAlarmVloss->guard.guardHand.guardFlag = vlossAlarmParam.armFlag;
	pAlarmVloss->linkage	                = vlossAlarmParam.linkage;
    
	WEEK_WORK_TIME *pTimerWeek	= &(vlossAlarmParam.armTimer);
	pAlarmVloss->guard.guardTimer.timeNum = 0;
    
	for ( int i = 0; i < MAX_WEEK_DAY; ++i )
    {
    	DAY_WORK_TIME *pTimeDay = &pTimerWeek->day[i];
    	if( FI_TRUE ==  pTimeDay->enableFlag )
        {
        	for ( int seg = 0; seg < MAX_DAY_TIME_SEG; ++seg )
            {        
            	TIME_SEG_ST *pTimeSeg = pTimeDay->timeSeg;
            	int timeStart	= i * ONE_DAY_SECOND
                                + pTimeSeg[seg].timeStart.hour * 3600
                                + pTimeSeg[seg].timeStart.minute * 60
                                + pTimeSeg[seg].timeStart.second;
                
            	int timeStop	= i * ONE_DAY_SECOND
                                + pTimeSeg[seg].timeEnd.hour * 3600
                                + pTimeSeg[seg].timeEnd.minute * 60
                                + pTimeSeg[seg].timeEnd.second;
                                
            	if ( timeStart < timeStop )
                {            
                	int n = pAlarmVloss->guard.guardTimer.timeNum;
                	pAlarmVloss->guard.guardTimer.timeSeg[n].timeStart = timeStart;
                	pAlarmVloss->guard.guardTimer.timeSeg[n].timeStop  = timeStop; 
                	pAlarmVloss->guard.guardTimer.timeNum++;
                }
            } // for ( int seg = 0; seg < MAX_DAY_TIME_SEG; ++seg )
        } // if( FI_TRUE ==  pTimeDay->enableFlag )
    } // for ( int i = 0; i < MAX_WEEK_DAY; ++i )

	pAlarmVloss->interval = vlossAlarmParam.scoutInterval;    
}



