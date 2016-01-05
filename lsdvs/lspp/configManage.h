/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此头文件提供了配置管理相关的函数的接口。
**  date           :  2013.11.24
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#ifndef _CONFIG_MANAGE_H
#define _CONFIG_MANAGE_H

#include "sysConfig.h"
#include "configProtocol.h"


#ifdef __cplusplus
extern "C" {
#endif

//
// 系统基本参数
//
int CfgManageGetDevName( char *devName );
int CfgManageGetDevId( char *devId );
int CfgManageSetTalkbackVoice( int voiceInLevel, int voiceOutLevel );
int CfgManageGetVideoStandard( int *standard );
int CfgManageGetNtpParam( unsigned char *ntpType, unsigned short *ntpPort, char *ntpAddr );
int CfgManageSetNtpParam( unsigned char ntpType, unsigned short ntpPort, char *ntpAddr );

//
// 网络参数
//
int CfgManageGetMacAddr( char *mac );
int CfgManageGetNetwork( char *ip, char *netmask, char *dns, char *dns2, char *gateway );
int CfgManageSetNetwork( char *ip, char *netmask, char *dns, char *dns2, char *gateway );
int CfgManageSetNetworkReboot( char *ip, char *netmask, char *dns, char *dns2, char *gateway );

//
// 云台控制参数
//
int CfgManageGetPtzControl( int channel, SYS_CONFIG_PTZ_CONTROL *ptzControl );
int CfgManageSetPtzControl( int channel, SYS_CONFIG_PTZ_CONTROL *ptzControl );

//
// 云台控制协议 
//
int CfgManageGetPtzProtocol( char *ptzName, SYS_CONFIG_PTZ_PROTOCOL *ptzProtocol );
int CfgManageSetPtzProtocol( SYS_CONFIG_PTZ_PROTOCOL *ptzProtocol );

//
// 视频参数
//
int CfgManageGetVideoParam( int channel, SYS_CONFIG_VIDEO_BASE_PARAM *video );
int CfgManageSetVideoParam( int channel, SYS_CONFIG_VIDEO_BASE_PARAM *video );

int CfgManageGetBrightness( int channel, int *brightness );
int CfgManageSetBrightness( int channel, int brightness );
int CfgManageGetContrast( int channel, int *contrast );
int CfgManageSetContrast( int channel, int contrast );
int CfgManageGetHue( int channel, int *hue );
int CfgManageSetHue( int channel, int hue );
int CfgManageGetSaturation( int channel, int *saturation );
int CfgManageSetSaturation( int channel, int saturation );
int CfgManageGetExposure( int channel, int *exposure );
int CfgManageSetExposure( int channel, int exposure );

//
// OSD参数
//
int CfgManageGetOsdLogo( int channel, CONFIG_OSD_LOGO *osdLogo );
int CfgManageSetOsdLogo( int channel, CONFIG_OSD_LOGO *osdLogo );
int CfgManageGetOsdTime( int channel, CONFIG_OSD_TIME *osdTime );
int CfgManageSetOsdTime( int channel, CONFIG_OSD_TIME *osdTime );

//
// 视频编码
//
int CfgManageGetVideoEncodeParam( int channel, SYS_CONFIG_VIDEO_ENCODE *videoEncode, int chType = 0 );
int CfgManageSetVideoEncodeParam( int channel, SYS_CONFIG_VIDEO_ENCODE *videoEncode, int chType = 0 );

int CfgManageGetBitStreamRate( int channel, int chType, int *bitRate );
int CfgManageSetBitStreamRate( int channel, int chType, int bitRate );
int CfgManageGetFrameRate( int channel, int chType, int *frameRate );
int CfgManageSetFrameRate( int channel, int chType, int frameRate );
int CfgManageGetIFrameInterval( int channel, int chType, int *interval );
int CfgManageSetIFrameInterval( int channel, int chType, int interval );

//
// 录像参数
//
int CfgManageGetRecordTimerPolicy( int channel, int weekDay, CONFIG_TIMER_WEEK *recTimerPolicy );
int CfgManageSetRecordTimerPolicy( int channel, int weekDay, CONFIG_TIMER_WEEK *recTimerPolicy );
int CfgManageGetRecordHand( int channel, CONFIG_RECORD_HAND *getVal );
int CfgManageSetRecordHand( int channel, CONFIG_RECORD_HAND *setVal );
int CfgManageGetLoopRecordFlag( int *flag );
int CfgManageSetLoopRecordFlag( int flag );

#ifdef __cplusplus
}
#endif

