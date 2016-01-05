/***************************************************************************/
/*                                       */
/* ftplib.c - callable ftp access routines	               */
/* Copyright (C) 1996, 1997 Thomas Pfau, pfau@cnj.digex.net	       */
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
/*                                        */
/***************************************************************************/

#if defined(__unix__) || defined(__VMS)
#include <unistd.h>
#endif
#if defined(_WIN32)
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#if defined(__unix__)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#elif defined(VMS)
#include <types.h>
#include <socket.h>
#include <in.h>
#include <netdb.h>
#include <inet.h>
#elif defined(_WIN32)
#include <winsock.h>
#endif
#include <fcntl.h>

#include "debug.h"
#include "ftpLib.h"

#define BUILDING_LIBRARY

#if defined(_WIN32)
#define SETSOCKOPT_OPTVAL_TYPE (const char *)
#else
#define SETSOCKOPT_OPTVAL_TYPE (void *)
#endif

#define FTPLIB_BUFSIZ 8192
#define ACCEPT_TIMEOUT 30

#define FTPLIB_CONTROL 0
#define FTPLIB_READ 1
#define FTPLIB_WRITE 2

//static char *version       :   ="ftplib Release 3 12/xx/97, copyright 1996, 1997 Thomas Pfau";

GLOBALDEF int ftplib_debug = 0;

#if defined(__unix__) || defined(VMS)
#define net_read read
#define net_write write
#define net_close close
#elif defined(_WIN32)
#define net_read(x,y,z) recv(x,y,z,0)
#define net_write(x,y,z) send(x,y,z,0)
#define net_close closesocket
#endif

#if defined(VMS)
/*
 * VAX C does not supply a memccpy routine so I provide my own
 */
void *memccpy(void *dest, const void *src, int c, size_t n)
{
    int i=0;
    const unsigned char *ip=src;
    unsigned char *op=dest;

    while (i < n)
    {
	if ((*op++ = *ip++) == c)
        break;
	i++;
    }
    if (i == n)
	return NULL;
    return op;
}
#endif

/*
 * read a line of text
 *
 * return -1 on error or bytecount
 */
static int readline(char *buf,int max,netbuf *ctl)
{
    int x,retval = 0;
    char *end,*bp=buf;
    int eof = 0;

    if ((ctl->dir != FTPLIB_CONTROL) && (ctl->dir != FTPLIB_READ))
	return -1;
    if (max == 0)
	return 0;
    do
    {
    	if (ctl->cavail > 0)
        {
        x = (max >= ctl->cavail) ? ctl->cavail : max-1;
        end = (char *)memccpy(bp,ctl->cget,'\n',x);
        if (end != NULL)
    	x = end - bp;
        retval += x;
        bp += x;
        *bp = '\0';
        max -= x;
        ctl->cget += x;
        ctl->cavail -= x;
        if (end != NULL)
        {
    	bp -= 2;
    	if (strcmp(bp,"\r\n") == 0)
        {
            *bp++ = '\n';
            *bp++ = '\0';
            --retval;
        }
        	break;
        }
        }
    	if (max == 1)
        {
        *buf = '\0';
        break;
        }
    	if (ctl->cput == ctl->cget)
        {
        ctl->cput = ctl->cget = ctl->buf;
        ctl->cavail = 0;
        ctl->cleft = FTPLIB_BUFSIZ;
        }
	if (eof)
    {
        if (retval == 0)
    	retval = -1;
        break;
    }
    	if ((x = net_read(ctl->handle,ctl->cput,ctl->cleft)) == -1)
        {
        perror("read");
        retval = -1;
        break;
        }
	if (x == 0)
        eof = 1;
    	ctl->cleft -= x;
    	ctl->cavail += x;
    	ctl->cput += x;
    }
    while (1);
    return retval;
}

/*
 * write lines of text
 *
 * return -1 on error or bytecount
 */
