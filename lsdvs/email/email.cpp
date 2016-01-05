/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 邮件发送接口
********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>

#include "pthread.h"

#ifdef      EMAIL_SUPPORT_OPENSSL
#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#endif

#include "const.h"
#include "paramConfig.h"
#include "email.h"

#define MAIL_DATA_SIZE (10*1024) //附件大小
#define SEND_BUFFER_SIZE (0x50000)
#define GET_BIT(status, i)              (((status)&(1<<(i)))>>(i))

static int g_max_channel_num = REAL_CHANNEL_NUM;

static void *hh_mail_sendMailThread(void *param);
static int hh_mail_send(int sockfd,char *pFromaddr,char (*pToaddr)[100],char *pMailData,int sendPictureEnable_ch,int ch,char *param);
//static int hh_mail_send_ssl(int sockfd,char *pFromaddr,char (*pToaddr)[100],char *pMailData,int sendPictureEnable_ch,int ch,char *param);
//static int hh_mail_sendAttachment(int sockfd,int sendPictureEnable_ch,char *param);
//static int hh_mail_sendAttachment_ssl(BIO *bio,int sendPictureEnable_ch,char *param);
//static int hh_mail_getMailAttachmentDataFromFile(char *filepath,int sockfd);
//static int hh_mail_getMailAttachmentDataFromBuf(char *buf,int len,int sockfd);
//static int hh_mail_getMailAttachmentDataFromBuf_ssl(char *buf,int len,BIO *bio);
static int hh_mail_getMailBody(char *pMailData, char *pTempaddr, int ch ,int emailSendType);
static int hh_mail_checkMailSendStatus(char *buf);
static void base64_encode(char *in, const int in_len, char *out, int out_len);

static const char *base64codes = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static void hh_param_getEmailStruct(PARAM_CONFIG_EMAIL *temp)
{
	strcpy(temp->serverIP, "smtp.163.com");
	strcpy(temp->fromAddr, "rha@163.com");    
	strcpy(temp->password, "123456");
	strcpy(temp->toAddr, "rha@163.com");
	strcpy(temp->userName, "rha");

    
	strcpy(temp->subject, "rha send email");
	strcpy(temp->loginName, "rha@163.com");

	temp->port = 25;
}


/* 
 * 功能:发送邮件	                        
 * 参数:
    ch:通道号	                        
    emailSendType:根据该类型来决定email的内容
 */
int hh_mail_startSendMail(int ch, int emailSendType)
{
	send_mail_info_t *send_mail_info;
	PARAM_CONFIG_EMAIL mail_info;

	g_max_channel_num = 1;

	if(emailSendType == 0)
    	return -1;
	hh_param_getEmailStruct(&mail_info);
    
	if(     (unsigned char)mail_info.serverIP[0] == 0xff 
        || (unsigned char)mail_info.toAddr[0] == 0xff 
        || mail_info.serverIP[0] == 0 
        || mail_info.toAddr[0] == 0 )
    	return -1;
        
	send_mail_info = (send_mail_info_t *)malloc(sizeof(send_mail_info_t));
	if(send_mail_info == NULL)	return -1;
	bzero(send_mail_info,sizeof(send_mail_info_t));

	send_mail_info->ch = ch;
	send_mail_info->emailSendType = emailSendType;

	pthread_t sendmail_thread_id;
	if(pthread_create(&sendmail_thread_id, NULL, hh_mail_sendMailThread, (void *)send_mail_info) < 0)
    {
    	printf( "creat thread error!\r\n" );
    	free(send_mail_info);
    	return -1;
    }
    
	return 0;
}

static void *hh_mail_sendMailThread(void *param)
{
	send_mail_info_t send_mail_info;

	pthread_detach(pthread_self());
	bzero(&send_mail_info,sizeof(send_mail_info_t));
       	memcpy(&send_mail_info,(send_mail_info_t *)param,sizeof(send_mail_info_t));
	free(param);

	printf("33333333333333333333333333 hh_mail_sendMailThread!\r\n");
	hh_mail_sendMail(&send_mail_info,NULL);
	return NULL;
}

