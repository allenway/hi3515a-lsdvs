/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此文件实现了DVS数据传输协议
**  参考文档: <<DVS传输协议.doc>> V1.0
**  date           :  2013.12.07
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "debug.h"
#include "flash.h"
#include "malloc.h"
#include "public.h"
#include "netSocket.h"
#include "packData.h"
#include "dataService.h"
#include "messageProtocol.h"
#include "transmitProtocol.h"
#include "sysConfig.h"
#include "fitLspp.h"
#include "linuxFile.h"
#include "fitLspp.h"
#include "writeUpdateFile.h"
#include "linuxFile.h"

#define SEND_CFG_SIZE (4096)

typedef FIRS_PACK_HEAD TRANSMIT_PACK_HEAD;
typedef FIRS_PACK_DATA TRANSMIT_PACK_DATA;

static int	    	s_UpgradeAuthFlag	    = 0;
static int	    	s_UpgradeFileSize	    = 0;
static char	*    	s_UpgradeFileBuf	    = NULL;
static int	    	s_UpgradeFileOffset	    = 0;

//
// 获取数据包长度
//
static int GetTransmitPackLen( TRANSMIT_PACK_DATA *pPackData )
{
	return FirsGetPackLen( pPackData );
}

//
// 生成搜索数据包
//
static void PackTransmitDataPack(	unsigned char	subType, 
                                	unsigned char *	dataBuf, 
                                	unsigned short	dataLen,
                                	unsigned char *	packBuf,
                                	int &        	packLen )
{
	TRANSMIT_PACK_DATA *pTransmitPackData = ( TRANSMIT_PACK_DATA * )packBuf;
	FirsGetDataHead( pTransmitPackData->head );
    
	unsigned int	msgFlag	    = pTransmitPackData->head.msgFlag;
	unsigned int	msgType	    = pTransmitPackData->head.msgType;
	unsigned short	packSn	    = pTransmitPackData->head.packSn;
	unsigned int	packType	= pTransmitPackData->head.packType;
    
	FirsPackDataPack( msgFlag, msgType, packSn, packType, subType,
                        	dataBuf, dataLen, packBuf, packLen );
}

void ClearUpgradeBuf()
{
	ReleaseUpgradeMemory( s_UpgradeFileBuf );
	s_UpgradeAuthFlag	= 0;
	s_UpgradeFileSize	= 0;
	s_UpgradeFileOffset	= 0;
}

//
// 除非升级成功，否则10分钟内不允许重复发送升级请求。
//
void TimerUpgradeRequest()
{
	static int	lastTime	= 0;
	int	    	curTime	    = time( NULL );
    
	if ( curTime > lastTime + 600 )
    {    
    	lastTime = curTime;
    	ClearUpgradeBuf();
    }
}

int DealUpgradeRequest( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int nRet = -1;
	TRANSMIT_PACK_DATA *pTransmitPack = (TRANSMIT_PACK_DATA *)dataBuf;

	if ( pTransmitPack->head.subType == MSG_TRANSMIT_REQUEST )
    {
    	if (GetUpdateFlag() == 1)
        {
        	FiPrint("other update is running, update stop\n");
        	goto reqEnd;
        }

    	int fileSize = 0;
    	memcpy( &fileSize, pTransmitPack->data, sizeof(fileSize) );
    	fileSize = ntohl( fileSize );
    	if ( 0 < fileSize && fileSize <= MAX_UPGRADE_MEMORY )
        {
        	TimerUpgradeRequest();
        	if ( s_UpgradeAuthFlag == 0 )
            {
            	s_UpgradeFileBuf = ( char * )GetUpgradeMemory( sizeof(char) * fileSize );
            	if ( s_UpgradeFileBuf != NULL )
                {
                	s_UpgradeAuthFlag	= 1;
                	s_UpgradeFileSize	= fileSize;
                	s_UpgradeFileOffset	= 0;
                	nRet = 0;
                }
            }
        }
    }

reqEnd:
	unsigned char subType = ( nRet != -1 ) ? MSG_TRANSMIT_RESPONSE : MSG_TRANSMIT_ERROR;
	PackTransmitDataPack( subType, NULL, 0, dataBuf, dataLen );
	return 0;
}

