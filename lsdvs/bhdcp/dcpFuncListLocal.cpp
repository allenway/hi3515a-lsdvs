/********************************************************************************
**	Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**	author        :  sven
**	version       :  v1.0
**	date           :  2013.10.10
**	description  : 处理本地指令的函数清单
********************************************************************************/

#include "const.h"
#include "debug.h"
#include "malloc.h"
#include "linuxFile.h"
#include "dcpSignal.h"
#include "message.h"
#include "dcpFuncListLocal.h"
#include "alarm.h"
#include "dcpFuncList.h"

/***********************************************************************
* 对非法命令的统一处理
*************************************************************************/
int DflIavailableMsgType( MSG_CMD_T *pLocalMsg, CLIENT_CONNECT_T *pClient )
{    
	SVPrint( "inavailable local msg:0x%X!\r\n", pLocalMsg->cmd );        
    
	return 0;    
}


int	DflForward( MSG_CMD_T *pLocalMsg, CLIENT_CONNECT_T *pClient )
{
	uint writeLen, leftLen;

	leftLen	 = pClient->client[pLocalMsg->index].writeBufSize 
               - pClient->client[pLocalMsg->index].writeBufDataSize;
	writeLen = leftLen > pLocalMsg->dataLen ? pLocalMsg->dataLen : leftLen;
	Memcpy( pClient->client[pLocalMsg->index].writeBuf + 
        	pClient->client[pLocalMsg->index].writeBufDataSize, 
        	pLocalMsg->pData, writeLen );
	pClient->client[pLocalMsg->index].writeBufDataSize += writeLen;

	return 0;
}

int DflAlarmUpload( MSG_CMD_T *pLocalMsg, CLIENT_CONNECT_T *pClient )
{
	DCP_HEAD_T msgHead;    
	int i;
	char *temp_buf;

	temp_buf = (char *)Malloc( sizeof( DCP_ALARM_RES_T ) );
    
	if ( NULL != temp_buf )
    {
    	Memcpy( temp_buf, pLocalMsg->pData, sizeof( DCP_ALARM_RES_T ) );
    	Free( pLocalMsg->pData );
    	pLocalMsg->dataLen = sizeof( DCP_HEAD_T ) + sizeof( DCP_ALARM_RES_T );
    	pLocalMsg->pData = (char *)Malloc( pLocalMsg->dataLen );
    	if( NULL != pLocalMsg->pData )
        {
        	PacketClientHead( &msgHead, SV_MSG_RES_ALARM, 0, sizeof( DCP_ALARM_RES_T ), 0 );
        	Memcpy( pLocalMsg->pData, ( char* )&msgHead, sizeof( DCP_HEAD_T ) );
        	Memcpy( pLocalMsg->pData + sizeof( DCP_HEAD_T ), temp_buf, sizeof( DCP_ALARM_RES_T ) );
        	for ( i=1; i<pClient->num; i++ )
            {
            	pLocalMsg->index = i;
            	DflForward( pLocalMsg, pClient );
            }
        }
    	Free( temp_buf );
    }

	return 0;
}

// 硬盘状态异常
int DflHddStatErrReport( MSG_CMD_T *pLocalMsg, CLIENT_CONNECT_T *pClient )
{
	DCP_HEAD_T msgHead;    
	int i;
    DCP_HDD_STAT_ERR_T hddStatErr;

    Memcpy( (void *)&hddStatErr, pLocalMsg->pData, sizeof( DCP_HDD_STAT_ERR_T ) );
    Free( pLocalMsg->pData );
    pLocalMsg->dataLen = sizeof( DCP_HEAD_T ) + sizeof( DCP_HDD_STAT_ERR_T );
    pLocalMsg->pData = (char *)Malloc( pLocalMsg->dataLen );
    if( NULL != pLocalMsg->pData )
    {
        PacketClientHead( &msgHead, SV_MSG_RES_HDD_STAT_ERR, 0, sizeof( DCP_HDD_STAT_ERR_T ), 0 );
        Memcpy( pLocalMsg->pData, ( char* )&msgHead, sizeof( DCP_HEAD_T ) );
        Memcpy( pLocalMsg->pData + sizeof( DCP_HEAD_T ), (void *)&hddStatErr, sizeof( DCP_HDD_STAT_ERR_T ) );
        for ( i=1; i<pClient->num; i++ )
        {
            pLocalMsg->index = i;
            DflForward( pLocalMsg, pClient );
        }
    }

	return 0;
}



int DflParamChangedUpload( MSG_CMD_T *pLocalMsg, CLIENT_CONNECT_T *pClient )
{
	DCP_HEAD_T msgHead;    
	int i;

    pLocalMsg->dataLen = sizeof( DCP_HEAD_T );
    pLocalMsg->pData = (char *)Malloc( pLocalMsg->dataLen );
    if( NULL != pLocalMsg->pData )
    {
        PacketClientHead( &msgHead, SV_MSG_RES_PARAM_CHANGED, 0, 0, 0 );
        Memcpy( pLocalMsg->pData, ( char* )&msgHead, sizeof( DCP_HEAD_T ) );
        for ( i=1; i<pClient->num; i++ )
        {
            pLocalMsg->index = i;
            DflForward( pLocalMsg, pClient );
        }
    }

	return 0;
}

