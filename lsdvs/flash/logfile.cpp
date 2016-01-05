#include "logfile.h"
#include "mutex.h"

#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/mount.h>

#include "linuxFile.h"
#include "debug.h"


static ClMutexLock g_logFileLock;

int LogFileRead( char* pLogBuf, int nBufLen )
{
    int fd;
	int ret;

    if(NULL == pLogBuf || nBufLen <= 0)
    {
        return -1;
    }
    
	g_logFileLock.Lock();
	fd = Open( LOG_FILE, O_RDONLY );
	if( -1 == fd )
    {
    	SVPrint( "Open(%s) failed:%s!\r\n", LOG_FILE, STRERROR_ERRNO );        
    	g_logFileLock.Unlock();
    	return -1;
    }

	ret = Read( fd, pLogBuf, nBufLen );

	ret = ret > 0 ? 0 : -1;
    
	Close( fd );
	g_logFileLock.Unlock();
	return ret;    
}

int LogFileWrite( char* pLogBuf, int nBufLen )
{
    int fd;
	int ret;
    
	g_logFileLock.Lock();

	fd = Open( LOG_FILE, O_CREAT|O_WRONLY|O_TRUNC );
	if( -1 == fd )
    {
    	SVPrint( "Open(%s) failed:%s!\r\n", LOG_FILE, STRERROR_ERRNO );        
    	g_logFileLock.Unlock();
    	return -1;
    }

	ret = Write( fd, pLogBuf, nBufLen );

	ret = ret > 0 ? 0 : -1;

	Close( fd );
	g_logFileLock.Unlock();
	return ret; 
}


