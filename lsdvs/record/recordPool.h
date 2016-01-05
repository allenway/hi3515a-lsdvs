#ifndef __RECORDPOOL_H__
#define __RECORDPOOL_H__

#include "pcpRecord.h"    // 用到该头文件的结构体
#include "procon.h"    // 用到该头文件的结构体

typedef enum _RecordPoolState_
{
	RECORD_POOL_STATE_INIT = 0, // 初始状态
	RECORD_POOL_STATE_NORMAL,    // 常态
	RECORD_POOL_STATE_WAIT_I,    // 等待I 帧
} RECORD_POOL_STATE_EN;

typedef struct _RecordPool_
{
	int	        	state;    
	PCP_NODE_T         *pcpNode;
} RECORD_POOL_T;

void InitRecordPool();
void DeinitRecordPool();
int RecordPoolWrite( int channel, PROCON_NODE_T *pStream );
void RecordPoolSignal( int channel );
void RecordPoolWait( int channel );
PCP_NODE_T *RecordPoolRead( int channel );
void RecordPoolFree( PCP_NODE_T *pcpNode );

#endif // __RECORDPOOL_H__

