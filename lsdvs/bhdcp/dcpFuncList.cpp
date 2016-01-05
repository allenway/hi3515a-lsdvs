/********************************************************************************
**	Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**	author        :  sven
**	version       :  v1.0
**	date           :  2013.10.10
**	description  : 处理协议指令的函数清单
********************************************************************************/

#include <string.h>
#include "debug.h"
#include "linuxFile.h"
#include "paramManage.h"

#include "dcpCom.h"
#include "dcpSignal.h"
#include "dcpFuncList.h"
#include "dcpStream.h"
#include "dcpSearchRecord.h"
#include "dcpSearchLog.h"
#include "ptpool.h"
#include "dcpUpdate.h"
#include "log.h"
#include "dcpRecDownload.h"
#include "fit.h"
#include "record.h"
#include "recordDel.h"
#include "driver.h"
#include "netWire.h"
#include "alarm.h"
#include "network.h"
#include "netCom.h"
#include "netMac.h"
#include "netIp.h"
#include "netGateway.h"
#include "netMask.h"
#include "hdd.h"
#include "malloc.h"
#include "realTime.h"
#include "timeExchange.h"
#include "rtc.h"
#include "bhdcp.h"
#include "message.h"
#include "sysRunTime.h"
#include "reboot.h"
#include "vencParamEasy.h"


static unsigned int GetSeqForMsgHead(DCP_HEAD_T *msgHead, unsigned short msgType)
{
	static unsigned int seq = 0;
	if(msgType < SV_MSG_RES_BASE)    //如果是主动请求,那么递增
    	return ++seq;
	else
    	return msgHead->seq;        //如果是返回,那么把原来的seq返回
}

static void WriteDataToWriteBuf( CLIENT_COMMUNICATE_T *clientCom, void *data, uint dataLen )
{
	uint leftSize, cpSize;
    
	leftSize = clientCom->writeBufSize - clientCom->writeBufDataSize;
	cpSize = leftSize > dataLen ? dataLen : leftSize;
	memcpy( clientCom->writeBuf + clientCom->writeBufDataSize, data, cpSize );
	clientCom->writeBufDataSize += cpSize;
}

//打包消息头
int PacketClientHead( DCP_HEAD_T *msgHead,unsigned short msgType,unsigned short subType,
                        	unsigned int len,unsigned int ackResult )
{
	msgHead->mark         = CLIENT_MSG_MARK;
	msgHead->seq	    = GetSeqForMsgHead(msgHead, msgType);
	msgHead->msgType     = msgType;
	msgHead->subType	= subType;
	msgHead->ackResult	= ackResult;
	msgHead->len	    = len;

	return 0;    
}

static int VerifyUserLogin( char *pUser, char *pPasswd, LOGIN_DVS_RES_T *pLdr )
{
	int ret = -1, i;
	int cmpRes;
	int userFlag = -1, passwdFlag = -1;
	PARAM_CONFIG_CLIENT_USER clientUser;
    
	cmpRes = Strcmp( pUser, SUPER_USER );
	if( 0 == cmpRes )
    {
    	userFlag = 0;
    	cmpRes = Strcmp( pPasswd, SUPER_PASSWD );
    	if( 0 == cmpRes )
        {
        	passwdFlag = 0;
        	pLdr->level         = 1;
        	pLdr->permission	= 0xFFFFFFFF;
        }        
    }
	else
    {        
    	for( i = 0; i < MAX_CLIENT_USER_NUM; ++i )
        {
        	ret = ParamGetClientUser( i, &clientUser );
        	if( 0 == ret && 1 == clientUser.enable )
            {
            	cmpRes = Strcmp( pUser, clientUser.name );                
            	if( 0 == cmpRes )
                {
                	userFlag = 0;
                	cmpRes = Strcmp( pPasswd, clientUser.pwd );
                	if( 0 == cmpRes )
                    {
                    	passwdFlag = 0;
                    	pLdr->level         = clientUser.level;
                    	cmpRes = Strcmp( pUser, DEV_DEFAULT_USER_NAME );
                    	if( 0 == cmpRes )
                        {
                        	pLdr->permission = USER_PERMISSION_ALL;
                        }
                    	else
                        {
                        	pLdr->permission     = clientUser.permission;
                        }
                    	break;
                    }
                }
            }
        }
    }

	if( 0 == userFlag )
    {
    	if( 0 == passwdFlag )
        {
        	ret = 0;
        }
    	else
        {
        	ret = ERROR_TYPE_PASSWD_WRONG;
        }
    }
	else
    {
    	ret = ERROR_TYPE_USER_WRONG;
    }

	return ret;
}

//处理认证请求
int SVMsgVerifyReq( DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom )
{
	int ret;
	unsigned int 	dataLen;
	unsigned int 	threadId;
	LOGIN_DVS_REQ_T loginDvsReq;
	LOGIN_DVS_RES_T loginDvsRes;
    
	memcpy( &loginDvsReq,clientCom->readBuf+sizeof(DCP_HEAD_T),sizeof(loginDvsReq));

	if( MAX_CLIENT_SIZE <= DcpSignalConnectedUserNum() )
    {
    	ret = ERROR_TYPE_OVER_MAX_USER;
    }
	else
    {
    	ret = VerifyUserLogin( loginDvsReq.user, loginDvsReq.passwd, &loginDvsRes );        
    	if( 0 == ret )
        {
        	ret = DcpCreateStreamThread( &threadId );
        	if( 0 == ret )
            {
            	loginDvsRes.userStreamMark = threadId;
            	clientCom->threadId	= threadId;
            }
        }
    }

	if( 0 == ret )
    {
    	LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO, "%s %s Login Req, threadId:%d, ret %d.",
           loginDvsReq.user, loginDvsReq.passwd, threadId, ret);
    }
	dataLen = sizeof(loginDvsRes);
	PacketClientHead(msgHead, SV_MSG_RES_VARIFY, 0, dataLen, ret);
	WriteDataToWriteBuf( clientCom, (void *)msgHead, sizeof(DCP_HEAD_T) );
	WriteDataToWriteBuf( clientCom, (void *)&loginDvsRes, dataLen );

	if( ret != 0 )
    {
    	ret = DCP_COM_ERR_NEED_CLOSE_SOCKET;
    }
    else
    {
        CORRECTPRINT("user:%s;pswd:%s\n!",loginDvsReq.user,loginDvsReq.passwd);
    }

	return ret;
}

int SVMsgStreamStartReq(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	STREAM_REQ_T streamReq;
	unsigned int headLen     = sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(streamReq);
	memcpy( &streamReq, clientCom->readBuf+headLen, dataLen );

	ret = DcpStartChannelStream( streamReq.userStreamMark, streamReq.channel, clientCom->socket );
	if( 0 == ret )
    {
    	ret = DCP_COM_ERR_NEED_MOVE_SOCKET; // 告诉处理者该socket 对象已经被流发送线程接管,可以不用管理了
    }
	return ret;
}

int SVMsgGetBaseInfo(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	CONFIG_BASE_INFO     	baseInfo;
	PARAM_CONFIG_BASE_INFO 	baseInfoSys;
	unsigned int headLen     = sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(baseInfo);

	ret = ParamGetBaseInfo( &baseInfoSys );
	if(FI_SUCCESS == ret)
    {
    	ret = ERROR_TYPE_SUCCESSFUL;
    	baseInfo = baseInfoSys;
    }
	else
    {
    	ret = ERROR_TYPE_GET_PARAM_FAILED;
    }

	PacketClientHead(msgHead, SV_MSG_RES_GET_BASE_INFO, 0, dataLen, ret);
        
	memcpy(clientCom->writeBuf, msgHead, headLen);
	memcpy(clientCom->writeBuf+headLen, &baseInfo, dataLen);

	clientCom->writeBufDataSize = headLen + dataLen;

	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;
}


int SVMsgGetNetwork(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)    
{
	int ret;
	CONFIG_NETWORK     	network;
	PARAM_CONFIG_NETWORK	networkSys;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(network);

	ret = ParamGetNetwork( &networkSys );
	if(FI_SUCCESS == ret)
    {
    	ret = ERROR_TYPE_SUCCESSFUL;
    	network = networkSys;
    }
	else
    {
    	ret = ERROR_TYPE_GET_PARAM_FAILED;
    }

	PacketClientHead(msgHead, SV_MSG_RES_GET_NETWORK, 0, dataLen, ret);
        
	memcpy(clientCom->writeBuf, msgHead, headLen);
	memcpy(clientCom->writeBuf+headLen, &network, dataLen);

	clientCom->writeBufDataSize = headLen + dataLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;
}

