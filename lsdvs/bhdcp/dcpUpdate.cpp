/********************************************************************************
**	Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**	author        :  SVEN
**	version       :  v1.0
**	date           :  2013.10.10
**	description  : dcpupdate, 管理升级连接 
********************************************************************************/

#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
    
#include "const.h"
#include "debug.h"
#include "thread.h"
#include "malloc.h"
#include "message.h"
#include "linuxFile.h"
#include "netSocket.h"
#include "mutex.h"
#include "hton.h"
#include "writeUpdateFile.h"
    
#include "dcpCom.h"
#include "dcpFuncList.h"
#include "dcpUpdate.h"

#include "bhdcp.h"
#include "snapApp.h"
#include "hdd.h"
#include "fit.h"
#include "record.h"
#include "flash.h"
#include "log.h"
#include "reboot.h"
    
static UPDATE_COM_T     g_updateCom;
static UPDATE_PARAM_T   g_updateParam;

static int	   s_UpgradeFileSize     = 0;      //接收到的升级文件大小
static char	*  s_UpgradeFileBuf	     = NULL;   //接收文件缓冲区
static int	   s_UpgradeFileOffset   = 0;      //接收文件缓冲区接收偏移

void DcpUpdateClearUpgradeBuf()
{
	Free( s_UpgradeFileBuf );
	s_UpgradeFileSize	= 0;
	s_UpgradeFileOffset	= 0;
}
static int DcpUpdateInitUpdateCom(UPDATE_COM_T *pClientCom, int socket)
{
    int ret = 0;    

	Memset( pClientCom, 0x00, sizeof(UPDATE_COM_T) );

    pClientCom->readBufSize	 = MAX_MSG_DATA_LEN >> 1;
    pClientCom->writeBufSize = MAX_MSG_DATA_LEN >> 1;
    pClientCom->readBuf	    
        = (char *)Malloc(pClientCom->readBufSize);
    pClientCom->writeBuf	    
        = (char *)Malloc(pClientCom->writeBufSize);
    if( NULL == pClientCom->readBuf 
        || NULL == pClientCom->writeBuf)
    {
        Free( pClientCom->readBuf );                    
        Free( pClientCom->writeBuf );
        ret = -1;
    }                
    else
    {
        pClientCom->socket = socket;
    }
    return ret;
}

static int DcpUpdateDeInitUpdateCom(UPDATE_COM_T *pClientCom)
{
    if(pClientCom)
    {        
        Close(pClientCom->socket);
        pClientCom->socket = 0;
        Free( pClientCom->readBuf ); 
        Free( pClientCom->writeBuf );
    }
	return 0;
}

static void DcpUpdateShowRecvPercent( int offset, int size )
{
	static int lastPercent = 0;
	int curPercent = offset * 100 / size;
	if ( lastPercent+10 <= curPercent ) 
    {
    	lastPercent = curPercent;
    	SVPrint( "Recv upgrade file %d%% .\r\n", curPercent );    
    }
}

static int DcpUpdateRecvClientComData( UPDATE_COM_T *pClientCom )
{
	int ret;
	int toReadLen = pClientCom->readBufSize - pClientCom->readBufDataSize;

	ret = Read( pClientCom->socket, pClientCom->readBuf + pClientCom->readBufDataSize, toReadLen );
	if( ret > 0 )
    {
    	pClientCom->readBufDataSize += ret;
    	ret = pClientCom->readBufDataSize;
    }
	return ret;
}