int hh_mail_sendMail(send_mail_info_t *send_mail_info,char *param)
{
	int i = 0;
	int sockfd = 0;
	int result = 0;
	struct sockaddr_in serv_addr;
	struct hostent *hp;
	PARAM_CONFIG_EMAIL mail_info;
	struct timeval tv;
	char *pTempToAddr;
	char pMailData[MAIL_DATA_SIZE];
	char cFromaddr[100],cToaddr[20][100],cTempaddr[2048];
	char cTemp[100];
	char *pTemp = NULL;

	g_max_channel_num = 1;

	bzero(&mail_info,sizeof(PARAM_CONFIG_EMAIL));
	hh_param_getEmailStruct(&mail_info);

	printf("send_mail_info->ch = %d\n",send_mail_info->ch);
	printf("send_mail_info->emailSendType = %d\n",send_mail_info->emailSendType);
    
	bzero(&serv_addr,sizeof(struct sockaddr_in));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(mail_info.port);
    
	hp = gethostbyname(mail_info.serverIP);
	if(hp == NULL)
    {
    	printf("hp error: %d\n",h_errno);
    	goto __error;
    }
	memcpy(&serv_addr.sin_addr.s_addr,hp->h_addr_list[0],hp->h_length);
	printf("%d.%d.%d.%d",
    	hp->h_addr_list[0][0],hp->h_addr_list[0][1],hp->h_addr_list[0][2],hp->h_addr_list[0][3]);

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
    	printf("socket error\n");
    	goto __error;
    }


	tv.tv_sec = 10;
	tv.tv_usec = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	result=connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	if(result == -1)
    {
    	printf("connect error\n");
    	close(sockfd);
    	goto __error;
    }    

	bzero(pMailData, MAIL_DATA_SIZE);
	bzero(cToaddr,sizeof(cToaddr));
	bzero(cTempaddr,sizeof(cTempaddr));
	bzero(cTemp,sizeof(cTemp));

	sprintf(cFromaddr,"MAIL FROM: <%s>\r\n",mail_info.fromAddr);
	pTempToAddr = mail_info.toAddr;
	while(1)
    {
    	if(((pTemp = strsep(&pTempToAddr,";")) == NULL) || i == 20)
        	break;
    	if((strlen(pTemp) == 0) || (strlen(pTemp) >= 85))
        	break;
    	printf("strlen(pTemp) = %d\n",strlen(pTemp));
    	sprintf(cToaddr[i],"RCPT TO: <%s>\r\n",pTemp);
    	sprintf(cTemp,"To: <%s>\r\n",pTemp);
    	strcat(cTempaddr,cTemp);
    	i++;
    }

	if(send_mail_info->emailSendType == EMAIL_SEND_TYPE_IO_PROBER)
    {
    	hh_mail_getMailBody(pMailData,cTempaddr,
            	send_mail_info->ch,send_mail_info->emailSendType);//邮件体
    	if((hh_mail_send(sockfd,cFromaddr,cToaddr,pMailData,0 /*mail_info.proberalarm.sendPictureEnable_ch*/,send_mail_info->ch,param)) == -1)
        {
        	goto __error;
        }
    }
	else if(send_mail_info->emailSendType == EMAIL_SEND_TYPE_VIDEO_LOSE)
    {
    	hh_mail_getMailBody(pMailData,cTempaddr,
            	send_mail_info->ch,send_mail_info->emailSendType);//邮件体
    	if((hh_mail_send(sockfd,cFromaddr,cToaddr,pMailData,0 /*mail_info.lostalarm.sendPictureEnable_ch*/,send_mail_info->ch,param)) == -1)
        {
        	goto __error;
        }
    }
	else if(send_mail_info->emailSendType == EMAIL_SEND_TYPE_MOTION)
    {
    	hh_mail_getMailBody(pMailData,cTempaddr,
            	send_mail_info->ch,send_mail_info->emailSendType);//邮件体
    	if((hh_mail_send(sockfd,cFromaddr,cToaddr,pMailData, 0 /*mail_info.movealarm.sendPictureEnable_ch*/,send_mail_info->ch,param)) == -1)
        {
        	goto __error;
        }
    }
	else if(send_mail_info->emailSendType == EMAIL_SEND_TYPE_TIMER_CAPTURE)
    {
    	hh_mail_getMailBody(pMailData,cTempaddr,
            	send_mail_info->ch,send_mail_info->emailSendType);//邮件体
    	if((hh_mail_send(sockfd, cFromaddr, cToaddr,pMailData, 0 /*(1<<send_mail_info->ch)*/,send_mail_info->ch,NULL)) == -1)
        {
        	goto __error;
        }
    }
	else
    	goto __error;
__error:
	return 0;
}

