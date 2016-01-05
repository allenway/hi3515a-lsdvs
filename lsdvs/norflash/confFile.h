#ifndef __CONFFILE_H__
#define __CONFFILE_H__

#define CONFIG_FILE "/etc/dvs.conf"

int ConfFileRead( char *pBuf, int len );
int ConfFileWrite( char *pBuf, int len );

#endif // __CONFFILE_H__

