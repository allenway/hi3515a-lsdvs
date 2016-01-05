#ifndef __PROCONMD_H__
#define __PROCONMD_H__

#include "ttypes.h"
#include "procon.h"

void ProconMdInit( int chNum, int dataNode );
uint ProconMdOpen( int channel, int flag );
void ProconMdClose( uint fd );
int ProconMdWrite( uint fd, DATA_PIECE_T proDataInfo );
PROCON_NODE_T *ProconMdRead( uint fd );
void ProconMdFree( PROCON_NODE_T *pcpNode );

#endif 

