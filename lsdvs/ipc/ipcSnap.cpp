/*
*******************************************************************************
**	Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**	author        :  sven
**	version       :  v1.0
**	date           :  2013.10.10
**	description  : ipc 抓拍
*******************************************************************************
*/
#include "ipcSnap.h"
#include "debug.h"
#include "public.h"
#include "ttypes.h"
#include "mutex.h"
#include "pcpIpcSnap.h"
#include "ipcHttpSnap.h"
#include "proconApp.h"

#include "timeExchange.h"

static ClMutexLock g_ipcSnapMutex[IPC_JPG_CHN_NUM];

static void GetiPCJpgDatetimeInfo( int sec, char *date, char *time )
{
	int YY, MM, DD, hh, mm, ss;
	FiTimeUtcToHuman( sec, &YY, &MM, &DD, &hh, &mm, &ss );
	sprintf( date, "%04d-%02d-%02d", YY, MM, DD );    
	sprintf( time, "%02dh%02dm%02ds", hh, mm, ss );
}

static uint g_pcpIpcSnapFd[IPC_JPG_CHN_NUM];
static int GetChnIpcSnapPcpFd( int channel )
{
	return g_pcpIpcSnapFd[channel];
}

void InitIpcSnapPcpFd()
{
	int i;
	for( i = 0; i < IPC_JPG_CHN_NUM; ++i )
    {
    	g_pcpIpcSnapFd[i] = PcpIpcSnapOpen( i, OPEN_WRONLY );
    	if( 0 == g_pcpIpcSnapFd[i] )
        {
        	SVPrint( "ch(%d) PcpIpcSnapOpen failed!\r\n", i );
        }
    }
}

void DeinitIpcSnapPcpFd()
{
	int i;
	for( i = 0; i < IPC_JPG_CHN_NUM; ++i )
    {
    	PcpIpcSnapClose( g_pcpIpcSnapFd[i] );
    	g_pcpIpcSnapFd[i] = 0;
    }
}

/*
* fn: 从ipc 抓拍到图片,然后插入pcp, 此函数的作用跟SnapMpiGetJpgAndToProcon() 相似
* snapCh: 高清通道 0
* snapType: 抓怕类型, SNAP_TYPE_TIMER 等
*/
int IpcSnapAndToPcp( int snapCh, uint snapType )
{
	int ret = -1;
	int infoLen;
	static uint 	num[IPC_JPG_CHN_NUM];
	JPG_INFO_T		jpgInfo;
	PCP_NODE_T         *pPcpNode;
    
	if( snapCh < 0 || snapCh >= IPC_JPG_CHN_NUM )
    {
    	SVPrint( "error:snapCh < 0 || snapCh >= IPC_JPG_CHN_NUM!\r\n" );
    	return -1;
    }

	g_ipcSnapMutex[snapCh].Lock();
    
	pPcpNode = PcpIpcSnapTalloc( snapCh );
	if(  NULL != pPcpNode )
    {
    	jpgInfo.num     = num[snapCh]++;
    	jpgInfo.type	= snapType;
    	GetiPCJpgDatetimeInfo( time(NULL), jpgInfo.datel, jpgInfo.timel ); 
    	jpgInfo.len     = 0;        

    	infoLen = sizeof(jpgInfo);
    	ret = HttpSnap( pPcpNode->data + infoLen, MAX_IPC_SNAP_DATA_SIZE - infoLen );
    	if( ret > 0 )
        {
        	jpgInfo.len += ret;
        	memcpy( pPcpNode->data, &jpgInfo, infoLen );
        	ret = PcpIpcSnapWrite( GetChnIpcSnapPcpFd(snapCh), pPcpNode );
        }
    	else
        {
        	PcpIpcSnapFree( pPcpNode );
        	ret = -1;
        }
    }
    
	g_ipcSnapMutex[snapCh].Unlock();
    
	return ret;
}


