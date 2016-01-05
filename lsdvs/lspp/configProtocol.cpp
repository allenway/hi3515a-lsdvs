/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此文件实现了DVS传输及配置协议
**  参考文档: <<DVS传输及配置协议.doc>> V1.0
**  date           :  2013.11.11
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "public.h"
#include "rand.h"
#include "md5.h"
#include "malloc.h"
#include "message.h"
#include "netSocket.h"
#include "timeExchange.h"
#include "dataService.h"
#include "sysConfig.h"
#include "packData.h"
#include "lang.h"
#include "strSafe.h"
#include "licence.h"
#include "configManage.h"
#include "configProtocol.h"
#include "messageProtocol.h"
#include "wifiCom.h"
#include "wifiConf.h"
#include "wifiFifo.h"
#include "wifiState.h"
#include "record.h"
#include "recordList.h"
#include "ttypes.h"
#include "realTime.h"

#include "fitLspp.h"
#include "driver.h"
#include "osdApply.h"
#include "ntpApp.h"
#include "paramManage.h"
#include "mympi.h"
#include "record.h"
#include "recordDel.h"
#include "dtuApp.h"
#include "snapSearch.h"
#include "fit.h"
#include "ftpJpg.h"
#include "network.h"
#include "netMac.h"
#include "rtc.h"

typedef FIRS_PACK_HEAD CONFIG_PACK_HEAD;
typedef FIRS_PACK_DATA CONFIG_PACK_DATA;

static CMutexLock s_EmpowermentMutex;

int ConfigProtocolEmpowerment( int type, int *pData )
{
	static int empowermentKeep = 0;
	CMutexLockGuard lock( s_EmpowermentMutex );
	if(DATA_READ == type)
    {
        (*pData) = empowermentKeep;
    }
	else if(DATA_WRITE == type)
    {
    	empowermentKeep = (*pData);
    }
    
	return 0;
}

//
// 获取配置数据包长度: 包头长度+包数据长度+校验长度
//
static int GetCfgPackLen( CONFIG_PACK_DATA *pPackData )
{
	return FirsGetPackLen( pPackData );
}

//
// 生成配置数据包
//
static void PackConfigDataPack(	unsigned char	subType, 
                            	unsigned char *	dataBuf, 
                            	unsigned short	dataLen,
                            	unsigned char *	packBuf,
                            	int &        	packLen )
{
	CONFIG_PACK_DATA *pConfigPackData = ( CONFIG_PACK_DATA * )packBuf;
	FirsGetDataHead( pConfigPackData->head );
    
	unsigned int	msgFlag	    = pConfigPackData->head.msgFlag;
	unsigned int	msgType	    = pConfigPackData->head.msgType;
	unsigned short	packSn	    = pConfigPackData->head.packSn;
	unsigned int	packType	= pConfigPackData->head.packType;

	FirsPackDataPack( msgFlag, msgType, packSn, packType, subType,
                    	dataBuf, dataLen, packBuf, packLen );
}

//
// 响应客户端心跳函数
//
static int DealHeartBeat( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	PackConfigDataPack( MSG_CFG_DATA_RESPONSE, NULL, 0, dataBuf, dataLen );
	return 0;
}

/* ------------------------------------------------------------------------- */

