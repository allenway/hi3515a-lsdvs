#ifndef __NETGATEWAY_H__
#define __NETGATEWAY_H__

#define SYSTEM_ROUTE_PATH "/proc/net/route"
int NetRouteAddDefaultGateway( char *ifName, char *gateway );
int NetRouteGetGateway( char *pIf, char *gateway, int len );
void NetRouteDelDefaultGateway();

#endif