//将邮件所需的内容存入pMailData中
static int hh_mail_getMailBody(char *pMailData, char *pTempaddr, int ch ,int emailSendType)
{
	unsigned int i = 0;
	int chineseFlag = 0;
	char buffer[20];
	PARAM_CONFIG_EMAIL mail_info;
	char tmpbuffer[256];

	bzero(&mail_info,sizeof(PARAM_CONFIG_EMAIL));
	hh_param_getEmailStruct(&mail_info);
    
	chineseFlag = 0;
	for(i = 0; i < strlen(mail_info.userName); i++)
    {
    	if((unsigned char)mail_info.userName[i] > 128)
        {
        	chineseFlag = 1;
        	break;
        }
    }
	for(i = 0; i < strlen(mail_info.subject); i++)
    {
    	if((unsigned char)mail_info.subject[i] > 128)
        {
        	chineseFlag = 1;
        	break;
        }
    }

	if(chineseFlag)//是中文
    {
    	memset(tmpbuffer,0,sizeof(tmpbuffer));
    	base64_encode(mail_info.userName,strlen(mail_info.userName),tmpbuffer, strlen(mail_info.userName));

    	strcat(pMailData,"From: =?gb2312?B?");
    	strcat(pMailData,tmpbuffer);
    	strcat(pMailData,"?=<");
    }
	else
    {
    	strcat(pMailData,"From: ");
    	strcat(pMailData,mail_info.userName);
    	strcat(pMailData,"<");
    }
	strcat(pMailData,mail_info.fromAddr);
	strcat(pMailData,">\r\n");
    
	if(chineseFlag)//是中文
    {
    	memset(tmpbuffer,0,sizeof(tmpbuffer));
    	base64_encode(mail_info.subject,strlen(mail_info.subject),tmpbuffer, strlen(mail_info.subject));

    	strcat(pMailData,"Subject: =?gb2312?B?");
    	strcat(pMailData,tmpbuffer);
    	strcat(pMailData,"?=\r\n");
    }
	else
    {
    	strcat(pMailData,"Subject: ");
    	strcat(pMailData,mail_info.subject);
    	strcat(pMailData,"\r\n");
    }

	strcat(pMailData,pTempaddr);

	strcat(pMailData,"MIME-Version: 1.0\r\n");
	strcat(pMailData,"Content-Type: multipart/mixed;boundary=\"==part000==\"\r\n");
	strcat(pMailData,"\r\n\r\nThis is a multi-part message in MIME format.\r\n\r\n");
    //正文
	strcat(pMailData,"--==part000==\r\n");
	strcat(pMailData,"Content-Type: multipart/alternative;boundary=\"==part001\"\r\n\r\n");
	strcat(pMailData,"--==part001\r\n");
	strcat(pMailData,"Content-Type:text/plain;charset=\"gb2312\"\r\n");//gb2312//UTF-8//unicode
	strcat(pMailData,"Content-Transfer-Encoding:7bit\r\n\r\n");
#if 0	
	time_t tt;
	time(&tt);
    
	bzero(buffer,sizeof(buffer));
	sprintf(buffer,"%s",ctime(&tt));
	buffer[strlen(buffer)-1] = 0;//modify for special mail
	strcat(pMailData,buffer);
#endif
	time_t tt;
// 邮件 内容	
	time(&tt);
	bzero(buffer, sizeof(buffer));
	sprintf(buffer, "%d/%d/%d ", 5, 21, 2011);
	strcat(pMailData,buffer);

	if(emailSendType == EMAIL_SEND_TYPE_MOTION)
    {
    	bzero(buffer,sizeof(buffer));
    	sprintf(buffer," Channel %d is alert for ",ch+1);
    	strcat(pMailData,buffer);
    	strcat(pMailData,"motion detected.");
    }
	else if(emailSendType == EMAIL_SEND_TYPE_IO_PROBER)
    {
    	bzero(buffer,sizeof(buffer));
    	sprintf(buffer," 海华数码Channel %d is alert for ",ch+1);
    	strcat(pMailData,buffer);
    	strcat(pMailData,"I/O triggered.");
    }
	else if(emailSendType == EMAIL_SEND_TYPE_VIDEO_LOSE)
    {
    	bzero(buffer,sizeof(buffer));
    	sprintf(buffer," Channel %d is alert for ",ch+1);
    	strcat(pMailData,buffer);
    	strcat(pMailData,"video lost.");
    }
	else if(emailSendType == EMAIL_SEND_TYPE_TIMER_CAPTURE)
    {
    	bzero(buffer,sizeof(buffer));
    	sprintf(buffer," Channel %d ",ch+1);
    	strcat(pMailData,buffer);
    	strcat(pMailData,"scheduled snapshot.");
    }
	strcat(pMailData,"\r\n");
	strcat(pMailData,"--==part001--\r\n");
    
	return 0;
}