//
// 获取系统版本信息
//
static int GetSysInfo( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int             	authorization     = 0;
	int	            	nRet	        = 0;
	SYS_CONFIG_HEADER	sysHeader;

	memset( &sysHeader, 0x00, sizeof(sysHeader) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_HEADER, &sysHeader, sizeof(sysHeader) );
	if ( nRet != -1 )
    {
    	CONFIG_SYS_INFO info;
        
    	memset( &info, 0x00, sizeof(info) );
    	memcpy( info.configureVersion, sysHeader.configureVersion, sizeof(info.configureVersion) );
    	memcpy( info.softwareVersion, sysHeader.softwareVersion, sizeof(info.softwareVersion) );
    	memcpy( info.hardwareVersion, sysHeader.hardwareVersion, sizeof(info.hardwareVersion) );
    	memcpy( info.serialNo, sysHeader.serialNo, sizeof(info.serialNo) );

    	ConfigProtocolEmpowerment( DATA_READ, &authorization );
    	FiPrint("####2016 auth = %d\n", authorization);
    	info.authorization = htonl(authorization);
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE, 
                            (unsigned char *)&info, sizeof(info),
                        	dataBuf, dataLen );
    }
	else
    {
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

//
// 获取系统基本信息
//
static int GetSysBase( unsigned char *dataBuf, int &dataLen, const int bufSize )
{

	int	            	nRet = 0;
	CONFIG_BASE	    	base;
	SYS_CONFIG_BASE		sysBase;
    
	memset( &base, 0x00, sizeof(base) );
	memset( &sysBase, 0x00, sizeof(sysBase) );
    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_BASE, &sysBase, sizeof(sysBase) );
    
	if ( nRet != -1 )
    {
    	memcpy( base.devName, sysBase.devName, sizeof(base.devName) );
    	memcpy( base.devId,   sysBase.devId,   sizeof(base.devId) );
    	memcpy( base.ntpAddr, sysBase.ntpAddr, sizeof(base.ntpAddr) );
        
    	base.chipNum	        = sysBase.chipNum;
    	base.channelNum	        = REAL_CHANNEL_NUM;
    	base.ptzNum	            = sysBase.ptzNum;
    	base.timeZone	        = sysBase.timeZone;
    	base.year	            = sysBase.year;
    	base.month	            = sysBase.month;
    	base.day	            = sysBase.day;
    	base.hour	            = sysBase.hour;
    	base.minute	            = sysBase.minute;
    	base.second	            = sysBase.second;
    	base.ntpType	        = sysBase.ntpType;
    	base.dsEnableFlag	    = sysBase.dsEnableFlag;
    	base.videoPortInput	    = sysBase.videoPortInput;
    	base.videoPortOutput	= sysBase.videoPortOutput;
    	base.audioPortInput	    = sysBase.audioPortInput;
    	base.audioPortOutput	= sysBase.audioPortOutput;
    	base.alarmPortInput	    = sysBase.alarmPortInput;
    	base.alarmPortOutput	= sysBase.alarmPortOutput;
        
    	base.ntpPort	        = htons( sysBase.ntpPort );
    	base.langID	            = htonl( sysBase.langID );
    	base.videoStandard	    = htonl( sysBase.videoStandard );
    	base.serverPort	        = htons( sysBase.serverPort );
    	base.httpPort	        = htons( sysBase.httpPort );
    	base.talkPort	        = htonl( sysBase.talkPort );
    	base.streamPort	        = htonl( sysBase.streamPort );
    	base.talkVoiceInLevel	= htonl( sysBase.talkVoiceInLevel );
    	base.talkVoiceOutLevel	= htonl( sysBase.talkVoiceOutLevel );
    	base.timeIntZone        = htonl( sysBase.timeIntZone);

        /* 设备号,由MAC地址生成,不写入flash */
    	base.iDevMACNum         = htonl(123456);

        /* 2013-08-28 lpx: Add shopID for Ftp upload, begin */
    	base.shopID             = htonl( sysBase.shopID );
        /* 2013-08-28 lpx: Add shopID for Ftp upload, end */

    	FiPrint( "#################### Get Sys Base  ####################\r\n" );
    	FiPrint( "## timeIntZone %d\r\n",sysBase.timeIntZone );
    	FiPrint( "###############  End Get Sys Base  ####################\r\n" );        
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&base, sizeof(base),
                        	dataBuf, dataLen );
    }
	else
    {
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

//
// 设置系统基本信息
//
static int SetSysBase( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int oldTimeZone;
	int	            	nRet	        = 0;
	CONFIG_BASE	    	base;
	SYS_CONFIG_BASE		sysBase;
	CONFIG_PACK_DATA *	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;

	memset( &base, 0x00, sizeof(base) );
	memset( &sysBase, 0x00, sizeof(sysBase) );

	oldTimeZone = GetTimeZone();
        
	memcpy( (char *)&base, pConfigPackData->data, sizeof(base) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_BASE, &sysBase, sizeof(sysBase) );

	if ( nRet != -1 )
    {
    	memcpy( sysBase.devName, base.devName, sizeof(sysBase.devName) );
    	memcpy( sysBase.devId,   base.devId,   sizeof(sysBase.devId) );
    	memcpy( sysBase.ntpAddr, base.ntpAddr, sizeof(sysBase.ntpAddr) );

    	sysBase.chipNum	            = base.chipNum;
    //	sysBase.channelNum	        = base.channelNum;    	channelNum 不允许设置 !!!
    	sysBase.ptzNum	            = base.ptzNum;
    	sysBase.timeZone	        = base.timeZone;
    	sysBase.year	            = base.year;
    	sysBase.month	            = base.month;
    	sysBase.day	                = base.day;
    	sysBase.hour	            = base.hour;
    	sysBase.minute	            = base.minute;
    	sysBase.second	            = base.second;
    	sysBase.ntpType	            = base.ntpType;
    	sysBase.dsEnableFlag	    = base.dsEnableFlag;
    	sysBase.videoPortInput	    = base.videoPortInput;
    	sysBase.videoPortOutput	    = base.videoPortOutput;
    	sysBase.audioPortInput	    = base.audioPortInput;
    	sysBase.audioPortOutput	    = base.audioPortOutput;
    	sysBase.alarmPortInput	    = base.alarmPortInput;
    	sysBase.alarmPortOutput	    = base.alarmPortOutput;
        
    	sysBase.ntpPort	            = ntohs( base.ntpPort );
    	sysBase.serverPort	        = ntohl( base.serverPort );
    	sysBase.talkPort	        = ntohl( base.talkPort );
    	sysBase.streamPort	        = ntohl( base.streamPort );
    	sysBase.talkVoiceInLevel	= ntohl( base.talkVoiceInLevel );
    	sysBase.talkVoiceOutLevel	= ntohl( base.talkVoiceOutLevel );
    	sysBase.timeIntZone         = ntohl( base.timeIntZone );

        /* 2013-08-28 lpx: Add shopID for Ftp upload, begin */
    	sysBase.shopID	            = ntohl( base.shopID );
        /* 2013-08-28 lpx: Add shopID for Ftp upload, end */

    	int langID = ntohl( base.langID );
    	FiPrint( "Set Language ID %d.\r\n", langID );
    	if ( langID != sysBase.langID )
        {
        	nRet = SetLangID( langID );
        	if ( nRet == 0 ) sysBase.langID = langID;
        	else FiPrint( "Set Language ID %d.\r\n", langID );
        }
        
    	FiPrint( "#################### Set Sys Base  ####################\r\n" );
    	FiPrint( "## timeIntZone %d \r\n",sysBase.timeIntZone);
    	FiPrint( "###############  End Set Sys Base  ####################\r\n" );
    	if ( nRet == 0 )
        {
        	nRet = CfgManageSetTalkbackVoice( sysBase.talkVoiceInLevel, sysBase.talkVoiceOutLevel );
        	if ( nRet == -1 ) FiPrint( "Set Talkback Voice Error.\r\n" );
        }

    	if ( nRet == 0 )
        {
        	int videoStandard = ntohl( base.videoStandard );
        	if ( sysBase.videoStandard != videoStandard )
            {
            	sysBase.videoStandard = videoStandard;
            	RebootSystem();
            }
        	nRet = SetSysConfig( TYPE_SYS_CONFIG_BASE, &sysBase, sizeof(sysBase) );
        	if ( nRet == 0 ) 
            {
//            	nRet = WriteSysConfig();
            }            
        }
    }

	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 恢复系统默认参数
//
static int ResetDefaultParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int nRet = 0;
	ParamSetFactoryConfigure();
	if ( nRet == 0 ) RebootSystem();
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取系统时间
//
static int GetSysTime( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int nRet = 0;
	int year = 0, month = 0, day = 0;
	int hour = 0, minute = 0, second = 0;
                    
	FiTimeUtcToHuman( time(NULL), &year, &month, &day, &hour, &minute, &second );
	if ( nRet >= 0 )
    {
    	char timeData[100] = { 0 };
    	snprintf( timeData, sizeof(timeData),
                "%04d-%02d-%02d %02d:%02d:%02d", 
            	year, month, day, hour, minute, second );        
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)timeData, sizeof(timeData),
                        	dataBuf, dataLen );
    }
	else
    {
    	FiPrint( "GetSysTime failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

//
// 设置系统时间
//
static int SetSysTime( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int nRet = 0;
	int year = 0, month = 0, day = 0;
	int hour = 0, minute = 0, second = 0;

	FiPrint( "####################  Set System Time  ####################\r\n" );                                
	CONFIG_PACK_DATA *pConfigPackData = ( CONFIG_PACK_DATA * )dataBuf;
	char *pData = ( char * )Malloc( pConfigPackData->head.len + 1 );    
	if ( pData != NULL )
    {
    	memset( pData, 0, pConfigPackData->head.len + 1 );
    	memcpy( pData, pConfigPackData->data, pConfigPackData->head.len );
    	sscanf( pData, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second );
    	nRet = RealTimeSetDatetime( year, month, day, hour, minute, second );
    	if( 0 == nRet )
        {
        	nRet = RtcSetTime( year, month, day, hour, minute, second );
        }
    	Free( pData );
    }
	else
    {
    	nRet = -1;
    	FiPrint( "SetSysTime: Malloc memory failed !\r\n" );
    }
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	FiPrint( "################# End Set System Time,   ####################\r\n" );        
	return nRet;
}

//
// 获取网络参数
//
static int GetNetParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	    = 0;
	CONFIG_NETWORK	    	network;
	SYS_CONFIG_NETWORK_F	sysNetwork;
	PARAM_CONFIG_NETWORK 	networkParam;
    
	memset( &network, 0x00, sizeof(network) );
	memset( &sysNetwork, 0x00, sizeof(sysNetwork) );
	nRet = ParamGetNetwork( &networkParam );
	if ( nRet != -1 ) 
    {
    	network.netType = 1;
    	network.dhcpType = networkParam.wired.dhcpEnable;
    	memcpy( network.ip, networkParam.wired.ip, sizeof(network.ip) );
    	memcpy( network.netmask, networkParam.wired.netmask, sizeof(network.netmask) );
    	memcpy( network.gateway, networkParam.wired.gateway, sizeof(network.gateway) );
    	memcpy( network.dns, networkParam.dns.dns1, sizeof(network.dns) );
    	memcpy( network.dns2, networkParam.dns.dns2, sizeof(network.dns2) );
    	memcpy( network.broadAddr, networkParam.wired.broadAddr, sizeof(network.broadAddr) );
    	sprintf( network.webPort, "%u", networkParam.port.webPort );
    	CfgManageGetMacAddr( network.mac );
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&network, sizeof(network),
                        	dataBuf, dataLen );
    }
	else
    {
    	FiPrint( "GetNetParam failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

//
// 设置网络参数
//
static int SetNetParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	            	nRet	        = 0;
	CONFIG_NETWORK		network;
	SYS_CONFIG_NETWORK_F	sysNetwork;
	CONFIG_PACK_DATA *	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;
	PARAM_CONFIG_NETWORK 	networkParam;    
	uint webPort;

	memset( &network, 0x00, sizeof(network) );
	memset( &sysNetwork, 0x00, sizeof(sysNetwork) );

	memcpy( (char *)&network, pConfigPackData->data, sizeof(network) );
    
	nRet = ParamGetNetwork( &networkParam );
	if( 0 == nRet )
    {
    	network.netType = 1;
    	networkParam.wired.dhcpEnable = network.dhcpType;
    	memcpy( networkParam.wired.ip, network.ip, sizeof(network.ip) );
    	memcpy( networkParam.wired.netmask, network.netmask, sizeof(network.netmask) );
    	memcpy( networkParam.wired.gateway, network.gateway, sizeof(network.gateway) );
    	memcpy( networkParam.dns.dns1, network.dns, sizeof(network.dns) );
    	memcpy( networkParam.dns.dns2, network.dns2, sizeof(network.dns2) );
    	memcpy( networkParam.wired.broadAddr, network.broadAddr, sizeof(network.broadAddr) );
    	sscanf( network.webPort, "%u", &webPort );
    	networkParam.port.webPort = webPort;
    	nRet = ParamSetNetwork( &networkParam );
    }
    
	return nRet;
}

//
// 获取3G 设备状态
//
static int GetThreegState( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	                                        = 0;
	int	                    	nRet2	                                        = 0;
	int                     	sigVal	                                    = 0;
	CONFIG_THREEG_DEV_STATE		threegDevState;

    
	memset( &threegDevState, 0x00, sizeof(threegDevState) );
//	nRet2 = FiAtCsqlvl( &sigVal );
//	if (nRet2 < 0)	sigVal = 0;
    
	FiPrint("## nRet2(%d) sigval = %d\n", nRet2, sigVal);
    
	threegDevState.signalState = htonl(sigVal);

	PackConfigDataPack( MSG_CFG_DATA_RESPONSE
            , (unsigned char *)&threegDevState, sizeof(threegDevState)
            , dataBuf, dataLen );
	return nRet;
}

//
// 获取3G 设备参数
//
static int GetThreegParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                        	nRet	                = 0;
	int	                        	nRet2	                = 0;
	char                         	curIp[NET_ADDRSIZE];
	CONFIG_THREEG_NET_PARAM	    	threegNetParam;
	SYS_CONFIG_THREEG_NET_PARAM		sysThreegNetParam;

	memset(curIp, 0, sizeof(curIp));
	memset(&threegNetParam, 0, sizeof(threegNetParam));    
	memset( &sysThreegNetParam, 0x00, sizeof(sysThreegNetParam) );
    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_THREEG_NET_PARAM, &sysThreegNetParam, sizeof(sysThreegNetParam) );
	if ( nRet != -1 )
    {
    	threegNetParam.enableFlag = htonl(sysThreegNetParam.enableFlag);
    	threegNetParam.connectState = htonl(1);
    	nRet2 = FiNetGetPppIpAddr(curIp, NET_ADDRSIZE);
    	if (nRet2 == FI_FAILED)
        {
        	snprintf(curIp, sizeof(curIp), "0.0.0.0");
        	threegNetParam.connectState = htonl(0);
        }
    	snprintf(threegNetParam.wanIP, sizeof(threegNetParam.wanIP), "%s", curIp);

    	FiPrint("##enbflag = %d, ctState = %d, ip = |%s|\n", threegNetParam.enableFlag, threegNetParam.connectState,threegNetParam.wanIP);

    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE
                            , (unsigned char *)&threegNetParam, sizeof(threegNetParam)
                            , dataBuf, dataLen );
    }
	else
    {
    	FiPrint( "GetNetParam failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

//
// 设置3G 设备参数
//
static int SetThreegParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                        	nRet	            = 0;
	CONFIG_THREEG_NET_PARAM	    	threegNetParam;
	SYS_CONFIG_THREEG_NET_PARAM		sysThreegNetParam;
	CONFIG_PACK_DATA *            	pConfigPackData	    = ( CONFIG_PACK_DATA * )dataBuf;

	memset(&threegNetParam, 0, sizeof(threegNetParam));    
	memset( &sysThreegNetParam, 0x00, sizeof(sysThreegNetParam) );

	memcpy( (char *)&threegNetParam, pConfigPackData->data, sizeof(threegNetParam) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_THREEG_NET_PARAM, &sysThreegNetParam, sizeof(sysThreegNetParam) );
	if ( nRet != -1 )
    {
    	sysThreegNetParam.enableFlag = ntohl(threegNetParam.enableFlag);
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_THREEG_NET_PARAM, &sysThreegNetParam, sizeof(sysThreegNetParam) );            
//    	if ( nRet != -1 ) nRet = WriteSysConfig();
    }
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取视频基本参数
//
static int GetVideoParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	        = 0;
	CONFIG_VIDEO_PARAM	    	videoParam;
	CONFIG_PACK_DATA *        	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;
	int	                    	channel	        = *(pConfigPackData->data);
	PARAM_CONFIG_VIDEO_BASE_PARAM vpparam;

	memset( &videoParam, 0, sizeof(videoParam) );    

	nRet = ParamGetVideoBaseParam( channel, &vpparam );
	if ( nRet != -1 )
    {
    	videoParam.channel	    = channel;
    	videoParam.brightness	= htonl( vpparam.brightness );
    	videoParam.contrast	    = htonl( vpparam.contrast );
    	videoParam.hue	        = htonl( vpparam.hue );
    	videoParam.saturation	= htonl( vpparam.saturation );
    	videoParam.exposure	    = htonl( vpparam.exposure );
    }
    
	if ( nRet != -1 )
    {
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&videoParam, sizeof(videoParam),
                        	dataBuf, dataLen );
    }
	else
    {
    	FiPrint( "GetVideoParam failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

//
// 设置视频基本参数
//
static int SetVideoParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	            	nRet	        = 0;
	CONFIG_VIDEO_PARAM	videoParam;
	CONFIG_PACK_DATA *	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;
	PARAM_CONFIG_VIDEO_BASE_PARAM 	vbparam;

    
	memset( &videoParam, 0, sizeof(videoParam) );    
	memcpy( (char *)&videoParam, pConfigPackData->data, sizeof(videoParam) );
    
	int channel             = videoParam.channel;        
	videoParam.brightness	= ntohl( videoParam.brightness );
	videoParam.contrast	    = ntohl( videoParam.contrast );
	videoParam.hue	        = ntohl( videoParam.hue );
	videoParam.saturation	= ntohl( videoParam.saturation );
	videoParam.exposure	    = ntohl( videoParam.exposure );
    
	if ( channel < 0 || MAX_CHANNEL_NUM <= channel ) 
    {
    	nRet = -1;
    }
	else
    {
    	nRet = DriverSetAdVideoCapParam( channel, videoParam.brightness,
                	videoParam.contrast, videoParam.hue, videoParam.saturation );
    	if( 0 == nRet )
        {
        	vbparam.brightness     = videoParam.brightness;            
        	vbparam.contrast     = videoParam.contrast;
        	vbparam.saturation     = videoParam.saturation;
        	vbparam.hue	        = videoParam.hue;
        	ParamSetVideoBaseParam( channel, &vbparam );
        }
    }
#if 0
	if ( nRet == 0 ) nRet = CfgManageSetBrightness( channel, videoParam.brightness );
	if ( nRet == 0 ) nRet = CfgManageSetContrast( channel, videoParam.contrast );        
	if ( nRet == 0 ) nRet = CfgManageSetHue( channel, videoParam.hue );    
	if ( nRet == 0 ) nRet = CfgManageSetSaturation( channel, videoParam.saturation );    
	if ( nRet == 0 ) nRet = CfgManageSetExposure( channel, videoParam.exposure );
#endif
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取用户参数
//
static int GetUserParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	        	nRet	                = 0;    
	CONFIG_USER		user[MAX_USER_NUM];
	SYS_CONFIG_USER	sysUser[MAX_USER_NUM];

    
	memset( &user, 0, sizeof(user) );    
	memset( &sysUser, 0, sizeof(sysUser) );    
    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_USER, &sysUser, sizeof(sysUser) );    
	if ( nRet != -1 )
    {
    	for ( int i = 0; i < MAX_USER_NUM; ++i )
        {
        	memcpy( user[i].userName, sysUser[i].userName, sizeof(user[i].userName) );
        	memset( user[i].pwd, '*', sizeof(user[i].pwd) );  // 密码不允许查询 !!!
        	user[i].level = htonl( sysUser[i].level );
        	user[i].capability = htonl( sysUser[i].capability );
        }
        
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&user, sizeof(user),
                        	dataBuf, dataLen );
    }
	else
    {
    	FiPrint( "GetUserParam failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

//
// 设置用户参数
//
static int SetUserParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	            	nRet	                = 0;
	CONFIG_USER	    	user[MAX_USER_NUM];
	SYS_CONFIG_USER		sysUser[MAX_USER_NUM];
	CONFIG_PACK_DATA *	pConfigPackData	        = ( CONFIG_PACK_DATA * )dataBuf;
    
	memset( user, 0, sizeof(user) );    
	memset( sysUser, 0, sizeof(sysUser) );    

	memcpy( (char *)&user, pConfigPackData->data, sizeof(user) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_USER, &sysUser, sizeof(sysUser) );
	if ( nRet != -1 )
    {
    	for ( int i = 0; i < MAX_USER_NUM; ++i )
        {
        	if ( user[i].userName[0] != '\0' && user[i].pwd[0] != '\0' )
            {
            	memcpy( sysUser[i].userName, user[i].userName, sizeof(sysUser[i].userName) );
                // TODO !!! 对密码进行加密解密
            	memcpy( sysUser[i].pwd, user[i].pwd, sizeof(sysUser[i].pwd) );
            	sysUser[i].level = htonl( user[i].level );
            	sysUser[i].capability = htonl( user[i].capability );
            }
        }
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_USER, &sysUser, sizeof(sysUser) );
//    	if ( nRet == 0 ) nRet = WriteSysConfig();
    }
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取云台控制参数
//
static int GetPtzControl( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	        = 0;    
	CONFIG_PTZ_CONTROL		ptzControl;
	SYS_CONFIG_PTZ_CONTROL	sysPtzControl;
	CONFIG_PACK_DATA *    	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;
	int	                	channel	        = *(pConfigPackData->data);

	memset( &ptzControl, 0, sizeof(ptzControl) );    
	memset( &sysPtzControl, 0, sizeof(sysPtzControl) );    
	nRet = CfgManageGetPtzControl( channel, &sysPtzControl );    
	if ( nRet != -1 )
    {        
    	memcpy( ptzControl.ptzName, sysPtzControl.ptzName, sizeof(ptzControl.ptzName) );
    	ptzControl.channel	    = channel;
    	ptzControl.baudRate	    = htonl( sysPtzControl.baudRate );
    	ptzControl.dataBits	    = sysPtzControl.dataBits;
    	ptzControl.stopBits	    = sysPtzControl.stopBits;
    	ptzControl.parity	    = sysPtzControl.parity;
    	ptzControl.cAddr	    = sysPtzControl.cAddr;
    	ptzControl.cSpeedPan	= sysPtzControl.cSpeedPan;
    	ptzControl.cSpeedTilt	= sysPtzControl.cSpeedTilt;
    	ptzControl.cSpeedZoom	= sysPtzControl.cSpeedZoom;
    	ptzControl.cPos	        = sysPtzControl.cPos;
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&ptzControl, sizeof(ptzControl),
                        	dataBuf, dataLen );
    }
	else
    {
    	FiPrint( "GetPtzControl failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

//
// 设置云台控制参数
//
static int SetPtzControl( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	        = 0;
	CONFIG_PTZ_CONTROL		ptzControl;
	SYS_CONFIG_PTZ_CONTROL	sysPtzControl;
	CONFIG_PACK_DATA *    	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;

	memset( &ptzControl, 0, sizeof(ptzControl) );    
	memset( &sysPtzControl, 0, sizeof(sysPtzControl) );
	memcpy( (char *)&ptzControl, pConfigPackData->data, sizeof(ptzControl) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_PTZ_CONTROL, &sysPtzControl, sizeof(sysPtzControl) );
	if ( nRet != -1 )
    {    
    	int	channel = ptzControl.channel;
    	memcpy( sysPtzControl.ptzName, ptzControl.ptzName, sizeof(ptzControl.ptzName) );        
    	sysPtzControl.baudRate	    = ntohl( ptzControl.baudRate );
    	sysPtzControl.dataBits	    = ptzControl.dataBits;
    	sysPtzControl.stopBits	    = ptzControl.stopBits;
    	sysPtzControl.parity	    = ptzControl.parity;
    	sysPtzControl.cAddr	        = ptzControl.cAddr;
    	sysPtzControl.cSpeedPan	    = ptzControl.cSpeedPan;
    	sysPtzControl.cSpeedTilt	= ptzControl.cSpeedTilt;
    	sysPtzControl.cSpeedZoom	= ptzControl.cSpeedZoom;
    	sysPtzControl.cPos	        = ptzControl.cPos;    
    	nRet = CfgManageSetPtzControl( channel, &sysPtzControl );
    }
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 搜索云台控制协议 
//
static int SearchPtzProtocol( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	        	nRet	= 0;
	SYS_CONFIG_BASE	sysBase;

	memset( &sysBase, 0, sizeof(sysBase) );    

	nRet = GetSysConfig( TYPE_SYS_CONFIG_BASE, &sysBase, sizeof(sysBase) );
	if ( nRet != -1 )
    {
    	int	            	ptzNum	    = sysBase.ptzNum;
    	int	            	ptzSize	    = sizeof(ptzNum) + sizeof(PTZ_NAME) * ptzNum;
    	CONFIG_PTZ_NAME *	pPtzName	= ( CONFIG_PTZ_NAME * )Malloc( ptzSize );
    	if ( pPtzName != NULL )
        {
        	pPtzName->ptzNum = htonl( ptzNum );
        	SYS_CONFIG_PTZ_PROTOCOL ptzProtocol;
        	for ( int i = 0; i < ptzNum; ++i )
            {
            	nRet = GetSysConfig( TYPE_SYS_CONFIG_PTZ_PROTOCOL,
                                    &ptzProtocol, sizeof(ptzProtocol), i );
            	memcpy( &pPtzName->ptzName[i], ptzProtocol.ptzName, sizeof(pPtzName->ptzName[i]) );                
            }
        	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                                (unsigned char *)pPtzName, ptzSize,
                            	dataBuf, dataLen );
        	Free( pPtzName );
        }
    }
	else
    {
    	FiPrint( "SearchPtzProtocol failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

//
// 获取云台控制协议 
//
static int GetPtzProtocol( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	        = 0;
	char	            	ptzName[32];
	CONFIG_PTZ_PROTOCOL		ptzProtocol;
	SYS_CONFIG_PTZ_PROTOCOL	sysPtzProtocol;
	CONFIG_PACK_DATA *    	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;

    
	memset( &ptzName, 0, sizeof(ptzName) );    
	memset( &ptzProtocol, 0, sizeof(ptzProtocol) );    
	memset( &sysPtzProtocol, 0, sizeof(sysPtzProtocol) );    
	memcpy( ptzName, pConfigPackData->data, sizeof(ptzName) );

	nRet = CfgManageGetPtzProtocol( ptzName, &sysPtzProtocol );
	if ( nRet == 0 ) 
    {
    	memcpy( ptzProtocol.ptzName, sysPtzProtocol.ptzName, sizeof(ptzProtocol.ptzName) );
    	ptzProtocol.bWaitResponse	    = sysPtzProtocol.bWaitResponse;
    	ptzProtocol.cSpeedPanDefault	= sysPtzProtocol.cSpeedPanDefault;
    	ptzProtocol.cSpeedPanMin	    = sysPtzProtocol.cSpeedPanMin;
    	ptzProtocol.cSpeedPanMax	    = sysPtzProtocol.cSpeedPanMax;
    	ptzProtocol.cSpeedTiltDefault	= sysPtzProtocol.cSpeedTiltDefault;
    	ptzProtocol.cSpeedTiltMin	    = sysPtzProtocol.cSpeedTiltMin;
    	ptzProtocol.cSpeedTiltMax	    = sysPtzProtocol.cSpeedTiltMax;
    	ptzProtocol.cSpeedZoomDefault	= sysPtzProtocol.cSpeedZoomDefault;
    	ptzProtocol.cSpeedZoomMin	    = sysPtzProtocol.cSpeedZoomMin;
    	ptzProtocol.cSpeedZoomMax	    = sysPtzProtocol.cSpeedZoomMax;
    	ptzProtocol.cPosDefault	        = sysPtzProtocol.cPosDefault;
    	ptzProtocol.cPosMin	            = sysPtzProtocol.cPosMin;
    	ptzProtocol.cPosMax	            = sysPtzProtocol.cPosMax;
    	ptzProtocol.bIsManual	        = sysPtzProtocol.bIsManual;
        
    	for ( int i = 0; i < MAX_CMD_PER_PTZ; ++i )
        	ptzProtocol.cmd[i] = sysPtzProtocol.cmd[i];
    
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&ptzProtocol, sizeof(ptzProtocol),
                        	dataBuf, dataLen );
    }
	else
    {
    	FiPrint( "GetPtzProtocol failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );                    
    }
	return nRet;
}

//
// 设置云台控制协议 
//
static int SetPtzProtocol( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	        = 0;
	CONFIG_PTZ_PROTOCOL		ptzProtocol;
	SYS_CONFIG_PTZ_PROTOCOL	sysPtzProtocol;
	CONFIG_PACK_DATA *    	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;

	memset( &ptzProtocol, 0, sizeof(ptzProtocol) );    
	memset( &sysPtzProtocol, 0, sizeof(sysPtzProtocol) );
    
	memcpy( (char *)&ptzProtocol, pConfigPackData->data, sizeof(ptzProtocol) );
	memcpy( sysPtzProtocol.ptzName, ptzProtocol.ptzName, sizeof(sysPtzProtocol.ptzName) );
	sysPtzProtocol.bWaitResponse	    = ptzProtocol.bWaitResponse;
	sysPtzProtocol.cSpeedPanDefault	    = ptzProtocol.cSpeedPanDefault;
	sysPtzProtocol.cSpeedPanMin	        = ptzProtocol.cSpeedPanMin;
	sysPtzProtocol.cSpeedPanMax	        = ptzProtocol.cSpeedPanMax;
	sysPtzProtocol.cSpeedTiltDefault	= ptzProtocol.cSpeedTiltDefault;
	sysPtzProtocol.cSpeedTiltMin	    = ptzProtocol.cSpeedTiltMin;
	sysPtzProtocol.cSpeedTiltMax	    = ptzProtocol.cSpeedTiltMax;
	sysPtzProtocol.cSpeedZoomDefault	= ptzProtocol.cSpeedZoomDefault;
	sysPtzProtocol.cSpeedZoomMin	    = ptzProtocol.cSpeedZoomMin;
	sysPtzProtocol.cSpeedZoomMax	    = ptzProtocol.cSpeedZoomMax;
	sysPtzProtocol.cPosDefault	        = ptzProtocol.cPosDefault;
	sysPtzProtocol.cPosMin	            = ptzProtocol.cPosMin;
	sysPtzProtocol.cPosMax	            = ptzProtocol.cPosMax;
	sysPtzProtocol.bIsManual	        = ptzProtocol.bIsManual;
    
	for ( int i = 0; i < MAX_CMD_PER_PTZ; ++i )
    	sysPtzProtocol.cmd[i] = ptzProtocol.cmd[i];
        
	nRet = CfgManageSetPtzProtocol( &sysPtzProtocol );
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取NTP配置 
//
static int GetNtpParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	        	nRet	    = 0;
	CONFIG_NTP		ntp;
	unsigned short	ntpPort	    = 0;
	PARAM_CONFIG_NTP   ntpParam;

	memset( &ntp, 0, sizeof(ntp) );    

	nRet = ParamGetNtp( &ntpParam );
	if ( nRet != -1 )
    {
    	ntp.ntpType = ntpParam.enable;
    	ntp.ntpPort = htons( ntpPort );
    	memcpy( ntp.ntpAddr, ntpParam.host, sizeof(ntp.ntpAddr) );
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&ntp, sizeof(ntp),
                        	dataBuf, dataLen );
    }
	else
    {
    	FiPrint( "GetNtpParam failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

//
// 设置NTP配置 
//
static int SetNtpParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	            	nRet	        = 0;
	CONFIG_NTP	    	ntp;
	CONFIG_PACK_DATA *	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;
	unsigned short     	port;
	PARAM_CONFIG_NTP   ntpParam;

	memset( &ntp, 0, sizeof(ntp) );    

	nRet = ParamGetNtp( &ntpParam );
    
	memcpy( (char *)&ntp, pConfigPackData->data, sizeof(ntp) );
	port = ntohs(ntp.ntpPort);
	ntpParam.enable = ntp.ntpType;
	strcpy( ntpParam.host, ntp.ntpAddr );
	nRet = ParamSetNtp( &ntpParam );
	if( 0 == nRet )
    {
    	NtpAppOutMaintain();
    }
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取通道OSD 参数 
//
static int GetOsdParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	            	nRet	        = 0;
	CONFIG_OSD_INFO		osd;
	CONFIG_PACK_DATA *	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;
	int	            	channel	        = *(pConfigPackData->data);
	PARAM_CONFIG_OSD_LOGO osdLogo;
	PARAM_CONFIG_OSD_TIME osdTime;

    
	memset( &osd, 0, sizeof(osd) );    
    
	nRet =  ParamGetOsdLogo( channel, &osdLogo );
	if( 0 == nRet )
    {        
    	nRet =	ParamGetOsdTime( channel, &osdTime );
    }
	if( 0 == nRet )
    {
    	osd.channel	= (unsigned char)channel;
    	memcpy( &osd.osdLogo, &osdLogo, sizeof(osd.osdLogo) );
    	memcpy( &osd.osdTime, &osdTime, sizeof(osd.osdTime) );
        
    	osd.osdTime.xPos	= htons( osd.osdTime.xPos );
    	osd.osdTime.yPos	= htons( osd.osdTime.yPos );
    	osd.osdLogo.xPos	= htons( osd.osdLogo.xPos );
    	osd.osdLogo.yPos	= htons( osd.osdLogo.yPos );
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&osd, sizeof(osd),
                        	dataBuf, dataLen );
    }
	else
    {
    	FiPrint( "GetOsdParam failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

//
// 设置通道OSD 参数 
//
static int SetOsdParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	            	nRet	        = 0;    
	CONFIG_OSD_INFO		osd;
	CONFIG_PACK_DATA *	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;
	PARAM_CONFIG_OSD_LOGO osdLogo;
	PARAM_CONFIG_OSD_TIME osdTime;
	int channel;

    
	memset( &osd, 0, sizeof(osd) );    
	memcpy( (char *)&osd, pConfigPackData->data, sizeof(osd) );
	channel	        = osd.channel;
	osd.osdTime.xPos	= ntohs( osd.osdTime.xPos );
	osd.osdTime.yPos	= ntohs( osd.osdTime.yPos );
	osd.osdLogo.xPos	= ntohs( osd.osdLogo.xPos );
	osd.osdLogo.yPos	= ntohs( osd.osdLogo.yPos );
    #if 0
	strcpy( osdLogo.logo, osd.osdLogo.logo );
	osdLogo.enable = osd.osdLogo.enable;    
	osdLogo.colorRed = osd.osdLogo.colorRed;
	osdLogo.colorGreen= osd.osdLogo.colorGreen;
	osdLogo.colorBlue= osd.osdLogo.colorBlue;
	osdLogo.xPos= osd.osdLogo.xPos;    
	osdLogo.yPos= osd.osdLogo.yPos;
    #endif
	nRet = FitFiOsdSetLogoOsdConfig( channel, &osd.osdLogo);
	if( 0 == nRet )
    {
    	memcpy( &osdLogo, &osd.osdLogo, sizeof(osdLogo) );    
    	nRet =  ParamSetOsdLogo( channel, &osdLogo );
    }    
	if( 0 == nRet )
    {
#if 0
    	osdTime.colorRed = osd.osdTime.colorRed;
    	osdTime.colorGreen = osd.osdTime.colorGreen;
    	osdTime.colorBlue = osd.osdTime.colorBlue;
    	osdTime.xPos = osd.osdTime.xPos;
    	osdTime.yPos = osd.osdTime.yPos;
    	osdTime.enable = osd.osdTime.enable;
#endif	        
    	nRet = FitFiOsdSetTimeOsdConfig( channel, &osd.osdTime );
    	if( 0 == nRet )
        {
        	memcpy( &osdTime, &osd.osdTime, sizeof(osdTime) );    
        	nRet = ParamSetOsdTime( channel, &osdTime );
        }
    }
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取邮件参数 
//
static int GetEmailParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	    = 0;
	CONFIG_EMAIL_PARAM		email;
	SYS_CONFIG_EMAIL_PARAM	sysEmail;

    
	memset( &email, 0, sizeof(email) );    
	memset( &sysEmail, 0, sizeof(sysEmail) );    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_EMAIL_PARAM, &sysEmail, sizeof(sysEmail) );
	if ( nRet != -1 ) 
    {
    	memcpy( (char *)&email, (char *)&sysEmail, sizeof(email) );        
    	email.activeFlag	= htonl( email.activeFlag );
    	email.serverPort	= htonl( email.serverPort );
        
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&email, sizeof(email),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "GetEmailParam failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

//
// 设置邮件参数 
//
static int SetEmailParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	        = 0;    
	CONFIG_EMAIL_PARAM		email;
	SYS_CONFIG_EMAIL_PARAM	sysEmail;
	CONFIG_PACK_DATA *    	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;
    
	memset( &email, 0, sizeof(email) );    
	memset( &sysEmail, 0, sizeof(sysEmail) );    
    
	memcpy( (char *)&email, pConfigPackData->data, sizeof(email) );
	email.activeFlag	= ntohl( email.activeFlag );
	email.serverPort	= ntohl( email.serverPort );
    
	memcpy( (char *)&sysEmail, (char *)&email, sizeof(email) );
	nRet = SetSysConfig( TYPE_SYS_CONFIG_EMAIL_PARAM, &sysEmail, sizeof(sysEmail) );
//	if ( nRet == 0 ) nRet = WriteSysConfig();
    
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取音频编码参数
//
static int GetAudioEncode( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	        = 0;
	CONFIG_AUDIO_ENCODE		audio;
	SYS_CONFIG_AUDIO_ENCODE	sysAudio;
	CONFIG_PACK_DATA *    	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;

	memset( &audio, 0, sizeof(audio) );    
	memset( &sysAudio, 0, sizeof(sysAudio) );    
	int channel = *(pConfigPackData->data);
	nRet = GetSysConfig( TYPE_SYS_CONFIG_AUDIO_ENCODE, &sysAudio, sizeof(sysAudio), channel );
	if ( nRet != -1 ) 
    {
    	audio.sampleRate	= htonl( sysAudio.sampleRate );
    	audio.bitWidth	    = sysAudio.bitWidth;
    	audio.encodeType	= sysAudio.encodeType;
    	audio.amrMode	    = sysAudio.amrMode;
    	audio.amrFormat	    = sysAudio.amrFormat;        
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&audio, sizeof(audio),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "GetAudioEncode failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

//
// 设置音频编码参数
//
static int SetAudioEncode( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	        = 0;    
	CONFIG_AUDIO_ENCODE		audio;
	SYS_CONFIG_AUDIO_ENCODE	sysAudio;
	CONFIG_PACK_DATA *    	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;

	memset( &audio, 0, sizeof(audio) );    
	memset( &sysAudio, 0, sizeof(sysAudio) );
    
	memcpy( (char *)&audio, pConfigPackData->data, sizeof(audio) );
	int channel	        = audio.channel;
	sysAudio.sampleRate	= ntohl( audio.sampleRate );
	sysAudio.bitWidth	= audio.bitWidth;
	sysAudio.encodeType	= audio.encodeType;
	sysAudio.amrMode	= audio.amrMode;
	sysAudio.amrFormat	= audio.amrFormat;
    
	memcpy( (char *)&sysAudio, (char *)&audio, sizeof(audio) );
	nRet = SetSysConfig( TYPE_SYS_CONFIG_AUDIO_ENCODE, &sysAudio, sizeof(sysAudio), channel );
//	if ( nRet == 0 ) nRet = WriteSysConfig();
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取视频编码参数
//
static int GetVideoEncode( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	        = 0;
	CONFIG_VIDEO_ENCODE		video;
	CONFIG_PACK_DATA *    	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;
	int	                	channel	        = *(pConfigPackData->data);
	int	                	chType	        = *(pConfigPackData->data + 1);
	PARAM_CONFIG_VIDEO_ENCODE vencParam;    
	PARAM_CONFIG_VIDEO_ENCODE_PUBLIC vencParamPub;

	memset( &video, 0, sizeof(video) );    

	video.channel	= channel;
	video.chType	= chType;    
    
	nRet = ParamGetVideoEncodePublic( &vencParamPub );
	if( 0 == nRet && 0 == video.chType ) // 主码流
    {
    	nRet = ParamGetVideoEncode( video.channel, &vencParam );
    }
	else if( 0 == nRet ) // 从码流
    {
    	nRet = ParamGetVideoEncodeSlave( video.channel, &vencParam );
    }    
	if ( nRet != -1 ) 
    {        
    	video.videoStandard	    = htonl( vencParamPub.videoStandard );
    	if(vencParam.resolution >= 3 ) vencParam.resolution--;
    	video.resolution	    = htonl( vencParam.resolution );
    	video.bitrateType	    = htonl( vencParam.bitrateType );
    	video.quality	        = htonl( vencParam.bitrate);
    	video.frameRate	        = htonl( vencParam.frameRate );
    	video.iFrameInterval	= htonl( vencParam.iFrameInterval );
    	video.preferFrame	    = htonl( vencParam.preferFrame );
        //video.qp	            = htonl( vencParam.qp );        
    	video.encodeType	    = htonl( vencParam.encodeType );
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&video, sizeof(video),
                        	dataBuf, dataLen );
    	return 0;
    }    

	FiPrint( "GetVideoEncode failed !\r\n" );
	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );    
	return nRet;
}

//
// 设置视频编码参数
//
static int SetVideoEncode( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	        = -1;    
	CONFIG_VIDEO_ENCODE		video;
	CONFIG_PACK_DATA *    	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;

	PARAM_CONFIG_VIDEO_ENCODE vencParamOrg;    
	PARAM_CONFIG_VIDEO_ENCODE vencParam;    
	int channel;
	int chType;

    
	memset( &video, 0, sizeof(video) );    
	memcpy( (char *)&video, pConfigPackData->data, sizeof(video) );
	channel	= video.channel;
	chType	= video.chType;

    //vencParamPub.videoStandard	= ntohl( video.videoStandard );
	vencParam.resolution	    = ntohl( video.resolution );    
	if(vencParam.resolution >= 2 ) vencParam.resolution++;
	vencParam.bitrateType	    = ntohl( video.bitrateType );
	vencParam.bitrate	        = ntohl( video.quality );
	vencParam.frameRate	        = ntohl( video.frameRate );
	vencParam.iFrameInterval	= ntohl( video.iFrameInterval );
	vencParam.preferFrame	    = ntohl( video.preferFrame );
    //vencParam.qp	        = htonl( video.qp );        
	vencParam.encodeType	    = ntohl( video.encodeType );

	if( 0 == chType ) // 主码流
    {
    	nRet = ParamGetVideoEncode( video.channel, &vencParamOrg );
    	if( 0 == nRet )
        {
            //nRet = MympiSetLevel( channel, int val );
        	nRet = FitMympiSetBitrateType( channel, vencParam.bitrateType );
        	if( 0 == nRet )
            {
            	nRet = FitMympiSetBitrate( channel, vencParam.bitrate );
            }
        	if( 0 == nRet )
            {
            	nRet = FitMympiSetFramerate( channel, vencParam.frameRate );
            }
        	if( 0 == nRet )
            {
            	nRet = FitMympiSetIframeInterval( channel, vencParam.iFrameInterval );
            }
            
        	if( 0 == nRet && vencParamOrg.resolution != vencParam.resolution )
            {
            	nRet = FitMympiSetResolution();
            }
        	if( 0 == nRet )
            {
            	nRet = ParamSetVideoEncode( video.channel, &vencParam );
            }
        }        
    }
	else // 从码流
    {
    	nRet = ParamSetVideoEncodeSlave( video.channel, &vencParam );
    }
    
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取通道参数
//
static int GetChannelParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	        = 0;
	CONFIG_CHANNEL_PARAM		channelParam;
	SYS_CONFIG_CHANNEL_PARAM	sysChannelParam;
	CONFIG_PACK_DATA *        	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;
    
	memset( &channelParam, 0, sizeof(channelParam) );    
	memset( &sysChannelParam, 0, sizeof(sysChannelParam) );    
    
	int channel = *(pConfigPackData->data);
	nRet = GetSysConfig( TYPE_SYS_CONFIG_CHANNEL_PARAM,
                        &sysChannelParam, sizeof(sysChannelParam), channel );
	if ( nRet != -1 ) 
    {
    	memcpy( channelParam.channelName,
            	sysChannelParam.channelName,
            	sizeof(channelParam.channelName) );
    	channelParam.voiceInput  = htonl( sysChannelParam.voiceInput );
    	channelParam.voiceOutput = htonl( sysChannelParam.voiceOutput );
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&channelParam, sizeof(channelParam),
                        	dataBuf, dataLen );
    }
	else
    {
    	FiPrint( "GetChannelParam failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

//
// 设置通道参数
//
static int SetChannelParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	        = 0;    
	CONFIG_CHANNEL_PARAM		channelParam;
	SYS_CONFIG_CHANNEL_PARAM	sysChannelParam;
	CONFIG_PACK_DATA *        	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;
    
	memset( &channelParam, 0, sizeof(channelParam) );    
	memset( &sysChannelParam, 0, sizeof(sysChannelParam) );    
	memcpy( (char *)&channelParam, pConfigPackData->data, sizeof(channelParam) );
	int channel = channelParam.channel;
	memcpy( sysChannelParam.channelName,
        	channelParam.channelName,
        	sizeof(sysChannelParam.channelName) );
	sysChannelParam.voiceInput	= ntohl( channelParam.voiceInput );
	sysChannelParam.voiceOutput	= ntohl( channelParam.voiceOutput );
	nRet = SetSysConfig( TYPE_SYS_CONFIG_CHANNEL_PARAM,
                        &sysChannelParam, sizeof(sysChannelParam), channel );
//	if ( nRet == 0 ) nRet = WriteSysConfig();
    
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取自动维护参数
//
static int GetAutoMaintain( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                        	nRet	        = 0;
	CONFIG_AUTO_MAINTAIN_PARAM		autoMaintain;
	SYS_CONFIG_AUTO_MAINTAIN_PARAM	sysAutoMaintain;

	memset( &autoMaintain, 0, sizeof(autoMaintain) );    
	memset( &sysAutoMaintain, 0, sizeof(sysAutoMaintain) );    
    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_AUTO_MAINTAIN_PARAM,
                        &sysAutoMaintain, sizeof(sysAutoMaintain) );
	if ( nRet != -1 ) 
    {
    	autoMaintain.rebootFlag	    = sysAutoMaintain.rebootFlag;
    	autoMaintain.rebootDate	    = htonl( sysAutoMaintain.rebootDate );
    	autoMaintain.rebootTime	    = htonl( sysAutoMaintain.rebootTime );
    	autoMaintain.delFileFlag	= htonl( sysAutoMaintain.delFileFlag );
    	autoMaintain.dayNumber	    = htonl( sysAutoMaintain.dayNumber );
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&autoMaintain, sizeof(autoMaintain),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "GetAutoMaintain failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
    
	return nRet;
}

//
// 设置自动维护参数
//
static int SetAutoMaintain( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                        	nRet	        = 0;    
	CONFIG_AUTO_MAINTAIN_PARAM		autoMaintain;
	SYS_CONFIG_AUTO_MAINTAIN_PARAM	sysAutoMaintain;
	CONFIG_PACK_DATA *            	pConfigPackData = ( CONFIG_PACK_DATA * )dataBuf;

	memset( &autoMaintain, 0, sizeof(autoMaintain) );    
	memset( &sysAutoMaintain, 0, sizeof(sysAutoMaintain) );    
    
	memcpy( (char *)&autoMaintain, pConfigPackData->data, sizeof(autoMaintain) );
	sysAutoMaintain.rebootFlag	    = autoMaintain.rebootFlag;
	sysAutoMaintain.rebootDate	    = ntohl( autoMaintain.rebootDate );
	sysAutoMaintain.rebootTime	    = ntohl( autoMaintain.rebootTime );    
	sysAutoMaintain.delFileFlag	    = ntohl( autoMaintain.delFileFlag );
	sysAutoMaintain.dayNumber	    = ntohl( autoMaintain.dayNumber );

	unsigned char	flag	= sysAutoMaintain.rebootFlag;
	unsigned int	date	= sysAutoMaintain.rebootDate;
	unsigned int	time	= sysAutoMaintain.rebootTime;
	nRet = CfgManageAutoMaintainReboot( flag, date, time );
	if ( nRet == 0 ) nRet = SetSysConfig( TYPE_SYS_CONFIG_AUTO_MAINTAIN_PARAM,
                                        &sysAutoMaintain, sizeof(sysAutoMaintain) );
//	if ( nRet == 0 ) nRet = WriteSysConfig();
    
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取录像参数
//
static int GetRecordParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	        = 0;
	CONFIG_RECORD_PARAM		record;        
	SYS_CONFIG_RECORD_PARAM sysRecord;
	CONFIG_PACK_DATA *    	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;
	int                 	i, j;
	unsigned char          	channel;
	PARAM_CONFIG_RECORD_PUBLIC 	recParamPub;
	PARAM_CONFIG_RECORD_PARAM 	recParam;

	memset( &record, 0, sizeof(record) );    
	memset( &sysRecord, 0, sizeof(sysRecord) ); 

	channel = *(pConfigPackData->data);        
    
	nRet = ParamGetRecordPublic( &recParamPub );
	if( 0 == nRet )
    {
    	nRet = ParamGetRecordParam( channel, &recParam);
    }
	if ( nRet != -1 ) 
    {
    	record.channel	            = channel;
    	memcpy( &record.recPublic, &recParamPub, sizeof(record.recPublic) );
    	record.recHand.recHand	    = recParam.recHand.recFlag;
    	for( i = 0; i < MAX_WEEK_DAY; ++i )
        {
        	record.recTimer.timerWeek[i].enableFlag = recParam.recTimer.day[i].enableFlag;
        	for( j = 0; j < MAX_TIME_SEG_NUM; ++j )
            {
            	record.recTimer.timerWeek[i].timeSeg[j].startHour 
                            = recParam.recTimer.day[i].timeSeg[j].timeStart.hour;
            	record.recTimer.timerWeek[i].timeSeg[j].startMinute
                            = recParam.recTimer.day[i].timeSeg[j].timeStart.minute;
            	record.recTimer.timerWeek[i].timeSeg[j].startSecond
                            = recParam.recTimer.day[i].timeSeg[j].timeStart.second;
            	record.recTimer.timerWeek[i].timeSeg[j].stopHour 
                            = recParam.recTimer.day[i].timeSeg[j].timeEnd.hour;
            	record.recTimer.timerWeek[i].timeSeg[j].stopMinute
                            = recParam.recTimer.day[i].timeSeg[j].timeEnd.minute;
            	record.recTimer.timerWeek[i].timeSeg[j].stopSecond
                            = recParam.recTimer.day[i].timeSeg[j].timeEnd.second;                
            }
        }
    	record.recPublic.delSpace	= htonl( sysRecord.recPublic.delSpace );
    	record.recPublic.delSize	= htonl( sysRecord.recPublic.delSize );
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&record, sizeof(record),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "GetRecordParam failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

//
// 设置录像参数
//
static int SetRecordParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	        = 0;    
	CONFIG_RECORD_PARAM		record;
	CONFIG_PACK_DATA *    	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;
	unsigned char         	channel;
	PARAM_CONFIG_RECORD_PUBLIC 	recParamPub;
	PARAM_CONFIG_RECORD_PARAM 	recParam;
	int i, j;
    
	memset( &record, 0, sizeof(record) );    
	memcpy( (char *)&record, pConfigPackData->data, sizeof(record) );
	record.recPublic.delSpace	= ntohl( record.recPublic.delSpace );
	record.recPublic.delSize	= ntohl( record.recPublic.delSize );
	channel = record.channel;

	memcpy( &recParamPub, &record.recPublic, sizeof(record.recPublic) );
	recParam.recHand.recFlag = record.recHand.recHand;
	for( i = 0; i < MAX_WEEK_DAY; ++i )
    {
         recParam.recTimer.day[i].enableFlag = record.recTimer.timerWeek[i].enableFlag;
    	for( j = 0; j < MAX_TIME_SEG_NUM; ++j )
        {
        	recParam.recTimer.day[i].timeSeg[j].timeStart.hour
                = record.recTimer.timerWeek[i].timeSeg[j].startHour;
        	recParam.recTimer.day[i].timeSeg[j].timeStart.minute
                = record.recTimer.timerWeek[i].timeSeg[j].startMinute;
        	recParam.recTimer.day[i].timeSeg[j].timeStart.second
                = record.recTimer.timerWeek[i].timeSeg[j].startSecond;
        	recParam.recTimer.day[i].timeSeg[j].timeEnd.hour
                = record.recTimer.timerWeek[i].timeSeg[j].stopHour;
        	recParam.recTimer.day[i].timeSeg[j].timeEnd.minute
                = record.recTimer.timerWeek[i].timeSeg[j].stopMinute;
        	recParam.recTimer.day[i].timeSeg[j].timeEnd.second
                = record.recTimer.timerWeek[i].timeSeg[j].stopSecond;                
        }
    	nRet = FiRecSetTimerRecordPolicy( channel, i, &recParam );
    	if( 0 == nRet )
        {
        	nRet = FiRecSetHandRecord( channel, &recParam );
        }
    	if( 0 == nRet )
        {
        	nRet = FiRecSetSupportLoopRecordFlag( recParamPub.loopRecord );
        }
    }
	if( 0 == nRet )
    {
    	nRet = ParamSetRecordPublic( &recParamPub );
    }
	if( 0 == nRet )
    {
    	nRet = ParamSetRecordParam( channel, &recParam);
    }
    
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

extern int FiPtzExecCmd( int channel, char *pCmd, int param );    
static int SetPtzCmd( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	            	nRet	        = 0;    
	CONFIG_PTZ_CMD		ptzCmd;
	CONFIG_PACK_DATA *	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;

    
	memset( &ptzCmd, 0, sizeof(ptzCmd) );    
    
	memcpy( (char *)&ptzCmd, pConfigPackData->data, sizeof(ptzCmd) );
	nRet = FiPtzExecCmd( (int)ptzCmd.channel, ptzCmd.cmd, ntohl(ptzCmd.param) );
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

static int GetSdCardInfo( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	            	nRet	    = 0;    
	CONFIG_SD_CARD_INFO	sdInfo;
	int	            	existFlag	= 0;

    
	memset( &sdInfo, 0, sizeof(sdInfo) );    
	nRet = CfgManageGetSdExistFlag( &existFlag );    
	if ( nRet == 0 )
    {
    	sdInfo.enalbeFlag = (unsigned char)existFlag;
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&sdInfo, sizeof(sdInfo),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "GetSdCardInfo failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

static int GetUsbHdInfo( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	            	nRet	    = 0;    
	CONFIG_USB_HD_INFO	usbInfo;
	int             	existFlag     = 0;

    
	memset( &usbInfo, 0, sizeof(usbInfo) );    
	nRet = CfgManageGetUsbExistFlag( &existFlag );
	if ( nRet == 0 )
    {
    	usbInfo.enalbeFlag = (unsigned char)existFlag;
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&usbInfo, sizeof(usbInfo),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "GetUsbHdInfo failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

//
// 获取告警参数
//
static int GetAlarmParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	    = 0;
	CONFIG_ALARM_PARAM		alarm;        
	SYS_CONFIG_ALARM_PARAM	sysAlarm;

    
	memset( &alarm, 0, sizeof(alarm) );    
	memset( &sysAlarm, 0, sizeof(sysAlarm) );    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_ALARM_PARAM, &sysAlarm, sizeof(sysAlarm) );
	if ( nRet != -1 ) 
    {
    	alarm.alarmInId	            = sysAlarm.alarmInId;
    	alarm.enalbeFlag	        = sysAlarm.enalbeFlag;
    	alarm.type	                = sysAlarm.type;
    	alarm.alarmInDetectTimer	= sysAlarm.alarmInDetectTimer;
    	alarm.trigger	            = sysAlarm.trigger;
    	memcpy( alarm.alarmInName, sysAlarm.alarmInName, sizeof(alarm.alarmInName) );        
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&alarm, sizeof(alarm),
                        	dataBuf, dataLen );
    }
	else
    {
    	FiPrint( "GetAlarmParam failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

//
// 设置告警参数
//    
static int SetAlarmParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	        = 0;    
	CONFIG_ALARM_PARAM		alarm;
	SYS_CONFIG_ALARM_PARAM	sysAlarm;
	CONFIG_PACK_DATA *    	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;

	memset( &alarm, 0, sizeof(alarm) );    
	memset( &sysAlarm, 0, sizeof(sysAlarm) );

	memcpy( (char *)&alarm, pConfigPackData->data, sizeof(alarm) );    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_ALARM_PARAM, &sysAlarm, sizeof(sysAlarm) );
	if ( nRet != -1 )
    {
    	if ( memcmp(&alarm, &sysAlarm, sizeof(alarm)) )
        {
        	sysAlarm.alarmInId	        = alarm.alarmInId;
        	sysAlarm.enalbeFlag	        = alarm.enalbeFlag;
        	sysAlarm.type	            = alarm.type;
        	sysAlarm.alarmInDetectTimer	= alarm.alarmInDetectTimer;
        	sysAlarm.trigger	        = alarm.trigger;
        	memcpy( sysAlarm.alarmInName, alarm.alarmInName, sizeof(sysAlarm.alarmInName) );        
        	nRet = SetSysConfig( TYPE_SYS_CONFIG_ALARM_PARAM, &sysAlarm, sizeof(sysAlarm) );
        	if ( nRet == 0 )
            {
                
        //
        // TODO !!!
        //
        ///    	SetAlarmType( sysAlarm.type );
//            	nRet = WriteSysConfig();
            }
        }
    }
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取RS232参数
//
static int GetRs232Param( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	    = 0;    
	CONFIG_RS232_PARAM		rs232;        
	SYS_CONFIG_RS232_PARAM	sysRs232;

	memset( &rs232, 0, sizeof(rs232) );    
	memset( &sysRs232, 0, sizeof(sysRs232) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_RS232_PARAM, &sysRs232, sizeof(sysRs232) );
	if ( nRet != -1 ) 
    {
    	rs232.baudRate	= htonl( sysRs232.baudRate );
    	rs232.dataBits	= sysRs232.dataBits;
    	rs232.stopBits	= sysRs232.stopBits;
    	rs232.parity	= sysRs232.parity;
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&rs232, sizeof(rs232),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "GetRs232Param failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

//
// 设置Rs232参数
//
static int SetRs232Param( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	        = 0;    
	CONFIG_RS232_PARAM		rs232;
	SYS_CONFIG_RS232_PARAM	sysRs232;
	CONFIG_PACK_DATA *    	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;

	memset( &rs232, 0, sizeof(rs232) );    
	memset( &sysRs232, 0, sizeof(sysRs232) );
	memcpy( (char *)&rs232, pConfigPackData->data, sizeof(rs232) );    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_RS232_PARAM, &sysRs232, sizeof(sysRs232) );
	if ( nRet != -1 ) 
    {
    	sysRs232.baudRate	= ntohl( rs232.baudRate );
    	sysRs232.dataBits	= rs232.dataBits;
    	sysRs232.stopBits	= rs232.stopBits;
    	sysRs232.parity	    = rs232.parity;
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_RS232_PARAM, &sysRs232, sizeof(sysRs232) );
//    	if ( nRet == 0 ) nRet = WriteSysConfig();
    }        

	if ( nRet != 0 ) FiPrint( "SetRs232Param failed !\r\n" );
    
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
    
	return nRet;
}

//
// 获取RS485参数
//
static int GetRs485Param( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	    = 0;    
	CONFIG_RS485_PARAM		rs485;        
	SYS_CONFIG_RS485_PARAM	sysRs485;

	memset( &rs485, 0, sizeof(rs485) );    
	memset( &sysRs485, 0, sizeof(sysRs485) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_RS485_PARAM, &sysRs485, sizeof(sysRs485) );
	if ( nRet != -1 ) 
    {
    	rs485.baudRate	= htonl( sysRs485.baudRate );
    	rs485.dataBits	= sysRs485.dataBits;
    	rs485.stopBits	= sysRs485.stopBits;
    	rs485.parity	= sysRs485.parity;
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&rs485, sizeof(rs485),
                        	dataBuf, dataLen );
        //FiPrint("get the rs485param is %d %d %d %d %d ",rs485.baudRate,sysRs485.baudRate
        //    ,rs485.dataBits,rs485.stopBits,rs485.parity);

    }        
	else
    {
    	FiPrint( "GetRs485Param failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

//
// 设置Rs485参数
//
static int SetRs485Param( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	        = 0;    
	CONFIG_RS485_PARAM		rs485;
	SYS_CONFIG_RS485_PARAM	sysRs485;
	CONFIG_PACK_DATA *    	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;
    
	memset( &rs485, 0, sizeof(rs485) );    
	memset( &sysRs485, 0, sizeof(sysRs485) );
	memcpy( (char *)&rs485, pConfigPackData->data, sizeof(rs485) );    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_RS485_PARAM, &sysRs485, sizeof(sysRs485) );
	if ( nRet != -1 ) 
    {
    	sysRs485.baudRate	= ntohl( rs485.baudRate );
    	sysRs485.dataBits	= rs485.dataBits;
    	sysRs485.stopBits	= rs485.stopBits;
    	sysRs485.parity	    = rs485.parity;
        //FiPrint("set the rs485param is %d %d %d %d ",sysRs485.baudRate
        //    ,sysRs485.dataBits,sysRs485.stopBits,sysRs485.parity);
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_RS485_PARAM, &sysRs485, sizeof(sysRs485) );
//    	if ( nRet == 0 ) nRet = WriteSysConfig();
    }        
    
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	if ( nRet != 0 ) FiPrint( "SetRs485Param failed !\r\n" );
    
	return nRet;
}


//
// 系统重启
//
static int DealRebootSystem( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	RebootSystem();
	PackConfigDataPack( MSG_CFG_DATA_RESPONSE, NULL, 0, dataBuf, dataLen );
	return 0;
}

//
// 发送Email
//
static int DealSendEmail( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int nRet = CfgManageSendEmail( GetLangText(10119, (char *)"这是一封测试邮件!") );
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return 0;
}

//
// 登录认证
//
static const int	MD5_CHECK_CODE_LEN	= 16;
static unsigned int	s_AuthRandNum	    = RandInt();

static int GetAuthRandNum( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	s_AuthRandNum = RandInt();
	unsigned int randNum = htonl( s_AuthRandNum );
	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                        (unsigned char *)&randNum, sizeof(randNum),
                    	dataBuf, dataLen );
	return 0;    
}

static int AuthUserPwd( char authInfo[MD5_CHECK_CODE_LEN],
                    	char *pPwd, unsigned int randNum )
{
	int nRet = 0;
	char pwd[32+1] = { 0 };
	strncpy( pwd, pPwd, 32 );

	MD5_CONTEXT md5Context;
	memset( &md5Context, 0x00, sizeof(md5Context) );
	unsigned char md5HashCode[MD5_CHECK_CODE_LEN] = { 0 };
	Md5Init( &md5Context );
	Md5Update( &md5Context, (unsigned char*)pwd, strlen(pwd) );
	Md5Update( &md5Context, (unsigned char*)&randNum, sizeof(randNum) );
	Md5Final( md5HashCode, &md5Context );
    
	if ( !memcmp(md5HashCode, authInfo, MD5_CHECK_CODE_LEN) ) 
    {
    	nRet = 1;    
    }
	return nRet;
}

static int DealLoginAuth( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	            	nRet	                = 0;
	unsigned char		auth	                = 0;
	CONFIG_LOGIN_AUTH	login;
	SYS_CONFIG_USER		sysUser[MAX_USER_NUM];
	CONFIG_PACK_DATA *	pConfigPackData	        = ( CONFIG_PACK_DATA * )dataBuf;


	memset( &login, 0, sizeof(login) );    
	memset( sysUser, 0, sizeof(sysUser) );

	memcpy( (char *)&login, pConfigPackData->data, sizeof(login) );
    
	if ( !strncmp(login.userName, "admin", sizeof(login.userName)) ) // 通过超级密码登录
    {
    	auth = AuthUserPwd( login.auth, (char *)SYS_CONFIG_SUPER_PWD, s_AuthRandNum );
    }

	if ( auth == 0 )
    {
    	nRet = GetSysConfig( TYPE_SYS_CONFIG_USER, &sysUser, sizeof(sysUser) );
    	if ( nRet != -1 )
        {
        	for ( int idx = 0 ; idx < MAX_USER_NUM; ++idx )
            {
            	if ( !strncmp(login.userName, sysUser[idx].userName, sizeof(login.userName)) )
                {
                	auth = AuthUserPwd( login.auth, sysUser[idx].pwd, s_AuthRandNum );
                	break;
                }
            }
        }
    }    
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, &auth, sizeof(auth), dataBuf, dataLen );
	return 0;
}

void ChangeTimeToInt( char tBuf[19], time_t &t )
{
	char timeBuf[100] = { 0 };
	int year = 0, month = 0, day = 0;
	int hour = 0, minute = 0, second = 0;    
	memcpy( timeBuf, tBuf, 19 );
	sscanf( timeBuf, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second );    
	t = FiTimeHumanToUtc( year, month, day, hour, minute, second );
}

void ChangeTimeToString( time_t t, char tBuf[19] )
{
	char timeBuf[100] = { 0 };
	int year = 0, month = 0, day = 0;
	int hour = 0, minute = 0, second = 0;
	FiTimeUtcToHuman( t, &year, &month, &day, &hour, &minute, &second );
	snprintf( timeBuf, sizeof(timeBuf), "%04d-%02d-%02d %02d:%02d:%02d",
            	year, month, day, hour, minute, second );
	memcpy( tBuf, timeBuf, 19 );    
}

void GetSearchInfo( CONFIG_SEARCH_INFO &searchInfo,
                	int &            	channel,
                	time_t &        	startTime,
                	time_t &        	endTime,
                	int &            	fileType,
                	int &            	offset	)
{
	channel	    = searchInfo.channel;
	fileType	= ntohl( searchInfo.fileType );
	offset	    = ntohl( searchInfo.offset );
	ChangeTimeToInt( searchInfo.startTime, startTime );
	ChangeTimeToInt( searchInfo.endTime, endTime );
}

//
// 录像搜索
//
static int SearchRecord( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	CONFIG_SEARCH_INFO		searchInfo;
	CONFIG_RECORD_LIST *	pRecordList	            = NULL;        
	CONFIG_PACK_DATA *    	pConfigPackData	        = ( CONFIG_PACK_DATA * )dataBuf;
	RECORD_INQUIRE_LIST *	pRecordInquireList	    = NULL;
	RECORD_INQUIRE *    	pCurRecord	            = NULL;
	int	                	channel	                = 0;
	int	                	recordType	            = 0;
	int	                	offset	                = 0;
	time_t	            	startTime	            = 0;
	time_t	            	endTime	                = 0;

	memset( &searchInfo, 0, sizeof(searchInfo) );    
    
	memcpy( &searchInfo, pConfigPackData->data, sizeof(searchInfo) );

    // 查询录像
	GetSearchInfo( searchInfo, channel, startTime, endTime, recordType, offset );    
	pRecordInquireList = FiRecInquireRecordFile( channel, recordType, startTime, endTime );
	if ( pRecordInquireList == NULL )
    {
    	FiPrint( "Inquire no record file !\r\n" );
    	CONFIG_RECORD_LIST recordList = { 0 };
    	int listLen = sizeof(recordList) - sizeof(CONFIG_RECORD_NODE);
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE, 
                            (unsigned char *)&recordList, listLen,
                        	dataBuf, dataLen );
    	return 0;
    }

    // 统计录像总个数
	int totalRecordNum = 0;
	pCurRecord = pRecordInquireList->head;
	while ( pCurRecord != NULL )
    {
    	totalRecordNum++;
    	pCurRecord = pCurRecord->next;
    }

    // 因为录像列表可能比较大, 所以必须分包发送;
    // 一次最多发送40个录像的列表, 录像列表的长度不能大于bufSize!
    // offset表示希望读取的录像列表的偏移
	int recordNum = totalRecordNum - offset + 1;
	if ( recordNum < 0 ) recordNum = 0;
	if ( recordNum > 40 ) recordNum = 40;
	int recordListLen = sizeof(CONFIG_RECORD_LIST) - sizeof(CONFIG_RECORD_NODE)
                        + sizeof(CONFIG_RECORD_NODE) * recordNum;
	while( recordListLen > bufSize )
    {
    	recordNum -= 1;
    	recordListLen -= sizeof(CONFIG_RECORD_NODE);
    }
	pRecordList	= ( CONFIG_RECORD_LIST * )Malloc( recordListLen );
	if ( pRecordList != NULL )
    {
    	pRecordList->totalNum	= htonl( totalRecordNum );
    	pRecordList->recordNum	= htonl( recordNum );

        // 跳到列表的偏移节点
    	int cnt = 0;
    	for ( pCurRecord = pRecordInquireList->head;
            	pCurRecord != NULL; 
            	pCurRecord = pCurRecord->next )            
        {
        	if ( ++cnt == offset ) break;
        }            

        // 从录像链表中读取录像列表到发送列表中
    	for ( int i = 0; i < recordNum && pCurRecord != NULL; ++i )
        {
        	strlcpy( pRecordList->recordList[i].recordName, pCurRecord->recFilename,
                    	sizeof(pRecordList->recordList[i].recordName) );
        	ChangeTimeToString( pCurRecord->startTime, pRecordList->recordList[i].startTime );
        	ChangeTimeToString( pCurRecord->stopTime, pRecordList->recordList[i].endTime );                        
        	pRecordList->recordList[i].recordLen	= htonl( pCurRecord->recLen );
        	pRecordList->recordList[i].recordType	= htonl( pCurRecord->recType );
        	pCurRecord = pCurRecord->next;
        }
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE, 
                            (unsigned char *)pRecordList, recordListLen,
                        	dataBuf, dataLen );
    	Free( pRecordList );
    }
	else
    {
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	FiRecFreeInquireRecord( pRecordInquireList );
	return 0;
}

//extern SNAP_INQUIRE_LIST *FiSnapInquireSnapFile( int channel, unsigned long snapType,
//                                            	time_t startTime, time_t stopTime );
//extern void FiSnapFreeInquireSnap( SNAP_INQUIRE_LIST *snap_inquire_ret );

//
// 抓拍搜索
//
static int SearchSnapFile( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	CONFIG_SEARCH_INFO		searchInfo;
	CONFIG_SNAP_LIST *    	pSnapList	            = NULL;        
	CONFIG_PACK_DATA *    	pConfigPackData	        = ( CONFIG_PACK_DATA * )dataBuf;
	SNAP_INQUIRE_LIST *    	pSnapInquireList	    = NULL;
	SNAP_INQUIRE *        	pCurSnap	            = NULL;
	int	                	channel	                = 0;
	int	                	snapType	            = 0;
	int	                	offset	                = 0;
	time_t	            	startTime	            = 0;
	time_t	            	endTime	                = 0;    

	memset( &searchInfo, 0, sizeof(searchInfo) );    

	memcpy( &searchInfo, pConfigPackData->data, sizeof(searchInfo) );
    
    // 查询抓拍文件
	GetSearchInfo( searchInfo, channel, startTime, endTime, snapType, offset );
	pSnapInquireList = FiSnapInquireSnapFile( channel, snapType, startTime, endTime );
	if ( pSnapInquireList == NULL )
    {
    	FiPrint( "Inquire no snap file !\r\n" );
    	CONFIG_SNAP_LIST snapList = { 0 };
    	int listLen = sizeof(snapList) - sizeof(CONFIG_SNAP_NODE);
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&snapList, listLen,
                        	dataBuf, dataLen );
    	return 0;
    }

    // 统计抓拍总个数
	int totalSnapNum = 0;
	pCurSnap = pSnapInquireList->head;
	while ( pCurSnap != NULL )
    {
    	totalSnapNum++;
    	pCurSnap = pCurSnap->next;
    }
    

    // 因为抓拍列表可能比较大, 所以必须分包发送;
    // 一次最多发送40个抓拍的列表, 抓拍列表的长度不能大于bufSize!
    // offset表示希望读取的抓拍列表的偏移
	int snapNum = totalSnapNum - offset + 1;
	if ( snapNum < 0 ) snapNum = 0;
	if ( snapNum > 40 ) snapNum = 40;
	int snapListLen = sizeof(CONFIG_SNAP_LIST) - sizeof(CONFIG_SNAP_NODE)
                        + sizeof(CONFIG_SNAP_NODE) * snapNum;
	while ( snapListLen > bufSize )
    {
    	snapNum	    -= 1;
    	snapListLen	-= sizeof(CONFIG_SNAP_NODE);
    }
	pSnapList = ( CONFIG_SNAP_LIST * )Malloc( snapListLen );
	if ( pSnapList != NULL )
    {
    	pSnapList->totalNum	= htonl( totalSnapNum );
    	pSnapList->snapNum	= htonl( snapNum );

        // 跳到列表的偏移节点
    	int cnt = 0;
    	for ( pCurSnap = pSnapInquireList->head;
            	pCurSnap != NULL; 
            	pCurSnap = pCurSnap->next )            
        {
        	if ( ++cnt == offset ) break;
        }            

        // 从抓拍链表中读取抓拍列表到发送列表中
    	for ( int i = 0; i < snapNum && pCurSnap != NULL; ++i )
        {
        	strlcpy( pSnapList->snapList[i].snapName, pCurSnap->snapName,
                    	sizeof(pSnapList->snapList[i].snapName) );
        	ChangeTimeToString( pCurSnap->snapTime, pSnapList->snapList[i].snapTime );
        	pSnapList->snapList[i].snapLen	= htonl( pCurSnap->snapLen );
        	pSnapList->snapList[i].snapType	= htonl( pCurSnap->snapType );
        	pCurSnap = pCurSnap->next;
        }
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE, 
                            (unsigned char *)pSnapList, snapListLen,
                        	dataBuf, dataLen );
    	Free( pSnapList );
    }
	else
    {
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	FiSnapFreeInquireSnap( pSnapInquireList );

	return 0;
}

//
// 获取数人参数
//
static int GetPcParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	            = 0;    
	CONFIG_PC_PARAM	    	pcParam;
	SYS_CONFIG_PC_PARAM		sysPcParam;
	CONFIG_PACK_DATA *    	pConfigPackData	    = ( CONFIG_PACK_DATA * )dataBuf;
	int	                	channel	            = *(pConfigPackData->data);

    
	memset( &pcParam, 0, sizeof(pcParam) );    
	memset( &sysPcParam, 0, sizeof(sysPcParam) );    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_PC_PARAM, &sysPcParam, sizeof(sysPcParam), channel );
	if ( nRet != -1 ) 
    {
    	pcParam.channel	        = channel;
    	pcParam.nWidth	        = htonl( sysPcParam.nWidth );
    	pcParam.nHeight	        = htonl( sysPcParam.nHeight );
    	pcParam.nRoiLeft	    = htonl( sysPcParam.nRoiLeft );
    	pcParam.nRoiTop	        = htonl( sysPcParam.nRoiTop );
    	pcParam.nRoiRight	    = htonl( sysPcParam.nRoiRight );
    	pcParam.nRoiBottom	    = htonl( sysPcParam.nRoiBottom );
    	pcParam.bOriVertical	= htonl( sysPcParam.bOriVertical );
    	pcParam.nLoiterTime	    = htonl( sysPcParam.nLoiterTime );
    	pcParam.nFPSOfDet	    = htonl( sysPcParam.nFPSOfDet );
    	pcParam.nMaxMatchDist	= htonl( sysPcParam.nMaxMatchDist );
    	pcParam.bOriEnter	    = htonl( sysPcParam.bOriEnter );    
    	pcParam.bOpenPcDetect	= htonl( sysPcParam.bOpenPcDetect );
    	pcParam.videoMode	    = htonl( sysPcParam.videoMode );
    	pcParam.nStepMotion	    = htonl( sysPcParam.nStepMotion);
    	pcParam.nMinHeadWidth	= htonl( sysPcParam.nMinHeadWidth );
        
    	pcParam.nMaxHeadWidth	= htonl( sysPcParam.nMaxHeadWidth );
    	pcParam.bHighPrecision	= htonl( sysPcParam.bHighPrecision );        
            
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&pcParam, sizeof(pcParam),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "GetPcParam failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;    
}

//
// 设置数人参数
//
static int SetPcParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	            = 0;
	CONFIG_PC_PARAM	    	pcParam;    
	SYS_CONFIG_PC_PARAM		sysPcParam;
	CONFIG_PACK_DATA *    	pConfigPackData	    = ( CONFIG_PACK_DATA * )dataBuf;
	int	                	channel	            = 0;

	memset( &pcParam, 0, sizeof(pcParam) );    
	memset( &sysPcParam, 0, sizeof(sysPcParam) );    
	memcpy( (char *)&pcParam, pConfigPackData->data, sizeof(pcParam) );    
	channel = pcParam.channel;
	nRet = GetSysConfig( TYPE_SYS_CONFIG_PC_PARAM, &sysPcParam, sizeof(sysPcParam), channel );
	if ( nRet == 0 ) 
    {    
//    	int		nWidth	        = ntohl( pcParam.nWidth );
//    	int		nHeight	        = ntohl( pcParam.nHeight );
    	int		nRoiLeft	    = ntohl( pcParam.nRoiLeft );
    	int		nRoiTop	        = ntohl( pcParam.nRoiTop );
    	int		nRoiRight	    = ntohl( pcParam.nRoiRight );
    	int		nRoiBottom	    = ntohl( pcParam.nRoiBottom );
    	int		bOriVertical	= ntohl( pcParam.bOriVertical );
    	int		nLoiterTime	    = ntohl( pcParam.nLoiterTime );
    	int		nFPSOfDet	    = ntohl( pcParam.nFPSOfDet );
    	int		nMaxMatchDist	= ntohl( pcParam.nMaxMatchDist );
    	int		bOriEnter	    = ntohl( pcParam.bOriEnter );
    	int		bOpenPcDetect	= ntohl( pcParam.bOpenPcDetect );    
    	int		videoMode	    = ntohl( pcParam.videoMode );
    	int		nStepMotion	    = ntohl( pcParam.nStepMotion );
    	int     nMinHeadWidth	= ntohl( pcParam.nMinHeadWidth );
        
    	int		nMaxHeadWidth	= ntohl( pcParam.nMaxHeadWidth );
    	int		bHighPrecision	= ntohl( pcParam.bHighPrecision );
        
    	if ( nRoiLeft < 0 || CIF_WIDTH <= nRoiLeft ) nRet = -1;
    	else if ( nRoiTop < 0 || CIF_HEIGHT <= nRoiTop ) nRet = -1;
    	else if ( nRoiRight < 0 || CIF_WIDTH <= nRoiRight ) nRet = -1;
    	else if ( nRoiBottom < 0 || CIF_HEIGHT <= nRoiBottom ) nRet = -1;
    	else if ( bOriVertical != 0 && bOriVertical != 1 ) nRet = -1;
    	else if ( nLoiterTime < 1000 || 60*1000 < nLoiterTime ) nRet = -1;
    	else if ( nFPSOfDet < 0 || 25 < nFPSOfDet ) nRet = -1;
    	else if ( nMaxMatchDist < 0 || 150 < nMaxMatchDist ) nRet = -1;
    	else if ( bOriEnter != 0 && bOriEnter != 1 ) nRet = -1;
    	else if ( bOpenPcDetect != 0 && bOpenPcDetect != 1 ) nRet = -1;
    	else if ( videoMode != TYPE_VIDEO_MODE_CIF 
                && videoMode != TYPE_VIDEO_MODE_NINTH_D1
                && videoMode != TYPE_VIDEO_MODE_QCIF
                && videoMode != TYPE_VIDEO_MODE_D1
                && videoMode != TYPE_VIDEO_MODE_FNINTH_D1 
                //&& videoMode != TYPE_VIDEO_MODE_10
                //&& videoMode != TYPE_VIDEO_MODE_20
                && videoMode != TYPE_VIDEO_MODE_30
                && videoMode != TYPE_VIDEO_MODE_40
                && videoMode != TYPE_VIDEO_MODE_50
                //&& videoMode != TYPE_VIDEO_MODE_60
                //&& videoMode != TYPE_VIDEO_MODE_70
                && videoMode != TYPE_VIDEO_MODE_80
                && videoMode != TYPE_VIDEO_MODE_90) nRet = -1;
    	else if ( nStepMotion != 0 && nStepMotion != 1 ) nRet = -1;    

        
    	if ( nRet == 0 )
        {
        	sysPcParam.nRoiLeft	        = nRoiLeft;
        	sysPcParam.nRoiTop	        = nRoiTop;
        	sysPcParam.nRoiRight	    = nRoiRight;
        	sysPcParam.nRoiBottom	    = nRoiBottom;
        	sysPcParam.bOriVertical	    = bOriVertical;
        	sysPcParam.nLoiterTime	    = nLoiterTime;
        	sysPcParam.nFPSOfDet	    = nFPSOfDet;
        	sysPcParam.nMaxMatchDist	= nMaxMatchDist;
        	sysPcParam.bOriEnter	    = bOriEnter;
        	sysPcParam.bOpenPcDetect	= bOpenPcDetect;
        	sysPcParam.videoMode	    = videoMode;
        	sysPcParam.nStepMotion	    = nStepMotion;
        	sysPcParam.nMinHeadWidth	= nMinHeadWidth;        
        	sysPcParam.nMaxHeadWidth	= nMaxHeadWidth;
        	sysPcParam.bHighPrecision	= bHighPrecision;            
        }
    	else
        {
        	FiPrint( "SetPcParam Param Error !\r\n" );
        }        
    }    

	if ( nRet == 0 )
    {
    	if ( sysPcParam.bOpenPcDetect )
        {
            	nRet = CfgManageOpenPcDetect( channel );
        }
    	else
        {
            	nRet = CfgManageClosePcDetect( channel );
        }
    }
    
	if ( nRet == 0 )
    {
    	nRet = CfgManageSetPcDirection(    	channel,
                                        	sysPcParam.bOriVertical,
                                        	sysPcParam.bOriEnter	);
    }
	if ( nRet == 0 )
    {
    	nRet = CfgManageSetPcDetectArea(	channel,
                                        	sysPcParam.nRoiLeft,
                                        	sysPcParam.nRoiTop,
                                        	sysPcParam.nRoiRight,
                                        	sysPcParam.nRoiBottom	);
    }
	if ( nRet == 0 ) nRet = CfgManageSetVideoMode( channel, sysPcParam.videoMode );
	if ( nRet == 0 )
    {
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_PC_PARAM,
                            &sysPcParam, sizeof(sysPcParam), channel );
//    	if ( nRet == 0 ) nRet = WriteSysConfig();
    }
	if ( nRet != 0 ) FiPrint( "SetPcParam failed !\r\n" );
    
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 设置 nMaxStepLen和nMaxMatchDist.
//
int SetPcParam_d(int iMaxMatchDist, int iMaxStepLen, int ch)
{
	int	                	nRet	            = 0;
	SYS_CONFIG_PC_PARAM		sysPcParam;
	int	                	channel	            = 0;

	memset( &sysPcParam, 0, sizeof(sysPcParam) );    
	channel = ch;
	nRet = GetSysConfig( TYPE_SYS_CONFIG_PC_PARAM, &sysPcParam, sizeof(sysPcParam), channel );
	if ( nRet == 0 ) 
    {    
    	int		nMaxMatchDist	= iMaxMatchDist;
    	int		nMaxStepLen	    = iMaxStepLen;
        
    	if ( nRet == 0 )
        {
        	sysPcParam.nMaxMatchDist	= nMaxMatchDist;
        	sysPcParam.nMaxStepLen	    = nMaxStepLen;
        }
    	else
        {
        	FiPrint( "SetPcParam Param Error !\r\n" );
        }
    }    

	if ( nRet == 0 )
    {
    	if ( sysPcParam.bOpenPcDetect )
        {
            	nRet = CfgManageOpenPcDetect( channel );
        }
    	else
        {
            	nRet = CfgManageClosePcDetect( channel );
        }
    }
    
	if ( nRet == 0 )
    {
    	nRet = CfgManageSetPcDirection(    	channel,
                                        	sysPcParam.bOriVertical,
                                        	sysPcParam.bOriEnter	);
    }
	if ( nRet == 0 )
    {
    	nRet = CfgManageSetPcDetectArea(	channel,
                                        	sysPcParam.nRoiLeft,
                                        	sysPcParam.nRoiTop,
                                        	sysPcParam.nRoiRight,
                                        	sysPcParam.nRoiBottom	);
    }
	if ( nRet == 0 ) nRet = CfgManageSetVideoMode( channel, sysPcParam.videoMode );
	if ( nRet == 0 )
    {
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_PC_PARAM,
                            &sysPcParam, sizeof(sysPcParam), channel );
//    	if ( nRet == 0 ) nRet = WriteSysConfig();
    }
	if ( nRet != 0 ) FiPrint( "SetPcParam failed !\r\n" );
    
	return nRet;
}

//
//检查pc参数nMaxStepLen是否已经设置到算法中
//
int CheckStepLen()
{
	int	                	nRet	            = 0;
	SYS_CONFIG_PC_PARAM		sysPcParam;
	int	                	channel	            = 0;

    
	memset( &sysPcParam, 0, sizeof(sysPcParam) );    
	for (channel = 0; channel < REAL_CHANNEL_NUM; ++channel )
    {
    	nRet = GetSysConfig( TYPE_SYS_CONFIG_PC_PARAM, &sysPcParam, sizeof(sysPcParam), channel );
    	if ( nRet == 0 ) 
        {    
        	if (sysPcParam.nMaxStepLen != DEF_STEP_LEN)
            {
            	nRet = SetPcParam_d(DEF_MAXMATCHDIST, DEF_STEP_LEN, channel);
            	if (nRet < 0)
                {
                	FiPrint("xxx set DEF_STEP_LEN ERROR\n");
                	break;
                }
            }
        }
    	else
        {
        	FiPrint("xxx GetSysConfig ERROR\n");
        	break;
        }
    }

	return nRet;
}

// 获取Licence信息
//
static int GetLicenceInfo( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int		nRet	= 0;
	LICENCE	licence	= { 0 };

	memset( &licence, 0, sizeof(licence) );    

	nRet = SysGetLicence( &licence );
	if ( nRet != -1 ) 
    {
    	CONFIG_LICENCE_INFO info = { 0 };
        
    	info.flag	    = htonl( licence.flag );
    	info.oemVersion	= licence.oemVersion;
    	info.platform	= licence.platform;
        
    	for ( int channel = 0; channel < MAX_CHANNEL_NUM; ++channel )
        {
        	info.odType[channel]     = htonl( licence.odType[channel] );
        	info.odFlag[channel]     = licence.odFlag[channel];
        	info.encodeFlag[channel] = licence.encodeFlag[channel];
        }
        
    	info.serialType	    = licence.serialType;
    	info.protocolType	= licence.protocolType;
    	info.pcType	        = licence.pcType;
    	info.dataUploadFlag	= licence.dataUploadFlag;
    	info.fivRf433mFlag	= licence.fivRf433mFlag;
    	info.led	        = licence.led;
    	info.mainSlave	    = licence.mainSlave;
    	info.fivType	    = licence.fivType;
    	info.threeg	        = licence.threeg;
    	info.wifi	        = licence.wifi;
    	info.wlPlatform	    = licence.wlPlatform;
    	info.bsl	        = licence.bsl;
    	info.synovate       = licence.synovate;

    	info.czcf             = licence.czcf;
    	info.shelterf	    = licence.shelterf;
    	info.qirui	        = licence.qirui;
    	info.jyzht	        = licence.jyzht;
    	info.devType	    = licence.devType;

    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE, 
                            (unsigned char *)&info, sizeof(info),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "GetLicenceInfo failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

//
// 获取中间件参数
//
static int GetMidWareParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	                            = 0;
	SYS_CONFIG_MIDWARE		sysMidParam[MAX_MIDWARE_USER_NUM];    
	PARAM_CONFIG_DTU     	dtauParam;

	memset( sysMidParam, 0, sizeof(sysMidParam) );    

	nRet = ParamGetDtu( &dtauParam );
	if ( nRet != -1 ) 
    {    
    	for( int i = 0; i < MAX_MIDWARE_USER_NUM; ++i )
        {
        	sysMidParam[i].regEnable	    = dtauParam.enable;
        	sysMidParam[i].transType	    = dtauParam.transProtocol + 1; // 协议和本地的值相差1
        	sysMidParam[i].net.port         = htons( dtauParam.serverPort );
        	strcpy( sysMidParam[i].net.ip, dtauParam.severIp );
        	sprintf( sysMidParam[i].auth.user, "%u", dtauParam.interval );
        	snprintf( sysMidParam[i].auth.passwd, sizeof(sysMidParam[i].auth.passwd), 
                            "%s", dtauParam.heartbeatContent );
        	sysMidParam[i].serial.baudRate     = 0; //htonl( sysMidParam[i].serial.baudRate );    


        }
            
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)sysMidParam, sizeof(sysMidParam),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "GetMidWareParam failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;    
}

//
// 设置中间件参数
//
static int SetMidWareParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	                            = 0;
	SYS_CONFIG_MIDWARE		setMidParam[MAX_MIDWARE_USER_NUM];
	CONFIG_PACK_DATA *    	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;
	PARAM_CONFIG_DTU     	dtauParam;
	uint                 	interval;

	memset( &setMidParam, 0, sizeof(setMidParam) );    

	memcpy( setMidParam, pConfigPackData->data, sizeof(setMidParam) );
    // for( int i = 0; i < MAX_MIDWARE_USER_NUM; ++i )
    {
    	dtauParam.enable = setMidParam[0].regEnable;
    	dtauParam.transProtocol = setMidParam[0].transType - 1; // 协议和本地的值相差1
    	dtauParam.serverPort = htons( setMidParam[0].net.port );
        	strcpy( dtauParam.severIp, setMidParam[0].net.ip );
        	sscanf( setMidParam[0].auth.user, "%u", &interval );
        	dtauParam.interval = interval;
        	snprintf( dtauParam.heartbeatContent, sizeof(setMidParam[0].auth.passwd), 
                            "%s", setMidParam[0].auth.passwd );
    }


	nRet = ParamSetDtu( &dtauParam );
	if ( nRet != -1 ) 
    {    
    	DtuSendParamChangeMessage();
    }        

	if ( nRet != 0 ) 
    {
    	FiPrint( "SetMidWareParam failed !\r\n" );        
    }
    
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
    
	return nRet;
}

//
// 获取门规则参数
//
static int GetDoorRuleParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	return 0;
}

//
// 设置门规则参数
//
static int SetDoorRuleParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	return 0;
}

//
// 获取数人配置
//
static int GetPcConfig( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	            = 0;    
	CONFIG_PC_CONFIG		pcConfig;
	SYS_CONFIG_PC_CONFIG	sysPcConfig;
	char tmp[3];
    
	memset( &pcConfig, 0, sizeof(pcConfig) );        
	memset( &sysPcConfig, 0, sizeof(sysPcConfig) );    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_PC_CONFIG, &sysPcConfig, sizeof(sysPcConfig) );
	if ( nRet != -1 ) 
    {
    	pcConfig.bSaveHistoryData = sysPcConfig.bSaveHistroyData;
    	memcpy( pcConfig.countTime, sysPcConfig.countTime, sizeof(pcConfig.countTime) );
    	memcpy( pcConfig.clrCountTime, sysPcConfig.clrCountTime, sizeof(pcConfig.clrCountTime) );    
    	pcConfig.limitedNumber = htonl( sysPcConfig.limitedNumber );
    	if (SysGetOemType() == OEM_JSYD)
        {
        	memcpy( &(pcConfig.upModNum), &(sysPcConfig.upModNum), sizeof(tmp) );
        }
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&pcConfig, sizeof(pcConfig),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "GetPcConfig failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;    
}

//
// 设置数人配置
//
static int SetPcConfig( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	            = 0;
	CONFIG_PC_CONFIG		pcConfig;
	CONFIG_PACK_DATA *    	pConfigPackData	    = ( CONFIG_PACK_DATA * )dataBuf;
	int contentSize = sizeof(pcConfig), cpySize;
	int maxNum = 0;

	memset( &pcConfig, 0, sizeof(pcConfig) );        
    // 针对后来扩展
	pConfigPackData->head.len = ntohs( pConfigPackData->head.len );
	FiPrint2( "SetPcConfig:pConfigPackData->head.len = %d!\r\n", pConfigPackData->head.len );
	cpySize = (pConfigPackData->head.len > contentSize) ? contentSize : pConfigPackData->head.len;
	strcpy( pcConfig.countTime[0], "00:00-23:59" );
    
	memcpy( (char *)&pcConfig, pConfigPackData->data, cpySize );    
	nRet = CfgManageSetSaveHistoryData( pcConfig.bSaveHistoryData );
	if ( 0 == nRet ) nRet = CfgManageSetPcCountTime( pcConfig.countTime );
	if ( 0 == nRet ) nRet = CfgManageSetPcClsCountTime((CONFIG_CLEAR_PC_PERIOD *)&(pcConfig.clrCountTime));
	if (SysGetOemType() == OEM_JSYD)
    	if ( 0 == nRet ) nRet = CfgManageSetPcCarMod((SYS_CONFIG_PC_CONFIG *)&pcConfig);
	if ( 0 == nRet )
    {
    	maxNum = ntohl(pcConfig.limitedNumber);
    	nRet = CfgManageSetPcLimitedNum(maxNum);
    }
	if ( nRet == -1 ) FiPrint( "SetPcConfig failed !\r\n" );

	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取车牌检测参数
//
static int GetLpParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	            = 0;    
	CONFIG_LP_PARAM	    	lpParam;
	SYS_CONFIG_LP_PARAM		sysLpParam;
	CONFIG_PACK_DATA *    	pConfigPackData	    = ( CONFIG_PACK_DATA * )dataBuf;
	int	                	channel	            = *(pConfigPackData->data);

	memset( &lpParam, 0, sizeof(lpParam) );        
	memset( &sysLpParam, 0, sizeof(sysLpParam) );    
    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_LP_PARAM, &sysLpParam, sizeof(sysLpParam), channel );
	if ( nRet != -1 ) 
    {
    	lpParam.channel	            = channel;
    	lpParam.nWidth	            = htonl( sysLpParam.nWidth );
    	lpParam.nHeight	            = htonl( sysLpParam.nHeight );
    	lpParam.nMinTrackFrame	    = htonl( sysLpParam.nMinTrackFrame );
    	lpParam.nMaxLostFrame	    = htonl( sysLpParam.nMaxLostFrame );
    	lpParam.nMaxDistance	    = htonl( sysLpParam.nMaxDistance );
    	lpParam.nDetectCarType	    = htonl( sysLpParam.nDetectCarType );
    	lpParam.bDetectPlate	    = htonl( sysLpParam.bDetectPlate );
    	lpParam.bEnableTrackCar	    = htonl( sysLpParam.bEnableTrackCar );
    	lpParam.bEnableTrackPlate	= htonl( sysLpParam.bEnableTrackPlate );
        
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&lpParam, sizeof(lpParam),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "GetLpParam failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;    
}

//
// 设置车牌检测参数
//
static int SetLpParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	            = 0;
	CONFIG_LP_PARAM	    	lpParam;    
	SYS_CONFIG_LP_PARAM		sysLpParam;
	CONFIG_PACK_DATA *    	pConfigPackData	    = ( CONFIG_PACK_DATA * )dataBuf;
	int	                	channel	            = 0;

	memset( &lpParam, 0, sizeof(lpParam) );        
	memset( &sysLpParam, 0, sizeof(sysLpParam) );    
    
	memcpy( (char *)&lpParam, pConfigPackData->data, sizeof(lpParam) );    
	channel = lpParam.channel;
	nRet = GetSysConfig( TYPE_SYS_CONFIG_LP_PARAM, &sysLpParam, sizeof(sysLpParam), channel );
	if ( nRet == 0 ) 
    {    
    	sysLpParam.nMinTrackFrame	    = ntohl( lpParam.nMinTrackFrame );
    	sysLpParam.nMaxLostFrame	    = ntohl( lpParam.nMaxLostFrame );
    	sysLpParam.nMaxDistance	        = ntohl( lpParam.nMaxDistance );
    	sysLpParam.nDetectCarType	    = ntohl( lpParam.nDetectCarType );
    	sysLpParam.bDetectPlate	        = ntohl( lpParam.bDetectPlate );
    	sysLpParam.bEnableTrackCar	    = ntohl( lpParam.bEnableTrackCar );
    	sysLpParam.bEnableTrackPlate	= ntohl( lpParam.bEnableTrackPlate );
        
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_LP_PARAM, &sysLpParam, sizeof(sysLpParam), channel );
    	if ( nRet == 0 )
        {
//        	nRet = WriteSysConfig();
        }
    }

	if ( nRet != 0 ) FiPrint( "SetLpParam failed !\r\n" );
        
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取数据上传协议参数
//
static int GetDataUpload( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	            = 0;    
	CONFIG_DATA_UPLOAD		dataUpload;
	SYS_CONFIG_DATA_UPLOAD	sysDataUpload;

	memset( &dataUpload, 0, sizeof(dataUpload) );        
	memset( &sysDataUpload, 0, sizeof(sysDataUpload) );    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_DATA_UPLOAD, &sysDataUpload, sizeof(sysDataUpload) );
	if ( nRet != -1 ) 
    {
    	memcpy( dataUpload.ip, sysDataUpload.ip, sizeof(dataUpload.ip) );
        
    	dataUpload.port	    = htonl( sysDataUpload.port );
    	dataUpload.flag	    = htonl( sysDataUpload.flag );
    	dataUpload.interval	= htonl( sysDataUpload.interval );
    	dataUpload.mode	    = htonl( sysDataUpload.mode );
        
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&dataUpload, sizeof(dataUpload),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "GetDataUpload failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;    
}

//
// 设置数据上传协议参数
//
static int SetDataUpload( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	            = 0;    
	CONFIG_DATA_UPLOAD		dataUpload;
	SYS_CONFIG_DATA_UPLOAD	sysDataUpload;
	CONFIG_PACK_DATA *    	pConfigPackData	    = ( CONFIG_PACK_DATA * )dataBuf;

	memset( &dataUpload, 0, sizeof(dataUpload) );        
	memset( &sysDataUpload, 0, sizeof(sysDataUpload) );    
	memcpy( (char *)&dataUpload, pConfigPackData->data, sizeof(dataUpload) );    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_DATA_UPLOAD, &sysDataUpload, sizeof(sysDataUpload) );
	if ( nRet == 0 ) 
    {    
    	memcpy( sysDataUpload.ip, dataUpload.ip, sizeof(sysDataUpload.ip) );

    	sysDataUpload.port	    = ntohl( dataUpload.port );
    	sysDataUpload.flag	    = ntohl( dataUpload.flag );
    	sysDataUpload.interval	= ntohl( dataUpload.interval );
    	sysDataUpload.mode	    = ntohl( dataUpload.mode );

    	nRet = SetSysConfig( TYPE_SYS_CONFIG_DATA_UPLOAD, &sysDataUpload, sizeof(sysDataUpload) );
//    	if ( nRet == 0 ) nRet = WriteSysConfig();
    }

	if ( nRet != 0 ) FiPrint( "SetDataUpload failed !\r\n" );

	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取AB门参数
//
static int GetABDoorParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	            = 0;    
	CONFIG_ABDOOR_PARAM	    	abDoorParam;
	SYS_CONFIG_ABDOOR_PARAM		sysABDoorParam;
	CONFIG_PACK_DATA *        	pConfigPackData	    = ( CONFIG_PACK_DATA * )dataBuf;
	int	                    	channel	            = *(pConfigPackData->data);

	memset( &abDoorParam, 0, sizeof(abDoorParam) );        
	memset( &sysABDoorParam, 0, sizeof(sysABDoorParam) );    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_ABDOOR_PARAM,
                        &sysABDoorParam, sizeof(sysABDoorParam), channel );
	if ( nRet != -1 ) 
    {
    	abDoorParam.channel	            = channel;
    	abDoorParam.nWidth	            = htonl( sysABDoorParam.nWidth );
    	abDoorParam.nHeight	            = htonl( sysABDoorParam.nHeight );
    	abDoorParam.nRoiLeft	        = htonl( sysABDoorParam.nRoiLeft );
    	abDoorParam.nRoiTop	            = htonl( sysABDoorParam.nRoiTop );
    	abDoorParam.nRoiRight	        = htonl( sysABDoorParam.nRoiRight );
    	abDoorParam.nRoiBottom	        = htonl( sysABDoorParam.nRoiBottom );
    	abDoorParam.bOriVertical	    = htonl( sysABDoorParam.bOriVertical );
    	abDoorParam.nLoiterTime	        = htonl( sysABDoorParam.nLoiterTime );
    	abDoorParam.nFPSOfDet	        = htonl( sysABDoorParam.nFPSOfDet );
    	abDoorParam.nMaxMatchDist	    = htonl( sysABDoorParam.nMaxMatchDist );
    	abDoorParam.bOriEnter	        = htonl( sysABDoorParam.bOriEnter );
    	abDoorParam.bOpenABDoorDetect	= htonl( sysABDoorParam.bOpenABDoorDetect );
    	abDoorParam.stayerNum	        = htonl( sysABDoorParam.stayerNum );    
    	abDoorParam.sensitivity	        = htonl( sysABDoorParam.sensitivity );
    	abDoorParam.videoMode	        = htonl( sysABDoorParam.videoMode );

    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&abDoorParam, sizeof(abDoorParam),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "GetABDoorParam failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;    
}

//
// 设置AB门参数
//
static int SetABDoorParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	            = 0;
	CONFIG_ABDOOR_PARAM	    	abDoorParam;
	SYS_CONFIG_ABDOOR_PARAM		sysABDoorParam;
	CONFIG_PACK_DATA *        	pConfigPackData	    = ( CONFIG_PACK_DATA * )dataBuf;
	int	                    	channel	            = 0;
    
	memset( &abDoorParam, 0, sizeof(abDoorParam) );        
	memset( &sysABDoorParam, 0, sizeof(sysABDoorParam) );    
	memcpy( (char *)&abDoorParam, pConfigPackData->data, sizeof(abDoorParam) );    
	channel = abDoorParam.channel;
	nRet = GetSysConfig( TYPE_SYS_CONFIG_ABDOOR_PARAM,
                        &sysABDoorParam, sizeof(sysABDoorParam), channel );
	if ( nRet == 0 ) 
    {    
    	sysABDoorParam.nRoiLeft	            = ntohl( abDoorParam.nRoiLeft );
    	sysABDoorParam.nRoiTop	            = ntohl( abDoorParam.nRoiTop );
    	sysABDoorParam.nRoiRight	        = ntohl( abDoorParam.nRoiRight );
    	sysABDoorParam.nRoiBottom	        = ntohl( abDoorParam.nRoiBottom );
    	sysABDoorParam.bOriVertical	        = ntohl( abDoorParam.bOriVertical );
    	sysABDoorParam.nLoiterTime	        = ntohl( abDoorParam.nLoiterTime );
    	sysABDoorParam.nFPSOfDet	        = ntohl( abDoorParam.nFPSOfDet );
    	sysABDoorParam.nMaxMatchDist	    = ntohl( abDoorParam.nMaxMatchDist );
    	sysABDoorParam.bOriEnter	        = ntohl( abDoorParam.bOriEnter );
    	sysABDoorParam.bOpenABDoorDetect	= ntohl( abDoorParam.bOpenABDoorDetect );
    	sysABDoorParam.stayerNum	        = ntohl( abDoorParam.stayerNum );
    	sysABDoorParam.sensitivity	        = ntohl( abDoorParam.sensitivity );    
    	sysABDoorParam.videoMode	        = ntohl( abDoorParam.videoMode );
    }    

	if ( nRet == 0 )
    {
    	if ( sysABDoorParam.bOpenABDoorDetect )
        {
        	nRet = CfgManageOpenABDoorDetect( channel );
        }
    	else
        {
        	nRet = CfgManageCloseABDoorDetect( channel );
        }
    }
    
	if ( nRet == 0 )
    {
    	nRet = CfgManageSetABDoorDirection(    	channel,
                                            	sysABDoorParam.bOriVertical,
                                            	sysABDoorParam.bOriEnter	);
    }
	if ( nRet == 0 )
    {
    	nRet = CfgManageSetABDoorDetectArea(	channel,
                                            	sysABDoorParam.nRoiLeft,
                                            	sysABDoorParam.nRoiTop,
                                            	sysABDoorParam.nRoiRight,
                                            	sysABDoorParam.nRoiBottom	);
    }
	if ( nRet == 0 ) nRet = CfgManageSetVideoMode( channel, sysABDoorParam.videoMode );
	if ( nRet == 0 )
    {
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_ABDOOR_PARAM,
                            &sysABDoorParam, sizeof(sysABDoorParam), channel );
//    	if ( nRet == 0 ) nRet = WriteSysConfig();
    }
    
	if ( nRet != 0 ) FiPrint( "SetABDoorParam failed !\r\n" );
        
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取FIV参数
//
static int GetFivParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	            = 0;    
	CONFIG_FIV_PARAM	    	fivParam;
	SYS_CONFIG_FIV_PARAM		sysFivParam;
	CONFIG_PACK_DATA *        	pConfigPackData	    = ( CONFIG_PACK_DATA * )dataBuf;
	int	                    	channel	            = *(pConfigPackData->data);
    
	memset( &fivParam, 0, sizeof(fivParam) );        
	memset( &sysFivParam, 0, sizeof(sysFivParam) );    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_FIV_PARAM,
                        &sysFivParam, sizeof(sysFivParam), channel );
	if ( nRet != -1 ) 
    {
    	fivParam.channel	    = channel;
    	fivParam.fAlpha	        = sysFivParam.fAlpha;
    	fivParam.fFactor	    = sysFivParam.fFactor;
    	fivParam.fT	            = sysFivParam.fT;
    	fivParam.nWindowSize	= htonl( sysFivParam.nWindowSize );
    	fivParam.nTargetSize	= htonl( sysFivParam.nTargetSize );
    	fivParam.nMaxLost	    = htonl( sysFivParam.nMaxLost );
    	fivParam.nMinFrame	    = htonl( sysFivParam.nMinFrame );
    	fivParam.nMaxDistance	= htonl( sysFivParam.nMaxDistance );
        
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&fivParam, sizeof(fivParam),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "GetFivParam failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;    
}

//
// 设置FIV参数
//
static int SetFivParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	            = 0;
	CONFIG_FIV_PARAM	    	fivParam;
	SYS_CONFIG_FIV_PARAM		sysFivParam;
	CONFIG_PACK_DATA *        	pConfigPackData	    = ( CONFIG_PACK_DATA * )dataBuf;
	int	                    	channel	            = 0;

	memset( &fivParam, 0, sizeof(fivParam) );        
	memset( &sysFivParam, 0, sizeof(sysFivParam) );        
	memcpy( (char *)&fivParam, pConfigPackData->data, sizeof(fivParam) );    
	channel = fivParam.channel;
	nRet = GetSysConfig( TYPE_SYS_CONFIG_FIV_PARAM,
                        &sysFivParam, sizeof(sysFivParam), channel );
	if ( nRet == 0 ) 
    {    
    	sysFivParam.fAlpha	        = fivParam.fAlpha;
    	sysFivParam.fFactor	        = fivParam.fFactor;
    	sysFivParam.fT	            = fivParam.fT;
    	sysFivParam.nWindowSize	    = ntohl( fivParam.nWindowSize );
    	sysFivParam.nTargetSize	    = ntohl( fivParam.nTargetSize );
    	sysFivParam.nMaxLost	    = ntohl( fivParam.nMaxLost );
    	sysFivParam.nMinFrame	    = ntohl( fivParam.nMinFrame );
    	sysFivParam.nMaxDistance	= ntohl( fivParam.nMaxDistance );
        
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_FIV_PARAM,
                            &sysFivParam, sizeof(sysFivParam), channel );
//    	if ( nRet == 0 ) nRet = WriteSysConfig();
    }    

	if ( nRet != 0 ) FiPrint( "SetFivParam failed !\r\n" );

	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取FIV配置
//
static int GetFivConfig( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	            = 0;    
	CONFIG_FIV_CONFIG	    	fivConfig;
	SYS_CONFIG_FIV_CONFIG		sysFivConfig;
	CONFIG_PACK_DATA *        	pConfigPackData	    = ( CONFIG_PACK_DATA * )dataBuf;
	int	                    	channel	            = *(pConfigPackData->data);

	memset( &fivConfig, 0, sizeof(fivConfig) );        
	memset( &sysFivConfig, 0, sizeof(sysFivConfig) );        
	nRet = GetSysConfig( TYPE_SYS_CONFIG_FIV_CONFIG,
                        &sysFivConfig, sizeof(sysFivConfig), channel );
	if ( nRet != -1 ) 
    {
    	fivConfig.channel	= channel;
    	fivConfig.width	    = htonl( sysFivConfig.width );
    	fivConfig.height	= htonl( sysFivConfig.height );
    	fivConfig.left	    = htonl( sysFivConfig.left );
    	fivConfig.top	    = htonl( sysFivConfig.top );
    	fivConfig.right	    = htonl( sysFivConfig.right );
    	fivConfig.bottom	= htonl( sysFivConfig.bottom );
    	fivConfig.fivFlag	= htonl( sysFivConfig.fivFlag );
    	fivConfig.fivType	= htonl( sysFivConfig.fivType );
        
    	for ( int i = 0; i < FIV_MAX_RECT_NUM; ++i )
        {
        	fivConfig.rgn[i].rgnType	= htonl( sysFivConfig.rgn[i].rgnType );
        	fivConfig.rgn[i].rect.x1	= htonl( sysFivConfig.rgn[i].rect.x1 );
        	fivConfig.rgn[i].rect.y1	= htonl( sysFivConfig.rgn[i].rect.y1 );
        	fivConfig.rgn[i].rect.x2	= htonl( sysFivConfig.rgn[i].rect.x2 );
        	fivConfig.rgn[i].rect.y2	= htonl( sysFivConfig.rgn[i].rect.y2 );
        }
    
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&fivConfig, sizeof(fivConfig),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "GetFivConfig failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;    
}

//
// 设置FIV配置
//
static int SetFivConfig( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	            = 0;
	CONFIG_FIV_CONFIG	    	fivConfig;
	SYS_CONFIG_FIV_CONFIG		sysFivConfig;
	CONFIG_PACK_DATA *        	pConfigPackData	    = ( CONFIG_PACK_DATA * )dataBuf;
	int	                    	channel	            = 0;
	int	                    	fivFlag	            = FIV_ENABLE_DEFENSE;

	memset( &fivConfig, 0, sizeof(fivConfig) );        
	memset( &sysFivConfig, 0, sizeof(sysFivConfig) );    

	memcpy( (char *)&fivConfig, pConfigPackData->data, sizeof(fivConfig) );    
	channel = fivConfig.channel;
	nRet = GetSysConfig( TYPE_SYS_CONFIG_FIV_CONFIG,
                        &sysFivConfig, sizeof(sysFivConfig), channel );
	if ( nRet == 0 ) 
    {    
    	sysFivConfig.width	    = ntohl( fivConfig.width );
    	sysFivConfig.height	    = ntohl( fivConfig.height );
    	sysFivConfig.left	    = ntohl( fivConfig.left );
    	sysFivConfig.top	    = ntohl( fivConfig.top );
    	sysFivConfig.right	    = ntohl( fivConfig.right );
    	sysFivConfig.bottom	    = ntohl( fivConfig.bottom );
    	sysFivConfig.fivType	= ntohl( fivConfig.fivType );
    	fivFlag	                = ntohl( fivConfig.fivFlag );

    	for ( int i = 0; i < FIV_MAX_RECT_NUM; ++i )
        {
        	sysFivConfig.rgn[i].rgnType	= ntohl( fivConfig.rgn[i].rgnType );
        	sysFivConfig.rgn[i].rect.x1	= ntohl( fivConfig.rgn[i].rect.x1 );
        	sysFivConfig.rgn[i].rect.y1	= ntohl( fivConfig.rgn[i].rect.y1 );
        	sysFivConfig.rgn[i].rect.x2	= ntohl( fivConfig.rgn[i].rect.x2 );
        	sysFivConfig.rgn[i].rect.y2	= ntohl( fivConfig.rgn[i].rect.y2 );
        }
        
    	if ( sysFivConfig.fivFlag != fivFlag )
        {
        	nRet = -1;
        	if ( fivFlag == FIV_ENABLE_DEFENSE )
            {
            	nRet = CfgManageOpenSmartDetect( channel );
            }
        	else
        	if ( fivFlag == FIV_DISABLE_DEFENSE )
            {            
            	nRet = CfgManageCloseSmartDetect( channel );
            }
        	if ( nRet == 0 ) sysFivConfig.fivFlag = fivFlag;
        }
    	if ( nRet == 0 ) nRet = SetSysConfig( TYPE_SYS_CONFIG_FIV_CONFIG,
                                        &sysFivConfig, sizeof(sysFivConfig), channel );
//    	if ( nRet == 0 ) nRet = WriteSysConfig();
    }    

	if ( nRet != 0 ) FiPrint( "SetFivConfig failed !\r\n" );

	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取RF433M配置
//
static int GetRf433mConfig( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	            = 0;    
	CONFIG_RF433M_REQ	    	rf433mReq;
	SYS_CONFIG_RF433M_PARAM		sysRf433mConfig;
	CONFIG_PACK_DATA *        	pConfigPackData	    = ( CONFIG_PACK_DATA * )dataBuf;

	memset( &rf433mReq, 0, sizeof(rf433mReq) );        
	memset( &sysRf433mConfig, 0, sizeof(sysRf433mConfig) );    

	memcpy( (char *)&rf433mReq, pConfigPackData->data, sizeof(rf433mReq) );    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_RF433M_CONFIG,
                        &sysRf433mConfig, sizeof(sysRf433mConfig), 0 );                        
	if ( nRet != -1 ) 
    {    
    	if ( rf433mReq.totalFlag == 1 )
        {
        	for ( int i = 0; i < sysRf433mConfig.num; ++i )
            {
            	sysRf433mConfig.alarmIn[i].trigger.capType         = htonl( sysRf433mConfig.alarmIn[i].trigger.capType );
            	sysRf433mConfig.alarmIn[i].trigger.recType	    = htonl( sysRf433mConfig.alarmIn[i].trigger.recType );
            	sysRf433mConfig.alarmIn[i].trigger.triggerFiv	= htons( sysRf433mConfig.alarmIn[i].trigger.triggerFiv );
            }
            
        	unsigned char devNum	= sysRf433mConfig.num;
        	int bufLen = 1 + sizeof(CONFIG_RF433M_CONFIG) * devNum;
        	char *pBuf = (char *)Malloc( bufLen );
        	if ( pBuf != NULL )
            {
            	memcpy( pBuf, &sysRf433mConfig, bufLen );                
            	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                                (unsigned char *)pBuf, bufLen,
                            	dataBuf, dataLen );    
            	Free( pBuf );
            }
        	else
            {
            	nRet = -1;
            }
        }
    	else
        {
        	unsigned char devNum = 1;
        	int bufLen = 1 + sizeof(CONFIG_RF433M_CONFIG);
        	char *pBuf = (char *)Malloc( bufLen );
        	if ( pBuf != NULL )
            {
            	memcpy( pBuf, &devNum, sizeof(devNum) );
            	int i;
            	for ( i = 0; i < sysRf433mConfig.num; ++i )
                {                    
                	if ( memcmp( rf433mReq.id, sysRf433mConfig.alarmIn[i].id, sizeof(rf433mReq.id) ) == 0 )
                    {
                    	sysRf433mConfig.alarmIn[i].trigger.capType         = htonl( sysRf433mConfig.alarmIn[i].trigger.capType );
                    	sysRf433mConfig.alarmIn[i].trigger.recType	    = htonl( sysRf433mConfig.alarmIn[i].trigger.recType );
                    	sysRf433mConfig.alarmIn[i].trigger.triggerFiv	= htons( sysRf433mConfig.alarmIn[i].trigger.triggerFiv );
                    	memcpy( pBuf+sizeof(devNum), &sysRf433mConfig.alarmIn[i], sizeof(CONFIG_RF433M_CONFIG) );
                    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                                            (unsigned char *)pBuf, bufLen,
                                        	dataBuf, dataLen );    
                    	break;
                    }
                }
            	if ( sysRf433mConfig.num == i )
                {
                	PrintHex( "rf433mReq.id:", (unsigned char *)rf433mReq.id, sizeof(rf433mReq.id), "\r\n" );
                	nRet = -1;
                }
                
            	Free( pBuf );
            }
        	else
            {
            	nRet = -1;
            }
        }                    
    }
	if ( nRet == -1 ) PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
	return 0;    
}

static int SetRf433mConfig( unsigned char *dataBuf, int &dataLen, const int bufSize )
{    
	int	                    	nRet	        = 0;    
	CONFIG_RF433M_CONFIG     	rf433mConfig;
	SYS_CONFIG_RF433M_PARAM		sysRf433mConfig;
	CONFIG_PACK_DATA *        	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;

	memset( &rf433mConfig, 0, sizeof(rf433mConfig) );        
	memset( &sysRf433mConfig, 0, sizeof(sysRf433mConfig) );    

	memcpy( (char *)&rf433mConfig, pConfigPackData->data, sizeof(rf433mConfig) );    
	PrintHex( "SetRf433mConfig ID:", (unsigned char *)rf433mConfig.id, 5, "\r\n" );
    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_RF433M_CONFIG,
                        &sysRf433mConfig, sizeof(sysRf433mConfig), 0 );                        
	if ( nRet != -1 ) 
    {
    	int i;
    	for ( i = 0; i < sysRf433mConfig.num; ++i )
        {
        	if( memcmp( sysRf433mConfig.alarmIn[i].id, rf433mConfig.id, sizeof(rf433mConfig.id) ) == 0 )
            {
            	memcpy( &sysRf433mConfig.alarmIn[i], &rf433mConfig, sizeof(sysRf433mConfig.alarmIn[i]) );
            	sysRf433mConfig.alarmIn[i].trigger.triggerFiv     = ntohs( sysRf433mConfig.alarmIn[i].trigger.triggerFiv );
            	sysRf433mConfig.alarmIn[i].trigger.recType	    = ntohl( sysRf433mConfig.alarmIn[i].trigger.recType );
            	sysRf433mConfig.alarmIn[i].trigger.capType	    = ntohl( sysRf433mConfig.alarmIn[i].trigger.capType );

            	nRet = SetSysConfig( TYPE_SYS_CONFIG_RF433M_CONFIG,
                                &sysRf433mConfig, sizeof(sysRf433mConfig), 0 );                        
            	if ( nRet == 0 ) 
                {                        
//                	nRet = WriteSysConfig();
                }
                
            	if ( nRet == 0 ) PackConfigDataPack( MSG_CFG_DATA_RESPONSE, NULL, 0, dataBuf, dataLen );    
            	break;
            }
        }
    	if ( sysRf433mConfig.num == i )
        {
        	PrintHex( "Set rf433m param failed, id:", (unsigned char *)rf433mConfig.id, sizeof(rf433mConfig.id), "\r\n" );
        	nRet = -1;
        }
    }
	if ( nRet == -1 ) PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );    
    
	return 0;
}

//
// 获取Eye配置
//
static int GetEyeConfig( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	            = 0;    
	CONFIG_EYE_CONFIG	    	eyeConfig;
	SYS_CONFIG_EYE_CONFIG		sysEyeConfig;
	CONFIG_PACK_DATA *        	pConfigPackData	    = ( CONFIG_PACK_DATA * )dataBuf;
	int	                    	channel	            = *(pConfigPackData->data);
    
	memset(&eyeConfig, 0, sizeof(eyeConfig));
	memset(&sysEyeConfig, 0, sizeof(sysEyeConfig));

	nRet = GetSysConfig( TYPE_SYS_CONFIG_EYE_CONFIG,
                        &sysEyeConfig, sizeof(sysEyeConfig), channel );
	if ( nRet != -1 ) 
    {
    	eyeConfig.channel	    = channel;
    	eyeConfig.bFaceDetect	= htons( sysEyeConfig.bFaceDetect );
    	eyeConfig.sensitivity	= sysEyeConfig.bslSensitivity;
        
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&eyeConfig, sizeof(eyeConfig),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "GetEyeConfig failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

//
// 设置Eye配置
//
static int SetEyeConfig( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	            = 0;
	CONFIG_EYE_CONFIG	    	eyeConfig;
	SYS_CONFIG_EYE_CONFIG		sysEyeConfig;
	CONFIG_PACK_DATA *        	pConfigPackData	    = ( CONFIG_PACK_DATA * )dataBuf;
	int	                    	channel	            = 0;

	memset(&eyeConfig, 0, sizeof(eyeConfig));
	memset(&sysEyeConfig, 0, sizeof(sysEyeConfig));
    
	memcpy( (char *)&eyeConfig, pConfigPackData->data, sizeof(eyeConfig) );    
	channel = eyeConfig.channel;
	nRet = GetSysConfig( TYPE_SYS_CONFIG_EYE_CONFIG,
                        &sysEyeConfig, sizeof(sysEyeConfig), channel );
	if ( nRet == 0 ) 
    {    
    	sysEyeConfig.bFaceDetect = ntohs( eyeConfig.bFaceDetect );        
    	sysEyeConfig.bslSensitivity = eyeConfig.sensitivity;
        
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_EYE_CONFIG,
                            &sysEyeConfig, sizeof(sysEyeConfig), channel );
//    	if ( nRet == 0 ) nRet = WriteSysConfig();
    }
	if ( nRet != 0 ) FiPrint( "SetEyeConfig failed !\r\n" );

	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

static int ClearPcNum( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int             	nRet = 0;
	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取IO配置
//
static int GetIoConfig( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	            = 0;    
	CONFIG_IO_CONFIG	    	ioConfig;
	SYS_CONFIG_IO_CONFIG		sysIoConfig;

	memset(&ioConfig, 0, sizeof(ioConfig));
	memset(&sysIoConfig, 0, sizeof(sysIoConfig));
	nRet = GetSysConfig( TYPE_SYS_CONFIG_IO_CONFIG,
                        &sysIoConfig, sizeof(sysIoConfig) );
	if ( nRet != -1 ) 
    {    
    	ioConfig.bNormalOpen = htons( sysIoConfig.bNormalOpen );
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&ioConfig, sizeof(ioConfig),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "GetIoConfig failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;    
}

//
//设置IO配置
//
static int SetIoConfig( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	            = 0;
	CONFIG_IO_CONFIG	    	ioConfig;
	SYS_CONFIG_IO_CONFIG		sysIoConfig;
	CONFIG_PACK_DATA *        	pConfigPackData	    = ( CONFIG_PACK_DATA * )dataBuf;

	memset(&ioConfig, 0, sizeof(ioConfig));
	memset(&sysIoConfig, 0, sizeof(sysIoConfig));

	memcpy( (char *)&ioConfig, pConfigPackData->data, sizeof(ioConfig) );    
    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_IO_CONFIG,
                        &sysIoConfig, sizeof(sysIoConfig) );
	if ( nRet == 0 ) 
    {        
    	sysIoConfig.bNormalOpen = ntohs( ioConfig.bNormalOpen );            
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_IO_CONFIG,
                            &sysIoConfig, sizeof(sysIoConfig) );
    	if ( nRet == 0 )
        {
//        	if( nRet == 0 ) nRet = WriteSysConfig();
        }
    }
	if ( nRet != 0 ) FiPrint( "SetIoConfig failed !\r\n" );

	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取次声公共配置
//
static int GetRf433mPbulic( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	            = 0;    
	CONFIG_RF433M_PUBLIC		rf433mPublic;
	SYS_CONFIG_RF433M_PUBLIC	sysRf433mPublic;
    
	memset(&rf433mPublic, 0, sizeof(rf433mPublic));
	memset(&sysRf433mPublic, 0, sizeof(sysRf433mPublic));    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_RF433M_PUBLIC,
                        &sysRf433mPublic, sizeof(sysRf433mPublic) );
	if ( nRet != -1 ) 
    {    
    	rf433mPublic.bSupportRf433m = sysRf433mPublic.bSupportRf433m;    
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&rf433mPublic, sizeof(rf433mPublic),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "GetRf433mPbulic failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;    
}

//
//设置次声公共配置
//
static int SetRf433mPbulic( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	            = 0;
	CONFIG_RF433M_PUBLIC		rf433mPublic;
	SYS_CONFIG_RF433M_PUBLIC	sysRf433mPublic;
	CONFIG_PACK_DATA *        	pConfigPackData	    = ( CONFIG_PACK_DATA * )dataBuf;

	memset(&rf433mPublic, 0, sizeof(rf433mPublic));
	memset(&sysRf433mPublic, 0, sizeof(sysRf433mPublic));

	memcpy( (char *)&rf433mPublic, pConfigPackData->data, sizeof(rf433mPublic) );    
    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_RF433M_PUBLIC,
                        &sysRf433mPublic, sizeof(sysRf433mPublic) );
	if ( nRet == 0 ) 
    {        
    	sysRf433mPublic.bSupportRf433m = rf433mPublic.bSupportRf433m;            
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_RF433M_PUBLIC,
                            &sysRf433mPublic, sizeof(sysRf433mPublic) );
    	if ( nRet == 0 )
        {
//        	nRet = WriteSysConfig();
        }
    }
	if ( nRet != 0 ) FiPrint( "SetRf433mPbulic failed !\r\n" );

	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取LED显示设置
//
static int GetLedShowSetting( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
    int	                    	nRet	            = 0;    
	CONFIG_LEDSHOW_SETTING     	ledShowSetting;
	SYS_CONFIG_LEDSHOW_SETTING  sysLedShowSetting;

	memset(&ledShowSetting, 0, sizeof(ledShowSetting));
	memset(&sysLedShowSetting, 0, sizeof(sysLedShowSetting));    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_LED_SHOW_SETTING,
                        &sysLedShowSetting, sizeof(sysLedShowSetting) );
	if ( nRet != -1 ) 
    {
    	ledShowSetting.displaySetting = htonl( sysLedShowSetting.displaySetting );
    	memcpy( ledShowSetting.customInfo, sysLedShowSetting.customInfo, sizeof(ledShowSetting.customInfo) );

    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&ledShowSetting, sizeof(ledShowSetting),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "GetLedShowSetting failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

//
// 设置LED显示设置
//
static int SetLedShowSetting( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	            = 0;
	CONFIG_LEDSHOW_SETTING     	ledShowSetting;
	SYS_CONFIG_LEDSHOW_SETTING  sysLedShowSetting;
	CONFIG_PACK_DATA *        	pConfigPackData	    = ( CONFIG_PACK_DATA * )dataBuf;

	memset(&ledShowSetting, 0, sizeof(ledShowSetting));
	memset(&sysLedShowSetting, 0, sizeof(sysLedShowSetting));    

	memcpy( (char *)&ledShowSetting, pConfigPackData->data, sizeof(ledShowSetting) );    
    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_LED_SHOW_SETTING,
                        &sysLedShowSetting, sizeof(sysLedShowSetting) );
	if ( nRet == 0 ) 
    {        
    	sysLedShowSetting.displaySetting= ntohl( ledShowSetting.displaySetting );
    	memcpy( sysLedShowSetting.customInfo, ledShowSetting.customInfo, sizeof(sysLedShowSetting.customInfo) );

    	FiPrint( "#################### SetLedShowSetting  ####################\r\n" );
    	FiPrint( "## displaySetting  = %d\r\n", sysLedShowSetting.displaySetting );
    	FiPrint( "## customInfo      = %s\r\n", sysLedShowSetting.customInfo );
    	FiPrint( "######################################################\r\n" );
        
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_LED_SHOW_SETTING,
                            &sysLedShowSetting, sizeof(sysLedShowSetting) );
    	if ( nRet == 0 )
        {
//        	nRet = WriteSysConfig();
        }        
    }
	if ( nRet != 0 ) FiPrint( "SetLedShowSetting failed !\r\n" );

	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取LED单板设置
//
static int GetLedBoardSetting( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	            = 0;    
	CONFIG_LEDBOARD_SETTING 	ledBoardSetting;
	SYS_CONFIG_LEDBOARD_SETTING sysLedBoardSetting;
    
	memset( &ledBoardSetting, 0x00, sizeof(ledBoardSetting) );
	memset( &sysLedBoardSetting, 0x00, sizeof(sysLedBoardSetting) );
    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_LED_BOARD_SETTING,
                        &sysLedBoardSetting, sizeof(sysLedBoardSetting) );
	if ( nRet != -1 ) 
    {
    	ledBoardSetting.move_action         = sysLedBoardSetting.move_action;
    	ledBoardSetting.move_speed             = sysLedBoardSetting.move_speed;
    	ledBoardSetting.rol_frame	        = sysLedBoardSetting.rol_frame;
    	ledBoardSetting.stop_delay	        = sysLedBoardSetting.stop_delay;
    	ledBoardSetting.color	            = sysLedBoardSetting.color;
    	ledBoardSetting.font_class	        = sysLedBoardSetting.font_class;

    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&ledBoardSetting, sizeof(ledBoardSetting),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "GetLedBoardSetting failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

//
// 设置LED单板设置
//
static int SetLedBoardSetting( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	            = 0;
	CONFIG_LEDBOARD_SETTING 	ledBoardSetting;
	SYS_CONFIG_LEDBOARD_SETTING sysLedBoardSetting;

	CONFIG_PACK_DATA *        	pConfigPackData	    = ( CONFIG_PACK_DATA * )dataBuf;

	memset( &ledBoardSetting, 0x00, sizeof(ledBoardSetting) );
	memset( &sysLedBoardSetting, 0x00, sizeof(sysLedBoardSetting) );

	memcpy( (char *)&ledBoardSetting, pConfigPackData->data, sizeof(ledBoardSetting) );    
    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_LED_BOARD_SETTING,
                        &sysLedBoardSetting, sizeof(sysLedBoardSetting) );
	if ( nRet == 0 ) 
    {
    	sysLedBoardSetting.move_action         = ledBoardSetting.move_action;
     	sysLedBoardSetting.move_speed	    = ledBoardSetting.move_speed;
     	sysLedBoardSetting.rol_frame	    = ledBoardSetting.rol_frame;
    	sysLedBoardSetting.stop_delay	    = ledBoardSetting.stop_delay;
    	sysLedBoardSetting.color	        = ledBoardSetting.color;
    	sysLedBoardSetting.font_class	    = ledBoardSetting.font_class;
        
    	FiPrint( "#################### SetLedBoardSetting  ####################\r\n" );
    	FiPrint( "## move_action         = %d\r\n", sysLedBoardSetting.move_action );
    	FiPrint( "## move_speed          = %d\r\n", sysLedBoardSetting.move_speed );    
    	FiPrint( "## rol_frame              = %d\r\n", sysLedBoardSetting.rol_frame );
    	FiPrint( "## stop_delay             = %d\r\n", sysLedBoardSetting.stop_delay );
    	FiPrint( "## color                 = %d\r\n", sysLedBoardSetting.color );
    	FiPrint( "## font_class             = %d\r\n", sysLedBoardSetting.font_class );
    	FiPrint( "######################################################\r\n" );

    	nRet = SetSysConfig( TYPE_SYS_CONFIG_LED_BOARD_SETTING,
                            &sysLedBoardSetting, sizeof(sysLedBoardSetting) );
    	if ( nRet == 0 )
        {
//        	nRet = WriteSysConfig();
        }
    }
	if ( nRet != 0 ) FiPrint( "SetLedBoardSetting failed !\r\n" );

	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取主从设备设置
//
static int GetMasterSlaverSetting( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	            = 0;    
	CONFIG_GROUP_SETTING         groupSetting;
	SYS_CONFIG_GROUP_SETTING    sysGroupSetting;

	memset( &groupSetting, 0x00, sizeof(groupSetting) );
	memset( &sysGroupSetting, 0x00, sizeof(sysGroupSetting) );
    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_GROUP_SETTING,
                        &sysGroupSetting, sizeof(sysGroupSetting) );
	if ( nRet != -1 ) 
    {
    	groupSetting.enable	    = sysGroupSetting.enable;
    	groupSetting.masterTag	= sysGroupSetting.masterTag;
    	memcpy( groupSetting.ip, sysGroupSetting.ip, sizeof(groupSetting.ip) );
    	groupSetting.syncInterval= htons( sysGroupSetting.syncInterval );
    	groupSetting.clrHour     = sysGroupSetting.clrHour;
    	groupSetting.clrMinute   = sysGroupSetting.clrMinute;        
    	groupSetting.limitPeople = htonl( sysGroupSetting.limitPeople );
    	groupSetting.alarmPeople = htonl( sysGroupSetting.alarmPeople );

    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&groupSetting, sizeof(groupSetting),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "GetGroupSetting failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;
}

//
// 设置主从设备设置
//
static int SetMasterSlaverSetting( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	            = 0;
	CONFIG_GROUP_SETTING         groupSetting;
	SYS_CONFIG_GROUP_SETTING    sysGroupSetting;
	CONFIG_PACK_DATA *        	pConfigPackData	    = ( CONFIG_PACK_DATA * )dataBuf;

	memset( &groupSetting, 0x00, sizeof(groupSetting) );
	memset( &sysGroupSetting, 0x00, sizeof(sysGroupSetting) );    

	memcpy( (char *)&groupSetting, pConfigPackData->data, sizeof(groupSetting) );    
    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_GROUP_SETTING,
                        &sysGroupSetting, sizeof(sysGroupSetting) );
	if ( nRet == 0 ) 
    {
    	int bMaster1, bMaster2;
    	bMaster1 = bMaster2 = 0;
    	if ( sysGroupSetting.masterTag )
        	bMaster1 = 1;
    	if ( groupSetting.masterTag )
        	bMaster2 = 1;
    
    	sysGroupSetting.enable	    = groupSetting.enable;
    	sysGroupSetting.masterTag	= groupSetting.masterTag;
    	memcpy( sysGroupSetting.ip, groupSetting.ip, sizeof(sysGroupSetting.ip) );
    	sysGroupSetting.syncInterval= ntohs( groupSetting.syncInterval );
    	sysGroupSetting.clrHour     = groupSetting.clrHour;
    	sysGroupSetting.clrMinute   = groupSetting.clrMinute;
    	sysGroupSetting.limitPeople = ntohl( groupSetting.limitPeople );
    	sysGroupSetting.alarmPeople	= ntohl( groupSetting.alarmPeople );
            
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_GROUP_SETTING,
                            &sysGroupSetting, sizeof(sysGroupSetting) );
    	if ( nRet == 0 )
        {
            
//        	nRet = WriteSysConfig();
        }
    }
	if ( nRet != 0 ) FiPrint( "SetGroupSetting failed !\r\n" );

	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取乌鲁木齐平台服务器配置
//
static int GetWlPlateform( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	nRet, devId;    
	WLPLATFORM_SERVER_PARAM	wlPlatform;
	SYS_CONFIG_WL_PLATFORM	sysWlPlateform;

	memset( &wlPlatform, 0x00, sizeof(wlPlatform) );
	memset( &sysWlPlateform, 0x00, sizeof(sysWlPlateform) );
        
	nRet = GetSysConfig( TYPE_SYS_CONFIG_WL_PLATFORM,
                        &sysWlPlateform, sizeof(sysWlPlateform) );
	if ( nRet != -1 ) 
    {    
    	strcpy( wlPlatform.ip, sysWlPlateform.ipAddr );    
    	wlPlatform.port = htons( sysWlPlateform.port );

    	devId = 123456;
    	wlPlatform.devId = htonl(devId);
        
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&wlPlatform, sizeof(wlPlatform),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "SetWlPlateform failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;    
}

//
// 设置乌鲁木齐平台服务器配置
//
static int SetWlPlateform( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	            = 0;
	WLPLATFORM_SERVER_PARAM		wlPlatform;
	SYS_CONFIG_WL_PLATFORM		sysWlPlateform;
	CONFIG_PACK_DATA *        	pConfigPackData	    = ( CONFIG_PACK_DATA * )dataBuf;

	memset( &wlPlatform, 0x00, sizeof(wlPlatform) );
	memset( &sysWlPlateform, 0x00, sizeof(sysWlPlateform) );

	memcpy( (char *)&wlPlatform, pConfigPackData->data, sizeof(wlPlatform) );    
    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_WL_PLATFORM,
                        &sysWlPlateform, sizeof(sysWlPlateform) );
	if ( nRet == 0 ) 
    {        
    	sysWlPlateform.port = ntohs(wlPlatform.port);        
    	strcpy( sysWlPlateform.ipAddr, wlPlatform.ip );
        
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_WL_PLATFORM,
                            &sysWlPlateform, sizeof(sysWlPlateform) );
    	if ( nRet == 0 )
        {
//        	nRet = WriteSysConfig();
        }
    }
	if ( nRet != 0 ) FiPrint( "SetWlPlateform failed !\r\n" );

	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取wifi搜索结果
//
static int GetWifiScanReault( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int                 	nRet	            = 0;    
	WIFI_SCAN_RESULT         *pWifiScanResult     = NULL;
	int resultSize;    

	nRet = WifiScanResult( &pWifiScanResult, &resultSize );
	FiPrint2( "GetWifiScanReault :nRet = %d, resultSize=%d!\r\n", nRet, resultSize );
	if ( nRet != -1 ) 
    {        
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)pWifiScanResult, resultSize,
                        	dataBuf, dataLen );    
    }        
	else
    {
    	FiPrint( "GetWifiNetwork failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }

	Free( pWifiScanResult );
	return nRet;    
}  

//
// 获取wifi网络配置
//
static int GetWifiNetwork( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	        = 0;    
	CONFIG_WIFI_NETWORK	    	wifiNetwork;
	PARAM_CONFIG_NETWORK     	network;    

	memset( &wifiNetwork, 0x00, sizeof(wifiNetwork) );

	nRet = ParamGetNetwork( &network );    
	if ( nRet != -1 ) 
    {    
    	wifiNetwork.enableWifi	= network.wifi.enableFlag;
    	wifiNetwork.netType     = !network.wifi.dhcpEnable;
    	memcpy( wifiNetwork.ip, network.wifi.ip, sizeof(wifiNetwork.ip) );
    	memcpy( wifiNetwork.netmask, network.wifi.netmask, sizeof(wifiNetwork.netmask) );
    	memcpy( wifiNetwork.gateway, network.wifi.gateway, sizeof(wifiNetwork.gateway) );    
    	memcpy( wifiNetwork.broadAddr, network.wifi.broadAddr, sizeof(wifiNetwork.broadAddr) );
    	NetGetMacAddr( (char *)NET_WIFI_NAME, wifiNetwork.mac, sizeof(wifiNetwork.mac) );
        
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&wifiNetwork, sizeof(wifiNetwork),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "GetWifiNetwork failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;    
}

//
//设置wifi网络配置
//
static int SetWifiNetwork( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	            = 0;
	CONFIG_WIFI_NETWORK	    	wifiNetwork;
	CONFIG_PACK_DATA *        	pConfigPackData	    = ( CONFIG_PACK_DATA * )dataBuf;
	int                     	bReboot	            = FI_FALSE;    
	PARAM_CONFIG_NETWORK     	network;    

	memset( &wifiNetwork, 0x00, sizeof(wifiNetwork) );
	memcpy( (char *)&wifiNetwork, pConfigPackData->data, sizeof(wifiNetwork) );        
	if( nRet != -1 ) 
    {        
    	nRet = ParamGetNetwork( &network ); 
    }
	if ( nRet == 0 ) 
    {        
    	network.wifi.enableFlag = wifiNetwork.enableWifi;
    	network.wifi.dhcpEnable = !wifiNetwork.netType;
    	memcpy( network.wifi.ip, wifiNetwork.ip, sizeof(network.wifi.ip) );
    	memcpy( network.wifi.netmask, wifiNetwork.netmask, sizeof(network.wifi.netmask) );
    	memcpy( network.wifi.gateway, wifiNetwork.gateway, sizeof(network.wifi.gateway) );    
    	memcpy( network.wifi.broadAddr, wifiNetwork.broadAddr, sizeof(network.wifi.broadAddr) );

    	nRet = ParamSetNetwork( &network ); 
    }
	if ( nRet != 0 ) FiPrint( "SetWifiNetwork failed !\r\n" );

	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );

	if( nRet != -1 && FI_TRUE == bReboot ) RebootSystem();
    
	return nRet;
}

//
// 获取wifi连接配置
//
static int GetWifiConnect( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	        = 0;    
	CONFIG_WIFI_CONNECT	    	wifiConnect;
	PARAM_CONFIG_WIFI_CONNECT_T paramWifiConnect;    

	memset( &wifiConnect, 0x00, sizeof(wifiConnect) );
	nRet = ParamGetWifiConnect( &paramWifiConnect );    
	if ( nRet != -1 ) 
    {        
    	WifiStateGet( &wifiConnect.connectFlag, &wifiConnect.signalLevel );
    	memcpy( wifiConnect.essid, paramWifiConnect.essid, sizeof(wifiConnect.essid) );
    	memcpy( wifiConnect.key, paramWifiConnect.key, sizeof(wifiConnect.key) );
        
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&wifiConnect, sizeof(wifiConnect),
                        	dataBuf, dataLen );
    }        
	else
    {
    	FiPrint( "GetWifiConnect failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;    
}

//
//设置wifi连接配置
//
static int SetWifiConnect( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int                     	nRet	            = 0;
	CONFIG_WIFI_CONNECT     	wifiConnect;
	CONFIG_PACK_DATA *        	pConfigPackData     = ( CONFIG_PACK_DATA * )dataBuf;
	PARAM_CONFIG_WIFI_CONNECT_T paramWifiConnect;    

	memset( &wifiConnect, 0x00, sizeof(wifiConnect) );
	memcpy( (char *)&wifiConnect, pConfigPackData->data, sizeof(wifiConnect) ); 
    
	if( nRet != -1 )
    {
    	nRet = ParamGetWifiConnect( &paramWifiConnect );
    }
	if ( nRet == 0 ) 
    {        
        {
        	nRet = WifiSetConncetInfoToFile( wifiConnect.essid, wifiConnect.key );            
        	if( nRet == 0 ) 
            {
            	memcpy( paramWifiConnect.essid, wifiConnect.essid, sizeof(paramWifiConnect.essid) );
            	memcpy( paramWifiConnect.key, wifiConnect.key, sizeof(paramWifiConnect.key) );
            	nRet = ParamSetWifiConnect( &paramWifiConnect );        
            }
        }            
    }
    
	if ( nRet == 0 ) nRet = WifiFifoReStart();
	if ( nRet != 0 ) FiPrint( "SetWifiConnect failed !\r\n" );

	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );

	return nRet;
}

/*synovate_add*/
static int GetSynovateParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	char tmpIpBuf[42];
	char tmp1Buf[7];
	char tmp2Buf[7];
	char tmp3Buf[7];
	int i;
	int	            	nRet	    = 0;
	SYS_CONFIG_SYNOVATE		netSynovate;
	SYS_CONFIG_SYNOVATE		sysSynovate;

	memset( &netSynovate, 0x00, sizeof(netSynovate) );
	memset( &sysSynovate, 0x00, sizeof(sysSynovate) );

	memset(tmp1Buf,0x00,sizeof(tmp1Buf));
	memset(tmp2Buf,0x00,sizeof(tmp2Buf));    
	memset(tmp3Buf,0x00,sizeof(tmp3Buf));    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_SYNOVATE, &sysSynovate, sizeof(sysSynovate) );
	if ( nRet != -1 ) 
    {
    	netSynovate.setdaylight = htonl(sysSynovate.setdaylight);
    	memcpy(netSynovate.synovateIp,sysSynovate.synovateIp,sizeof(netSynovate.synovateIp));
    	netSynovate.synovatePort = htons(sysSynovate.synovatePort);

    	netSynovate.fdaylight = sysSynovate.fdaylight;
    	for(i=0;i<6;i++)
        {
        	netSynovate.start[i] = sysSynovate.start[i];
        	netSynovate.end[i] = sysSynovate.end[i];

        	tmp1Buf[i] = sysSynovate.start[i];
        	tmp2Buf[i] = sysSynovate.end[i];
        	tmp3Buf[i] = netSynovate.sn[i];
        }

    	netSynovate.ftypedaylight = sysSynovate.ftypedaylight;    
    	netSynovate.startMonth = htonl(sysSynovate.startMonth);
    	netSynovate.startWhichDis = htonl(sysSynovate.startWhichDis);
    	netSynovate.startWhichDay = htonl(sysSynovate.startWhichDay);
    	netSynovate.startHour = htonl(sysSynovate.startHour);
        
    	netSynovate.endMonth = htonl(sysSynovate.endMonth);
    	netSynovate.endWhichDis = htonl(sysSynovate.endWhichDis);
    	netSynovate.endWhichDay = htonl(sysSynovate.endWhichDay);
    	netSynovate.endHour = htonl(sysSynovate.endHour);
        
    	if(0!=netSynovate.synovateIp[0])
        {
        	memset(tmpIpBuf,0x00,sizeof(tmpIpBuf));
        	memcpy(tmpIpBuf,sysSynovate.synovateIp,sizeof(sysSynovate.synovateIp));
        	FiPrint( "## ip              = %s\r\n", tmpIpBuf );
        }
    	else
        {
        	FiPrint( "## ip              = not set\r\n");
        }
        
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&netSynovate, sizeof(netSynovate),
                        	dataBuf, dataLen );
    	FiPrint( "Get Synovate Param OK !\r\n" );
    }
	else
    {
    	FiPrint( "Get Synovate Param failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
    
	return nRet;
}

/*synovate_add*/
static int SetSynovateParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	char tmp1Buf[7];
	char tmp2Buf[7];
	char tmp3Buf[7];
	int i;
	int	                        	nRet	        = -1;    
	CONFIG_PACK_DATA *pConfigPackData = ( CONFIG_PACK_DATA * )dataBuf;
	SYS_CONFIG_SYNOVATE		netSynovate;
	SYS_CONFIG_SYNOVATE		sysSynovate;

	memset( &netSynovate, 0x00, sizeof(netSynovate) );
	memset( &sysSynovate, 0x00, sizeof(sysSynovate) );

	memset(tmp1Buf,0x00,sizeof(tmp1Buf));
	memset(tmp2Buf,0x00,sizeof(tmp2Buf));    
	memset(tmp3Buf,0x00,sizeof(tmp3Buf));
	FiPrint( "#################### Set Synovate Param  ####################\r\n" );    
	memcpy( (char *)&netSynovate, pConfigPackData->data, sizeof(netSynovate) );

    //sysSynovate.setdaylight	    = ntohl( netSynovate.setdaylight );
	sysSynovate.synovatePort   =  ntohs( netSynovate.synovatePort );
	memcpy(sysSynovate.synovateIp, netSynovate.synovateIp, sizeof(sysSynovate.synovateIp) );

	sysSynovate.ftypedaylight	= netSynovate.ftypedaylight;

	sysSynovate.startMonth	    = ntohl( netSynovate.startMonth );
	sysSynovate.startWhichDis	= ntohl( netSynovate.startWhichDis );
	sysSynovate.startWhichDay	= ntohl( netSynovate.startWhichDay );
	sysSynovate.startHour	    = ntohl( netSynovate.startHour );

	sysSynovate.endMonth	    = ntohl( netSynovate.endMonth );
	sysSynovate.endWhichDis	    = ntohl( netSynovate.endWhichDis );
	sysSynovate.endWhichDay	    = ntohl( netSynovate.endWhichDay );
	sysSynovate.endHour	        = ntohl( netSynovate.endHour );

	sysSynovate.fdaylight = netSynovate.fdaylight;
	sysSynovate.Ch = netSynovate.Ch;
	for(i=0;i<6;i++)
    {
    	sysSynovate.sn[i] = netSynovate.sn[i];
    	sysSynovate.start[i] = netSynovate.start[i];
    	sysSynovate.end[i] =  netSynovate.end[i];
    }

	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 获取ftp录像相关参数
//
static int GetFtpRecParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	        = 0;    
	CONFIG_FTP_REC	    	ftpRec;
	PARAM_CONFIG_FTP     	ftpParam;    

	memset( &ftpRec, 0x00, sizeof(ftpRec) );
	nRet = ParamGetFtp( &ftpParam );
	if ( nRet != -1 ) 
    {        
    	ftpRec.recFlag = ftpParam.enable;
    	snprintf( ftpRec.host, sizeof(ftpRec.host), "ftp://%s", ftpParam.ip );        
    	snprintf( ftpRec.user, sizeof(ftpRec.host), "%s", ftpParam.user );
    	snprintf( ftpRec.passwd, sizeof(ftpRec.passwd), "%s", ftpParam.passwd );
    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&ftpRec, 
                        	sizeof(ftpRec),
                        	dataBuf,
                        	dataLen );
    }        
	else
    {
    	FiPrint( "GetWifiConnect failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
	return nRet;    
}

//
// 设置ftp录像相关参数
//
static int SetFtpRecParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	        = -1;
	CONFIG_FTP_REC	        	ftpRec;
	PARAM_CONFIG_FTP         	ftpParam;    
	CONFIG_PACK_DATA *        	pConfigPackData	= ( CONFIG_PACK_DATA * )dataBuf;

	memcpy((char *)&ftpRec, pConfigPackData->data, sizeof(ftpRec));    
	if(strlen(ftpRec.host) <= 0 || strlen(ftpRec.user) <= 0 || strlen(ftpRec.passwd) <= 0)
    {
    	FiPrint( "Set Ftp Rec failed !\r\n" );
    	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
    	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
    	return nRet;
    }
    
	nRet = ParamGetFtp( &ftpParam );
	if ( nRet == 0 ) 
    {    
    	ftpParam.enable = ftpRec.recFlag;
    	snprintf( ftpParam.ip, sizeof(ftpParam.ip), "%s", ftpRec.host+strlen("ftp://") );        
    	snprintf( ftpParam.user, sizeof(ftpParam.user), "%s", ftpRec.user );
    	snprintf( ftpParam.passwd, sizeof(ftpParam.passwd), "%s", ftpRec.passwd );
    	nRet = ParamSetFtp( &ftpParam );
    	if( 0 == nRet )
        {
        	FtpJpgSendParamChangeMessage();
        }
    }
	if ( nRet != 0 ) FiPrint( "Set Ftp Rec failed !\r\n" );

	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );
	return nRet;
}

//
// 上传报警信息数据包
//
int GetAlarmPack( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	DVS_ALARM		alarmInfo;
	int         	channel	    = 0;
	time_t         	curTime	    = time( NULL );
	struct tm        *ptime ;
	char alarmstr[] = " Too many people stay here ";

	memset( &alarmInfo, 0x00, sizeof(alarmInfo) );
    
	memcpy( &channel, dataBuf, sizeof( channel ) );
	alarmInfo.channel = htonl( channel );
	alarmInfo.alarmType =	htonl( MSG_ALARM_PC_UPPER_LIMIT );
	ptime = gmtime( &curTime );                    
	snprintf( alarmInfo.alarmTime, sizeof( alarmInfo.alarmTime ), 
                    "%4d-%02d-%02d %02d:%02d:%02d",
                	ptime->tm_year+1900, ptime->tm_mon+1, ptime->tm_mday, 
                	ptime->tm_hour, ptime->tm_min, ptime->tm_sec);
	memcpy( alarmInfo.alarmInfo , alarmstr, sizeof( alarmstr ) );
                    
	FirsPackDataPack( MSG_DATA_FLAG , MSG_UPL_DATA_TYPE , 1,  MSG_CFG_UPLOAD_ALARM, 
                        	MSG_CFG_DATA_ALARM, (unsigned char *)&alarmInfo, 
                        	sizeof( alarmInfo ), dataBuf, dataLen );
	return 0;    
}

static int GetRlwTxtPath(unsigned char *dataBuf, int &dataLen, const int bufSize)
{
    int nRet    = -1;
    int filelen = 0;
    RLW_TXT_PATH txtpath;

    if(1) //(RlwGetTxtInfo(&filelen,txtpath.txtpath) == 0)
    {
        txtpath.nFileLen = htonl(filelen);
        nRet = 0;
        PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&txtpath, sizeof(RLW_TXT_PATH),
                        	dataBuf, dataLen );
    }
    else
    {
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }
    return nRet;
}

