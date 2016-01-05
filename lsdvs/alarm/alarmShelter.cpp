/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.18
**  description  : 视频遮挡报警
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
#include "mpiViFrame.h"
#include "ttypes.h"
#include "fit.h"


//每一位标识一个视频丢失报警状态
static uint gAlarmShelterFlag;

/******************************************************************************
* fn: 检测告警输入, 判断是否触发告警.
* 返回: FI_TRUE,检测到报警; FI_FALSE,没检测到报警
*******************************************************************************/
static int GetDetectShelterAlarmSt()
{
	int value	= FI_TRUE;
	int nRet = FI_SUCCESS;///GetAlarmIn( &value );
	int i;

	for ( i=0; i<REAL_CHANNEL_NUM; i++ )
    {
    	if ( (FitMympiGetShelterDetect( i ))&&(0==DriverGetAdVideoLoss( i )) ) 
        {
        	gAlarmShelterFlag |= 1<<i;
        }
    	else 
        {    
        	gAlarmShelterFlag &= ~(1<<i);    
        }
    }
	if ( gAlarmShelterFlag>0 ) 
    {
    	nRet = FI_SUCCESS;
    }    
	else {
    	nRet = FI_FAIL;
    }

	if ( FI_SUCCESS == nRet ) return value ? FI_TRUE : FI_FALSE;
	else return FI_FALSE;
}



static int CheckIsShelterAlarmOutPut( CONFIG_ALARM_LINKAGE_T	linkage, ALARM_LINKAGE_OUTPUT *pOutput )
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

static int CheckIsShelterAlarmCapture( CONFIG_ALARM_LINKAGE_T linkage, ALARM_LINKAGE_CAPTURE *pCapture )
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
            	pCapture->captureType	    = SNAP_TYPE_ALARM_SHELTER; 
            	pCapture->captureNum	    = linkage.captureNum;
            }

        	ret = FI_TRUE;
        	break;
        }
    }    

	return ret;
}

static int CheckIsShelterAlarmRecord( CONFIG_ALARM_LINKAGE_T linkage, ALARM_LINKAGE_RECORD *pRecord )
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
            	pRecord->recordType	    = RECORD_TYPE_ALARM_SHELTER;
            	pRecord->recordDuration	= linkage.recordDuration;
            }

        	ret = FI_TRUE;
        	break;
        }
    }    

	return ret;
}

static int CheckIsShelterAlarmBuzzer( CONFIG_ALARM_LINKAGE_T linkage, ALARM_LINKAGE_BUZZER *pBuzzer )
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

void ShelterAlarmDetectLinkage( ALARM_WORK_T pAlarmWork )
{    
	ALARM_LINKAGE_NODE	alarmNode	    = { 0 };


        // 触发告警则将告警处理函数插入告警列表。
    	if ( CheckIsGuard( pAlarmWork.guard ) )
        {            
        	AlarmTransmit( RES_ALARM_TYPE_SHELTER, gAlarmShelterFlag );
        	ALARM_LINKAGE_OUTPUT *pOutput = (ALARM_LINKAGE_OUTPUT *)Calloc( 1, sizeof(ALARM_LINKAGE_OUTPUT) );
        	if(FI_TRUE == CheckIsShelterAlarmOutPut(pAlarmWork.linkage, pOutput))
            {
            	alarmNode.fun	= AlarmLinkageOutput;
            	alarmNode.args	= pOutput;
            	PutAlarmLinkageList( &alarmNode );
            }
        	else Free(pOutput);    
            
        	ALARM_LINKAGE_CAPTURE *pCapture = (ALARM_LINKAGE_CAPTURE *)Calloc( 1, sizeof(ALARM_LINKAGE_CAPTURE) );
        	if(FI_TRUE == CheckIsShelterAlarmCapture(pAlarmWork.linkage, pCapture))
            {
            	alarmNode.fun	= AlarmLinkageCapture;
            	alarmNode.args	= pCapture;                
            	PutAlarmLinkageList( &alarmNode );
            }
        	else Free(pCapture);
            
        	ALARM_LINKAGE_RECORD *pRecord = (ALARM_LINKAGE_RECORD *)Calloc( 1, sizeof(ALARM_LINKAGE_RECORD) );
        	if(FI_TRUE == CheckIsShelterAlarmRecord(pAlarmWork.linkage, pRecord))
            {
            	alarmNode.fun	= AlarmLinkageRecord;
            	alarmNode.args	= pRecord;
            	PutAlarmLinkageList( &alarmNode );
            }
        	else Free(pRecord);            

        	ALARM_LINKAGE_BUZZER *pBuzzer = (ALARM_LINKAGE_BUZZER *)Calloc( 1, sizeof(ALARM_LINKAGE_BUZZER) );
        	if(FI_TRUE == CheckIsShelterAlarmBuzzer(pAlarmWork.linkage, pBuzzer))
            {
            	alarmNode.fun	= AlarmLinkageBuzzer;
            	alarmNode.args	= pBuzzer;
            	PutAlarmLinkageList( &alarmNode );
            }
        	else Free(pBuzzer);                        
        } // if ( linkageTriggerFlag )

}

//
// 检测告警输入, 判断是否触发告警.
//
int DetectVideoShelterAlarm( int channel, ALARM_WORK_T *pAlarmWork )
{
	static uint		detectFlag             = 0;
	int	        	linkageTriggerFlag	    = 0;

	if ( FI_TRUE == GetDetectShelterAlarmSt() )
    {
    	if ( (gAlarmShelterFlag != detectFlag) || (detectFlag == 0) )
        {
        	FiPrint2("alarm debug: had DetectVideoShelterAlarm!\r\n");
        	detectFlag             = gAlarmShelterFlag;
        	linkageTriggerFlag     = 1;
        }
    }
	else
    {
    	detectFlag = gAlarmShelterFlag;
    }
	AlarmUpdateFlag( RES_ALARM_TYPE_SHELTER, gAlarmShelterFlag ); 
    
	if ( linkageTriggerFlag )
    {
    	ShelterAlarmDetectLinkage( *pAlarmWork );
    }
	return 0;
}



void GetAlarmShelterSt( PARAM_CONFIG_ALARM_VIDEO_SHELTER videoShelterAlarmParam, ALARM_WORK_T *pAlarmShelter )
{

	gAlarmShelterFlag = 0;
	pAlarmShelter->guard.guardHand.guardFlag = videoShelterAlarmParam.armFlag;
	pAlarmShelter->linkage	                = videoShelterAlarmParam.linkage;
    
	WEEK_WORK_TIME *pTimerWeek	= &(videoShelterAlarmParam.armTimer);
	pAlarmShelter->guard.guardTimer.timeNum = 0;
    
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
                	int n = pAlarmShelter->guard.guardTimer.timeNum;
                	pAlarmShelter->guard.guardTimer.timeSeg[n].timeStart = timeStart;
                	pAlarmShelter->guard.guardTimer.timeSeg[n].timeStop  = timeStop; 
                	pAlarmShelter->guard.guardTimer.timeNum++;
                }
            } // for ( int seg = 0; seg < MAX_DAY_TIME_SEG; ++seg )
        } // if( FI_TRUE ==  pTimeDay->enableFlag )
    } // for ( int i = 0; i < MAX_WEEK_DAY; ++i )

	pAlarmShelter->interval = videoShelterAlarmParam.scoutInterval;    
    
}