static int DcpUpdateClientComMsgCheck( UPDATE_COM_T *pClientCom, DCP_HEAD_T *pMsgHead )
{
	int ret = FI_FAIL;
	int mark = CLIENT_MSG_MARK;
	DCP_HEAD_T msgHead;
	unsigned int headSize = sizeof( msgHead );    
	unsigned int left;
	void *pmem;

	pmem = Memmem( pClientCom->readBuf, pClientCom->readBufDataSize, &mark, sizeof(mark) );
	if( NULL != pmem )
    {
    	if( pmem != pClientCom->readBuf )
        {
        	left = pClientCom->readBufDataSize - ((unsigned int)pmem - (unsigned int)pClientCom->readBuf);
        	Memmove( pClientCom->readBuf, pmem, left );
        	pClientCom->readBufDataSize = left;
        }

    	if( pClientCom->readBufDataSize >= headSize )
        {
        	Memcpy( &msgHead, pClientCom->readBuf, headSize );
        	if( msgHead.len <= (pClientCom->readBufSize - headSize) )
            {
            	if( pClientCom->readBufDataSize >= (headSize + msgHead.len) )
                {
                	ret = FI_SUCCESS;
                }
            	else
                {
                	ret = FI_FAIL;
                }
            }
        	else // 出错了
            {
                SVPrint("msgType %x dataLen %d.\r\n", msgHead.msgType, msgHead.len);
            	pClientCom->readBufDataSize = 0;
            	ret = FI_FAIL;
            }
        }
    } // if( NULL != pmem )
	else
    {
    	left = sizeof(mark) - 1; // 可能还没有收完一个mark
    	pmem = pClientCom->readBuf + pClientCom->readBufDataSize - left;
    	Memmove( pClientCom->readBuf, pmem, left );
    	pClientCom->readBufDataSize = left;
    }    
    
	if( FI_SUCCESS == ret ) *pMsgHead = msgHead;
    
	return ret;
}

static int DcpUpdateDealFileData( DCP_HEAD_T *pMsgHead, UPDATE_COM_T *pClientCom )
{
    int nRet = DCP_FILE_ERROR;
    unsigned int headLen = sizeof(DCP_HEAD_T);

    if ( pMsgHead->msgType == SV_MSG_REQ_TXUPDATEFILE
         && (pMsgHead->subType == UPDATE_FILE_DATA_SEGMENT
             || pMsgHead->subType == UPDATE_FILE_LAST_SEGMENT) )
    {
        int dataLen = pMsgHead->len;
        if ( dataLen <= s_UpgradeFileSize - s_UpgradeFileOffset )
        {
            char *pFileBuf = s_UpgradeFileBuf + s_UpgradeFileOffset;
            Memcpy( pFileBuf, pClientCom->readBuf + headLen, dataLen );
            s_UpgradeFileOffset += dataLen;
            
            DcpUpdateShowRecvPercent( s_UpgradeFileOffset, s_UpgradeFileSize );
            
            nRet = DCP_FILE_RECEIVE;
            if ( pMsgHead->subType == UPDATE_FILE_LAST_SEGMENT )
            {
                SVPrint( "Receive upgrade file %d bytes !\r\n", s_UpgradeFileSize );
                nRet = DCP_FILE_COMPLETE;
            }
        }
        else
        {
            SVPrint( "Upgrade size error %d %d %d !\r\n", dataLen, s_UpgradeFileSize, s_UpgradeFileOffset);
        }
    }
    else
    {
        SVPrint( "Upgrade file msgType %x, subType %x error !\r\n", pMsgHead->msgType, pMsgHead->subType);
    }    
    return nRet;
}
static int DcpUpdateHandLeClientComMsg( UPDATE_COM_T *pClientCom )
{
    int check;
	int ret = DCP_FILE_RECEIVE;
	unsigned int leftSize;
	DCP_HEAD_T msgHead = { 0 };
	uint headSize = sizeof( msgHead );
	uint dataLen;
    
	leftSize = pClientCom->readBufDataSize;
	while( leftSize  >= headSize )
    {
    	check = DcpUpdateClientComMsgCheck( pClientCom, &msgHead );        
        // FiPrint2( "leftSize = %d!\r\n", leftSize );
        // FiPrint2( "check = %d!\r\n", check );
    	if( FI_SUCCESS == check )
        {
        	dataLen = msgHead.len;
            ret = DcpUpdateDealFileData(&msgHead, pClientCom );
            
        	pClientCom->readBufDataSize = pClientCom->readBufDataSize - (headSize+dataLen);
        	Memmove( pClientCom->readBuf, pClientCom->readBuf + (headSize+dataLen), pClientCom->readBufDataSize );                
        	if( (DCP_FILE_ERROR == ret) || (DCP_FILE_COMPLETE == ret) ) // 接收文件错或完成接收
            {
                ERRORPRINT("%s ret %x\r\n", __FUNCTION__, ret);
            	break;
            }
        }
        
        // 如果不加判端只赋值，在readBufDataSize不消耗时，会死循环，
        // 收到数据头和不完整数据会出现这种情况.
        if(leftSize > pClientCom->readBufDataSize)
        {
            leftSize = pClientCom->readBufDataSize;
        }
        else
        {
            break;
        }
    }

	return ret;
}