/* 客户端获取服务端Ftp上传客流信息函数 */
static int GetFtpUploadParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	        = 0;    
	CONFIG_FTP_UPLOAD	    	ftpUpload;
	SYS_CONFIG_FTP_UPLOAD		sysFtpUpload;

    
	memset( &ftpUpload, 0x00, sizeof(ftpUpload) );
	memset( &sysFtpUpload, 0x00, sizeof(sysFtpUpload) );
	nRet = CfgManageGetFtpUploadConfig(&sysFtpUpload);
	if (-1 != nRet) 
    {        
        //memcpy(&ftpUpload, &sysFtpUpload, sizeof(ftpUpload));
        //转化为网络端传输
    	memcpy( ftpUpload.strHost, sysFtpUpload.strHost, sizeof(ftpUpload.strHost) );
    	memcpy( ftpUpload.strUserName, sysFtpUpload.strUserName, sizeof(ftpUpload.strUserName) );
    	memcpy( ftpUpload.strPassword, sysFtpUpload.strPassword, sizeof(ftpUpload.strPassword) );

    	ftpUpload.nUploadInterval	= htonl( sysFtpUpload.nUploadInterval);
    	ftpUpload.nUploadFileFormat	= htonl( sysFtpUpload.nUploadFileFormat);
    	ftpUpload.nRunFlag	        = htonl( sysFtpUpload.nRunFlag);
    	ftpUpload.nPort	            = htonl( sysFtpUpload.nPort);

    	PackConfigDataPack( MSG_CFG_DATA_RESPONSE,
                            (unsigned char *)&ftpUpload, 
                        	sizeof(ftpUpload),
                        	dataBuf,
                        	dataLen );
    }        
	else
    {
    	FiPrint( "GetFtpUploadParam failed !\r\n" );
    	PackConfigDataPack( MSG_CFG_DATA_ERROR, NULL, 0, dataBuf, dataLen );
    }

	return nRet;

}

