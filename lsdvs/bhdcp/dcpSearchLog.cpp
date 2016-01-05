#include "const.h"
#include "linuxFile.h"
#include "malloc.h"
#include "debug.h"
#include "log.h"
#include "message.h"
#include "dcpSearchLog.h"
#include "dcpIns.h"
#include "dcpInsLocal.h"

static void PackSearchLogLocalCmd( uint cmd, ushort index, char *pData,  
                                    	uint dataLen, MSG_CMD_T *pMsgCmd )
{
	pMsgCmd->cmd      = cmd;
	pMsgCmd->dataLen = dataLen;
	pMsgCmd->index   = index;
	if( pMsgCmd->dataLen > 0 )
    {
    	pMsgCmd->pData = (char *)Malloc( pMsgCmd->dataLen );
    	if( NULL != pMsgCmd->pData )
        {
        	Memcpy( pMsgCmd->pData, pData, pMsgCmd->dataLen );
        }
    }
}

static void PackSearchLogLocalCmd( uint cmd, ushort index, char *p1, uint l1,
                            	char *p2, uint l2, MSG_CMD_T *pMsgCmd )
{
	uint off;
    
	pMsgCmd->cmd      = cmd;
	pMsgCmd->dataLen = l1 + l2;
	pMsgCmd->index   = index;
	if( pMsgCmd->dataLen > 0 )
    {
    	pMsgCmd->pData = (char *)Malloc( pMsgCmd->dataLen );
    	if( NULL != pMsgCmd->pData )
        {
        	off = 0;
        	if( l1 > 0 )
            {
            	Memcpy( pMsgCmd->pData + off, p1, l1 );
            	off += l1;
            }
        	if( l2 > 0 )
            {
            	Memcpy( pMsgCmd->pData + off, p2, l2 );
            	off += l2;
            }
        }
    }
}

void *DcpSearchLogThread( void *args )
{
	char *ptr;
	uint len;
	MSG_CMD_T msgCmd;
	LP_LOG pLog;
	DCP_SEARCH_LOG_ARGS_T *pDsla;
	uint logItemSize = sizeof( LOG_ITEM_T );
    int itemIndex;

	SVPrint( "%s start!\r\n", __FUNCTION__ );
	if( NULL == args )
    {
    	SVPrint( "error:NULL == args!\r\n" );
    	return NULL;
    }
	pDsla = (DCP_SEARCH_LOG_ARGS_T *)args;
    pLog = (LP_LOG)Malloc( sizeof(LOG_T) );
	if( NULL == pLog ) // ·ÖÅä»º´æÊ§°Ü
    {
    	SVPrint( "Malloc pLog fali!\r\n" );
    	pDsla->msgHead.len         = 0;
    	pDsla->msgHead.msgType     = SV_MSG_RES_GET_LOG;
    	PackSearchLogLocalCmd( DIL_FORWARD, pDsla->index, (char *)&pDsla->msgHead, 
                                        	sizeof(pDsla->msgHead), &msgCmd );
    	MessageSend( MSG_ID_DCP_SIGNAL, (char *)&msgCmd, sizeof(msgCmd) );
        return NULL;
    }

    LogGet(pDsla->getLogCond.logType, pDsla->getLogCond.logLevel, pLog);
	if( 0 == pLog->head.totalItemNum )
    {
    	SVPrint( "get log none: 0 == pLog->head.totalItemNum!\r\n" );
    	pDsla->msgHead.len         = 0;
    	pDsla->msgHead.msgType     = SV_MSG_RES_GET_LOG;
    	PackSearchLogLocalCmd( DIL_FORWARD, pDsla->index, (char *)&pDsla->msgHead, 
                                                	sizeof(pDsla->msgHead), &msgCmd );
    	MessageSend( MSG_ID_DCP_SIGNAL, (char *)&msgCmd, sizeof(msgCmd) );
    }
	else
    {
    	SVPrint( "get log totalItem %d.\r\n", pLog->head.totalItemNum );
        itemIndex = 0;
    	ptr = (char *)Malloc( SEARCH_LOG_PACK_SIZE );
    	if( NULL != ptr )
        {
        	len = 0;
        	while( 1 )
            {
            	if(itemIndex == pLog->head.totalItemNum)
                {
                	break;
                }
            	Memcpy( ptr + len, &pLog->itemArray[itemIndex], logItemSize );
            	len += logItemSize;        
                itemIndex++;

            	if( logItemSize > (SEARCH_LOG_PACK_SIZE - len)
                    || pLog->head.totalItemNum == itemIndex )
                {
                	pDsla->msgHead.len         = len;
                	pDsla->msgHead.msgType     = SV_MSG_RES_GET_LOG;
                	PackSearchLogLocalCmd( DIL_FORWARD, pDsla->index, (char *)&pDsla->msgHead, 
                                                	sizeof(pDsla->msgHead), ptr, len, &msgCmd );
                	MessageSend( MSG_ID_DCP_SIGNAL, (char *)&msgCmd, sizeof(msgCmd) );

                	len = 0;
                }
            }
        	Free( ptr );
        }
        else
        {
            SVPrint("Malloc ptr fail.\r\n");
        }
    }
    
	Free( pLog );    
	SVPrint( "%s stop!\r\n", __FUNCTION__ );

	return NULL;
}

