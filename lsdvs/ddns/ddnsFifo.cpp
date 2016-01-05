/********************************************************************************
**	Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**	author        :  sven
**	version       :  v1.0
**	date           :  2013.10.10
**	description  : 管理和ddns.sh 通信的fifo
********************************************************************************/
#include "debug.h"
#include "linuxFile.h"
#include "ddnsFifo.h"
#include "mutex.h"
#include "linuxFile.h"
#include "paramManage.h"

static ClMutexLock g_ddnsFifoLock;

int DdnsFifoCmd( DDNS_FIFO_CMD_EN cmd )
{
	int ret, fd;
	int writeLen;
	char writeBuf[4] = { 0 };
    
	g_ddnsFifoLock.Lock();
    
	fd = Open( DDNS_FIFO_DEVICE, O_RDWR );
	if( fd != -1 )
    {
    	sprintf( writeBuf, "%d", cmd );    
    	writeLen = strlen(writeBuf);    
    	SVPrint( "writen ddns cmd(%s) to %s!\r\n", writeBuf, DDNS_FIFO_DEVICE );
    	ret = Writen( fd, writeBuf, writeLen );
    	if( ret != writeLen )
        {
        	SVPrint( "Writen error:%s!\r\n", STRERROR_ERRNO );
        	ret = -1;
        }
    	else
        {
        	ret = 0;
        }
    	Close( fd );
    }
	else
    {
    	SVPrint( "open(%s) error:%s!\r\n", DDNS_FIFO_DEVICE, STRERROR_ERRNO );
    	ret = -1;
    }
	g_ddnsFifoLock.Unlock();
    
	return ret;
}

/********************************************************************************
* fn: 删除某个文件.
    	在删除前先检测该文件是否存在,如果存在,则尝试删除; 如果不存在,则返回成功
* pathname: 被删除的文件名
*********************************************************************************/
static int UnlinkFile( char *pathname )
{
	int ret ;

	ret = Access( pathname, F_OK );
	if( -1 == ret )
    {
    	return 0;
    }
	else
    {
    	ret = Unlink( pathname );
    }

	return ret;
}

/*********************************************************************************
* fn: 写花生壳配置
[settings] 
szHost =  
szUserID =  
szUserPWD = 
nicName = eth0 
szLog = /var/log/phddns.log 
********************************************************************************/
static void FputsOray( FILE *fddnsConf, PARAM_CONFIG_DDNS ddnsConf )
{
	char buf[256];
    
    
    
	Fputs( (char *)"[settings]\n", fddnsConf );
	Fputs( (char *)"szHost = PhLinux3.Oray.Net\n", fddnsConf );
    
	sprintf( buf, "szUserID = %s\n", ddnsConf.userName );    
	Fputs( buf, fddnsConf );
	sprintf( buf, "szUserPWD = %s\n", ddnsConf.passwd );    
	Fputs( buf, fddnsConf );
	sprintf( buf, "nicName = %s\n", ddnsConf.ifName );    
	Fputs( buf, fddnsConf );
    
	Fputs( (char *)"szLog = /var/log/phddns.log\n", fddnsConf );
    
	sprintf( buf, "# enable = %d\n", ddnsConf.enable );    
	Fputs( buf, fddnsConf );    
	Fputs( (char *)"# vender = oray\n", fddnsConf );
}

/*****************************************************************************
* fn: 写希网配置
service-type=qdns
user=
host=
interface=eth0
***************************************************************************/
static void Fputs3322Org( FILE *fddnsConf, PARAM_CONFIG_DDNS ddnsConf )
{
	char buf[256];
    
	strcpy( buf, "service-type=");
	if( DDNS_VENDER_QDNS == ddnsConf.vender )
    {
    	strcat( buf, "qdns\n" );
    }
	else if( DDNS_VENDER_DYNDNS == ddnsConf.vender )
    {        
    	strcat( buf, "dyndns\n" );
    }
	Fputs( buf, fddnsConf );
    
	sprintf( buf, "user=%s:%s\n", ddnsConf.userName, ddnsConf.passwd );    
	Fputs( buf, fddnsConf );
    
	sprintf( buf, "host=%s\n", ddnsConf.url );    
	Fputs( buf, fddnsConf );
    
	sprintf( buf, "interface=%s\n", ddnsConf.ifName );    
	Fputs( buf, fddnsConf );

	sprintf( buf, "# enable = %d\n", ddnsConf.enable );    
	Fputs( buf, fddnsConf );    
	Fputs( (char *)"# vender = 3322org\n", fddnsConf );
}

/****************************************************************************
* fn: 从配置模块读出配置, 格式化后写ddns 配置到文件
* pathname: 配置文件的名字
*****************************************************************************/
int DdnsWriteConff( char *pathname )
{
	int ret;
	FILE *fddnsConf;
	PARAM_CONFIG_DDNS ddnsConf;
    
	ret = ParamGetDdns( 0, &ddnsConf );
	if( ret != 0 )
    {
    	return -1;
    }
    
	g_ddnsFifoLock.Lock();

	ret = UnlinkFile( pathname );
	if( 0 != ret )
    {
    	SVPrint( "error:UnlinkFile(%s):%s!\r\n", pathname, STRERROR_ERRNO );
    }
	else
    {        
    	fddnsConf = Fopen( pathname, (char *)"w" );
    	if( NULL == fddnsConf )
        {
        	ret = -1;
        	SVPrint( "error:Fopen(%s):%s\r\n", pathname, STRERROR_ERRNO );
        }
    	else
        {
        	if( DDNS_VENDER_ORAY == ddnsConf.vender )
            {
            	FputsOray( fddnsConf, ddnsConf );
            }
        	else
            {
            	Fputs3322Org( fddnsConf, ddnsConf );
            }

        	Fclose( fddnsConf );
        }
    }
    
	g_ddnsFifoLock.Unlock();

	return ret;
}

