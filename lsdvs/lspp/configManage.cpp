/*
*******************************************************************************
**  Copyright (c) 2013, 
**  All rights reserved.
**    
**  description  : 此文件实现了配置管理相关的函数，
**            用于其它模块获取、设置、保存系统配置。
**  date           :  2013.11.24
**
**  version       :  1.0
*******************************************************************************
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/reboot.h>
#include "const.h"
#include "debug.h"
#include "mutex.h"
#include "timer.h"
#include "strSafe.h"
#include "sysConfig.h"
#include "licence.h"
#include "configManage.h"
#include "configProtocol.h"
#include "fitLspp.h"
#include "hdd.h"

extern int FiTalkbackSetVolumeIn( int inVol );
extern int FiTalkbackSetVolumeOut( int outVol );

//
// 系统基本参数
//
int CfgManageGetDevTimeZone(void)
{
	int	        	nRet	= -1;
	int	        	value	= 100;
	SYS_CONFIG_BASE	sysBase;

	memset( &sysBase, 0x00, sizeof(sysBase) );
	if( FI_TRUE == SysGetSynovateFlag())
    {    
    	nRet = GetSysConfig( TYPE_SYS_CONFIG_BASE, &sysBase, sizeof(sysBase) );    
    	if ( nRet != -1 )
        {
        	value = sysBase.timeIntZone;
        	if ( -12 <= value && value <= 13 )
            {
            	FiPrint("get time zoom is %d\r\n",value);
            	return value;
            }
        }
    	else
        {
        	FiPrint("get time zoom id Error,give 8\r\n");
        	return 8;
        }
    }
    
	return 8;
}

int CfgManageGetDevName( char *devName )
{
	int nRet = -1;
	if ( devName != NULL )
    {
    	SYS_CONFIG_BASE	sysBase;
    	memset( &sysBase, 0x00, sizeof(sysBase) );
    	nRet = GetSysConfig( TYPE_SYS_CONFIG_BASE, (void *)&sysBase, sizeof(sysBase) );
    	if ( nRet == 0 ) strlcpy( devName, sysBase.devName, sizeof(sysBase.devName) );
    }
	return nRet;    
}

int CfgManageGetDevId( char *devId )
{
	int nRet = -1;
	if ( devId != NULL )
    {
    	SYS_CONFIG_BASE	sysBase;
    	memset( &sysBase, 0x00, sizeof(sysBase) );
    	nRet = GetSysConfig( TYPE_SYS_CONFIG_BASE, (void *)&sysBase, sizeof(sysBase) );
    	if ( nRet == 0 ) strlcpy( devId, sysBase.devId, sizeof(sysBase.devId) );
    }
	return nRet;    
}

int CfgManageSetTalkbackVoice( int voiceInLevel, int voiceOutLevel )
{
	int	        	nRet	= 0;
    
#ifdef PLATFORM_HI3512
	SYS_CONFIG_BASE	sysBase;
    
	memset( &sysBase, 0x00, sizeof(sysBase) );
    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_BASE, &sysBase, sizeof(sysBase) );
    
	if ( nRet != -1 )
    {        
    	if ( nRet == 0 && sysBase.talkVoiceInLevel != voiceInLevel )
        {
        	sysBase.talkVoiceInLevel = voiceInLevel;
        	nRet = FiTalkbackSetVolumeIn( voiceInLevel );
        }
    	if ( nRet == 0 && sysBase.talkVoiceOutLevel != voiceOutLevel )
        {
        	sysBase.talkVoiceOutLevel = voiceOutLevel;
        	nRet = FiTalkbackSetVolumeOut( voiceOutLevel );
        }
    	if ( nRet == 0 ) nRet = SetSysConfig( TYPE_SYS_CONFIG_BASE, &sysBase, sizeof(sysBase) );
//    	if ( nRet == 0 ) nRet = WriteSysConfig();        
    }
#endif
	return nRet;
}

int CfgManageGetVideoStandard( int *standard )
{
	int nRet = -1;
	SYS_CONFIG_BASE sysBase;
    
	memset( &sysBase, 0x00, sizeof(sysBase) );
	if ( standard != NULL )
    {        
    	nRet = GetSysConfig( TYPE_SYS_CONFIG_BASE, (void *)&sysBase, sizeof(sysBase) );
    	if ( nRet == 0 ) *standard = sysBase.videoStandard;
    }
	return nRet;
}

int CfgManageGetNtpParam( unsigned char *ntpType, unsigned short *ntpPort, char *ntpAddr )
{
	int	        	nRet	= -1;
	SYS_CONFIG_BASE	sysBase;
    
	memset( &sysBase, 0x00, sizeof(sysBase) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_BASE, &sysBase, sizeof(sysBase) );    
	if ( nRet != -1 )
    {
    	if ( ntpType != NULL ) *ntpType = sysBase.ntpType;
    	if ( ntpPort != NULL ) *ntpPort = SYS_CONFIG_DEFAULT_NTP_PORT;
    	if ( ntpAddr != NULL ) strlcpy( ntpAddr, sysBase.ntpAddr, sizeof(sysBase.ntpAddr) );
    }
	return nRet;
}

int CfgManageSetNtpParam( unsigned char ntpType, unsigned short ntpPort, char *ntpAddr )
{
	int	        	nRet	= -1;
	SYS_CONFIG_BASE	sysBase;
    
	memset( &sysBase, 0x00, sizeof(sysBase) );
	if ( ntpType != 0 && ntpType != 1 && ntpType != 2 ) return nRet;
	if ( ntpAddr == NULL ) return nRet;
    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_BASE, &sysBase, sizeof(sysBase) );    
	if ( nRet != -1 )
    {
    	sysBase.ntpType = ntpType;
    	sysBase.ntpPort = ntpPort;
    	strlcpy( sysBase.ntpAddr, ntpAddr, sizeof(sysBase.ntpAddr) );
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_BASE, &sysBase, sizeof(sysBase) );
//    	if ( nRet == 0 ) nRet = WriteSysConfig();
    }    
	return nRet;
}

//
// 网络参数
//
int CfgManageGetMacAddr( char *mac )
{
	int	            	nRet	    = 0;
	char	        	macAddr[24]    = { 0 };
	SYS_CONFIG_NETWORK_F	network;

    
	memset( &network, 0x00, sizeof(network) );
	if ( mac != NULL )
    {
    	if ( FiNetGetMacAddr( macAddr, sizeof(macAddr) ) == 0 )
        {
        	strlcpy( mac, macAddr, sizeof(macAddr) );
        }
    	else
        {
        	nRet = GetSysConfig( TYPE_SYS_CONFIG_NETWORK, (void *)&network, sizeof(network) );
        	if ( nRet == 0 ) strlcpy( mac, network.mac, sizeof(network.mac) );
        }
    }
	return nRet;
}



int CfgManageGetNetwork( char *ip, char *netmask, char *dns, char *dns2, char *gateway )
{
	SYS_CONFIG_NETWORK_F	network;

	memset( &network, 0x00, sizeof(network) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_NETWORK, (void *)&network, sizeof(network) );
	if ( nRet == 0 )
    {
    	if ( ip != NULL )    	strlcpy( ip, network.ip, sizeof(network.ip) );
    	if ( netmask != NULL )	strlcpy( netmask, network.netmask, sizeof(network.netmask) );
    	if ( dns != NULL )    	strlcpy( dns, network.dns, sizeof(network.dns) );
    	if ( dns2 != NULL )    	strlcpy( dns2, network.dns2, sizeof(network.dns2) );
    	if ( gateway != NULL )	strlcpy( gateway, network.gateway, sizeof(network.gateway) );
    }
	return nRet;
}

static int SetNetwork( char *ip, char *netmask, char *dns, char *dns2, char *gateway )
{
	SYS_CONFIG_NETWORK_F	network;    
	memset( &network, 0x00, sizeof(network) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_NETWORK, (void *)&network, sizeof(network) );
	if ( nRet == 0 )
    {
    	if ( ip != NULL && strncmp(network.ip, ip, sizeof(network.ip)) )
        {
        	strlcpy( network.ip, ip, sizeof(network.ip) );
        	FiPrint( "Config manage set network ip: %s.\r\n", ip );
            ++nRet;
        }
    	if ( netmask != NULL && strncmp(network.netmask, netmask, sizeof(network.netmask)) )
        {
        	strlcpy( network.netmask, netmask, sizeof(network.netmask) );
        	FiPrint( "Config manage set network netmask: %s.\r\n", netmask );
            ++nRet;
        }
    	if ( dns != NULL && strncmp( network.dns, dns, sizeof(network.dns) ) )    
        {
        	strlcpy( network.dns, dns, sizeof(network.dns) );
        	FiPrint( "Config manage set network dns: %s.\r\n", dns );
            ++nRet;
        }
    	if ( dns2 != NULL && strncmp(network.dns2, dns2, sizeof(network.dns2)) )    
        {
        	strlcpy( network.dns2, dns2, sizeof(network.dns2) );
        	FiPrint( "Config manage set network dns2: %s.\r\n", dns2 );
            ++nRet;
        }
    	if ( gateway != NULL && strncmp(network.gateway, gateway, sizeof(network.gateway)) )
        {
        	strlcpy( network.gateway, gateway, sizeof(network.gateway) );
        	FiPrint( "Config manage set network mac: %s.\r\n", gateway );
            ++nRet;
        }    
    	if ( nRet > 0 )
        {
        	nRet = SetSysConfig( TYPE_SYS_CONFIG_NETWORK, (void *)&network, sizeof(network) );
//        	if ( ret == 0 ) ret = WriteSysConfig();
//        	if ( ret != 0 ) nRet = -1;
        }
    }
	return nRet;
}

/*
*	fn	: 设置网络参数,但不重启
*	返回 : >0-有真正修改flash,设置网络参数, == 0 没有修改flash, == -1 发生错误
*/
int CfgManageSetNetwork( char *ip, char *netmask, char *dns, char *dns2, char *gateway )
{
	int nRet = SetNetwork( ip, netmask, dns, dns2, gateway );
	return nRet;
}

int CfgManageSetNetworkReboot( char *ip, char *netmask, char *dns, char *dns2, char *gateway )
{
	int nRet = SetNetwork( ip, netmask, dns, dns2, gateway );
	if ( nRet > 0 ) RebootSystem();
	return nRet >= 0 ? 0 : -1;
}

//
// 云台控制参数
//
int CfgManageGetPtzControl( int channel, SYS_CONFIG_PTZ_CONTROL *ptzControl )
{
	int nRet = -1;    
	if ( ptzControl == NULL ) return nRet;

	SYS_CONFIG_PTZ_CONTROL sysPtzControl;
	memset( &sysPtzControl, 0x00, sizeof(sysPtzControl) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_PTZ_CONTROL, &sysPtzControl,
                        	sizeof(sysPtzControl), channel );    
	if ( nRet == 0 ) *ptzControl = sysPtzControl;    
	return nRet;
}

int CfgManageSetPtzControl( int channel, SYS_CONFIG_PTZ_CONTROL *ptzControl )
{
	int nRet = -1;
	if ( ptzControl == NULL ) return nRet;

    //nRet = FiPtzSetChnPtzControlParam(channel,ptzControl); 
    //if( nRet == 0 )
    {
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_PTZ_CONTROL,
                         	ptzControl, sizeof(*ptzControl), channel );
    }
//	if ( nRet == 0 ) nRet = WriteSysConfig();
	return nRet;
}

//
// 云台控制协议 
//
int CfgManageGetPtzProtocol( char *ptzName, SYS_CONFIG_PTZ_PROTOCOL *ptzProtocol )
{
	int	        	nRet	= -1;
	SYS_CONFIG_BASE	sysBase;
	memset( &sysBase, 0x00, sizeof(sysBase) );
    
	if ( ptzName == NULL || ptzProtocol == NULL ) return nRet;
    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_BASE, &sysBase, sizeof(sysBase) );
    
	if ( nRet != -1 )
    {    
    	nRet = -1;    // 没有找到Ptz协议
    	int ptzNum = sysBase.ptzNum;
    	SYS_CONFIG_PTZ_PROTOCOL sysPtzProtocol;
    	memset( &sysPtzProtocol, 0x00, sizeof(sysPtzProtocol) );
    	for ( int i = 0; i < ptzNum; ++i )
        {
        	nRet = GetSysConfig( TYPE_SYS_CONFIG_PTZ_PROTOCOL,
                                &sysPtzProtocol, sizeof(sysPtzProtocol), i );
            
        	if ( nRet != -1 )
            {
            	FiPrint( "i=%d, ptzName(%s), sysPtzProtocol.ptzName(%s)!\r\n",
                    	i, ptzName, sysPtzProtocol.ptzName );
            	if ( !strncmp(ptzName, sysPtzProtocol.ptzName, sizeof(sysPtzProtocol.ptzName)) )
                {
                	memcpy( ptzProtocol, &sysPtzProtocol, sizeof(sysPtzProtocol) );
                	nRet = 0;
                }
            }
        }
    }
    
	return nRet;
}