static int writeline(char *buf, int len, netbuf *nData)
{
    int x, nb=0, w;
    char *ubp = buf, *nbp;
    char lc=0;

    if (nData->dir != FTPLIB_WRITE)
	return -1;
    nbp = nData->buf;
    for (x=0; x < len; x++)
    {
	if ((*ubp == '\n') && (lc != '\r'))
    {
        if (nb == FTPLIB_BUFSIZ)
        {
    	w = net_write(nData->handle, nbp, FTPLIB_BUFSIZ);
    	if (w != FTPLIB_BUFSIZ)
        {
            SVPrint("net_write(1) returned %d, errno = %d\n", w, errno);
            return(-1);
        }
    	nb = 0;
        }
        nbp[nb++] = '\r';
    }
	if (nb == FTPLIB_BUFSIZ)
    {
        w = net_write(nData->handle, nbp, FTPLIB_BUFSIZ);
        if (w != FTPLIB_BUFSIZ)
        {
    	SVPrint("net_write(2) returned %d, errno = %d\n", w, errno);
    	return(-1);
        }
        nb = 0;
    }
	nbp[nb++] = lc = *ubp++;
    }
    if (nb)
    {
	w = net_write(nData->handle, nbp, nb);
	if (w != nb)
    {
        SVPrint("net_write(3) returned %d, errno = %d\n", w, errno);
        return(-1);
    }
    }
    return len;
}

/*
 * read a response from the server
 *
 * return 0 if first char doesn't match
 * return 1 if first char matches
 */
//static int readresp(char c, netbuf *nControl)
int readresp(char c, netbuf *nControl)
{
    char match[5];
    if (readline(nControl->response,256,nControl) == -1)
    {
	perror("Control socket read failed");
	return 0;
    }
    if (ftplib_debug > 1)
	fprintf(stderr,"%s",nControl->response);
    if (nControl->response[3] == '-')
    {
	strncpy(match,nControl->response,3);
	match[3] = ' ';
	match[4] = '\0';
	do
    {
        if (readline(nControl->response,256,nControl) == -1)
        {
    	perror("Control socket read failed");
    	return 0;
        }
        if (ftplib_debug > 1)
    	fprintf(stderr,"%s",nControl->response);
    }
	while (strncmp(nControl->response,match,4));
    }
    if (nControl->response[0] == c)
	return 1;
    return 0;
}

/*
 * FtpInit for stupid operating systems that require it (Windows NT)
 */
GLOBALDEF void FtpInit(void)
{
#if defined(_WIN32)
    WORD wVersionRequested;
    WSADATA wsadata;
    int err;
    wVersionRequested = MAKEWORD(1,1);
    if ((err = WSAStartup(wVersionRequested,&wsadata)) != 0)
	fprintf(stderr,"Network failed to start: %d\n",err);
#endif
}

/*
 * FtpLastResponse - return a pointer to the last response received
 */
GLOBALDEF char *FtpLastResponse(netbuf *nControl)
{
    if ((nControl) && (nControl->dir == FTPLIB_CONTROL))
    	return nControl->response;
    return NULL;
}

/*
 * FtpConnect - connect to remote server
 *
 * return 1 if connected, 0 if not
 */

#if 1	//by maohw

/*
¹¦ÄÜ:
	NVSÁ¬½Ó×¢²á·þÎñÆ÷
²ÎÊý:
	sockfd :	·¢ËÍÊý¾ÝµÄÌ×½Ó×Ö,ÒÑ¾­ÓëVAP¶Ë¿Ú°ó¶¨ÁË
	saptr   :	CMGÉÏÃæ×¢²á·þÎñÆ÷µÄµØÖ·ºÍ¶Ë¿ÚºÅ
	salen   :	µØÖ·³¤¶È
	nsec	    :	³¬Ê±Ê±¼äå?å?å?
·µ»ØÖµ:
    -1:Á¬½ÓÊ§°Ü;   0-Á¬½Ó³É¹¦
*/
int sbell_connect_nonb(int sockfd, const struct sockaddr *saptr, socklen_t salen, int nsec)
{
	int	        	flags, n, error;
	socklen_t		len;
	fd_set	    	rset, wset;
	struct timeval	tval;
    //if(sockfd<0)
    //	return -1;

	flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

	error = 0;
	if ( (n = connect(sockfd, (struct sockaddr *) saptr, salen)) < 0)  /*Á¬½Ó×¢²á·þÎñÆ÷*/
    	if (errno != EINPROGRESS)
        	return(-1);

    /* Do whatever we want while the connect is taking place. */

	if (n == 0)
    	goto done;    /* connect completed immediately */

	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);
	wset = rset;
	tval.tv_sec = nsec;
	tval.tv_usec = 0;

	if ( (n = select(sockfd+1, &rset, &wset, NULL,
                     nsec ? &tval : NULL)) == 0) {
        //close(sockfd);        /* timeout */
    	errno = ETIMEDOUT;
    	return(-1);
    }

	if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) {
    	len = sizeof(error);
    	if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
        	return(-1);            /* Solaris pending error */
    } else
    	SVPrint("select error: sockfd not set");

