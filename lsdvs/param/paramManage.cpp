/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.10.10
**  description  : 参数管理
********************************************************************************/

#include <string.h>

#include "debug.h"
#include "const.h"
#include "linuxFile.h"

#include "paramConfig.h"
#include "paramManage.h"
#include "vencParamEasy.h"

// 恢复出厂设置
void ParamSetFactoryConfigure()
{    
	SysConfigInitMemberAddr();
	SysConfigRestoreFactoryConf();
    //SaveParamConfig();
	SyncParamConfig();
}

// 恢复默认参数
void ParamSetDefaultParam()
{    
	SysConfigInitMemberAddr();
	SysConfigSetDefaultParam();
    //SaveParamConfig();
	SyncParamConfig();
}

// 系统基本参数
int ParamGetBaseInfo(PARAM_CONFIG_BASE_INFO *pParam)
{
	int ret;

	ret = GetParamConfig(INDEX_PARAM_CONFIG_BASE_INFO, pParam, sizeof(*pParam), 0);
	Strcpy( pParam->softwareVersion, DEV_SOFTWARE_VERSION );
	Strcpy( pParam->hardwareVersion, DEV_HARDWARE_VERSION );
	Strcpy( pParam->configureVersion, DEV_CONFIG_VERSION );
	Strcpy( pParam->serialNo, DEV_SERIAL_NUM );

	return ret;
}
int ParamSetBaseInfo(PARAM_CONFIG_BASE_INFO *pParam)
{
	PARAM_CONFIG_BASE_INFO param;
	int ret;

	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetBaseInfo( &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_BASE_INFO, pParam, sizeof(*pParam), 0);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

// 网络参数
int ParamGetNetwork(PARAM_CONFIG_NETWORK *pParam)
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_NETWORK, pParam, sizeof(*pParam), 0);

	return ret;
}
int ParamSetNetwork(PARAM_CONFIG_NETWORK *pParam)
{
	PARAM_CONFIG_NETWORK param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetNetwork( &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_NETWORK, pParam, sizeof(*pParam), 0);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

// 客户端或IE登录用户参数
int ParamGetClientUser(int n, PARAM_CONFIG_CLIENT_USER *pParam)
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_CLIENT_USER, pParam, sizeof(*pParam), n);

	return ret;
}
int ParamSetClientUser(int n, PARAM_CONFIG_CLIENT_USER *pParam)
{
	PARAM_CONFIG_CLIENT_USER param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetClientUser( n, &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_CLIENT_USER, pParam, sizeof(*pParam), n);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

// 视频编码公共参数
int ParamGetVideoEncodePublic(PARAM_CONFIG_VIDEO_ENCODE_PUBLIC *pParam)
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_VIDEO_ENCODE_PUBLIC, pParam, sizeof(*pParam), 0);

	return ret;
}
int ParamSetVideoEncodePublic(PARAM_CONFIG_VIDEO_ENCODE_PUBLIC *pParam)
{
	PARAM_CONFIG_VIDEO_ENCODE_PUBLIC param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetVideoEncodePublic( &param );
	if( FI_SUCCESS == ret )
    {
    	ret = FI_FAIL;
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
            //VencParamEasySetVideoStandard( pParam->videoStandard );
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_VIDEO_ENCODE_PUBLIC, pParam, sizeof(*pParam), 0);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();                
            }
        }
    }
    
	return ret;
}

