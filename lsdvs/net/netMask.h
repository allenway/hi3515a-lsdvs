#ifndef __NETMASK_H__
#define __NETMASK_H__

int NetGetMask( char *pIf, char *netMask, int len );
int NetSetMask( char *pIf, const char *netMask );

#endif