done:
	fcntl(sockfd, F_SETFL, flags);    /* restore file status flags */

	if (error) {
        //close(sockfd);        /* just in case */
    	errno = error;
    	return(-1);
    }
	return(0);
}
#endif

GLOBALDEF int FtpConnect(const char *host, netbuf **nControl)
{
    int sControl;
    struct sockaddr_in sin;
    struct hostent *phe;
    struct servent *pse;
    int on=1;
    netbuf *ctrl;
    char *lhost;
    char *pnum;

    //ql add
	struct timeval tv;
    // int send_buf_size = 128*1024;
	tv.tv_sec = 30;
	tv.tv_usec = 0;

    memset(&sin,0,sizeof(sin));
    sin.sin_family = AF_INET;
    lhost = strdup(host);
    pnum = strchr(lhost,':');
    if (pnum == NULL)
    {
#if defined(VMS)
    	sin.sin_port = htons(21);
#else
    	if ((pse = getservbyname("ftp","tcp")) == NULL)
        {
        perror("getservbyname");
        return 0;
        }
    	sin.sin_port = pse->s_port;
#endif
    }
    else
    {
    *pnum++ = '\0';
	if (isdigit(*pnum))
        sin.sin_port = htons(atoi(pnum));
	else
    {
        pse = getservbyname(pnum,"tcp");
        sin.sin_port = pse->s_port;
    }
    }
    if ((sin.sin_addr.s_addr = inet_addr(lhost)) == (unsigned int)-1)
    {
    	if ((phe = gethostbyname(lhost)) == NULL)
        {
        perror("gethostbyname");
        return 0;
        }
    	memcpy((char *)&sin.sin_addr, phe->h_addr, phe->h_length);
    }
    free(lhost);
    sControl = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sControl == -1)
    {
	perror("socket");
	return 0;
    }
    if (setsockopt(sControl,SOL_SOCKET,SO_REUSEADDR,
           SETSOCKOPT_OPTVAL_TYPE &on, sizeof(on)) == -1)
    {
	perror("setsockopt");
	net_close(sControl);
	return 0;
    }       
//·¢ËÍÊ±ÏÞ
if(setsockopt(sControl, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv))==-1)
{
	SVPrint("setsockopt set send timeout error\n");
	net_close(sControl);
	return -1;
}
//½ÓÊÕÊ±ÏÞ
if(setsockopt(sControl, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))==-1)
{
	SVPrint("setsockopt set rvc timeout error\n");
	net_close(sControl);
	return -1;
}
#if 0
//·¢ËÍ»º³åÇø
if(setsockopt(sControl, SOL_SOCKET, SO_SNDBUF, &send_buf_size, sizeof(int))==-1)
{
	SVPrint("setsockopt set send buffer error\n");
	net_close(sControl);
	return -1;
}
#endif
//   if (connect(sControl, (struct sockaddr *)&sin, sizeof(sin)) == -1)
  if (sbell_connect_nonb(sControl, (struct sockaddr *)&sin, sizeof(sin),8) == -1)    
    {
	perror("connect");
	net_close(sControl);
	return 0;
    }
    ctrl = (netbuf *)calloc(1,sizeof(netbuf));
    if (ctrl == NULL)
    {
	perror("calloc");
	net_close(sControl);
	return 0;
    }
    ctrl->buf = (char *)malloc(FTPLIB_BUFSIZ);
    if (ctrl->buf == NULL)
    {
	perror("calloc");
	net_close(sControl);
	free(ctrl);
	return 0;
    }
    ctrl->handle = sControl;
    ctrl->dir = FTPLIB_CONTROL;
    if (readresp('2', ctrl) == 0)
    {
	net_close(sControl);
	free(ctrl->buf);
	free(ctrl);
	return 0;
    }
    *nControl = ctrl;
    return 1;
}

