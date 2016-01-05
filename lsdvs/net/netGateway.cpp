/********************************************************************************
**	Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**	author        :  sven
**	version       :  v1.0
**	date           :  2013.10.10
**	description  : net 的gateway 接口函数
********************************************************************************/
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/route.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include "const.h"
#include "netCom.h"
#include "linuxFile.h"
#include "debug.h"
#include "netGateway.h"

#define CH_TO_HEX(ch) ((ch)<='9'?(ch)-48:(ch)-'A'+10)

int NetRouteGetGateway( char *pIf, char *gateway, int len )
{
	int i;
	int n[4] = {0};
	int fd = -1;
	char name[10] = {0}, dest[10] = {0}, dg[10] = {0}, buf[1024] = {0};

	if( NULL == gateway || NULL == pIf )
    {
    	SVPrint( "error:NULL == gateway || NULL == pIf!\r\n" );
    	return FI_FAILED;
    }

	memset(gateway, '\0', len);
	if( (fd = Open(SYSTEM_ROUTE_PATH, O_RDONLY)) < 0 )
    {
    	SVPrint("Open() error:%s!\r\n",STRERROR_ERRNO);    
    	return FI_FAILED;
    }
	while( Readline(fd, buf, 1024) > 0 )
    {
    	if( memcmp(buf, pIf, 4) != 0 )
        {
        	continue;
        }
    	sscanf(buf, "%4s\t%8s\t%8s\t", name, dest, dg);
    	if( !strcmp(dest, "00000000") )
        {
        	for (i = 0; i < 8; i = i+2)
            {
            	n[i/2] = CH_TO_HEX(dg[i])*16+CH_TO_HEX(dg[i+1]);
            }
            /*注意字节序*/
        	snprintf(gateway, len, "%d.%d.%d.%d", n[3], n[2], n[1], n[0]);
        	break;
        }
    }
	Close(fd);
	if(gateway[0] == '\0')
    {
    	SVPrint("get gateway failed:gateway[0] == 0\r\n");
    	return FI_FAILED;
    }
	return FI_SUCCESSFUL;
}

void NetRouteDelDefaultGateway()
{
	static char devname[16][256];
	struct rtentry rt[16];
	unsigned long val;
	int sock;
	struct in_addr inaddr;
	char buf[1024], *p = buf, tBuf[256];
	int i, j, num, len, coldev=0, coldst=1, colgw=2, colmask=7, colflag=3;
    
	int fd = Open( SYSTEM_ROUTE_PATH, O_RDONLY );
	if(-1 == fd) 
    {
    	SVPrint( "Open(%s)error:%s!\r\n", SYSTEM_ROUTE_PATH, STRERROR_ERRNO );
    	return;
    }
    
	len = Read( fd, buf, 1024-1 );
	buf[len] = 0;
	Close( fd );

	memset( rt, 0, sizeof(struct rtentry)*16 );

	i=0;
	while(*p) 
    {
    	while(!(((*p>='A') && (*p<='Z')) || ((*p>='a') && (*p<='z')) || ((*p=='\r') || (*p=='\n')))) p++;
        
    	if(*p==0 || *p=='\r' || *p=='\n') break;
        
    	if(strncasecmp(p,"iface",5)==0) coldev=i;
    	else if(strncasecmp(p,"destination",11)==0) coldst=i;
    	else if(strncasecmp(p,"gateway",7)==0) colgw=i;
    	else if(strncasecmp(p,"mask",4)==0) colmask=i;
    	else if(strncasecmp(p,"flags",5)==0) colflag=i;
        
    	while(((*p>='A') && (*p<='Z')) || ((*p>='a') && (*p<='z'))) p++;
        
    	i++;
    }
	while(*p=='\r' || *p=='\n') p++;
    
	i=0;
	num=0;
	while(*p) 
    {
    	j=0;
    	while(*p==' ' || *p=='\t') p++;
        
    	while(*p && (*p!=' ' && *p!='\t' && *p!='\r' && *p!='\n')) tBuf[j++]=*p++;
        
    	tBuf[j]=0;
    	if(j) 
        {
        	if(i==coldev) 
            {
            	strcpy(devname[num],tBuf);
            	rt[num].rt_dev=devname[num];
            } 
        	else 
            {
            	val = strtoul(tBuf,NULL,16);
            	if(i==coldst) 
                {
                    ((struct sockaddr_in*)&(rt[num].rt_dst))->sin_family = PF_INET;
                    ((struct sockaddr_in*)&rt[num].rt_dst)->sin_addr.s_addr = val;
                } 
            	else if(i==colgw) 
                {
                    ((struct sockaddr_in*)&rt[num].rt_gateway)->sin_family = PF_INET;
                    ((struct sockaddr_in*)&rt[num].rt_gateway)->sin_addr.s_addr=val;
                } 
            	else if(i==colmask) 
                {
                    ((struct sockaddr_in*)&rt[num].rt_genmask)->sin_family = PF_INET;
                    ((struct sockaddr_in*)&rt[num].rt_genmask)->sin_addr.s_addr = val;
                } 
            	else if(i==colflag) rt[num].rt_flags=(short)val;
            }
        }
    	i++;
    	if(*p==0 || *p=='\r' || *p=='\n') 
        {
        	if( (rt[num].rt_flags & RTF_UP) && ((struct sockaddr_in*)&rt[num].rt_dst)->sin_addr.s_addr==0 ) 
            {
            	num++;
            	if(num>=16) break;
            }
        	while(*p=='\r' || *p=='\n') p++;
        	i=0;
        }
    	if( *p == 0 ) break;    
    }
	if(num) 
    {
    	sock = socket( AF_INET, SOCK_DGRAM, 0 );
    	if( sock != -1 ) 
        {
        	for( i=0; i<num; i++ ) 
            {
            	inaddr.s_addr=((struct sockaddr_in*)&rt[i].rt_gateway)->sin_addr.s_addr;
            	if( ioctl(sock, SIOCDELRT, &rt[i]) < 0 ) 
                {
                	SVPrint( "ioctl SIOCDELRT(%s): %s\r\n", inet_ntoa(inaddr), STRERROR_ERRNO );
                }
            }
        	Close( sock );
        }
    }
}

