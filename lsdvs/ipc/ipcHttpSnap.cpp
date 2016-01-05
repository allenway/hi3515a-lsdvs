/*********************************************************************
*filename: httpclient.c
*purpose: HTTP协议客户端程序，可以用来下载网页
*wrote by: zhoulifa(zhoulifa@163.com) 周立发(http://zhoulifa.bokee.com)
           Linux爱好者 Linux知识传播者 SOHO族 开发者 最擅长C语言
*date time:2006-03-11 21:49:00
*Note: 任何人可以任意复制代码并运用这些代码，当然包括你的商业用途
*                         但请遵循GPL
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <limits.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <time.h>

#include "ipcHttpSnap.h"
#include "netSocket.h"
#include "debug.h"

/**************************************************************
功能：从字符串src中分析出网站地址和端口，并得到用户要下载的文件
***************************************************************/
void GetHost(char * src, char * web, char * file, int * port)  
{
  char * pA;
  char * pB;
  memset(web, 0, sizeof(web));
  memset(file, 0, sizeof(file));
  *port = 0;
  if(!(*src))  return;
  pA = src;
  if(!strncmp(pA, "http://", strlen("http://")))  pA = src+strlen("http://");
  else if(!strncmp(pA, "https://", strlen("https://")))  pA = src+strlen("https://");
  pB = strchr(pA, '/');
  if(pB)  {
    memcpy(web, pA, strlen(pA) - strlen(pB));
    if(pB+1)  {
      memcpy(file, pB + 1, strlen(pB) - 1);
      file[strlen(pB) - 1] = 0;
    }
  }
  else  memcpy(web, pA, strlen(pA));
  if(pB)  web[strlen(pA) - strlen(pB)] = 0;
  else  web[strlen(pA)] = 0;
  pA = strchr(web, ':');
  if(pA)  *port = atoi(pA + 1);
  else *port = 80;
}

/*
* fn: 通过HTTP 协议向抓拍图片
    < Syntax >
	http://<serverIP>/cgi-bin/anv/snapshot.jpg?<parameter>=<value>[&<parameter>=<value>
	http://192.168.1.176/cgi-bin/anv/snapshot.jpg?chnanel=0
    <Response data>
	HTTP/1.0 200 OK\r\n   
	Content-Type: image/jpeg\r\n 
	Content-Length: <image size>\r\n
    \r\n
    <binary JPEG image data>

* pBuf: out, 存放获取到的图片
* len: buf 的长度
*/
int HttpSnap( char *pBuf, int len ) 
{
  int sockfd = -1;
  char buffer[1024];
  int portnumber,nbytes;
  char host_addr[256];
  char host_file[1024];
  char request[1024];
  int send, totalsend;
  int i, ret, hadReadLen = -1;
  char snapAddr[] = "http://192.168.1.100/cgi-bin/anv/snapshot.jpg?chnanel=0";

  if( NULL == pBuf )
  {
	SVPrint( "error:NULL == pBuf!\r\n" );
	return -1;
  }

  GetHost(snapAddr, host_addr, host_file, &portnumber);/*分析网址、端口、文件名等*/
  ret = SocketTcpConnectBlock( &sockfd, host_addr, portnumber );  
  if( ret == -1 )
  {
      SVPrint( "SocketTcpConnectTimtout(%s), error:%s\r\n", snapAddr, STRERROR_ERRNO );
      return -1;
  }
  /*特别注意,下面这段"sprintf()" 不要随便搬动, 否则会导致获取不到IPC 图片 */
  sprintf(request, "GET /%s HTTP/1.1\r\nAccept: */*\r\nAccept-Language: zh-cn\r\n\
User-Agent: Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0)\r\n\
Host: %s:%d\r\nConnection: Close\r\n\r\n", host_file, host_addr, portnumber);

  /*发送http请求request*/
  send         = 0;
  totalsend = 0;
  nbytes     = strlen(request);
  while(totalsend < nbytes) 
  {
    send = write(sockfd, request + totalsend, nbytes - totalsend);
    if( send == -1 )  
    {
    	printf("send error!%s\n", strerror(errno));
    	return -1;
    }
    totalsend += send;
    /*发送的字节数为197 bytes 为正确!*/
    SVPrint("%d bytes send OK!\n", totalsend);
  }
  
  i=0;
  /* 连接成功了，接收http响应，response */
  while( (nbytes=read(sockfd,buffer,1)) == 1 )
  {
    if( i < 4 )  
    {
      if(buffer[0] == '\r' || buffer[0] == '\n')  i++;
      else i = 0;
    }
    else  
    {
      hadReadLen = 0;
      memcpy( pBuf, buffer, 1 );
      hadReadLen += 1;
      while( (nbytes=read(sockfd,pBuf+hadReadLen,len - hadReadLen)) > 0 )
      {
      	hadReadLen += nbytes;
      }
      break;      
    }
  } // while( (nbytes=read(sockfd,buffer,1)) == 1
  /* 结束通讯 */
  close(sockfd);

  return hadReadLen;
}