/*
 * FtpSendCmd - send a command and wait for expected response
 *
 * return 1 if proper response received, 0 otherwise
 */
static int FtpSendCmd(const char *cmd, char expresp, netbuf *nControl)
{
    char buf[256];
    if (nControl->dir != FTPLIB_CONTROL)
	return 0;
    if (ftplib_debug > 2)
	fprintf(stderr,"%s\n",cmd);
    sprintf(buf,"%s\r\n",cmd);
    if (net_write(nControl->handle,buf,strlen(buf)) <= 0)
    {
	perror("write");
	return 0;
    }    
    return readresp(expresp, nControl);
}

/*
 * FtpLogin - log in to remote server
 *
 * return 1 if logged in, 0 otherwise
 */
GLOBALDEF int FtpLogin(const char *user, const char *pass, netbuf *nControl)
{
    char tempbuf[64];

    sprintf(tempbuf,"USER %s",user);
    if (!FtpSendCmd(tempbuf,'3',nControl))
    {
	if (nControl->response[0] == '2')
        return 1;
	return 0;
    }
    sprintf(tempbuf,"PASS %s",pass);
    return FtpSendCmd(tempbuf,'2',nControl);
}

/*
 * FtpOpenPort - set up data connection
 *
 * return 1 if successful, 0 otherwise
 */
static int FtpOpenPort(netbuf *nControl, netbuf **nData, int mode, int dir)
{
    int sData;
    union {
	struct sockaddr sa;
	struct sockaddr_in in;
    } sin;
    struct linger lng = { 0, 0 };
    int l;
    int on=1;
    char *cp;
    unsigned int v[6];
    netbuf *ctrl;
//ql add
	struct timeval tv;
//	int send_buf_size = 128*1024;
	tv.tv_sec = 30;
	tv.tv_usec = 0;

    if (nControl->dir != FTPLIB_CONTROL)
	return -1;
    if ((dir != FTPLIB_READ) && (dir != FTPLIB_WRITE))
    {
	sprintf(nControl->response, "Invalid direction %d\n", dir);
	return -1;
    }
    if ((mode != FTPLIB_ASCII) && (mode != FTPLIB_IMAGE))
    {
	sprintf(nControl->response, "Invalid mode %c\n", mode);
	return -1;
    }
    l = sizeof(sin);
    memset(&sin, 0, l);
    sin.in.sin_family = AF_INET;
    if (!FtpSendCmd("PASV",'2',nControl))
	return -1;
    cp = strchr(nControl->response,'(');
    if (cp == NULL)
	return -1;
    cp++;
    sscanf(cp,"%u,%u,%u,%u,%u,%u",&v[2],&v[3],&v[4],&v[5],&v[0],&v[1]);
    sin.sa.sa_data[2] = v[2];
    sin.sa.sa_data[3] = v[3];
    sin.sa.sa_data[4] = v[4];
    sin.sa.sa_data[5] = v[5];
    sin.sa.sa_data[0] = v[0];
    sin.sa.sa_data[1] = v[1];
    sData = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (sData == -1)
    {
	perror("socket");
	return -1;
    }
    if (setsockopt(sData,SOL_SOCKET,SO_REUSEADDR,
           SETSOCKOPT_OPTVAL_TYPE &on,sizeof(on)) == -1)
    {
	perror("setsockopt");
	net_close(sData);
	return -1;
    }
    if (setsockopt(sData,SOL_SOCKET,SO_LINGER,
           SETSOCKOPT_OPTVAL_TYPE &lng,sizeof(lng)) == -1)
    {
	perror("setsockopt");
	net_close(sData);
	return -1;
    }

//·¢ËÍÊ±ÏÞ
if(setsockopt(sData, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv))==-1)
{
	SVPrint("setsockopt set send timeout error\n");
	net_close(sData);
	return -1;
}
//½ÓÊÕÊ±ÏÞ
if(setsockopt(sData, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))==-1)
{
	SVPrint("setsockopt set rvc timeout error\n");
	net_close(sData);
	return -1;
}
//·¢ËÍ»º³åÇø
#if 0
if(setsockopt(sData, SOL_SOCKET, SO_SNDBUF, &send_buf_size, sizeof(int))==-1)
{
	SVPrint("setsockopt set send buffer error\n");
	net_close(sData);
	return -1;
}
#endif
//   if (connect(sData, &sin.sa, sizeof(sin.sa)) == -1)    
   if (sbell_connect_nonb(sData, &sin.sa, sizeof(sin.sa),8) == -1)
    {
	perror("connect");
	net_close(sData);
	return -1;
    }
    ctrl = (netbuf *)calloc(1,sizeof(netbuf));
    if (ctrl == NULL)
    {
	perror("calloc");
	net_close(sData);
	return -1;
    }
    if ((mode == 'A') && ((ctrl->buf = (char *)malloc(FTPLIB_BUFSIZ)) == NULL))
    {
	perror("calloc");
	net_close(sData);
	free(ctrl);
	return -1;
    }
    ctrl->handle = sData;
    ctrl->dir = dir;
    *nData = ctrl;
    return 1;
}

