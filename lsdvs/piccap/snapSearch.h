#ifndef __SNAPSEARCH_H__
#define __SNAPSEARCH_H__

#include "const.h"
#include "public.h"

typedef struct _SnapInquire
{
	char 	snapName[MAX_SNAP_JPG_PATH_LEN];
	int 	snapTime;        // 该图片的抓拍时间
	int 	stopTime;        // reserve
	uint 	snapLen;        // 大小
	uint 	snapType;         // reserve
	struct _SnapInquire *next;
	struct _SnapInquire *prev;
}SNAP_INQUIRE;

typedef struct _SnapInquireList
{
	SNAP_INQUIRE *head;
	SNAP_INQUIRE *cur;
	SNAP_INQUIRE *tail;
}SNAP_INQUIRE_LIST;

SNAP_INQUIRE_LIST *FiSnapInquireSnapFile( int channel, uint snapType, 
                                    	int startTime, int stopTime );
void FiSnapFreeInquireSnap( SNAP_INQUIRE_LIST *snapInquireResult );


#endif

