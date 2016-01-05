/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 报警链表
********************************************************************************/

#include <string.h>
#include "debug.h"
#include "const.h"
#include "malloc.h"
#include "mutex.h"
#include "condition.h"
#include "ringList.h"
#include "alarmList.h"

static CRingList 	s_AlarmLinkage;
static ClCondition 	s_AlarmMsgCondition;

void WaitAlarmLinkageList()
{
	if ( s_AlarmLinkage.Size() == 0 )
    	s_AlarmMsgCondition.Wait();
}

void SignalAlarmLinkageList()
{
	s_AlarmMsgCondition.Signal();    
}

void InitAlarmLinkageList()
{
	const int MAX_ALARM_LINKAGE_NUM = 128;
	s_AlarmLinkage.SetMax( MAX_ALARM_LINKAGE_NUM );
}

int PopAlarmLinkageList( ALARM_LINKAGE_NODE *pAlarmLinkage )
{
	if( NULL == pAlarmLinkage )
    {
    	SVPrint( "error:NULL == pAlarmLinkage!\r\n" );
    	return -1;
    }
	WaitAlarmLinkageList();
	return s_AlarmLinkage.Pop( pAlarmLinkage, NULL );
}

int PutAlarmLinkageList( ALARM_LINKAGE_NODE *pAlarmLinkage )
{
	int nRet = 0;
    
	if( NULL == pAlarmLinkage )
    {
    	SVPrint( "error:NULL == pAlarmLinkage!\r\n" );
    	return -1;
    }
    
	if ( s_AlarmLinkage.IsFull() )
    {
    	SVPrint( "failed: Alarm message list is full!\r\n" );
    	nRet = -1;
    }
	if ( nRet == 0 )
    {
    	nRet = s_AlarmLinkage.Put( (void *)pAlarmLinkage, sizeof(*pAlarmLinkage) );
    	if ( nRet == 0 && s_AlarmLinkage.Size() == 1 ) SignalAlarmLinkageList();
    }
	return nRet;
}

void ClearAlarmLinkageList()
{
	int	        	nRet	= -1;
	ALARM_LINKAGE_NODE	aNode	= { 0 };
    
	while ( ! s_AlarmLinkage.IsEmpty() )
    {
    	nRet = s_AlarmLinkage.Pop( (void *)&aNode, NULL );
    	if ( nRet == 0 ) Free( aNode.args );
    }
}

