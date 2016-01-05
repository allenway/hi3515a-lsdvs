/********************************************************************************
**	Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**	author        :  sven
**	version       :  v1.0
**	date           :  2013.10.10
**	description  : net 模块mac 接口函数
********************************************************************************/
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>

#include "const.h"
#include "debug.h"
#include "linuxFile.h"
#include "netCom.h"
#include "rand.h"

/***********************************************************************
* fn: change rand number to char
***********************************************************************/
static unsigned char RandomChar()
{
    unsigned char a;

    a =(unsigned char)RandInt();
    if (a < 'A')
    {
    	a = a % 10 + 48;
    }
    else if (a < 'F')
       {
    	a = a % 6 + 65;
    }
    else if (a < 'a' || a > 'f')
    {
    	a = a % 6 + 97;
    }

    return a;
}

/*******************************************************************
* fn: 产生一个随机的mac 地址
* pMac: 存放产生后的mac 地址
********************************************************************/
void NetGenRandMac( char *pMac )
{
    char mac[18]={'0','0',};
    int i;
    /*set mac addr */
    for(i=3; i<17; i++)
    {
        usleep(10);
        mac[i] = RandomChar();
    }
    mac[2] = mac[5] = mac[8] = mac[11] = mac[14] = ':';
    mac[17] = '\0';

	if(NULL != pMac) 
    {
    	strcpy(pMac, mac);
    }
    
    SVPrint("Gen Rand Mac:%s!\r\n", mac);
}

/********************************************************************
* fn: 获取网卡的mac 地址
* pIf: 网卡名字,例如eth0
* macAddr: out,存放获取到的IP 地址
* len: macAddr所指向空间的大小
**********************************************************************/
int NetGetMacAddr( char *pIf, char *macAddr, int len )
{
    int fd;
    char buffer[NET_ADDRSIZE];
    struct ifreq ifr;

	if( NULL == macAddr || NULL == pIf )
    {
    	SVPrint("error: NULL == macAddr || NULL == pIf !\r\n");
    	return FI_FAILED;
    }

    if( (fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0 )
    {
        strncpy(ifr.ifr_name, pIf, IFNAMSIZ);
        ifr.ifr_name[IFNAMSIZ - 1] = '\0';
        if(ioctl(fd, SIOCGIFHWADDR, &ifr) == 0)
        {
            snprintf(buffer, NET_ADDRSIZE, "%02x:%02x:%02x:%02x:%02x:%02x",
                    (unsigned char)ifr.ifr_hwaddr.sa_data[0],
                    (unsigned char)ifr.ifr_hwaddr.sa_data[1],
                    (unsigned char)ifr.ifr_hwaddr.sa_data[2],
                    (unsigned char)ifr.ifr_hwaddr.sa_data[3],
                    (unsigned char)ifr.ifr_hwaddr.sa_data[4],
                    (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
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
	strncpy(macAddr, buffer, len);
    
	Close( fd );
	return FI_SUCCESSFUL;
}

int NetConvertMacToInt( const char *macAddr, int *p )
{
    int i = 0, j = 0;
    int n1 = 0, n2 = 0;

	if( NULL == macAddr || NULL == p )
    {
    	SVPrint("error:NULL==macAddr || NULL==p!\r\n");
    	return FI_FAILED;
    }
	int len = strlen(macAddr);
    for (i = 0, j = 0; i < len; i = i+3, j++)
    {
    	if (macAddr[i] >= 48 && macAddr[i] <= 57) /*'0' <--> '9'*/
        	n1 = macAddr[i] - 48; /*'0' == 48*/
        else if(macAddr[i] >= 65 && macAddr[i] <= 70) /*'A' <--> 'F'*/
        	n1 = macAddr[i] - 55; /*'A' == 65, and in hex 'A' = 10.*/
    	else if (macAddr[i] >= 97) /*'a' <--> 'f'*/
            n1 = macAddr[i] - 87; /*'a' == 97, and in hex 'a' = 10.*/
        if (macAddr[i+1] >= 48 && macAddr[i+1] <= 57)
            n2 = macAddr[i+1] - 48;
    	else if (macAddr[i+1] >= 65 && macAddr[i+1] <= 70)
        	n2 = macAddr[i+1] - 55;
        else if (macAddr[i+1] >= 97)
            n2 = macAddr[i+1] - 87;

        p[j] = n1*16 + n2;
    }
    return FI_SUCCESSFUL;
}

int NetSetMacAddr( char *pIf, const char *macAddr )
{
	int i = 0;
	int fd;
	int imac[6];
	struct ifreq ifr;

	if( NULL == macAddr )
    {
    	SVPrint( "error:NULL==macAddr!\r\n" );
    	return FI_FAILED;
    }
    
	if (CheckMacAddress(macAddr) < 0)
    {
    	SVPrint( "MAC addr format is not correct. %s!\r\n", macAddr );                
    	return FI_FAILED;
    }
	if ((fd =socket(AF_INET,SOCK_DGRAM,0)) < 0)
    {
    	SVPrint( "socket() error:%s!\r\n", STRERROR_ERRNO );
    	return FI_FAILED;
    }
	strncpy( ifr.ifr_name, pIf, IFNAMSIZ );
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    /*shutdown ifName.*/
	ifr.ifr_flags &= ~IFF_UP;
	if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0)
    {
    	Close( fd );
    	SVPrint( "ioctl() error:%s!\r\n", STRERROR_ERRNO );    
    	return FI_FAILED;
    }
    /*change mac address.*/
	ifr.ifr_hwaddr.sa_family = 1;
	NetConvertMacToInt( macAddr, imac );
	for (i = 0; i < 6; i++)
    {
    	ifr.ifr_hwaddr.sa_data[i] = (unsigned char)imac[i];
    }
    if (ioctl(fd,SIOCSIFHWADDR,&ifr) < 0)
    {
    	Close(fd);
    	SVPrint("ioctl() error:%s!\r\n",STRERROR_ERRNO);    
        return FI_FAILED;
    }
    /*up ifName.*/
	ifr.ifr_flags |= IFF_UP;
	if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0)
    {
    	Close(fd);
    	SVPrint("ioctl() error:%s!\r\n",STRERROR_ERRNO);    
    	return FI_FAILED;
    }
	Close(fd);
	return FI_SUCCESSFUL;
}


