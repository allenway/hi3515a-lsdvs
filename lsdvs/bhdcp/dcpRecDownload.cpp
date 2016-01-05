/********************************************************************************
**	Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**	author        :  sven
**	version       :  v1.0
**	date           :  2013.10.10
**	description  : 录像下载
********************************************************************************/

#include "debug.h"
#include "netSocket.h"
#include "malloc.h"
#include "thread.h"
#include "linuxFile.h"
#include "dcpRecDownload.h"

static void ReleaseRecDownload( REC_DOWNLOAD_T *pRecDownload )
{
	Close( pRecDownload->socket );    
	Close( pRecDownload->fd );
	Free( pRecDownload );
}

static void *DcpRecDownloadThread( void *args )
{
	int ret;
	int readLen = MAX_RECORD_DATA_SEND_PACKET_SIZE;
	REC_DOWNLOAD_T *pRecDownload = (REC_DOWNLOAD_T *)args;

	ThreadDetach( ThreadSelf() );
    
	SVPrint( "%s start!\r\n", __FUNCTION__ );
	while( 1 )
    {
    	ret = Read( pRecDownload->fd, pRecDownload->sendPack.dataBuf, readLen );
    	if( ret > 0 )
        {
        	pRecDownload->sendPack.packHead.len = ret;            
        	pRecDownload->len = pRecDownload->sendPack.packHead.len 
                                + sizeof(pRecDownload->sendPack.packHead);
            
        	ret = Sendn( pRecDownload->socket, &pRecDownload->sendPack.packHead, pRecDownload->len );
            
        	if( ret != 0 )
            {
            	break; 
            }
        }
    	else
        {
        	break; 
        }
    }
    
	ReleaseRecDownload( pRecDownload );
    
	SVPrint( "%s stop!\r\n", __FUNCTION__ );
    
	return NULL;
}

/**************************************************************************
* fn: 
* socket: 发送的socket
* fd: 录像文件的句柄
***************************************************************************/
int DcpStartRecDownload( int socket, int fd )
{
	int ret = -1;
	pthread_t threadId;
	REC_DOWNLOAD_T *pRecDownload = (REC_DOWNLOAD_T *)Malloc( sizeof(REC_DOWNLOAD_T) );

	if( NULL != pRecDownload )
    {
    	pRecDownload->socket     = socket;        
    	pRecDownload->fd         = fd;
    	Memset( &pRecDownload->sendPack.packHead, 0x00, 
            	sizeof(pRecDownload->sendPack.packHead) );
    	ret = ThreadCreate( &threadId, DcpRecDownloadThread, pRecDownload );
    }

	return ret;
}