int CfgManageSetPtzProtocol( SYS_CONFIG_PTZ_PROTOCOL *ptzProtocol )
{
	int	                	nRet	        = 0;
	SYS_CONFIG_BASE	    	sysBase;
	SYS_CONFIG_PTZ_PROTOCOL	sysPtzProtocol;
	memset( &sysBase, 0x00, sizeof(sysBase) );
	memset( &sysPtzProtocol, 0x00, sizeof(sysPtzProtocol) );

	if ( ptzProtocol == NULL ) return nRet;

	nRet = GetSysConfig( TYPE_SYS_CONFIG_BASE, &sysBase, sizeof(sysBase) );
	if ( nRet != -1 )
    {
    	int		ptzNum	= sysBase.ptzNum;
    	char *	ptzName	= ptzProtocol->ptzName;
    	for ( int i = DEFAULT_PTZ_PROTOCOL_NUM; i < ptzNum; ++i )
        {
        	nRet = GetSysConfig( TYPE_SYS_CONFIG_PTZ_PROTOCOL,
                                &sysPtzProtocol, sizeof(sysPtzProtocol), i );
        	if ( nRet == -1 ) break;
        	if ( !strncmp(ptzName, sysPtzProtocol.ptzName, sizeof(sysPtzProtocol.ptzName)) )
            {
            	nRet = SetSysConfig( TYPE_SYS_CONFIG_PTZ_PROTOCOL,
                                	ptzProtocol, sizeof(*ptzProtocol), i );
                //if ( nRet == 0 ) nRet = WriteSysConfig();
            	break;
            }
        }
    }
	return nRet;
}

//
// 视频参数
//
int CfgManageGetVideoParam( int channel, SYS_CONFIG_VIDEO_BASE_PARAM *video )
{
	int nRet = -1;
	if ( video != NULL )
    {
    	nRet = GetSysConfig( TYPE_SYS_CONFIG_VIDEO_BASE_PARAM,
                            (void *)video, sizeof(*video), channel );
    }
	return nRet;
}

int CfgManageSetVideoParam( int channel, SYS_CONFIG_VIDEO_BASE_PARAM *video )
{
	int nRet = -1;
	if ( video != NULL )
    {
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_VIDEO_BASE_PARAM,
                            (void *)video, sizeof(*video), channel );
        //if ( nRet != -1 ) nRet = WriteSysConfig();
    }
	return nRet;
}

int CfgManageGetBrightness( int channel, int *brightness )
{
	int	                        	nRet	= -1;
	SYS_CONFIG_VIDEO_BASE_PARAM		video;
	memset( &video, 0x00, sizeof(video) );
	if ( brightness != NULL )
    {
    	nRet = GetSysConfig( TYPE_SYS_CONFIG_VIDEO_BASE_PARAM,
                            (void *)&video, sizeof(video), channel );
    	if ( nRet == 0 ) *brightness = video.brightness;
    }
	return nRet;
}

int CfgManageSetBrightness( int channel, int brightness )
{
	SYS_CONFIG_VIDEO_BASE_PARAM		video;
	memset( &video, 0x00, sizeof(video) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_VIDEO_BASE_PARAM,
                            (void *)&video, sizeof(video), channel );
	if ( nRet == 0 )
    {
    	if ( 0 <= brightness && brightness <= 255 )
        {
        	if ( brightness != video.brightness )
            {
            	if ( nRet != -1 ) 
                {
                	video.brightness = brightness;
                	nRet = SetSysConfig( TYPE_SYS_CONFIG_VIDEO_BASE_PARAM,
                                        (void *)&video, sizeof(video), channel );
                }
//            	if ( nRet != -1 ) nRet = WriteSysConfig();
            }
        }
    	else
        {
        	nRet = -1;
        }
    }
	return nRet;
}

int CfgManageGetContrast( int channel, int *contrast )
{
	int	                        	nRet	= -1;
	SYS_CONFIG_VIDEO_BASE_PARAM		video;
	memset( &video, 0x00, sizeof(video) );
	if ( contrast != NULL )
    {
    	nRet = GetSysConfig( TYPE_SYS_CONFIG_VIDEO_BASE_PARAM,
                            (void *)&video, sizeof(video), channel );
    	if ( nRet == 0 ) *contrast = video.contrast;
    }
	return nRet;
}

int CfgManageSetContrast( int channel, int contrast )
{
	SYS_CONFIG_VIDEO_BASE_PARAM		video;
	memset( &video, 0x00, sizeof(video) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_VIDEO_BASE_PARAM,
                            (void *)&video, sizeof(video), channel );
	if ( nRet == 0 )
    {
    	if ( 0 <= contrast && contrast <= 255 )
        {
        	if ( contrast != video.contrast )
            {
            	if ( nRet != -1 ) 
                {
                	video.contrast = contrast;
                	nRet = SetSysConfig( TYPE_SYS_CONFIG_VIDEO_BASE_PARAM,
                                        (void *)&video, sizeof(video), channel );
                }
//            	if ( nRet != -1 ) nRet = WriteSysConfig();        
            }
        }
    	else
        {
        	nRet = -1;
        }
    }
	return nRet;
}

int CfgManageGetHue( int channel, int *hue )
{
	int	                        	nRet	= -1;
	SYS_CONFIG_VIDEO_BASE_PARAM		video;
	memset( &video, 0x00, sizeof(video) );
	if ( hue != NULL )
    {
    	nRet = GetSysConfig( TYPE_SYS_CONFIG_VIDEO_BASE_PARAM, 
                            (void *)&video, sizeof(video), channel );
    	if ( nRet == 0 ) *hue = video.hue;
    }
	return nRet;
}

int CfgManageSetHue( int channel, int hue )
{
	SYS_CONFIG_VIDEO_BASE_PARAM		video;
	memset( &video, 0x00, sizeof(video) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_VIDEO_BASE_PARAM,
                            (void *)&video, sizeof(video), channel );
	if ( nRet == 0 )
    {
    	if ( -128 <= hue && hue < 128 )
        {
        	if ( hue != video.hue )
            {
            	if ( nRet != -1 ) 
                {
                	video.hue = hue;
                	nRet = SetSysConfig( TYPE_SYS_CONFIG_VIDEO_BASE_PARAM,
                                        (void *)&video, sizeof(video), channel );
                }
//            	if ( nRet != -1 ) nRet = WriteSysConfig();            
            }
        }
    	else
        {
        	nRet = -1;
        }        
    }
	return nRet;
}

int CfgManageGetSaturation( int channel, int *saturation )
{
	int	                        	nRet	= -1;
	SYS_CONFIG_VIDEO_BASE_PARAM		video;
	memset( &video, 0x00, sizeof(video) );
	if ( saturation != NULL )
    {
    	nRet = GetSysConfig( TYPE_SYS_CONFIG_VIDEO_BASE_PARAM,
                            (void *)&video, sizeof(video), channel );
    	if ( nRet == 0 ) *saturation = video.saturation;
    }
	return nRet;
}

int CfgManageSetSaturation( int channel, int saturation )
{
	SYS_CONFIG_VIDEO_BASE_PARAM		video;
	memset( &video, 0x00, sizeof(video) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_VIDEO_BASE_PARAM,
                            (void *)&video, sizeof(video), channel );
	if ( nRet == 0 )
    {
    	if ( 0 <= saturation && saturation <= 255 )
        {
        	if ( saturation != video.saturation )
            {
            	if ( nRet != -1 ) 
                {
                	video.saturation = saturation;
                	nRet = SetSysConfig( TYPE_SYS_CONFIG_VIDEO_BASE_PARAM,
                                        (void *)&video, sizeof(video), channel );
                }
//            	if ( nRet != -1 ) nRet = WriteSysConfig();                
            }
        }
    	else
        {
        	nRet = -1;
        }        
    }
	return nRet;
}

int CfgManageGetExposure( int channel, int *exposure )
{
	int	                        	nRet	= -1;
	SYS_CONFIG_VIDEO_BASE_PARAM		video;
	memset( &video, 0x00, sizeof(video) );
	if ( exposure != NULL )
    {
    	nRet = GetSysConfig( TYPE_SYS_CONFIG_VIDEO_BASE_PARAM,
                            (void *)&video, sizeof(video), channel );
    	if ( nRet == 0 ) *exposure = video.exposure;
    }
	return nRet;
}

int CfgManageSetExposure( int channel, int exposure )
{
	SYS_CONFIG_VIDEO_BASE_PARAM		video;
	memset( &video, 0x00, sizeof(video) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_VIDEO_BASE_PARAM,
                            (void *)&video, sizeof(video), channel );
	if ( nRet == 0 )
    {
    	if ( 0 <= exposure && exposure <= 255 )
        {
        	if ( exposure != video.exposure )
            {
            	if ( nRet != -1 ) 
                {
                	video.exposure = exposure;
                	nRet = SetSysConfig( TYPE_SYS_CONFIG_VIDEO_BASE_PARAM,
                                        (void *)&video, sizeof(video), channel );
                }
//            	if ( nRet != -1 ) nRet = WriteSysConfig();                        
            }
        }
    	else
        {
        	nRet = -1;
        }        
    }
	return nRet;
}

int CfgManageGetOsdTime( int channel, CONFIG_OSD_TIME *osdTime )
{
	int	                	nRet	    = -1;
#ifdef PLATFORM_HI3512	
	SYS_CONFIG_OSD_INFO		osdInfo;
	memset( &osdInfo, 0x00, sizeof(osdInfo) );
	if ( osdTime == NULL ) return nRet;

	nRet = GetSysConfig( TYPE_SYS_CONFIG_OSD_INFO,
                        (void *)&osdInfo, sizeof(osdInfo), channel );
	if ( nRet == 0 )
    {
    	memcpy( (char *)osdTime, (char *)&osdInfo.osdTime, sizeof(*osdTime) );
    }
#endif
	return nRet;
}

int CfgManageSetOsdTime( int channel, CONFIG_OSD_TIME *osdTime )
{
	int	                	nRet	    = -1;
#if 0
	SYS_CONFIG_OSD_INFO		osdInfo	    = { 0 };
	if ( osdTime == NULL ) return nRet;
    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_OSD_INFO, 
                        (void *)&osdInfo, sizeof(osdInfo), channel );
	if ( nRet == 0 )
    {
    	if ( memcmp((char *)&osdInfo.osdTime, (char *)osdTime, sizeof(*osdTime)) )
        {
        	nRet = FiOsdSetTimeOsdConfig( channel, osdTime );            
        	if ( nRet == 0 )
            {
            	memcpy( (char *)&osdInfo.osdTime, (char *)osdTime, sizeof(*osdTime) );
            	nRet = SetSysConfig( TYPE_SYS_CONFIG_OSD_INFO,
                                    (void *)&osdInfo, sizeof(osdInfo), channel );
            }
//        	if ( nRet == 0 ) nRet = WriteSysConfig();
        }
    }
#endif
	return nRet;
}

//
// 视频编码
//
int CfgManageGetVideoEncodeParam( int channel, SYS_CONFIG_VIDEO_ENCODE *videoEncode, int chType )
{
	int nRet = -1;
	if ( chType != 0 && chType != 1 ) return nRet;
    
	int type = ( chType == 0 ) 
                ? TYPE_SYS_CONFIG_VIDEO_ENCODE
                : TYPE_SYS_CONFIG_VIDEO_SUB_ENCODE;
                
	nRet = GetSysConfig( type, (void *)videoEncode, sizeof(*videoEncode), channel );
	return nRet;
}

int CfgManageSetVideoEncodeParam( int channel, SYS_CONFIG_VIDEO_ENCODE *videoEncode, int chType )
{
	int nRet = -1;
	if ( chType != 0 && chType != 1 ) return nRet;
    
	int type = ( chType == 0 ) 
                ? TYPE_SYS_CONFIG_VIDEO_ENCODE
                : TYPE_SYS_CONFIG_VIDEO_SUB_ENCODE;
                
	nRet = SetSysConfig( type, (void *)videoEncode, sizeof(*videoEncode), channel );
//	if ( nRet == 0 ) nRet = WriteSysConfig();
	return nRet;
}

