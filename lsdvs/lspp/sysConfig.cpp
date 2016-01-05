/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此文件实现了系统配置相关的函数，
**            用于其它模块获取、设置、保存系统配置。
**  date           :  2013.11.11
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/reboot.h>
#include <unistd.h>
#include <stdlib.h>

#include "const.h"
#include "flash.h"
#include "mutex.h"
#include "strSafe.h"
#include "message.h"
#include "debug.h"
#include "lang.h"
#include "timeExchange.h"
#include "configManage.h"

#include "sysConfig.h"
#include "public.h"
#include "fitLspp.h"

static SYSCONFIG g_SysConfig;
static CMutexLockRecursive g_MutexConfig;

static void InitSysConfigHeader( SYS_CONFIG_HEADER *pHeader )
{
	if ( pHeader != NULL )
    {
    	memset( pHeader, 0, sizeof(*pHeader) );
    	pHeader->size = sizeof( SYSCONFIG );
    	pHeader->serialNo[0] = '\0';
        strlcpy( pHeader->signature, 
            	SYS_CONFIG_SIGNATURE,
            	sizeof(pHeader->signature) );
    	strlcpy( pHeader->configureVersion,
            	SYS_CONFIG_VERSION_CONFIG,
            	sizeof(pHeader->configureVersion) );
    	strlcpy( pHeader->softwareVersion,
            	SYS_CONFIG_VERSION_SOFTWARE,
            	sizeof(pHeader->softwareVersion) );
    	strlcpy( pHeader->hardwareVersion,
            	SYS_CONFIG_VERSION_HARDWARE,
            	sizeof(pHeader->hardwareVersion) );
//    	DM2016ReadSN( pHeader->serialNo );
    }
}

static void InitSysConfigBase( SYS_CONFIG_BASE *pBase )
{
	if ( pBase != NULL )
    {
        strlcpy( pBase->devName, SYS_CONFIG_DEVICE_NAME, sizeof(pBase->devName) );
    	pBase->devId[0]            = '\0';
    	pBase->chipNum	        = 1;
    	pBase->channelNum	    = REAL_CHANNEL_NUM;    
    	pBase->ptzNum	        = DEFAULT_PTZ_PROTOCOL_NUM;
    	pBase->langID	        = LANG_CHINESE;
    	pBase->timeZone	        = SYS_CONFIG_DEFAULT_TIME_ZONE;
    	pBase->timeIntZone      = SYS_CONFIG_DEFAULT_TIME_ZONE;
    	time_t curTime	        = time( NULL );

        /* 2013-08-28 lpx: Add shopID for Ftp upload, begin */
    	pBase->shopID	        = 0;
        /* 2013-08-28 lpx: Add shopID for Ftp upload, end */
        
    	struct tm gTm;
    	if ( gmtime_r( &curTime, &gTm ) != NULL )
        {
        	if ( (gTm.tm_year + 1900) < 2012 ) 
            {
            	memset( &gTm, 0, sizeof(gTm) );
            	gTm.tm_year	= 2012 - 1900;
            	gTm.tm_mon	= 0;
            	gTm.tm_mday	= 1;
            }
        	pBase->year   = gTm.tm_year - 100;
        	pBase->month  = gTm.tm_mon + 1;
        	pBase->day    = gTm.tm_mday;
        	pBase->hour   = gTm.tm_hour;
        	pBase->minute = gTm.tm_min;
        	pBase->second = gTm.tm_sec;
        }

    	pBase->ntpType = 0;
    	pBase->ntpPort = SYS_CONFIG_DEFAULT_NTP_PORT;
    	strlcpy( pBase->ntpAddr, SYS_CONFIG_DEFAULT_NTP_ADDR, sizeof(pBase->ntpAddr) );
        
    	pBase->videoStandard	    = 0;
    	pBase->serverPort	        = 6010;
    	pBase->httpPort	            = 80;
    	pBase->talkPort	            = 0;
    	pBase->streamPort	        = 0;
    	pBase->talkVoiceInLevel	    = 0;
    	pBase->talkVoiceOutLevel	= 0;
    	pBase->dsEnableFlag	        = 0;
    	pBase->videoPortInput	    = REAL_CHANNEL_NUM;
    	pBase->videoPortOutput	    = 1;
    	pBase->audioPortInput	    = 1;
    	pBase->audioPortOutput	    = 1;
    	pBase->alarmPortInput	    = 1;
    	pBase->alarmPortOutput	    = 1;
    }
}

static void InitSysConfigNetwork( SYS_CONFIG_NETWORK_F *pNetwork )
{    
	if ( pNetwork != NULL )
    {
    	pNetwork->netType = 1;
    	pNetwork->dhcpType = 0;    
    	strlcpy( pNetwork->ip, SYS_CONFIG_DEFAULT_IP, sizeof(pNetwork->ip) );
    	strlcpy( pNetwork->netmask, SYS_CONFIG_DEFAULT_NETMASK, sizeof(pNetwork->netmask) );
    	strlcpy( pNetwork->gateway, SYS_CONFIG_DEFAULT_GATEWAY, sizeof(pNetwork->gateway) );
    	strlcpy( pNetwork->dns, SYS_CONFIG_DEFAULT_DNS, sizeof(pNetwork->dns) );
    	strlcpy( pNetwork->dns2, SYS_CONFIG_DEFAULT_DNS2, sizeof(pNetwork->dns2) );
    	strlcpy( pNetwork->mac, SYS_CONFIG_DEFAULT_MAC, sizeof(pNetwork->mac) );
    	strlcpy( pNetwork->broadAddr, SYS_CONFIG_DEFAULT_BROADADDR, sizeof(pNetwork->broadAddr) );
    	strlcpy( pNetwork->webPort, SYS_CONFIG_DEFAULT_WEBPORT, sizeof(pNetwork->webPort) );
    }
}

static void InitSysConfigUser( SYS_CONFIG_USER *pUser )
{
	if ( pUser != NULL )
    {
    	SYS_CONFIG_USER admin = {
        	SYS_CONFIG_DEFAULT_USER,
        	SYS_CONFIG_DEFAULT_PWD,
        	1,
        	1,
        };
        *pUser = admin;
    }
}

