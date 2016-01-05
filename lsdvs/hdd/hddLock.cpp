/*
*******************************************************************************
**	Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**	author        :  sven
**	version       :  v1.0
**	date           :  2013.10.10
**	description  : 顶一个使用hdd 的全局互斥锁接口
*******************************************************************************
*/

#include "mutex.h"

static CMutexLockRecursive g_hddMutex;

void HddMutexLock()
{
	g_hddMutex.Lock();
}

void HddMutexUnlock()
{
	g_hddMutex.Unlock();
}