int CfgManageGetBitStreamRate( int channel, int chType, int *bitRate )
{
	int	                    	nRet	        = -1;
	SYS_CONFIG_VIDEO_ENCODE		videoEncode;
	memset( &videoEncode, 0x00, sizeof(videoEncode) );
	if ( chType != 0 && chType != 1 ) return nRet;
	if ( bitRate == NULL ) return nRet;
    
	int type = ( chType == 0 ) 
                ? TYPE_SYS_CONFIG_VIDEO_ENCODE
                : TYPE_SYS_CONFIG_VIDEO_SUB_ENCODE;
                
	nRet = GetSysConfig( type, (void *)&videoEncode, sizeof(videoEncode), channel );
	if ( nRet == 0 ) *bitRate = videoEncode.quality;
	return nRet;
}

int CfgManageSetBitStreamRate( int channel, int chType, int bitRate )
{
	int	                    	nRet	        = -1;
	SYS_CONFIG_VIDEO_ENCODE		videoEncode;
	memset( &videoEncode, 0x00, sizeof(videoEncode) );
#ifdef PLATFORM_HI3512	
	if ( bitRate < 32 || bitRate > 4*1000 ) return nRet;
#else
	if ( bitRate < 32 || bitRate > 1000 ) return nRet;
#endif
    
	if ( chType != 0 && chType != 1 ) return nRet;
    
	int type = ( chType == 0 ) 
                ? TYPE_SYS_CONFIG_VIDEO_ENCODE
                : TYPE_SYS_CONFIG_VIDEO_SUB_ENCODE;
                
	nRet = GetSysConfig( type, (void *)&videoEncode, sizeof(videoEncode), channel );
#ifdef PLATFORM_HI3512	
	if ( nRet == 0 ) nRet = FiEncSetBitRate( channel, chType, bitRate );
#endif	
	if ( nRet >= 0 )
    {
    	videoEncode.quality = bitRate;
    	nRet = SetSysConfig( type, (void *)&videoEncode, sizeof(videoEncode), channel );
        
    }
//	if ( nRet == 0 ) nRet = WriteSysConfig();
	return nRet;
}

int CfgManageGetFrameRate( int channel, int chType, int *frameRate )
{
	int	                    	nRet	        = -1;
	SYS_CONFIG_VIDEO_ENCODE		videoEncode;
	memset( &videoEncode, 0x00, sizeof(videoEncode) );
	if ( chType != 0 && chType != 1 ) return nRet;
	if ( frameRate == NULL ) return nRet;
    
	int type = ( chType == 0 ) 
                ? TYPE_SYS_CONFIG_VIDEO_ENCODE
                : TYPE_SYS_CONFIG_VIDEO_SUB_ENCODE;
                
	nRet = GetSysConfig( type, (void *)&videoEncode, sizeof(videoEncode), channel );
	if ( nRet == 0 ) *frameRate = videoEncode.frameRate;
	return nRet;
}

int CfgManageSetFrameRate( int channel, int chType, int frameRate )
{
	int	                    	nRet	        = -1;
	SYS_CONFIG_VIDEO_ENCODE		videoEncode;
	memset( &videoEncode, 0x00, sizeof(videoEncode) );
	if ( chType != 0 && chType != 1 ) return nRet;
    
	int type = ( chType == 0 ) 
                ? TYPE_SYS_CONFIG_VIDEO_ENCODE
                : TYPE_SYS_CONFIG_VIDEO_SUB_ENCODE;
                
	nRet = GetSysConfig( type, (void *)&videoEncode, sizeof(videoEncode), channel );
	if ( nRet == 0 )
    {
#ifdef PLATFORM_HI3512	
    	if ( videoEncode.videoStandard == HI_PAL )
        {
        	if ( frameRate < 1 || frameRate > 25 ) nRet = -1;
        }
    	else
    	if ( videoEncode.videoStandard == HI_NTSC )
        {
        	if ( frameRate < 1 || frameRate > 30 ) nRet = -1;        
        }
    	if ( nRet == 0 ) nRet = FiEncSetVideoFrameRate( channel, chType, frameRate );
#endif	
    	if ( nRet >= 0 )
        {
        	videoEncode.frameRate = frameRate;
        	nRet = SetSysConfig( type, (void *)&videoEncode, sizeof(videoEncode), channel );        
        }
//    	if ( nRet == 0 ) nRet = WriteSysConfig();
    }
	return nRet;
}

int CfgManageGetIFrameInterval( int channel, int chType, int *interval )
{
	int	                    	nRet	        = -1;
	SYS_CONFIG_VIDEO_ENCODE		videoEncode;
	memset( &videoEncode, 0x00, sizeof(videoEncode) );
	if ( chType != 0 && chType != 1 ) return nRet;
	if ( interval == NULL )	return nRet;

	int type = ( chType == 0 ) 
                ? TYPE_SYS_CONFIG_VIDEO_ENCODE
                : TYPE_SYS_CONFIG_VIDEO_SUB_ENCODE;
                
	nRet = GetSysConfig( type, (void *)&videoEncode, sizeof(videoEncode), channel );
	if ( nRet == 0 ) *interval = videoEncode.iFrameInterval;
	return nRet;
}

int CfgManageSetIFrameInterval( int channel, int chType, int interval )
{
	int	                    	nRet	        = -1;
	SYS_CONFIG_VIDEO_ENCODE		videoEncode;
	memset( &videoEncode, 0x00, sizeof(videoEncode) );
	if ( interval < 1 || interval > 150 ) return nRet;
	if ( chType != 0 && chType != 1 ) return nRet;

	int type = ( chType == 0 ) 
                ? TYPE_SYS_CONFIG_VIDEO_ENCODE
                : TYPE_SYS_CONFIG_VIDEO_SUB_ENCODE;
                
	nRet = GetSysConfig( type, (void *)&videoEncode, sizeof(videoEncode), channel );
#ifdef PLATFORM_HI3512
	if ( nRet == 0 ) nRet = FiEncSetIFrameInterval( channel, chType, interval );
#endif	
	if ( nRet >= 0 )
    {
    	videoEncode.iFrameInterval = interval;
    	nRet = SetSysConfig( type, (void *)&videoEncode, sizeof(videoEncode), channel );
    }
//	if ( nRet == 0 ) nRet = WriteSysConfig();
	return nRet;
}

//
// 录像参数
//
int CfgManageGetRecordTimerPolicy( int channel, int weekDay, CONFIG_TIMER_WEEK *recTimerPolicy )
{
	int	                    	nRet	        = -1;
	SYS_CONFIG_RECORD_PARAM		recordParam;
	memset( &recordParam, 0x00, sizeof(recordParam) );
	if ( recTimerPolicy == NULL ) return nRet;
    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_RECORD_PARAM,
                        (void *)&recordParam, sizeof(recordParam), channel );
	if ( nRet == 0 ) *recTimerPolicy = recordParam.recTimer.timerWeek[weekDay];
	return nRet;
}

int CfgManageSetRecordTimerPolicy( int channel, int weekDay, CONFIG_TIMER_WEEK *recTimerPolicy )
{
	int	                    	nRet	        = -1;
	SYS_CONFIG_RECORD_PARAM		recordParam;
	memset( &recordParam, 0x00, sizeof(recordParam) );
	if ( recTimerPolicy == NULL ) return nRet;
    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_RECORD_PARAM,
                        (void *)&recordParam, sizeof(recordParam), channel );
    //if ( nRet == 0 ) nRet = FiRecSetTimerRecordPolicy( channel, weekDay, recTimerPolicy ); // TODO
	if ( nRet == 0 )
    {
    	recordParam.recTimer.timerWeek[weekDay] = *(CONFIG_TIMER_WEEK *)recTimerPolicy;
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_RECORD_PARAM,
                            (void *)&recordParam, sizeof(recordParam), channel );    
    }
//	if ( nRet == 0 ) nRet = WriteSysConfig();
	return nRet;
}

int CfgManageGetRecordHand( int channel, CONFIG_RECORD_HAND *getVal )
{
	int	                    	nRet	        = -1;
	SYS_CONFIG_RECORD_PARAM		recordParam;
	memset( &recordParam, 0x00, sizeof(recordParam) );
	if ( getVal == NULL ) return nRet;
    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_RECORD_PARAM,
                        (void *)&recordParam, sizeof(recordParam), channel );        
	if ( nRet == 0 ) *getVal = recordParam.recHand;    

	return nRet;
}

int CfgManageSetRecordHand( int channel, CONFIG_RECORD_HAND *setVal )
{
	int	                    	nRet	        = -1;
	SYS_CONFIG_RECORD_PARAM		recordParam;
	memset( &recordParam, 0x00, sizeof(recordParam) );
	if ( setVal == NULL ) return nRet;
    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_RECORD_PARAM,
                        (void *)&recordParam, sizeof(recordParam), channel );
    //if ( nRet == 0 ) nRet = FiRecSetHandRecord( channel, setVal );
	if ( nRet == 0 )
    {
    	recordParam.recHand = *setVal;
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_RECORD_PARAM,
                            (void *)&recordParam, sizeof(recordParam), channel );    
    }
//	if ( nRet == 0 ) nRet = WriteSysConfig();
	return nRet;
}

int CfgManageGetLoopRecordFlag( int *flag )
{
	int	                    	nRet	        = -1;
	SYS_CONFIG_RECORD_PUBLIC	recordPublic;
	memset( &recordPublic, 0x00, sizeof(recordPublic) );
	if ( flag != NULL )
    {
    	nRet = GetSysConfig( TYPE_SYS_CONFIG_RECORD_PUBLIC,
                            (void *)&recordPublic, sizeof(recordPublic) );
    	if ( nRet == 0 && NULL != flag ) *flag = recordPublic.loopRecordFlag;
    }
	return nRet;
}

int CfgManageSetLoopRecordFlag( int flag )
{
	int	                    	nRet	        = -1;
	SYS_CONFIG_RECORD_PUBLIC	recordPublic;
	memset( &recordPublic, 0x00, sizeof(recordPublic) );
    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_RECORD_PUBLIC,
                        (void *)&recordPublic, sizeof(recordPublic) );
    //if ( nRet == 0 ) nRet = FiRecSetSupportLoopRecordFlag( flag );
	if ( nRet == 0 )
    {
    	recordPublic.loopRecordFlag = flag;
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_RECORD_PUBLIC,
                            (void *)&recordPublic, sizeof(recordPublic) );    
    }
//	if ( nRet == 0 ) nRet = WriteSysConfig();
	return nRet;
}

//
// 自动维护: 定时重启系统
//
CMutexLock s_AutoMaintainMutex;
static const int MAX_DIF_TIME	    = 3;
static const int DAY_SECONDS	    = (3600 * 24);
static const int WEEK_DAYS	        = 7;

typedef struct RebootTime
{
	int date;
	int sTime;
} REBOOT_TIME;

static void *AutoMaintainReboot( void *args )
{
	int cTime = -1, sTime = -1, dTime = -1;
	REBOOT_TIME *rebootTime = (REBOOT_TIME *)args;
    
	if ( rebootTime->date == 7 )
    {
    	cTime = time(NULL) % DAY_SECONDS;
    	sTime = rebootTime->sTime % DAY_SECONDS;
    }
	else
    {
    	cTime = GetWeekdaySeconds( time(NULL) );
    	sTime = GetWeekdaySeconds( rebootTime->sTime );
    }

	dTime = cTime - sTime;
	if ( 0 <= dTime && dTime <= MAX_DIF_TIME )
    {
    	sleep( MAX_DIF_TIME );
    	RebootSystem();
    }
	return NULL;
}

