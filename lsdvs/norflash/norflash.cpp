/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**	文件名: norflash.cpp
**  description  : 封装norflsh MTD接口
**  date           :  2013.12.03
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/

#include <stdio.h> 
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <mtd/mtd-user.h>
#include <getopt.h>

#include "debug.h"
#include "linuxFile.h"
#include "const.h"
#include "norflash.h"

int mtd_erase( const char *mtdname, unsigned int start, unsigned int size )
{
	int fd;
	struct mtd_info_user mtd;
	struct erase_info_user erase;
    
	fd = Open( mtdname, O_RDWR );
	if(fd < 0)
    {
    	FiPrint( "Open(%s) error:%s!", mtdname, STRERROR_ERRNO );
    	return FI_FAILED;
    }
	if( ioctl(fd,MEMGETINFO,&mtd) < 0 )
    {
    	FiPrint( "ioctl() error:%s!\r\n", STRERROR_ERRNO );    
    	Close( fd );
    	return FI_FAILED;
    }
	if( (start+size)>mtd.size || 0!=(start%mtd.erasesize) )
    {
    	FiPrint( "failed:start+size(%d))>mtd.size(%d) || start(%d)%mtd.erasesize(%d)!=0!\r\n",
                        	start+size,mtd.size,start,mtd.erasesize );
    	Close( fd );
    	return FI_FAILED;
    }
	erase.start     = start;
	erase.length     = (size + mtd.erasesize - 1) / mtd.erasesize;
	erase.length     *= mtd.erasesize;

	if( ioctl(fd,MEMERASE,&erase) < 0 )
    {
    	FiPrint( "ioctl()error:%s!", STRERROR_ERRNO );
    	Close( fd );
    	return FI_FAILED;
    }
	Close( fd );
	return FI_SUCCESSFUL;    
}

int mtd_write( const char *mtdname, unsigned int start, unsigned int size, char *buffer )
{    
	int fd;
	int result;        
	struct mtd_info_user mtd;
    
	fd = Open( mtdname, O_RDWR );
	if(fd < 0)
    {
    	FiPrint( "Open(%s) error:%s!", mtdname, STRERROR_ERRNO );
    	return FI_FAILED;
    }
	if( ioctl(fd,MEMGETINFO,&mtd) < 0 )
    {
    	FiPrint( "ioctl() error:%s!\r\n", STRERROR_ERRNO );
    	Close( fd );
    	return FI_FAILED;
    }
	if((start+size)>mtd.size || 0!=(start%mtd.erasesize))
    {
    	FiPrint( "failed:start+size(%d))>mtd.size(%d) || start(%d)%mtd.erasesize(%d)!=0!\r\n",
                        	start+size,mtd.size,start,mtd.erasesize );
    	Close( fd );
    	return FI_FAILED;
    }
    
	if( lseek(fd, start, SEEK_SET) < 0 )
    {
    	FiPrint("lseek() error:%s!",STRERROR_ERRNO);
    	Close(fd);
    	return FI_FAILED;
    }    
	result = write( fd, buffer, size );
	if( result<0 || (unsigned int)result != size )
    {
    	FiPrint( "failed:result != size!" );
    	Close( fd );
    	return FI_FAILED;
    }
    
	Close(fd);
	return FI_SUCCESSFUL;    
}

int mtd_read( const char *mtdname, unsigned int start, unsigned int size, char *buffer )
{    
	int fd;
	int result;     
//	struct mtd_info_user mtd;
    
	fd = Open( mtdname, O_RDONLY );
	if( fd < 0 )
    {
    	FiPrint( "Open(%s) error:%s!", mtdname, STRERROR_ERRNO );
    	return FI_FAILED;
    }
#if 0
	if( ioctl( fd, MEMGETINFO, &mtd ) < 0 )
    {
    	FiPrint("ioctl() error:%s!\r\n",STRERROR_ERRNO);    
    	Close( fd );
    	return FI_FAILED;
    }
	if( (start+size)>mtd.size || 0!=(start%mtd.erasesize) )
    {
    	FiPrint( "failed:start+size(%d))>mtd.size(%d) || start(%d)%mtd.erasesize(%d)!=0!\r\n",
                        	start+size,mtd.size,start,mtd.erasesize );
    	Close( fd );
    	return FI_FAILED;
    }
#endif
	if(lseek(fd,start,SEEK_SET) < 0)
    {
    	FiPrint("lseek() error:%s!",STRERROR_ERRNO);
    	Close(fd);
    	return FI_FAILED;
    }    
	result = Readn( fd, buffer, size );
	if(result<0 || (unsigned int)result != size)
    {
    	FiPrint( "failed:result != size!" );
    	Close( fd );
    	return FI_FAILED;
    }
    
	Close( fd );
	return FI_SUCCESSFUL;    
}

