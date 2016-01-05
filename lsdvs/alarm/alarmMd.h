#ifndef __ALARMMD_H__
#define __ALARMMD_H__

#include "alarm.h"
#include "paramManage.h"

void GetAlarmMdSt( int channel, PARAM_CONFIG_ALARM_MOVE_DETECT pcamd, ALARM_WORK_T *pAlarmIo );
void *MdAlarmDetectTimer( void *args );
void AlarmMdfdOpen();
int DetectMdAlarm( int channel, ALARM_WORK_T *pAlarmWork );

#endif 

