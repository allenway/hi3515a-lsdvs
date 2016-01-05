#ifndef _ALARM_MSG_LIST_H
#define _ALARM_MSG_LIST_H

typedef void * (*ALARM_FUN)( void * );

typedef struct alarmlinkageNode
{
	ALARM_FUN		fun;        // 告警处理函数
	void *        	args;        // 告警处理函数参数
} ALARM_LINKAGE_NODE;

void WaitAlarmLinkageList();
void SignalAlarmLinkageList();
void InitAlarmLinkageList();
int PopAlarmLinkageList( ALARM_LINKAGE_NODE *pAlarmLinkage );
int PutAlarmLinkageList( ALARM_LINKAGE_NODE *pAlarmLinkage );
void ClearAlarmLinkageList();

#endif  