/* 客户端设置服务端Ftp上传客流信息函数 */
static int SetFtpUploadParam( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                    	nRet	            = -1;
	CONFIG_FTP_UPLOAD	    	ftpUpload;
	SYS_CONFIG_FTP_UPLOAD		sysftpUpload;
	CONFIG_PACK_DATA *        	pConfigPackData	    = ( CONFIG_PACK_DATA * )dataBuf;

	memset( &ftpUpload, 0x00, sizeof(ftpUpload) );
	memset( &sysftpUpload, 0x00, sizeof(sysftpUpload) );

	memcpy((char *)&ftpUpload, pConfigPackData->data, sizeof(ftpUpload));

	nRet = CfgManageGetFtpUploadConfig(&sysftpUpload);

	if (-1 != nRet)
    {
        //从网络端转化为机器端
    	memcpy( sysftpUpload.strHost, ftpUpload.strHost, sizeof(sysftpUpload.strHost) );
    	memcpy( sysftpUpload.strUserName, ftpUpload.strUserName, sizeof(sysftpUpload.strUserName) );
    	memcpy( sysftpUpload.strPassword, ftpUpload.strPassword, sizeof(sysftpUpload.strPassword) );
        
    	sysftpUpload.nUploadInterval	= ntohl( ftpUpload.nUploadInterval);
    	sysftpUpload.nUploadFileFormat	= ntohl( ftpUpload.nUploadFileFormat);
    	sysftpUpload.nRunFlag	        = ntohl( ftpUpload.nRunFlag);
    	sysftpUpload.nPort	            = ntohl( ftpUpload.nPort);
        
    	nRet = CfgManageSetFtpUploadConfig( (SYS_CONFIG_FTP_UPLOAD *)&sysftpUpload );
    	if (0 != nRet)
        {
        	FiPrint( "Set FtpUpload Config failed !\r\n" );
        }
    	else
        {
        	FiPrint( "Set FtpUpload Config success !\r\n" );
        }    
        
    }

	unsigned char subType = ( nRet != -1 ) ? MSG_CFG_DATA_RESPONSE : MSG_CFG_DATA_ERROR;
	PackConfigDataPack( subType, NULL, 0, dataBuf, dataLen );

	return nRet;
}



