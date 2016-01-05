#ifndef __RECORDSEARCH_H__
#define __RECORDSEARCH_H__

#include <sys/time.h>
#include "recordFit.h"

typedef struct _RecordInquire
{
	char recFilename[RECORD_FILENAME_LEN];
	time_t startTime;
	time_t stopTime;    
	uint recLen;
	uint recType; //reserve
	struct _RecordInquire *next;
	struct _RecordInquire *prev;
}RECORD_INQUIRE;

typedef struct _RecordInquireList
{
	RECORD_INQUIRE *head;
	RECORD_INQUIRE *cur;
	RECORD_INQUIRE *tail;
}RECORD_INQUIRE_LIST;

RECORD_INQUIRE_LIST *FiRecInquireRecordFile(int channel,uint recType,
                        	time_t startTime,time_t stopTime);
void FiRecFreeInquireRecord(RECORD_INQUIRE_LIST *recInquireRet);

#endif // __RECORDSEARCH_H__

