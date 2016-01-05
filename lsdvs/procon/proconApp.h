#ifndef __PROCONAPP_H__
#define __PROCONAPP_H__

#define MAX_JPG_DATA_NODE	    10
#define MAX_H264_DATA_NODE	    (16 << 1)//(32<<1)//    // 考虑音视频同步
#define MAX_MD_DATA_NODE	    8
#define MAX_IPC_SNAP_DATA_NODE	5
#define MAX_IPC_SNAP_DATA_SIZE	(512 * 1024) // 512 KBytes 的1080p 图片
#define MAX_RECORD_DATA_NODE	8
#define MAX_RECORD_DATA_SIZE	(512 * 1024) // 512 KBytes 写一次磁盘
#define MAX_YUV_DATA_NODE		8
#define MAX_YUV_DATA_SIZE	    (720 * 576 * 2) // 一帧YUV数据


void ProconInit();

#endif

