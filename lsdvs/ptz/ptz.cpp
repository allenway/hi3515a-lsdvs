/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 云台控制的通用类封装
*******************************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "debug.h"

#include "const.h"
#include "mutex.h"
#include "linuxFile.h"
#include "ptz.h"
//#include "configManage.h"

static CPTZ g_Ptz;
static CMutexLock g_ptzCtrlMutex;
static int PrintfPtzCmd(int channel,char *cmd,int cmdLen);

CPTZ::CPTZ()
{
	m_nTty=-1;
	m_nDev=-1;
	m_bUseRs232=FALSE;
}

CPTZ::~CPTZ()
{
	Close();
}

int CPTZ::Open(char* pDev)
{
	Close();
	if(strcmp(pDev,PORT_RS232)==0) {
    	m_bUseRs232=TRUE;
    	SVPrint("Message output disabled for PORT busy !");
    	usleep(200*1000);
    }

	m_nTty=open(pDev,O_RDWR);
	if(m_nTty!=-1) 
    {
    	SVPrint("open %s ok !\r\n",pDev);
        
    	tcgetattr(m_nTty,&m_termios);
    	ioctl(m_nTty,TIOCMGET,&m_tiocm);
    	return 0;
    } 
	else 
    {
    	SVPrint("open %s fail !",pDev);
    	return -1;
    }
}

void CPTZ::Close()
{
	if(m_nTty!=-1) {
    	ioctl(m_nTty,TIOCMSET,&m_tiocm);
    	tcsetattr(m_nTty,TCSANOW,&m_termios);
    	close(m_nTty);
    	m_nTty=-1;

    	if(m_bUseRs232) {
        	SVPrint("Message output enabled for PORT free !");
        }
    }
	m_nDev=-1;
	m_bUseRs232=FALSE;
}

/*
* 选择哪一个云镜
*/
int CPTZ::SelectDev(int nDev)
{
	int nRet=-1;
    #if 1 // ql debug
    
	m_nDev=nDev;
	strcpy( m_Devs[nDev].DevName, "Pocol-d" );
	m_Devs[m_nDev].cBaudrate = 6;
	m_Devs[m_nDev].cDatabits = 3;
	m_Devs[m_nDev].cStopbits = 0;
	m_Devs[m_nDev].cParity	 = 0;
    #endif
	if(m_nTty!=-1 && nDev>=0 && nDev<_MAX_PTZ && m_Devs[nDev].DevName[0]) 
    {
        // init COMx port
    	struct termios options;
    	int tiocm;

    	const tcflag_t BAUDRATE[] = { B110, B300, B600, B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200 };
    	const tcflag_t DATABITS[] = { CS5, CS6, CS7, CS8 };
    	const tcflag_t STOPBITS[] = { 0, CSTOPB };
    	const tcflag_t PARITY[] = { 0, PARENB | PARODD, PARENB };        
        
    	m_nDev=nDev;
    	memcpy(&options,&m_termios,sizeof(options));
    	cfsetispeed(&options,BAUDRATE[m_Devs[m_nDev].cBaudrate]);
    	cfsetospeed(&options,BAUDRATE[m_Devs[m_nDev].cBaudrate]);
    	options.c_lflag &= ~(ECHO|ICANON|IEXTEN|ISIG|NOFLSH);
    	options.c_cflag &= ~CSIZE;
    	options.c_cflag |= DATABITS[m_Devs[m_nDev].cDatabits];
    	options.c_cflag |= STOPBITS[m_Devs[m_nDev].cStopbits];
    	options.c_cflag &= ~(PARENB | PARODD);
    	options.c_cflag |= PARITY[m_Devs[m_nDev].cParity];
    	options.c_cflag &= ~CRTSCTS;
    	options.c_iflag &= ~(BRKINT|ICRNL|ISTRIP);
    	options.c_iflag &= ~(IXON|IXOFF); // no XOR/XOFF flow control
    	options.c_cc[VMIN]=1;
    	options.c_cc[VTIME]=0;
    	options.c_line=0;
    	options.c_iflag &= ~(INLCR | ICRNL | IGNCR);
    	options.c_oflag &= ~(ONLCR | OCRNL | ONOCR | ONLRET);        

    	tcsetattr(m_nTty,TCSANOW,&options);

    	tiocm = m_tiocm;
    	tiocm |= TIOCM_DTR|TIOCM_RTS;
    	tiocm &= ~(TIOCM_DSR|TIOCM_CTS);
    	ioctl(m_nTty,TIOCMSET,&tiocm);

    	nRet=0;
    } 
	else if(nDev == -1)
    {
    	m_nDev = -1;        
    }

	SVPrint("ch(%d) ptz:ptotocol(%s) Baudrate(%d) Databits(%d) Stopbits(%d) Parity(%d)!\r\n",
                	nDev,m_Devs[nDev].DevName,
                	m_Devs[m_nDev].cBaudrate,m_Devs[m_nDev].cDatabits,
                	m_Devs[m_nDev].cStopbits,m_Devs[m_nDev].cParity);

	return nRet;
}

