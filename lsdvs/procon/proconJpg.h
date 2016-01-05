#ifndef __PROCONJPG_H__
#define __PROCONJPG_H__

#include "ttypes.h"
#include "procon.h"

void ProconJpgInit( int chNum, int dataNode );
uint ProconJpgOpen( int channel, int flag );
void ProconJpgClose( uint fd );
int ProconJpgWrite( uint fd, DATA_PIECE_T proDataInfo );
PROCON_NODE_T *ProconJpgRead( uint fd );
void ProconJpgFree( PROCON_NODE_T *pcpNode );

#endif 

