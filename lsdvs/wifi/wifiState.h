/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 获取wifi的状态
**  date           :  2011.11.10
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/

#ifndef __WIFISTATE_H__
#define __WIFISTATE_H__

#define WIFI_STATE_FILE "/tmp/wifi.state"
#define WIFI_STATE_MARK_FLAG "state "
#define WIFI_STATE_MARK_SIGNAL "signal "

int WifiStateGet( unsigned char *pConnectState, unsigned char *pSignalLevel );

#endif // __WIFISTATE_H__


