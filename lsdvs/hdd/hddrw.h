#ifndef __HDDRW_H__
#define __HDDRW_H__

#include "linuxFile.h"

int HddrwStatfs( const char *path, struct statfs *buf );
int HddrwLstat( const char *path, struct stat *buf );
int HddrwClosedir( DIR *dir );
int HddrwUnlink( const char *pathname );
int HddrwChmod( const char *path, unsigned int mode );
int HddrwRmdir( const char *pathname );
DIR *HddrwOpendir( const char *name );
struct dirent *HddrwReaddir( DIR *dir );
int HddrwMkdir( const char *pathname, uint mode );
int HddrwMount( const char *source, const char *target,
                 const char *filesystemtype, unsigned long mountflags,
                 const void *data );
int HddrwUmount( const char *target );
int HddrwOpen( const char *pathname, int flags );
int HddrwWrite( int fd, const void *buf, unsigned int count );
int HddrwRead( int fd, void *buf, unsigned int count );
void HddrwClose( int fd );
int HddrwFsync( int fd );

#endif // __HDDRW_H__