int SVMsgSetNetwork(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom) 
{
	int ret = 0;
	CONFIG_NETWORK 	network, networkOrg;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(network);

    ParamGetNetwork( &networkOrg );

	memcpy(&network, clientCom->readBuf+headLen, dataLen);
    
    if(network.wired.dhcpEnable == 0)
    {
    	if( 0 != CheckIpAddress(network.wired.ip) 
            || 0 != CheckIpAddress(network.wired.netmask)
            || 0 != CheckIpAddress(network.wired.gateway) )
        {
        	ret = -1;
        }
        if( ret == 0 ) ret = NetSetIpAddr( (char *)NET_WIRED_NAME, network.wired.ip );
        if( ret == 0 ) ret = NetSetMask( (char *)NET_WIRED_NAME, network.wired.netmask );
        if( ret == 0 ) ret = NetRouteAddDefaultGateway( (char *)NET_WIRED_NAME, network.wired.gateway );
    }

    if( ret == 0 ) ret = ParamSetNetwork( &network );    
	if(FI_SUCCESS == ret) 	ret = ERROR_TYPE_SUCCESSFUL;
	else                 	ret = ERROR_TYPE_SET_PARAM_FAILED;    
    
    LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO, "SetNetWork, mac:%s, ip:%s, dhcp:%d, ret:%d.",
           network.wired.mac, network.wired.ip, network.wired.dhcpEnable, ret);
	PacketClientHead(msgHead, SV_MSG_RES_SET_NETWORK, 0, 0, ret);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;

	if( 0 == ret )
    {
        if( (networkOrg.wired.dhcpEnable !=  network.wired.dhcpEnable) 
            || Strcmp(networkOrg.wired.ip, network.wired.ip) )
        {
        	SyncParamConfig();
            RebootSendMessage();
        }
    }
    
    SVPrint("SetNetWork, mac:%s, ip:%s, dhcp:%x, ret:%x.", 
        	network.wired.mac, network.wired.ip, network.wired.dhcpEnable, ret);
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;
}

void PrintfClientUser( CONFIG_CLIENT_USER clientUser[MAX_CLIENT_USER_NUM] )
{
	int i;
    
	FiPrint2( "#####  %s  ######\r\n", __FUNCTION__ );
	for(i = 0; i < MAX_CLIENT_USER_NUM; i++)
    {
    	FiPrint2( "*****  start i = %d  *****\r\n", i );        
    	FiPrint2( "name	    : %s \r\n", clientUser[i].name );
    	FiPrint2( "pwd	        : %s \r\n", clientUser[i].pwd );        
    	FiPrint2( "permission	: %d \r\n", clientUser[i].permission );
    	FiPrint2( "level	    : %d \r\n", clientUser[i].level );
    	FiPrint2( "enable	    : %d \r\n", clientUser[i].enable );
    	FiPrint2( "***** end i = %d  *****\r\n", i );
    }    
	FiPrint2( "#####  end %s  ######\r\n", __FUNCTION__ );
}

int SVMsgGetClientUser(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret, i;
	CONFIG_CLIENT_USER     	clientUser[MAX_CLIENT_USER_NUM];
	PARAM_CONFIG_CLIENT_USER	clientUserSys[MAX_CLIENT_USER_NUM];
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(clientUser);

	for(i = 0; i < MAX_CLIENT_USER_NUM; i++)
    {
    	ret = ParamGetClientUser( i, &clientUserSys[i] );
    	if(FI_SUCCESS == ret)
        {
        	ret = ERROR_TYPE_SUCCESSFUL;
        	clientUser[i] = clientUserSys[i];
        }
    	else
        {
        	ret = ERROR_TYPE_GET_PARAM_FAILED;
        	break;
        }
    }    
	PrintfClientUser( clientUserSys );
	PacketClientHead(msgHead, SV_MSG_RES_GET_CLIENT_USER, 0, dataLen, ret);
        
	memcpy(clientCom->writeBuf, msgHead, headLen);
	memcpy(clientCom->writeBuf+headLen, &clientUser, dataLen);

	clientCom->writeBufDataSize = headLen + dataLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;
}

int SVMsgSetClientUser(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret, i;
	CONFIG_CLIENT_USER     	clientUser[MAX_CLIENT_USER_NUM];
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(clientUser);

	memcpy(&clientUser, clientCom->readBuf+headLen, dataLen);

	for(i = 0; i < MAX_CLIENT_USER_NUM; i++)
    {
    	ret = ParamSetClientUser( i, &clientUser[i] );
    	if(FI_SUCCESS == ret)
        {
        	ret = ERROR_TYPE_SUCCESSFUL;
        }
    	else
        {
        	ret = ERROR_TYPE_SET_PARAM_FAILED;
        	break;
        }
    }
    
    LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO, "SetClientUser, ret:%d.", ret);
	PacketClientHead(msgHead, SV_MSG_RES_SET_CLIENT_USER, 0, 0, ret);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;
}

int SVMsgGetVideoBaseParam(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	int channel = (int)msgHead->subType;
	CONFIG_VIDEO_BASE_PARAM 	videoBaseParam;
	PARAM_CONFIG_VIDEO_BASE_PARAM	videoBaseParamSys;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(videoBaseParam);

	ret = ParamGetVideoBaseParam( channel, &videoBaseParamSys );
	if(FI_SUCCESS == ret)
    {
    	ret = ERROR_TYPE_SUCCESSFUL;
    	videoBaseParam = videoBaseParamSys;
    }
	else
    {
    	ret = ERROR_TYPE_GET_PARAM_FAILED;
    }

	PacketClientHead(msgHead, SV_MSG_RES_GET_VIDEO_BASE_PARAM, channel, dataLen, ret);
        
	memcpy(clientCom->writeBuf, msgHead, headLen);
	memcpy(clientCom->writeBuf+headLen, &videoBaseParam, dataLen);

	clientCom->writeBufDataSize = headLen + dataLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;
}

int SVMsgSetVideoBaseParam(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
    static int lastSetTime[MAX_CHANNEL_NUM] = {0};
    int curTime;
	int channel = (int)msgHead->subType;
	CONFIG_VIDEO_BASE_PARAM videoBaseParam;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(videoBaseParam);

	memcpy(&videoBaseParam, clientCom->readBuf+headLen, dataLen);

	ret = DriverSetAdVideoCapParam( channel, videoBaseParam.brightness,
                                	videoBaseParam.contrast, videoBaseParam.hue, 
                                	videoBaseParam.saturation );
	if( FI_SUCCESS == ret )
    {
    	ret = ParamSetVideoBaseParam( channel, &videoBaseParam );    
    }
	if(FI_SUCCESS == ret) 	
    {
        ret = ERROR_TYPE_SUCCESSFUL;
    }
	else
    {
        ret = ERROR_TYPE_SET_PARAM_FAILED;  
    }  
    
    
    curTime = SysRunTimeGet();
    if(curTime - lastSetTime[channel] > 600)
    {
        lastSetTime[channel] = curTime;
        LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO,
               "SetVideoBaseParam, bri:%d, con:%d, sat:%d, exp:%d, hue:%d, ret:%d.",
               videoBaseParam.brightness, videoBaseParam.contrast, videoBaseParam.saturation,
               videoBaseParam.exposure, videoBaseParam.hue, ret);
    }

	PacketClientHead(msgHead, SV_MSG_RES_SET_VIDEO_BASE_PARAM, channel, 0, ret);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;    
}

int SVMsgGetOsdLogo(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	int channel = (int)msgHead->subType;
	CONFIG_OSD_LOGO_T     	osdLogo;
	PARAM_CONFIG_OSD_LOGO 	osdLogoSys;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(osdLogo);

	ret = ParamGetOsdLogo( channel, &osdLogoSys );
	if(FI_SUCCESS == ret)
    {
    	ret     = ERROR_TYPE_SUCCESSFUL;
    	osdLogo = osdLogoSys;
    }
	else
    {
    	ret     = ERROR_TYPE_GET_PARAM_FAILED;
    }

	PacketClientHead(msgHead, SV_MSG_RES_GET_OSD_LOGO, channel, dataLen, ret);
        
	memcpy(clientCom->writeBuf, msgHead, headLen);
	memcpy(clientCom->writeBuf+headLen, &osdLogo, dataLen);

	clientCom->writeBufDataSize = headLen + dataLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;
}

