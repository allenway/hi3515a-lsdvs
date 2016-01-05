#ifndef __DCPSEARCHRECORD_H__
#define __DCPSEARCHRECORD_H__

#include "const.h"
#include "dcpTypes.h"

#define SEARCH_RESULT_PACK_SIZE (5*1024)

typedef struct _DcpSearchRecordArgs_
{
	DCP_HEAD_T msgHead;                // 消息头
	DCP_SEARCH_RECORD_COND_T searchCond; // 消息体
	ushort index;                    // 用来指明是哪个用户的操作
} DCP_SEARCH_RECORD_ARGS_T;

void *DcpSearchRecordThread( void *args );

#endif 