// 主码流视频编码参数
int ParamGetVideoEncode(int n, PARAM_CONFIG_VIDEO_ENCODE *pParam)
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_VIDEO_ENCODE, pParam, sizeof(*pParam), n);

	return ret;
}
int ParamSetVideoEncode(int n, PARAM_CONFIG_VIDEO_ENCODE *pParam)
{
	PARAM_CONFIG_VIDEO_ENCODE param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetVideoEncode( n, &param );
	if( FI_SUCCESS == ret )
    {
    	ret = FI_FAIL;
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
            //VencParamEasySetResolution( n, pParam->resolution );
        	VencParamEasySetFramerate( n, pParam->frameRate );
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_VIDEO_ENCODE, pParam, sizeof(*pParam), n);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

//从码流视频编码参数
int ParamGetVideoEncodeSlave(int n, PARAM_CONFIG_VIDEO_ENCODE *pParam)
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_VIDEO_ENCODE_SLAVE, pParam, sizeof(*pParam), n);

	return ret;
}
int ParamSetVideoEncodeSlave(int n, PARAM_CONFIG_VIDEO_ENCODE *pParam)
{
	PARAM_CONFIG_VIDEO_ENCODE param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetVideoEncodeSlave( n, &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_VIDEO_ENCODE_SLAVE, pParam, sizeof(*pParam), n);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

// 视频采集基本参数
int ParamGetVideoBaseParam(int n, PARAM_CONFIG_VIDEO_BASE_PARAM *pParam)
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_VIDEO_BASE_PARAM, pParam, sizeof(*pParam), n);

	return ret;
}
int ParamSetVideoBaseParam(int n, PARAM_CONFIG_VIDEO_BASE_PARAM *pParam)
{
	PARAM_CONFIG_VIDEO_BASE_PARAM param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetVideoBaseParam( n, &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_VIDEO_BASE_PARAM, pParam, sizeof(*pParam), n);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

// logo OSD 参数
int ParamGetOsdLogo(int n, PARAM_CONFIG_OSD_LOGO *pParam)
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_OSD_LOGO, pParam, sizeof(*pParam), n);

	return ret;
}
int ParamSetOsdLogo(int n, PARAM_CONFIG_OSD_LOGO *pParam)
{
	PARAM_CONFIG_OSD_LOGO param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetOsdLogo( n, &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_OSD_LOGO, pParam, sizeof(*pParam), n);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

// 时间 OSD 参数
int ParamGetOsdTime(int n, PARAM_CONFIG_OSD_TIME *pParam)
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_OSD_TIME, pParam, sizeof(*pParam), n);

	return ret;
}
int ParamSetOsdTime(int n, PARAM_CONFIG_OSD_TIME *pParam)
{
	PARAM_CONFIG_OSD_TIME param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetOsdTime( n, &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_OSD_TIME, pParam, sizeof(*pParam), n);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

// 音频参数
int ParamGetAudio(PARAM_CONFIG_AUDIO_ENCODE *pParam)
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_AUDIO, pParam, sizeof(*pParam), 0);

	return ret;
}
int ParamSetAudio(PARAM_CONFIG_AUDIO_ENCODE *pParam)
{
	PARAM_CONFIG_AUDIO_ENCODE param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetAudio( &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
            #if 0
        	VencParamEasySetAudioSampleRate( pParam->sampleRate );
        	VencParamEasySetAudioBitWidth( pParam->bitWidth );
        	VencParamEasySetAudioEncodeType( pParam->encodeType );
        	VencParamEasySetAudioChMode( pParam->chMode );
            #endif
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_AUDIO, pParam, sizeof(*pParam), 0);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

// 自动维护参数
int ParamGetAutoMaintain(PARAM_CONFIG_AUTO_MAINTAIN *pParam)
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_AUTO_MAINTAIN, pParam, sizeof(*pParam), 0);

	return ret;
}
int ParamSetAutoMaintain(PARAM_CONFIG_AUTO_MAINTAIN *pParam)
{
	PARAM_CONFIG_AUTO_MAINTAIN param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetAutoMaintain( &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_AUTO_MAINTAIN, pParam, sizeof(*pParam), 0);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

// 录像公共参数
int ParamGetRecordPublic(PARAM_CONFIG_RECORD_PUBLIC *pParam)
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_RECORD_PUBLIC, pParam, sizeof(*pParam), 0);

	return ret;
}
int ParamSetRecordPublic(PARAM_CONFIG_RECORD_PUBLIC *pParam)
{
	PARAM_CONFIG_RECORD_PUBLIC param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetRecordPublic( &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_RECORD_PUBLIC, pParam, sizeof(*pParam), 0);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

// 录像通道参数
int ParamGetRecordParam(int n, PARAM_CONFIG_RECORD_PARAM *pParam)
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_RECORD_PARAM, pParam, sizeof(*pParam), n);

	return ret;
}
int ParamSetRecordParam(int n, PARAM_CONFIG_RECORD_PARAM *pParam)
{
	PARAM_CONFIG_RECORD_PARAM param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetRecordParam( n, &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_RECORD_PARAM, pParam, sizeof(*pParam), n);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

// IO报警参数设置
int ParamGetAlarmIo(PARAM_CONFIG_ALARM_IO *pParam)
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_ALARM_IO, pParam, sizeof(*pParam), 0);

	return ret;
}
int ParamSetAlarmIo(PARAM_CONFIG_ALARM_IO *pParam)
{
	PARAM_CONFIG_ALARM_IO param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetAlarmIo( &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_ALARM_IO, pParam, sizeof(*pParam), 0);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}