int SVMsgSetOsdLogo(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	int channel = (int)msgHead->subType;
	CONFIG_OSD_LOGO_T     	osdLogo;
    ConfigOsdLogo           osdLogoConf;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(osdLogo);
//ColorPrint(COLORYELLOW,"channel is %d\n",channel);
    // PARAM_CONFIG_OSD_TIME 	osdTimeSys;
    // ret = ParamGetOsdTime( channel, &osdTimeSys );

	memcpy(&osdLogo, clientCom->readBuf+headLen, dataLen);
    memcpy( &osdLogoConf, &osdLogo, sizeof(osdLogoConf) );
    
	ret = ParamSetOsdLogo( channel, &osdLogo );    
	if(FI_SUCCESS == ret) 	ret = ERROR_TYPE_SUCCESSFUL;
	else                 	ret = ERROR_TYPE_SET_PARAM_FAILED;    
    if(ERROR_TYPE_SUCCESSFUL == ret)
    {
         FitFiOsdSetLogoOsdConfig( channel, &osdLogoConf);
    }
    
    LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO,
           "SetOsdLogo, channel:%d, en:%d, xPos:%d, yPos:%d, ret:%d.",
           channel, osdLogo.enable, osdLogo.xPos, osdLogo.yPos, ret);
	PacketClientHead(msgHead, SV_MSG_RES_SET_OSD_LOGO, channel, 0, ret);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);        
	return 0;    
}

int SVMsgGetOsdTime(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	int channel = (int)msgHead->subType;
	CONFIG_OSD_TIME_T     	osdTime;
	PARAM_CONFIG_OSD_TIME 	osdTimeSys;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(osdTime);

	ret = ParamGetOsdTime( channel, &osdTimeSys );
	if(FI_SUCCESS == ret)
    {
    	ret = ERROR_TYPE_SUCCESSFUL;
    	osdTime = osdTimeSys;
    }
	else
    {
    	ret = ERROR_TYPE_GET_PARAM_FAILED;
    }

	PacketClientHead(msgHead, SV_MSG_RES_GET_OSD_TIME, channel, dataLen, ret);
        
	memcpy(clientCom->writeBuf, msgHead, headLen);
	memcpy(clientCom->writeBuf+headLen, &osdTime, dataLen);

	clientCom->writeBufDataSize = headLen + dataLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;
}

/*
static void adjustXY(unsigned short *smallX, unsigned short *bigX, unsigned short *smallY,
                     unsigned short *bigY, unsigned short xLen, unsigned short yLen)
{
    unsigned short deltaX, deltaY;
    deltaX = *bigX - *smallX;
    deltaY = *bigY - *smallY;
    // SVPrint("adjustXY begin %d, %d, %d, %d.\r\n", *smallX, *smallY, *bigX, *bigY);
    if(deltaY < yLen && deltaX < xLen)
    {
        // 如果x有小于两个字符重叠，自动调节x
        if( (xLen - deltaX) < 16)
        {
            deltaX = xLen - deltaX;
            if(deltaX < *smallX) *smallX = *smallX - deltaX;
            else *bigX = *bigX + deltaX;
        }
        else
        {
            deltaY = yLen - deltaY;
            if(deltaY < *smallY) *smallY = *smallY - deltaY;
            else *bigY = *bigY + deltaY;
            *bigX = *smallX;
        }
    }
    // SVPrint("adjustXY end %d, %d, %d, %d.\r\n", *smallX, *smallY, *bigX, *bigY);

}



int SVMsgSetOsdTime(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
    return 0;
    unsigned short *smallX, *bigX, *smallY, *bigY, xLen, yLen;
	int ret;
	int channel = (int)msgHead->subType;
	CONFIG_OSD_TIME_T     	osdTimeSys;
    ConfigOsdTime           osdTimeConf;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(osdTimeSys);
    unsigned char timeEn, logoEn;

	PARAM_CONFIG_OSD_LOGO 	osdLogoSys;
    ConfigOsdLogo           osdLogoConf;
	ret = ParamGetOsdLogo( channel, &osdLogoSys );

	memcpy(&osdTimeSys, clientCom->readBuf+headLen, dataLen);

	if(FI_SUCCESS == ret)
    {
        if(osdLogoSys.enable && osdTimeSys.enable)
        {
            if( osdLogoSys.yPos > osdTimeSys.yPos )
            {
                smallY = &osdTimeSys.yPos;
                bigY = &osdLogoSys.yPos;
                yLen = 16;
            }
            else
            {
                bigY = &osdTimeSys.yPos;
                smallY = &osdLogoSys.yPos;
                yLen = 16;
            }
            if( osdLogoSys.xPos > osdTimeSys.xPos )
            {
                smallX = &osdTimeSys.xPos;
                bigX = &osdLogoSys.xPos;
                xLen = 8 * 19; //yyyy-mm-dd hh:mm:ss
            }
            else
            {
                bigX = &osdTimeSys.xPos;
                smallX = &osdLogoSys.xPos;
                xLen = 8 * strlen(osdLogoSys.logo);
            }
            adjustXY(smallX, bigX, smallY, bigY, xLen, yLen);
        }
        memcpy( &osdLogoConf, &osdLogoSys, sizeof(osdLogoConf) );
        memcpy( &osdTimeConf, &osdTimeSys, sizeof(osdTimeConf) );
        // SVPrint("adjustXY %d, %d, %d, %d.\r\n", osdLogoConf.xPos, osdLogoConf.yPos, osdTimeConf.xPos, osdTimeConf.yPos);
    }
    if(FI_SUCCESS == ret) ret = ParamSetOsdTime( channel, &osdTimeSys );    
    if(FI_SUCCESS == ret) ret = ParamSetOsdLogo( channel, &osdLogoSys );    

	if(FI_SUCCESS == ret) 	ret = ERROR_TYPE_SUCCESSFUL;
	else                 	ret = ERROR_TYPE_SET_PARAM_FAILED;
    if(ERROR_TYPE_SUCCESSFUL == ret)
    {
        timeEn = osdTimeConf.enable; 
        logoEn = osdLogoConf.enable;
        osdTimeConf.enable = 0;
        osdLogoConf.enable = 0;
    	//FitFiOsdSetLogoOsdConfig( channel, &osdLogoConf );
    	FitFiOsdSetTimeOsdConfig( channel, &osdTimeConf );
        if(logoEn)
        {
            //osdLogoConf.enable = logoEn;
            //FitFiOsdSetLogoOsdConfig( channel, &osdLogoConf );
        }
        if(timeEn)
        {
            osdTimeConf.enable = timeEn;
            FitFiOsdSetTimeOsdConfig( channel, &osdTimeConf );
        }
    }

    LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO,
           "SetOsdTime, channel:%d, en:%d, xPos:%d, yPos:%d, ret:%d.",
           channel, osdTimeConf.enable, osdTimeConf.xPos, osdTimeConf.yPos, ret);
	PacketClientHead(msgHead, SV_MSG_RES_SET_OSD_TIME, channel, 0, ret);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
        
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;    
}*/

int SVMsgSetOsdTime(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret = 0;
    
	int channel = (int)msgHead->subType;

//ColorPrint(COLORYELLOW,"channel is %d\n",channel);
    CONFIG_OSD_TIME_T     	osdTimeSys;
    ConfigOsdTime           osdTimeConf;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(osdTimeSys);

	memcpy(&osdTimeSys, clientCom->readBuf+headLen, dataLen);
    memcpy( &osdTimeConf, &osdTimeSys, sizeof(osdTimeConf) );
    
	ret = ParamSetOsdTime( channel, &osdTimeSys );    
	if(FI_SUCCESS == ret) 	ret = ERROR_TYPE_SUCCESSFUL;
	else                 	ret = ERROR_TYPE_SET_PARAM_FAILED;    
    if(ERROR_TYPE_SUCCESSFUL == ret)
    {
         FitFiOsdSetTimeOsdConfig( channel, &osdTimeConf);
    }
    
    LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO,
           "SetOsdTime, channel:%d, en:%d, xPos:%d, yPos:%d, ret:%d.",
           channel, osdTimeSys.enable, osdTimeSys.xPos, osdTimeSys.yPos, ret);
	PacketClientHead(msgHead, SV_MSG_RES_SET_OSD_TIME, channel, 0, ret);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);        
	return 0;    
}

