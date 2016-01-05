/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此文件为DVS系统主文件，包含对其他各个模块的初始化，调用。
**  date           :  2011.11.8
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/

#ifndef __WIFICONF_H__
#define __WIFICONF_H__

#define WIFI_CONF_CONNECT_FILE	    "/tmp/wifi.conf"
#define WIFI_CONF_NETWORK_FILE	    "/tmp/wifi.network"
#define WIFI_CONF_ESSID     "essid"
#define WIFI_CONF_PASSWD     "passwd"

#define WIFI_CONF_IP         "ip"
#define WIFI_CONF_NETMASK     "netmask"
#define WIFI_CONF_GATEWAY     "gateway"

int WifiSetConncetInfoToFile( char *pEssid, char *pPasswd );
int WifiSetNetworkInfoToFile( char *pIp, char *pNetmask, char *pGateway );

#endif // __WIFICONF_H__