static int DcpUpdateSend( int socket, unsigned char *buf, int len)
{
    int ret;

    ret = SelectWrite( socket, 1 );
    if( ret > 0 )
    {            
        ret = Write(socket, buf, len);
    }
    return ret;
}

static int DcpUpdateSendClientComBack( UPDATE_COM_T *pClientCom )
{
	int ret = 0;

	if( pClientCom->writeBufDataSize> 0 
        && pClientCom->writeBufDataSize <= pClientCom->writeBufSize )
    {
    	ret = SelectWrite( pClientCom->socket, 1 );
    	if( ret > 0 )
        {            
        	ret = Write( pClientCom->socket, pClientCom->writeBuf, pClientCom->writeBufDataSize );
        	if( ret > 0 )
            {
            	pClientCom->writeBufDataSize -= ret;    
            	if( pClientCom->writeBufDataSize > 0 ) // 没有发送完
                {
                	Memmove( pClientCom->writeBuf, pClientCom->writeBuf+ret, pClientCom->writeBufDataSize);
                }                
            }
        	else
            {                
            	SVPrint( "Error: writen ret = %d, %s!\r\n", ret, STRERROR_ERRNO );
                // DcpUpdateDelClientCom( &g_updateCom, pClientCom->socket, 0 );
            }
        }
    }
	else // 异常处理
    {
    	pClientCom->writeBufDataSize = 0;
    }

	return ret;
}

#if 0
static int DcpUpdateSendAck(UPDATE_COM_T *pClientCom, int ackCode)
{
    DCP_HEAD_T msgHead;
    PacketClientHead(&msgHead, SV_MSG_RES_UPDATE, 0, 0, ackCode);    
    DcpUpdateSend( pClientCom->socket, (unsigned char *)&msgHead, sizeof(DCP_HEAD_T) );
    // Memcpy(pClientCom->writeBuf, &msgHead, headLen);    
    // pClientCom->writeBufDataSize = headLen;
    // ret = DcpUpdateSendClientComBack(pClientCom);
}
#endif

static void DcpUpdateSendProgress(UPDATE_COM_T *pClientCom, int percent)
{
    DCP_HEAD_T msgHead;
    int headLen = sizeof(DCP_HEAD_T);
    WRITE_FLASH_PROGRESS_T wfp;
    Memset( &wfp, 0, sizeof(wfp) );

    if( percent > 0 && percent <= 100 )
    {
        SVPrint("%s %d\r\n", __FUNCTION__, percent);

        wfp.percent = percent;
        PacketClientHead(&msgHead, SV_MSG_RES_WRITE_FLASH_PROGRESS, 0, sizeof(wfp), 0);    
        Memcpy(pClientCom->writeBuf, &msgHead, headLen);    
        pClientCom->writeBufDataSize = headLen;
        Memcpy(pClientCom->writeBuf + headLen, (void *)&wfp, sizeof(wfp));
        pClientCom->writeBufDataSize += sizeof(wfp);

        DcpUpdateSendClientComBack(pClientCom);
    }
}