int SVMsgGetVideoEncodePublic(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom) 
{
	int ret;
	CONFIG_VIDEO_ENCODE_PUBLIC		videoEncodePublic;
	PARAM_CONFIG_VIDEO_ENCODE_PUBLIC	videoEncodePublicSys;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(videoEncodePublic);

	ret = ParamGetVideoEncodePublic( &videoEncodePublicSys );
	if(FI_SUCCESS == ret)
    {
    	ret = ERROR_TYPE_SUCCESSFUL;
    	videoEncodePublic = videoEncodePublicSys;
    }
	else
    {
    	ret = ERROR_TYPE_GET_PARAM_FAILED;
    }

	PacketClientHead(msgHead, SV_MSG_RES_GET_VIDEO_ENCODE_PUBLIC, 0, dataLen, ret);
        
	memcpy(clientCom->writeBuf, msgHead, headLen);
	memcpy(clientCom->writeBuf+headLen, &videoEncodePublic, dataLen);

	clientCom->writeBufDataSize = headLen + dataLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;
}

int SVMsgSetVideoEncodePublic(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	CONFIG_VIDEO_ENCODE_PUBLIC		videoEncodePublic;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(videoEncodePublic);

	memcpy(&videoEncodePublic, clientCom->readBuf+headLen, dataLen);
    
	ret = ParamSetVideoEncodePublic( &videoEncodePublic );    
	if(FI_SUCCESS == ret)     
    {
    	ret = FitMympiSetVideoStandard();         
    }    
    
	if(FI_SUCCESS != ret)     
    {
    	ret = ERROR_TYPE_SET_PARAM_FAILED;    
    }
	if( FI_SUCCESS == ret )
    {
       	LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO, "SetVencPub, videoStandard:%d, ret:%d.", videoEncodePublic.videoStandard, ret);
    }
	PacketClientHead(msgHead, SV_MSG_RES_SET_VIDEO_ENCODE_PUBLIC, 0, 0, ret);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;
}

int SVMsgGetAudio(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	CONFIG_AUDIO audio;
	PARAM_CONFIG_AUDIO_ENCODE	audioSys;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(audio);

	ret = ParamGetAudio( &audioSys );
	if(FI_SUCCESS == ret)
    {
    	ret     = ERROR_TYPE_SUCCESSFUL;
    	audio     = audioSys;
    }
	else
    {
    	ret     = ERROR_TYPE_GET_PARAM_FAILED;
    }

	PacketClientHead(msgHead, SV_MSG_RES_GET_AUDIO, 0, dataLen, ret);
        
	memcpy(clientCom->writeBuf, msgHead, headLen);
	memcpy(clientCom->writeBuf+headLen, &audio, dataLen);

	clientCom->writeBufDataSize = headLen + dataLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;
}

int SVMsgSetAudio(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	CONFIG_AUDIO audio, audioSys;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(audio);

	memcpy(&audio, clientCom->readBuf+headLen, dataLen);
 
	ParamGetAudio( &audioSys ); 

	SVPrint( "%s:encodeType = %u!\r\n", __FUNCTION__, audio.encodeType );
	ret = ParamSetAudio( &audio );    
	if(FI_SUCCESS == ret)     
    {        
    	if( audio.encodeType != audioSys.encodeType ) 
        {
        	FitMympiSetAudioEncType();
        }
    	ret = ERROR_TYPE_SUCCESSFUL;
    }
	else                     
    {
    	ret = ERROR_TYPE_SET_PARAM_FAILED;    
    }
    LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO,
           "SetAudio, chMode:%d, type:%d, bitWidth:%d, rate:%d, ret:%d.",
           audio.chMode, audio.encodeType, audio.bitWidth, audio.sampleRate, ret);
	PacketClientHead(msgHead, SV_MSG_RES_SET_AUDIO, 0, 0, ret);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;
}

int SVMsgGetAutoMaintain(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom) 
{
	int ret;
	CONFIG_AUTO_MAINTAIN     	autoMaintain;
	PARAM_CONFIG_AUTO_MAINTAIN 	autoMaintainSys;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(autoMaintain);

	ret = ParamGetAutoMaintain( &autoMaintainSys );
	if(FI_SUCCESS == ret)
    {
    	ret = ERROR_TYPE_SUCCESSFUL;
    	autoMaintain = autoMaintainSys;
    }
	else
    {
    	ret = ERROR_TYPE_GET_PARAM_FAILED;
    }

	PacketClientHead(msgHead, SV_MSG_RES_GET_AUTO_MAINTAIN, 0, dataLen, ret);
        
	memcpy(clientCom->writeBuf, msgHead, headLen);
	memcpy(clientCom->writeBuf+headLen, &autoMaintain, dataLen);

	clientCom->writeBufDataSize = headLen + dataLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;
}

int SVMsgSetAutoMaintain(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom) 
{
	int ret;
	CONFIG_AUTO_MAINTAIN 	autoMaintain;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(autoMaintain);

	memcpy(&autoMaintain, clientCom->readBuf+headLen, dataLen);

	ret = ParamSetAutoMaintain( &autoMaintain );    
	if(FI_SUCCESS == ret) 	ret = ERROR_TYPE_SUCCESSFUL;
	else                 	ret = ERROR_TYPE_SET_PARAM_FAILED;    

    LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO, "SetAutoMaintain, flag:%d, ret:%d.", autoMaintain.rebooDatetFlag, ret);
	PacketClientHead(msgHead, SV_MSG_RES_SET_AUTO_MAINTAIN, 0, 0, ret);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;
}

int SVMsgGetRecordPublic(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom) 
{
	int ret;
	CONFIG_RECORD_PUBLIC      recordPublic;
	PARAM_CONFIG_RECORD_PUBLIC recordPublicSys;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(recordPublic);

	ret = ParamGetRecordPublic( &recordPublicSys );
	if(FI_SUCCESS == ret)
    {
    	ret = ERROR_TYPE_SUCCESSFUL;
    	recordPublic = recordPublicSys;
    }
	else
    {
    	ret = ERROR_TYPE_GET_PARAM_FAILED;
    }

	PacketClientHead(msgHead, SV_MSG_RES_GET_RECORD_PUBLIC, 0, dataLen, ret);
        
	memcpy(clientCom->writeBuf, msgHead, headLen);
	memcpy(clientCom->writeBuf+headLen, &recordPublic, dataLen);

	clientCom->writeBufDataSize = headLen + dataLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;
}

int SVMsgSetRecordPublic(DCP_HEAD_T *msgHead,CLIENT_COMMUNICATE_T *clientCom) 
{
	int ret;
	CONFIG_RECORD_PUBLIC 	recordPublic;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(recordPublic);

	memcpy(&recordPublic, clientCom->readBuf+headLen, dataLen);

    ret = FiRecSetSupportLoopRecordFlag(recordPublic.loopRecord);
	if(FI_SUCCESS == ret)
    {
       	ret = FiRecSetRecordDelSpace( recordPublic.delSpace );
    }
	if(FI_SUCCESS == ret)
    {
    	ret = FiRecSetRecFileSwitchTime(recordPublic.switchFileTime);
    }
    SVPrint(" record switchFileTime %d, delSpace %d, loopRecord.\r\n",
            recordPublic.switchFileTime, recordPublic.delSpace, recordPublic.loopRecord);

	if(FI_SUCCESS == ret)
    {
    	ret = ParamSetRecordPublic( &recordPublic );    
    }
	if(FI_SUCCESS == ret) 	ret = ERROR_TYPE_SUCCESSFUL;
	else                 	ret = ERROR_TYPE_SET_PARAM_FAILED;    

    LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO,
           "SetRecordPublic, delSpace:%d, loop:%d, preRecord:%d, switchTime:%d, ret:%d.",
           recordPublic.delSpace, recordPublic.loopRecord, recordPublic.preRecord, recordPublic.switchFileTime, ret);
	PacketClientHead(msgHead, SV_MSG_RES_SET_RECORD_PUBLIC, 0, 0, ret);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
    
	return 0;
}

int SVMsgGetRecordParam(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	int channel = (int)msgHead->subType;
	CONFIG_RECORD_PARAM 	recordParam;
	PARAM_CONFIG_RECORD_PARAM	recordParamSys;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(recordParam);

	ret = ParamGetRecordParam( channel, &recordParamSys );
	if(FI_SUCCESS == ret)
    {
    	ret = ERROR_TYPE_SUCCESSFUL;
    	recordParam = recordParamSys;
    }
	else
    {
    	ret = ERROR_TYPE_GET_PARAM_FAILED;
    }

	PacketClientHead(msgHead, SV_MSG_RES_GET_RECORD_PARAM, channel, dataLen, ret);
        
	memcpy(clientCom->writeBuf, msgHead, headLen);
	memcpy(clientCom->writeBuf+headLen, &recordParam, dataLen);

	clientCom->writeBufDataSize = headLen + dataLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);

	return 0;
}

