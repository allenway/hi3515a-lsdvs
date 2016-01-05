#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "const.h"

#define NET_WIRED_NAME     "eth0"    // 有线网卡的名字
#define NET_WIFI_NAME     "ra0"    // wifi网卡的名字

int NetworkGetIpAddr( char *pIf, char *ipAddr, int len );
int NetworkGetMacAddr( char *pIf, char *macAddr, int len );

#endif

