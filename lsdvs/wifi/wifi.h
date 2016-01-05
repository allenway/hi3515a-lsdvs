/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 管理wifi模块
**  date           :  2013.10.21
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#ifndef __WIFI_H__
#define __WIFI_H__

#include "ttypes.h"

#define WIFI_SCAN_LINE_SIZE		128  // 每行最多支持128个字符
#define WIFI_SCAN_MAX_LINE		4      // 4行就可以确定一个wifi路由的信息
#define WIFI_SCAN_INTERVAL		5  // 至少隔5秒钟才搜一次

#define WIFI_SCAN_RESULT_FILENAME     "/tmp/wifiscanused.txt"
#define WIFI_SCAN_FINISH_FLAG         "/tmp/wifiscanfinish.txt"

#define WIFI_MARK_ESSID             "ESSID:"
#define WIFI_MARK_WIFI_ENC_WPA      "802.1x"
#define WIFI_MARK_WIFI_ENC_WPAPSK   "PSK"
#define WIFI_MARK_SIGNAL	        "Signal level:"
#define WIFI_MARK_KEY_ON	        ":on"
 
int WifiScanResult( WIFI_SCAN_RESULT **pWifiScanResult, int *pScanSize );
int WifiInitNetwork();

#endif // __WIFI_H__