int CfgManageAutoMaintainReboot( unsigned char flag, unsigned int date, unsigned int second )
{
	int	                        	nRet	        = -1;
	static bool	                	timerFlag	    = false;
	static unsigned	            	timerId	        = 0;
	static REBOOT_TIME	        	rebootTime;
	bool	                    	changeFlag	    = false;
	SYS_CONFIG_AUTO_MAINTAIN_PARAM	autoMaintain;
    
	memset( &rebootTime, 0x00, sizeof(rebootTime) );
	memset( &autoMaintain, 0x00, sizeof(autoMaintain) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_AUTO_MAINTAIN_PARAM,
                        (void *)&autoMaintain, sizeof(autoMaintain) );    
	if ( nRet == 0 )
    {
    	if ( flag != autoMaintain.rebootFlag
            || date != autoMaintain.rebootDate
            || second != autoMaintain.rebootTime )
        	changeFlag = true;
    	if ( ! changeFlag ) nRet = -1;
    	s_AutoMaintainMutex.Lock();
    	if ( ! timerFlag ) nRet = 0;
    	else nRet = DeleteRTimer( timerId );            
    	s_AutoMaintainMutex.Unlock();
    }

	if ( nRet == 0 )
    {
    	time_t sTime = 0;
    	if ( 0 <= date && date <= 7 && flag == 1 )
        {
        	time_t cTime = time(NULL);
        	cTime = cTime - cTime % DAY_SECONDS;

            // 计算自动重启时间	    
        	if ( date == 7 )
            {
            	sTime = cTime + second;
            	if ( sTime < time(NULL) ) sTime += DAY_SECONDS;
            }
        	else
            {
            	struct tm gTm = { 0 };
            	if ( gmtime_r( &cTime, &gTm ) != NULL )
                {
                	int difDays = (date + WEEK_DAYS - gTm.tm_wday) % WEEK_DAYS;                
                	sTime = cTime + difDays * DAY_SECONDS + second;                        
                	if ( sTime < time(NULL) ) sTime += DAY_SECONDS * WEEK_DAYS;
                }
            }
        	s_AutoMaintainMutex.Lock();    
        	rebootTime.date = date;
        	rebootTime.sTime = sTime;
        	nRet = AddRTimer( AutoMaintainReboot, &rebootTime, 1, &timerId );                
        	if ( nRet == 0 )
            {
                //PrintGmtime( "Auto Maintain Reboot Time :", sTime );
            	timerFlag = true;
            }
        	else FiPrint( "CfgManageAutoMaintainReboot: Add Timer Error !\r\n" );
        	s_AutoMaintainMutex.Unlock();
        }
    }

	if ( nRet == 0 && changeFlag )
    {
    	autoMaintain.rebootFlag = flag;
    	autoMaintain.rebootDate = date;
    	autoMaintain.rebootTime = second;        
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_AUTO_MAINTAIN_PARAM,
                            (void *)&autoMaintain, sizeof(autoMaintain) );
//    	if ( nRet == 0 ) WriteSysConfig();
    }        

	return nRet;
}

int CfgManageAutoMaintainReboot()
{
	int	                        	nRet	        = -1;
	SYS_CONFIG_AUTO_MAINTAIN_PARAM	autoMaintain;
	memset( &autoMaintain, 0x00, sizeof(autoMaintain) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_AUTO_MAINTAIN_PARAM,
                        (void *)&autoMaintain, sizeof(autoMaintain) );    
	if ( nRet == 0 )
    {
    	unsigned char	flag	= autoMaintain.rebootFlag;
    	unsigned int	date	= autoMaintain.rebootDate;
    	unsigned int	second	= autoMaintain.rebootTime;        
    	nRet = CfgManageAutoMaintainReboot( flag, date, second );
    }
	return nRet;
}

int CfgManageSendEmail( char *message )
{
	SYS_CONFIG_EMAIL_PARAM emailParam;
	memset( &emailParam, 0x00, sizeof(emailParam) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_EMAIL_PARAM, &emailParam, sizeof(emailParam) );
	if ( nRet == -1 ) return nRet;

	int	    	activeFlag	= emailParam.activeFlag;
	const char *serverName	= emailParam.serverName;
	int	    	serverPort	= emailParam.serverPort;
	const char *userName	= emailParam.userName;
	const char *password	= emailParam.password;
	const char *senderName	= emailParam.senderName;
	const char *emailTitle	= emailParam.emailTitle;
	const char *emailMsg	= (message == NULL) ? "\0" : message;
	const char *recvAddr1	= emailParam.recvAddr1;
	const char *recvAddr2	= emailParam.recvAddr2;
	const char *recvAddr3	= emailParam.recvAddr3;

	if ( activeFlag )
    {
    	FiPrint(    "Please wait sending email ...\r\n"
                "    TO1:       %s\r\n"
                "    TO2:       %s\r\n"
                "    TO3:       %s\r\n"                
                "    FROM:      %s\r\n"
                "    TITLE:     %s\r\n"
                "    MESSAGE:   %s\r\n"
                "    USER:      %s\r\n"
                "    PASSWORD:  %s\r\n"
                "    SERVER:    %s\r\n"
                "    PORT:      %d\r\n",
            	recvAddr1, recvAddr2, recvAddr3,
            	senderName, emailTitle, emailMsg,
            	userName, password, serverName, serverPort );
        
    }
	return nRet;
}

int CfgManageGetSdExistFlag( int *flag )
{    
	return FiHddGetHddExistFlag( HDD_TYPE_MMC, flag );    
}

int CfgManageGetUsbExistFlag( int *flag )
{    
	return FiHddGetHddExistFlag( HDD_TYPE_USB, flag );    
}

int CfgManageGetMountDevFlag()
{
	int             	extSd	= 0;
	int             	extUsb	= 0;

 	CfgManageGetSdExistFlag( &extSd );
	CfgManageGetUsbExistFlag( &extUsb );

	return (extSd | extUsb);
}

int CfgManageGetDataUploadMode( int *mode )
{
	if ( mode == NULL ) return -1;
	SYS_CONFIG_DATA_UPLOAD dataUpload;
	memset( &dataUpload, 0x00, sizeof(dataUpload) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_DATA_UPLOAD, &dataUpload, sizeof(dataUpload) );
	if ( nRet != -1 ) *mode = dataUpload.mode;
	return nRet;
}

int CfgManageGetDataUploadLastTime( time_t *lastTime )
{
	if ( lastTime == NULL ) return -1;
	SYS_CONFIG_DATA_UPLOAD dataUpload;
	memset( &dataUpload, 0x00, sizeof(dataUpload) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_DATA_UPLOAD, &dataUpload, sizeof(dataUpload) );
	if ( nRet != -1 ) *lastTime = dataUpload.lastTime;
	return nRet;
}

int CfgManageSetDataUploadLastTime( time_t lastTime )
{
	SYS_CONFIG_DATA_UPLOAD dataUpload;
	memset( &dataUpload, 0x00, sizeof(dataUpload) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_DATA_UPLOAD, &dataUpload, sizeof(dataUpload) );
	if ( nRet != -1 )
    {
    	dataUpload.lastTime = lastTime;
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_DATA_UPLOAD, &dataUpload, sizeof(dataUpload) );
//    	if ( nRet != -1 ) WriteSysConfig();
    }
	return nRet;
}

int CfgManageGetDataUploadParam( SYS_CONFIG_DATA_UPLOAD *dataUpload )
{
	if ( dataUpload == NULL ) return -1;
	else return GetSysConfig( TYPE_SYS_CONFIG_DATA_UPLOAD, dataUpload, sizeof(*dataUpload) );    
}

int CfgManageGetDataUploadOK()
{
	return SysGetDataUploadFlag();
}

int CfgManageGetPcDirection( int channel, int *oriVertical, int *oriEnter )
{
	SYS_CONFIG_PC_PARAM pcParam;
	memset( &pcParam, 0x00, sizeof(pcParam) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_PC_PARAM, &pcParam, sizeof(pcParam), channel );
	if ( nRet != -1 )
    {
    	if ( oriVertical != NULL ) *oriVertical = pcParam.bOriVertical;
    	if ( oriEnter != NULL ) *oriEnter = pcParam.bOriEnter;
    }
	return nRet;
}

int CfgManageSetPcDirection( int channel, int oriVertical, int oriEnter )
{
	int	            	nRet	= -1;
	SYS_CONFIG_PC_PARAM	pcParam;
    
	memset( &pcParam, 0x00, sizeof(pcParam) );
	if ( oriVertical != 0 && oriVertical != 1 ) return nRet;
	if ( oriEnter != 0 && oriEnter != 1 ) return nRet;

	nRet = GetSysConfig( TYPE_SYS_CONFIG_PC_PARAM, &pcParam, sizeof(pcParam), channel );
	if ( nRet == -1 ) return nRet;
    
	bool bOriVerticalFlag	= false;
	bool bOriEnterFlag	    = false;
    
	if ( pcParam.bOriVertical != oriVertical )
    {
    	pcParam.bOriVertical = oriVertical;
    	bOriVerticalFlag = true;
    }
	if ( pcParam.bOriEnter != oriEnter )
    {
    	pcParam.bOriEnter = oriEnter;
    	bOriEnterFlag = true;
    }

	if ( bOriVerticalFlag || bOriEnterFlag )
    {	nRet = SetSysConfig( TYPE_SYS_CONFIG_PC_PARAM, &pcParam, sizeof(pcParam), channel );        
    	if ( nRet != -1 ) 
        {            
//        	WriteSysConfig();
        }
    }
	return nRet;
}

int CfgManageGetPcDetectArea( int channel, int *left, int *top, int *right, int *bottom )
{
	SYS_CONFIG_PC_PARAM pcParam;
	memset( &pcParam, 0x00, sizeof(pcParam) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_PC_PARAM, &pcParam, sizeof(pcParam), channel );
	if ( nRet != -1 )
    {
    	if ( left != NULL ) *left = pcParam.nRoiLeft;
    	if ( top != NULL ) *top = pcParam.nRoiTop;
    	if ( right != NULL ) *right = pcParam.nRoiRight;
    	if ( bottom != NULL ) *bottom = pcParam.nRoiBottom;
    }
	return nRet;
}

int CfgManageSetPcDetectArea( int channel, int left, int top, int right, int bottom )
{
	SYS_CONFIG_PC_PARAM pcParam;
	memset( &pcParam, 0x00, sizeof(pcParam) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_PC_PARAM, &pcParam, sizeof(pcParam), channel );
	if ( nRet != -1 )
    {
    	if ( left < 0 || pcParam.nWidth <= left ) nRet = -1;
    	if ( top < 0 || pcParam.nHeight <= top ) nRet = -1;
    	if ( right < 0 || pcParam.nWidth <= right ) nRet = -1;
    	if ( bottom < 0 || pcParam.nHeight <= bottom ) nRet = -1;
    	if ( left >= right || top >= bottom ) nRet = -1;

    	if ( nRet != -1 )
        {
        	pcParam.nRoiLeft	= left;
        	pcParam.nRoiTop	    = top;
        	pcParam.nRoiRight	= right;
        	pcParam.nRoiBottom	= bottom;
            
        	nRet = SetSysConfig( TYPE_SYS_CONFIG_PC_PARAM, &pcParam, sizeof(pcParam), channel );
        	if ( nRet != -1 )
            {
//            	WriteSysConfig();        
            }
        }
    }
	return nRet;
}

int CfgManageIsOpenPcDetect( int channel )
{
	int	                	bOpenPcDetect	= 0;
	SYS_CONFIG_PC_PARAM		pcParam;
    
	memset( &pcParam, 0x00, sizeof(pcParam) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_PC_PARAM, &pcParam, sizeof(pcParam), channel );
	if ( nRet != -1 ) bOpenPcDetect = pcParam.bOpenPcDetect;
	return bOpenPcDetect;
}

int CfgManageOpenPcDetect( int channel )
{
	SYS_CONFIG_PC_PARAM		pcParam	        = { 0 };
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_PC_PARAM, &pcParam, sizeof(pcParam), channel );
	if ( nRet != -1 )
    {
    	pcParam.bOpenPcDetect = 1;
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_PC_PARAM, &pcParam, sizeof(pcParam), channel );
//    	if ( nRet != -1 ) WriteSysConfig();        
    }
	return nRet;
}

int CfgManageClosePcDetect( int channel )
{
	SYS_CONFIG_PC_PARAM		pcParam;
	memset( &pcParam, 0x00, sizeof(pcParam) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_PC_PARAM, &pcParam, sizeof(pcParam), channel );
	if ( nRet != -1 )
    {
    	pcParam.bOpenPcDetect = 0;
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_PC_PARAM, &pcParam, sizeof(pcParam), channel );
//    	if ( nRet != -1 ) WriteSysConfig();        
    }
	return nRet;
}

int CfgManageGetPcVideoMode( int channel )
{
	int videoMode = TYPE_VIDEO_MODE_CIF;
	SYS_CONFIG_PC_PARAM pcParam;
	memset( &pcParam, 0x00, sizeof(pcParam) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_PC_PARAM, &pcParam, sizeof(pcParam), channel );
	if ( nRet != -1 ) videoMode = pcParam.videoMode;
	return videoMode;
}

int CfgManageSetPcVideoMode( int channel, int videoMode )
{
	SYS_CONFIG_PC_PARAM pcParam;
	memset( &pcParam, 0x00, sizeof(pcParam) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_PC_PARAM, &pcParam, sizeof(pcParam), channel );
	if ( nRet != -1 )
    {
    	pcParam.videoMode = videoMode;
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_PC_PARAM, &pcParam, sizeof(pcParam), channel );
//    	if ( nRet != -1 ) WriteSysConfig();
    }
	return nRet;
}