void ShowRecvPercent( int offset, int size )
{
	static int lastPercent = 0;
	int curPercent = offset * 100 / size;
	if ( lastPercent+10 <= curPercent ) 
    {
    	lastPercent = curPercent;
    	printf( "Recv upgrade file %d%% .\r\n", curPercent );    
    }
}

int DealUpgradeFile( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	        = -1;
	TRANSMIT_PACK_DATA *	pTransmitPack	= (TRANSMIT_PACK_DATA *)dataBuf;

    // 未发送升级请求则不允许发送升级文件
	if ( s_UpgradeAuthFlag == 0 )
    {
    	PackTransmitDataPack( MSG_TRANSMIT_ERROR, NULL, 0, dataBuf, dataLen );
    	return 0;
    }
    
	if ( pTransmitPack->head.subType == MSG_TRANSMIT_DATA_SEGMENT
        || pTransmitPack->head.subType == MSG_TRANSMIT_LAST_SEGMENT )
    {
    	static int packCnt = 0;
    	if ( ++packCnt == 1 ) FiPrint( "Start receive upgrade file !\r\n" );
    	int dataLen = ntohs( pTransmitPack->head.len );
    	if ( dataLen <= s_UpgradeFileSize - s_UpgradeFileOffset )
        {
        	char *pFileBuf = s_UpgradeFileBuf + s_UpgradeFileOffset;
        	memcpy( pFileBuf, pTransmitPack->data, dataLen );
        	s_UpgradeFileOffset += dataLen;
            
        	ShowRecvPercent( s_UpgradeFileOffset, s_UpgradeFileSize );
            
        	nRet = 0;
        	if ( pTransmitPack->head.subType == MSG_TRANSMIT_LAST_SEGMENT )
            {
            	FiPrint( "Receive upgrade file %d bytes !\r\n", s_UpgradeFileSize );
            	unsigned char errorNo = UPGRADE_OK;    
            	if ( s_UpgradeFileSize == s_UpgradeFileOffset 
                    && s_UpgradeFileSize > (int)sizeof(UPGRADE_FILE_INFO) )
                {
                    // 文件接收完毕, 开始系统升级.
                    //extern int QuitVideoServiceThread();
                    //QuitVideoServiceThread();
                	errorNo = UpgradeSystem( s_UpgradeFileBuf, s_UpgradeFileSize );
                	if ( errorNo == UPGRADE_OK ) 
                    {
                    	RebootSystem();
                    }                    
                	else
                    {
                        //extern void *DealVideoServiceThread( void *args );
                    	usleep(200000);
                        //DealVideoServiceThread( NULL );
                    }
                }
            	else
                {
                	errorNo = UPGRADE_OTHER_ERROR;
                	FiPrint( "Upgrade file size error !\r\n" );
                }

            	if (s_UpgradeFileBuf != NULL)
                	SetUpdateFlag(0);
            	ClearUpgradeBuf();
                
            	unsigned char subType = MSG_TRANSMIT_RESPONSE;
            	PackTransmitDataPack( subType, &errorNo, sizeof(errorNo), dataBuf, dataLen );
            	return 0;
            }
        }
    	else
        {
        	FiPrint( "Upgrade file size error !\r\n" );
        	if (s_UpgradeFileBuf != NULL)
            	SetUpdateFlag(0);
        	ClearUpgradeBuf();
        }
    }
	else
    {
    	FiPrint( "Upgrade file subType error !\r\n" );
    	if (s_UpgradeFileBuf != NULL)
        	SetUpdateFlag(0);
    }    

	unsigned char subType = ( nRet != -1 ) ? MSG_TRANSMIT_RESPONSE : MSG_TRANSMIT_ERROR;
	PackTransmitDataPack( subType, NULL, 0, dataBuf, dataLen );
  
	return 0;
}

