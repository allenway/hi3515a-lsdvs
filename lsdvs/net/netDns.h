#ifndef __NETDNS_H__
#define __NETDNS_H__

#define NET_RESOLVCONF_LINK "/tmp/resolv.conf"

int NetGetDns( char *firstDns, char *secondDns, int len );
int NetSetDns( const char *firstDns, const char *secondDns );

#endif