int CfgManageGetMidUserParam( SYS_CONFIG_MIDWARE midParam[] )
{
	int	            	nRet	                        = -1;
	SYS_CONFIG_MIDWARE	midConf[MAX_MIDWARE_USER_NUM];
	memset( midConf, 0x00, sizeof(midConf) );
    
	if ( midParam == NULL ) return nRet;

	for ( int i = 0; i< MAX_MIDWARE_USER_NUM; i++ )
    {
    	nRet = GetSysConfig( TYPE_SYS_CONFIG_MID_PARAM, 
                            &midConf[i], sizeof(midConf[i]), i );
    	if ( nRet == -1 ) break;    
    }

	if ( nRet != -1 )
    {
    	if ( midParam != NULL ) memcpy( midParam, midConf, sizeof(midConf) );
    	else nRet = -1;
    }
    
	return nRet;
}

int CfgManageSetMidUserParam( SYS_CONFIG_MIDWARE midParam[] )
{
	int	            	nRet	                        = -1;
	SYS_CONFIG_MIDWARE	midConf[MAX_MIDWARE_USER_NUM];
    
	memset( midConf, 0x00, sizeof(midConf) );
	if ( midParam == NULL ) return nRet;
    
	for ( int i = 0; i < MAX_MIDWARE_USER_NUM; i++ )
    {        
    	nRet = GetSysConfig( TYPE_SYS_CONFIG_MID_PARAM, 
                            &midConf[i], sizeof(midConf[i]), i );
    	if ( nRet == -1 ) break;    
    }
    
	if ( nRet != -1 )
    {
    	for ( int i = 0; i < MAX_MIDWARE_USER_NUM; i++ )
        {
        	if (0 == i)
            {
            	if (OEM_WKP == SysGetOemVersion())
                {
                	int st = SysGetSerialType();
                	if (st == SERIAL_RS232)
                    {
                    	midParam[i].transType = 0;
                    }
                	else if (st == SERIAL_RS485)
                    {        
                    	midParam[i].transType = 0;
                    }
                }
            }
            
        	if ( memcmp( &midConf[i], &midParam[i], sizeof(midConf[i])) )
            {
            	memcpy( &midConf[i], &midParam[i], sizeof(midConf[i]) );
            	nRet = SetSysConfig( TYPE_SYS_CONFIG_MID_PARAM,
                                    &midConf[i], sizeof(midConf[i]), i );
            	if ( nRet == -1 ) break;
            }
        }
    	if ( nRet != -1 )
        {
//        	WriteSysConfig();        
        }
        
    	FiPrint("###midParam[0].ip = %s, port = %d\n", midConf[0].net.ip, midConf[0].net.port);
    }
    
	return nRet;
}

int CfgManageGetPcDoorRule( int *pcType, int *doorType, int *doorRule )
{
	SYS_CONFIG_DOOR_RULE sysDoorRule;
	memset( &sysDoorRule, 0x00, sizeof(sysDoorRule) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_DOOR_RULE, &sysDoorRule, sizeof(sysDoorRule) );
	if ( nRet != -1 )
    {
    	if ( pcType != NULL ) *pcType = sysDoorRule.pcType;
    	if ( doorType != NULL ) *doorType = sysDoorRule.doorType;
    	if ( doorRule != NULL ) *doorRule = sysDoorRule.doorRule;
    }
	return nRet;
}

int CfgManageSetPcDoorRule( int pcType, int doorType, int doorRule )
{
	SYS_CONFIG_DOOR_RULE sysDoorRule;
	memset( &sysDoorRule, 0x00, sizeof(sysDoorRule) );

	int nRet = GetSysConfig( TYPE_SYS_CONFIG_DOOR_RULE, &sysDoorRule, sizeof(sysDoorRule) );
	if ( nRet != -1 )
    {
        //if ( SetDoorRule( pcType, doorType, doorRule ) )
        {
        	sysDoorRule.pcType = pcType;
        	sysDoorRule.doorType = doorType;
        	sysDoorRule.doorRule = doorRule;
        	nRet = SetSysConfig( TYPE_SYS_CONFIG_DOOR_RULE, &sysDoorRule, sizeof(sysDoorRule) );
//        	if ( nRet != -1 ) WriteSysConfig();
        }
    }
	return nRet;
}

int CfgManageSetSaveHistoryData( int bSaveHistoryData )
{
	SYS_CONFIG_PC_CONFIG sysPcConfig;
	memset( &sysPcConfig, 0x00, sizeof(sysPcConfig) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_PC_CONFIG, &sysPcConfig, sizeof(sysPcConfig) );
	if ( nRet != -1 ) 
    {
    	sysPcConfig.bSaveHistroyData = bSaveHistoryData;
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_PC_CONFIG, &sysPcConfig, sizeof(sysPcConfig) );
    	if ( nRet != -1 ) 
        {
//        	WriteSysConfig();
        }
    }
	return nRet;    
}

int CfgManageGetPcCountTime( char pTime[PC_COUNT_TIME_NUM][PC_COUNT_TIME_LEN] )
{
	SYS_CONFIG_PC_CONFIG sysPcConfig;
	memset( &sysPcConfig, 0x00, sizeof(sysPcConfig) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_PC_CONFIG, &sysPcConfig, sizeof(sysPcConfig) );
	if ( nRet != -1 ) 
    {
    	memcpy( pTime, sysPcConfig.countTime, sizeof(sysPcConfig.countTime) );        
    }
	return nRet;    
}

int CfgManageSetPcCountTime( char pTime[PC_COUNT_TIME_NUM][PC_COUNT_TIME_LEN] )
{
	SYS_CONFIG_PC_CONFIG sysPcConfig;
	memset( &sysPcConfig, 0x00, sizeof(sysPcConfig) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_PC_CONFIG, &sysPcConfig, sizeof(sysPcConfig) );
	if ( nRet != -1  ) 
    {
    	memcpy( sysPcConfig.countTime, pTime, sizeof(sysPcConfig.countTime) );
    	if( nRet == 0 ) nRet = SetSysConfig( TYPE_SYS_CONFIG_PC_CONFIG, &sysPcConfig, sizeof(sysPcConfig) );
    	if( nRet != -1 ) 
        {
//        	WriteSysConfig();
        }
    }
	return nRet;    
}

int CfgManageGetPcClsCountTime(CONFIG_CLEAR_PC_PERIOD *clsPeriod)
{
	SYS_CONFIG_PC_CONFIG sysPcConfig;
	memset( &sysPcConfig, 0x00, sizeof(sysPcConfig) );

	if (clsPeriod == NULL)
    {
    	FiPrint("NULL err\n");
    	return -1;
    }

	int nRet = GetSysConfig( TYPE_SYS_CONFIG_PC_CONFIG, &sysPcConfig, sizeof(sysPcConfig) );
	if ( nRet != -1 ) 
    {
    	memcpy( clsPeriod, sysPcConfig.clrCountTime, sizeof(sysPcConfig.clrCountTime) );        
    }
	return nRet;
}

int CfgManageSetPcClsCountTime( CONFIG_CLEAR_PC_PERIOD *clsPeriod )
{
	SYS_CONFIG_PC_CONFIG sysPcConfig;
	memset( &sysPcConfig, 0x00, sizeof(sysPcConfig) );

	if (clsPeriod == NULL)
    {
    	FiPrint("NULL err\n");
    	return -1;
    }

	int nRet = GetSysConfig( TYPE_SYS_CONFIG_PC_CONFIG, &sysPcConfig, sizeof(sysPcConfig) );
	if ( nRet != -1  ) 
    {
    	memcpy( sysPcConfig.clrCountTime, clsPeriod, sizeof(sysPcConfig.clrCountTime) );

    	FiPrint("set clear time ret = %d, cls.firs = |%s|\n", nRet, sysPcConfig.clrCountTime[0]);
    	if( nRet == 0 ) 
        {
        	nRet = SetSysConfig( TYPE_SYS_CONFIG_PC_CONFIG, &sysPcConfig, sizeof(sysPcConfig) );
        }
    	if( nRet != -1 ) 
        {
//        	WriteSysConfig();
        }
    }
	return nRet;    
}

int CfgManageGetPcCarMod(SYS_CONFIG_PC_CONFIG *carMod )
{
	SYS_CONFIG_PC_CONFIG sysPcConfig;
	memset( &sysPcConfig, 0x00, sizeof(sysPcConfig) );

	if (carMod == NULL)
    {
    	FiPrint("NULL err\n");
    	return -1;
    }

	int nRet = GetSysConfig( TYPE_SYS_CONFIG_PC_CONFIG, &sysPcConfig, sizeof(sysPcConfig) );
	if ( nRet != -1 ) 
    {
    	memcpy(carMod, &sysPcConfig, sizeof(SYS_CONFIG_PC_CONFIG));        
    }
	return nRet;
}

int CfgManageSetPcCarMod(SYS_CONFIG_PC_CONFIG *carMod )
{
	SYS_CONFIG_PC_CONFIG sysPcConfig;
	char tmp[3];
	memset( &sysPcConfig, 0x00, sizeof(sysPcConfig) );

	if (carMod == NULL)
    {
    	FiPrint("NULL err\n");
    	return -1;
    }

	int nRet = GetSysConfig( TYPE_SYS_CONFIG_PC_CONFIG, &sysPcConfig, sizeof(sysPcConfig) );
	if ( nRet != -1  ) 
    {
    	memcpy(&(sysPcConfig.upModNum), &(carMod->upModNum), sizeof(tmp));        
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_PC_CONFIG, &sysPcConfig, sizeof(sysPcConfig) );
    	if( nRet != -1 ) 
        {
//        	WriteSysConfig();
        }
    }
	return nRet;    
}

int CfgManageIsSaveHistoryData()
{
	int bSaveHistoryData = 1;
	SYS_CONFIG_PC_CONFIG sysPcConfig;
	memset( &sysPcConfig, 0x00, sizeof(sysPcConfig) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_PC_CONFIG, &sysPcConfig, sizeof(sysPcConfig) );
	if ( nRet != -1 ) bSaveHistoryData = sysPcConfig.bSaveHistroyData;
	return bSaveHistoryData;    
}

//
// 获取通道上限人数值
//
int CfgManageGetPcLimitedNum( int *limitedNumber )
{
	int nRet = -1;
	SYS_CONFIG_PC_CONFIG sysPcConfig;
	memset( &sysPcConfig, 0x00, sizeof(sysPcConfig) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_PC_CONFIG, &sysPcConfig, sizeof( sysPcConfig ) );
	if ( nRet != -1 ) 
    {
        *limitedNumber = sysPcConfig.limitedNumber;
    }
	return nRet;    
}

//
// 设置通道上限人数值
//
int CfgManageSetPcLimitedNum( int limitedNumber )
{
	int nRet = -1;
	SYS_CONFIG_PC_CONFIG sysPcConfig;
	memset( &sysPcConfig, 0x00, sizeof(sysPcConfig) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_PC_CONFIG, &sysPcConfig, sizeof( sysPcConfig ) );
	if ( nRet != -1  ) 
    {
    	sysPcConfig.limitedNumber = limitedNumber;
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_PC_CONFIG, &sysPcConfig, sizeof( sysPcConfig ) );
    	if( nRet != -1 ) 
        {
//        	WriteSysConfig();
        }
    }
	return nRet;    
}


int CfgManageGetABDoorDirection( int channel, int *oriVertical, int *oriEnter )
{
	SYS_CONFIG_ABDOOR_PARAM abDoorParam;
	memset( &abDoorParam, 0x00, sizeof(abDoorParam) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_ABDOOR_PARAM,
                            &abDoorParam, sizeof(abDoorParam), channel );
	if ( nRet != -1 )
    {
    	if ( oriVertical != NULL ) *oriVertical = abDoorParam.bOriVertical;
    	if ( oriEnter != NULL ) *oriEnter = abDoorParam.bOriEnter;
    }
	return nRet;
}

int CfgManageSetABDoorDirection( int channel, int oriVertical, int oriEnter )
{
	int	                	nRet	    = -1;
	SYS_CONFIG_ABDOOR_PARAM	abDoorParam;
    
	memset( &abDoorParam, 0x00, sizeof(abDoorParam) );
	if ( oriVertical != 0 && oriVertical != 1 ) return nRet;
	if ( oriEnter != 0 && oriEnter != 1 ) return nRet;

	nRet = GetSysConfig( TYPE_SYS_CONFIG_ABDOOR_PARAM,
                        &abDoorParam, sizeof(abDoorParam), channel );
	if ( nRet == -1 ) return nRet;
    
	bool bOriVerticalFlag	= false;
	bool bOriEnterFlag	    = false;
    
	if ( abDoorParam.bOriVertical != oriVertical )
    {
    	abDoorParam.bOriVertical = oriVertical;
    	bOriVerticalFlag = true;
    }
	if ( abDoorParam.bOriEnter != oriEnter )
    {
    	abDoorParam.bOriEnter = oriEnter;
    	bOriEnterFlag = true;
    }

	if ( bOriVerticalFlag || bOriEnterFlag )
    {	nRet = SetSysConfig( TYPE_SYS_CONFIG_ABDOOR_PARAM,
                            &abDoorParam, sizeof(abDoorParam), channel );        
    	if ( nRet != -1 ) 
        {
//        	WriteSysConfig();
        }
    }
	return nRet;
}

