#ifndef __SHELTERALARM_H__
#define __SHELTERALARM_H__

#include "paramManage.h"
#include "alarm.h"

void *ShelterAlarmDetectTimer( void *args );
void GetAlarmShelterSt( PARAM_CONFIG_ALARM_VIDEO_SHELTER videoShelterAlarmParam, ALARM_WORK_T *pAlarmShelter );
int DetectVideoShelterAlarm( int channel, ALARM_WORK_T *pAlarmWork );

#endif 