//
// 自动维护
//
int CfgManageAutoMaintainReboot( unsigned char flag, unsigned int date, unsigned int time );
int CfgManageAutoMaintainReboot();

//
// 发送Email
//
int CfgManageSendEmail( char *message );
int CfgManageGetSdExistFlag( int *flag );
int CfgManageGetUsbExistFlag( int *flag );

//
// 数据上传协议配置
//
int CfgManageGetDataUploadMode( int *mode );
int CfgManageGetDataUploadLastTime( time_t *lastTime );
int CfgManageSetDataUploadLastTime( time_t lastTime );
int CfgManageGetDataUploadParam( SYS_CONFIG_DATA_UPLOAD *dataUpload );
int CfgManageGetDataUploadOK();

//
// PC检测参数
//
int CfgManageGetPcDirection( int channel, int *oriVertical, int *oriEnter );
int CfgManageSetPcDirection( int channel, int oriVertical, int oriEnter );
int CfgManageGetPcDetectArea( int channel, int *left, int *top, int *right, int *bottom );
int CfgManageSetPcDetectArea( int channel, int left, int top, int right, int bottom );
int CfgManageIsOpenPcDetect( int channel );
int CfgManageOpenPcDetect( int channel );
int CfgManageClosePcDetect( int channel );
int CfgManageGetVideoMode( int channel );
int CfgManageSetVideoMode( int channel, int videoMode );

//
//中间件参数
//
int CfgManageGetMidUserParam( SYS_CONFIG_MIDWARE midParam[] );
int CfgManageSetMidUserParam( SYS_CONFIG_MIDWARE midParam[] );

//
// PC门规则参数
//
int CfgManageGetPcDoorRule( int *pcType, int *doorType, int *doorRule );
int CfgManageSetPcDoorRule( int pcType, int doorType, int doorRule );

//
// PC配置
//
int CfgManageSetSaveHistoryData( int bSaveHistoryData );
int CfgManageIsSaveHistoryData();

//
// AB门检测
//
int CfgManageGetABDoorDirection( int channel, int *oriVertical, int *oriEnter );
int CfgManageSetABDoorDirection( int channel, int oriVertical, int oriEnter );
int CfgManageGetABDoorDetectArea( int channel, int *left, int *top, int *right, int *bottom );
int CfgManageSetABDoorDetectArea( int channel, int left, int top, int right, int bottom );
int CfgManageIsOpenABDoorDetect( int channel );
int CfgManageOpenABDoorDetect( int channel );
int CfgManageCloseABDoorDetect( int channel );
int CfgManageGetABDoorSensitivity( int channel );
int CfgManageGetABDoorStayerMax( int channel );

//
// FIV运动目标检测
//
int CfgManageOpenFivDetect( int channel );
int CfgManageCloseFivDetect( int channel );

//
// 智能算法检测
//
int CfgManageIsOpenSmartDetect( int channel );
int CfgManageOpenSmartDetect( int channel );
int CfgManageCloseSmartDetect( int channel );

// 
// RF433M配置
//
int CfgManageGetRf433mConfig( SYS_CONFIG_RF433M_PARAM *rf433mParam );
int CfgManageSetRf433mConfig( SYS_CONFIG_RF433M_PARAM *rf433mParam );
int CfgManageIsSupportRf433m();    //在licence有RF433M的版本下,可以在DVS客户端选择是否支持RF433M报警

//
// u-boot参数擦除标识
// flag: = UBOOT_ENV_ERASE_FLAG, 则说明u-boot参数被擦除; = 其他值,则没有擦除
//
int CfgManageSetUbootEnvEraseFlag( unsigned int flag );
int CfgManageGetUbootEnvEraseFlag( unsigned int *flag );

