/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 报警
********************************************************************************/
#include "debug.h"
#include "linuxFile.h"
#include "mutex.h"
#include "confFile.h"

static ClMutexLock g_confFileLock;
int ConfFileRead( char *pBuf, int len )
{
	int fd;
	int ret;
	g_confFileLock.Lock();
	fd = Open( CONFIG_FILE, O_RDONLY );
	if( -1 == fd )
    {
    	ERRORPRINT( "Open(%s) failed:%s!\r\n", CONFIG_FILE, STRERROR_ERRNO );        
    	g_confFileLock.Unlock();
    	return -1;
    }

	ret = Read( fd, pBuf, len );

	ret = ret > 0 ? 0 : -1;
    
	Close( fd );
	g_confFileLock.Unlock();
	return ret;    
}

int ConfFileWrite( char *pBuf, int len )
{
	int fd;
	int ret;
    
	g_confFileLock.Lock();

	fd = Open( CONFIG_FILE, O_CREAT|O_WRONLY|O_TRUNC );
	if( -1 == fd )
    {
    	ERRORPRINT( "Open(%s) failed:%s!\r\n", CONFIG_FILE, STRERROR_ERRNO );        
    	g_confFileLock.Unlock();
    	return -1;
    }

	ret = Write( fd, pBuf, len );

	ret = ret > 0 ? 0 : -1;

	Close( fd );
	g_confFileLock.Unlock();
	return ret; 
}