int SendSysConfig( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int sndSock = 0;
	int ret = -1;
	SYSCONFIG *pSysConfig = NULL;
	unsigned char * us = NULL;
	unsigned char * ps = NULL;
	int nSize = sizeof( SYSCONFIG );

	unsigned char * sendBuf = ( unsigned char * )Malloc( SEND_CFG_SIZE );
	if (sendBuf == NULL)
    {
    	FiPrint( "SendSysConfig: Malloc Memory failed !!!\r\n" );
    	ret = -1;
    	goto sndEnd;
    }
    
	pSysConfig = ( SYSCONFIG * )Malloc( sizeof(SYSCONFIG) );
	if (pSysConfig == NULL)
    {
    	FiPrint( "SendSysConfig: Malloc Memory failed !!!\r\n" );
    	ret = -1;
    	goto sndEnd;
    }
	memset(pSysConfig, 0, nSize);

	ret = GetSysConfig(pSysConfig);
	if (ret == -1)
    {
    	ret = -1;
    	goto sndEnd;
    }

	PassData( (char *)pSysConfig, sizeof(*pSysConfig) );

	memcpy(&sndSock, dataBuf+MAX_DATA_BUF_SIZE, sizeof(int));
	if (sndSock <= 0)
    {
    	FiPrint("get 6010 socket error\n");
    	ret = -1;
    	goto sndEnd;
    }

	us = (unsigned char *)pSysConfig;
	for (int i = 0; i<nSize; i+=SEND_CFG_SIZE)
    {
    	if (i >= (nSize-SEND_CFG_SIZE))
        {
        	FiPrint("end: send data, nSize - i = %d, i = %d\n", nSize - i, i);
        	memset(sendBuf, 0, SEND_CFG_SIZE);
        	ps = us + i;
        	memcpy(sendBuf, ps, nSize-i);
        	PackTransmitDataPack( MSG_TRANSMIT_LAST_SEGMENT, sendBuf, nSize-i, dataBuf, dataLen );
        	Writen(sndSock, (char *)dataBuf, dataLen);
        	break;
        }

    	ps = us + i;
    	memcpy(sendBuf, ps, SEND_CFG_SIZE);
    	PackTransmitDataPack( MSG_TRANSMIT_DATA_SEGMENT, sendBuf, SEND_CFG_SIZE, dataBuf, dataLen );
    	Writen(sndSock, (char *)dataBuf, dataLen);
    }

sndEnd:
	Free(pSysConfig);
	Free(sendBuf);

	dataLen = 0;
    
	return ret;
}

//
// 处理函数列表, 根据协议不断扩展.
//
static ID_CMD s_TransmitIdCmd[] = 
{
    { MSG_TRANSMIT_UPGRADE_REQUEST,    	DealUpgradeRequest	}, //请求系统升级
    { MSG_TRANSMIT_UPGRADE_FILE,    	DealUpgradeFile	    }, //接收升级文件	    
    { MSG_TRANSMIT_GET_SYSCONFIGURE,	SendSysConfig	    }, //发送系统配置
};

int DealDataTransmitProcess( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int nRet = -1;
	TRANSMIT_PACK_DATA *pTransmitPack = (TRANSMIT_PACK_DATA *)dataBuf;
	CIdCmd dealTransmit;
	dealTransmit.Init( s_TransmitIdCmd, sizeof(s_TransmitIdCmd)/sizeof(ID_CMD) );
    
	int cmdId = ntohl( pTransmitPack->head.packType );
	ID_CMD_ACTION DealTransmitCmd = dealTransmit.GetCmd( cmdId );
	if ( DealTransmitCmd != NULL )
    	nRet = DealTransmitCmd( dataBuf, dataLen, bufSize );

	if ( nRet < 0 ) FiPrint( "DealTransmitProcess Error !\r\n" );
	return nRet;
}

//
// 对数据包进行合法性检查
//
int CheckDataTransmitProcess( unsigned char *dataBuf, int dataLen, int bufSize, int &offset )
{
	int	            	nRet	        = -1;
	TRANSMIT_PACK_DATA *pTransmitPack	= (TRANSMIT_PACK_DATA *)dataBuf;
	int	            	packLen	        = GetTransmitPackLen( pTransmitPack );
    
	if ( packLen > bufSize )
    {
    	offset = sizeof( TRANSMIT_PACK_HEAD );  // 包长出错, 丢弃包头.
    	FiPrint( "Check Data Transmit Pack Length Error !\r\n" );
    }
	else
	if ( packLen <= dataLen )
    {
    	if ( IsCheckSumOK(dataBuf, packLen) )
        {
        	if ( pTransmitPack->head.msgType == MSG_DATA_TRANSMIT_TYPE )
            {
            	nRet = packLen;
            	offset = 0;
            }
        	else
            {
            	offset = packLen;
            	FiPrint( "Check Data Transmit Type Error !\r\n" );
            }
        }
    	else
        {
        	offset = packLen;
        	FiPrint( "Check Sum Error !\r\n" );
        }
    }
	return nRet;
}

