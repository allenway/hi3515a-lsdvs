#include "debug.h"
#include "thread.h"
#include "avSendThread.h"
#include "sysConfig.h"
#include "lsppApp.h"

static THREAD_MAINTAIN_T g_avSendTm;

void StartAvSendThread()
{
	int ret;
	g_avSendTm.runFlag = 1;
	ret = ThreadCreate( &g_avSendTm.id, DealAVSendThread, NULL );
	if( 0 != ret )
    {
    	FiPrint( "ThreadCreate(DealAVSendThread) error: %s!\r\n", STRERROR_ERRNO );
    }
}

void StopAvSendThread()
{
	g_avSendTm.runFlag = 0;
	ThreadJoin( g_avSendTm.id, NULL );
}

static THREAD_MAINTAIN_T g_netMsgTm;
void StartNetMsgThread()
{
	int ret;
	g_netMsgTm.runFlag = 1;
	ret = ThreadCreate( &g_netMsgTm.id, DealMessageDataThread, NULL );
	if( 0 != ret )
    {
    	FiPrint( "ThreadCreate(DealMessageDataThread) error: %s!\r\n", STRERROR_ERRNO );
    }
}

void StopNetMsgThread()
{
	g_netMsgTm.runFlag = 0;
	ThreadJoin( g_netMsgTm.id, NULL );
}

void LsppAppServiceStart()
{
	StartAvSendThread();
	StartNetMsgThread();
}
void LsppAppServiceSop()
{
	StopAvSendThread();
	StopNetMsgThread();
}

