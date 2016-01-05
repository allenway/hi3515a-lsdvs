/********************************************************************************
**	Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**	author        :  sven
**	version       :  v1.0
**	date           :  2013.10.10
**	description  : net 掩码接口函数
********************************************************************************/

#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
    
#include "const.h"
#include "debug.h"
#include "linuxFile.h"
#include "netCom.h"

/*********************************************************************************
* fn: 获取网卡的子网掩码
* pIf: 网卡名字,例如eth0
* netMask: out,存放获取到的子网掩码
* len: netMask所指向空间的大小
*********************************************************************************/
int NetGetMask( char *pIf, char *netMask, int len )
{
	int fd;
	char buffer[NET_ADDRSIZE];
	struct ifreq ifr;
	struct sockaddr_in *addr;

	if(NULL==netMask)
    {
    	SVPrint( "error:NULL == netMask!\r\n" );
    	return FI_FAILED;
    }
	if( (fd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0 )
    {
    	strncpy(ifr.ifr_name, pIf, IFNAMSIZ);
    	ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    	if( ioctl(fd, SIOCGIFNETMASK, &ifr) == 0 )
        {
        	addr = (struct sockaddr_in *)&(ifr.ifr_addr);
        	inet_ntop(AF_INET, &addr->sin_addr, buffer, 20);
        }
    	else
        {
        	Close( fd );
        	SVPrint( "ioctl() error:%s!\r\n", STRERROR_ERRNO );
        	return FI_FAILED;
        }
    }
	else
    {
    	SVPrint( "socket() error:%s!\r\n", STRERROR_ERRNO );    
    	return FI_FAILED;
    }
    
	if( strlen(buffer) > (unsigned int)(len-1) )
    {        
    	Close( fd );
    	SVPrint( "error:strlen(buffer) = %d > %d!\r\n", strlen(buffer), len-1 );
    	return FI_FAILED;
    }
	strncpy( netMask, buffer, len );

	Close( fd );
	return FI_SUCCESSFUL;
}

/*
* fn: 设置网卡的子网掩码
* pIf: 网卡名字,例如eth0
* netMask: 所指定的子网掩码
*/
int NetSetMask( char *pIf, const char *netMask )
{
	int fd;
	struct ifreq ifr;
	struct sockaddr_in addr;    

	if( NULL == netMask )
    {
    	SVPrint( "error:NULL == netMask!\r\n" );
    	return FI_FAILED;
    }

	if( CheckIpAddress(netMask) < 0 )
    {
    	SVPrint( "error netMask:%s!\r\n", netMask );
    	return FI_FAILED;
    }
	if( (fd = socket (AF_INET, SOCK_DGRAM, 0)) < 0 )
    {
    	SVPrint( "socket() error:%s!\r\n", STRERROR_ERRNO );
    	return FI_FAILED;
    }
	strncpy( ifr.ifr_name, pIf, IFNAMSIZ );
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';
	bzero( &addr, sizeof(struct sockaddr_in) );
	if( inet_pton(AF_INET, netMask, &addr.sin_addr) < 0 )
    {        
    	Close(fd);
    	SVPrint( "inet_pton() error:%s!\r\n", STRERROR_ERRNO );
        return FI_FAILED;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    memcpy( &ifr.ifr_addr, &addr, sizeof(struct sockaddr) );
    if( ioctl(fd, SIOCSIFNETMASK, &ifr) < 0 )
    {
    	Close( fd );
    	SVPrint( "ioctl() error:%s!\r\n", STRERROR_ERRNO );
        return FI_FAILED;
    }
	Close( fd );
	return FI_SUCCESSFUL;
}