int CfgManageGetABDoorDetectArea( int channel, int *left, int *top, int *right, int *bottom )
{
	SYS_CONFIG_ABDOOR_PARAM abDoorParam;
	memset( &abDoorParam, 0x00, sizeof(abDoorParam) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_ABDOOR_PARAM,
                            &abDoorParam, sizeof(abDoorParam), channel );
	if ( nRet != -1 )
    {
    	if ( left != NULL ) *left = abDoorParam.nRoiLeft;
    	if ( top != NULL ) *top = abDoorParam.nRoiTop;
    	if ( right != NULL ) *right = abDoorParam.nRoiRight;
    	if ( bottom != NULL ) *bottom = abDoorParam.nRoiBottom;
    }
	return nRet;
}

int CfgManageSetABDoorDetectArea( int channel, int left, int top, int right, int bottom )
{
	SYS_CONFIG_ABDOOR_PARAM abDoorParam;
	memset( &abDoorParam, 0x00, sizeof(abDoorParam) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_ABDOOR_PARAM,
                            &abDoorParam, sizeof(abDoorParam), channel );
	if ( nRet != -1 )
    {
    	if ( left < 0 || abDoorParam.nWidth <= left ) nRet = -1;
    	if ( top < 0 || abDoorParam.nHeight <= top ) nRet = -1;
    	if ( right < 0 || abDoorParam.nWidth <= right ) nRet = -1;
    	if ( bottom < 0 || abDoorParam.nHeight <= bottom ) nRet = -1;
    	if ( left >= right || top >= bottom ) nRet = -1;

    	if ( nRet != -1 )
        {
        	abDoorParam.nRoiLeft	= left;
        	abDoorParam.nRoiTop	    = top;
        	abDoorParam.nRoiRight	= right;
        	abDoorParam.nRoiBottom	= bottom;
            
        	nRet = SetSysConfig( TYPE_SYS_CONFIG_ABDOOR_PARAM,
                                &abDoorParam, sizeof(abDoorParam), channel );
        	if ( nRet != -1 )
            {
//            	WriteSysConfig();        
            }
        }
    }
	return nRet;
}

int CfgManageIsOpenABDoorDetect( int channel )
{
	int	                    	bOpenABDoorDetect	= 0;
	SYS_CONFIG_ABDOOR_PARAM		abDoorParam;
	memset( &abDoorParam, 0x00, sizeof(abDoorParam) );
    
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_ABDOOR_PARAM,
                            &abDoorParam, sizeof(abDoorParam), channel );
	if ( nRet != -1 ) bOpenABDoorDetect = abDoorParam.bOpenABDoorDetect;
	return bOpenABDoorDetect;
}

int CfgManageOpenABDoorDetect( int channel )
{
	SYS_CONFIG_ABDOOR_PARAM		abDoorParam;
	memset( &abDoorParam, 0x00, sizeof(abDoorParam) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_ABDOOR_PARAM,
                            &abDoorParam, sizeof(abDoorParam), channel );
	if ( nRet != -1 )
    {
    	abDoorParam.bOpenABDoorDetect = 1;
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_ABDOOR_PARAM,
                            &abDoorParam, sizeof(abDoorParam), channel );
//    	if ( nRet != -1 ) WriteSysConfig();        
    }
	return nRet;
}

int CfgManageCloseABDoorDetect( int channel )
{
	SYS_CONFIG_ABDOOR_PARAM		abDoorParam;
	memset( &abDoorParam, 0x00, sizeof(abDoorParam) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_ABDOOR_PARAM,
                            &abDoorParam, sizeof(abDoorParam), channel );
	if ( nRet != -1 )
    {
    	abDoorParam.bOpenABDoorDetect = 0;
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_ABDOOR_PARAM,
                            &abDoorParam, sizeof(abDoorParam), channel );
//    	if ( nRet != -1 ) WriteSysConfig();        
    }
	return nRet;
}

int CfgManageGetABDoorSensitivity( int channel )
{
	int	                    	sensitivity	    = 6;
	SYS_CONFIG_ABDOOR_PARAM		abDoorParam;
	memset( &abDoorParam, 0x00, sizeof(abDoorParam) );
    
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_ABDOOR_PARAM,
                            &abDoorParam, sizeof(abDoorParam), channel );
	if ( nRet != -1 ) sensitivity = abDoorParam.sensitivity;
	return sensitivity;
}

int CfgManageGetABDoorStayerMax( int channel )
{
	int	                    	stayNum	        = 1;
	SYS_CONFIG_ABDOOR_PARAM		abDoorParam;
	memset( &abDoorParam, 0x00, sizeof(abDoorParam) );
    
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_ABDOOR_PARAM,
                            &abDoorParam, sizeof(abDoorParam), channel );
	if ( nRet != -1 ) stayNum = abDoorParam.stayerNum;
	return stayNum;
}

int CfgManageGetABDoorVideoMode( int channel )
{
	int videoMode = TYPE_VIDEO_MODE_CIF;
	SYS_CONFIG_ABDOOR_PARAM abdoorParam;
	memset( &abdoorParam, 0x00, sizeof(abdoorParam) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_ABDOOR_PARAM,
                            &abdoorParam, sizeof(abdoorParam), channel );
	if ( nRet != -1 ) videoMode = abdoorParam.videoMode;
	return videoMode;
}

int CfgManageSetABDoorVideoMode( int channel, int videoMode )
{
	SYS_CONFIG_ABDOOR_PARAM abdoorParam;
	memset( &abdoorParam, 0x00, sizeof(abdoorParam) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_ABDOOR_PARAM,
                            &abdoorParam, sizeof(abdoorParam), channel );
	if ( nRet != -1 )
    {
    	abdoorParam.videoMode = videoMode;
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_ABDOOR_PARAM,
                            &abdoorParam, sizeof(abdoorParam), channel );
//    	if ( nRet != -1 ) WriteSysConfig();
    }
	return nRet;
}
int CfgManageIsOpenFivDetect( int channel )
{
	int	                    	bOpenFivDetect	    = 0;
	SYS_CONFIG_FIV_PARAM		fivParam;
	memset( &fivParam, 0x00, sizeof(fivParam) );
    
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_FIV_PARAM,
                            &fivParam, sizeof(fivParam), channel );
	if ( nRet != -1 ) bOpenFivDetect = fivParam.bOpenFivDetect;
	return bOpenFivDetect;
}

int CfgManageOpenFivDetect( int channel )
{
	SYS_CONFIG_FIV_PARAM		fivParam;
	memset( &fivParam, 0x00, sizeof(fivParam) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_FIV_PARAM, &fivParam, sizeof(fivParam), channel );
	if ( nRet != -1 )
    {
    	fivParam.bOpenFivDetect = 1;
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_FIV_PARAM, &fivParam, sizeof(fivParam), channel );
//    	if ( nRet != -1 ) WriteSysConfig();        
    }
	return nRet;
}

int CfgManageCloseFivDetect( int channel )
{
	SYS_CONFIG_FIV_PARAM	fivParam;
	memset( &fivParam, 0x00, sizeof(fivParam) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_FIV_PARAM, &fivParam, sizeof(fivParam), channel );
	if ( nRet != -1 )
    {
    	fivParam.bOpenFivDetect = 0;
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_FIV_PARAM, &fivParam, sizeof(fivParam), channel );
//    	if ( nRet != -1 ) WriteSysConfig();        
    }
	return nRet;
}

int CfgManageIsOpenSmartDetect( int channel )
{
	int nRet = 0;    
        
	return nRet;
}

int CfgManageOpenSmartDetect( int channel )
{
	int nRet = 0;

	return nRet;
}

int CfgManageCloseSmartDetect( int channel )
{
	int nRet = 0;
    
	return nRet;
}

int CfgManageGetVideoMode( int channel )
{
	int videoMode = TYPE_VIDEO_MODE_CIF;
    
	return videoMode;
}

int CfgManageSetVideoMode( int channel, int videoMode )
{
	int nRet = -1;
    
	nRet = CfgManageSetPcVideoMode( channel, videoMode );
        
    
    
	return nRet;
}

int CfgManageGetRf433mConfig( SYS_CONFIG_RF433M_PARAM *rf433mParam )
{
	SYS_CONFIG_RF433M_PARAM		sysRf433mConfig;
	memset( &sysRf433mConfig, 0x00, sizeof(sysRf433mConfig) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_RF433M_CONFIG, &sysRf433mConfig, sizeof(sysRf433mConfig), 0 );
	if ( nRet != -1 )
    {
    	if( sysRf433mConfig.num > MAX_RF433M_SLAVE_NUM ) 
        {    
        	sysRf433mConfig.num = MAX_RF433M_SLAVE_NUM;
        	nRet = SetSysConfig( TYPE_SYS_CONFIG_RF433M_CONFIG, &sysRf433mConfig, sizeof(sysRf433mConfig), 0 );
//        	if ( nRet != -1 ) WriteSysConfig();    
        }
    	if ( rf433mParam != NULL ) *rf433mParam = sysRf433mConfig;
    }
    
	return nRet;
}

int CfgManageSetRf433mConfig( SYS_CONFIG_RF433M_PARAM *rf433mParam )
{
	SYS_CONFIG_RF433M_PARAM		sysRf433mConfig;
	memset( &sysRf433mConfig, 0x00, sizeof(sysRf433mConfig) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_RF433M_CONFIG, &sysRf433mConfig, sizeof(sysRf433mConfig), 0 );
	if ( nRet != -1 )
    {
    	if ( rf433mParam != NULL )
        {
        	if( memcmp( &sysRf433mConfig, rf433mParam, sizeof(sysRf433mConfig) ) != 0 ) 
            {
            	sysRf433mConfig = *rf433mParam;
            	nRet = SetSysConfig( TYPE_SYS_CONFIG_RF433M_CONFIG, &sysRf433mConfig, sizeof(sysRf433mConfig), 0 );
//            	if ( nRet != -1 ) WriteSysConfig();    
            }
        }
    }
    
	return nRet;
}

int CfgManageIsFaceDetect( int channel )
{
	int	                	bFaceDetect	    = 1;
	SYS_CONFIG_EYE_CONFIG	sysEyeConfig;
	memset( &sysEyeConfig, 0x00, sizeof(sysEyeConfig) );
    
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_EYE_CONFIG,
                        &sysEyeConfig, sizeof(sysEyeConfig), channel );
	if ( nRet == 0 ) bFaceDetect = sysEyeConfig.bFaceDetect;
	return bFaceDetect;
}

int CfgManageIsNormalOpen()
{
	int	                	bNormalOpen	    = 0;
	SYS_CONFIG_IO_CONFIG	sysIoConfig;
	memset( &sysIoConfig, 0x00, sizeof(sysIoConfig) );
    
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_IO_CONFIG,
                        &sysIoConfig, sizeof(sysIoConfig) );
	if ( nRet == 0 ) bNormalOpen = sysIoConfig.bNormalOpen;
	return bNormalOpen;    
}

int CfgManageIsSupportRf433m()
{
	int	                    	bSupportRf433m	    = 0;
	SYS_CONFIG_RF433M_PUBLIC	sysRf433mPublic;
	memset( &sysRf433mPublic, 0x00, sizeof(sysRf433mPublic) );
    
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_RF433M_PUBLIC,
                        &sysRf433mPublic, sizeof(sysRf433mPublic) );
	if ( nRet == 0 ) bSupportRf433m = sysRf433mPublic.bSupportRf433m;
	return bSupportRf433m;    
}

//
// u-boot参数擦除标识
// flag: = UBOOT_ENV_ERASE_FLAG, 则说明u-boot参数被擦除; = 其他值,则没有擦除
//
int CfgManageSetUbootEnvEraseFlag( unsigned int flag )
{
	int nRet;
	SYS_CONFIG_UBOOT_ENV_BACKUP	ubootEnvBackup;    
	memset( &ubootEnvBackup, 0x00, sizeof(ubootEnvBackup) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_UBOOT_ENV_BACKUP, &ubootEnvBackup, sizeof(ubootEnvBackup) );
	if ( nRet != -1 )
    {
    	ubootEnvBackup.flag = flag;
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_UBOOT_ENV_BACKUP, &ubootEnvBackup, sizeof(ubootEnvBackup) );
//    	if ( nRet != -1 ) WriteSysConfig();
    }
	return nRet;
}

