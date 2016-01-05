#ifndef __NETMAC_H__
#define __NETMAC_H__

void NetGenRandMac( char *pMac );
int NetGetMacAddr( char *pIf, char *macAddr, int len );
int NetSetMacAddr( char *pIf, const char *macAddr );
int NetConvertMacToInt( const char *macAddr, int *p );

#endif

