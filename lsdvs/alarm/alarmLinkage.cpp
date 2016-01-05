/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 报警联动
********************************************************************************/

#include <unistd.h>

#include "const.h"
#include "debug.h"
#include "malloc.h"

// #include "email.h"
#include "alarmLinkage.h"
#include "gpio.h"
#include "record.h"

//
// 告警输出
//
void *AlarmLinkageOutput( void *args )
{
	ALARM_LINKAGE_OUTPUT *pLinkage = (ALARM_LINKAGE_OUTPUT *)args;

    //FiPrint2("Alarm debug:AlarmLinkageOutput!\r\n");
	if( NULL != pLinkage)
    {
    	for ( int i = 0; i < pLinkage->alarmOutNum; ++i )
        {
        	if ( pLinkage->linkageAlarmOut & (0x01 << i) )
            {
            //
            // TODO !!!
            //
            //	SetAlarmOut( i, FI_TRUE );
            	sleep( pLinkage->alarmOutDuration );
            //	SetAlarmOut( i, FI_FAIL );    
            }
        }
        //FiPrint2("Alarm debug:AlarmLinkageOutput 2, pLinkage(0x%X)!\r\n", pLinkage);

    	Free( pLinkage );
        
        //FiPrint2("Alarm debug:AlarmLinkageOutput 3!\r\n");
    }
	return NULL;
}

//
// 告警抓拍
//
void *AlarmLinkageCapture( void *args )
{    
	ALARM_LINKAGE_CAPTURE *pLinkage = (ALARM_LINKAGE_CAPTURE *)args;
    
	FiPrint2("Alarm debug:AlarmLinkageCapture!\r\n");
	if( NULL != pLinkage)
    {
    	for ( int i = 0; i < REAL_CHANNEL_NUM; ++i )
        {
        	if ( pLinkage->linkageCapture & (0x01 << i) )
                ;// TODO
        }
        
    	Free( pLinkage );        
    }
	return NULL;
}

//
// 告警录像
// TODO, 以后要加上录像时长
void *AlarmLinkageRecord( void *args )
{    
	ALARM_LINKAGE_RECORD *pLinkage = (ALARM_LINKAGE_RECORD *)args;
    
	FiPrint2("Alarm debug:AlarmLinkageRecord, pLinkage->linkageRecord(0x%X)!\r\n", pLinkage->linkageRecord);

	if( NULL != pLinkage)
    {
    	for ( int i = 0; i < REAL_CHANNEL_NUM; ++i )
        {
        	if ( pLinkage->linkageRecord & (0x01 << i) )
            	FiRecStartRecord( i, pLinkage->recordType );        
        }    

    	Free( pLinkage );
    }
    
	return NULL;
}

//
// 蜂鸣器告警
//
void *AlarmLinkageBuzzer( void *args )
{    
	ALARM_LINKAGE_BUZZER *pLinkage = (ALARM_LINKAGE_BUZZER *)args;
    
	FiPrint2("Alarm debug:AlarmLinkageBuzzer!\r\n");

	if( NULL != pLinkage )
    {
        // GpioBuzzerEnable();
    	sleep( pLinkage->buzzerDuration );
        // GpioBuzzerDisable();
    	Free(pLinkage);
    }
	return NULL;
}

//
// Email告警
//
void *AlarmLinkageEmail( void *args )
{    

	ALARM_LINKAGE_RECORD *pLinkage = (ALARM_LINKAGE_RECORD *)args;
    
	FiPrint2("Alarm debug:AlarmLinkageEmail!\r\n");

	if( NULL != pLinkage )
    {
    	SVPrint("TODO: please send alarm email!\r\n");
    	Free(pLinkage);
    }
    
    //hh_mail_startSendMail( 0, EMAIL_SEND_TYPE_IO_PROBER );
    
	return NULL;
}