int CfgManageGetUbootEnvEraseFlag( unsigned int *flag )
{
	int nRet;
	SYS_CONFIG_UBOOT_ENV_BACKUP	ubootEnvBackup;
	memset( &ubootEnvBackup, 0x00, sizeof(ubootEnvBackup) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_UBOOT_ENV_BACKUP, &ubootEnvBackup, sizeof(ubootEnvBackup) );
	if ( nRet != -1 )
    {
    	if(NULL != flag) *flag = ubootEnvBackup.flag;
    }

	return nRet;
}

//
// u-boot参数里的MAC地址备份
// flag: = UBOOT_ENV_ERASE_FLAG, 则说明u-boot参数被擦除; = 其他值,则没有擦除
//
int CfgManageSetUbootEnvBackupMac( char *mac )
{
	int nRet;
	SYS_CONFIG_UBOOT_ENV_BACKUP	ubootEnvBackup;
	memset( &ubootEnvBackup, 0x00, sizeof(ubootEnvBackup) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_UBOOT_ENV_BACKUP, &ubootEnvBackup, sizeof(ubootEnvBackup) );
	if ( nRet != -1 )
    {
    	if( NULL != mac ) strncpy(ubootEnvBackup.mac, mac, sizeof(ubootEnvBackup.mac) - 1);
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_UBOOT_ENV_BACKUP, &ubootEnvBackup, sizeof(ubootEnvBackup) );
//    	if ( nRet != -1 ) WriteSysConfig();
    }
	return nRet;
}

int CfgManageGetUbootEnvBackupMac( char *mac )
{
	int nRet;
	SYS_CONFIG_UBOOT_ENV_BACKUP	ubootEnvBackup;
	memset( &ubootEnvBackup, 0x00, sizeof(ubootEnvBackup) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_UBOOT_ENV_BACKUP, &ubootEnvBackup, sizeof(ubootEnvBackup) );
	if ( nRet != -1 )
    {
    	if(NULL != mac) strncpy(mac, ubootEnvBackup.mac, sizeof(ubootEnvBackup.mac) - 1);
    }

	return nRet;
}
//
// LED配置
//
int CfgManagerGetLedShowSetting( SYS_CONFIG_LEDSHOW_SETTING  *pLedShowSetting )
{
	int nRet = -1;
	SYS_CONFIG_LEDSHOW_SETTING stLedshowSetting;
	memset( &stLedshowSetting, 0x00, sizeof(stLedshowSetting) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_LED_SHOW_SETTING, &stLedshowSetting, sizeof(stLedshowSetting) );
	if ( -1 != nRet )
    {
    	if ( NULL != pLedShowSetting )
        {
        	memcpy( pLedShowSetting, &stLedshowSetting, sizeof(*pLedShowSetting) );
        }
    }

	return nRet;
}

int CfgManagerGetLedBoardSetting( SYS_CONFIG_LEDBOARD_SETTING *pLedBoardSetting )
{
	int nRet = -1;
	SYS_CONFIG_LEDBOARD_SETTING stLedboardSetting;
	memset( &stLedboardSetting, 0x00, sizeof(stLedboardSetting) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_LED_BOARD_SETTING, &stLedboardSetting, sizeof(stLedboardSetting) );
	if ( -1 != nRet )
    {
    	if ( NULL != pLedBoardSetting )
        {
        	memcpy( pLedBoardSetting, &stLedboardSetting, sizeof(*pLedBoardSetting) );
        }
    }
    
	return nRet;
}

int CfgManagerLedIsDispDatetime()
{
	int nRet = 0;
	SYS_CONFIG_LEDSHOW_SETTING stLedshowSetting;
	memset( &stLedshowSetting, 0x00, sizeof(stLedshowSetting) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_LED_SHOW_SETTING, &stLedshowSetting, sizeof(stLedshowSetting) );
	if ( -1 != nRet )
    {
    	nRet = stLedshowSetting.displaySetting & DISPLAY_DATETIME;
    }
	else
    {
    	nRet = 0;
    }

	return nRet;
}

int CfgManagerLedIsDispDayTotalIn()
{
	int nRet = 0;
	SYS_CONFIG_LEDSHOW_SETTING stLedshowSetting;
	memset( &stLedshowSetting, 0x00, sizeof(stLedshowSetting) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_LED_SHOW_SETTING, &stLedshowSetting, sizeof(stLedshowSetting) );
	if ( -1 != nRet )
    {
    	nRet = stLedshowSetting.displaySetting & DISPLAY_DAYTOTOLIN;
    }
	else
    {
    	nRet = 0;
    }
    
	return nRet;
}

int CfgManagerLedIsDispHourTotalIn()
{
	int nRet = 0;
	SYS_CONFIG_LEDSHOW_SETTING stLedshowSetting;
	memset( &stLedshowSetting, 0x00, sizeof(stLedshowSetting) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_LED_SHOW_SETTING, &stLedshowSetting, sizeof(stLedshowSetting) );
	if ( -1 != nRet )
    {
    	nRet = stLedshowSetting.displaySetting & DISPLAY_LASTHOURIN;
    }
	else
    {
    	nRet = 0;
    }
    
	return nRet;

}

int CfgManagerLedIsDispInnerCount()
{
	int nRet = 0;
	SYS_CONFIG_LEDSHOW_SETTING stLedshowSetting;
	memset( &stLedshowSetting, 0x00, sizeof(stLedshowSetting) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_LED_SHOW_SETTING, &stLedshowSetting, sizeof(stLedshowSetting) );
	if ( -1 != nRet )
    {
    	nRet = stLedshowSetting.displaySetting & DISPLAY_INNERCOUNT;
    }
	else
    {
    	nRet = 0;
    }
    
	return nRet;

}

int CfgManagerLedIsDispCustom()
{
	int nRet = 0;
	SYS_CONFIG_LEDSHOW_SETTING stLedshowSetting;
	memset( &stLedshowSetting, 0x00, sizeof(stLedshowSetting) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_LED_SHOW_SETTING, &stLedshowSetting, sizeof(stLedshowSetting) );
	if ( -1 != nRet )
    {
    	nRet = stLedshowSetting.displaySetting & DISPLAY_CUSTOM;
    }
	else
    {
    	nRet = 0;
    }
    
	return nRet;

}


//
// 组设置
//
int CfgManagerGetGroupSetting( SYS_CONFIG_GROUP_SETTING *pGroupSetting )
{    
	int nRet = -1;
	SYS_CONFIG_GROUP_SETTING stGroupSetting;
	memset( &stGroupSetting, 0x00, sizeof(stGroupSetting) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_GROUP_SETTING, &stGroupSetting, sizeof(stGroupSetting) );
	if ( -1 != nRet )
    {
    	if ( NULL != pGroupSetting )
        {
        	memcpy( pGroupSetting, &stGroupSetting, sizeof(*pGroupSetting) );
        }
    }

	return nRet;
}


int CfgManagerGetEverydayConf( int *pHour, int *pMinute )
{
	int nRet = -1;
	SYS_CONFIG_GROUP_SETTING stGroupSetting;
	memset( &stGroupSetting, 0x00, sizeof(stGroupSetting) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_GROUP_SETTING, &stGroupSetting, sizeof(stGroupSetting) );
	if ( -1 != nRet )
    {
    	if ( pHour )    *pHour = stGroupSetting.clrHour;
    	if ( pMinute )    *pMinute = stGroupSetting.clrMinute;
    }

	return nRet;
}


int CfgManagerGetGroupPeopleInfo( int *pLimitPeople, int *pAlarmPeople )
{
	int nRet = 0;
	SYS_CONFIG_GROUP_SETTING stGroupSetting;
	memset( &stGroupSetting, 0x00, sizeof(stGroupSetting) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_GROUP_SETTING, &stGroupSetting, sizeof(stGroupSetting) );
	if ( -1 !=  nRet )
    {
    	if ( pLimitPeople )        *pLimitPeople = stGroupSetting.limitPeople;
    	if ( pAlarmPeople )        *pAlarmPeople = stGroupSetting.alarmPeople;
    }
    
	return nRet;
}


int CfgManagerIsSupportGroup()
{
	int nRet = 0;
	SYS_CONFIG_GROUP_SETTING stGroupSetting;
	memset( &stGroupSetting, 0x00, sizeof(stGroupSetting) );
    
	if ( -1 != GetSysConfig(TYPE_SYS_CONFIG_GROUP_SETTING, &stGroupSetting, sizeof(stGroupSetting)) )
    {
    	nRet = stGroupSetting.enable;
    }
    
	return nRet;
}

int CfgManagerIsMasterDev()
{
	int nRet = 0;
	SYS_CONFIG_GROUP_SETTING stGroupSetting;
	memset( &stGroupSetting, 0x00, sizeof(stGroupSetting) );
    
	if ( -1 != GetSysConfig(TYPE_SYS_CONFIG_GROUP_SETTING, &stGroupSetting, sizeof(stGroupSetting)) )
    {
    	nRet = ( stGroupSetting.masterTag!=0 ? 1: 0 );
    }
    
	return nRet;
}

int	CfgManagerGetMasterServerIp( char *pIP )
{
	int nRet = -1;
	SYS_CONFIG_GROUP_SETTING stGroupSetting;
	memset( &stGroupSetting, 0x00, sizeof(stGroupSetting) );
    
	if ( -1 != GetSysConfig(TYPE_SYS_CONFIG_GROUP_SETTING, &stGroupSetting, sizeof(stGroupSetting)) )
    {
    	if ( pIP && stGroupSetting.enable && !stGroupSetting.masterTag) 
        {
        	strncpy( pIP, (const char*)stGroupSetting.ip, sizeof(stGroupSetting.ip)-1 );
        	nRet = 0;
        }
    }
    
	return nRet;
}

int CfgManagerGetSyncInterval( int *pSyncInterval )
{
	int nRet = -1;
	SYS_CONFIG_GROUP_SETTING stGroupSetting;
	memset( &stGroupSetting, 0x00, sizeof(stGroupSetting) );
    
	if ( -1 != GetSysConfig(TYPE_SYS_CONFIG_GROUP_SETTING, &stGroupSetting, sizeof(stGroupSetting)) )
    {
    	if ( pSyncInterval /*&& stGroupSetting.enable && !stGroupSetting.masterTag*/ ) 
        {
            *pSyncInterval = stGroupSetting.syncInterval;
        	nRet = 0;
        }
    }
    
	return nRet;
}

int CfgManageGetWlPlateform( char *pIpAddr, unsigned short *pPort )
{
	int nRet = -1;
	SYS_CONFIG_WL_PLATFORM wlPlatform;
	memset( &wlPlatform, 0x00, sizeof(wlPlatform) );

	if ( -1 != GetSysConfig(TYPE_SYS_CONFIG_WL_PLATFORM, &wlPlatform, sizeof(wlPlatform)) )
    {
    	if ( NULL != pIpAddr && NULL != pPort ) 
        {
        	strcpy( pIpAddr, wlPlatform.ipAddr );
            *pPort = wlPlatform.port;
        	nRet = 0;
        }
    }
    
	return nRet;
}



// 
// 获取3G使能标志
//
int CfgManageGetThreegEnableFlag()
{
	int	                    	threegEnableFlag	= 0;
	SYS_CONFIG_THREEG_NET_PARAM sysThreegNetParam;
	memset( &sysThreegNetParam, 0x00, sizeof(sysThreegNetParam) );
    
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_THREEG_NET_PARAM,
                        &sysThreegNetParam, sizeof(sysThreegNetParam) );
	if ( nRet == 0 ) threegEnableFlag = sysThreegNetParam.enableFlag;
	return threegEnableFlag;    
}

//
// 获取百世龙疲劳检测灵敏度
//
char CfgManageGetBslEyeSensitivity( int channel )
{
	char bslSensitivity = 3;
	SYS_CONFIG_EYE_CONFIG sysEyeConfig;
	memset( &sysEyeConfig, 0x00, sizeof(sysEyeConfig) );

	int nRet = GetSysConfig( TYPE_SYS_CONFIG_EYE_CONFIG,&sysEyeConfig, 
                                        	sizeof(sysEyeConfig) , channel );
	if( 0 == nRet ) bslSensitivity = sysEyeConfig.bslSensitivity;

	return bslSensitivity;
}