static void InitSysConfigPtzProtocol( SYS_CONFIG_PTZ_PROTOCOL *pPtzProtocol )
{
	if ( pPtzProtocol != NULL )
    {
    	SYS_CONFIG_PTZ_PROTOCOL pelcoP = {
            "PELCO-P", 0,
        	0x20, 0x00, 0x40,
        	0x20, 0x00, 0x3F,
        	0x00, 0x00, 0x00, 
        	0x01, 0x01, 0x14,
            {
                { "HOME",    "A0Xx00070063AFXOR(0-6)" },
                { "RIGHT",    "A0Xx0002Vv00AFXOR(0-6)" },
                { "LEFT",    "A0Xx0004Vv00AFXOR(0-6)" }, 
                { "UP",        "A0Xx000800WwAFXOR(0-6)" },
                { "DOWN",    "A0Xx001000WwAFXOR(0-6)" },
                { "STOP",    "A0Xx00000000AFXOR(0-6)" },
                { "TELE",    "A0Xx00200000AFXOR(0-6)" },
                { "WIDE",    "A0Xx00400000AFXOR(0-6)" },
                { "ZSTOP",    "A0Xx00000000AFXOR(0-6)" },
                { "FAR",    "A0Xx01000000AFXOR(0-6)" },
                { "NEAR",    "A0Xx02000000AFXOR(0-6)" },
                { "FSTOP",    "A0Xx00000000AFXOR(0-6)" },
                { "POS",    "A0Xx000300PpAFXOR(0-6)" },
                { "GOTO",    "A0Xx000700PpAFXOR(0-6)" },
                { "CLEAR",    "A0Xx000500PpAFXOR(0-6)" },
                { "APA",    "A0Xx04000000AFXOR(0-6)" },
                { "APL",    "A0Xx08000000AFXOR(0-6)" },
                { "ASTOP",    "A0Xx00000000AFXOR(0-6)" },
            },
        	0,
        };
    	SYS_CONFIG_PTZ_PROTOCOL pelcoD = {
            "PELCO-D", 0,
        	0x3f, 0x00, 0x40, 
        	0x20, 0x00, 0x3F, 
        	0x00, 0x00, 0x00, 
        	0x01, 0x01, 0x14,
            {
                { "HOME",    "FFXx00070063SUM(1-5)" },
                { "RIGHT",    "FFXx0002Vv00SUM(1-5)" },
                { "LEFT",    "FFXx0004Vv00SUM(1-5)" },
                { "UP",        "FFXx000800WwSUM(1-5)" },
                { "DOWN",    "FFXx001000WwSUM(1-5)" },
                { "STOP",    "FFXx00000000SUM(1-5)" },
                { "TELE",    "FFXx00200000SUM(1-5)" },//变倍加
                { "WIDE",    "FFXx00400000SUM(1-5)" },//变倍减
                { "ZSTOP",    "FFXx00000000SUM(1-5)" },
                { "FAR",    "FFXx00800000SUM(1-5)" },//焦距减
                { "NEAR",    "FFXx01000000SUM(1-5)" },//焦距加
                { "FSTOP",    "FFXx00000000SUM(1-5)" },
                { "POS",    "FFXx000300PpSUM(1-5)" },
                { "GOTO",    "FFXx000700PpSUM(1-5)" },
                { "CLEAR",    "FFXx000500PpSUM(1-5)" },
                { "APA",    "FFXx02000000SUM(1-5)" },//光圈加
                { "APL",    "FFXx04000000SUM(1-5)" },//光圈减
                { "ASTOP",    "FFXx00000000SUM(1-5)" },
            },
        	0,
        };
    	SYS_CONFIG_PTZ_PROTOCOL sonyEviD100P = {
            "Sony EVI-D100/P", 1,
        	0x04, 0x01, 0x18, 
        	0x03, 0x01, 0x14, 
        	0x00, 0x00, 0x07, 
        	0x00, 0x00, 0x05,
            {
                { "HOME",    "8x010604FF" },
                { "LEFT",    "8x010601VvWw0103FF" },
                { "RIGHT",    "8x010601VvWw0203FF" },
                { "UP",        "8x010601VvWw0301FF" },
                { "DOWN",    "8x010601VvWw0302FF" },
                { "STOP",    "8x010601VvWw0303FF" },
                { "TELE",    "8x0104072zFF" },
                { "WIDE",    "8x0104073zFF" },
                { "ZSTOP",    "8x01040700FF" },
                { "AF",        "8x01043802FF" },
                { "MF",        "8x01043803FF" },
                { "FAR",    "8x01040802FF" },
                { "NEAR",    "8x01040803FF" },
                { "FSTOP",    "8x01040800FF" },
                { "POS",     "8x01043F010pFF" },    // set position
                { "GOTO",    "8x01043F020pFF" },    // goto position
                { "CLEAR",    "8x01043F000pFF" },    // clear position
                { "RESET",    "8x010605FF" },
                { "left",    "8x010603VvWw0F0F0B0800000000FF" },    // step LEFT
                { "right",    "8x010603VvWw0000040800000000FF" },    // step RIGHT
                { "up",        "8x010603VvWw0000000000000102FF" },    // step UP
                { "down",    "8x010603VvWw000000000F0F0E0EFF" },    // step DOWN
            },
        	0,
        };    
        *pPtzProtocol++ = pelcoP;
        *pPtzProtocol++ = pelcoD;
        *pPtzProtocol++ = sonyEviD100P;
    }
}
    
static void InitSysConfigPtzControl( SYS_CONFIG_PTZ_CONTROL *pPtzControl )
{
	if ( pPtzControl != NULL )
    {
    	SYS_CONFIG_PTZ_CONTROL pelcoD = {
             "PELCO-D", 9600, 8, 1, 0,
         	0x01, 0x3f, 0x20, 0x00, 0x01 
         };
    	for ( int channel = 0; channel < MAX_CHANNEL_NUM; ++channel )
             *pPtzControl++ = pelcoD;
     }
}

static void InitSysConfigVideoParam( SYS_CONFIG_VIDEO_BASE_PARAM *pVideoParam )
{    
	if ( pVideoParam != NULL )
    {
    	for ( int channel = 0; channel < MAX_CHANNEL_NUM; ++channel )
        {
        	pVideoParam->brightness	    = 128;
        	pVideoParam->contrast	    = 128;
        	pVideoParam->hue	        = 0;
        	pVideoParam->saturation	    = 128;
        	pVideoParam->exposure	    = 0;
        	pVideoParam++;
        }
    }
}

static void InitSysConfigOsdInfo( SYS_CONFIG_OSD_INFO *pOsdInfo )
{
	if ( pOsdInfo != NULL )
    {
    	memset( pOsdInfo, 0, sizeof(*pOsdInfo) );    
    	for ( int i = 0; i < MAX_CHANNEL_NUM; ++i )
        {
        	pOsdInfo->osdLogo.yPos= 30;
        	pOsdInfo++;        
        }
    }
}

static void InitSysConfigVideoEncode( SYS_CONFIG_VIDEO_ENCODE *pVideoEncode )
{
	if ( pVideoEncode != NULL )
    {
    	for ( int i = 0; i < MAX_CHANNEL_NUM; ++i )
        {
        	pVideoEncode->videoStandard	    = 0;
        	pVideoEncode->resolution	    = 1;
        	pVideoEncode->bitrateType	    = 0;
        	pVideoEncode->quality	        = 384;
        	pVideoEncode->frameRate	        = 25;
        	pVideoEncode->iFrameInterval	= 50;
        	pVideoEncode->preferFrame	    = 1;
        	pVideoEncode->qp	            = 32;
        	pVideoEncode->encodeType	    = 1;
        
        /*
        	pVideoEncode->videoStandard	    = DEFAULT_ENCODE_STANDARD;
        	pVideoEncode->resolution	    = DEFAULT_ENCODE_RESOLUTION;
        	pVideoEncode->bitrateType	    = DEFAULT_ENCODE_BITRATE_TYPE;
        	pVideoEncode->quality	        = DEFAULT_ENCODE_LEVEL;
        	pVideoEncode->frameRate	        = DEFAULT_ENCODE_FRAME_RATE;
        	pVideoEncode->iFrameInterval	= DEFAULT_ENCODE_FRAME_INTERVAL;
        	pVideoEncode->preferFrame	    = DEFAULT_ENCODE_PREFER_FRAME;
        	pVideoEncode->qp	            = DEFAULT_ENCODE_MAX_QP;
        	pVideoEncode->encodeType	    = DEFAULT_ENCODE_TYPE;
            */
        	pVideoEncode++;        
        }    
    }
}

static void InitSysConfigVideoSubEncode( SYS_CONFIG_VIDEO_ENCODE *pVideoSubEncode )
{
	if ( pVideoSubEncode != NULL )
    {
    	for ( int i = 0; i < MAX_CHANNEL_NUM; ++i )
        {
    /*    	pVideoSubEncode->videoStandard	= DEFAULT_ENCODE_STANDARD;
        	pVideoSubEncode->resolution	    = DEFAULT_ENCODE_SUB_RESOLUTION;
        	pVideoSubEncode->bitrateType	= DEFAULT_ENCODE_BITRATE_TYPE;
        	pVideoSubEncode->quality	    = DEFAULT_ENCODE_LEVEL;
        	pVideoSubEncode->frameRate	    = DEFAULT_ENCODE_FRAME_RATE;
        	pVideoSubEncode->iFrameInterval	= DEFAULT_ENCODE_FRAME_INTERVAL;
        	pVideoSubEncode->preferFrame	= DEFAULT_ENCODE_PREFER_FRAME;
        	pVideoSubEncode->qp	            = DEFAULT_ENCODE_MAX_QP;
        	pVideoSubEncode->encodeType	    = DEFAULT_ENCODE_TYPE;
            */
        	pVideoSubEncode++;        
        }    
    }
}