int SVMsgSetRecordParam(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	int channel = (int)msgHead->subType;
	CONFIG_RECORD_PARAM 	recordParam;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(recordParam);
	int i;

	memcpy(&recordParam, clientCom->readBuf+headLen, dataLen);

	for( i = 0; i < MAX_WEEK_DAY; ++i )
    {
    	ret = FiRecSetTimerRecordPolicy( channel, i, &recordParam );
    	if (FI_FAIL == ret) break;
    }
	if( FI_SUCCESS == ret )
    {
    	ret = FiRecSetHandRecord( channel, &recordParam );
    }

	ret = ParamSetRecordParam( channel, &recordParam );    
	if(FI_SUCCESS == ret) 	ret = ERROR_TYPE_SUCCESSFUL;
	else                 	ret = ERROR_TYPE_SET_PARAM_FAILED;

    LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO, "SetRecordParam, channel:%d, recHand:%d, ret:%d.",
           channel, recordParam.recHand.recFlag, ret);
	PacketClientHead(msgHead, SV_MSG_RES_SET_RECORD_PARAM, channel, 0, ret);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;    
}

#if 0
int SVMsgGetAlarmIo(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	CONFIG_ALARM_IO     	alarmIo;
	PARAM_CONFIG_ALARM_IO 	alarmIoSys;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(alarmIo);

	ret = ParamGetAlarmIo( &alarmIoSys );
	if(FI_SUCCESS == ret)
    {
    	ret = ERROR_TYPE_SUCCESSFUL;
    	alarmIo = alarmIoSys;
    }
	else
    {
    	ret = ERROR_TYPE_GET_PARAM_FAILED;
    }

	PacketClientHead(msgHead, SV_MSG_RES_GET_ALARM_IO, 0, dataLen, ret);
        
	memcpy(clientCom->writeBuf, msgHead, headLen);
	memcpy(clientCom->writeBuf+headLen, &alarmIo, dataLen);
	clientCom->writeBufDataSize = headLen + dataLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;
}

int SVMsgSetAlarmIo(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	CONFIG_ALARM_IO     	alarmIo;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(alarmIo);

	memcpy(&alarmIo, clientCom->readBuf+headLen, dataLen);

	ret = ParamSetAlarmIo( &alarmIo );    
	if(FI_SUCCESS == ret) 	ret = ERROR_TYPE_SUCCESSFUL;
	else                 	ret = ERROR_TYPE_SET_PARAM_FAILED;    

    LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO, "SetAlarmInfo, normalcy:%d, armFlag:%d, interval:%d, ret:%d.",
           alarmIo.normalcy, alarmIo.armFlag, alarmIo.scoutInterval, ret);
	PacketClientHead(msgHead, SV_MSG_RES_SET_ALARM_IO, 0, 0, ret);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;
}
#endif

int SVMsgGetNtp(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	CONFIG_NTP     	ntp;
	PARAM_CONFIG_NTP	ntpSys;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(ntp);

	ret = ParamGetNtp( &ntpSys );
	if(FI_SUCCESS == ret)
    {
    	ret = ERROR_TYPE_SUCCESSFUL;
    	ntp = ntpSys;
    }
	else
    {
    	ret = ERROR_TYPE_GET_PARAM_FAILED;
    }

	PacketClientHead(msgHead, SV_MSG_RES_GET_NTP, 0, dataLen, ret);
        
	memcpy(clientCom->writeBuf, msgHead, headLen);
	memcpy(clientCom->writeBuf+headLen, &ntp, dataLen);
	clientCom->writeBufDataSize = headLen + dataLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;
}

int SVMsgSetNtp(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	CONFIG_NTP 	ntp;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(ntp);

	memcpy(&ntp, clientCom->readBuf+headLen, dataLen);

	ret = ParamSetNtp( &ntp );    
	if(FI_SUCCESS == ret) 	ret = ERROR_TYPE_SUCCESSFUL;
	else                 	ret = ERROR_TYPE_SET_PARAM_FAILED;    

    LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO, "SetNtp, en:%d, zone:%d, interval:%d, ret:%d.",
           ntp.enable, ntp.zone, ntp.interval, ret);
	PacketClientHead(msgHead, SV_MSG_RES_SET_NTP, 0, 0, ret);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;
}

int SVMsgGetEmail(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	CONFIG_EMAIL     	email;
	PARAM_CONFIG_EMAIL 	emailSys;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(email);

	ret = ParamGetEmail( &emailSys);
	if(FI_SUCCESS == ret)
    {
    	ret = ERROR_TYPE_SUCCESSFUL;
    	email = emailSys;
    }
	else
    {
    	ret = ERROR_TYPE_GET_PARAM_FAILED;
    }

	PacketClientHead(msgHead, SV_MSG_RES_GET_EMAIL, 0, dataLen, ret);
        
	memcpy(clientCom->writeBuf, msgHead, headLen);
	memcpy(clientCom->writeBuf+headLen, &email, dataLen);

	clientCom->writeBufDataSize = headLen + dataLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;
}

int SVMsgSetEmail(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	CONFIG_EMAIL email;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(email);

	memcpy(&email, clientCom->readBuf+headLen, dataLen);

	ret = ParamSetEmail( &email );    
	if(FI_SUCCESS == ret) 	ret = ERROR_TYPE_SUCCESSFUL;
	else                 	ret = ERROR_TYPE_SET_PARAM_FAILED;    

    LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO, "SetEmail, serverIP:%s, port:%d, ret:%d.",
           email.serverIP, email.port, ret);
	PacketClientHead(msgHead, SV_MSG_RES_SET_EMAIL, 0, 0, ret);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;    
}

int SVMsgGetVideoEncodeParam(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	int channel = (int)msgHead->subType;
	CONFIG_VIDEO_ENCODE 	videoEncodeParam;
	PARAM_CONFIG_VIDEO_ENCODE	videoEncodeParamSys;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(videoEncodeParam);

	ret = ParamGetVideoEncode( channel, &videoEncodeParamSys );
	if(FI_SUCCESS == ret)
    {
    	ret = ERROR_TYPE_SUCCESSFUL;
    	videoEncodeParam = videoEncodeParamSys;
    }
	else
    {
    	ret = ERROR_TYPE_GET_PARAM_FAILED;
    }

	PacketClientHead(msgHead, SV_MSG_RES_GET_VIDEO_ENCODE_PARAM, channel, dataLen, ret);
        
	memcpy(clientCom->writeBuf, msgHead, headLen);
	memcpy(clientCom->writeBuf+headLen, &videoEncodeParam, dataLen);

	clientCom->writeBufDataSize = headLen + dataLen;

	return 0;
}

int SVMsgSetVideoEncodeParam(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	int channel = (int)msgHead->subType;
	CONFIG_VIDEO_ENCODE 	videoEncodeParam;
	PARAM_CONFIG_VIDEO_ENCODE param;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(videoEncodeParam);

	memcpy(&videoEncodeParam, clientCom->readBuf+headLen, dataLen);

	SVPrint( "reso(%d), btype(%d), fr(%d), ifi(%d), pref(%d), etype(%d), bitrate(%d)!\r\n", 
    	videoEncodeParam.resolution, 
    	videoEncodeParam.bitrateType, 
    	videoEncodeParam.frameRate, 
    	videoEncodeParam.iFrameInterval, 
    	videoEncodeParam.preferFrame, 
    	videoEncodeParam.encodeType, 
    	videoEncodeParam.bitrate );
	ret = ParamGetVideoEncode( channel, &param );

	if(FI_SUCCESS == ret) 
    {        
    	ret = ParamSetVideoEncode( channel, &videoEncodeParam );    
    }
    
	if( FI_SUCCESS == ret )     
    {
        
    	ret = FitMympiSetBitrateType( channel, videoEncodeParam.bitrateType );
        
    	if( FI_SUCCESS == ret 
            && videoEncodeParam.bitrate != param.bitrate )
        {
        	ret = FitMympiSetBitrate( channel, videoEncodeParam.bitrate );
        }
        
    	if( FI_SUCCESS == ret 
            && videoEncodeParam.frameRate != param.frameRate )
        {
        	ret = FitMympiSetFramerate( channel, videoEncodeParam.frameRate );
        }
        
    	if( FI_SUCCESS == ret 
            && videoEncodeParam.iFrameInterval != param.iFrameInterval )
        {
        	ret = FitMympiSetIframeInterval( channel, videoEncodeParam.iFrameInterval );
        }
        
    	if( FI_SUCCESS == ret 
            && videoEncodeParam.encodeType != param.encodeType )
        {            
        	ret = FitMympiSetAvencAccompanyingAudio( channel, videoEncodeParam.encodeType );
        }
        
    	if( FI_SUCCESS == ret 
            && videoEncodeParam.resolution != param.resolution )
        {        
        	ret = FitMympiSetResolution();
        }

        //if ( standard != VencParamEasyGetVideoStandard() )
        //{
            
        //	ret = FitMympiSetVideoStandard();
        //}  
        ret = FitMympiSetVideoStandard();//直接重启音视频编解码
        
    	ret = ERROR_TYPE_SUCCESSFUL;
    }
	if( FI_SUCCESS != ret )
    {
    	ret = ERROR_TYPE_SET_PARAM_FAILED;
    }
    
    LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO, "SetVencParam, channel:%d, res:%d, bitrate:%d,frameRate:%d, ret:%d.",
           channel, videoEncodeParam.resolution, videoEncodeParam.bitrate, videoEncodeParam.frameRate, ret);
	PacketClientHead(msgHead, SV_MSG_RES_SET_VIDEO_ENCODE_PARAM, channel, 0, ret);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;    
}