#if 0
//从Buf中得到附件内容,并编码
static int hh_mail_getMailAttachmentDataFromBuf(char *buf,int len,int sockfd)
{
	int i = 0;
        char pAttachmentData[2*MAIL_DATA_SIZE];
	int n =0;

	while(1)
    {
    	n = (len > (MAIL_DATA_SIZE - 1))?(MAIL_DATA_SIZE-1):len;
    	base64_encode(buf+i*(MAIL_DATA_SIZE-1), n,pAttachmentData, n);//编码
    	send(sockfd,pAttachmentData,strlen(pAttachmentData),0);
    	len = len - n;
    	if(len <= 0)
        	break;
    	i++;
    }
	return 0;
}

static int hh_mail_getMailAttachmentDataFromBuf_ssl(char *buf,int len,BIO *bio)
{
	int i = 0;
//        char pAttachmentData[2*MAIL_DATA_SIZE];
	int n =0;

	BIO *b64;

	b64 = BIO_new(BIO_f_base64());
	BIO_push(b64,bio);

	while(1)
    {
    	n = (len > (MAIL_DATA_SIZE - 1))?(MAIL_DATA_SIZE-1):len;
    	BIO_write(b64,buf+i*(MAIL_DATA_SIZE-1),n);
    	BIO_flush(b64);

    	len = len - n;
    	if(len <= 0)
        	break;
    	i++;
    }
	BIO_free(b64);
	return 0;
}
#endif
#if 0
//通过文件路径得到附件内容,并编码
static int hh_mail_getMailAttachmentDataFromFile(char *filepath,int sockfd)
{
	FILE *fp;
	size_t n;
	char pAttachmentData[2 * MAIL_DATA_SIZE];
	char buf[MAIL_DATA_SIZE];
    
	if(!(fp = fopen(filepath,"rb")))//发送图片附件
    {
    	debug_Printf("fp < 0\n");
    	return -1;
    }
	while(1)
    {
    	bzero(pAttachmentData,MAIL_DATA_SIZE);
    	bzero(buf,sizeof(buf));
    	n = fread(buf,1,MAIL_DATA_SIZE - 1,fp);
    	if(n == 0)
        	break;
    	base64_encode(buf,n,pAttachmentData,n);//编码
    	send(sockfd,pAttachmentData,strlen(pAttachmentData),0);
    }
	fclose(fp);
	return 0;
}
#endif
#if 0
static int hh_mail_sendAttachment(int sockfd,int sendPictureEnable_ch,char *param)
{
	char	tempbuffer[256];
	char	tmp[64];
	char	buffer[SEND_BUFFER_SIZE];
	int	datalen = 0;
	int	i = 0;
	int	ret = 0;
	alarm_cap_t *tmp_alarm;

	printf("hh_mail_sendAttachment sendPictureEnable_ch = %x sockfd = %d\n",sendPictureEnable_ch,sockfd);
	for (i = 0; i < g_max_channel_num; i++)
    {
    	if ((GET_BIT(sendPictureEnable_ch, i)) && (param == NULL))
        {
        	ret = hh_enc_getSnapJPEG(i, buffer, &datalen);
        	if(ret < 0)
            	return 0;
        }
    	else if(param != NULL)
        {
        	tmp_alarm = (alarm_cap_t *)param;
        	if((tmp_alarm->cap_type&0x02) == 0x02)
            {

            	if((tmp_alarm->email_cap[i].size > 0) && (tmp_alarm->email_cap[i].cap_buf != NULL))
                {
                	memcpy(buffer,tmp_alarm->email_cap[i].cap_buf,tmp_alarm->email_cap[i].size);
                	datalen = tmp_alarm->email_cap[i].size;
                }
            	else
                	continue;
            }
        }
    	else
        	continue;
    	bzero(tempbuffer,sizeof(tempbuffer));
    	strcpy(tempbuffer,"--==part000==\r\n");
    	bzero(tmp,sizeof(tmp));
    	sprintf(tmp,"Content-Type:image/jpeg;name=\"channel_%d.jpg\"\r\n",i+1);
    	strcat(tempbuffer,tmp);
    	strcat(tempbuffer,"Content-Transfer-Encoding:base64\r\n");
    	bzero(tmp,sizeof(tmp));
    	sprintf(tmp,"Content-Disposition:attachment;filename=\"channel_%d.jpg\"\r\n\r\n",i+1);
    	strcat(tempbuffer,tmp);
    	send(sockfd,tempbuffer,strlen(tempbuffer),0);
    	hh_mail_getMailAttachmentDataFromBuf(buffer,datalen,sockfd);
    	bzero(tempbuffer,sizeof(tempbuffer));
    	strcat(tempbuffer,"\r\n");
    	send(sockfd,tempbuffer,strlen(tempbuffer),0);
    	printf("**** ccccccccccccch%d  ***\n",i);
    }
	printf("***************** send ok *********************** \n");
	return 0;
}
static int hh_mail_sendAttachment_ssl(BIO *bio,int sendPictureEnable_ch,char *param)
{
	char	tempbuffer[256];
	char	tmp[64];
	char	buffer[SEND_BUFFER_SIZE];
	int	datalen = 0;
	int	i;
	int 	ret = 0;
	alarm_cap_t *tmp_alarm;// = *(alarm_info_t *)param;

	printf("==== hh_mail_sendAttachment sendPictureEnable_ch = %x ****\n",sendPictureEnable_ch);
	for (i = 0; i < g_max_channel_num; i++)
    {
    	if ((GET_BIT(sendPictureEnable_ch, i)) && (param == NULL))
        {
        	ret = hh_enc_getSnapJPEG(i, buffer, &datalen);
        	if(ret < 0)
            	return 0;
        }
    	else if(param != NULL)
        {
        	tmp_alarm = (alarm_cap_t *)param;
        	if((tmp_alarm->cap_type&0x02) == 0x02)
            {
            	if((tmp_alarm->email_cap[i].size > 0) && (tmp_alarm->email_cap[i].cap_buf != NULL))
                {
                	memcpy(buffer,tmp_alarm->email_cap[i].cap_buf,tmp_alarm->email_cap[i].size);
                	datalen = tmp_alarm->email_cap[i].size;
                }
            }
        }
    	else
        	continue;
    	printf("hh_enc_getSnapJPEG  ret = %d\n",ret);
    	bzero(tempbuffer,sizeof(tempbuffer));
    	strcpy(tempbuffer,"--==part000==\r\n");
    	bzero(tmp,sizeof(tmp));
    	sprintf(tmp,"Content-Type:image/jpeg;name=\"channel_%d.jpg\"\r\n",i+1);
    	strcat(tempbuffer,tmp);
    	strcat(tempbuffer,"Content-Transfer-Encoding:base64\n");
    	bzero(tmp,sizeof(tmp));
    	sprintf(tmp,"Content-Disposition:attachment;filename=\"channel_%d.jpg\"\r\n\r\n",i+1);
    	strcat(tempbuffer,tmp);
    	BIO_write(bio,tempbuffer,strlen(tempbuffer));
    	hh_mail_getMailAttachmentDataFromBuf_ssl(buffer,datalen,bio);
    	bzero(tempbuffer,sizeof(tempbuffer));
    	strcat(tempbuffer,"\r\n");
    	BIO_write(bio,tempbuffer,strlen(tempbuffer));
    	printf("**** ccccccccccccch%d  ***\n",i);
    }
	printf("*****************ssl send ok *********************** \n");
	return 0;
}
#endif

