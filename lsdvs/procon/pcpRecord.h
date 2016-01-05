#ifndef __PCPRECORD_H__
#define __PCPRECORD_H__

#include "pcp.h"

void PcpRecordInit( int chNum, int dataNode, uint maxSize );
uint PcpRecordOpen( int channel, int flag );
void PcpRecordClose( uint fd );
int PcpRecordWrite( uint fd, PCP_NODE_T *pExNode );
PCP_NODE_T *PcpRecordRead( uint fd );
void PcpRecordFree( PCP_NODE_T *pcpNode );
PCP_NODE_T *PcpRecordTalloc( int channel );

#endif