/*
 * FtpAccess - return a handle for a data stream
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpAccess(const char *path, int typ, int mode, netbuf *nControl,
    netbuf **nData)
{
    char buf[256];
    int dir;
    if ((path == NULL) && ((typ == FTPLIB_FILE_WRITE) || (typ == FTPLIB_FILE_READ)))
    {
	sprintf(nControl->response, "Missing path argument for file transfer\n");
	return 0;
    }
    sprintf(buf, "TYPE %c", mode);
    if (!FtpSendCmd(buf, '2', nControl))
	return 0;
    switch (typ)
    {
      case FTPLIB_DIR:
    	strcpy(buf,"NLST");
	dir = FTPLIB_READ;
	break;
      case FTPLIB_DIR_VERBOSE:
	strcpy(buf,"LIST");
	dir = FTPLIB_READ;
	break;
      case FTPLIB_FILE_READ:
	strcpy(buf,"RETR");
	dir = FTPLIB_READ;
	break;
      case FTPLIB_FILE_WRITE:
	strcpy(buf,"STOR");
	dir = FTPLIB_WRITE;
	break;
      default:
	sprintf(nControl->response, "Invalid open type %d\n", typ);
	return 0;
    }
    if (path != NULL)
	sprintf(buf+strlen(buf)," %s",path);
    if (FtpOpenPort(nControl, nData, mode, dir) == -1)
	return 0;
    if (!FtpSendCmd(buf, '1', nControl))
    {
	FtpClose(*nData);
    *nData = NULL;
	return 0;
    }
    return 1;
}

/*
 * FtpRead - read from a data connection
 */
GLOBALDEF int FtpRead(void *buf, int max, netbuf *nData)
{
    if (nData->dir != FTPLIB_READ)
	return 0;
    if (nData->buf)
    	return readline((char *)buf, max, nData);
    return net_read(nData->handle, buf, max);
}

/*
 * FtpWrite - write to a data connection
 */
GLOBALDEF int FtpWrite(void *buf, int len, netbuf *nData)
{
    if (nData->dir != FTPLIB_WRITE)
	return 0;
    if (nData->buf)
    	return writeline((char *)buf, len, nData);
    return net_write(nData->handle, buf, len);
}

/*
 * FtpClose - close a data connection
 ÌØ±ð×¢Òâ: sven Íù ´Ëº¯ÊýÖÐÌí¼ÓÁË free(nData),·ÀÖ¹ÄÚ´æÐ¹Â¶,
           µ«Ê¹ÓÃµÄÊ±ºònData ±ØÐëÊÇÖ¸Ïò¶Ñ(malloc)µÄÄÚ´æ¡£
 */
GLOBALDEF int FtpClose(netbuf *nData)
{
	if( NULL == nData )
    {
    	return -1;
    }
    if (nData->dir == FTPLIB_WRITE)
    {
    	if (nData->buf != NULL)
        	writeline(NULL, 0, nData);
    }
    else if (nData->dir != FTPLIB_READ)
    {
    	SVPrint( "%s:nData->dir = %d\r\n", __FUNCTION__, nData->dir );
    	return 0;
    }
    if (nData->buf)
    	free(nData->buf);
    shutdown(nData->handle,2);
    net_close(nData->handle);
    free(nData); // sven add
    return 1;
}

