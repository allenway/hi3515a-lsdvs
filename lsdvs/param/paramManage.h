#ifndef __PARAMMANAGE_H__
#define __PARAMMANAGE_H__

#include "paramConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

// 回复出厂设置
void ParamSetFactoryConfigure();

// 恢复默认参数
void ParamSetDefaultParam();

// 系统基本参数
int ParamGetBaseInfo(PARAM_CONFIG_BASE_INFO *pParam);
int ParamSetBaseInfo(PARAM_CONFIG_BASE_INFO *pParam);

// 网络参数
int ParamGetNetwork(PARAM_CONFIG_NETWORK *pParam);
int ParamSetNetwork(PARAM_CONFIG_NETWORK *pParam);

// 客户端或IE登录用户参数
int ParamGetClientUser(int n, PARAM_CONFIG_CLIENT_USER *pParam);
int ParamSetClientUser(int n, PARAM_CONFIG_CLIENT_USER *pParam);

// 视频编码公共参数
int ParamGetVideoEncodePublic(PARAM_CONFIG_VIDEO_ENCODE_PUBLIC *pParam);
int ParamSetVideoEncodePublic(PARAM_CONFIG_VIDEO_ENCODE_PUBLIC *pParam);

// 主码流视频编码参数
int ParamGetVideoEncode(int n, PARAM_CONFIG_VIDEO_ENCODE *pParam);
int ParamSetVideoEncode(int n, PARAM_CONFIG_VIDEO_ENCODE *pParam);

//从码流视频编码参数
int ParamGetVideoEncodeSlave(int n, PARAM_CONFIG_VIDEO_ENCODE *pParam);
int ParamSetVideoEncodeSlave(int n, PARAM_CONFIG_VIDEO_ENCODE *pParam);

// 视频采集基本参数
int ParamGetVideoBaseParam(int n, PARAM_CONFIG_VIDEO_BASE_PARAM *pParam);
int ParamSetVideoBaseParam(int n, PARAM_CONFIG_VIDEO_BASE_PARAM *pParam);

// logo OSD 参数
int ParamGetOsdLogo(int n, PARAM_CONFIG_OSD_LOGO *pParam);
int ParamSetOsdLogo(int n, PARAM_CONFIG_OSD_LOGO *pParam);

// 时间 OSD 参数
int ParamGetOsdTime(int n, PARAM_CONFIG_OSD_TIME *pParam);
int ParamSetOsdTime(int n, PARAM_CONFIG_OSD_TIME *pParam);

// 音频参数
int ParamGetAudio(PARAM_CONFIG_AUDIO_ENCODE *pParam);
int ParamSetAudio(PARAM_CONFIG_AUDIO_ENCODE *pParam);

// 自动维护参数
int ParamGetAutoMaintain(PARAM_CONFIG_AUTO_MAINTAIN *pParam);
int ParamSetAutoMaintain(PARAM_CONFIG_AUTO_MAINTAIN *pParam);

// 录像公共参数
int ParamGetRecordPublic(PARAM_CONFIG_RECORD_PUBLIC *pParam);
int ParamSetRecordPublic(PARAM_CONFIG_RECORD_PUBLIC *pParam);

// 录像通道参数
int ParamGetRecordParam(int n, PARAM_CONFIG_RECORD_PARAM *pParam);
int ParamSetRecordParam(int n, PARAM_CONFIG_RECORD_PARAM *pParam);

// IO报警参数设置
int ParamGetAlarmIo(PARAM_CONFIG_ALARM_IO *pParam);
int ParamSetAlarmIo(PARAM_CONFIG_ALARM_IO *pParam);

// 串口参数
int ParamGetSerial(PARAM_CONFIG_SERIAL *pParam);
int ParamSetSerial(PARAM_CONFIG_SERIAL *pParam);

// NTP
int ParamGetNtp(PARAM_CONFIG_NTP *pParam);
int ParamSetNtp(PARAM_CONFIG_NTP *pParam);

// Email参数
int ParamGetEmail(PARAM_CONFIG_EMAIL *pParam);
int ParamSetEmail(PARAM_CONFIG_EMAIL *pParam);

// 移动侦测参数
int ParamGetAlarmMoveDetect(int n, PARAM_CONFIG_ALARM_MOVE_DETECT *pParam);
int ParamSetAlarmMoveDetect(int n, PARAM_CONFIG_ALARM_MOVE_DETECT *pParam);

// 视频遮挡
int ParamGetVideoOverlay(int n, PARAM_CONFIG_VIDEO_OVERLAY *pParam);
int ParamSetVideoOverlay(int n, PARAM_CONFIG_VIDEO_OVERLAY *pParam);

// 视频丢失报警设置
int ParamGetAlarmVideoLose(int n, PARAM_CONFIG_ALARM_VIDEO_LOSE *pParam);
int ParamSetAlarmVideoLose(int n, PARAM_CONFIG_ALARM_VIDEO_LOSE *pParam);

// ftp 配置
int ParamGetFtp(PARAM_CONFIG_FTP *pParam);
int ParamSetFtp(PARAM_CONFIG_FTP *pParam);

// 定时抓拍
int ParamGetSnapTimer(int n, PARAM_CONFIG_SNAP_TIMER *pParam);
int ParamSetSnapTimer(int n, PARAM_CONFIG_SNAP_TIMER *pParam);

// ICMP
int ParamGetIcmp(int n, PARAM_CONFIG_ICMP *pParam);
int ParamSetIcmp(int n, PARAM_CONFIG_ICMP *pParam);

// ddns 参数
int ParamGetDdns(int n, PARAM_CONFIG_DDNS *pParam);
int ParamSetDdns(int n, PARAM_CONFIG_DDNS *pParam);

// dtu 参数
int ParamGetDtu( PARAM_CONFIG_DTU *pParam);
int ParamSetDtu( PARAM_CONFIG_DTU *pParam);

// wifi 连接参数
int ParamGetWifiConnect( PARAM_CONFIG_WIFI_CONNECT_T *pParam );
int ParamSetWifiConnect( PARAM_CONFIG_WIFI_CONNECT_T *pParam );

int ParamGetThreeg( PARAM_CONFIG_THREEG_T *pParam );
int ParamSetThreeg( PARAM_CONFIG_THREEG_T *pParam );

// 视频遮挡
int ParamGetAlarmVideoShelter( int n, PARAM_CONFIG_ALARM_VIDEO_SHELTER *pParam );
int ParamSetAlarmVideoShelter( int n, PARAM_CONFIG_ALARM_VIDEO_SHELTER *pParam );

//所有参数
int ParamGetAllParam(SYS_CONFIG *pParam);
int ParamSetAllParam(SYS_CONFIG *pParam);

#ifdef __cplusplus
}
#endif 

#endif 
