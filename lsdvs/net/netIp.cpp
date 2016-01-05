/********************************************************************************
**	Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**	author        :  sven
**	version       :  v1.0
**	date           :  2013.10.10
**	description  : net 模块IP接口函数
********************************************************************************/
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>

#include "const.h"
#include "debug.h"
#include "linuxFile.h"
#include "netCom.h"

/***************************************************************************
* fn: 获取网卡的IP 地址
* pIf: 网卡名字,例如eth0
* ipAddr: out,存放获取到的IP 地址
* len: ipAddr所指向空间的大小
***************************************************************************/
int NetGetIpAddr( char *pIf, char *ipAddr, int len )
{
	int fd, errNo;
	char buffer[NET_ADDRSIZE];
	struct ifreq ifr;
	struct sockaddr_in *addr;

	if( NULL == ipAddr || NULL == pIf )
    {
    	SVPrint( "error:NULL == ipAddr || NULL == pIf!\r\n" );
    	return FI_FAILED;
    }
    
	if( (fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0 )
    {
    	strncpy( ifr.ifr_name, pIf, IFNAMSIZ );
    	ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    	if( ioctl(fd, SIOCGIFADDR, &ifr) == 0 )
        {
        	addr = (struct sockaddr_in *)&(ifr.ifr_addr);
        	inet_ntop( AF_INET, &addr->sin_addr, buffer, 20 );
        }
    	else
        {
        	Close( fd );
        	errNo = errno;
            //防止不停打印, 19 == errNo 表示设备不存在;99 == errNo 表示不能分配请求的地址
        	if( 99 != errNo && 19 != errNo ) 
            {
            	SVPrint( "ioctl() error:%s,%d!\r\n", strerror(errNo), errNo );
            }
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
    	SVPrint( "error:strlen(buffer) > (len-1)!\r\n" );    
    	return FI_FAILED;
    }
	strncpy( ipAddr, buffer, len );
	Close( fd );
    
	return FI_SUCCESSFUL;
}

/*************************************************************
* fn: 设置网卡的IP 地址
* pIf: 网卡名字,例如eth0
* ipAddr: 所指定的IP 地址
****************************************************************/
int NetSetIpAddr( char *pIf, const char *ipAddr )
{
	int fd;
	struct ifreq ifr;
	struct sockaddr_in addr;

	if( NULL == ipAddr || NULL == pIf )
    {
    	SVPrint( "error: NULL == ipAddr || NULL == pIf!\r\n" );
    	return FI_FAILED;
    }
    
	if( CheckIpAddress(ipAddr) < 0 )
    {
    	SVPrint( "CheckIpAddress() error!\r\n" );    
    	return FI_FAILED;
    }
	if( (fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
    {
    	SVPrint( "socket() error:%s!\r\n", STRERROR_ERRNO );
    	return FI_FAILED;
    }
	strncpy( ifr.ifr_name, pIf, IFNAMSIZ );
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';
	bzero( &addr, sizeof(struct sockaddr_in) );
	if( inet_pton(AF_INET, ipAddr, &addr.sin_addr) < 0 )
    {        
    	Close(fd);
    	SVPrint( "inet_pton() error:%s!\r\n", STRERROR_ERRNO );
        return FI_FAILED;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    memcpy( &ifr.ifr_addr, &addr, sizeof(struct sockaddr) );
    if( ioctl(fd, SIOCSIFADDR, &ifr) < 0 )
    {        
    	Close(fd);
    	SVPrint( "ioctl() error:%s!\r\n", STRERROR_ERRNO );
       	return FI_FAILED;
    }
	Close( fd );
    
	return FI_SUCCESSFUL;
}

