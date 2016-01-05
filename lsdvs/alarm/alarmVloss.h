#ifndef __VLOSSALARM_H__
#define __VLOSSALARM_H__

#include "paramManage.h"
#include "alarm.h"

void *VlossAlarmDetectTimer( void *args );
void GetAlarmVlossSt( PARAM_CONFIG_ALARM_VIDEO_LOSE vlossAlarmParam, ALARM_WORK_T *pAlarmVloss );
int DetectVlossAlarm( int channel, ALARM_WORK_T *pAlarmWork );

#endif 