static void InitSysConfigAudioEncode( SYS_CONFIG_AUDIO_ENCODE *pAudioEncode )
{
	if ( pAudioEncode != NULL )
    {
    	for ( int i = 0; i < MAX_CHANNEL_NUM; ++i )
        {
        	pAudioEncode->sampleRate	= 8 * 1000;
        	pAudioEncode->bitWidth	    = 16;
    //    	pAudioEncode->encodeType	= AUDIO_FORMAT_G726_24;
        	pAudioEncode->amrMode	    = 0;
        	pAudioEncode->amrFormat	    = 0;
        	pAudioEncode++;
        }
    }
}

static void InitSysConfigAudioEncodeTalkback( SYS_CONFIG_AUDIO_ENCODE *pAudioEncode )
{    
	if ( pAudioEncode != NULL )
    {
    	pAudioEncode->sampleRate	= 1 * 1024;
    	pAudioEncode->bitWidth	    = 16;
    //	pAudioEncode->encodeType	= AUDIO_FORMAT_G726_24;
    	pAudioEncode->amrMode	    = 0;
    	pAudioEncode->amrFormat	    = 0;
    }
}

static void InitSysConfigEmailParam( SYS_CONFIG_EMAIL_PARAM *pEmailParam )
{
	if ( pEmailParam != NULL )
    {
    	memset( pEmailParam, 0, sizeof(*pEmailParam) );
    }
}

static void InitSysConfigAutoMaintainParam( SYS_CONFIG_AUTO_MAINTAIN_PARAM *pAutoMaintain )
{
	if ( pAutoMaintain != NULL )
    {
    	memset( pAutoMaintain, 0, sizeof(*pAutoMaintain) );
    }
}

static void InitSysConfigRecordParam( SYS_CONFIG_RECORD_PARAM *pRecordParam )
{
	if ( pRecordParam != NULL )
    {
    	memset( pRecordParam, 0, sizeof(*pRecordParam) );
    }
}

static void InitSysConfigRecordPublic( SYS_CONFIG_RECORD_PUBLIC *pRecordPublic )
{
	if ( pRecordPublic != NULL )
    {
    	pRecordPublic->loopRecordFlag = 0;
    }
}

static void InitSysConfigChannelParam( SYS_CONFIG_CHANNEL_PARAM *pChannelParam )
{
	if ( pChannelParam != NULL )
    {
    	char channelName[32] = { 0 };
    	for ( int i = 0; i < MAX_CHANNEL_NUM; ++i )
        {
        	snprintf( channelName, sizeof(channelName), "CHANNEL %d", i );
        	strlcpy( pChannelParam->channelName, channelName, sizeof(pChannelParam->channelName) );
        	pChannelParam->voiceInput  = 16;
        	pChannelParam->voiceOutput = 5;
        	pChannelParam++;
        }
    }
}

static void InitSysConfigAlarmParam( SYS_CONFIG_ALARM_PARAM *pAlarmParam )
{
	if ( pAlarmParam != NULL )
    {
    	memset( (char *)pAlarmParam, 0, sizeof(*pAlarmParam) );
    }
}

static void InitSysConfigRs232Param( SYS_CONFIG_RS232_PARAM *pRs232Param )
{
	if ( pRs232Param != NULL )
    {
    	pRs232Param->baudRate	= 9600;
    	pRs232Param->dataBits	= 8;
    	pRs232Param->stopBits	= 1;
    	pRs232Param->parity	    = 0;
    }
}

static void InitSysConfigRs485Param(SYS_CONFIG_RS485_PARAM * pRs485Param)
{
	if ( pRs485Param != NULL )
    {
    	pRs485Param->baudRate	= 9600;
    	pRs485Param->dataBits	= 8;
    	pRs485Param->stopBits	= 1;
    	pRs485Param->parity	    = 0;
    }
}

static void InitSysConfigDoorRule( SYS_CONFIG_DOOR_RULE *pDoorRule )
{
	if ( pDoorRule != NULL )
    {
    	pDoorRule->pcType	= 1;
    	pDoorRule->doorType	= 1;
    	pDoorRule->doorRule	= 1; 
    }
}

static void InitSysConfigDataUpload( SYS_CONFIG_DATA_UPLOAD *pDataUpload )
{
	if ( pDataUpload != NULL )
    {    
    	for ( int i = 0; i < MAX_DATA_UPLOAD_NUM; ++i )
        {
        	pDataUpload->flag	    = 0;
        	pDataUpload->port	    = 4066;
        	pDataUpload->interval	= 10;
        	pDataUpload->lastTime	= 0;
        	pDataUpload->mode	    = 0;
        	strlcpy( pDataUpload->ip, "192.168.18.25", sizeof(pDataUpload->ip) );
        	pDataUpload++;
        }
    }
}

static void InitSysConfigPcParam( SYS_CONFIG_PC_PARAM *pPcParam )
{
	if ( pPcParam != NULL )
    {
    	for ( int i = 0; i < MAX_CHANNEL_NUM; ++i )
        {
        	pPcParam->nWidth	    = CIF_WIDTH;
        	pPcParam->nHeight	    = CIF_HEIGHT;
        	pPcParam->nRoiLeft	    = 50;
        	pPcParam->nRoiTop	    = 42;
        	pPcParam->nRoiRight	    = CIF_WIDTH - 50;
        	pPcParam->nRoiBottom	= CIF_HEIGHT - 42;
        	pPcParam->bOriVertical	= 1;
        	pPcParam->bOriEnter	    = 0;
        	pPcParam->bOpenPcDetect	= 1;
        	pPcParam->videoMode	    = TYPE_VIDEO_MODE_CIF;
            
        	pPcParam->nLoiterTime	    = 1000;         
        	pPcParam->nFPSOfDet         = 12;            
        	pPcParam->nMaxMatchDist     = DEF_MAXMATCHDIST;         
        	pPcParam->nStepMotion	    = 1;
        	pPcParam->nMaxStepLen	    = 0;  // 使用默认值

        	pPcParam->nMinStepCount	    = 0;  // 不起作用
        	pPcParam->nMinHeadWidth     = 40;    
        	pPcParam->nMaxHeadWidth	    = 60;  
          	pPcParam->bHighPrecision     = 1;    
        	pPcParam++;
        }
    }
}

#define MIDWARE_TRANSTYPE_TCP	0x01

static void InitSysConfigMidRegister( SYS_CONFIG_MIDWARE *midParam )
{
	char         	defIp[]     = "192.168.18.225";
	char         	user[]         = "admin";
	char         	passwd[]     = "88888888";
	unsigned short 	defPort     = 5155;    
    
	if ( midParam != NULL )
    {
    	for ( int i = 0; i < MAX_MIDWARE_USER_NUM; ++i )
        {        
        	memset( midParam, 0, sizeof(*midParam) );
        	if ( i == 0 )
            {
            	midParam->transType = MIDWARE_TRANSTYPE_TCP;
            	midParam->regEnable = FI_TRUE;
            	strncpy( midParam->net.ip, defIp, sizeof(midParam->net.ip) );
            	midParam->net.port = defPort;
            	strncpy( midParam->auth.user, user, sizeof(midParam->auth.user) );
            	strncpy( midParam->auth.passwd, passwd, sizeof(midParam->auth.passwd) );
            }
        	midParam++;
        }
    }
}