int CPTZ::GetDev()
{
	return m_nDev;
}

int CPTZ::SetDevParam(LPDEV_PARAM lpDevs)
{
	int i;
	if(lpDevs == NULL) 
    	return -1;
	memcpy(m_Devs,lpDevs,sizeof(DEV_PARAM)*_MAX_PTZ);
	for(i=0;i<_MAX_PTZ;i++) 
    	if(lpDevs->DevName[0]=='\0') 
        	break;
	if(m_nDev>=i) 
    	m_nDev=-1;
	return 0;
}

//ql add
void CPTZ::SetChannelCurProtocol(int channel,LPDEV_PARAM lpDevs)
{
	memcpy(&m_Devs[channel],lpDevs,sizeof(DEV_PARAM));    
}

void CPTZ::SetSerial(int channel,unsigned char cBaudrate,unsigned char cDatabits,
                	unsigned char cStopbits,unsigned char cParity)
{
	m_Devs[channel].cBaudrate = cBaudrate;
	m_Devs[channel].cDatabits = cDatabits;
	m_Devs[channel].cStopbits = cStopbits;
	m_Devs[channel].cParity = cParity;    
}

void CPTZ::SetProtocolVariable(int channel,unsigned char cAddr,unsigned char cSpeedPan,
                    	unsigned char cSpeedTilt,unsigned char cSpeedZoom,unsigned char cPos)
{
	m_Devs[channel].cAddr = cAddr;
	m_Devs[channel].cSpeedPan = cSpeedPan;
	m_Devs[channel].cSpeedTilt = cSpeedTilt;
	m_Devs[channel].cSpeedZoom = cSpeedZoom;
	m_Devs[channel].cPos = cPos;
}    

int CPTZ::GetDevParam(LPDEV_PARAM lpDevs)
{
	if(lpDevs==NULL) return -1;
	memcpy(lpDevs,m_Devs,sizeof(DEV_PARAM)*_MAX_PTZ);
	return 0;
}

char* CPTZ::List()
{
	static char DevList[4096];
	char tBuf[256];
	int i;

	memset(DevList,0,sizeof(DevList));
	for(i=0;i<_MAX_PTZ;i++) 
    {
    	if(m_Devs[i].DevName[0]) 
        {
        	sprintf(tBuf,"%s\n",m_Devs[i].DevName);
        	strcat(DevList,tBuf);
        } 
    	else 
        	break;
    }
	if(i) 
    {
    	sprintf(tBuf,"\n%d",m_nDev);
    	strcat(DevList,tBuf);
    }
	return DevList;
}


