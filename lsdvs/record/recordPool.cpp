/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 录像数据缓冲池
*******************************************************************************
*/

#include "debug.h"
#include "malloc.h"
#include "condition.h"
#include "recordPool.h"
#include "linuxFile.h"
#include "proconApp.h"

static ClCondition	 g_recordPoolCond[REAL_CHANNEL_NUM];
static RECORD_POOL_T g_recordPool[REAL_CHANNEL_NUM];
static uint g_recordWriteFd[REAL_CHANNEL_NUM];
static uint g_recordReadFd[REAL_CHANNEL_NUM];

static int GetRecordWriteFd( int channel )
{
	return g_recordWriteFd[channel];
}

static int GetRecordReadFd( int channel )
{
	return g_recordReadFd[channel];
}

void RecordPoolSignal( int channel )
{
	g_recordPoolCond[channel].Signal();
}
//若要使用本地录像，请使用H264PROCON
void InitRecordPool()
{
	int i;
	for( i = 0; i < REAL_CHANNEL_NUM; ++i )
    {
    	g_recordWriteFd[i] = PcpRecordOpen( i, OPEN_WRONLY );
    	if( 0 == g_recordWriteFd[i] )
        {
        	SVPrint( "ch(%d) PcpRecordOpen failed!\r\n", i );
        }

    	g_recordReadFd[i] = PcpRecordOpen( i, OPEN_RDONLY );
    	if( 0 == g_recordReadFd[i] )
        {
        	SVPrint( "ch(%d) PcpRecordOpen failed!\r\n", i );
        }

    	g_recordPool[i].pcpNode = NULL;
    	g_recordPool[i].state     = RECORD_POOL_STATE_INIT;
    }

	Memset( g_recordPool, 0x00, sizeof(g_recordPool) );
}

void DeinitRecordPool()
{
	int i;
	for( i = 0; i < REAL_CHANNEL_NUM; ++i )
    {
    	PcpRecordClose( g_recordWriteFd[i] );
    	g_recordWriteFd[i] = 0;

    	PcpRecordClose( g_recordReadFd[i] );
    	g_recordReadFd[i] = 0;
    }
}

static uchar GetFrameType( PROCON_NODE_T *pStream )
{
	uchar type;
	STREAM_HEAD_T *pStreamHead = (STREAM_HEAD_T *)pStream->data;
	type = pStreamHead->frameHead.frameType;

	return type;
}

static int WriteNormal( int channel, RECORD_POOL_T *prp, PROCON_NODE_T *pStream )
{
	int ret = -1;
	if( (prp->pcpNode->pcpHead.len + pStream->proconHead.len ) > MAX_RECORD_DATA_SIZE )
    {
    	ret = PcpRecordWrite( GetRecordWriteFd(channel), prp->pcpNode );
    	if( -1 == ret )
        {
        	PcpRecordFree( prp->pcpNode );
        }
    	else
        {
        	RecordPoolSignal( channel );
        }

    	prp->pcpNode = PcpRecordTalloc( channel );
    	if( NULL != prp->pcpNode && pStream->proconHead.len <= MAX_RECORD_DATA_SIZE )
        {
        	Memcpy( prp->pcpNode->data, pStream->data, pStream->proconHead.len );
        	prp->pcpNode->pcpHead.type = GetFrameType( pStream );
        	prp->pcpNode->pcpHead.len += pStream->proconHead.len;
        	prp->state	= RECORD_POOL_STATE_WAIT_I;
        	ret = 0;
        }
    }
	else
    {
    	Memcpy( prp->pcpNode->data + prp->pcpNode->pcpHead.len, 
                	pStream->data, pStream->proconHead.len );
    	prp->pcpNode->pcpHead.len += pStream->proconHead.len;
    	ret = 0;
    }

	return ret;
}

static int WriteWaitI( int channel, RECORD_POOL_T *prp, PROCON_NODE_T *pStream )
{
	int ret = -1;
	uchar frameType;
    
	frameType = GetFrameType( pStream );
	if( FI_FRAME_TYPE_VIDEO_I == frameType )
    {
    	if( NULL != prp->pcpNode )
        {
        	ret = PcpRecordWrite( GetRecordWriteFd(channel), prp->pcpNode );
        	if( -1 == ret )
            {
            	PcpRecordFree( prp->pcpNode );
            }
        	else
            {
            	RecordPoolSignal( channel );
            }
        }
    	prp->pcpNode = PcpRecordTalloc( channel );
    	if( NULL != prp->pcpNode && pStream->proconHead.len <= MAX_RECORD_DATA_SIZE )
        {
        	Memcpy( prp->pcpNode->data, pStream->data, pStream->proconHead.len );
        	prp->pcpNode->pcpHead.type = GetFrameType( pStream );
        	prp->pcpNode->pcpHead.len += pStream->proconHead.len;
        	prp->state	= RECORD_POOL_STATE_NORMAL;
        	ret = 0;
        }
    }
	else if( (prp->pcpNode->pcpHead.len + pStream->proconHead.len ) > MAX_RECORD_DATA_SIZE )
    {
    	ret = PcpRecordWrite( GetRecordWriteFd(channel), prp->pcpNode );
    	if( -1 == ret )
        {
        	PcpRecordFree( prp->pcpNode );
        }
    	else
        {
        	RecordPoolSignal( channel );
        }

    	prp->pcpNode = PcpRecordTalloc( channel );
    	if( NULL != prp->pcpNode && pStream->proconHead.len <= MAX_RECORD_DATA_SIZE )
        {
        	Memcpy( prp->pcpNode->data, pStream->data, pStream->proconHead.len );
        	prp->pcpNode->pcpHead.type = GetFrameType( pStream );
        	prp->pcpNode->pcpHead.len += pStream->proconHead.len;
        	ret = 0;
        }
    }
	else
    {
    	Memcpy( prp->pcpNode->data + prp->pcpNode->pcpHead.len, 
                	pStream->data, pStream->proconHead.len );
    	prp->pcpNode->pcpHead.len += pStream->proconHead.len;
    	ret = 0;
    }

	return ret;
}

/*
* fn: 从ipc 抓拍到图片,然后插入pcp, 此函数的作用跟SnapMpiGetJpgAndToProcon() 相似
* snapCh: 高清通道 0
* snapType: 抓怕类型, SNAP_TYPE_TIMER 等
*/
int RecordPoolWrite( int channel, PROCON_NODE_T *pStream )
{
	int ret;
	RECORD_POOL_T *prp = &g_recordPool[channel];

	switch( prp->state )
    {
    	case RECORD_POOL_STATE_NORMAL:
        	ret = WriteNormal( channel, prp, pStream );
        	break;

    	case RECORD_POOL_STATE_WAIT_I:
    	case RECORD_POOL_STATE_INIT:
        	ret = WriteWaitI( channel, prp, pStream );
    	break;

    	default:
        	SVPrint( "Not support prp->state(%d)!\r\n", prp->state );
        	ret = -1;
        	break;
    }
	return ret;
} 

void RecordPoolFree( PCP_NODE_T *pcpNode )
{
	PcpRecordFree( pcpNode );
}

void RecordPoolWait( int channel )
{
	g_recordPoolCond[channel].Wait();
}    

PCP_NODE_T *RecordPoolRead( int channel )
{    
	return PcpRecordRead( GetRecordReadFd(channel) );    
}