static void InitSysConfigPcConfig( SYS_CONFIG_PC_CONFIG *pPcConfig )
{
	if ( pPcConfig != NULL )
    {
    	memset(pPcConfig, 0, sizeof(SYS_CONFIG_PC_CONFIG));
    	pPcConfig->bSaveHistroyData	= 1;
    	snprintf(pPcConfig->clrCountTime[0], 9, "%s", "13:00:00");
    	snprintf(pPcConfig->clrCountTime[1], 9, "%s", "20:00:00");
    	pPcConfig->upModNum = 5;
    	pPcConfig->limitedNumber = 100000;
    }
}

static void InitSysConfigLpParam( SYS_CONFIG_LP_PARAM *pLpParam )
{
	if ( pLpParam != NULL )
    {
    	for ( int i = 0; i < MAX_CHANNEL_NUM; ++i )
        {
        	pLpParam->nWidth	            = CIF_WIDTH;
        	pLpParam->nHeight	            = CIF_HEIGHT;
        	pLpParam->nMinTrackFrame	    = 2;
        	pLpParam->nMaxLostFrame	        = 5;
        	pLpParam->nMaxDistance	        = 200;
            
        	if ( i == 0 ) pLpParam->nDetectCarType = 2;
        	else pLpParam->nDetectCarType = 1;
            
        	pLpParam->bDetectPlate	        = 1;
        	pLpParam->bEnableTrackCar	    = 0;
        	pLpParam->bEnableTrackPlate	    = 0;    
        	pLpParam++;
        }
    }
}

static void InitSysConfigABDoorParam( SYS_CONFIG_ABDOOR_PARAM *pABDoorParam )
{
	if ( pABDoorParam != NULL )
    {
    	for ( int i = 0; i < MAX_CHANNEL_NUM; ++i )
        {
        	pABDoorParam->nWidth	        = CIF_WIDTH;
        	pABDoorParam->nHeight	        = CIF_HEIGHT;
        	pABDoorParam->nRoiLeft	        = 10;
        	pABDoorParam->nRoiTop	        = CIF_HEIGHT / 4;
        	pABDoorParam->nRoiRight	        = CIF_WIDTH - 10;
        	pABDoorParam->nRoiBottom	    = CIF_HEIGHT * 3 / 4;
        	pABDoorParam->bOriVertical	    = 1;
        	pABDoorParam->nLoiterTime	    = 30 * 1000;
        	pABDoorParam->nFPSOfDet	        = 0;
        	pABDoorParam->nMaxMatchDist	    = 30;
        	pABDoorParam->bOriEnter	        = 0;
        	pABDoorParam->bOpenABDoorDetect	= 1;
        	pABDoorParam->stayerNum	        = 1;
        	pABDoorParam->sensitivity	    = 6;
        	pABDoorParam->videoMode	        = TYPE_VIDEO_MODE_CIF;
        	pABDoorParam++;
        }
    }
}

static void InitSysConfigFivParam( SYS_CONFIG_FIV_PARAM *pFivParam )
{
	if ( pFivParam != NULL )
    {
    	for ( int i = 0; i < MAX_CHANNEL_NUM; ++i )
        {
        	pFivParam->fAlpha	        = 0.01;
        	pFivParam->fFactor	        = 2.5;
        	pFivParam->fT	            = 0.7;
        	pFivParam->nWindowSize	    = 3;
        	pFivParam->nTargetSize	    = 36;
        	pFivParam->nMaxLost	        = 3;
        	pFivParam->nMinFrame	    = 1;
        	pFivParam->nMaxDistance	    = 10;
        	pFivParam->bOpenFivDetect	= 1;
        	pFivParam++;
        }
    }
}

static void InitSysConfigFivConfig( SYS_CONFIG_FIV_CONFIG *pFivConfig )
{
	if ( pFivConfig != NULL )
    {
    	for ( int i = 0; i < MAX_CHANNEL_NUM; ++i )
        {
        	memset( pFivConfig, 0, sizeof(*pFivConfig) );
        	pFivConfig->width	= CIF_WIDTH;
        	pFivConfig->height	= CIF_HEIGHT;
        	pFivConfig->left	= CIF_WIDTH / 2;
        	pFivConfig->top	    = 10;
        	pFivConfig->right	= CIF_WIDTH - 10;
        	pFivConfig->bottom	= CIF_HEIGHT - 10;
        	pFivConfig->fivFlag	= 0;
        	pFivConfig->fivType	= 1;
        	pFivConfig->rgn[0].rgnType	= 1;
        	pFivConfig->rgn[0].rect.x1	= 20;
        	pFivConfig->rgn[0].rect.y1	= 20;
        	pFivConfig->rgn[0].rect.x2	= CIF_WIDTH - 20;
        	pFivConfig->rgn[0].rect.y2	= CIF_HEIGHT - 20;    
        	pFivConfig++;
        }
    }
}

static void InitSysConfigRf433Config( SYS_CONFIG_RF433M_PARAM *rf433m )
{
	if ( rf433m != NULL )
    {
    	memset( rf433m, 0x00, sizeof(*rf433m) );
        //rf433m->num = 4;
    }
}
        
static void InitSysConfigEyeConfig( SYS_CONFIG_EYE_CONFIG *pEyeConfig )
{
	if ( pEyeConfig != NULL )
    {
    	for ( int i = 0; i < MAX_CHANNEL_NUM; ++i )
        {
        	pEyeConfig->bFaceDetect	                = 1;            
        	pEyeConfig->sensitivity[0].carSpeed	    = 40;
        	pEyeConfig->sensitivity[0].detectTime	= 3000;
        	pEyeConfig->sensitivity[1].carSpeed	    = 80;
        	pEyeConfig->sensitivity[1].detectTime	= 2000;    
        	pEyeConfig->sensitivity[2].carSpeed	    = 120;
        	pEyeConfig->sensitivity[2].detectTime	= 1200;    
        	pEyeConfig->bslSensitivity	            = 3;
        	pEyeConfig++;
        }
    }
}

void InitSysConfigIoConfig( SYS_CONFIG_IO_CONFIG *pIoConfig )
{
	if ( pIoConfig != NULL )
    {
    	memset( pIoConfig, 0x00, sizeof(*pIoConfig) );
    	pIoConfig->bNormalOpen = 0; //默认常敝	    
    }
}

static void InitSysConfigsynovateSys( SYS_CONFIG_SYNOVATE *pSynovate )
{
    //char buf[6];
    //char ch = 0;
	if( NULL != pSynovate )
    {
    	memset( pSynovate, 0x00, sizeof(SYS_CONFIG_SYNOVATE) );
    }
    
//	InitSynovateInitCLandSN();
//	SynovateSnAndChFuc(SYNOVATE_SN_CH_GET,pSynovate->sn,&(pSynovate->Ch));
}

static void InitSysConfigRf433mPublic( SYS_CONFIG_RF433M_PUBLIC *pRf433mPublic )
{
	if ( pRf433mPublic != NULL )
    {
    	memset( pRf433mPublic, 0x00, sizeof(*pRf433mPublic) );
    }    
}


static void InitSysConfigUbootEnvBackup( SYS_CONFIG_UBOOT_ENV_BACKUP *pUbootEnvBackup )
{
	if( NULL != pUbootEnvBackup )
    {        
    	memset( pUbootEnvBackup, 0x00, sizeof(*pUbootEnvBackup) );
    }
}

void InitSysWlPlateformConfig( SYS_CONFIG_WL_PLATFORM *pWlPlateform )
{
	if( NULL != pWlPlateform )
    {
    	memset( pWlPlateform, 0x00, sizeof( *pWlPlateform ) );
    	strcpy( pWlPlateform->ipAddr, SYS_CONFIG_DEFAULT_WLPLATFORM_IP );
    	pWlPlateform->port = SYS_CONFIG_DEFAULT_WLPLATFORM_PORT;
    }
}

