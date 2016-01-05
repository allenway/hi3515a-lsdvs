#ifndef __NETIP_H__
#define __NETIP_H__

int NetGetIpAddr( char *pIf, char *ipAddr, int len );
int NetSetIpAddr( char *pIf, const char *ipAddr );

#endif

