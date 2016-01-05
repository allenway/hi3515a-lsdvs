/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : ipc 抓拍生产者消费者
********************************************************************************/

#include "pcpIpcSnap.h"

static CPcp g_ipcSnapPcp;

void PcpIpcSnapInit( int chNum, int dataNode, uint maxSize )
{
	g_ipcSnapPcp.Init( chNum, dataNode, maxSize );
}

uint PcpIpcSnapOpen( int channel, int flag )
{
	return g_ipcSnapPcp.Open( channel, flag );
}

void PcpIpcSnapClose( uint fd )
{
	g_ipcSnapPcp.Close( fd );
}

int PcpIpcSnapWrite( uint fd, PCP_NODE_T *pExNode )
{
	int ret;    
    
	ret = g_ipcSnapPcp.Write( fd, pExNode );    

	return ret;
}

PCP_NODE_T *PcpIpcSnapRead( uint fd )
{
	return g_ipcSnapPcp.Read( fd );
}

void PcpIpcSnapFree( PCP_NODE_T *pcpNode )
{
	g_ipcSnapPcp.Tfree( pcpNode );
}

PCP_NODE_T *PcpIpcSnapTalloc( int channel )
{
	return g_ipcSnapPcp.Talloc( channel );
}

