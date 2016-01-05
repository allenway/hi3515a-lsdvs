#ifndef __FILE_FTP_H__
#define   __FILE_FTP_H__

#include "ftpLib.h"

int FtpFileInit( const char *host, const char *user, const char *pass, netbuf **nControl );
int FtpFileQuit( netbuf *nControl );
int FtpFileOpen( const char *path, int typ, netbuf *nControl, netbuf **nData, int dirChangFlag );
int FtpFileWrite( void *buf, int len, netbuf *nData );
int FtpFileRead( void *buf, int max, netbuf *nData );
int FtpFileClose( netbuf *nControl, netbuf *nData );

#endif