static void InitSysConfigWifiConnect( SYS_CONFIG_WIFI_CONNECT *pWifiConnect )
{
	if( NULL != pWifiConnect )
    {
    	memset( pWifiConnect, 0x00, sizeof(pWifiConnect) );
    }
}
static void InitSysConfigThreegNetParam(SYS_CONFIG_THREEG_NET_PARAM *pThreegNetParam)
{
	if ( pThreegNetParam != NULL )
    {
    	memset(pThreegNetParam, 0x00, sizeof(*pThreegNetParam));
    	memcpy(pThreegNetParam->smsAuthPasswd, "666666", 6);
    }
}
static void InitSysConfigWifiNetwork( SYS_CONFIG_WIFI_NETWORK *pWifiNetwork )
{
	if( NULL != pWifiNetwork )
    {
    	memset( pWifiNetwork, 0x00, sizeof(pWifiNetwork) );
    	pWifiNetwork->netType = 1;
    }
}
static void InitSysConfigFtpRec(SYS_CONFIG_FTP_REC *pFtpRec)
{
	if (pFtpRec != NULL)
    {
    	char *host = (char*)"ftp://192.168.18.218";
    	char *user = (char*)"admin";
    	char *pwd = (char*)"admin";
        
    	memset(pFtpRec, 0, sizeof(SYS_CONFIG_FTP_REC));
    	strncpy(pFtpRec->host, host, strlen(host)+1); 
    	strncpy(pFtpRec->user, user, strlen(user)+1); 
    	strncpy(pFtpRec->passwd, pwd, strlen(pwd)+1); 
    }
}
static void InitSysConfigFtpUpload(SYS_CONFIG_FTP_UPLOAD *pFtpUpload)
{
	if (NULL != pFtpUpload)
    {
    	char *host = (char*)"192.168.18.184";
    	char *user = (char*)"lpx";
    	char *pwd = (char*)"lpx";

    	memset(pFtpUpload, 0, sizeof(SYS_CONFIG_FTP_UPLOAD));

    	pFtpUpload->nUploadInterval = 60;
    	pFtpUpload->nUploadFileFormat = 1;

    	pFtpUpload->nRunFlag = 0;
    	pFtpUpload->nPort = 21;
    	strncpy(pFtpUpload->strHost, host, strlen(host)+1); 
    	strncpy(pFtpUpload->strUserName, user, strlen(user)+1); 
    	strncpy(pFtpUpload->strPassword, pwd, strlen(pwd)+1); 

    }

}

/*
static void InitSysConfigClearPcPeriod(SYS_CONFIG_CLEAR_PC_PERIOD *pClearPcPeriod)
{
	if ( pClearPcPeriod != NULL )
    {
    	memset(pClearPcPeriod, 0, sizeof(SYS_CONFIG_CLEAR_PC_PERIOD));
    	memcpy(pClearPcPeriod->firstPeriod, "99:99:99", 8);
    	memcpy(pClearPcPeriod->secondPeriod, "99:99:99", 8);
    }
}
*/

int IsHeaderValid( SYSCONFIG *pSysConfig )
{
	if ( pSysConfig != NULL )
    {
    	SYS_CONFIG_HEADER *pHeader = &pSysConfig->header;
    	if ( strncmp(pHeader->signature, SYS_CONFIG_SIGNATURE, 
            	sizeof(pHeader->signature)) ) return 0;
    	if ( strncmp(pHeader->configureVersion, g_SysConfig.header.configureVersion,
            	sizeof(pHeader->configureVersion)) ) return 0;
    	return 1;
    }
	return 0;
}

void ReleaseUbootEnvIfUbootUpdate()
{
	int 	setMacFlag = 0;
	char 	mac[24] = { 0 };
	char	*pMac = NULL;
	char	ubootEnvMacName[] = "ethaddr";
	unsigned int 	ubootUpdateFlag = 0;
	int 	ret;
    
	ret = CfgManageGetUbootEnvEraseFlag( &ubootUpdateFlag );
    
	if( ret == 0 && UBOOT_ENV_ERASE_FLAG == ubootUpdateFlag )
    {
    	setMacFlag = 1;
    	ret = CfgManageGetUbootEnvBackupMac( mac );
        
    	if( ret != 0 
            || 0 == strcasecmp(mac, UBOOT_ENV_DEFAULT_MAC) 
            || 0 == strcasecmp(mac, UBOOT_ENV_DEFAULT_MAC_CIM) )    
        {            
        	FiNetGenRandMac( mac );            
        }        
    }
	else	// 查看u-boot里面的有无mac,或mac是否等于默认的
    {                
    	pMac = GetUbootEnv( ubootEnvMacName );
        
    	if( pMac == NULL || pMac[0] == '\0' 
            || 0 == strcasecmp(pMac, UBOOT_ENV_DEFAULT_MAC)
            || 0 == strcasecmp(pMac, UBOOT_ENV_DEFAULT_MAC_CIM) )
        {    
            //找不到MAC或者MAC是默认的,则产生一个随机的MAC地址
        	FiNetGenRandMac( mac );
        	setMacFlag = 1;
        }
    }    
    #if 0
	if( setMacFlag == 1 )
    {        
    	ret = CfgManageSetUbootEnvEraseFlag( 0 );
    	printf("### CfgManageSetUbootEnvEraseFlag, ret = %d!\r\n", ret);
    	fflush(stdout);
    	if( ret == 0 ) ret = FiNetSetMacAddrUboot( mac );    
    	printf("### FiNetSetMacAddrUboot, ret = %d!\r\n", ret);
    	fflush(stdout);
    	if( ret > 0 ) //有真正修改u-boot mac才重启
        {
            //FiNetSetMacAddr( mac );
        	printf("### NOTE:Set the mac(%s), and reboot now!\r\n", mac);
        	fflush(stdout);
        	RebootSystem();
        }
    }
    #endif
}

void UpdateSysConfig()
{
	int		gpioType	= 0;
	char	macAddr[24]    = { 0 };

	g_MutexConfig.Lock();
    
	InitSysConfigHeader( &g_SysConfig.header );
    
	g_SysConfig.base.channelNum = REAL_CHANNEL_NUM;            // 实际通道个数
	g_SysConfig.base.videoPortInput = REAL_CHANNEL_NUM;        // 视频输入端口个数
    
    //ReleaseUbootEnvIfUbootUpdate();
    
	if ( FiNetGetMacAddr( macAddr, sizeof(macAddr) ) == 0 )    // 物理地址
    	strlcpy( g_SysConfig.network.mac, macAddr, sizeof(g_SysConfig.network.mac) );
    
	InitSysConfigPtzProtocol( g_SysConfig.ptzProtocol );     // 默认PTZ参数不能修改
	gpioType = g_SysConfig.alarmParam.type;
	g_MutexConfig.Unlock();

//
// TODO !!!
//
///	SetAlarmType( gpioType );    // 设置告警类型: 常开, 常闭.
}

/* ------------------------------------------------------------------------- */
//
// 操作系统配置接口函数
//
int ReadSysConfig()
{
#if 0
	int nSize = sizeof( SYSCONFIG );
	SYSCONFIG *pSysConfig = ( SYSCONFIG * )Malloc( sizeof(SYSCONFIG) );
	if ( pSysConfig == NULL )
    {
    	FiPrint( "ReadSysConfig: Malloc Memory failed !!!\r\n" );
    	return -1;
    }
	memset( pSysConfig, 0, nSize );
	if ( ReadConfig( (char *)pSysConfig, nSize ) == -1 )
    {
    	FiPrint( "ReadSysConfig: Read Config Error !!!\r\n" );
    	Free( pSysConfig );
    	return -1;
    }
	PassData( (char *)pSysConfig, nSize );
	if ( !IsHeaderValid( pSysConfig ) )
    {
    	FiPrint( "ReadSysConfig: Read Config Header Error !!!\r\n" );
    	Free( pSysConfig );
    	return -1;        
    }
    
	if ( pSysConfig->header.size != nSize )
    {        
    	int nOldSize = pSysConfig->header.size;
    	if ( nOldSize > nSize ) nOldSize = nSize;
    	pSysConfig->header.size = nSize;
    	nSize = nOldSize;
    }
	g_MutexConfig.Lock();
	memcpy( &g_SysConfig, pSysConfig, nSize );    
	InitSysConfigsynovateSysSnAndCh(&g_SysConfig.synovateSys);/*synovate_add*/
	g_MutexConfig.Unlock();
    
	Free( pSysConfig );
    #endif 
	return 0;
}