int SVMsgGetVideoEncodeParamSlave(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	int channel = (int)msgHead->subType;
	CONFIG_VIDEO_ENCODE 	videoEncodeParam;
	PARAM_CONFIG_VIDEO_ENCODE	videoEncodeParamSys;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(videoEncodeParam);

	ret = ParamGetVideoEncodeSlave( channel, &videoEncodeParamSys );
	if(FI_SUCCESS == ret)
    {
    	ret = ERROR_TYPE_SUCCESSFUL;
    	videoEncodeParam = videoEncodeParamSys;
    }
	else
    {
    	ret = ERROR_TYPE_GET_PARAM_FAILED;
    }

	PacketClientHead(msgHead, SV_MSG_RES_GET_VIDEO_ENCODE_PARAM_SLAVE, channel, dataLen, ret);
        
	memcpy(clientCom->writeBuf, msgHead, headLen);
	memcpy(clientCom->writeBuf+headLen, &videoEncodeParam, dataLen);

	clientCom->writeBufDataSize = headLen + dataLen;

	return 0;
}

int SVMsgSetVideoEncodeParamSlave(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	int channel = (int)msgHead->subType;
	CONFIG_VIDEO_ENCODE 	videoEncodeParam;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(videoEncodeParam);

	memcpy(&videoEncodeParam, clientCom->readBuf+headLen, dataLen);

	ret = ParamSetVideoEncodeSlave( channel, &videoEncodeParam );    
	if(FI_SUCCESS == ret) 	ret = ERROR_TYPE_SUCCESSFUL;
	else                 	ret = ERROR_TYPE_SET_PARAM_FAILED;

    LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO, "SetVencParamSlave, channel:%d, res:%d, bitrate:%d,frameRate:%d, ret:%d.",
           channel, videoEncodeParam.resolution, videoEncodeParam.bitrate, videoEncodeParam.frameRate, ret);
	PacketClientHead(msgHead, SV_MSG_RES_SET_VIDEO_ENCODE_PARAM_SLAVE, channel, 0, ret);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;    
}

int SVMsgGetOsd(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	int channel = (int)msgHead->subType;
	CONFIG_OSD 	osd;
	PARAM_CONFIG_OSD_LOGO osdLogoSys;
	PARAM_CONFIG_OSD_TIME osdTimeSys;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(osd);

	ret = ParamGetOsdLogo( channel, &osdLogoSys );
	if(FI_SUCCESS == ret) ret = ParamGetOsdTime( channel, &osdTimeSys );
	if(FI_SUCCESS == ret)
    {
    	ret         = ERROR_TYPE_SUCCESSFUL;
    	osd.osdLogo = osdLogoSys;
    	osd.osdTime = osdTimeSys;
    }
	else
    {
    	ret     = ERROR_TYPE_GET_PARAM_FAILED;
    }

	PacketClientHead(msgHead, SV_MSG_RES_GET_OSD, channel, dataLen, ret);
        
	memcpy(clientCom->writeBuf, msgHead, headLen);
	memcpy(clientCom->writeBuf+headLen, &osd, dataLen);

	clientCom->writeBufDataSize = headLen + dataLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;
}

int SVMsgSetOsd(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	int channel = (int)msgHead->subType;
	CONFIG_OSD	osd;
    
    ConfigOsdLogo osdLogo;
    ConfigOsdTime osdTime;

	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(osd);

	memcpy(&osd, clientCom->readBuf+headLen, dataLen);
    memcpy( &osdLogo, &osd.osdLogo, sizeof(osdLogo) );
    memcpy( &osdTime, &osd.osdTime, sizeof(osdTime) );
    
	ret = ParamSetOsdLogo( channel, &osd.osdLogo);    
	if(FI_SUCCESS == ret) 	
    {
        ret = ParamSetOsdTime(channel, &osd.osdTime);
    }
    ret = (FI_SUCCESS == ret) ? ERROR_TYPE_SUCCESSFUL : ERROR_TYPE_SET_PARAM_FAILED;
	
    if(ERROR_TYPE_SUCCESSFUL == ret)
    {
        FitFiOsdSetLogoOsdConfig( channel, &osdLogo);
    	FitFiOsdSetTimeOsdConfig( channel, &osdTime );
    }
    
    LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO, "SetOsd, channel:%d, logoEn:%d, timeEn:%d, ret:%d.",
           channel, osdLogo.enable, osdTime.enable, ret);
	PacketClientHead(msgHead, SV_MSG_RES_SET_OSD, channel, 0, ret);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);        
	return 0;    
}

int SVMsgSearchRecord( DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom )
{
	int ret = 0;
	DCP_SEARCH_RECORD_ARGS_T searchRecord;
	unsigned int headLen     = sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(searchRecord.searchCond);
	Memcpy( &searchRecord.msgHead, msgHead, headLen );
	Memcpy( &searchRecord.searchCond, clientCom->readBuf+headLen, dataLen );
	searchRecord.index = clientCom->index;

	ret = PtpoolAdd( DcpSearchRecordThread, (void *)&searchRecord, sizeof(searchRecord) );
    
	return ret;
}

int SVMsgRecordReq( DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom )
{
	int ret = -1, fd;
	DCP_RECORD_REQ_T recordReq;
	unsigned int headLen     = sizeof( DCP_HEAD_T );    
	unsigned int dataLen	= sizeof( recordReq );
	Memcpy( &recordReq, clientCom->readBuf+headLen, dataLen );
    
	fd = Open( recordReq.recFileName, O_RDONLY );
	if( fd > 0 )
    {
    	ret = DcpStartRecDownload( clientCom->socket, fd );
    }

	if( 0 == ret )
    {
    	ret = DCP_COM_ERR_NEED_MOVE_SOCKET;
    }
	return ret;
}

int SVMsgStopStream( DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom )
{
	int ret;
	int channel;

	channel = msgHead->subType;
	ret = DcpStopChannelStream( clientCom->threadId, channel );
    
	return ret;
}

int SVMsgLogout( DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom )
{
	int ret;

	ret = DCP_COM_ERR_NEED_CLOSE_SOCKET;
    
	return ret;
}


