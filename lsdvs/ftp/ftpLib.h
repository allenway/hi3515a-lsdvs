/***************************************************************************/
/*                                       */
/* ftplib.h - header file for callable ftp access routines                 */
/* Copyright (C) 1996, 1997 Thomas Pfau, pfau@cnj.digex.net                */
/*	73 Catherine Street, South Bound Brook, NJ, 08880	       */
/*                                       */
/* This library is free software; you can redistribute it and/or	   */
/* modify it under the terms of the GNU Library General Public	       */
/* License as published by the Free Software Foundation; either	       */
/* version       :   2 of the License, or (at your option) any later version       :  .       */
/*                                        */
/* This library is distributed in the hope that it will be useful,       */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of	   */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU	   */
/* Library General Public License for more details.               */
/*                                        */
/* You should have received a copy of the GNU Library General Public	   */
/* License along with this progam; if not, write to the	           */
/* Free Software Foundation, Inc., 59 Temple Place - Suite 330,           */
/* Boston, MA 02111-1307, USA.                           */
/*                                       */
/***************************************************************************/

#if !defined(__FTPLIB_H)
#define __FTPLIB_H

#if defined(__unix__) || defined(VMS)
#define GLOBALDEF
#define GLOBALREF extern
#elif defined(_WIN32)
#if defined BUILDING_LIBRARY
#define GLOBALDEF __declspec(dllexport)
#define GLOBALREF __declspec(dllexport)
#else
#define GLOBALREF __declspec(dllimport)
#endif
#endif

/* FtpAccess() type codes */
#define FTPLIB_DIR 1
#define FTPLIB_DIR_VERBOSE 2
#define FTPLIB_FILE_READ 3
#define FTPLIB_FILE_WRITE 4

/* FtpAccess() mode codes */
#define FTPLIB_ASCII 'A'
#define FTPLIB_IMAGE 'I'
#define FTPLIB_TEXT FTPLIB_ASCII
#define FTPLIB_BINARY FTPLIB_IMAGE

#ifdef __cplusplus
extern "C" {
#endif

struct NetBuf {
    char *cput,*cget;
    int handle;
    int cavail,cleft;
    char *buf;
    int dir;
    char response[256];
};

typedef struct NetBuf netbuf;

/* v1 compatibility stuff */
//netbuf *DefaultNetbuf;

//#define ftplib_lastresp FtpLastResponse(DefaultNetbuf)
#define ftpInit FtpInit
#define ftpOpen(x, m) FtpConnect(x, m)
#define ftpLogin(x,y, m) FtpLogin(x, y, (netbuf *)m)
#define ftpSite(x, m) FtpSite(x, (netbuf *)m)
#define ftpMkdir(x, m) FtpMkdir(x, (netbuf *)m)
#define ftpChdir(x, m) FtpChdir(x, (netbuf *)m)
#define ftpRmdir(x, m) FtpRmdir(x, (netbuf *)m)
#define ftpNlst(x, y, m) FtpNlst(x, y, (netbuf *)m)
#define ftpDir(x, y, m) FtpDir(x, y, (netbuf *)m)
#define ftpGet(x, y, z, m) FtpGet(x, y, z, (netbuf *)m)
#define ftpGet2(x, x2, y, z, m) FtpGet2(x, x2, y, z, (netbuf *)m)
#define ftpPut(x, y, z, m) FtpPut(x, y, z, (netbuf *)m)
#define ftpPut2(x, y, z, c, m) FtpPut2(x, y, z, c, (netbuf *)m)
#define ftpRename(x, y, m) FtpRename(x, y, (netbuf *)m)
#define ftpDelete(x, m) FtpDelete(x, (netbuf *)m)
#define ftpQuit(m) FtpQuit((netbuf *)m)
/* end v1 compatibility stuff */

GLOBALREF int ftplib_debug;
GLOBALREF void FtpInit(void);
GLOBALREF char *FtpLastResponse(netbuf *nControl);
GLOBALREF int FtpConnect(const char *host, netbuf **nControl);
GLOBALREF int FtpLogin(const char *user, const char *pass, netbuf *nControl);
GLOBALREF int FtpAccess(const char *path, int typ, int mode, netbuf *nControl,
    netbuf **nData);
GLOBALREF int FtpRead(void *buf, int max, netbuf *nData);
GLOBALREF int FtpWrite(void *buf, int len, netbuf *nData);
GLOBALREF int FtpClose(netbuf *nData);
GLOBALREF int FtpSite(const char *cmd, netbuf *nControl);
GLOBALREF int FtpMkdir(const char *path, netbuf *nControl);
GLOBALREF int FtpChdir(const char *path, netbuf *nControl);
GLOBALREF int FtpRmdir(const char *path, netbuf *nControl);
GLOBALREF int FtpNlst(const char *output, const char *path, netbuf *nControl);
GLOBALREF int FtpDir(const char *output, const char *path, netbuf *nControl);
GLOBALREF int FtpGet(const char *output, const char *path, char mode,
	netbuf *nControl);
GLOBALDEF int FtpGet2(char *local_buf, unsigned int *file_size, const char *path,
	char mode, netbuf *nControl);
GLOBALREF int FtpPut(const char *input, const char *path, char mode,
	netbuf *nControl);

typedef unsigned int (* packet_ts_T)(char *localfile, netbuf* p_nData);

GLOBALREF int register_packet_ts(packet_ts_T p_packet_ts);
GLOBALREF int FtpPut2(const char *input, const char *path, char mode
    , unsigned int *count,  netbuf *nControl);

GLOBALREF int FtpRename(const char *src, const char *dst, netbuf *nControl);
GLOBALREF int FtpDelete(const char *fnm, netbuf *nControl);
GLOBALREF void FtpQuit(netbuf *nControl);

int readresp(char c, netbuf *nControl);

#ifdef __cplusplus
};
#endif


#define MAX_UPDATE_FILE_SIZE (6*1024*1024)

#endif /* __FTPLIB_H */

