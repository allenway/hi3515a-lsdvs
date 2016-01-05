/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 定义一些公共的结构体
********************************************************************************/

#ifndef __TTYPES_H__
#define __TTYPES_H__

#include "const.h"
#include <pthread.h>

typedef enum _PackType_
{
	PACK_TYPE_VIDEO = 0,
	PACK_TYPE_AUDIO,    
	PACK_TYPE_SMART,
} PACK_TYPE_E;

#ifndef PACK_ALIGN
#define PACK_ALIGN	__attribute__((packed))
#endif

typedef struct _PackHead_
{
	uchar 	packType;    // 包类型, 0:视频包; 1:音频包; 2:智能包, 见 PACK_TYPE_E
	uchar	frameHeadLen;    // 帧头长度,该长度根据packType 的值变化而取 sizeof(不同的结构体)    
	char	reserved[2];
}PACK_ALIGN PACK_HEAD_T;

// 音视频帧头描述
typedef struct _FrameHead_
{
    uint  frameLen;            // 帧裸流长度
    uint  frameNo;            // 帧号,单调递增
    uchar videoStandard;    // 视频:标准 0,PAL; 1,NTSC. // 音频:通道模式, 0:单通道, 1:立体声, 默认值 0
    uchar videoResolution;    // 视频:分辨率 0,qcif; 1,cif; 2,hd1; 3,d1. // 音频:采样率, 0:8k, 1:16k, 默认值 0
    uchar frameRate;          // 视频:帧率. // 音频:位宽,0:8bits, 1:16bits, 默认值 1
    uchar frameType;         // 视频:帧类型 5,I帧; 1,P帧 . // 音频::编码方式,0:adpcm, 1:g711, 2:g726, 默认值 0
    uint  sec;                // 从本地获取的时间,秒
    uint  usec;                // 从本地获取的时间,微妙
    uint64_t  pts;                // 从海思底层获取的时间戳,(微秒)
}PACK_ALIGN FRAME_HEAD_ST;

typedef struct _StreamHead_
{
	PACK_HEAD_T 	packHead;
	FRAME_HEAD_ST	frameHead;
}PACK_ALIGN STREAM_HEAD_T;

#undef PACK_ALIGN

/* ======================= end 流相关 ======================= */

/* ======================= 其它 ======================= */
// 数据类型
typedef enum _DataType_
{
	DATA_TYPE_NOMAL = 0,    // 常规数据
	DATA_TYPE_VIDEO_H264,    // 视频H264 数据
	DATA_TYPE_VIDEO_YUV,    // 视频YUV 数据
	DATA_TYPE_AUDIO,        // 音频
	DATA_TYPE_H264_I,        // H264 数据中的I 帧	
	DATA_TYPE_H264_P,        // H264 数据中的P 帧
	DATA_TYPE_REC_STOP,        // 录像的结束标识
} DATA_TYPE_EN;    

#define MAX_NAL_IN_ONE_FRAME	16 // 一帧最多允许有8 个nal
typedef struct _NalInfo_
{
	uint	 nalNum;    // 一帧里面有多少个nal
	uint nalSize[MAX_NAL_IN_ONE_FRAME]; // 每个nal 的长度
	uint nalStartOff;        // nal 开始偏移的位置
} NAL_INFO_T;

// 通用的数据组成
#define MAX_DATA_PIECE_SIZE 16
typedef struct _DataPiece_
{
	int         	count; // 注意, count 必须<= MAX_DATA_PIECE_SIZE
	char	        *buf[MAX_DATA_PIECE_SIZE];    // 第几块数据指向的地址
	unsigned int 	len[MAX_DATA_PIECE_SIZE];    // 第几块数据的长度
	NAL_INFO_T		nalInfo;    // 本帧中nal 的信息
} DATA_PIECE_T;

#define	MAX_PROCON_DATA_NODE		64	// 一个通道最多缓存64 个节点
#define MAX_PROCON_DATA_NODE_SIZE	(256*1024) // 一般情况下一个节点最多支持128K

typedef struct _ProconHead_
{
	int type;    // 数据类型,DATA_TYPE_EN 中的其中一种
	int len;    // 节点数据的大小
	NAL_INFO_T nalInfo;    // 视频帧的时候用来描述nal 的信息
} PROCON_HEAD_T;

typedef struct _ProConNode_
{
	PROCON_HEAD_T 	proconHead;
	char         	data[1];
} PROCON_NODE_T;

typedef struct _ProconSt_
{
	int	            	writePos;
	pthread_mutex_t		dataMutex;
	PROCON_NODE_T	    *dataPool[MAX_PROCON_DATA_NODE];
} PROCON_T;

typedef struct _ProconRead_
{
	int flag;
	int channel;
	int readBegin;
	int readEnd;
} PROCON_READ_T;

typedef struct _ProconWrite_
{
	int flag;
	int channel;
} PROCON_WRITE_T;

// producer consumer open flag
// 用于 ProconH264Open() 等函数的flag 参数
typedef enum _OpenEn_
{
	OPEN_RDONLY = 0,
	OPEN_WRONLY
} OPEN_EN;

