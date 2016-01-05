/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**	文件名: recordDel.h
**  description  : for recordDel.h
**  date           :  2013.10.18
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/

#ifndef __RECORD_DEL_H__
#define __RECORD_DEL_H__

#include "record.h"

#define RECORD_DEL_SPACE	100	//剩余多少空间就删除录像,单位M;删除以天为单位

typedef	struct	_delOldDate
{
	char	dirName[RECORD_FILENAME_LEN];
	struct	_delOldDate	*next;
}DEL_OLD_DATE;

void FiRecStartAutoDelOldFile(void);
int FiRecSetSupportLoopRecordFlag(int flag);
int FiRecGetSupportLoopRecordFlag(void);
int FiRecInitLoopRecordFlag();

int FiRecSetRecordDelSpace(int delSpace);
int FiRecGetRecordDelSpace(void);

#endif //__RECORD_DEL_H__