//++++++++++++++++++++++++++++++++开始发送邮件++++++++++++++++++++++++++++++++        
static int hh_mail_send(int sockfd,char *pFromaddr,char (*pToaddr)[100],char *pMailData,int sendPictureEnable_ch,int ch,char *param)
{
	int iLength;
	char buf[200];
//	int i = 0;
	PARAM_CONFIG_EMAIL mail_info;

	bzero(&mail_info,sizeof(PARAM_CONFIG_EMAIL));
	hh_param_getEmailStruct(&mail_info);
    
	if(mail_info.port == 465)
    {
    	close(sockfd);
        //hh_mail_send_ssl(sockfd,pFromaddr,pToaddr,pMailData,sendPictureEnable_ch,ch,param);
    	return 0;
    }

	bzero(buf,sizeof(buf));
	iLength=recv(sockfd, buf, sizeof(buf), 0);
	printf("R: %s!\r\n",buf);
	if(hh_mail_checkMailSendStatus(buf) == -1)
        goto quit;

	strcpy(buf, "HELO SMTP\r\n");
	send(sockfd, buf, strlen(buf), 0);
	printf("S: %s!\r\n", buf);
	bzero(buf, sizeof(buf));
	iLength=recv(sockfd, buf, sizeof(buf), 0);
	printf("R: %s!\r\n",buf);
	if(hh_mail_checkMailSendStatus(buf) == -1)
        goto quit;

	char tmpbuffer[64];
	base64_encode(mail_info.loginName,strlen(mail_info.loginName),tmpbuffer, strlen(mail_info.loginName));
	strcpy(buf, "AUTH LOGIN\r\n");
	send(sockfd, buf, strlen(buf), 0);
	printf("S: %s!\r\n", buf);
	bzero(buf,sizeof(buf));
	iLength=recv(sockfd, buf, sizeof(buf), 0);//服务器是否准备好.
	printf("R: %s!\r\n", buf);
	if(strstr(buf,"STARTTLS") != NULL)
    {
    	printf("must use STARTTLS mode\n");
    	close(sockfd);
        //hh_mail_send_ssl(sockfd,pFromaddr,pToaddr,pMailData,sendPictureEnable_ch,ch,param);
    	return 0;
    }

	strcat(tmpbuffer, "\r\n");
	send(sockfd, tmpbuffer, strlen(tmpbuffer), 0);    
	printf("S: loginname(%s), after base64(%s)!\r\n", mail_info.loginName, tmpbuffer);
	bzero(buf,sizeof(buf));
	iLength=recv(sockfd,buf,sizeof(buf),0);
	printf("R: %s!\r\n",buf);
    
	bzero(tmpbuffer,sizeof(tmpbuffer));
	base64_encode(mail_info.password,strlen(mail_info.password),tmpbuffer, strlen(mail_info.password));
	strcat(tmpbuffer, "\r\n");
	send(sockfd, tmpbuffer, strlen(tmpbuffer), 0);
	printf("S: passwd(%s), after base64(%s)!\r\n", mail_info.password, tmpbuffer);
	bzero(buf, sizeof(buf));
	iLength = recv(sockfd, buf, sizeof(buf), 0);
	printf("R: %s!\r\n", buf);
    
	send(sockfd, pFromaddr, strlen(pFromaddr), 0);
	printf("S: %s!\r\n", pFromaddr);
	bzero(buf,sizeof(buf));
	iLength=recv(sockfd, buf, sizeof(buf), 0);
	printf("R: %s!\r\n", buf);
	if(hh_mail_checkMailSendStatus(buf) == -1)
        	goto quit;

	while(1)
    {
    	if(strlen(*pToaddr) == 0)
        	break;
    	send(sockfd, *pToaddr, strlen(*pToaddr), 0);
    	printf("S: %s!\r\n", *pToaddr);
    	bzero(buf, sizeof(buf));
    	iLength=recv(sockfd, buf, sizeof(buf), 0);
    	printf("R: %s!\r\n", buf);
    	pToaddr++;
    	if(hh_mail_checkMailSendStatus(buf) == -1)
        	goto quit;
    }

	strcpy(buf, "DATA\r\n"); 
	send(sockfd, buf, strlen(buf), 0);  
	printf("S:%s!\r\n", buf);
	bzero(buf, sizeof(buf));
	iLength=recv(sockfd, buf, sizeof(buf), 0);
	printf("R: %s!\r\n", buf);
	if(hh_mail_checkMailSendStatus(buf) == -1)
        goto quit;
        
	send(sockfd, pMailData, strlen(pMailData), 0);
	printf("S: %s!\r\n", pMailData);
	printf("sendPictureEnable_ch = %x\n",sendPictureEnable_ch);
	if(sendPictureEnable_ch != 0)
    {
    	printf("have to send attachment!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
        ;//hh_mail_sendAttachment(sockfd,sendPictureEnable_ch,param);
    }

	strcpy(buf, "\r\n--==part000--\r\n.\r\n");
	send(sockfd, buf, strlen(buf), 0);
	printf("S:%s!\r\n", buf);
	bzero(buf, sizeof(buf));
	iLength=recv(sockfd, buf, sizeof(buf), 0);
	printf("R: %s!\r\n",buf);
	if(hh_mail_checkMailSendStatus(buf) == -1)
        	goto quit;

quit:
	strcpy(buf, "QUIT\r\n");
	send(sockfd, buf, strlen(buf), 0);
	printf("S: %s!\r\n", buf);
	bzero(buf,sizeof(buf));
	recv(sockfd,buf,sizeof(buf),0);    
	printf("R: %s!\r\n",buf);
    
	close(sockfd);

	return 0;
}
#if 0
static int hh_mail_send_ssl(int sockfd,char *pFromaddr,char (*pToaddr)[100],char *pMailData,int sendPictureEnable_ch,int ch,char *param)
{
	int iLength;
	char buf[1024];
	char temp_buf[128];
	BIO *bio;
	SSL * ssl;
	SSL_CTX * ctx;
	PARAM_CONFIG_EMAIL mail_info;

	bzero(&mail_info,sizeof(PARAM_CONFIG_EMAIL));
	hh_param_getEmailStruct(&mail_info);

    /* Set up the library */
	ERR_load_BIO_strings();
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();

    /* Set up the SSL context */
	ctx = SSL_CTX_new(SSLv23_client_method());

    /* Load the trust store */
	if(! SSL_CTX_load_verify_locations(ctx, "/dvs/TrustStore.pem", NULL))
    {
    	fprintf(stderr, "Error loading trust store\n");
    	ERR_print_errors_fp(stderr);
    	SSL_CTX_free(ctx);
    	return 0;
    }

    /* Setup the connection */
	bio = BIO_new_ssl_connect(ctx);

    /* Set the SSL_MODE_AUTO_RETRY flag */
	BIO_get_ssl(bio, & ssl);
	SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    /* Create and setup the connection */
	bzero(temp_buf,sizeof(temp_buf));
//	sprintf(temp_buf,"%s:%d",mail_info.serverIP,mail_info.port);
	sprintf(temp_buf,"%s:465",mail_info.serverIP);
	printf("temp_buf = %s\n",temp_buf);
	BIO_set_conn_hostname(bio, temp_buf);

	if(BIO_do_connect(bio) <= 0)
    {
    	fprintf(stderr, "Error attempting to connect\n");
    	ERR_print_errors_fp(stderr);
    	BIO_free_all(bio);
    	SSL_CTX_free(ctx);
    	return 0;
    }

    /* Check the certificate */
	if(SSL_get_verify_result(ssl) != X509_V_OK)
    {
    	fprintf(stderr, "Certificate verification error: %ld\n", SSL_get_verify_result(ssl));
    	BIO_free_all(bio);
    	SSL_CTX_free(ctx);
    	return 0;
    }

    /* Send the request */
	iLength = BIO_read(bio, buf, 1023);
	buf[iLength] = 0;
	printf("BEGIN received: %s \n",buf);
	if(hh_mail_checkMailSendStatus(buf) == -1)
    	goto quit;

	bzero(buf,sizeof(buf));
	BIO_write(bio, "EHLO\r\n", strlen("EHLO\r\n"));
	iLength = BIO_read(bio, buf, 1023);
	buf[iLength] = 0;
	printf("EHLO received: %s \n",buf);
	if(hh_mail_checkMailSendStatus(buf) == -1)
    	goto quit;

	char tmpbuffer[64];
//	base64_encode(mail_info.fromAddr,strlen(mail_info.fromAddr),tmpbuffer, strlen(mail_info.fromAddr));
	base64_encode(mail_info.loginName,strlen(mail_info.loginName),tmpbuffer, strlen(mail_info.loginName));
	printf("mail_info.loginName = %s\n",mail_info.loginName);
	BIO_write(bio, "AUTH LOGIN \r\n", strlen("AUTH LOGIN \r\n"));
	printf("AUTH LOGIN \n");
	bzero(buf,sizeof(buf));
	iLength = BIO_read(bio,buf,sizeof(buf));
	printf("AUTH LOGIN received: %s \n",buf);

	BIO_write(bio,tmpbuffer,strlen(tmpbuffer));
	BIO_write(bio,"\r\n",strlen("\r\n"));
	bzero(buf,sizeof(buf));
	iLength = BIO_read(bio,buf,sizeof(buf));
	printf("USER received: %s \n",buf);
    
	bzero(tmpbuffer,sizeof(tmpbuffer));
	base64_encode(mail_info.password,strlen(mail_info.password),tmpbuffer, strlen(mail_info.password));
	BIO_write(bio,tmpbuffer,strlen(tmpbuffer));
	printf("tmpbuffer = %s\n",tmpbuffer);
	BIO_write(bio,"\r\n",strlen("\r\n"));
	bzero(buf,sizeof(buf));
	iLength = BIO_read(bio,buf,sizeof(buf));
	printf("PASSWORD received: %s \n",buf);
    
	BIO_write(bio,pFromaddr,strlen(pFromaddr));
	bzero(buf,sizeof(buf));
	iLength = BIO_read(bio,buf,sizeof(buf));
	printf("MAIL FROM SENT received: %s\n",buf);
	if(hh_mail_checkMailSendStatus(buf) == -1)
    	goto quit;

	while(1)
    {
    	if(strlen(*pToaddr) == 0)
        	break;
    	BIO_write(bio,*pToaddr,strlen(*pToaddr));
    	bzero(buf,sizeof(buf));
    	iLength = BIO_read(bio,buf,sizeof(buf));
    	printf("RCPT TO SENT received: %s\n",buf);
        *pToaddr++;
    	if(hh_mail_checkMailSendStatus(buf) == -1)
        	goto quit;
    }

	BIO_write(bio,"DATA\r\n",strlen("DATA\r\n"));
	bzero(buf,sizeof(buf));
	iLength = BIO_read(bio,buf,sizeof(buf));
	printf("DATA SENT received: %s\n",buf);
	if(hh_mail_checkMailSendStatus(buf) == -1)
    	goto quit;
        
	BIO_write(bio,pMailData,strlen(pMailData));
	printf("sendPictureEnable_ch = %x\n",sendPictureEnable_ch);
	if(sendPictureEnable_ch != 0)
    {
    	printf("have to seng attachment ssl!!!!!!!!!!!!!!!!!!\r\n");
        ;//hh_mail_sendAttachment_ssl(bio,sendPictureEnable_ch,param);
    }
	BIO_write(bio,"\r\n--==part000--\r\n.\r\n",strlen("\r\n--==part000--\r\n.\r\n"));
	bzero(buf,sizeof(buf));
	iLength = BIO_read(bio,buf,sizeof(buf));
	printf("DATA BODY SENT received: %s\n",buf);
	if(hh_mail_checkMailSendStatus(buf) == -1)
    	goto quit;

quit:
	BIO_write(bio,"QUIT\r\n",strlen("QUIT\r\n"));
	printf("QUIT\n");
	bzero(buf,sizeof(buf));
	BIO_read(bio,buf,sizeof(buf));

	BIO_free_all(bio);
	SSL_CTX_free(ctx);

	return 0;
}
#endif

//检查邮件发送的状态
static int hh_mail_checkMailSendStatus(char *buf)
{
	char c;
	int i;

	c = *buf;
	i = (int)c;
	switch (i)
    { 
    	case 50:
        	return 0;
        	break;
    	case 51:
        	return 0;
        	break;
    	case 53:
        	return -1;
        	break;
    	default:
                return -1;
    }
}

//base64编码函数
static void base64_encode(char *in, const int in_len, char *out, int out_len)
{
	int base64_len = 4 * ((in_len+2)/3);
	if(out_len >= base64_len)
    	printf("out_len >= base64_len\n");
	char *p = out;
        int times = in_len / 3;
        int i;

        for(i=0; i<times; ++i)
           {
        *p++ = base64codes[(in[0] >> 2) & 0x3f];
                *p++ = base64codes[((in[0] << 4) & 0x30) + ((in[1] >> 4) & 0xf)];
                *p++ = base64codes[((in[1] << 2) & 0x3c) + ((in[2] >> 6) & 0x3)];
            *p++ = base64codes[in[2] & 0x3f];
                in += 3;
        }
        if(times * 3 + 1 == in_len) 
    {
        *p++ = base64codes[(in[0] >> 2) & 0x3f];
                *p++ = base64codes[((in[0] << 4) & 0x30) + ((in[1] >> 4) & 0xf)];
                *p++ = '=';
                *p++ = '=';
        }
	if(times * 3 + 2 == in_len) 
    {
               *p++ = base64codes[(in[0] >> 2) & 0x3f];
               *p++ = base64codes[((in[0] << 4) & 0x30) + ((in[1] >> 4) & 0xf)];
               *p++ = base64codes[((in[1] << 2) & 0x3c)];
               *p++ = '=';
       }
       *p = 0;
}

#if 0
int main()
{
	printf("test send email\r\n");
	hh_mail_startSendMail(0, EMAIL_SEND_TYPE_IO_PROBER);
	printf("test send email\r\n");

	while(1)
    {
    	printf("send email test!\r\n");
    	sleep(10);
    }
	return 0;
}
#endif

