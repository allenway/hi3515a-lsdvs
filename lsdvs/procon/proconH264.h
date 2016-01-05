#ifndef __PROCONH264_H__
#define __PROCONH264_H__

#include "const.h"
#include "ttypes.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif

void ProconH264Init( int chNum, int dataNode );
uint ProconH264Open( int channel, int flag );
void ProconH264Close( uint fd );
int ProconH264Write( uint fd, DATA_PIECE_T proDataInfo );
PROCON_NODE_T *ProconH264Read( uint fd );
void ProconH264Free( PROCON_NODE_T *pcpNode );

void set_h264SeqParam(int ch, int ch_type, char *param, int len, int cur_pos);
void set_h264PicParam(int ch, int ch_type, char *param, int len, int cur_pos);
int get_h264SeqParam(int ch, int ch_type, char *param, int *len, int cur_pos);
int get_h264PicParam(int ch, int ch_type, char *param, int *len, int cur_pos);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif 

#endif 