static int DcpUpdateRecvFile(UPDATE_COM_T *pClientCom)
{
    int ret;
    int recvFileStat;

    // 开始接受升级文件数据
    while(1)
    {
        ret = SelectRead(pClientCom->socket, 9000);    
        if(ret > 0) 
        {
            ret = DcpUpdateRecvClientComData(pClientCom); // 接收客户端的数据
            if( ret <= 0 )
            {
                ERRORPRINT("dcpupdate recv wrong\n");
                recvFileStat = DCP_FILE_ERROR;
                break;
            }
            else
            {
                SVPrint( "ql debug ------------rds(%u)!\r\n", pClientCom->readBufDataSize );
                recvFileStat = DcpUpdateHandLeClientComMsg( pClientCom );    // 处理所有接收到的消息
                ret = DcpUpdateSendClientComBack( pClientCom );            // 返回处理结果
                if( (DCP_FILE_ERROR == recvFileStat) || (DCP_FILE_COMPLETE == recvFileStat) ) // 接收文件错或完成接收
                {
                    break;
                }
            }
        }
        else 
        {
            SVPrint("SelectRead ret %d\r\n", ret);
            recvFileStat = DCP_FILE_ERROR;
            break;
        }
        usleep(10);
    }
    // 结束接收升级文件
    return recvFileStat;
}
unsigned short WriteFlashResult2AckResult(int wfresult)
{
    unsigned short ackResult;

    if( wfresult == UPGRADE_OK || wfresult == UPGRADE_SAME_FILE )
    {
        ackResult = ERROR_TYPE_SUCCESSFUL;
    }
    else if( wfresult == MD5_CHECK_ERROR )
    {
        ackResult = ERROR_TYPE_UPDATE_FILE_WRONG_MD5;
    }
    else if( wfresult == FILE_TYPE_ERROR )
    {
        ackResult = ERROR_TYPE_UPDATE_FILE_WRONG_TYPE;
    }
    else if( wfresult == WRITE_FLASH_FAIL )
    {
        ackResult = ERROR_TYPE_UPDATE_WRITE_FLASH;
    }
    else
    {
        ackResult = ERROR_TYPE_OTHER;
    }
    return ackResult;
}

/*****************************************************************
* fn: 写升级文件前调用些函数关闭所有的线程.
*
********************************************************************/
static void StopAllServiceForUpdate()
{
    // BhdcpServiceStop();
    // StopUpdateAppThread();
    // StopUpdateServerThread();
    // StopBroadcastServerThread();
    // StopHyalineThread();
    // AlarmStopService();
    // TestServiceStop();
    // StopSnapAppThread();
    // StopFtpJpgThread();
    // StopNtpAppService();
    // LsppAppServiceSop();
    
    // StopSnapJpgThread();
    // FiHddStopHotplugService();
    // FiRecStopRecordService();
    // FitMpiServiceStop();
    // StopIcmpAppThread();
    // StopDtuAppThread();

    //RecordStopServiceForDel();
}
static void ReStartAllServiceForUpdate()
{
    // BhdcpServiceStart();
    // StopUpdateAppThread();
    // StopUpdateServerThread();
    // StopBroadcastServerThread();
    // StopHyalineThread();
    // AlarmStopService();
    // TestServiceStop();
    // StopSnapAppThread();
    // StopFtpJpgThread();
    // StopNtpAppService();
    // LsppAppServiceStart();
    
    // StopSnapJpgThread();
    // FitMpiServiceStart();
    // FiRecStartRecordService();
    // FiHddStartHotplugService();
    // StopIcmpAppThread();
    // StopDtuAppThread();

    RecordRestartServiceForDel();
}

