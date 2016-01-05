/*
*******************************************************************************
**	Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**	author        :  sven
**	version       :  v1.0
**	date           :  2013.10.10
**	description  : ipc 应用
*******************************************************************************
*/
#include "ipcSnap.h"

void InitIpc()
{
	InitIpcSnapPcpFd();
}

void DeinitIpc()
{
	DeinitIpcSnapPcpFd();
}

