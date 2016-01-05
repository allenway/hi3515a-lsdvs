/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 探头报警
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
#include "ttypes.h"


//每一位标识一个IO报警状态
static uint gAlarmIoFlag = 0;

/***************************************************************************
* fn: 检测告警输入, 判断是否触发告警.
* RETURN: FI_TRUE,检测到报警; FI_FALSE,没检测到报警
***************************************************************************/
static int DetectIoAlarm()
{
	int value	= FI_TRUE;
	int nRet0 = FI_SUCCESS;///GetAlarmIn( &value );
	int nRet1 = FI_SUCCESS;///GetAlarmIn( &value );
    //
    // TODO !!!
    //

	if ( GpioGetBit( GPIO_ALARM0_GROUP, GPIO_ALARM0_BIT ) )
    {
    	gAlarmIoFlag &= ~0x1;
    	nRet0 = FI_FAIL;
    }
	else 
    {    
    	gAlarmIoFlag |= 0x1;
    	nRet0 = FI_SUCCESS;
    }

	if ( GpioGetBit( GPIO_ALARM1_GROUP, GPIO_ALARM1_BIT ) )
    {
    	gAlarmIoFlag &= ~0x2;
    	nRet1 = FI_FAIL;
    }
	else 
    {    
    	gAlarmIoFlag |= 0x2;
    	nRet1 = FI_SUCCESS;
    }



    //gAlarmIoFlag |=   //填充IO口报警位
    
	if ( (FI_SUCCESS == nRet0)||(FI_SUCCESS == nRet1) ) return value ? FI_TRUE : FI_FALSE;
	else return FI_FALSE;
}

static int CheckIsIoAlarmOutPut( CONFIG_ALARM_LINKAGE_T	linkage, ALARM_LINKAGE_OUTPUT *pOutput )
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

static int CheckIsIoAlarmCapture( CONFIG_ALARM_LINKAGE_T linkage, ALARM_LINKAGE_CAPTURE *pCapture )
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
            	pCapture->captureType	    = SNAP_TYPE_ALARM_IO; 
            	pCapture->captureNum	    = linkage.captureNum;
            }

        	ret = FI_TRUE;
        	break;
        }
    }    

	return ret;
}

static int CheckIsIoAlarmRecord( CONFIG_ALARM_LINKAGE_T linkage, ALARM_LINKAGE_RECORD *pRecord )
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
            	pRecord->recordType	    = RECORD_TYPE_ALARM_IO;
            	pRecord->recordDuration	= linkage.recordDuration;
            }

        	ret = FI_TRUE;
        	break;
        }
    }    

	return ret;
}

static int CheckIsIoAlarmBuzzer( CONFIG_ALARM_LINKAGE_T linkage, ALARM_LINKAGE_BUZZER *pBuzzer )
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

void *IoAlarmDetectTimer( void *args )
{    
	ALARM_WORK_T         *pAlarmIo	    = (ALARM_WORK_T *)args;
	ALARM_LINKAGE_NODE	alarmNode	    = { 0 };
	static unsigned int	detectFlag	    = 0;    
	int	    	linkageTriggerFlag	    = 0;
    
	if ( CheckIsGuard( pAlarmIo->guard ) )
    {    
    	if ( FI_TRUE == DetectIoAlarm() )
        {    
        	if ( (gAlarmIoFlag != detectFlag) || (detectFlag == 0) )
            {
            	FiPrint2("alarm debug: had DetectIoAlarm!\r\n");
            	detectFlag             = gAlarmIoFlag;
            	linkageTriggerFlag     = 1;
            }
        }
    	else
        {
        	detectFlag = gAlarmIoFlag;
        }
    	AlarmUpdateFlag( RES_ALARM_TYPE_IO, gAlarmIoFlag );

        // 触发告警则将告警处理函数插入告警列表。
    	if ( linkageTriggerFlag )
        {
        	AlarmTransmit( RES_ALARM_TYPE_IO, gAlarmIoFlag );
        	ALARM_LINKAGE_OUTPUT *pOutput = (ALARM_LINKAGE_OUTPUT *)Calloc( 1, sizeof(ALARM_LINKAGE_OUTPUT) );
        	if(FI_TRUE == CheckIsIoAlarmOutPut(pAlarmIo->linkage, pOutput))
            {
            	alarmNode.fun	= AlarmLinkageOutput;
            	alarmNode.args	= pOutput;
            	PutAlarmLinkageList( &alarmNode );
            }
        	else Free(pOutput);    
            
        	ALARM_LINKAGE_CAPTURE *pCapture = (ALARM_LINKAGE_CAPTURE *)Calloc( 1, sizeof(ALARM_LINKAGE_CAPTURE) );
        	if(FI_TRUE == CheckIsIoAlarmCapture(pAlarmIo->linkage, pCapture))
            {
            	alarmNode.fun	= AlarmLinkageCapture;
            	alarmNode.args	= pCapture;                
            	PutAlarmLinkageList( &alarmNode );
            }
        	else Free(pCapture);
            
        	ALARM_LINKAGE_RECORD *pRecord = (ALARM_LINKAGE_RECORD *)Calloc( 1, sizeof(ALARM_LINKAGE_RECORD) );
        	if(FI_TRUE == CheckIsIoAlarmRecord(pAlarmIo->linkage, pRecord))
            {
            	alarmNode.fun	= AlarmLinkageRecord;
            	alarmNode.args	= pRecord;
            	PutAlarmLinkageList( &alarmNode );
            }
        	else Free(pRecord);            
            
        	ALARM_LINKAGE_BUZZER *pBuzzer = (ALARM_LINKAGE_BUZZER *)Calloc( 1, sizeof(ALARM_LINKAGE_BUZZER) );
        	if(FI_TRUE == CheckIsIoAlarmBuzzer(pAlarmIo->linkage, pBuzzer))
            {
            	alarmNode.fun	= AlarmLinkageBuzzer;
            	alarmNode.args	= pBuzzer;
            	PutAlarmLinkageList( &alarmNode );
            }
        	else Free(pBuzzer);                
        } // if ( linkageTriggerFlag )
    } // if ( CheckIsGuard( pAlarmIo->guard ) )
	return NULL;
}

void GetAlarmIoSt( PARAM_CONFIG_ALARM_IO ioAlarmParam, ALARM_WORK_T *pAlarmIo )
{
	gAlarmIoFlag = 0;

	pAlarmIo->guard.guardHand.guardFlag = ioAlarmParam.armFlag;
	pAlarmIo->normalcy	                = ioAlarmParam.normalcy;
	pAlarmIo->linkage	                = ioAlarmParam.linkage;
    
	WEEK_WORK_TIME *pTimerWeek	= &(ioAlarmParam.armTimer);
	pAlarmIo->guard.guardTimer.timeNum = 0;
    
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
                	int n = pAlarmIo->guard.guardTimer.timeNum;
                	pAlarmIo->guard.guardTimer.timeSeg[n].timeStart = timeStart;
                	pAlarmIo->guard.guardTimer.timeSeg[n].timeStop  = timeStop; 
                	pAlarmIo->guard.guardTimer.timeNum++;
                }
            }
        }
    }

	pAlarmIo->interval = ioAlarmParam.scoutInterval;
}