int SyncSysConfig()
{
	int nRet = -1;
	SYSCONFIG *pSysConfig = ( SYSCONFIG * )Malloc( sizeof(SYSCONFIG) );
	if ( pSysConfig == NULL ) return nRet;
    
	g_MutexConfig.Lock();
	memcpy( pSysConfig, &g_SysConfig, sizeof(*pSysConfig) );
	g_MutexConfig.Unlock();    
	PassData( (char *)pSysConfig, sizeof(*pSysConfig) );
    //nRet = WriteConfig( (char *)pSysConfig, sizeof(*pSysConfig) );
	Free( pSysConfig );
	return nRet;
}

#if 0
int WriteSysConfig()
{
    //return MessageSend( MSG_ID_TIMER_WRITE_SYS_CONFIG );
}

void *WriteSysConfig( void *args )
{
	if ( MessageFind( MSG_ID_TIMER_WRITE_SYS_CONFIG ) )
    {
    	while ( MessageRecv( MSG_ID_TIMER_WRITE_SYS_CONFIG ) >= 0 )    { ; }
        
    	SYSCONFIG *pSysConfig = ( SYSCONFIG * )Malloc( sizeof(SYSCONFIG) );
    	if ( pSysConfig != NULL )
        {        
        	g_MutexConfig.Lock();
        	memcpy( pSysConfig, &g_SysConfig, sizeof(*pSysConfig) );
        	g_MutexConfig.Unlock();    
        	PassData( (char *)pSysConfig, sizeof(*pSysConfig) );
        	WriteConfigMsg( (char *)pSysConfig, sizeof(*pSysConfig) );
        	Free( pSysConfig );
        }
    }
	return NULL;
}
#endif
void InitDefaultSysConfig()
{
	g_MutexConfig.Lock();
    
	memset( &g_SysConfig, 0, sizeof(SYSCONFIG) );
	InitSysConfigHeader( &g_SysConfig.header );
	InitSysConfigBase( &g_SysConfig.base );
	InitSysConfigNetwork( &g_SysConfig.network );
	InitSysConfigUser( g_SysConfig.user );
	InitSysConfigPtzProtocol( g_SysConfig.ptzProtocol );
	InitSysConfigPtzControl( g_SysConfig.ptzControl );
	InitSysConfigVideoParam( g_SysConfig.videoParam );
	InitSysConfigOsdInfo( g_SysConfig.osdInfo );
	InitSysConfigVideoEncode( g_SysConfig.videoEncode );
	InitSysConfigVideoSubEncode( g_SysConfig.videoSubEncode );
	InitSysConfigAudioEncode( g_SysConfig.audioEncode );
	InitSysConfigAudioEncodeTalkback( &g_SysConfig.audioEncodeTalkback );
	InitSysConfigEmailParam( &g_SysConfig.emailParam );
	InitSysConfigAutoMaintainParam( &g_SysConfig.autoMaintainParam );
	InitSysConfigRecordParam( g_SysConfig.recordParam );
	InitSysConfigRecordPublic( &g_SysConfig.recordPublic );
	InitSysConfigChannelParam( g_SysConfig.channelParam );
	InitSysConfigAlarmParam( &g_SysConfig.alarmParam );
	InitSysConfigRs232Param( &g_SysConfig.rs232Param );    
	InitSysConfigRs485Param( &g_SysConfig.rs485Param );
	InitSysConfigDoorRule( &g_SysConfig.doorRule );
	InitSysConfigDataUpload( g_SysConfig.dataUpload );
	InitSysConfigPcParam( g_SysConfig.pcParam );    
	InitSysConfigMidRegister( g_SysConfig.midParam );
	InitSysConfigPcConfig( &g_SysConfig.pcConfig );
	InitSysConfigLpParam( g_SysConfig.lpParam );
	InitSysConfigABDoorParam( g_SysConfig.abDoorParam );
	InitSysConfigFivParam( g_SysConfig.fivParam );
	InitSysConfigFivConfig( g_SysConfig.fivConfig );
	InitSysConfigRf433Config( &g_SysConfig.rf433mParam );
	InitSysConfigEyeConfig( g_SysConfig.eyeConfig );
	InitSysConfigIoConfig( &g_SysConfig.ioConfig );
	InitSysConfigRf433mPublic( &g_SysConfig.rf433mPublic );
	InitSysConfigUbootEnvBackup( &g_SysConfig.ubootEnvBackup );
	InitSysWlPlateformConfig( &g_SysConfig.wlPlatform );
	InitSysConfigThreegNetParam( &g_SysConfig.threegNetParam);
	InitSysConfigWifiNetwork( &g_SysConfig.wifiNetwork );
	InitSysConfigWifiConnect( &g_SysConfig.wifiConnect );
	InitSysConfigsynovateSys(&g_SysConfig.synovateSys);        /*synovate_add*/
	InitSysConfigFtpRec(&g_SysConfig.ftpRec);
	InitSysConfigFtpUpload(&g_SysConfig.ftpUpload);            /* lpx: 初始化客流信息配置项初始值 */

	g_MutexConfig.Unlock();
}

int LsppInitSysConfig()
{    
	InitDefaultSysConfig();
    
	int nRet1 = ReadSysConfig();
    
    
	UpdateSysConfig();
    
    //int nRet2 = SyncSysConfig();
    
	return (nRet1) ? -1 : 0 ;
}

int InitThttpdConfig()
{
	unsigned short port = 0;
	g_MutexConfig.Lock();
	port = atoi( g_SysConfig.network.webPort );
	g_MutexConfig.Unlock();

    FILE *fp = fopen( SYS_CONFIG_THTTPD_CONFIG, "wb+" );
    if ( fp == NULL )
    {
    	int errNo = errno;
    	FiPrint( "Open thttpd config file error: %s !\r\n", strerror(errNo) );
    	return -1;
    }
	fprintf( fp, "port=%d\r\n", port );    // 向配置文件中写入网页端口号
    fflush( fp );
    fclose( fp );
	return 0;
}

int InitLangConfig()
{
	int nRet = ReadLangCfg();
	if ( nRet == 0 ) 
    {    
    	g_MutexConfig.Lock();
    	int langID = g_SysConfig.base.langID;
    	g_MutexConfig.Unlock();    
//    	SetIeDefaultLink( langID );
    	if ( langID == 0 || (nRet = SetLangID(langID)) != 0 ) 
        {
        	if( (nRet = SetLangID(SYS_CONFIG_DEFAULT_LANG)) == 0 )
            {
            	g_MutexConfig.Lock();
            	g_SysConfig.base.langID = SYS_CONFIG_DEFAULT_LANG;
            	g_MutexConfig.Unlock();    
            }
        }
    } 
	else
    {
    	FiPrint( "Read language config file error !\r\n" );
        
        // 读取语言配置出错则加载默认语言
    	if ( LoadLangText((char *)"SimplifiedChinese.lang") == 0 )
        {
        	g_MutexConfig.Lock();
        	g_SysConfig.base.langID = SYS_CONFIG_DEFAULT_LANG;
        	g_MutexConfig.Unlock();    
        }
    }
	return nRet;
}

int GetSysConfig( SYSCONFIG *pSysConfig )
{
	if ( pSysConfig == NULL )
    {
    	FiPrint( "GetSysConfig Param Error !\r\n" );
    	return -1;
    }
	g_MutexConfig.Lock();
	memcpy( pSysConfig, &g_SysConfig, sizeof(g_SysConfig) );
	g_MutexConfig.Unlock();    
	return 0;
}

