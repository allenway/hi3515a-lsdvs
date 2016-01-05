/********************************************************************************
**	Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**	author        :  sven
**	version       :  v1.0
**	date           :  2013.10.10
**	description  : net 模块公共函数
********************************************************************************/
#include <string.h>
#include <stdlib.h>
#include "const.h"
#include "debug.h"
#include "linuxFile.h"


/************************************************************************
* fn: 检查ip 地址是否合法
* ipAddr: 被检查的IP 地址
* 返回: 0, 合法; -1, 非法
*************************************************************************/
int CheckIpAddress( const char *ipAddr )
{
	int i = 0, flag = 0;
	char buf[16] = {0};
	char *tmp, *ptr;

	if( NULL == ipAddr )
    {
    	SVPrint( "error:NULL == ipAddr!\r\n" );
    	return FI_FAILED;
    }

	if (strlen(ipAddr) > 15)
    {
    	return FI_FAILED;
    }
	strncpy( buf, ipAddr, 16 );

	while (buf[i] != '\0')
    {
    	switch (buf[i])
        {
        	case '0':
        	case '1':
        	case '2':
        	case '3':
        	case '4':
        	case '5':
        	case '6':
        	case '7':
        	case '8':
        	case '9':
            	break;
        	case '.':
            	flag++;
            	break;
        	default:
            	return FI_FAILED;
        }
    	i++;
    }
	if (flag != 3)
    {
    	return FI_FAILED;
    }

	ptr = buf;
	for (i = 0; i < 4; i++)
    {
    	tmp = Strsep(&ptr, ".");
    	if(tmp == NULL)    
        {
        	return FI_FAILED;
        }
    	if( atoi(tmp) > 255 || strlen(tmp) > 3 || (tmp[0] == '0' && strlen(tmp) != 1) )
        {
        	return FI_FAILED;
        }
    }        
	return FI_SUCCESSFUL;
}

/*****************************************************************
* fn: 检查mac 地址是否合法
* macAddr: 被检查的mac 地址
* 返回: 0, 合法; -1, 非法
*******************************************************************/
int CheckMacAddress( const char *macAddr )
{
    int i = 0;
    int flag = 0;

    if( NULL == macAddr )
    {
    	SVPrint( "error:NULL == macAddr!\r\n" );
    	return FI_FAILED;
    }
    if( strlen(macAddr) != 17 )
    {
    	SVPrint( "error:strlen(macAddr) != 17!\r\n" );
        return FI_FAILED;
    }
    while (macAddr[i] != '\0')
    {
        switch (macAddr[i])
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
                break;
            case ':':
                flag++;
                break;
            default:
            	SVPrint( "mac has wrong letter, please check!\r\n" );
                return FI_FAILED;
        }
        i++;
    }
    if (flag != 5)
    {
    	SVPrint( "error: num of : if less than 5!\r\n" );
    	return FI_FAILED;
    }
    if (macAddr[2] != ':' || macAddr[5] != ':' || macAddr[8] != ':' 
    || macAddr[11] != ':' || macAddr[14] != ':')
    {            
    	SVPrint( "error: mac is not like aa:bb:cc:ee:ff:gg!\r\n" );
    	return FI_FAILED;
    }        
    
    return FI_SUCCESSFUL;
}

