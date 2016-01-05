#ifndef __PCPIPC_H__
#define __PCPIPC_H__

#include "pcp.h"

void PcpIpcSnapInit( int chNum, int dataNode, uint maxSize );
uint PcpIpcSnapOpen( int channel, int flag );
void PcpIpcSnapClose( uint fd );
int PcpIpcSnapWrite( uint fd, PCP_NODE_T *pExNode );
PCP_NODE_T *PcpIpcSnapRead( uint fd );
void PcpIpcSnapFree( PCP_NODE_T *pcpNode );
PCP_NODE_T *PcpIpcSnapTalloc( int channel );

#endif 