int SVMsgUpdateReq(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret, retTemp;
	unsigned int ackResult;
	DCP_UPDATE_REQ_T updateReq;
	unsigned int headLen    = sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(updateReq);

    ret = DCP_COM_ERR_NEED_CLOSE_SOCKET;
    if(msgHead->len < dataLen)
    {
        ackResult = ERROR_TYPE_UPDATE_PARAM_INVAL;
    }
    else
    {
        memcpy( &updateReq, clientCom->readBuf+headLen, dataLen );
        if(updateReq.fileSize < 1) // 升级文件大小判断
        {
            ackResult = ERROR_TYPE_UPDATE_PARAM_INVAL;
        }
        else
        {
            retTemp = IsDcpUpdateThreadRun();
            if( 0 == retTemp )
            {
                retTemp = DcpCreateUpdateThread( updateReq.fileSize, clientCom->socket );
                if( 0 == retTemp )
                {
                    ackResult = ERROR_TYPE_SUCCESSFUL;
                    ret = DCP_COM_ERR_NEED_MOVE_SOCKET; // 告诉处理者该socket 对象已经被升级交互线程接管,可以不用管理了
                }
                else
                {
                    ackResult = ERROR_TYPE_UPDATE_THREAD_FAIL;
                }
            }
            else
            {
                ackResult = ERROR_TYPE_upDATE_OTHER_UPDATE;
                ERRORPRINT("The other updated\r\n");
            }
        }
    }
    // ERROR_TYPE_SUCCESSFUL 不用回复，在update thread 里再回复
    if( ERROR_TYPE_SUCCESSFUL != ackResult )
    {
        PacketClientHead(msgHead, SV_MSG_RES_UPDATE, 0, 0, ackResult);    
        memcpy(clientCom->writeBuf, msgHead, headLen);    
        clientCom->writeBufDataSize = headLen;
    }
    LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO, "UpdateReq, fileSize:%d, ret:%d.", updateReq.fileSize, ackResult);

	return ret;
}

int SVMsgGetLog( DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom )
{
	int ret = 0;
	DCP_SEARCH_LOG_ARGS_T searchLog;
	unsigned int headLen     = sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(searchLog.getLogCond);
	Memcpy( &searchLog.msgHead, msgHead, headLen );
	Memcpy( &searchLog.getLogCond, clientCom->readBuf+headLen, dataLen );
	searchLog.index = clientCom->index;

	ret = PtpoolAdd( DcpSearchLogThread, (void *)&searchLog, sizeof(searchLog) );
    
	return ret;
}

int SVMsgGetAlarmIoParam(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	int channel = (int)msgHead->subType;
	CONFIG_ALARM_IO     	alarmIoParam;
	PARAM_CONFIG_ALARM_IO	alarmIoParamSys;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(alarmIoParam);

	ret = ParamGetAlarmIo( &alarmIoParamSys );
	if(FI_SUCCESS == ret)
    {
    	ret = ERROR_TYPE_SUCCESSFUL;
    	alarmIoParam = alarmIoParamSys;
    }
	else
    {
    	ret = ERROR_TYPE_GET_PARAM_FAILED;
    }

	PacketClientHead(msgHead, SV_MSG_RES_GET_ALARM_IO, channel, dataLen, ret);
        
	memcpy(clientCom->writeBuf, msgHead, headLen);
	memcpy(clientCom->writeBuf+headLen, &alarmIoParam, dataLen);

	clientCom->writeBufDataSize = headLen + dataLen;

	return 0;
}

int SVMsgSetAlarmIoParam(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	int channel = (int)msgHead->subType;
	CONFIG_ALARM_IO     	alarmIoParam;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(alarmIoParam);

	memcpy(&alarmIoParam, clientCom->readBuf+headLen, dataLen);

	ret = ParamSetAlarmIo( &alarmIoParam );    
	if(FI_SUCCESS == ret) 	ret = ERROR_TYPE_SUCCESSFUL;
	else                 	ret = ERROR_TYPE_SET_PARAM_FAILED;    

	AlarmSendMsgIoAlarmParamChange();

    LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO, "SetAlarmIo, normalcy:%d, interval:%d, arm:%d, ret:%d.",
           alarmIoParam.normalcy, alarmIoParam.scoutInterval, alarmIoParam.armFlag, ret);
	PacketClientHead(msgHead, SV_MSG_RES_SET_ALARM_IO, channel, 0, ret);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;    
}

int SVMsgGetAlarmVlossParam(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	int channel = (int)msgHead->subType;
	CONFIG_ALARM_VIDEO_LOSE     	alarmVlossParam;
	PARAM_CONFIG_ALARM_VIDEO_LOSE 	alarmVlossParamSys;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(alarmVlossParam);

	ret = ParamGetAlarmVideoLose( channel, &alarmVlossParamSys );
	if(FI_SUCCESS == ret)
    {
    	ret = ERROR_TYPE_SUCCESSFUL;
    	alarmVlossParam = alarmVlossParamSys;
    }
	else
    {
    	ret = ERROR_TYPE_GET_PARAM_FAILED;
    }

	PacketClientHead(msgHead, SV_MSG_RES_GET_ALARM_VLOSS, channel, dataLen, ret);
        
	memcpy(clientCom->writeBuf, msgHead, headLen);
	memcpy(clientCom->writeBuf+headLen, &alarmVlossParam, dataLen);

	clientCom->writeBufDataSize = headLen + dataLen;

	return 0;
}

int SVMsgSetAlarmVlossParam(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	int channel = (int)msgHead->subType;
	CONFIG_ALARM_VIDEO_LOSE     	alarmVlossParam;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(alarmVlossParam);

	memcpy(&alarmVlossParam, clientCom->readBuf+headLen, dataLen);

	ret = ParamSetAlarmVideoLose( channel, &alarmVlossParam );    
	if(FI_SUCCESS == ret) 	ret = ERROR_TYPE_SUCCESSFUL;
	else                 	ret = ERROR_TYPE_SET_PARAM_FAILED;    

	AlarmSendMsgVlossAlarmParamChange( channel );

    LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO, "SetAlarmVloss, interval:%d, arm:%d, ret:%d.",
           alarmVlossParam.scoutInterval, alarmVlossParam.armFlag, ret);
	PacketClientHead(msgHead, SV_MSG_RES_SET_ALARM_VLOSS, channel, 0, ret);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;    
}

int SVMsgGetAlarmVideoShelterParam(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	int channel = (int)msgHead->subType;
	CONFIG_ALARM_VIDEO_SHELTER         	alarmVideoShelterParam;
	PARAM_CONFIG_ALARM_VIDEO_SHELTER 	alarmVideoShelterParamSys;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(alarmVideoShelterParam);

	ret = ParamGetAlarmVideoShelter( channel, &alarmVideoShelterParamSys );
	if(FI_SUCCESS == ret)
    {
    	ret = ERROR_TYPE_SUCCESSFUL;
    	alarmVideoShelterParam = alarmVideoShelterParamSys;
    }
	else
    {
    	ret = ERROR_TYPE_GET_PARAM_FAILED;
    }

	PacketClientHead(msgHead, SV_MSG_RES_GET_ALARM_SHELTER, channel, dataLen, ret);
        
	memcpy(clientCom->writeBuf, msgHead, headLen);
	memcpy(clientCom->writeBuf+headLen, &alarmVideoShelterParam, dataLen);

	clientCom->writeBufDataSize = headLen + dataLen;

	return 0;
}

int SVMsgSetAlarmVideoShelterParam(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	int channel = (int)msgHead->subType;
	CONFIG_ALARM_VIDEO_SHELTER         	alarmVideoShelterParam;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(alarmVideoShelterParam);

	memcpy(&alarmVideoShelterParam, clientCom->readBuf+headLen, dataLen);

	ret = ParamSetAlarmVideoShelter( channel, &alarmVideoShelterParam );    
	if(FI_SUCCESS == ret) 	ret = ERROR_TYPE_SUCCESSFUL;
	else                 	ret = ERROR_TYPE_SET_PARAM_FAILED;    

	AlarmSendMsgShelterAlarmParamChange( channel );

    LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO, "SetAlarmVideoShelter, sensitivity:%d, interval:%d, arm:%d, ret:%d.",
           alarmVideoShelterParam.sensitivity, alarmVideoShelterParam.scoutInterval, alarmVideoShelterParam.armFlag, ret);
	PacketClientHead(msgHead, SV_MSG_RES_SET_ALARM_SHELTER, channel, 0, ret);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;    
}

int SVMsgGetAlarmMDParam(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	int channel = (int)msgHead->subType;
	CONFIG_ALARM_MOVE_DETECT	    	alarmMDParam;
	PARAM_CONFIG_ALARM_MOVE_DETECT     	alarmMDParamSys;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(alarmMDParam);

	ret = ParamGetAlarmMoveDetect( channel, &alarmMDParamSys );
	if(FI_SUCCESS == ret)
    {
    	ret = ERROR_TYPE_SUCCESSFUL;
    	alarmMDParam = alarmMDParamSys;
    }
	else
    {
    	ret = ERROR_TYPE_GET_PARAM_FAILED;
    }

	PacketClientHead(msgHead, SV_MSG_RES_GET_ALARM_MD, channel, dataLen, ret);
        
	memcpy(clientCom->writeBuf, msgHead, headLen);
	memcpy(clientCom->writeBuf+headLen, &alarmMDParam, dataLen);

	clientCom->writeBufDataSize = headLen + dataLen;

	return 0;
}

