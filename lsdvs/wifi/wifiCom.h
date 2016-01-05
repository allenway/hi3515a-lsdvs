/*
*******************************************************************************
**  Copyright (c) 2014, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : wifi的一些公共宏
**  date           :  2013.10.21
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#ifndef __WIFICOM_H__
#define __WIFICOM_H__

typedef enum _WifiEncTypeEn_
{
	WIFI_ENC_NONE = 0,
	WIFI_ENC_WEP,
	WIFI_ENC_WPA,
	WIFI_ENC_WPAPSK
} WIFI_ENC_TYPE_EN;

typedef enum _WifiSignalLevel_
{
	WIFI_SIGNAL_LEVEL_0 = 0,
	WIFI_SIGNAL_LEVEL_1,
	WIFI_SIGNAL_LEVEL_2,
	WIFI_SIGNAL_LEVEL_3,
	WIFI_SIGNAL_LEVEL_4,    
	WIFI_SIGNAL_LEVEL_5,    
} WIFI_SIGNAL_LEVEL;

typedef enum _WifiFifoCmdEn_
{
	WIFI_FIFO_CMD_SCAN = 11,    // 搜索
	WIFI_FIFO_CMD_START,        // 开始wifi
	WIFI_FIFO_CMD_STOP,            // 停止wifi
	WIFI_FIFO_CMD_RESTART,        // 重启wifi
	WIFI_FIFO_CMD_START_DHCP,    // 以DHCP的方式来启动wifi
	WIFI_FIFO_CMD_RESTART_DHCP	// 以DHCP的方式来重启wifi
} WIFI_FIFO_CMD_EN;


typedef struct _WifiSignalLevelMap_
{
	int signalVal;    // 单位 (dBm)
	int level;        // 单位 (格)
} WIFI_SIGNAL_LEVEL_MAP;

int WifiSignalDbmToLevel( int dbm );

#endif // __WIFICOM_H__