/*
 * FtpSite - send a SITE command
 *
 * return 1 if command successful, 0 otherwise
 */
GLOBALDEF int FtpSite(const char *cmd, netbuf *nControl)
{
    char buf[256];
    sprintf(buf,"SITE %s",cmd);
    if (!FtpSendCmd(buf,'2',nControl))
	return 0;
    return 1;
}

/*
 * FtpMkdir - create a directory at server
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpMkdir(const char *path, netbuf *nControl)
{
    char buf[256];
    sprintf(buf,"MKD %s",path);
    if (!FtpSendCmd(buf,'2', nControl))
	return 0;
    return 1;
}

/*
 * FtpChdir - change path at remote
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpChdir(const char *path, netbuf *nControl)
{
    char buf[256];
    sprintf(buf,"CWD %s",path);
    if (!FtpSendCmd(buf,'2',nControl))
	return 0;
    return 1;
}

/*
 * FtpRmdir - remove directory at remote
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpRmdir(const char *path, netbuf *nControl)
{
    char buf[256];
    sprintf(buf,"RMD %s",path);
    if (!FtpSendCmd(buf,'2',nControl))
	return 0;
    return 1;
}

/*
 * FtpXfer - issue a command and transfer data
 *
 * return 1 if successful, 0 otherwise
 */
#if 1
static int FtpXfer(const char *localfile, const char *path,
	netbuf *nControl, int typ, int mode)
{
    int l,c;
    char *dbuf;
    FILE *local = NULL;
    netbuf *nData;

    if (localfile != NULL)
    {
	local = fopen(localfile, (typ == FTPLIB_FILE_WRITE) ? "r" : "w");
	if (local == NULL)
    {
        strcpy(nControl->response, strerror(errno));
        return 0;
    }
    }
    if (local == NULL)
	local = (typ == FTPLIB_FILE_WRITE) ? stdin : stdout;
    if (!FtpAccess(path, typ, mode, nControl, &nData))
	return 0;
    dbuf = (char *)malloc(FTPLIB_BUFSIZ);
    if (typ == FTPLIB_FILE_WRITE)
    {
	while ((l = fread(dbuf, 1, FTPLIB_BUFSIZ, local)) > 0)
        if ((c = FtpWrite(dbuf, l, nData)) < l)
    	SVPrint("short write: passed %d, wrote %d\n", l, c);
    }
    else
    {
    	while ((l = FtpRead(dbuf, FTPLIB_BUFSIZ, nData)) > 0)
        if (fwrite(dbuf, 1, l, local) <= 0)
        {
    	perror("localfile write");
    	break;
        }
    }
    fflush(local);
    if (localfile != NULL)
	fclose(local);
    FtpClose(nData);
    free(dbuf);
    return readresp('2', nControl);
}
#endif
static int FtpXfer2(char *local_buf, unsigned int *file_size, const char *path,
	netbuf *nControl, int typ, int mode)
{
    int l,c;
    unsigned int count = *file_size;
    char *dbuf;
    netbuf *nData;
    
    if (local_buf == NULL)
    {
        SVPrint("local_buf == NULL.\n");
        return 0;
    }
    if (!FtpAccess(path, typ, mode, nControl, &nData))
    {
        SVPrint(" FtpAccess error.\n");
        return 0;
    }
    dbuf = (char *)malloc(FTPLIB_BUFSIZ);

    if (typ == FTPLIB_FILE_WRITE)
    {
        while(1)
        {
        	l = (count > FTPLIB_BUFSIZ)? FTPLIB_BUFSIZ : count;
    	if(l<=0)
        	break;
    	memcpy(dbuf, local_buf,l);
    	local_buf += l;
        	if ((c = FtpWrite(dbuf, l, nData)) < l)
        	SVPrint("short write: passed %d, wrote %d\n", l, c);
        	if(l != FTPLIB_BUFSIZ)
        	break;
    	count -= l;
        }
    }
    else
    {
    	while ((l = FtpRead(dbuf, FTPLIB_BUFSIZ, nData)) > 0)
    {
    	memcpy(local_buf,dbuf,l);
        (local_buf) +=l;
        	count += l;

    	if (count > MAX_UPDATE_FILE_SIZE)
        {
        	SVPrint("update file size %d > %d\n", count, MAX_UPDATE_FILE_SIZE);
        	count = 0;    
        	break;
        }
    }
    *file_size = count;
    }
    FtpClose(nData);
    free(dbuf);
    return readresp('2', nControl);
}