static THREAD_MAINTAIN_T g_dcpUpdateTm;
static void *DcpUpdateThread( void *arg )
{
	int ret;
    int  myid;
    WRITE_FLASH_MSG_T wfm;
    unsigned short ackResult;
    int recvFileStat = DCP_FILE_RECEIVE;
    UPDATE_PARAM_T *pUpdateParam = (UPDATE_PARAM_T *)arg;
	UPDATE_COM_T *pClientCom = &g_updateCom;
    DCP_HEAD_T msgHead;
    int headLen = sizeof(DCP_HEAD_T);

	myid = ThreadSelf();
	ThreadDetach( myid );

	ColorPrint( COLORYELLOW,"########%s start!\r\n", __FUNCTION__ );

    // LogAdd(0xff, LOG_TYPE_SYSTEM, LOG_LEVEL_INFO, "update start.");
	ret = DcpUpdateInitUpdateCom(pClientCom, pUpdateParam->socket);

    if( ret == 0 )
    {
        StopAllServiceForUpdate();
        s_UpgradeFileBuf = (char *)Malloc( pUpdateParam->fileSize );
        if(s_UpgradeFileBuf)
        {
            // 完成升级请求的确认工作，发送信息给客户，开始传输文件
            PacketClientHead(&msgHead, SV_MSG_RES_UPDATE, 0, 0, ERROR_TYPE_SUCCESSFUL);    
            Memcpy(pClientCom->writeBuf, &msgHead, headLen);
            pClientCom->writeBufDataSize = headLen;
            ret = DcpUpdateSendClientComBack(pClientCom);

            s_UpgradeFileSize = pUpdateParam->fileSize;
            s_UpgradeFileOffset = 0;
            recvFileStat = DcpUpdateRecvFile(pClientCom);
            if( DCP_FILE_COMPLETE == recvFileStat )
            {
                if ( s_UpgradeFileSize == s_UpgradeFileOffset 
                    && s_UpgradeFileSize > (int)sizeof(UPGRADE_FILE_INFO) )
                {
                    SVPrint("%s start writeflash.\r\n", __FUNCTION__);
                    StartUpdateFlashThread();
                    while(1)
                    {
                        if ( MessageRecv( MSG_ID_SYS_UPDATE_PRO, (char *)&wfm, sizeof(wfm) ) >= 0 )
                        {
                            if(wfm.msgType == WRITE_FLASH_MSG_PROGRESS)
                            {
                                DcpUpdateSendProgress(pClientCom, wfm.msgData);
                            }
                            else
                            {
                                break;
                            }
                        }
                        else
                        {
                            sleep(1);
                        }
                    }
                    StopUpdateFlashThread();
                }
                else
                {
                    SVPrint( "Upgrade file size error !\r\n" );
                    wfm.msgType = WRITE_FLASH_MSG_RESULT;
                    wfm.msgData = FILE_SIZE_ERROR;
                }
            }
            else
            {
                SVPrint("recv file error.\r\n");
                wfm.msgType = WRITE_FLASH_MSG_RESULT;
                wfm.msgData = UPGRADE_OTHER_ERROR;
            }
            if(recvFileStat == DCP_FILE_ERROR)
            {
                ackResult = ERROR_TYPE_OTHER;
                PacketClientHead(&msgHead, SV_MSG_RES_UPDATE_RESULT, 0, 0, ackResult);    
            }
            else
            {
                SVPrint(" write flash result %d.\r\n", wfm.msgData);
                ackResult = WriteFlashResult2AckResult(wfm.msgData);
                PacketClientHead(&msgHead, SV_MSG_RES_UPDATE_RESULT, 0, 0, ackResult);    
            }
            Memcpy(pClientCom->writeBuf, &msgHead, headLen);    
            pClientCom->writeBufDataSize = headLen;
            ret = DcpUpdateSendClientComBack(pClientCom);

            DcpUpdateClearUpgradeBuf();
            DcpUpdateDeInitUpdateCom(pClientCom);

            if(wfm.msgData == UPGRADE_OK || wfm.msgData == UPGRADE_SAME_FILE)
            {
                LogAddAndWriteFlash(0xff, LOG_TYPE_SYSTEM, LOG_LEVEL_CRITICAL, "update success, reboot.");
                SVPrint("dcpupdate success.\r\n");
                RebootSendMessage();
            }
            else
            {
                LogAddAndWriteFlash(0xff, LOG_TYPE_SYSTEM, LOG_LEVEL_CRITICAL, "update fail, result : %x.", wfm.msgData);
                SVPrint("dcpupdate fail.\r\n");
                ReStartAllServiceForUpdate();
            }
        }
        else
        {
            SVPrint("dcpupdate malloc fail.\r\n");
            PacketClientHead(&msgHead, SV_MSG_RES_UPDATE, 0, 0, ERROR_TYPE_DEV_MEMORY_NOT_ENOUGH);    
            Memcpy(pClientCom->writeBuf, &msgHead, headLen);    
            pClientCom->writeBufDataSize = headLen;
            ret = DcpUpdateSendClientComBack(pClientCom);

            DcpUpdateDeInitUpdateCom(pClientCom);
            ReStartAllServiceForUpdate();
        }
    }
    else
    {
        ERRORPRINT("dcpupdate DcpUpdateInitUpdateCom fail.\r\n");
        PacketClientHead(&msgHead, SV_MSG_RES_UPDATE, 0, 0, ERROR_TYPE_UPDATE_THREAD_FAIL);    
        DcpUpdateSend(pUpdateParam->socket, (unsigned char *)&msgHead, headLen);
        Close(pUpdateParam->socket);
    }
	ColorPrint(COLORBLUE, "!!!!!!!!!!%s stop!\r\n", __FUNCTION__ );
    g_dcpUpdateTm.runFlag = 0;
	return NULL;
}

