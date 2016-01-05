/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 条用mpi 模块的接口,抓拍
********************************************************************************/

#include <unistd.h>
#include "debug.h"
#include "thread.h"
#include "ttypes.h"
#include "snapMpi.h"
#include "message.h"
#include "ipcSnap.h"
#include "fit.h"
#include "public.h"

static THREAD_MAINTAIN_T g_snapAppTm;
static void *SnapAppThread( void *arg )
{
	int ret;
	SNAP_MSG_T snapMsg;
	SVPrint( "SnapAppThread start!\r\n" );

	while( g_snapAppTm.runFlag )
    {
    	ret = MessageRecv( MSG_ID_SNAP, (char *)&snapMsg, sizeof(snapMsg) );
    	if( ret < 0 )
        {
        	sleep(1);
        	continue; 
        }        
    	if( (MAX_JPG_CHN_NUM-1) == snapMsg.channel )
        {
            //ret = IpcSnapAndToPcp( 0, snapMsg.snapType );
        }
    	else
        {        
        	ret = FitSnapMpiGetJpgAndToProcon( snapMsg.channel, snapMsg.snapType ); 
        }
    }
    
	SVPrint( "SnapAppThread stop!\r\n" );
	return NULL;
}

void StartSnapAppThread()
{
	int ret;
	g_snapAppTm.runFlag = 1;
	ret = ThreadCreate( &g_snapAppTm.id, SnapAppThread, NULL );
	if( 0 != ret )
    {
    	g_snapAppTm.runFlag = 0;
    	SVPrint( "error:ThreadCreate:%s!\r\n", STRERROR_ERRNO );
    }    
}

void StopSnapAppThread()
{
	int ret;
	g_snapAppTm.runFlag = 0;
	ret = ThreadJoin( g_snapAppTm.id, NULL );
	if( 0 != ret )
    {
    	SVPrint( "error:ThreadJoin:%s!\r\n", STRERROR_ERRNO );
    }
}


/******************************************************************************
* fn: 给其他线程调用, 发送一条消息给本模块线程,启动抓拍
*********************************************************************************/
int SnapAppMessageSend( int channel, int snapType )
{
	int ret;
	SNAP_MSG_T snapMsg;

	snapMsg.channel     = channel;
	snapMsg.snapType     = snapType;

	ret = MessageSend( MSG_ID_SNAP, (char *)&snapMsg, sizeof(snapMsg) );

	return ret;
}

