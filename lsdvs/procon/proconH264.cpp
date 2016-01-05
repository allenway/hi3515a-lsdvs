/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : H264 生产者消费者
        note: 描述视频的帧头为VIDEO_FRAME_HEAD, 在ttypes.h 中定义。
********************************************************************************/

#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "debug.h"
#include "procon.h"
#include "malloc.h"
#include "thread.h"
#include "proconH264.h"

static CProcon g_h264Procon( BLOCK_NO, DATA_TYPE_NOMAL );

void ProconH264Init( int chNum, int dataNode )
{
	g_h264Procon.Init( chNum, dataNode );
}

uint ProconH264Open( int channel, int flag )
{
	return g_h264Procon.Open( channel, flag );
}

void ProconH264Close( uint fd )
{
	g_h264Procon.Close( fd );
}

int ProconH264Write( uint fd, DATA_PIECE_T proDataInfo )
{
	int ret;    
    
	ret = g_h264Procon.Write( fd, proDataInfo );    

	return ret;
}

PROCON_NODE_T *ProconH264Read( uint fd )
{
	return g_h264Procon.Read( fd );
}

void ProconH264Free( PROCON_NODE_T *pcpNode )
{
	g_h264Procon.Tfree( pcpNode );
}


#define H264_NAL_TYPE_SEQ_PARAM 0x7
#define H264_NAL_TYPE_PIC_PARAM 0x8
typedef struct  __h264_encode_param_t
{
	char    data[1500];
	int     len;
}h264_encode_param_t;

static h264_encode_param_t g_h264_seqParamSet[MAX_CHANNEL_NUM][2];
static h264_encode_param_t g_h264_picParamSet[MAX_CHANNEL_NUM][2];
void set_h264SeqParam(int ch, int ch_type, char *param, int len, int cur_pos)
{
	if (cur_pos >= 0 && cur_pos < 4 && len <= 1500)
    {
    	g_h264_seqParamSet[ch][ch_type].len = len;
    	memcpy(g_h264_seqParamSet[ch][ch_type].data, param, len);
    }
}

void set_h264PicParam(int ch, int ch_type, char *param, int len, int cur_pos)
{
	if (cur_pos >= 0 && cur_pos < 10 && len <= 1500)
    {
    	g_h264_picParamSet[ch][ch_type].len = len;
    	memcpy(g_h264_picParamSet[ch][ch_type].data, param, len);
    }
}

int get_h264SeqParam(int ch, int ch_type, char *param, int *len, int cur_pos)
{
    *len = g_h264_seqParamSet[ch][ch_type].len;
	if (param == NULL ||  (cur_pos < 0 || cur_pos >= 4) || *len <= 0)
    {
    	return -1;
    }
	memcpy(param, g_h264_seqParamSet[ch][ch_type].data, g_h264_seqParamSet[ch][ch_type].len);
	return 0;
}

int get_h264PicParam(int ch, int ch_type, char *param, int *len, int cur_pos)
{
    *len = g_h264_picParamSet[ch][ch_type].len;
	if (param == NULL ||  (cur_pos < 0 || cur_pos >= 10) || *len <= 0)
    {
    	return -1;
    }
	memcpy(param, g_h264_picParamSet[ch][ch_type].data, g_h264_picParamSet[ch][ch_type].len);
	return 0;
}