static int SetDefaultRoute( int fd,  char *ifName ,unsigned int *routeAddr )   
{   
    struct  rtentry     rtent;   
    struct  sockaddr_in *p;   
   
    memset( &rtent,0,sizeof( struct rtentry ) );
    
    p = ( struct sockaddr_in * ) (&rtent.rt_dst);   
    p->sin_family = AF_INET;   
    p->sin_addr.s_addr = 0;   
    
    p = ( struct sockaddr_in * ) &rtent.rt_gateway;   
    p->sin_family = AF_INET;   
    p->sin_addr.s_addr = *routeAddr; 
    
    p = ( struct sockaddr_in * ) &rtent.rt_genmask;   
    p->sin_family = AF_INET;   
    p->sin_addr.s_addr = 0;   
   
    rtent.rt_dev = ifName;   
   
    rtent.rt_metric = 1;   
    rtent.rt_window = 0;   
    rtent.rt_flags  = (unsigned short)(RTF_UP | RTF_GATEWAY | RTF_DEFAULT | RTF_HOST);//RTF_UP | RTF_GATEWAY;   
    if ( ioctl( fd,SIOCADDRT,&rtent ) == -1 ) 
    {   
        if ( errno == ENETUNREACH )    /* possibly gateway is over the bridge */ 
        {   
            /* try adding a route to gateway first */   
            memset( &rtent,0,sizeof( struct rtentry ) );   
            p = ( struct sockaddr_in * ) &rtent.rt_dst;   
            p->sin_family = AF_INET;   
            p->sin_addr.s_addr = *routeAddr;   
            p = ( struct sockaddr_in * ) &rtent.rt_gateway;   
            p->sin_family = AF_INET;   
            p->sin_addr.s_addr = 0;   
            p = ( struct sockaddr_in * ) &rtent.rt_genmask;   
            p->sin_family = AF_INET;   
            p->sin_addr.s_addr = 0xffffffff;   
   
   
            rtent.rt_dev = ifName;   
            rtent.rt_metric = 0;   
            rtent.rt_flags = RTF_UP | RTF_HOST;   
            if ( ioctl( fd,SIOCADDRT,&rtent ) == 0 ) 
            {   
                memset( &rtent,0,sizeof( struct rtentry ) );   
                p = ( struct sockaddr_in * ) &rtent.rt_dst;   
                p->sin_family = AF_INET;   
                p->sin_addr.s_addr = 0;   
                p = ( struct sockaddr_in * ) &rtent.rt_gateway;   
                p->sin_family = AF_INET;   
                p->sin_addr.s_addr = *routeAddr;   
                p = ( struct sockaddr_in * ) &rtent.rt_genmask;   
                p->sin_family = AF_INET;   
                p->sin_addr.s_addr = 0;   
   
   
                rtent.rt_dev = ifName;   
   
                rtent.rt_metric = 1;   
                rtent.rt_window = 0;   
                rtent.rt_flags = RTF_UP | RTF_GATEWAY ;   
                if ( ioctl( fd,SIOCADDRT,&rtent ) == -1 ) 
                {   
                    ERRORPRINT("ioctl SIOCADDRT error: %s\n",STRERROR_ERRNO);   
                    return FI_FAILED;   
                }   
            }   
        } 
        else 
        {   
            ERRORPRINT("ioctl SIOCADDRT: error: %s\n",STRERROR_ERRNO);  
            return FI_FAILED;   
        }   
    }
    else
    {
        CORRECTPRINT("SET default gw successed!!\n");
    }
    return FI_SUCCESSFUL;   
}


int NetRouteAddDefaultGateway( char *ifName, char *gateway )
{
	int fd = -1;
	struct ifreq ifr;
	struct sockaddr_in addr;    

	if( NULL == gateway || NULL == ifName )
    {
    	SVPrint("error:NULL==gateway\r\n");
    	return FI_FAILED;
    }
    
	if( CheckIpAddress(gateway) < 0 )
    {
    	SVPrint( "CheckIpAddress() error!\r\n" );    
    	return FI_FAILED;
    }

	NetRouteDelDefaultGateway();
    
	if( (fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
    {
    	SVPrint( "socket() error:%s!\r\n", STRERROR_ERRNO );
    	return FI_FAILED;
    }
	strncpy( ifr.ifr_name, ifName, IFNAMSIZ );
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';
	bzero(&addr, sizeof(struct sockaddr_in));
	if( inet_pton(AF_INET, gateway, &addr.sin_addr) < 0 )
    {        
    	Close( fd );
    	SVPrint( "inet_pton() error:%s!\r\n", STRERROR_ERRNO );
        return FI_FAILED;
    }
    
    SetDefaultRoute( fd, ifName, &addr.sin_addr.s_addr );
    
	Close( fd );

	return FI_SUCCESSFUL;
}

