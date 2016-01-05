#ifndef __IOALARM_H__
#define __IOALARM_H__

#include "paramManage.h"
#include "alarm.h"

void *IoAlarmDetectTimer( void *args );
void GetAlarmIoSt( PARAM_CONFIG_ALARM_IO ioAlarmParam, ALARM_WORK_T *pAlarmIo );

#endif 