int SVMsgSetAlarmMDParam(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	int channel = (int)msgHead->subType;
	CONFIG_ALARM_MOVE_DETECT	    	alarmMDParam;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(alarmMDParam);

	memcpy(&alarmMDParam, clientCom->readBuf+headLen, dataLen);

	ret = ParamSetAlarmMoveDetect( channel, &alarmMDParam );    
	if(FI_SUCCESS == ret) 	ret = ERROR_TYPE_SUCCESSFUL;
	else                 	ret = ERROR_TYPE_SET_PARAM_FAILED;    

	AlarmSendMsgMdAlarmParamChange( channel );

    LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO, "SetAlarmMD, level:%d, interval:%d, arm:%d, ret:%d.",
           alarmMDParam.sensitiveLevel, alarmMDParam.scoutInterval, alarmMDParam.armFlag, ret);
	PacketClientHead(msgHead, SV_MSG_RES_SET_ALARM_MD, channel, 0, ret);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;    
}


int SVMsgGetStorageInfo( DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom )
{
	int ret = 0;
    int i;
	unsigned int headLen     = sizeof(DCP_HEAD_T);    
    unsigned int dataLen = 0;
    DCP_STORAGE_INFO_T    dcpStorageInfo;
    HDD_MANAGE_ST         *pHddManageSt;
    // HDD_PARTITION_INFO_ST partitionInfo;
    unsigned int dcpStorageInfoLen = sizeof(dcpStorageInfo);
    pHddManageSt = (HDD_MANAGE_ST *)Malloc( sizeof(HDD_MANAGE_ST) );

    clientCom->writeBufDataSize = headLen;
    if( pHddManageSt )
    {
        FiHddGetHddManage(pHddManageSt);
        for(i = 0; i < MAX_HDD_PARTITION_NUM; i++)
        {
            if(pHddManageSt->partHead[i].mountFlag != HDD_NONE)
            {
                dcpStorageInfo.type         = pHddManageSt->partHead[i].type;
                dcpStorageInfo.totalSize     = pHddManageSt->partHead[i].totalSize;
                dcpStorageInfo.freeSize     = pHddManageSt->partHead[i].freeSize;

                WriteDataToWriteBuf(clientCom, (void *)&dcpStorageInfo, dcpStorageInfoLen);
                dataLen += dcpStorageInfoLen;
            }

        }
        Free(pHddManageSt);
    }
    PacketClientHead(msgHead, SV_MSG_RES_STORAGE_INFO, 0, dataLen, 0);
    memcpy(clientCom->writeBuf, msgHead, headLen);    
    
	return ret;
}

//
// 获取系统时间
//
int SVMsgGetSysTime( DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom )
{
	int ret;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen;
	int nRet = 0;
	int year = 0, month = 0, day = 0;
	int hour = 0, minute = 0, second = 0;
	DCP_DATE_TIME_T timeData;

	dataLen = sizeof(DCP_DATE_TIME_T);
	FiTimeUtcToHuman( time(NULL), &year, &month, &day, &hour, &minute, &second );
	if ( nRet >= 0 )
    {
    	snprintf( timeData.dateTime, sizeof(timeData.dateTime),
                "%04d-%02d-%02d %02d:%02d:%02d", 
            	year, month, day, hour, minute, second );        
    	ret = ERROR_TYPE_SUCCESSFUL;
    }
	else
    {
    	ret = ERROR_TYPE_GET_PARAM_FAILED;
    }

	PacketClientHead(msgHead, SV_MSG_RES_GET_TIME, 0, dataLen, ret);
        
	memcpy(clientCom->writeBuf, msgHead, headLen);
	memcpy(clientCom->writeBuf+headLen, &timeData, dataLen);
	clientCom->writeBufDataSize = headLen + dataLen;

	return 0;
}

//
// 设置系统时间
//
int SVMsgSetSysTime( DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom )
{
	int ret;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen;
	int nRet = 0;
	int year = 0, month = 0, day = 0;
	int hour = 0, minute = 0, second = 0;
	DCP_DATE_TIME_T timeData;

	SVPrint( "####################  Set System Time  ####################\r\n" );

	dataLen = sizeof(DCP_DATE_TIME_T);
	memcpy(&timeData.dateTime, clientCom->readBuf+headLen, dataLen);
	sscanf( timeData.dateTime, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second );
	nRet = RealTimeSetDatetime( year, month, day, hour, minute, second );
	if( 0 == nRet )
    {
    	nRet = RtcSetTime( year, month, day, hour, minute, second );
    }
	ret = ( nRet != -1 ) ? ERROR_TYPE_SUCCESSFUL : ERROR_TYPE_SET_PARAM_FAILED;

    LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO,
           "SetSysTime, year:%d, month:%d, day:%d, ret:%d.",
           year, month, day, ret);
	PacketClientHead(msgHead, SV_MSG_RES_SET_TIME, 0, 0, ret);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
    
	return nRet;
}

// 系统重启
int SVMsgSysReboot( DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom )
{
	unsigned int headLen	= sizeof(DCP_HEAD_T);    

    LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO, "SysReboot.");
	PacketClientHead(msgHead, SV_MSG_RES_SYS_REBOOT, 0, 0, 0);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
    RebootSendMessage();

    return 0;
}

//
// 恢复出厂设置
//
int SVMsgResetFactory( DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom )
{
	unsigned int headLen	= sizeof(DCP_HEAD_T);    

	ParamSetFactoryConfigure();

    LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO, "ResetFactory.");
	PacketClientHead(msgHead, SV_MSG_RES_RESET_FACTORY, 0, 0, 0);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
    RebootSendMessage();

	return 0;
}

// 恢复默认参数
int SVMsgResetDefaultParam( DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom )
{
	unsigned int headLen	= sizeof(DCP_HEAD_T);    

	ParamSetDefaultParam();

    LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO, "ResetDefaultParam.");
	PacketClientHead(msgHead, SV_MSG_RES_RESET_DEFAULT_PARAM, 0, 0, 0);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
    RebootSendMessage();

	return 0;
}

int SVMsgHeartbeatReq( DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom )
{    
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
    
	PacketClientHead( msgHead, SV_MSG_RES_HEARTBEAT, 0, 0, ERROR_TYPE_SUCCESSFUL );
    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
	SVPrint("%s!\r\n", __FUNCTION__ );

	return 0;
}


int SVMsgGetAllParam(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
	int channel = (int)msgHead->subType;
	SYS_CONFIG	allParamSys;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(allParamSys);

	ret = ParamGetAllParam( &allParamSys );
	if(FI_SUCCESS == ret)
    {
    	ret = ERROR_TYPE_SUCCESSFUL;
    }
	else
    {
    	ret = ERROR_TYPE_GET_PARAM_FAILED;
    }

	PacketClientHead(msgHead, SV_MSG_RES_GET_ALL_PARAM, channel, dataLen, ret);
        
	memcpy(clientCom->writeBuf, msgHead, headLen);
	memcpy(clientCom->writeBuf+headLen, &allParamSys, dataLen);

	clientCom->writeBufDataSize = headLen + dataLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;
}

int SVMsgSetAllParam(DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom)
{
	int ret;
    int curTime;
	static int lastSetTime = 0;
	SYS_CONFIG allParam;
	unsigned int headLen	= sizeof(DCP_HEAD_T);    
	unsigned int dataLen	= sizeof(allParam);

	memcpy(&allParam, clientCom->readBuf+headLen, dataLen);

	ret = ParamSetAllParam( &allParam );    
	if(FI_SUCCESS == ret) 	ret = ERROR_TYPE_SUCCESSFUL;
	else                 	ret = ERROR_TYPE_SET_PARAM_FAILED;    
    
    
    curTime = SysRunTimeGet();
    if(curTime - lastSetTime > 600)
    {
        lastSetTime = curTime;
        LogAdd(0xff, LOG_TYPE_DATABASE, LOG_LEVEL_INFO, "SVMsgSetAllParam");
    }

	PacketClientHead(msgHead, SV_MSG_RES_SET_ALL_PARAM, 0, 0, ret);    
	memcpy(clientCom->writeBuf, msgHead, headLen);    
	clientCom->writeBufDataSize = headLen;
    
	SVPrint("%s,ret(%d)!\r\n", __FUNCTION__, ret);
	return 0;    
}


