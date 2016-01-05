/********************************************************************************
**	Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**   author  :  sven
**	version :  v1.0
**	date     : 2013.10.10
**	describe: bhdcp 模块入口
********************************************************************************/

#include "dcpSignal.h"
#include "dcpss.h"

void BhdcpServiceStart()
{
	StartDcpSignalThread();
	StartDcpssThread();
}

void BhdcpServiceStop()
{    
	StopDcpSignalThread();
	StopDcpssThread();
}