//
// u-boot参数里的MAC地址备份
// flag: = UBOOT_ENV_ERASE_FLAG, 则说明u-boot参数被擦除; = 其他值,则没有擦除
//
int CfgManageSetUbootEnvBackupMac( char *mac );
int CfgManageGetUbootEnvBackupMac( char *mac );
//
// LED配置
//
int CfgManagerGetLedShowSetting( SYS_CONFIG_LEDSHOW_SETTING  *pLedShowSetting );
int CfgManagerGetLedBoardSetting( SYS_CONFIG_LEDBOARD_SETTING *pLedBoardSetting );
int CfgManagerLedIsDispDatetime();
int CfgManagerLedIsDispDayTotalIn();
int CfgManagerLedIsDispHourTotalIn();
int CfgManagerLedIsDispInnerCount();
int CfgManagerLedIsDispCustom();

//
// 组设置
//
int CfgManagerGetGroupSetting( SYS_CONFIG_GROUP_SETTING *pGroupSetting );
int CfgManagerGetEverydayConf( int *pHour, int *pMinute );
int CfgManagerGetGroupPeopleInfo( int *pLimitPeople, int *pAlarmPeople );
int CfgManagerIsSupportGroup();    // 不支持组网的话不用发送人数消息
int CfgManagerIsMasterDev();    // 是否主设备
int	CfgManagerGetMasterServerIp( char *pIP ); // 获取主设备IP
int CfgManagerGetSyncInterval( int *pSyncInterval ); // 同步间隔

// 
// 获取乌鲁木齐配置
//
int CfgManageGetWlPlateform( char *pIpAddr, unsigned short *pPort );

//
// 使能3G拨号上网
//
int CfgManageGetThreegEnableFlag();
int CfgManageGetSmsAuthPasswd(char *smsAuthPasswd);
int CfgManageSetSmsAuthPasswd(char *smsAuthPasswd);

//
// 百世龙疲劳检测灵敏度
//
char CfgManageGetBslEyeSensitivity( int channel );
int CfgManageSetBslEyeSensitivity( int channel, char bslSensitivity );

//
// pc数人时间段
//
int CfgManageSetPcCountTime( char pTime[2][12] );
int CfgManageGetPcCountTime( char pTime[2][12] );
int CfgManageGetPcCarMod( SYS_CONFIG_PC_CONFIG *carMod );
int CfgManageSetPcCarMod( SYS_CONFIG_PC_CONFIG *carMod );
int CfgManageGetPcLimitedNum( int *limitedNumber );
int CfgManageSetPcLimitedNum( int limitedNumber );

//
// 获取wifi连接配置
//
int CfgManageGetWifiConnectParam( char *pEssid, char *pPasswd );
int CfgManageGetWifiNetType( unsigned char *pNetType );
int CfgManageGetNetworkWifi( char *ip, char *netmask, char *gateway, char *useWifiGateway );
int CfgManageGetPcClsCountTime(CONFIG_CLEAR_PC_PERIOD *clsPeriod);

int CfgManageGetsynovateIpPort( char *pIp, unsigned short *pPort);
int CfgManageAutoMaintainDaylight(char *pFDaylight,char *pStart,char *pEnd);
int CfgManageAutoDaylight(char *pftypedaylight,
                          int *pstartMonth,int *pstartWhichDis,int *pstartWhichDay,int *pstartHour,
                          int *pendMonth,int *pendWhichDis,int *pendWhichDay,int *pendHour);
int CfgManageSaveDaylight(int *psetdaylight);
int CfgManageGetMountDevFlag();
int GetFtpRecFlag();
int GetFtpNetParam(SYS_CONFIG_FTP_REC *sysFtpRec);

/* Ftp上传客流信息配置 */
int CfgManageGetFtpUploadConfig( SYS_CONFIG_FTP_UPLOAD *sysFtpUpload );
int CfgManageSetFtpUploadConfig( SYS_CONFIG_FTP_UPLOAD *sysFtpUpload );

/* 获得店铺号 */
int CfgManageGetShopID( int &nShopID );
int CfgManageGetSerialNum( char *pSerialNum );

#endif  // _CONFIG_MANAGE_H