//
// 设置百世龙检测灵敏度
//
int CfgManageSetBslEyeSensitivity( int channel, char bslSensitivity )
{
	SYS_CONFIG_EYE_CONFIG sysEyeConfig;
	memset( &sysEyeConfig, 0x00, sizeof(sysEyeConfig) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_EYE_CONFIG, &sysEyeConfig, sizeof(sysEyeConfig), channel );
	if ( nRet != -1 )
    {
    	if ( sysEyeConfig.bslSensitivity != bslSensitivity )
        {        
        	sysEyeConfig.bslSensitivity = bslSensitivity;
        	nRet = SetSysConfig( TYPE_SYS_CONFIG_EYE_CONFIG, &sysEyeConfig, sizeof(sysEyeConfig), channel );
//        	if ( nRet != -1 ) WriteSysConfig();                
        }
    }
    
	return nRet;
}

//
// 获取短信认证密码
//
int CfgManageGetSmsAuthPasswd(char *smsAuthPasswd)
{
	SYS_CONFIG_THREEG_NET_PARAM sysThreegNetParam;
    
	memset( &sysThreegNetParam, 0x00, sizeof(sysThreegNetParam) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_THREEG_NET_PARAM,
                        &sysThreegNetParam, sizeof(sysThreegNetParam) );
	if ( nRet == 0 ) 
    {
    	memcpy(smsAuthPasswd, sysThreegNetParam.smsAuthPasswd, sizeof(sysThreegNetParam.smsAuthPasswd));
    }

	return 0;    
}

//
// 设置短信认证密码
//
int CfgManageSetSmsAuthPasswd(char *smsAuthPasswd)
{
	int nRet = -1;
	SYS_CONFIG_THREEG_NET_PARAM sysThreegNetParam;
	memset( &sysThreegNetParam, 0x00, sizeof(sysThreegNetParam) );
    
	nRet = GetSysConfig( TYPE_SYS_CONFIG_THREEG_NET_PARAM
                        , &sysThreegNetParam, sizeof(sysThreegNetParam) );
	if ( nRet != -1 )
    {
    	memcpy(sysThreegNetParam.smsAuthPasswd, smsAuthPasswd, sizeof(sysThreegNetParam.smsAuthPasswd));
    	nRet = SetSysConfig( TYPE_SYS_CONFIG_THREEG_NET_PARAM
                                , &sysThreegNetParam, sizeof(sysThreegNetParam) );            
//    	if ( nRet != -1 ) nRet = WriteSysConfig();
    }
    
	return nRet;
}

//
// 获取wifi连接配置
//
int CfgManageGetWifiConnectParam( char *pEssid, char *pPasswd )
{
	int ret;
	SYS_CONFIG_WIFI_CONNECT wifiConnect;
	memset( &wifiConnect, 0x00, sizeof(wifiConnect) );

	if( NULL == pEssid || NULL == pPasswd )
    {
    	FiPrint( "error:NULL == pEssid || NULL == pPasswd!\r\n" ); 
    	return -1;
    }
	ret = GetSysConfig( TYPE_SYS_CONFIG_WIFI_CONNECT,
                        &wifiConnect, sizeof(wifiConnect) );
	memcpy( pEssid, wifiConnect.essid, sizeof(wifiConnect.essid) );
	memcpy( pPasswd, wifiConnect.key, sizeof(wifiConnect.key) );

	return ret;
}

// 
// 获取wifi网卡的连接类型
// pNetType: out, 0-dhcp; 1-静态IP; 2-拨号
// 
int CfgManageGetWifiNetType( unsigned char *pNetType )
{
	int ret;
	SYS_CONFIG_WIFI_NETWORK sysWifiNetwork;
	memset( &sysWifiNetwork, 0x00, sizeof(sysWifiNetwork) );
	if( NULL == pNetType )
    {
    	FiPrint( "error:NULL == pNetType!\r\n" ); 
    	return -1;
    }

	ret = GetSysConfig( TYPE_SYS_CONFIG_WIFI_NETWORK,
                        &sysWifiNetwork, sizeof(sysWifiNetwork) );
	if( 0 == ret )
    {
        *pNetType = sysWifiNetwork.netType;
    }

	return ret;
}

int CfgManageGetNetworkWifi( char *ip, char *netmask, char *gateway, char *useWifiGateway )
{
	SYS_CONFIG_WIFI_NETWORK	wifiNetwork;
	memset( &wifiNetwork, 0x00, sizeof(wifiNetwork) );
	int nRet = GetSysConfig( TYPE_SYS_CONFIG_WIFI_NETWORK, (void *)&wifiNetwork, sizeof(wifiNetwork) );
	if ( nRet == 0 )
    {
    	if( ip != NULL )    	strlcpy( ip, wifiNetwork.ip, sizeof(wifiNetwork.ip) );
    	if( netmask != NULL )	strlcpy( netmask, wifiNetwork.netmask, sizeof(wifiNetwork.netmask) );
    	if( gateway != NULL )	strlcpy( gateway, wifiNetwork.gateway, sizeof(wifiNetwork.gateway) );
    	if( useWifiGateway != NULL ) *useWifiGateway = wifiNetwork.useWifiGateway;
    }
	return nRet;
}


/*synovate_add*/
int CfgManageGetsynovateIpPort( char *pIp, unsigned short *pPort)
{
	int	                        	nRet	        = -1;
	SYS_CONFIG_SYNOVATE	sysSynovate;
	memset( &sysSynovate, 0x00, sizeof(sysSynovate) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_SYNOVATE, (void *)&sysSynovate, sizeof(sysSynovate) );
	if ( nRet == 0 )
    {
    	if ( pIp != NULL )    	strlcpy( pIp, sysSynovate.synovateIp, sizeof(sysSynovate.synovateIp) );
    	if ( pPort != NULL )    (*pPort) = sysSynovate.synovatePort;
    }
	else
    {
    	FiPrint("Cfg Manage Get synovate Ip Port error\r\n");
    }
	return nRet;
}

/*synovate_add*/
int CfgManageAutoMaintainDaylight(char *pFDaylight,char *pStart,char *pEnd)
{
	int i;
	int	                        	nRet	        = -1;
	char *pkeepStart = NULL;
	char *pkeepEnd = NULL;
	SYS_CONFIG_SYNOVATE	sysSynovate;
	memset( &sysSynovate, 0x00, sizeof(sysSynovate) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_SYNOVATE, (void *)&sysSynovate, sizeof(sysSynovate) );    
	pkeepStart = pStart;
	pkeepEnd   = pEnd;
	if ( nRet == 0 )
    {
    	if(NULL!=pFDaylight)
        {
            (*pFDaylight) = sysSynovate.fdaylight;
        }
        
    	for(i=0;i<6;i++)
        {
        	if(NULL!=pkeepStart)
            {
            	pStart[i] = sysSynovate.start[i]; 
            }
        	if(NULL!=pkeepEnd)
            {
            	pEnd[i] = sysSynovate.end[i];
            }
        }
        
    }
	return nRet;
}

int CfgManageAutoDaylight(char *pftypedaylight,
                          int *pstartMonth,int *pstartWhichDis,int *pstartWhichDay,int *pstartHour,
                          int *pendMonth,int *pendWhichDis,int *pendWhichDay,int *pendHour)
{
    //int i;
	int	                        	nRet	        = -1;
	SYS_CONFIG_SYNOVATE	sysSynovate;
	memset( &sysSynovate, 0x00, sizeof(sysSynovate) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_SYNOVATE, (void *)&sysSynovate, sizeof(sysSynovate) );    
	if ( nRet == 0 )
    {
    	if(NULL!=pftypedaylight) {    (*pftypedaylight) = sysSynovate.ftypedaylight;    }
        
    	if(NULL!=pstartMonth)    {  (*pstartMonth)    = sysSynovate.startMonth;        }
    	if(NULL!=pstartWhichDis) {    (*pstartWhichDis)    = sysSynovate.startWhichDis;}
    	if(NULL!=pstartWhichDay) {    (*pstartWhichDay)    = sysSynovate.startWhichDay;}
    	if(NULL!=pstartHour)     {    (*pstartHour)        = sysSynovate.startHour;     }

    	if(NULL!=pendMonth)       {    (*pendMonth)         = sysSynovate.endMonth;    }
    	if(NULL!=pendWhichDis)    {    (*pendWhichDis)      = sysSynovate.endWhichDis; }
    	if(NULL!=pendWhichDay)    {    (*pendWhichDay)      = sysSynovate.endWhichDay; }
    	if(NULL!=pendHour)          {    (*pendHour)          = sysSynovate.endHour;     }
        
        
    }
	return nRet;
}

int CfgManageSaveDaylight(int *psetdaylight)
{
    //int i;
	int	                        	nRet	        = -1;
	SYS_CONFIG_SYNOVATE	sysSynovate;
	memset( &sysSynovate, 0x00, sizeof(sysSynovate) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_SYNOVATE, (void *)&sysSynovate, sizeof(sysSynovate) );    
	if ( nRet == 0 )
    {
    	if(NULL!=psetdaylight) {    (*psetdaylight) = sysSynovate.setdaylight;    }
    
    }
	return nRet;
}

int GetFtpRecFlag()
{
	SYS_CONFIG_FTP_REC	sysFtpRec;    
	memset( &sysFtpRec, 0x00, sizeof(sysFtpRec) );
    
	int nRet = GetSysConfig(TYPE_SYS_CONFIG_FTP_REC, &sysFtpRec, sizeof(sysFtpRec));
	if ( nRet != -1 ) 
    {
    	return sysFtpRec.recFlag;
    }

	return -1;
}

int GetFtpNetParam(SYS_CONFIG_FTP_REC *sysFtpRec)
{
	return GetSysConfig(TYPE_SYS_CONFIG_FTP_REC, sysFtpRec, sizeof(SYS_CONFIG_FTP_REC));    
}

/* 获取Ftp上传客流信息配置项 */
int CfgManageGetFtpUploadConfig( SYS_CONFIG_FTP_UPLOAD *sysFtpUpload )
{
	return GetSysConfig(TYPE_SYS_CONFIG_FTP_UPLOAD, sysFtpUpload, sizeof(SYS_CONFIG_FTP_UPLOAD));    
}

/* 设置Ftp上传客流信息配置项 */
int CfgManageSetFtpUploadConfig( SYS_CONFIG_FTP_UPLOAD *sysFtpUpload )
{
	SYS_CONFIG_FTP_UPLOAD PreFtpUploadCfg;

	memset( &PreFtpUploadCfg, 0x00, sizeof(PreFtpUploadCfg) );

	int nRet = GetSysConfig( TYPE_SYS_CONFIG_FTP_UPLOAD, &PreFtpUploadCfg, sizeof(SYS_CONFIG_FTP_UPLOAD), 0 );
	if (-1 != nRet)
    {
    	if ( sysFtpUpload != NULL )
        {
        	if( memcmp( &PreFtpUploadCfg, sysFtpUpload, sizeof(PreFtpUploadCfg) ) != 0 ) 
            {
            	PreFtpUploadCfg = *sysFtpUpload;
            	int nRet = SetSysConfig( TYPE_SYS_CONFIG_FTP_UPLOAD, &PreFtpUploadCfg, sizeof(PreFtpUploadCfg), 0 );
            	if (-1 != nRet)
                {
                    /* 写Flash */
//                	WriteSysConfig();
                }
            }
        }
    }
	return nRet;

}

/* 获得店铺号 */
int CfgManageGetShopID( int &nShopID )
{
	int nRet = -1;

	SYS_CONFIG_BASE	sysBase;
    
	memset( &sysBase, 0x00, sizeof(sysBase) );

	nRet = GetSysConfig( TYPE_SYS_CONFIG_BASE, &sysBase, sizeof(sysBase) ); 
	if ( -1 != nRet )
    {
    	nShopID = sysBase.shopID;
    	if ( 0 > nShopID )
        {
        	nShopID = 0;
        }
    	nRet = 0;
    }

	return nRet;
}

/*
* fn: 获取设备序列号
*/
int CfgManageGetSerialNum( char *pSerialNum )
{
	int nRet = -1;

	SYS_CONFIG_HEADER sysHeader;

	memset( &sysHeader, 0x00, sizeof(sysHeader) );
	nRet = GetSysConfig( TYPE_SYS_CONFIG_HEADER, &sysHeader, sizeof(sysHeader) ); 
	if ( -1 != nRet )
    {
    	if( NULL != pSerialNum )
        {
        	strcpy( pSerialNum, sysHeader.serialNo );
        }
    	else
        {
        	nRet = -1;
        }
    }

	return nRet;
}



