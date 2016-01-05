/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 录像专用的数据缓冲池
********************************************************************************/


#include "pcpRecord.h"

static CPcp g_ipcSnapPcp;

void PcpRecordInit( int chNum, int dataNode, uint maxSize )
{
	g_ipcSnapPcp.Init( chNum, dataNode, maxSize );
}

uint PcpRecordOpen( int channel, int flag )
{
	return g_ipcSnapPcp.Open( channel, flag );
}

void PcpRecordClose( uint fd )
{
	g_ipcSnapPcp.Close( fd );
}

int PcpRecordWrite( uint fd, PCP_NODE_T *pExNode )
{
	int ret;    
    
	ret = g_ipcSnapPcp.Write( fd, pExNode );    

	return ret;
}

PCP_NODE_T *PcpRecordRead( uint fd )
{
	return g_ipcSnapPcp.Read( fd );
}

void PcpRecordFree( PCP_NODE_T *pcpNode )
{
	g_ipcSnapPcp.Tfree( pcpNode );
}

PCP_NODE_T *PcpRecordTalloc( int channel )
{
	return g_ipcSnapPcp.Talloc( channel );
}

