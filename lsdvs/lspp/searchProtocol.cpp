#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "debug.h"
#include "sysConfig.h"
#include "netSocket.h"
#include "packData.h"
#include "messageProtocol.h"
#include "searchProtocol.h"
#include "fitLspp.h"
#include "netIp.h"

typedef FIRS_PACK_HEAD SEARCH_PACK_HEAD;
typedef FIRS_PACK_DATA SEARCH_PACK_DATA;

//
// 获取搜索数据包长度
//
static int GetSearchPackLen( SEARCH_PACK_DATA *pPackData )
{
	return FirsGetPackLen( pPackData );
}

//
// 生成搜索数据包
//
static void PackSearchDataPack(	unsigned char	subType, 
                            	unsigned char *	dataBuf, 
                            	unsigned short	dataLen,
                            	unsigned char *	packBuf,
                            	int &        	packLen )
{
	SEARCH_PACK_DATA *pSearchPackData = ( SEARCH_PACK_DATA * )packBuf;
	FirsGetDataHead( pSearchPackData->head );
    
	unsigned int	msgFlag	    = pSearchPackData->head.msgFlag;
	unsigned int	msgType	    = pSearchPackData->head.msgType;
	unsigned short	packSn	    = pSearchPackData->head.packSn;
	unsigned int	packType	= pSearchPackData->head.packType;

	FirsPackDataPack( msgFlag, msgType, packSn, packType, subType,
                    	dataBuf, dataLen, packBuf, packLen );
}
                                
int DealDevSearchProcess( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int nRet = -1;
	SEARCH_PACK_DATA *pSearchPack = (SEARCH_PACK_DATA *)dataBuf;
	if ( pSearchPack->head.subType != MSG_DEV_SEARCH_REQUEST ) return nRet;
    
	SYS_CONFIG_BASE sysBase;
	memset( &sysBase, 0x00, sizeof(sysBase) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_BASE, &sysBase, sizeof(sysBase) );
	if ( nRet == -1 ) return nRet;

	SYS_CONFIG_NETWORK_F sysNetwork = { 0 };
	nRet = GetSysConfig( TYPE_SYS_CONFIG_NETWORK, &sysNetwork, sizeof(sysNetwork) );
	if ( nRet == -1 ) return nRet;
	NetGetIpAddr( (char *)"eth0", sysNetwork.ip, sizeof(sysNetwork.ip) );

	MESSAGE_DEV_SEARCH search = { 0 };
	search.devType = atoi( sysBase.devId );
	memcpy( search.devTypeName, sysBase.devId, sizeof(search.devTypeName) );
	memcpy( search.devName, sysBase.devName, sizeof(search.devName) );
	search.channelNum = htonl( sysBase.channelNum );
    
	memcpy( search.ip, sysNetwork.ip, sizeof(search.ip) );
	memcpy( search.netmask, sysNetwork.netmask, sizeof(search.netmask) );
	memcpy( search.gateway, sysNetwork.gateway, sizeof(search.gateway) );
	memcpy( search.dns, sysNetwork.dns, sizeof(search.dns) );
	memcpy( search.dns2, sysNetwork.dns2, sizeof(search.dns2) );
	memcpy( search.mac, sysNetwork.mac, sizeof(search.mac) );

	search.protocolPort = htons( sysBase.serverPort );
	search.httpPort = htons( atoi(sysNetwork.webPort) );

	PackSearchDataPack( MSG_DEV_SEARCH_RESPONSE, 
                        (unsigned char *)&search, sizeof(search),
                    	dataBuf, dataLen );
	FiPrint( "%s 4， nRet(%d)\r\n", __FUNCTION__, nRet );    
	return nRet;
}

//
// 对搜索数据包进行合法性检查
//
int CheckDevSearchProcess( unsigned char *dataBuf, int dataLen, int bufSize, int &offset )
{
	int	            	nRet	    = -1;
	SEARCH_PACK_DATA *	pSearchPack	= (SEARCH_PACK_DATA *)dataBuf;
	int	            	packLen	    = GetSearchPackLen( pSearchPack );
    
	if ( packLen > bufSize )
    {
    	offset = sizeof( SEARCH_PACK_HEAD );  // 包长出错, 丢弃包头.
    	FiPrint( "Check Search Pack Length Error !\r\n" );
    }
	else
	if ( packLen <= dataLen )
    {
    	if ( IsCheckSumOK(dataBuf, packLen) )
        {
        	if ( pSearchPack->head.msgType == MSG_DEV_SEARCH_TYPE )
            {
            	nRet = packLen;
            	offset = 0;
            }
        	else
            {
            	offset = packLen;
            	FiPrint( "Check Search Type Error !\r\n" );
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