int CPTZ::SetParameters(int nDev, char* pParam)
{
	int nRet=0;
	unsigned char cVal;
	if(pParam==NULL || strlen(pParam)<5 || m_nTty==-1 || nDev!=m_nDev || nDev<0 || nDev>=_MAX_PTZ) 
        	return -1;
	cVal=(char)atoi(pParam+4);
	if(strncasecmp(pParam,"ADD=",4)==0) 
    {
    	if(cVal<=m_Devs[nDev].cAddrMax-m_Devs[nDev].cAddrMin) 
        	m_Devs[nDev].cAddr=m_Devs[nDev].cAddrMin+cVal;
    } 
	else if(strncasecmp(pParam,"POS=",4)==0) 
    {
    	if(cVal<=m_Devs[nDev].cPosMax-m_Devs[nDev].cPosMin) 
        	m_Devs[nDev].cPos=m_Devs[nDev].cPosMin+cVal;
    } 
	else if(cVal<=9) 
    {
    	if(strncasecmp(pParam,"PAN=",4)==0) 
        {
        	m_Devs[nDev].cSpeedPan=(unsigned char)((m_Devs[nDev].cSpeedPanMax-m_Devs[nDev].cSpeedPanMin+1)/10.0f*cVal+0.5f);
        } 
    	else if(strncasecmp(pParam,"TLT=",4)==0) 
        {
        	m_Devs[nDev].cSpeedTilt=(unsigned char)((m_Devs[nDev].cSpeedTiltMax-m_Devs[nDev].cSpeedTiltMin+1)/10.0f*cVal+0.5f);
        } 
    	else if(strncasecmp(pParam,"ZOM=",4)==0) 
        {
        	m_Devs[nDev].cSpeedZoom=(unsigned char)((m_Devs[nDev].cSpeedZoomMax-m_Devs[nDev].cSpeedZoomMin+1)/10.0f*cVal+0.5f);
        } 
    	else nRet=-1;
    } 
	else 
    	nRet=-1;
	return nRet;
}

