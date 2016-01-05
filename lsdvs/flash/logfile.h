#ifndef __LOG_FILE_H__
#define __LOG_FILE_H__

#define LOG_FILE "/etc/dvs.log"

int LogFileRead( char* pLogBuf, int nBufLen );
int LogFileWrite( char* pLogBuf, int nBufLen );



#endif


