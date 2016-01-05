/********************************************************************************
**	Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**	author        :  sven
**	version       :  v1.0
**	date           :  2013.10.10
**	description  : net 的dns 接口函数
********************************************************************************/
#include <stdio.h>
#include <unistd.h>

#include "const.h"
#include "debug.h"
#include "netDns.h"
#include "linuxFile.h"
#include "netCom.h"

int NetGetDns( char *firstDns, char *secondDns, int len )
{
	int flag = 0;
	int fd;
	char buf[COM_BUF_SIZE] = {0}, *ptr;

	if(NULL==firstDns || NULL==secondDns)
    {
    	SVPrint("error:NULL==firstDns || NULL==secondDns\r\n");
    	return FI_FAILED;
    }
	if( (fd = Open(NET_RESOLVCONF_LINK, O_RDONLY)) < 0 )
    {
    	SVPrint( "Open() error:%s!\r\n", STRERROR_ERRNO );
    	return FI_FAILED;
    }
	while( Readline(fd, buf, COM_BUF_SIZE) > 0 )
    {
    	if( strncmp(buf, "nameserver", 10) == 0 )
        {
        	ptr = buf;
        	Strsep( &ptr, " " );
        	if( flag == 0 )
            {
            	strncpy( firstDns, Strsep(&ptr, " "), len );
            	if (firstDns[strlen(firstDns)-1] == '\n')
                {
                	firstDns[strlen(firstDns)-1] = '\0';
                }
            	flag = 1;
            	if( secondDns == NULL )
                {
                	break;
                }
            	continue;
            }
        	else
            {
            	strncpy( secondDns, Strsep(&ptr, " "), len );
            	if( secondDns[strlen(secondDns)-1] == '\n' )
                {
                	secondDns[strlen(secondDns)-1] = '\0';
                }
            	flag = 2;
            	break;
            }
        }
    }
    
	Close( fd );
	if (flag == 0)
    {
    	SVPrint( "get DNS failed!\r\n" );
    	return FI_FAILED;
    }
    
	return(flag);
}

int NetSetDns( const char *firstDns, const char *secondDns )
{
	FILE *fp;
    
	if( NULL==firstDns ) // 注意,可以只设置一个dns
    {
    	SVPrint( "error:NULL==firstDns!\r\n" );
    	return FI_FAILED;
    }
	if( CheckIpAddress(firstDns) < 0 )
    {
    	SVPrint( "error firstDns:%s!\r\n", firstDns );
    	return FI_FAILED;
    }

	chmod(NET_RESOLVCONF_LINK, 0777);
	usleep(10);
	Unlink(NET_RESOLVCONF_LINK);
	if( (fp = fopen(NET_RESOLVCONF_LINK, "w")) == NULL )
    {
    	SVPrint( "fopen() error:%s!\r\n", STRERROR_ERRNO );
    	return FI_FAILED;
    }
	fprintf( fp, "nameserver %s\n", firstDns );
	if( NULL != secondDns )
    {
    	if( CheckIpAddress(secondDns) < 0 )
        {
        	SVPrint( "error secondDns:%s!\r\n", secondDns );
        	fclose( fp );
        	return FI_FAILED;
        }
    	fprintf( fp, "nameserver %s\n", secondDns );
    }    
	fclose( fp );
    
	return FI_SUCCESSFUL;
}