typedef enum _BlockEn_
{
	BLOCK_NO = 0,    // 以非阻塞的方式
	BLOCK_YES	    // 以阻塞的方式
} BLOCK_EN;

#define FI_FRAME_TYPE_VIDEO_I		5	// I帧
#define FI_FRAME_TYPE_VIDEO_P		1	// P帧
#define FI_FRAME_TYPE_SEI	    	6     // 参数帧
#define FI_FRAME_TYPE_SPS	    	7     // 队列参数设置帧
#define FI_FRAME_TYPE_PPS	    	8     // 图像参数设置帧

// wifi搜索结果
typedef struct _WifiScanResult_
{
	char	essid[32];        // 最多支持32个字节
	char	encType;        // 0-无加密;1-wep;2-wpa/wpa2(802.1x);3-wpa-psk/wpa2-psk.目前终端支持0、1、3这三种连接
	char	signalLevel;    // 信号强度(单位 格)1-5
	char	reserved[2];            
} WIFI_SCAN_RESULT;

// 
// wifi连接配置
//
typedef struct _ConfigWifiConnect_
{
	char	essid[32];        // 最多支持32个字节,支持手动输入和从搜索中获取
	char	key[64];        // 密码
	unsigned char connectFlag;    // 只读, 0-没有连接到无线路由;1-已经连接到无线路由
	unsigned char signalLevel;    // 只读, 信号强度, 1~5格
	char	reserved[14];
} CONFIG_WIFI_CONNECT;


typedef struct talkbackNode
{
	unsigned int	timeStamp;        // 时间戳
	unsigned int	frameSeq;        // 帧序列
	unsigned int	frameLen;        // 帧长度
	unsigned char *	frameData;        // 帧内容
} TALKBACK_NODE;

//
// OSD参数配置
//
typedef struct ConfigOsdLogo
{
	unsigned char	enable;                // 是否显示OSD 1: enable, 0: disable
	unsigned char	colorRed;            // 红色
	unsigned char	colorGreen;            // 绿色
	unsigned char	colorBlue;            // 蓝色
	unsigned short	xPos;                // 坐标
	unsigned short	yPos;                // 坐标
	char	    	logo[64];            // 最大支持64Bytes的logo
	unsigned char	bgTransparence;        // 背景透明度 0~100,标示0%~100%
	char	reserved[7];
}  CONFIG_OSD_LOGO;

typedef struct ConfigOsdTime
{
	unsigned char	enable;
	unsigned char	colorRed;    
	unsigned char	colorGreen;    
	unsigned char	colorBlue;    
	unsigned short	xPos;
	unsigned short	yPos;
	unsigned char	bgTransparence;        // 背景透明度 0~100,标示0%~100%
	char	    	reserved[7];
}  CONFIG_OSD_TIME;

typedef struct _JpgInfo_
{
	unsigned int  type;      // 抓拍的类型
	unsigned int  num;        // 第几张
	char 	datel[DATE_LEN]; // 抓拍的日期时间
	char	timel[TIME_LEN]; //
	int 	len;    // 图片的长度
} JPG_INFO_T;

//
// 视频参数
//
#define FRAME_HEAD_ID	"FhIeRaSdDvV1S0"        

typedef enum
{
	FRAME_TYPE_NULL = 0,
	FRAME_TYPE_VIDEO,        // 视频帧类型
	FRAME_TYPE_AUDIO,        // 音频帧类型
	FRMAE_TYPE_SMART,        // 智能帧类型
} FRAME_TYPE_EN;

typedef enum
{
	VIDEO_FORMAT_NULL = 0,
	VIDEO_FORMAT_H264,
	VIDEO_FORMAT_MPEG,
} VIDEO_FORMAT_EN;

typedef struct _SnapMsg_
{
	int 	channel;
	uint 	snapType;
} SNAP_MSG_T;

typedef struct _SnapHead_
{
	int		channel;    // 通道
	uint	snapType;    // 抓拍类型
	int 	sec;        // 时间戳
	int 	jpgLen;        // jpg len
} SNAP_HEAD_T;

//DVS上传客户端报警类型定义
//#define RES_ALARM_TYPE_IO	        	0     //IO报警
//#define RES_ALARM_TYPE_VLOSS	    	1     //视频丢失报警
//#define RES_ALARM_TYPE_SHELTER	    	2     //视频遮挡
//#define RES_ALARM_TYPE_IO_CANCEL	    3     //IO报警取消
//#define RES_ALARM_TYPE_VLOSS_CANCEL		4     //视频丢失报警取消
//#define RES_ALARM_TYPE_SHELTER_CANCEL	5     //视频遮挡取消

typedef enum
{
    RES_ALARM_TYPE_IO = 0,
    RES_ALARM_TYPE_VLOSS,
    RES_ALARM_TYPE_SHELTER,
    RES_ALARM_TYPE_IO_CANCEL,
    RES_ALARM_TYPE_VLOSS_CANCEL,
    RES_ALARM_TYPE_SHELTER_CANCEL
}ALARM_TYPE_E;

#endif

