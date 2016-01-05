/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 利用ftpLib 接口封装的文件上传函数,
            	ftp 协议有两个上下文，一个用于指令控制; 另外一个用于传输数据
*******************************************************************************
*/

#include <stdio.h>
#include "debug.h"
#include "ftpLib.h"
#include "ftpFile.h"
#include "netSocket.h"

/*
* fn: 初始化一个ftp 连接, 登陆ftp 服务器
* host: 将要向该地址连接
* user: 登陆ftp 服务器的用户名
* pass: 登陆ftp 服务器的密码
* nControl: out, 成功登陆后得到的控制上下文
*/
int FtpFileInit( const char *host, const char *user, const char *pass, netbuf **nControl )
{
	if( host == NULL || user == NULL || pass == NULL )
    	return -1;
    
	if( FtpConnect(host, nControl) != 1 )
    {
    	SVPrint("connect host %s fail!\r\n", host);
    	return -1;
    }
	if( FtpLogin(user, pass, *nControl) != 1 )
    {
    	SVPrint( "login fail!\r\n" );
    	return -1;
    }
	return 0;
}

/*
* fn: 退出ftp 服务器
* nControl: FtpFileInit() 成功返回后得到的控制上下文
*/
int FtpFileQuit( netbuf *nControl )
{
	if( nControl == NULL )
    {
    	return -1;
    }
    
	FtpQuit( nControl );
	return 0;
}

/*
* fn: 在ftp服务器下打开一个文件
* path: ftp 服务器下要打开的文件名
* typ: FtpAccess() type codes, 参考ftpLib.h
* nControl: FtpFileInit() 成功返回后得到的控制上下文
* nData: out, 文件成功后返回的数据传输上下文
* dirChangFlag: path 是否只包含文件名, 0, 是的,不需要改变目录; 1,不是,需要改变目录
*/
int FtpFileOpen( const char *path, int typ, netbuf *nControl, netbuf **nData, int dirChangFlag )
{
	char dir[256] = {0};
	char *pTargetDir = NULL;
	unsigned int i;
	int mode = 'I';

	if( path == NULL || nControl == NULL || nData == NULL )
    {
    	return -1;
    }
	strncpy( dir, path, sizeof(dir)-1 );
	pTargetDir = dir;
	if( dirChangFlag == 1 )    
    {
    	for( i = 0; i < strlen(path); i++ )
        {
        	if( dir[i] == '/' )
            {
            	dir[i] = 0x00;
            	if( FtpChdir(pTargetDir, nControl) != 1 )
                {
                	if( FtpMkdir(pTargetDir, nControl) != 1 )
                    {
                    	SVPrint( "mkdir %s fail \r\n", pTargetDir );
                    	return -1;
                    }
                	if( FtpChdir(pTargetDir, nControl) != 1 )
                    {
                    	SVPrint( "chdir %s fail \r\n", pTargetDir );
                    	return -1;
                    }
                }

            	pTargetDir = &dir[i+1];
            }
        } // for( i = 0; i < strlen(path)                    
    }
	else
    {
    	for(i = strlen(dir)-1; i >= 0; i--)
        {
        	if( dir[i] == '/' )
            {
               pTargetDir = &dir[i+1];
               break;
            }
        }
    }
	if( !FtpAccess(pTargetDir, typ, mode, nControl, nData) )
    {
    	SVPrint("open ftp file fail!\n");
    	return -1;
    }
	return 0;
}

/*
* fn: 往ftp 服务器写数据
* buf: 数据buf
* len: 数据长度
* nData: FtpFileOpen() 成功返回后得到的数据上下文
*/
int FtpFileWrite( void *buf, int len, netbuf *nData )
{
	int ret;
	if ( buf == NULL || nData == NULL )
    {
    	return -1;
    }
	ret = SelectWrite( nData->handle, 0 );
	if( ret > 0 )
    {    
    	ret = FtpWrite( buf, len, nData );
    }

	return ret;
}

/*
* fn: 在ftp 服务器读数据
* buf: 数据buf
* len: 数据长度
* nData: FtpFileOpen() 成功返回后得到的数据上下文
*/
int FtpFileRead( void *buf, int max, netbuf *nData )
{
	return 0;
}

/*
* fn: 关闭操作的文件
* nControl: FtpFileInit() 成功返回后得到的控制上下文
* nData: FtpFileOpen() 成功返回后得到的数据上下文
*/
int FtpFileClose( netbuf *nControl, netbuf *nData )
{
	if( nData == NULL )
    {
    	return -1;
    }
    
	FtpClose( nData );    
	if( readresp('2', nControl) != 1 )
    {
    	return -1;
    }
	return 0;
}