int SetSysConfig( SYSCONFIG *pSysConfig )
{
	if ( pSysConfig == NULL )
    {
    	FiPrint( "SetSysConfig Param Error !\r\n" );
    	return -1;
    }
	g_MutexConfig.Lock();
	memcpy( &g_SysConfig, pSysConfig, sizeof(g_SysConfig) );
	g_MutexConfig.Unlock();    
	return 0;
}
    
int GetSysConfig( int type, void *configBuf, int configLen, int n )
{
	int nRet = -1;
	if ( configBuf == NULL || configLen <= 0 )
    {
    	FiPrint( "GetSysConfig Param Error : " );
    	FiPrint( "configBuf == NULL or configLen <= 0 !\r\n" );
    	return nRet;
    }
    
	g_MutexConfig.Lock();
	nRet = 0;    
	switch( type )
    {
	case TYPE_SYS_CONFIG_HEADER:
    	memcpy( configBuf, &g_SysConfig.header, configLen );
    	break;
	case TYPE_SYS_CONFIG_BASE:
    	memcpy( configBuf, &g_SysConfig.base, configLen );
    	break;
	case TYPE_SYS_CONFIG_NETWORK:
    	memcpy( configBuf, &g_SysConfig.network, configLen );
    	break;
	case TYPE_SYS_CONFIG_USER:
    	if ( 0 <= n && n < MAX_USER_NUM )
        	memcpy( configBuf, &g_SysConfig.user[n], configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_PTZ_PROTOCOL:
    	if ( 0 <= n && n < MAX_PTZ_NUM )
        	memcpy( configBuf, &g_SysConfig.ptzProtocol[n], configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_PTZ_CONTROL:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( configBuf, &g_SysConfig.ptzControl[n], configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_VIDEO_BASE_PARAM:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( configBuf, &g_SysConfig.videoParam[n], configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_OSD_INFO:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( configBuf, &g_SysConfig.osdInfo[n], configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_VIDEO_ENCODE:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( configBuf, &g_SysConfig.videoEncode[n], configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_VIDEO_SUB_ENCODE:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( configBuf, &g_SysConfig.videoSubEncode[n], configLen );
    	else nRet = -1;
    	break;        
	case TYPE_SYS_CONFIG_AUDIO_ENCODE:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( configBuf, &g_SysConfig.audioEncode[n], configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_AUDIO_ENCODE_TALKBACK:
    	memcpy( configBuf, &g_SysConfig.audioEncodeTalkback, configLen );
    	break;
	case TYPE_SYS_CONFIG_EMAIL_PARAM:
    	memcpy( configBuf, &g_SysConfig.emailParam, configLen );
    	break;
	case TYPE_SYS_CONFIG_AUTO_MAINTAIN_PARAM:
    	memcpy( configBuf, &g_SysConfig.autoMaintainParam, configLen );
    	break;
	case TYPE_SYS_CONFIG_RECORD_PARAM:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( configBuf, &g_SysConfig.recordParam[n], configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_RECORD_PUBLIC:
    	memcpy( configBuf, &g_SysConfig.recordPublic, configLen );
    	break;        
	case TYPE_SYS_CONFIG_CHANNEL_PARAM:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( configBuf, &g_SysConfig.channelParam[n], configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_ALARM_PARAM:
    	memcpy( configBuf, &g_SysConfig.alarmParam, configLen );
    	break;
	case TYPE_SYS_CONFIG_RS232_PARAM:
    	memcpy( configBuf, &g_SysConfig.rs232Param, configLen );
    	break;
	case TYPE_SYS_CONFIG_DOOR_RULE:
    	memcpy( configBuf, &g_SysConfig.doorRule, configLen );
    	break;
	case TYPE_SYS_CONFIG_DATA_UPLOAD:
    	if ( 0 <= n && n < MAX_DATA_UPLOAD_NUM )
        	memcpy( configBuf, &g_SysConfig.dataUpload[n], configLen );
    	else nRet = -1;
    	break;    
	case TYPE_SYS_CONFIG_PC_PARAM:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( configBuf, &g_SysConfig.pcParam[n], configLen );
    	else nRet = -1;
    	break;    
	case TYPE_SYS_CONFIG_MID_PARAM:        
    	if ( 0 <= n && n < MAX_MIDWARE_USER_NUM )
        	memcpy( configBuf, &g_SysConfig.midParam[n], configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_PC_CONFIG:
    	memcpy( configBuf, &g_SysConfig.pcConfig, configLen );
    	break;    
	case TYPE_SYS_CONFIG_LP_PARAM:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( configBuf, &g_SysConfig.lpParam[n], configLen );
    	else nRet = -1;
    	break;    
	case TYPE_SYS_CONFIG_ABDOOR_PARAM:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( configBuf, &g_SysConfig.abDoorParam[n], configLen );
    	else nRet = -1;
    	break;    
	case TYPE_SYS_CONFIG_FIV_PARAM:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( configBuf, &g_SysConfig.fivParam[n], configLen );
    	else nRet = -1;
    	break;    
	case TYPE_SYS_CONFIG_FIV_CONFIG:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( configBuf, &g_SysConfig.fivConfig[n], configLen );
    	else nRet = -1;
    	break;    
	case TYPE_SYS_CONFIG_RF433M_CONFIG:
    	memcpy( configBuf, &g_SysConfig.rf433mParam, configLen );
    	break;
	case TYPE_SYS_CONFIG_EYE_CONFIG:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( configBuf, &g_SysConfig.eyeConfig[n], configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_IO_CONFIG:
    	memcpy( configBuf, &g_SysConfig.ioConfig, configLen );
    	break;            
	case TYPE_SYS_CONFIG_RF433M_PUBLIC:
    	memcpy( configBuf, &g_SysConfig.rf433mPublic, configLen );
    	break;    
	case TYPE_SYS_CONFIG_UBOOT_ENV_BACKUP:
    	memcpy( configBuf, &g_SysConfig.ubootEnvBackup, configLen );
    	break;    
	case TYPE_SYS_CONFIG_LED_SHOW_SETTING:
    	memcpy( configBuf, &g_SysConfig.ledShowSetting, configLen );
    	break;    
	case TYPE_SYS_CONFIG_LED_BOARD_SETTING:
    	memcpy( configBuf, &g_SysConfig.ledBoardSetting, configLen );
    	break;    
	case TYPE_SYS_CONFIG_GROUP_SETTING:
    	memcpy( configBuf, &g_SysConfig.groupSetting, configLen );
    	break;
	case TYPE_SYS_CONFIG_WL_PLATFORM:
    	memcpy( configBuf, &g_SysConfig.wlPlatform, configLen );
    	break;
	case TYPE_SYS_CONFIG_THREEG_NET_PARAM:
    	memcpy( configBuf, &g_SysConfig.threegNetParam, configLen );
    	break;
	case TYPE_SYS_CONFIG_WIFI_NETWORK:        
    	memcpy( configBuf, &g_SysConfig.wifiNetwork, configLen );
    	break;
	case TYPE_SYS_CONFIG_WIFI_CONNECT:        
    	memcpy( configBuf, &g_SysConfig.wifiConnect, configLen );
    	break;
	case TYPE_SYS_CONFIG_SYNOVATE:        
    	memcpy( configBuf, &g_SysConfig.synovateSys, configLen );
    	break;
	case TYPE_SYS_CONFIG_RS485_PARAM:
    	memcpy( configBuf, &g_SysConfig.rs485Param, configLen );
    	break;
	case TYPE_SYS_CONFIG_FTP_REC:
    	memcpy( configBuf, &g_SysConfig.ftpRec, configLen );
    	break;
	case TYPE_SYS_CONFIG_FTP_UPLOAD:
    	memcpy( configBuf, &g_SysConfig.ftpUpload, configLen );
    	break;

	default:
    	FiPrint( "GetSysConfig Type Unknow !\r\n" );
    	nRet = -1;
    	break;
    }
	g_MutexConfig.Unlock();
    
	return nRet;
}

int SetSysConfig( int type, void *configBuf, int configLen, int n )
{
	int nRet = -1;
	if ( configBuf == NULL || configLen <= 0 )
    {
    	FiPrint( "SetSysConfig Param Error : " );
    	FiPrint( "configBuf == NULL or configLen <= 0 !\r\n" );
    	return nRet;
    }
    
	g_MutexConfig.Lock();
	nRet = 0;
	switch( type )
    {
	case TYPE_SYS_CONFIG_HEADER:
    	memcpy( &g_SysConfig.header, configBuf, configLen );
    	break;
	case TYPE_SYS_CONFIG_BASE:
    	memcpy( &g_SysConfig.base, configBuf, configLen );
    	break;
	case TYPE_SYS_CONFIG_NETWORK:
    	memcpy( &g_SysConfig.network, configBuf, configLen );
    	break;
	case TYPE_SYS_CONFIG_USER:
    	if ( 0 <= n && n < MAX_USER_NUM )
        	memcpy( &g_SysConfig.user[n], configBuf, configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_PTZ_PROTOCOL:
    	if ( 0 <= n && n < MAX_PTZ_NUM )
        	memcpy( &g_SysConfig.ptzProtocol[n], configBuf, configLen );
    	else nRet = -1;
    	break;        
	case TYPE_SYS_CONFIG_PTZ_CONTROL:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( &g_SysConfig.ptzControl[n], configBuf, configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_VIDEO_BASE_PARAM:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( &g_SysConfig.videoParam[n], configBuf, configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_OSD_INFO:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( &g_SysConfig.osdInfo[n], configBuf, configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_VIDEO_ENCODE:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( &g_SysConfig.videoEncode[n], configBuf, configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_VIDEO_SUB_ENCODE:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( &g_SysConfig.videoSubEncode[n], configBuf, configLen );
    	else nRet = -1;
    	break;        
	case TYPE_SYS_CONFIG_AUDIO_ENCODE:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( &g_SysConfig.audioEncode[n], configBuf, configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_AUDIO_ENCODE_TALKBACK:
    	memcpy( &g_SysConfig.audioEncodeTalkback, configBuf, configLen );
    	break;
	case TYPE_SYS_CONFIG_EMAIL_PARAM:
    	memcpy( &g_SysConfig.emailParam, configBuf, configLen );
    	break;
	case TYPE_SYS_CONFIG_AUTO_MAINTAIN_PARAM:
    	memcpy( &g_SysConfig.autoMaintainParam, configBuf, configLen );
    	break;
	case TYPE_SYS_CONFIG_RECORD_PARAM:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( &g_SysConfig.recordParam[n], configBuf, configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_RECORD_PUBLIC:
    	memcpy( &g_SysConfig.recordPublic, configBuf, configLen );
    	break;        
	case TYPE_SYS_CONFIG_CHANNEL_PARAM:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( &g_SysConfig.channelParam[n], configBuf, configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_ALARM_PARAM:
    	memcpy( &g_SysConfig.alarmParam, configBuf, configLen );
    	break;
	case TYPE_SYS_CONFIG_RS232_PARAM:
    	memcpy( &g_SysConfig.rs232Param, configBuf, configLen );
    	break;    
	case TYPE_SYS_CONFIG_DOOR_RULE:
    	memcpy( &g_SysConfig.doorRule, configBuf, configLen );
    	break;
	case TYPE_SYS_CONFIG_DATA_UPLOAD:
    	if ( 0 <= n && n < MAX_DATA_UPLOAD_NUM )
        	memcpy( &g_SysConfig.dataUpload[n], configBuf, configLen );
    	else nRet = -1;
    	break;    
	case TYPE_SYS_CONFIG_PC_PARAM:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( &g_SysConfig.pcParam[n], configBuf, configLen );
    	else nRet = -1;
    	break;    
	case TYPE_SYS_CONFIG_MID_PARAM:
    	if ( 0 <= n && n < MAX_MIDWARE_USER_NUM )
        	memcpy( &g_SysConfig.midParam[n], configBuf,  configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_PC_CONFIG:
    	memcpy( &g_SysConfig.pcConfig, configBuf, configLen );
    	break;    
	case TYPE_SYS_CONFIG_LP_PARAM:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( &g_SysConfig.lpParam[n], configBuf, configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_ABDOOR_PARAM:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( &g_SysConfig.abDoorParam[n], configBuf, configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_FIV_PARAM:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( &g_SysConfig.fivParam[n], configBuf, configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_FIV_CONFIG:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( &g_SysConfig.fivConfig[n], configBuf, configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_RF433M_CONFIG:
    	memcpy( &g_SysConfig.rf433mParam, configBuf, configLen );
    	break;        
	case TYPE_SYS_CONFIG_EYE_CONFIG:
    	if ( 0 <= n && n < MAX_CHANNEL_NUM )
        	memcpy( &g_SysConfig.eyeConfig[n], configBuf, configLen );
    	else nRet = -1;
    	break;
	case TYPE_SYS_CONFIG_IO_CONFIG:
    	memcpy( &g_SysConfig.ioConfig, configBuf, configLen );
    	break;    
	case TYPE_SYS_CONFIG_RF433M_PUBLIC:
    	memcpy( &g_SysConfig.rf433mPublic, configBuf, configLen );
    	break;
	case TYPE_SYS_CONFIG_UBOOT_ENV_BACKUP:
    	memcpy( &g_SysConfig.ubootEnvBackup, configBuf, configLen );
    	break;
	case TYPE_SYS_CONFIG_LED_SHOW_SETTING:
    	memcpy( &g_SysConfig.ledShowSetting, configBuf, configLen );
    	break;    
	case TYPE_SYS_CONFIG_LED_BOARD_SETTING:
    	memcpy( &g_SysConfig.ledBoardSetting, configBuf, configLen );
    	break;    
	case TYPE_SYS_CONFIG_GROUP_SETTING:
    	memcpy( &g_SysConfig.groupSetting, configBuf, configLen );
    	break;
	case TYPE_SYS_CONFIG_WL_PLATFORM:
    	memcpy( &g_SysConfig.wlPlatform, configBuf, configLen );
    	break;
	case TYPE_SYS_CONFIG_THREEG_NET_PARAM:
    	memcpy( &g_SysConfig.threegNetParam, configBuf, configLen );
    	break;
	case TYPE_SYS_CONFIG_WIFI_NETWORK:        
    	memcpy( &g_SysConfig.wifiNetwork, configBuf, configLen );
    	break;
	case TYPE_SYS_CONFIG_WIFI_CONNECT:        
    	memcpy( &g_SysConfig.wifiConnect, configBuf, configLen );
    	break;
	case TYPE_SYS_CONFIG_SYNOVATE:        
    	memcpy( &g_SysConfig.synovateSys, configBuf, configLen );
    	break;
	case TYPE_SYS_CONFIG_RS485_PARAM:
    	memcpy( &g_SysConfig.rs485Param, configBuf, configLen );
    	break;
	case TYPE_SYS_CONFIG_FTP_REC:
    	memcpy( &g_SysConfig.ftpRec, configBuf, configLen );
    	break;
	case TYPE_SYS_CONFIG_FTP_UPLOAD:
    	memset( &g_SysConfig.ftpUpload, 0, sizeof(g_SysConfig.ftpUpload));
    	memcpy( &g_SysConfig.ftpUpload, configBuf, configLen );
    	break;

	default:
    	FiPrint( "SetSysConfig Type Unknow !\r\n" );
    	nRet = -1;
    	break;
    }
	g_MutexConfig.Unlock();
	return nRet;
}

/* ------------------------------------------------------------------------- */