static packet_ts_T g_packet_ts = NULL;

int register_packet_ts(packet_ts_T p_packet_ts)
{
	g_packet_ts = p_packet_ts;
	return 0;
}

#if 0
static int FtpXfer3(const char *localfile, const char *path,
	netbuf *nControl, int typ, int mode, unsigned int *count)
{
	FILE *local = NULL;
	netbuf *nData;

    if (localfile != NULL)
    {
	local = fopen(localfile, (typ == FTPLIB_FILE_WRITE) ? "r" : "w");
	if (local == NULL)
    {
        strcpy(nControl->response, strerror(errno));
        return 0;
    }
    }
  
    if (localfile != NULL)
	fclose(local);

   
    if (!FtpAccess(path, typ, mode, nControl, &nData))
	return 0;
    
    
    if (typ == FTPLIB_FILE_WRITE)
    {
    	if(g_packet_ts != NULL)
    {
        *count = g_packet_ts(localfile, nData);
    }
    }
    else
    {
    }
    
    FtpClose(nData);
       
    return readresp('2', nControl);
}

#endif

/*
 * FtpNlst - issue an NLST command and write response to output
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpNlst(const char *outputfile, const char *path,
	netbuf *nControl)
{
    return FtpXfer(outputfile, path, nControl, FTPLIB_DIR, FTPLIB_ASCII);
}

/*
 * FtpDir - issue a LIST command and write response to output
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpDir(const char *outputfile, const char *path, netbuf *nControl)
{
    return FtpXfer(outputfile, path, nControl, FTPLIB_DIR_VERBOSE, FTPLIB_ASCII);
}

/*
 * FtpGet - issue a GET command and write received data to output
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpGet(const char *outputfile, const char *path,
	char mode, netbuf *nControl)
{
    return FtpXfer(outputfile, path, nControl, FTPLIB_FILE_READ, mode);
}

GLOBALDEF int FtpGet2(char *local_buf, unsigned int *file_size, const char *path,
	char mode, netbuf *nControl)
{
    return FtpXfer2(local_buf, file_size, path, nControl, FTPLIB_FILE_READ, mode);
}

/*
 * FtpPut - issue a PUT command and send data from input
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpPut(const char *inputfile, const char *path, char mode,
	netbuf *nControl)
{
    return FtpXfer(inputfile, path, nControl, FTPLIB_FILE_WRITE, mode);
}

GLOBALDEF int FtpPut2(const char *inputfile, const char *path, char mode
    , unsigned int *count, netbuf *nControl)
{
    return FtpXfer2((char *)inputfile, count, path, nControl, FTPLIB_FILE_WRITE, mode);
}

/*
 * FtpRename - rename a file at remote
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpRename(const char *src, const char *dst, netbuf *nControl)
{
    char cmd[256];
    sprintf(cmd,"RNFR %s",src);
    if (!FtpSendCmd(cmd,'3',nControl))
	return 0;
    sprintf(cmd,"RNTO %s",dst);
    if (!FtpSendCmd(cmd,'2',nControl))
	return 0;
    return 1;
}

/*
 * FtpDelete - delete a file at remote
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF int FtpDelete(const char *fnm, netbuf *nControl)
{
    char cmd[256];
    sprintf(cmd,"DELE %s",fnm);
    if (!FtpSendCmd(cmd,'2', nControl))
	return 0;
    return 1;
}

/*
 * FtpQuit - disconnect from remote
 *
 * return 1 if successful, 0 otherwise
 */
GLOBALDEF void FtpQuit(netbuf *nControl)
{
    if (nControl->dir != FTPLIB_CONTROL)
	return;
    FtpSendCmd("QUIT",'2',nControl);
    net_close(nControl->handle);
    free(nControl->buf);
    free(nControl);
}

