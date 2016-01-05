/*
*******************************************************************************
**	Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**	author        :  sven
**	version       :  v1.0
**	date           :  2013.10.10
**	description  : 对操作hdd 的接口进行一个封装
*******************************************************************************
*/

#include "linuxFile.h"
#include "hddLock.h"

int HddrwStatfs( const char *path, struct statfs *buf )
{
	int ret;
    
	HddMutexLock();
	ret = Statfs( path, buf );
	HddMutexUnlock();

	return ret;
}

int HddrwLstat( const char *path, struct stat *buf )
{
	int ret;
    
	HddMutexLock();
	ret = Lstat( path, buf );
	HddMutexUnlock();

	return ret;
}

int HddrwClosedir( DIR *dir )
{
	int ret;
    
	HddMutexLock();
	ret = Closedir( dir );
	HddMutexUnlock();

	return ret;
}

int HddrwUnlink( const char *pathname )
{
	int ret;
    
	HddMutexLock();
	ret = Unlink( pathname );
	HddMutexUnlock();

	return ret;
}

int HddrwChmod( const char *path, unsigned int mode )
{
	int ret;
    
	HddMutexLock();
	ret = Chmod( path, mode );
	HddMutexUnlock();

	return ret;
}

int HddrwRmdir( const char *pathname )
{
	int ret;
    
	HddMutexLock();
	ret = Rmdir( pathname );
	HddMutexUnlock();

	return ret;
}

DIR *HddrwOpendir( const char *name )
{
	DIR * retDir;
    
	HddMutexLock();
	retDir = Opendir( name );
	HddMutexUnlock();

	return retDir;
}

struct dirent *HddrwReaddir( DIR *dir )
{
	struct dirent * retDirent;
    
	HddMutexLock();
	retDirent = Readdir( dir );
	HddMutexUnlock();

	return retDirent;
}

int HddrwMkdir( const char *pathname, uint mode )
{
	int ret;
    
	HddMutexLock();
	ret = Mkdir( pathname, mode );
	HddMutexUnlock();

	return ret;
}


int HddrwMount( const char *source, const char *target,
                 const char *filesystemtype, unsigned long mountflags,
                 const void *data )
{
	int ret;
    
	HddMutexLock();
	ret = Mount(source, target, filesystemtype, mountflags, data );
	HddMutexUnlock();

	return ret;
}


int HddrwUmount( const char *target )
{
	int ret;
    
	HddMutexLock();
	ret = Umount( target );
	HddMutexUnlock();

	return ret;
}

int HddrwOpen( const char *pathname, int flags )
{
	int ret;
    
	HddMutexLock();
	ret = Open( pathname, flags );
	HddMutexUnlock();

	return ret;
}


int HddrwWrite( int fd, const void *buf, unsigned int count )
{
	int ret;
    
	HddMutexLock();
	ret = Write( fd, buf, count );
	HddMutexUnlock();

	return ret;
}

int HddrwRead( int fd, void *buf, unsigned int count )
{
	int ret;
    
	HddMutexLock();
	ret = Read( fd, buf, count );
	HddMutexUnlock();

	return ret;
}

void HddrwClose( int fd )
{    
	HddMutexLock();
	Close( fd );
	HddMutexUnlock();
}

int HddrwFsync( int fd )
{
	int ret;
	HddMutexLock();
	ret = Fsync( fd );
	HddMutexUnlock();

	return ret;
}