//
// 配置处理函数列表, 根据协议不断扩展.
//
static ID_CMD s_ConfigDataIdCmd[] = 
{
    { MSG_CFG_HEART_BEAT,            	DealHeartBeat	    },
    { MSG_CFG_GET_SYS_INFO,            	GetSysInfo	        },
    { MSG_CFG_RESET_DEFAULT_PARAM,    	ResetDefaultParam	},
    { MSG_CFG_GET_SYS_TIME,            	GetSysTime	        },
    { MSG_CFG_SET_SYS_TIME,            	SetSysTime	        },
    { MSG_CFG_GET_NET_PARAM,        	GetNetParam	        },
    { MSG_CFG_SET_NET_PARAM,        	SetNetParam	        }, 
    { MSG_CFG_GET_VIDEO_PARAM,        	GetVideoParam	    },
    { MSG_CFG_SET_VIDEO_PARAM,        	SetVideoParam	    },
    { MSG_CFG_GET_USER_PARAM,        	GetUserParam	    },
    { MSG_CFG_SET_USER_PARAM,        	SetUserParam	    },
    { MSG_CFG_GET_PTZ_CONTROL,        	GetPtzControl	    },
    { MSG_CFG_SET_PTZ_CONTROL,        	SetPtzControl	    },
    { MSG_CFG_GET_PTZ_PROTOCOL,        	GetPtzProtocol	    }, 
    { MSG_CFG_SET_PTZ_PROTOCOL,        	SetPtzProtocol	    }, 
    { MSG_CFG_GET_NTP_PARAM,        	GetNtpParam	        }, 
    { MSG_CFG_SET_NTP_PARAM,        	SetNtpParam	        }, 
    { MSG_CFG_GET_OSD_PARAM,        	GetOsdParam	        }, 
    { MSG_CFG_SET_OSD_PARAM,        	SetOsdParam	        }, 
    { MSG_CFG_GET_EMAIL_PARAM,        	GetEmailParam	    }, 
    { MSG_CFG_SET_EMAIL_PARAM,        	SetEmailParam	    }, 
    { MSG_CFG_GET_AUDIO_ENCODE,        	GetAudioEncode	    },
    { MSG_CFG_SET_AUDIO_ENCODE,        	SetAudioEncode	    },
    { MSG_CFG_GET_VIDEO_ENCODE,        	GetVideoEncode	    },
    { MSG_CFG_SET_VIDEO_ENCODE,        	SetVideoEncode	    },
    { MSG_CFG_GET_CHANNEL_PARAM,    	GetChannelParam	    },
    { MSG_CFG_SET_CHANNEL_PARAM,    	SetChannelParam	    },
    { MSG_CFG_GET_AUTO_MAINTAIN,    	GetAutoMaintain	    },
    { MSG_CFG_SET_AUTO_MAINTAIN,    	SetAutoMaintain	    },
    { MSG_CFG_GET_RECORD_PARAM,        	GetRecordParam	    },
    { MSG_CFG_SET_RECORD_PARAM,        	SetRecordParam	    },
    { MSG_CFG_SEARCH_PTZ_PROTOCOL,    	SearchPtzProtocol	},
    { MSG_CFG_GET_SYS_BASE,            	GetSysBase	        },
    { MSG_CFG_SET_SYS_BASE,            	SetSysBase	        },
    { MSG_CFG_SET_PTZ_CMD,            	SetPtzCmd	        },
    { MSG_CFG_GET_SD_CARD_INFO,        	GetSdCardInfo	    },
    { MSG_CFG_GET_USB_HD_INFO,        	GetUsbHdInfo	    },    
    { MSG_CFG_GET_ALARM_PARAM,        	GetAlarmParam	    },
    { MSG_CFG_SET_ALARM_PARAM,        	SetAlarmParam	    },        
    { MSG_CFG_GET_RS232_PARAM,        	GetRs232Param	    },
    { MSG_CFG_SET_RS232_PARAM,        	SetRs232Param	    },    
    { MSG_CFG_REBOOT_SYSTEM,        	DealRebootSystem	},
    { MSG_CFG_SEND_EMAIL,            	DealSendEmail	    },
    { MSG_CFG_AUTH_RAND_NUM,        	GetAuthRandNum	    },
    { MSG_CFG_LOGIN_AUTH,            	DealLoginAuth	    },
    { MSG_CFG_SEARCH_RECORD,        	SearchRecord	    },
    { MSG_CFG_SEARCH_SNAP_FILE,     	SearchSnapFile	    },
    { MSG_CFG_GET_PC_PARAM,         	GetPcParam	        },
    { MSG_CFG_SET_PC_PARAM,         	SetPcParam	        },
    { MSG_CFG_GET_LICENCE_INFO,     	GetLicenceInfo	    },
    { MSG_CFG_GET_MIDSYS_CONFIG,    	GetMidWareParam	    },
    { MSG_CFG_SET_MIDSYS_CONFIG,    	SetMidWareParam	    },    
    { MSG_CFG_GET_DOOR_RULE,         	GetDoorRuleParam	},
    { MSG_CFG_SET_DOOR_RULE,         	SetDoorRuleParam	},    
    { MSG_CFG_GET_PC_CONFIG,         	GetPcConfig	        },
    { MSG_CFG_SET_PC_CONFIG,         	SetPcConfig	        },
    { MSG_CFG_GET_LP_PARAM,         	GetLpParam	        },
    { MSG_CFG_SET_LP_PARAM,         	SetLpParam	        },
    { MSG_CFG_GET_DATA_UPLOAD,         	GetDataUpload	    },
    { MSG_CFG_SET_DATA_UPLOAD,         	SetDataUpload	    },
    { MSG_CFG_GET_ABDOOR_PARAM,     	GetABDoorParam	    },
    { MSG_CFG_SET_ABDOOR_PARAM,     	SetABDoorParam	    },    
    { MSG_CFG_GET_FIV_PARAM,         	GetFivParam	        },
    { MSG_CFG_SET_FIV_PARAM,         	SetFivParam	        },    
    { MSG_CFG_GET_FIV_CONFIG,         	GetFivConfig	    },
    { MSG_CFG_SET_FIV_CONFIG,         	SetFivConfig	    },        
    { MSG_CFG_GET_RF433_CONFIG,        	GetRf433mConfig	    },
    { MSG_CFG_SET_RF433_CONFIG,        	SetRf433mConfig	    },
    { MSG_CFG_GET_EYE_CONFIG,         	GetEyeConfig	    },
    { MSG_CFG_SET_EYE_CONFIG,         	SetEyeConfig	    },    
    { MSG_CFG_CLEAR_PC_NUM,            	ClearPcNum	        },
    { MSG_CFG_GET_IO_CONFIG,         	GetIoConfig	        },
    { MSG_CFG_SET_IO_CONFIG,         	SetIoConfig	        },    
    { MSG_CFG_GET_RF433M_PUBLIC,    	GetRf433mPbulic     },
    { MSG_CFG_SET_RF433M_PUBLIC,    	SetRf433mPbulic	    },
    { MSG_CFG_GET_LED_SHOW_SETTING,     GetLedShowSetting   },
    { MSG_CFG_SET_LED_SHOW_SETTINT,     SetLedShowSetting   },
    { MSG_CFG_GET_LED_BOARD_SETTING,	GetLedBoardSetting  },
    { MSG_CFG_SET_LED_BOARD_SETTING,	SetLedBoardSetting  },
    { MSG_CFG_GET_MASTERSLAVER_SETTING, GetMasterSlaverSetting	},
    { MSG_CFG_SET_MASTERSLAVER_SETTING, SetMasterSlaverSetting	},
    { MSG_CFG_GET_URUMCHILP_SERVER_PARAM, GetWlPlateform	    },    
    { MSG_CFG_SET_URUMCHILP_SERVER_PARAM, SetWlPlateform	    },    
    { MSG_CFG_GET_THREEG_STATE,        	GetThreegState	    },
    { MSG_CFG_GET_THREEG_PARAM,        	GetThreegParam	    },
    { MSG_CFG_SET_THREEG_PARAM,        	SetThreegParam	    },
    { MSG_CFG_GET_WIFI_SCAN_RESULT,    	GetWifiScanReault	},
    { MSG_CFG_GET_WIFI_NETWORK_CONFIG, 	GetWifiNetwork	    },
     { MSG_CFG_SET_WIFI_NETWORK_CONFIG,	SetWifiNetwork	    },
    { MSG_CFG_GET_WIFI_CONNECT_CONFIG,  GetWifiConnect	    },
     { MSG_CFG_SET_WIFI_CONNECT_CONFIG,	SetWifiConnect	    },
    {MSG_CFG_GET_SYNOVATE_CONFIG,    	GetSynovateParam	},/*synovate_add*/
    {MSG_CFG_SET_SYNOVATE_CONFIG,    	SetSynovateParam	},/*synovate_add*/
     { MSG_CFG_GET_FTP_REC,            	GetFtpRecParam	    },
     { MSG_CFG_SET_FTP_REC,            	SetFtpRecParam	    },
     { MSG_CFG_GET_RS485_PARAM,        	GetRs485Param	    },
    { MSG_CFG_SET_RS485_PARAM,        	SetRs485Param	    },
    { MSG_CFG_GET_FTP_UPLOAD,        	GetFtpUploadParam	},/* Add by lpx */
    { MSG_CFG_SET_FTP_UPLOAD,        	SetFtpUploadParam	},/* Add by lpx */
    { MSG_CFG_GET_RLW_TXTPATH,          GetRlwTxtPath       },
    { MSG_CFG_UPLOAD_ALARM ,        	GetAlarmPack	    },
};

