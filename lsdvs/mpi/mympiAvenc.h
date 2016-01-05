#ifndef __MYMPIAVENC_H__
#define __MYMPIAVENC_H__
#if defined MCU_HI3515



#if defined MCU_HI3515
#include "hi_comm_avenc.h"
#endif
#include "thread.h"

typedef struct _AudioFlag_
{
	int  operateFlag;    // 操作标志, 1 表示有被修改过
	uint val;             // 0,关闭音频(仅仅视频流); 1,开启音频(音视频流)
} AUDIO_FLAG_T;

typedef struct _AvencSt_
{
	pthread_t threadId;
	int  runFlag;
	AUDIO_FLAG_T	 avChange;    // 音视频流需求改变了
	AVENC_CHN          AVChnId;    // 通道
	AVENC_OPERATE_OBJECT_E enObject;
} AVENC_ST;

#ifdef __cplusplus
extern "C"{
#endif

int MympiAvencStart();
int MympiAvencStop();

#ifdef __cplusplus
}
#endif


#endif //#if defined MCU_HI3515A do not use now
#endif // __MYMPIAVENC_H__

