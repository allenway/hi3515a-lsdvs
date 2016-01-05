#ifndef __MPIVIFRAME_H__
#define __MPIVIFRAME_H__

#include <pthread.h>



typedef struct _ShelterDetect_
{
	uchar shelterStat;  // 0:无遮挡    1:被遮挡
	uchar shelterCnt;  //检测到若干帧有效，则认为被遮挡
	uchar noShelterCnt;  //检测到若干帧无效，则认为取消遮挡
	uchar reserved[1];
}SHELTER_DETECT_T;


typedef struct _ViFrameSt_
{
	pthread_t threadId;
	int  runFlag;
} ShelterThreadStatus_t;


typedef struct _ShelterSensitivity_
{
	uint rowDotNum;      // 每行采样个数
	int  frameNum;         //超过此帧数量才认为被遮挡
	int  variance;      //方差
} SHELTER_SENSITIVITY_T;

void StartViFrameThread();//do not use in HI3515A
void StopViFrameThread();//do not use in HI3515A

void StartOdThread();
void StopOdThread();
int VideoGetShelterDetect( int channel );

#endif // __MPIVIFRAME_H__

