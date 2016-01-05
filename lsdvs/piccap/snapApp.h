#ifndef __SNAPAPP_H__
#define __SNAPAPP_H__

typedef struct _SnapWorkTimer_
{
	int	enable;        // 是否使能定时抓拍
	int startSec;    // 开始时间,每天的的哪一秒
	int stopSec;    // 停止时间,每天的的哪一秒
	int interval;    // 抓拍间隔
} SNAP_WORK_PARAM_T;

void StartSnapAppThread();
void StopSnapAppThread();
int SnapAppMessageSend( int channel, int snapType );

void AddSnapTimer();
void InitSnapWorkParam();
void SnapTimerReleaseWorkParam();

#endif

