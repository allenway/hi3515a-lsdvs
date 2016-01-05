#include "const.h"
#include "linuxFile.h"
#include "malloc.h"
#include "debug.h"
#include "record.h"
#include "message.h"
#include "timeExchange.h"
#include "dcpSearchRecord.h"
#include "dcpIns.h"
#include "dcpInsLocal.h"

static void PackSearchRecordLocalCmd( uint cmd, ushort index, char *pData,  
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

static void PackSearchRecordLocalCmd( uint cmd, ushort index, char *p1, uint l1,
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

static void PackSearchResult( RECORD_INQUIRE *pRecInq, DCP_SEARCH_RECORD_RESULT_T *pDsrr )
{
	int YY, MM, DD, hh, mm, ss;
	Strncpy( pDsrr->recordName, pRecInq->recFilename, sizeof(pDsrr->recordName) );

	FiTimeUtcToHuman( pRecInq->startTime, &YY, &MM, &DD, &hh, &mm, &ss );
	sprintf( pDsrr->startTime, "%04d-%02d-%02d %02d:%02d:%02d", YY, MM, DD, hh, mm, ss );
    
	FiTimeUtcToHuman( pRecInq->stopTime, &YY, &MM, &DD, &hh, &mm, &ss );
	sprintf( pDsrr->endTime, "%04d-%02d-%02d %02d:%02d:%02d", YY, MM, DD, hh, mm, ss );

	pDsrr->recordLen = pRecInq->recLen;
	pDsrr->recordType= pRecInq->recType;
}

void *DcpSearchRecordThread( void *args )
{
	char *ptr;
	uint len;
	int YY, MM, DD, hh, mm, ss;
	int startTime, stopTime;
	MSG_CMD_T msgCmd;
	RECORD_INQUIRE_LIST *recInquireRret;
	DCP_SEARCH_RECORD_ARGS_T *pDsra;
	DCP_SEARCH_RECORD_RESULT_T dsrr;
	uint dssrSize = sizeof( dsrr );

	if( NULL == args )
    {
    	SVPrint( "error:NULL == args!\r\n" );
    	return NULL;
    }
	pDsra = (DCP_SEARCH_RECORD_ARGS_T *)args;
	sscanf( pDsra->searchCond.startTime, "%04d-%02d-%02d %02d:%02d:%02d",
                                        &YY, &MM, &DD, &hh, &mm, &ss );
	startTime = FiTimeHumanToUtc( YY, MM, DD, hh, mm, ss );    
	sscanf( pDsra->searchCond.endTime, "%04d-%02d-%02d %02d:%02d:%02d",
                                &YY, &MM, &DD, &hh, &mm, &ss );
	stopTime = FiTimeHumanToUtc( YY, MM, DD, hh, mm, ss );
	SVPrint( "DcpSearchRecordThread search startTime(%s),stopTime(%s)!\r\n", pDsra->searchCond.startTime, pDsra->searchCond.endTime );
	recInquireRret = FiRecInquireRecordFile( pDsra->searchCond.channel, 
                    	pDsra->searchCond.type, startTime, stopTime );
	if( NULL == recInquireRret ) // ËÑË÷²»µ½Â¼Ïñ
    {
    	SVPrint( "inquire no record file:NULL == recInquireRret!\r\n" );
    	pDsra->msgHead.len         = 0;
    	pDsra->msgHead.msgType     = SV_MSG_RES_SEARCH_RECORD;
    	PackSearchRecordLocalCmd( DIL_FORWARD, pDsra->index, (char *)&pDsra->msgHead, 
                                        	sizeof(pDsra->msgHead), &msgCmd );
    	MessageSend( MSG_ID_DCP_SIGNAL, (char *)&msgCmd, sizeof(msgCmd) );
    }
	else if( NULL == recInquireRret->head )
    {
    	SVPrint( "inquire no record file:NULL == recInquireRret->head!\r\n" );
    	pDsra->msgHead.len         = 0;
    	pDsra->msgHead.msgType     = SV_MSG_RES_SEARCH_RECORD;
    	PackSearchRecordLocalCmd( DIL_FORWARD, pDsra->index, (char *)&pDsra->msgHead, 
                                                	sizeof(pDsra->msgHead), &msgCmd );
    	MessageSend( MSG_ID_DCP_SIGNAL, (char *)&msgCmd, sizeof(msgCmd) );
    }
	else
    {
    	recInquireRret->cur = recInquireRret->head;
    	ptr = (char *)Malloc( SEARCH_RESULT_PACK_SIZE );
    	if( NULL != ptr )
        {
        	len = 0;
        	while( 1 )
            {
            	if( NULL == recInquireRret->cur )
                {
                	break;
                }
            	PackSearchResult( recInquireRret->cur, &dsrr );
            	Memcpy( ptr + len, &dsrr, dssrSize );
            	len += dssrSize;        
            	recInquireRret->cur = recInquireRret->cur->next;

            	if( dssrSize > (SEARCH_RESULT_PACK_SIZE - len)
                    || NULL == recInquireRret->cur )
                {
                	pDsra->msgHead.len         = len;
                	pDsra->msgHead.msgType     = SV_MSG_RES_SEARCH_RECORD;
                	PackSearchRecordLocalCmd( DIL_FORWARD, pDsra->index, (char *)&pDsra->msgHead, 
                                                	sizeof(pDsra->msgHead), ptr, len, &msgCmd );
                	MessageSend( MSG_ID_DCP_SIGNAL, (char *)&msgCmd, sizeof(msgCmd) );

                	len = 0;
                }
            } // while( 1 )
        	Free( ptr );
        }
    } // else
    
	FiRecFreeInquireRecord( recInquireRret );    
    SVPrint(" %s stop.\r\n", __FUNCTION__);

	return NULL;
}