int CPTZ::ExecCmd(char* pCmd)
{
	char tmpBuf[256];

	int nRet=-1;
    
	if(m_nTty!=-1 && pCmd && m_nDev>=0 && m_nDev<_MAX_PTZ) 
    {    
    	char cmd[10][16], *p=pCmd;
    	int i=0, j, nCmd=0, bRet=0;
    	while(1) 
        {
        	if(*p && *p!='|') 
            	cmd[nCmd][i++]=*p;
        	else 
            {
            	if(i) 
                	cmd[nCmd++][i]=0; 
            	i=0;
            	if(*p==0 || nCmd>=10) 
                	break;
            }
        	p++;
        }
        
    	for(j=0;j<nCmd;j++)
        {
        	for(i=0;i<_MAX_CMD_PER_PTZ;i++) 
            {
            	if(m_Devs[m_nDev].cmd[i].CmdName[0]==0) 
                {                    
                	i=_MAX_CMD_PER_PTZ;                    
                	break;
                }
            	if(strcmp(m_Devs[m_nDev].cmd[i].CmdName,cmd[j])==0) 
                {
                	int k, n, x, p1, p2, val;
                	char data[256], Xx[16], Vv[16], Ww[16], Zz[16], Pp[16];

                	if(j && bRet && !m_Devs[m_nDev].bWaitResponse) sleep(1); // wait 1s for next
                    // format command
                	strcpy(data,(char*)m_Devs[m_nDev].cmd[i].Cmd);
                	n=strlen(data);
                    // remove spaces
                	for(k=0;k<n;k++) 
                    {
                    	if(data[k]==' ') 
                        {
                        	n--;
                        	if(n>k) memmove(data+k,data+k+1,n-k);
                        	k--;
                        }
                    }
                	data[n]=0;
                    // fill varibles
                	sprintf(Xx,"%02X",m_Devs[m_nDev].cAddr);
                	sprintf(Vv,"%02X",m_Devs[m_nDev].cSpeedPan);
                	sprintf(Ww,"%02X",m_Devs[m_nDev].cSpeedTilt);
                	sprintf(Zz,"%02X",m_Devs[m_nDev].cSpeedZoom);
                	sprintf(Pp,"%02X",m_Devs[m_nDev].cPos);
                	for(k=0;k<n;k++) 
                    {
                    	if(strncasecmp(data+k,"SUM(",4)==0 || strncasecmp(data+k,"XOR(",4)==0) k+=7;
                    	else if(data[k]=='X') data[k]=Xx[0];
                    	else if(data[k]=='x') data[k]=Xx[1];
                    	else if(data[k]=='V') data[k]=Vv[0];
                    	else if(data[k]=='v') data[k]=Vv[1];
                    	else if(data[k]=='W') data[k]=Ww[0];
                    	else if(data[k]=='w') data[k]=Ww[1];
                    	else if(data[k]=='Z') data[k]=Zz[0];
                    	else if(data[k]=='z') data[k]=Zz[1];
                    	else if(data[k]=='P') data[k]=Pp[0];
                    	else if(data[k]=='p') data[k]=Pp[1];
                    	else if(((data[k]>='0') && (data[k]<='9')) || ((data[k]>='A') && (data[k]<='F')) || ((data[k]>='a') && (data[k]<='f'))) ;
                    	else 
                        {
                        	n=0;
                        	bRet=0;
                        	break;
                        }
                    }
                	x=0;
                	for(k=0;k<n;k+=2) 
                    {
                    	if(strncasecmp(data+k,"SUM(",4)==0 || strncasecmp(data+k,"XOR(",4)==0) 
                        {
                        	if(k!=x) 
                            	memcpy(data+x,data+k,8);
                        	x+=8;
                        	k+=6;
                        } 
                    	else 
                        {
                        	memcpy(Xx,data+k,2);
                        	Xx[2]=0;
                        	data[x++]=strtoul(Xx,NULL,16);
                        }
                    }
                	n=x;
                	x=0;
                    // calculate function
                	for(k=0;k<n;k++) 
                    {
                    	if(strncasecmp(data+k,"SUM(",4)==0) 
                        {
                        	p1=atoi(data+k+4);
                        	p2=atoi(data+k+6);
                        	val=data[p1];
                        	for(int m=p1+1;m<=p2;m++) 
                            	val+=data[m];
                        	data[x++]=val;
                        	n-=7;
                        	if(n>x) 
                            	memmove(data+x,data+k+8,n-x);
                        } 
                    	else if(strncasecmp(data+k,"XOR(",4)==0) 
                        {
                        	p1=atoi(data+k+4);
                        	p2=atoi(data+k+6);
                        	val=data[p1];
                        	for(int m=p1+1;m<=p2;m++) val^=data[m];
                        	data[x++]=val;
                        	n-=7;
                        	if(n>x) memmove(data+x,data+k+8,n-x);
                        } 
                    	else 
                        {
                        	if(x!=k) 
                            	data[x]=data[k];
                        	x++;
                        }
                    }
                	n=x;

                	if(n) 
                    {
                    	PrintfPtzCmd(m_nDev,data,n);                        
                    	if(Writen(m_nTty,data,n) != n)
                        {
                        	SVPrint("write() error:%s\r\n",STRERROR_ERRNO);
                        }
                        
                    	for(int ii=0;ii<n;ii++)
                        {
                        	FiPrint2("data[%d]:%02X\r\n",ii,data[ii]);
                        }
                        
                    	if(m_Devs[m_nDev].bWaitResponse) 
                        {
                        	fd_set r, e;
                        	struct timeval tv;

                        	FD_ZERO(&r); FD_ZERO(&e);
                        	FD_SET(m_nTty,&r); FD_SET(m_nTty,&e);
                        	tv.tv_sec=1; tv.tv_usec=0; // wait 1s
                        	int ret=select(m_nTty+1,&r,NULL,&e,&tv);
                        	if(ret>0 && FD_ISSET(m_nTty,&r)) 
                            {
                            	bRet=1;
                            	usleep(20*1000); // wait 20ms for all response data
                            	read(m_nTty,tmpBuf,250);
                            } 
                        	else 
                            	bRet=0;
                        } 
                    	else 
                        	bRet=1;
                    	nRet=0;
                    } 
                	else 
                    	bRet=0;
                	break;
                } //if(strcmp(m_Devs[m_nDev].cmd[i].CmdName,cmd[j])==0) 
            } //for(i=0;i<_MAX_CMD_PER_PTZ;i++) 
        	if(i>=_MAX_CMD_PER_PTZ) 
            { // NOT FOUND
            	for(i=0;i<_MAX_CMD_PER_PTZ;i++) 
                {
                	if(m_Devs[m_nDev].cmd[i].CmdName[0]==0) 
                    {
                    	i=_MAX_CMD_PER_PTZ;
                    	break;
                    }
                	if(strcasecmp(m_Devs[m_nDev].cmd[i].CmdName,cmd[j])==0) 
                    {
                    	ExecCmd(m_Devs[m_nDev].cmd[i].CmdName);
                    	if(strcmp(m_Devs[m_nDev].cmd[i].CmdName,"home")==0 ||
                          strcmp(m_Devs[m_nDev].cmd[i].CmdName,"left")==0 ||
                          strcmp(m_Devs[m_nDev].cmd[i].CmdName,"right")==0 ||
                          strcmp(m_Devs[m_nDev].cmd[i].CmdName,"up")==0 ||
                          strcmp(m_Devs[m_nDev].cmd[i].CmdName,"down")==0) 
                        {
                        	usleep(500000);
                        	ExecCmd((char *)"STOP");
                        } 
                    	else if(strcmp(m_Devs[m_nDev].cmd[i].CmdName,"tele")==0 ||
                          strcmp(m_Devs[m_nDev].cmd[i].CmdName,"wide")==0) 
                        {
                        	usleep(500000);
                        	ExecCmd((char *)"ZSTOP");
                        } 
                    	else if(strcmp(m_Devs[m_nDev].cmd[i].CmdName,"near")==0 ||
                          strcmp(m_Devs[m_nDev].cmd[i].CmdName,"far")==0) 
                        {
                        	usleep(500000);
                        	ExecCmd((char *)"FSTOP");
                        }
                    	nRet=0;
                    	break;
                    }
                }
            } //if(i>=_MAX_CMD_PER_PTZ) 
        	if(i>=_MAX_CMD_PER_PTZ) 
            {
            	bRet=0;
            }
        }
    }
	else
    {
    	SVPrint( "(m_nTty=%d, pCmd=%s, m_nDev = %d)!\r\n", m_nTty, pCmd, m_nDev );
    }
    
	return nRet;    
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
#if 0
CPort::CPort()
{
	m_nTty=-1;
}

CPort::~CPort()
{
	Close();
}

int CPort::GetHandle()
{
	return m_nTty;
}

int CPort::Open(unsigned char cBaudrate, 
            	unsigned char cDatabits, 
            	unsigned char cStopbits, 
            	unsigned char cParity, 
            	char* pDev)
{
    //char dBuf[256];

	Close();

	if(strcmp(pDev,PORT_RS232)==0) {
    	printf("Message output disabled for PORT busy !");
    	usleep(200*1000);
    }

	m_nTty=open(pDev,O_RDWR);
	if(m_nTty!=-1) {
    	printf("open %s ok !",pDev);
        
    	tcgetattr(m_nTty,&m_termios);
    	ioctl(m_nTty,TIOCMGET,&m_tiocm);

        // init COMx port
    	struct termios options;
    	int tiocm;

    	tcflag_t BAUDRATE[] = { B110, B300, B600, B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200 };
    	tcflag_t DATABITS[] = { CS5, CS6, CS7, CS8 };
    	tcflag_t STOPBITS[] = { 0, CSTOPB };
    	tcflag_t PARITY[] = { 0, PARENB | PARODD, PARENB };

    	memcpy(&options,&m_termios,sizeof(options));
    	cfsetispeed(&options,BAUDRATE[cBaudrate]);
    	cfsetospeed(&options,BAUDRATE[cBaudrate]);
    	options.c_lflag &= ~(ECHO|ICANON|IEXTEN|ISIG|NOFLSH);
    	options.c_cflag &= ~CSIZE;
    	options.c_cflag |= DATABITS[cDatabits];
    	options.c_cflag |= STOPBITS[cStopbits];
    	options.c_cflag &= ~(PARENB | PARODD);
    	options.c_cflag |= PARITY[cParity];
    	options.c_cflag &= ~CRTSCTS;
    	options.c_iflag &= ~(BRKINT|ICRNL|ISTRIP);
    	options.c_iflag &= ~(IXON|IXOFF); // no XOR/XOFF flow control
    	options.c_cc[VMIN]=1;
    	options.c_cc[VTIME]=0;
    	options.c_line=0;

    	tcsetattr(m_nTty,TCSANOW,&options);

    	tiocm = m_tiocm;
    	tiocm |= TIOCM_DTR | TIOCM_RTS;
    	tiocm &= ~(TIOCM_DSR | TIOCM_CTS);
    	ioctl(m_nTty,TIOCMSET,&tiocm);

    	return 0;
    } else {
    	printf("open %s fail !",pDev);
    	return -1;
    }
}

void CPort::Close()
{
	if(m_nTty!=-1) {
    	ioctl(m_nTty,TIOCMSET,&m_tiocm);
    	tcsetattr(m_nTty,TCSANOW,&m_termios);
    	close(m_nTty);
    	m_nTty=-1;

    	printf("Message output enabled for PORT free !");
    }
}
#endif
static ptz_control_t g_ptzControl[MAX_CHANNEL_NUM];

static int CheckPtzCmd(int channel,char *pCmd,int param)
{    
	if(NULL != pCmd)
    {    
    	if(	0==strcasecmp(pCmd,"POS")    ||
        	0==strcasecmp(pCmd,"GOTO")    ||
        	0==strcasecmp(pCmd,"CLEAR"))
        {
        	g_ptzCtrlMutex.Lock();
        	g_ptzControl[channel].pro.cPos = param;
        	g_ptzCtrlMutex.Unlock();
        	g_Ptz.SetProtocolVariable(channel,g_ptzControl[channel].pro.cAddr,
                                      g_ptzControl[channel].pro.cSpeedPan,
                                      g_ptzControl[channel].pro.cSpeedTilt,
                                      g_ptzControl[channel].pro.cSpeedZoom,
                                      g_ptzControl[channel].pro.cPos);
        }
    }
    
	return FI_SUCCESSFUL;
}

static int PrintfPtzCmd(int channel,char *cmd,int cmdLen)
{
	int i;
	int size = 0;
	char pBuf[384] = {0};
    
	if(NULL!=cmd && cmdLen<(128))
    {
    	for(i=0;i<cmdLen;i++)
        {
        	sprintf(&pBuf[size],"%02X ",cmd[i]);
        	size += 3;
        }    
    }
	SVPrint("ch(%d) send ptz cmd( %s)!\r\n",channel,pBuf);

	return 0;
}

/*
* 执行PTZ命令
* channel:通道;pCmd:类似于InitSysConfigPtzProtocol()里面的命令
* param:命令所带的参数,如果该命令不带参数,一般设为0;
*/
int FiPtzExecCmd(int channel,char *pCmd,int param)
{
	int ret = -1;
	if(!FiPtzGetComFlag())
    {
    	SVPrint("ptz serial is not open!\r\n");
    	return -1;
    }
	if(NULL == pCmd)
    {
    	SVPrint("error:NULL == pCmd!\r\n");
    	return -1;
    }
	SVPrint("ptz:ch(%d),cmd(%s),param(%d)\r\n",channel,pCmd,param);
	CheckPtzCmd(channel,pCmd,param);
	ret = g_Ptz.SelectDev(channel); //把相应通道的参数设到串口中	
	if(0 == ret)
    {
    	ret = g_Ptz.ExecCmd(pCmd);        //向串口发命令
    }

	return ret;
}

static int OpenPtzSerial(char* pDev)
{
	return g_Ptz.Open(pDev);
}

int FiPtzSevParam(LPDEV_PARAM lpDevs)
{
	return g_Ptz.SetDevParam(lpDevs);
}

#if 1 // TODO
static void GetChannelControlStruct(int channel,ptz_control_t *temp)
{
    *temp = g_ptzControl[channel];
}

static int SetChannelControlParam(int channel)
{
	ptz_control_t ptzCtrl;
    
	GetChannelControlStruct(channel,&ptzCtrl);
	g_ptzCtrlMutex.Lock();
	g_Ptz.SetSerial(channel,ptzCtrl.com.baudrate,ptzCtrl.com.databits,
                	ptzCtrl.com.stopbits,ptzCtrl.com.parity);
	g_Ptz.SetProtocolVariable(channel,ptzCtrl.pro.cAddr,ptzCtrl.pro.cSpeedPan,ptzCtrl.pro.cSpeedTilt,
                	ptzCtrl.pro.cSpeedZoom,ptzCtrl.pro.cPos);    
	g_ptzCtrlMutex.Unlock();                
	return 0;    
}

static void DefaultPtzChannelControl(void)
{
	int i;
	g_ptzCtrlMutex.Lock();
	for(i=0;i<MAX_CHANNEL_NUM;i++)
    {
    	strcpy(g_ptzControl[i].DevName,"PELCO-D");
    	g_ptzControl[i].com.baudrate     = eB9600;
    	g_ptzControl[i].com.databits     = eCS8;
    	g_ptzControl[i].com.stopbits     = eSTOP1;
    	g_ptzControl[i].com.parity	    = ePARNONE;
    	g_ptzControl[i].pro.cAddr         = 0x01;
    	g_ptzControl[i].pro.cSpeedPan     = 0x3f;
    	g_ptzControl[i].pro.cSpeedTilt     = 0x3f;
    	g_ptzControl[i].pro.cSpeedZoom     = 0x00;
    	g_ptzControl[i].pro.cPos         = 0x01;        
    }
	g_ptzCtrlMutex.Unlock();
}

#if 0

const unsigned int g_baudrate[16] = {110,300,600,1200,2400,4800,9600,19200,38400,57600,115200};
static int ParseBaudrate(unsigned int val)
{
	int i;
	int retVal = eB9600;
    
	for(i=0;i<=eB115200;i++)
    {
    	if(g_baudrate[i] == val)
        {
        	retVal = i;
        	break;
        }
    }
	return retVal;
}

const unsigned char g_databits[4] = {5,6,7,8};
static int ParseDatabits(unsigned char val)
{
	int i;
	int retVal = eCS8;
    
	for(i=0;i<=eCS8;i++)
    {
    	if(g_databits[i] == val)
        {
        	retVal = i;
        	break;
        }    
    }
	return retVal;
}

const unsigned char g_stopbits[2] = {1,2};
static int ParseStopBits(unsigned char val)
{
	int i;
	int retVal = eSTOP1;
    
	for(i=0;i<=eSTOP2;i++)
    {
    	if(g_stopbits[i] == val)
        {
        	retVal = i;
        	break;
        }    
    }
	return retVal;
}

const unsigned char g_parity[3] = {0,1,2};
static int ParseParity(unsigned char val)
{
	int i;
	int retVal = ePARNONE;
    
	for(i=0;i<=ePAREVEN;i++)
    {
    	if(g_parity[i] == val)
        {
        	retVal = i;
        	break;
        }    
    }
	return retVal;
}
#endif
static int SetChnCurProtocol(int channel)
{
#if 1
	int i;
	int ret = FI_FAILED;
	struct DEV_PARAM curProtocol;
	SYS_CONFIG_PTZ_PROTOCOL ptzProtocol;
    
    //ret = CfgManageGetPtzProtocol(g_ptzControl[channel].DevName,&ptzProtocol);
	if(ret == 0)
    {
    	memcpy(curProtocol.DevName,ptzProtocol.ptzName,sizeof(curProtocol.DevName));
    	curProtocol.bWaitResponse = ptzProtocol.bWaitResponse;
    	curProtocol.cSpeedPan     = ptzProtocol.cSpeedPanDefault;
    	curProtocol.cSpeedTilt     = ptzProtocol.cSpeedTiltDefault;
    	curProtocol.cSpeedZoom	= ptzProtocol.cSpeedZoomDefault;
    	curProtocol.cPos             = ptzProtocol.cPosDefault;
    	curProtocol.cSpeedPanMin     = ptzProtocol.cSpeedPanMin;
    	curProtocol.cSpeedPanMax  = ptzProtocol.cSpeedPanMax;
    	curProtocol.cSpeedTiltMin = ptzProtocol.cSpeedTiltMin;
    	curProtocol.cSpeedTiltMax = ptzProtocol.cSpeedTiltMax;
    	curProtocol.cSpeedZoomMin = ptzProtocol.cSpeedZoomMin;
    	curProtocol.cSpeedZoomMax = ptzProtocol.cSpeedZoomMax;
    	curProtocol.cPosMin         = ptzProtocol.cPosMin;
    	curProtocol.cPosMax	    = ptzProtocol.cPosMax;
    	curProtocol.bIsManual	    = ptzProtocol.bIsManual;
    	for(i=0;i<_MAX_CMD_PER_PTZ;i++)
        {
        	memcpy(curProtocol.cmd[i].CmdName,ptzProtocol.cmd[i].cmdName,sizeof(curProtocol.cmd[i].CmdName));
        	memcpy(curProtocol.cmd[i].Cmd,ptzProtocol.cmd[i].cmd,sizeof(curProtocol.cmd[i].Cmd));
        	curProtocol.cmd[i].bIsManual = ptzProtocol.cmd[i].bIsManual;            
        }
    	g_ptzCtrlMutex.Lock();
    	g_Ptz.SetChannelCurProtocol(channel,&curProtocol);
    	g_ptzCtrlMutex.Unlock();
    }

	return ret;    
#endif //
	return 0;
}
#endif

#if 1 // TODO
int FiPtzSetChnPtzControlParam(int channel,void *ptzCtrl)
{
	int ret = FI_SUCCESSFUL;
    
	if(channel<0 || channel>MAX_CHANNEL_NUM)
    {
    	SVPrint("failed:set channel(%d) ptz parameter failed\r\n",channel);
    	return FI_FAILED;
    }
#if 0	
	g_ptzCtrlMutex.Lock();
	strncpy(g_ptzControl[channel].DevName,ptzCtrl->ptzName,sizeof(g_ptzControl[channel].DevName));
	g_ptzControl[channel].DevName[sizeof(g_ptzControl[channel].DevName)-1] = '\0';
    
	g_ptzControl[channel].com.baudrate      = 6;//(PTZ_SERIAL_BAUDRATE)ParseBaudrate(ptzCtrl->baudRate);
	g_ptzControl[channel].com.databits   = 3;//(PTZ_SERIAL_DATABITS)ParseDatabits(ptzCtrl->dataBits);
	g_ptzControl[channel].com.stopbits      = 0;//(PTZ_SERIAL_STOPBITS)ParseStopBits(ptzCtrl->stopBits);
	g_ptzControl[channel].com.parity      = 0;//(PTZ_SERIAL_PARITY)ParseParity(ptzCtrl->parity);

	g_ptzControl[channel].pro.cAddr      = ptzCtrl->cAddr;
	g_ptzControl[channel].pro.cSpeedPan  = ptzCtrl->cSpeedPan;
	g_ptzControl[channel].pro.cSpeedTilt = ptzCtrl->cSpeedTilt;
	g_ptzControl[channel].pro.cSpeedZoom = ptzCtrl->cSpeedZoom;
	g_ptzControl[channel].pro.cPos          = ptzCtrl->cPos;    
	g_ptzCtrlMutex.Unlock();    
    
#endif
	SetChnCurProtocol(channel);
	SetChannelControlParam(channel);
	return ret;
}
#endif
static int InitPtzCtrlParam(void)
{
	int i;
    //int ret = FI_FAILED;
    //PARAM_CONFIG_PTZ_CONTROL ptzCtrl;
    
	DefaultPtzChannelControl();    
#if 1// TODO
	for(i=0;i<REAL_CHANNEL_NUM;i++)
    {
        //ret = CfgManageGetPtzControl(i,&ptzCtrl);
        //if(FI_SUCCESSFUL == ret)
        {
        	FiPtzSetChnPtzControlParam(i, NULL);            
        }        
    }    
#endif
	return 0;
}

static int g_ptzComFlag = 0;
int FiPtzGetComFlag()
{
	return g_ptzComFlag;
}
int FiPtzSetComFlag(int val)
{
	g_ptzCtrlMutex.Lock();
	g_ptzComFlag = val;
	g_ptzCtrlMutex.Unlock();

	return 0;
}

void FiPtzInit()
{
	int ret = FI_FAILED;    
    
	InitPtzCtrlParam();    
	ret = OpenPtzSerial((char *)DEFAULT_PTZ_SERIAL);    
	if(0 == ret) FiPtzSetComFlag(1);
}

