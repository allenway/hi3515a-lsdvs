/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.06.13
**  description  : 有线网络管理
********************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "debug.h"
#include "malloc.h"
#include "linuxFile.h"
#include "paramConfig.h"

#include "sysRunTime.h"
#include "paramManage.h"
#include "network.h"
#include "netIp.h"
#include "netMask.h"
#include "netGateway.h"
#include "netCom.h"
#include "paramMac.h"
#include "netMac.h"
#include "netUdhcpcFifo.h"

int WiredInitNetwork()
{
	int 	ret;
	char	mac[24];
	PARAM_CONFIG_NETWORK network;

	ret = ParamGetMac( mac );
	if( 0 == ret )
    {
    	ret = NetSetMacAddr( NET_WIRED_NAME, mac );
    }
	ret = ParamGetNetwork( &network );    
	if( FI_SUCCESSFUL == ret )
    {    
    	if( 0 == network.wired.dhcpEnable )
        {
        	if( CheckIpAddress( network.wired.ip ) < 0 )
            {
            	SVPrint( "CheckIpAddress() error! set to default value\r\n" );    
            	strcpy( network.wired.ip, DEV_DEFAULT_IP );
            }
        	if( CheckIpAddress( network.wired.netmask) < 0 )
            {
            	SVPrint( "CheckNetmaskAddress() error! set to default value\r\n" );    
            	strcpy( network.wired.netmask, DEV_DEFAULT_NETMASK );
            }

        	ret = NetSetIpAddr( (char *)NET_WIRED_NAME, network.wired.ip );
        	if( ret == 0 ) ret = NetSetMask( (char *)NET_WIRED_NAME, network.wired.netmask );
        	if (ret == 0) ret = NetRouteAddDefaultGateway( (char *)NET_WIRED_NAME, network.wired.gateway );
        }
    	else
        {
        	UdhcpcForkCtrl( UDHCPC_FIFO_CMD_START );
        }
    }

	return 0;
}