int DcpCreateUpdateThread(uint fileSize, int socket )
{
    int ret;
    g_updateParam.fileSize = fileSize;
    g_updateParam.socket = socket;

    g_dcpUpdateTm.runFlag = 1;
    ret = ThreadCreate(&g_dcpUpdateTm.id, DcpUpdateThread, (void *)&g_updateParam);
	if(ret)
    {        
    	g_dcpUpdateTm.runFlag = 0;
    	ERRORPRINT( "error:ThreadCreate:%s\r\n", STRERROR_ERRNO );
    }
    return ret;
    
}

int IsDcpUpdateThreadRun()
{
    return g_dcpUpdateTm.runFlag;
}


static THREAD_MAINTAIN_T g_updateFlashTm;
static void *UpdateFlashThread(void *arg)
{
    unsigned char ret;
    int runUpdateFlag = 0;
    WRITE_FLASH_MSG_T wfm;

	SVPrint( "%s start!\r\n", __FUNCTION__ );
    while(g_updateFlashTm.runFlag)
    {
#if 0
        sleep(1);
        if( progress < 100)
        {
            progress+=10;
            MessageSend(MSG_ID_SYS_UPDATE_PRO, (char *)&progress, sizeof(int));
        }
#endif
        if ( runUpdateFlag == 0 )
        {
            runUpdateFlag = 1;
            ret = UpgradeSystem(s_UpgradeFileBuf, s_UpgradeFileSize);

            wfm.msgType = WRITE_FLASH_MSG_RESULT;
            wfm.msgData = ret;
            MessageSend(MSG_ID_SYS_UPDATE_PRO, (char *)&wfm, sizeof(wfm));
        }
        else
        {
            sleep(1);
        }
    }
	SVPrint( "%s stop!\r\n", __FUNCTION__ );
    return NULL;
}
    
void StartUpdateFlashThread()
{
    int ret;
    g_updateFlashTm.runFlag = 1;
    ret = ThreadCreate( &g_updateFlashTm.id, UpdateFlashThread, NULL );
    if( 0!= ret )
    {        
        g_updateFlashTm.runFlag = 0;
        SVPrint( "error:ThreadCreate:%s\r\n", STRERROR_ERRNO );
    }
}

void StopUpdateFlashThread()
{
    int ret;
    g_updateFlashTm.runFlag = 0;
    ret = ThreadJoin( g_updateFlashTm.id, NULL );
    if( 0 != ret )
    {
        SVPrint( "error:ThreadJoin:%s\r\n", STRERROR_ERRNO );
    }
}