// 串口参数
int ParamGetSerial(PARAM_CONFIG_SERIAL *pParam)
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_SERIAL, pParam, sizeof(*pParam), 0);

	return ret;
}
int ParamSetSerial(PARAM_CONFIG_SERIAL *pParam)
{
	PARAM_CONFIG_SERIAL param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetSerial( &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_SERIAL, pParam, sizeof(*pParam), 0);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

// NTP参数
int ParamGetNtp( PARAM_CONFIG_NTP *pParam )
{
	int ret = GetParamConfig( INDEX_PARAM_CONFIG_NTP, pParam, sizeof(*pParam), 0 );

	return ret;
}
int ParamSetNtp(PARAM_CONFIG_NTP *pParam)
{
	PARAM_CONFIG_NTP param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetNtp( &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_NTP, pParam, sizeof(*pParam), 0);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

// email参数
int ParamGetEmail(PARAM_CONFIG_EMAIL *pParam)
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_EMAIL, pParam, sizeof(*pParam), 0);

	return ret;
}
int ParamSetEmail(PARAM_CONFIG_EMAIL *pParam)
{
	PARAM_CONFIG_EMAIL param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetEmail( &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_EMAIL, pParam, sizeof(*pParam), 0);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

// 移动侦测报警参数
int ParamGetAlarmMoveDetect(int n, PARAM_CONFIG_ALARM_MOVE_DETECT *pParam)
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_ALARM_MOVE_DETECT, pParam, sizeof(*pParam), n);

	return ret;
}
int ParamSetAlarmMoveDetect(int n, PARAM_CONFIG_ALARM_MOVE_DETECT *pParam)
{
	PARAM_CONFIG_ALARM_MOVE_DETECT param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetAlarmMoveDetect( n, &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_ALARM_MOVE_DETECT, pParam, sizeof(*pParam), n);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

// 视频遮挡
int ParamGetVideoOverlay(int n, PARAM_CONFIG_VIDEO_OVERLAY *pParam)
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_VIDEO_OVERLAY, pParam, sizeof(*pParam), n);

	return ret;
}
int ParamSetVideoOverlay(int n, PARAM_CONFIG_VIDEO_OVERLAY *pParam)
{
	PARAM_CONFIG_VIDEO_OVERLAY param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetVideoOverlay( n, &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_VIDEO_OVERLAY, pParam, sizeof(*pParam), n);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

// 视频丢失报警设置
int ParamGetAlarmVideoLose(int n, PARAM_CONFIG_ALARM_VIDEO_LOSE *pParam)
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_ALARM_VIDEO_LOSE, pParam, sizeof(*pParam), n);

	return ret;
}
int ParamSetAlarmVideoLose(int n, PARAM_CONFIG_ALARM_VIDEO_LOSE *pParam)
{
	PARAM_CONFIG_ALARM_VIDEO_LOSE param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetAlarmVideoLose( n, &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_ALARM_VIDEO_LOSE, pParam, sizeof(*pParam), n);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

// ftp 参数
int ParamGetFtp( PARAM_CONFIG_FTP *pParam )
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_FTP, pParam, sizeof(*pParam), 0);

	return ret;
}
int ParamSetFtp( PARAM_CONFIG_FTP *pParam )
{
	PARAM_CONFIG_FTP param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetFtp( &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_FTP, pParam, sizeof(*pParam), 0);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

// 定时抓拍
int ParamGetSnapTimer(int n, PARAM_CONFIG_SNAP_TIMER *pParam)
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_SNAP_TIMER, pParam, sizeof(*pParam), n);

	return ret;
}
int ParamSetSnapTimer(int n, PARAM_CONFIG_SNAP_TIMER *pParam)
{
	PARAM_CONFIG_SNAP_TIMER param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetSnapTimer( n, &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_SNAP_TIMER, pParam, sizeof(*pParam), n);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

// icmp 参数
int ParamGetIcmp(int n, PARAM_CONFIG_ICMP *pParam)
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_ICMP, pParam, sizeof(*pParam), n);

	return ret;
}
int ParamSetIcmp(int n, PARAM_CONFIG_ICMP *pParam)
{
	PARAM_CONFIG_ICMP param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetIcmp( n, &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_ICMP, pParam, sizeof(*pParam), n);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

// ddns 参数
int ParamGetDdns(int n, PARAM_CONFIG_DDNS *pParam)
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_DDNS, pParam, sizeof(*pParam), n);

	return ret;
}
int ParamSetDdns(int n, PARAM_CONFIG_DDNS *pParam)
{
	PARAM_CONFIG_DDNS param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetDdns( n, &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_DDNS, pParam, sizeof(*pParam), n);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

// dtu 参数
int ParamGetDtu( PARAM_CONFIG_DTU *pParam )
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_DTU, pParam, sizeof(*pParam), 0);

	return ret;
}
int ParamSetDtu( PARAM_CONFIG_DTU *pParam)
{
	PARAM_CONFIG_DTU param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetDtu( &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_DTU, pParam, sizeof(*pParam), 0);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

// wifi连接参数
int ParamGetWifiConnect( PARAM_CONFIG_WIFI_CONNECT_T *pParam )
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_WIFI_CONNECT, pParam, sizeof(*pParam), 0);

	return ret;
}
int ParamSetWifiConnect( PARAM_CONFIG_WIFI_CONNECT_T *pParam )
{
	PARAM_CONFIG_WIFI_CONNECT_T param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetWifiConnect( &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_WIFI_CONNECT, pParam, sizeof(*pParam), 0);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

int ParamGetThreeg( PARAM_CONFIG_THREEG_T *pParam )
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_THREEG, pParam, sizeof(*pParam), 0);

	return ret;
}
int ParamSetThreeg( PARAM_CONFIG_THREEG_T *pParam )
{
	PARAM_CONFIG_THREEG_T param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetThreeg( &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_THREEG, pParam, sizeof(*pParam), 0);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

int ParamGetAlarmVideoShelter( int n, PARAM_CONFIG_ALARM_VIDEO_SHELTER *pParam )
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_VIDEO_SHELTER, pParam, sizeof(*pParam), n);

	return ret;
}
int ParamSetAlarmVideoShelter( int n, PARAM_CONFIG_ALARM_VIDEO_SHELTER *pParam )
{
	PARAM_CONFIG_ALARM_VIDEO_SHELTER param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetAlarmVideoShelter( n, &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_VIDEO_SHELTER, pParam, sizeof(*pParam), n);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}


// 所有参数
int ParamGetAllParam(SYS_CONFIG *pParam)
{
	int ret = GetParamConfig(INDEX_PARAM_CONFIG_TOTAL, pParam, sizeof(*pParam), 0);

	return ret;
}
int ParamSetAllParam(SYS_CONFIG *pParam)
{
	SYS_CONFIG param;
	int ret;
    
	memset( &param, 0x00, sizeof(param) );
	ret = ParamGetAllParam( &param );
	if( FI_SUCCESS == ret )
    {
    	if( 0 != memcmp(&param, pParam, sizeof(param)) )
        {
        	ret = SetParamConfig(INDEX_PARAM_CONFIG_TOTAL, pParam, sizeof(*pParam), 0);
        	if( FI_SUCCESS == ret )
            {
            	SaveParamConfig();
            }
        }
    }
    
	return ret;
}

