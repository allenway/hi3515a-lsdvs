/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**	文件名: recordList.h
**  description  : for recordList.cpp
**  date           :  2013.10.18
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/

#ifndef __RECORD_LIST_H__
#define __RECORD_LIST_H__

#include "ttypes.h"

#define	FRAME_TYPE_I_FIRSCOM 	FI_FRAME_TYPE_VIDEO_I	//I帧
#define	FRAME_TYPE_P_FIRSCOM 	FI_FRAME_TYPE_VIDEO_P	//P帧

#define	MAX_FRAME_NODE	20	    //最大录像流的帧节点数目
#define MAX_FRAME_SIZE	(128*1024)

#define	MIN_FREE_MB		4	

#define	FIRSCOM_FRAME_IDENTIFY_V10	0x1FDF0201
#define	FIRSCOM_FRAME_IDENTIFY	FIRSCOM_FRAME_IDENTIFY_V10

/*do not change*/
typedef struct _RecordFrameHead
{
    unsigned long	frameIdentify;    //帧标识,0x1FDF0201,四个字节的意义 | 31 | 255-31取最近质数 | 2 | 1 | //前两个字节是固定的,后两个字节是可扩展的
    unsigned long	FrameHeadSize;    //本帧头大小
    unsigned long	frameSize;        //帧大小
    unsigned long	frameNo;        //编码器reset后所编过的帧数
	unsigned char	videoResolution;//视频时表分辨率,CIF,QCIF,D1,音频时为AMR码率模式(bit7~2)和AMR打包格式(bit1~0)    
    unsigned char	frameRate;      //当为视频帧时表帧率 当为音频帧时表采样率(bit7~2)和采样位宽(bit1~0)
	unsigned char	videoStandard;    //当为视频帧时表N/P, 为音频帧时表编码方式
	unsigned char	frameType;        //video frame: i, P and audio frame
    unsigned long	sec;            //绝对值,系统时间戳,用来快进快退
    unsigned long	usec;
	unsigned long long	pts;            //相对值,视频采样时间戳,用来智能同步
}RECORD_FRAME_HEAD;

typedef struct _RecordFrame
{
	RECORD_FRAME_HEAD frameHead;
	unsigned char *frameData;
}RECORD_FRAME;

typedef	struct	_RecordMem
{
	RECORD_FRAME fb;
	struct	_RecordMem	*next;
}RECORD_MEM;

typedef	struct	_RecordList
{
	RECORD_MEM	*freememHead;
	RECORD_MEM	*freememTail;
	RECORD_MEM	*usedmemHead;
	RECORD_MEM	*usedmemTail;
    
	unsigned int	usedListFrameCount;    //usedlist 里链表的个数,qljt用来预录
    
	pthread_mutex_t	freeListMutex;
	pthread_mutex_t	usedListMutex;
	pthread_cond_t 	condVar;
}RECORD_LIST;     

typedef struct _RecordListFrame
{
	unsigned char	frameType;        //帧类型,video frame: i, P and audio frame
    unsigned char	frameRate;      //当为视频帧时表帧率 当为音频帧时表采样率(bit7~2)和采样位宽(bit1~0)
	unsigned char	videoStandard;    //当为视频帧时表N/P, 为音频帧时表编码方式
	unsigned char 	videoResolution;//视频时表分辨率,CIF,QCIF,D1,音频时为AMR码率模式(bit7~2)和AMR打包格式(bit1~0)    
	unsigned long long	pts;        //相对值,视频采样时间戳,用来智能同步
	unsigned long     	headLen;
	unsigned char         *frameHead;
	unsigned long     	frameLen;         //帧长度	
	unsigned char         *frameData;         //帧数据	    
}RECORD_LIST_FRAME;

typedef struct _NeedFrameStatus_
{
	int state;  // 0, 当前需要I帧,1 当前需要P帧
} NEED_FRAME_STATUS_T;

// 内部
void FiRecMemListInit(int channel);
void FiRecMemListDestroy(int channel);
RECORD_MEM *FiRecGetMbFromFreeList(int channel, int size);
RECORD_MEM *FiRecGetMbFromUsedList(int channel);
void FiRecInsertMbToUsedList(int channel, RECORD_MEM *mb);
//int FiRecSendFrameToRecordStreamList(int channel, int ch_type, void *t_p_stream, int frameRate);
void FiRecRecordStreamBroadcast(int channel);

// 外部
int FiRecInitRecordStream();
void FiRecDestroyRecordStream(void);
int FiRecSendFrameToRecordStreamListEx(int channel, int ch_type, void *pRecordFrame);
void FiRecRecordStreamWaiting(int channel);
RECORD_MEM* FiRecGetFrameFromRecordStreamList(int channel);
void FiRecInsertMbToFreeList(int channel, RECORD_MEM *mb);

#endif //__RECORD_LIST_H__