//
// 对配置数据包进行数据处理
// 根据包类型调用相应的处理函数
//
int DealConfigDataProcess( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int nRet = -1;
	CONFIG_PACK_DATA *pConfigPackData = ( CONFIG_PACK_DATA * )dataBuf;
	int nConfigPackLen = GetCfgPackLen( pConfigPackData );
	CIdCmd dealConfigData;
	dealConfigData.Init( s_ConfigDataIdCmd, sizeof(s_ConfigDataIdCmd)/sizeof(ID_CMD) );

	if ( IsCheckSumOK( dataBuf, nConfigPackLen ) )
    {
    	int cmdId = ntohl( pConfigPackData->head.packType );
    	ID_CMD_ACTION DealConfigDataCmd = dealConfigData.GetCmd( cmdId );
    	if ( DealConfigDataCmd != NULL ) 
        {
            //FiPrint("cmdId is %d\r\n",cmdId);
        	nRet = DealConfigDataCmd( dataBuf, dataLen, bufSize );
        }
    	if ( nRet < 0 ) FiPrint( "DealConfigDataProcess Error: Cmd ID = 0x%04X !\r\n", cmdId );
    }
	else
    {
    	FiPrint( "Check Sum Error !\r\n" );
    }
	return 0;
}

//
// 对配置数据包进行合法性检查
//
int CheckConfigDataProcess( unsigned char *dataBuf, int dataLen, int bufSize, int &offset )
{
	int	            	nRet	    = -1;
	CONFIG_PACK_DATA *	pConfigPack	= (CONFIG_PACK_DATA *)dataBuf;
	int	            	packLen	    = GetCfgPackLen( pConfigPack );

	if ( packLen > bufSize )
    {
    	offset = sizeof( CONFIG_PACK_HEAD );  // 包长出错, 丢弃包头.
    	FiPrint( "Check Message Pack Length Error !\r\n" );
    }
	else
	if ( packLen <= dataLen )
    {
    	if ( IsCheckSumOK(dataBuf, packLen) )
        {
        	if ( pConfigPack->head.msgType == MSG_CFG_DATA_TYPE )
            {
            	nRet = packLen;
            	offset = 0;
            }
        	else
            {
            	offset = packLen;
            	FiPrint( "Check Message Type Error !\r\n" );
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

