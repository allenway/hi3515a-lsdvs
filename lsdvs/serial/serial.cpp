/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 串口函数的封装            
**  date           :  2014.10.15
**
**  version       :  1.0
**  author        :  sven
********************************************************************************/

#include <termios.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "debug.h"
#include "mutex.h"
#include "serial.h"
#include "linuxFile.h"

typedef struct _SerialDev_
{
	int handle;
	int openCount;
}SERIAL_DEV_ST;

static SERIAL_DEV_ST g_RS232;
static SERIAL_DEV_ST g_RS485;
static SERIAL_DEV_ST g_RSTHREEG;
static SERIAL_DEV_ST g_RSGPS;

const unsigned int baudRate[] = {
    110, 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200
};

const SERIAL_BAUDRATE_ST serialBuadRate[]=
{
	SERIAL_BAUDRATE_110 ,
	SERIAL_BAUDRATE_300,
	SERIAL_BAUDRATE_600,
	SERIAL_BAUDRATE_1200,
	SERIAL_BAUDRATE_2400,
	SERIAL_BAUDRATE_4800,
	SERIAL_BAUDRATE_9600,
	SERIAL_BAUDRATE_19200,
	SERIAL_BAUDRATE_38400,
	SERIAL_BAUDRATE_57600,
	SERIAL_BAUDRATE_115200	
};


static int openSerialDev(int type)
{
	int ret = -1;
    
	if(SERIAL_TYPE_RS232 == type)
    {
    	if(-1 == g_RS232.handle)
        {
        	g_RS232.handle = open(SERIAL_DEV_RS232,O_RDWR|O_NDELAY);
        	if(-1 == g_RS232.handle)
            {
            	ret = -1;
            	SVPrint("open(%s) failed:%s\r\n",SERIAL_DEV_RS232,STRERROR_ERRNO);
            }
        	else
            {
            	ret = g_RS232.handle;            
            	g_RS232.openCount++;
            }
        }
    	else
        {
        	ret = g_RS232.handle;            
        	g_RS232.openCount++;
        }
    }
	else if(SERIAL_TYPE_RS485 == type)
    {
    	if(-1 == g_RS485.handle)
        {
        	g_RS485.handle = open(SERIAL_DEV_RS485,O_RDWR|O_NDELAY);
        	if(-1 == g_RS485.handle)
            {
            	ret = -1;
            	SVPrint("open(%s) failed:%s\r\n",SERIAL_DEV_RS485,STRERROR_ERRNO);
            }
        	else
            {
            	ret = g_RS485.handle;
            	g_RS485.openCount++;
            }
        }
    	else
        {
        	ret = g_RS485.handle;
        	g_RS485.openCount++;
        }
    }
	else if(SERIAL_TYPE_THREEG_AT == type)
    {
    	if(-1 == g_RSTHREEG.handle)
        {
        	g_RSTHREEG.handle = open(SERIAL_DEV_THREEG_AT, O_RDWR|O_NDELAY);
        	if(-1 == g_RSTHREEG.handle)
            {
            	ret = -1;
            	SVPrint("open(%s) failed:%s\r\n",SERIAL_DEV_THREEG_AT, STRERROR_ERRNO);
            }
        	else
            {
            	ret = g_RSTHREEG.handle;
            	g_RSTHREEG.openCount++;
            }
        }
    	else
        {
        	ret = g_RSTHREEG.handle;
        	g_RSTHREEG.openCount++;
        }
    }
	else if(SERIAL_TYPE_GPS == type)
    {
    	if(-1 == g_RSGPS.handle)
        {
        	g_RSGPS.handle = open(SERIAL_DEV_GPS, O_RDWR|O_NDELAY);
        	if(-1 == g_RSGPS.handle)
            {
            	ret = -1;
            	SVPrint("open(%s) failed:%s\r\n",SERIAL_DEV_GPS,STRERROR_ERRNO);
            }
        	else
            {
            	ret = g_RSGPS.handle;
            	g_RSGPS.openCount++;
            }
        }
    	else
        {
        	ret = g_RSGPS.handle;
        	g_RSGPS.openCount++;
        }
    }
    
	return ret;
}

static CMutexLock g_serialRS232Mutex;

/*********************************************************************
* function: 打开串口
* return   : >=0,成功, 串口的句柄; < 0,失败
*********************************************************************/
int FiSerialOpenRS232()
{
	int ret;
    
	g_serialRS232Mutex.Lock();
	ret = openSerialDev(SERIAL_TYPE_RS232);
	g_serialRS232Mutex.Unlock();

	return ret;
}

/**************************************************************************
* function: 对串口安全写
* return   : 写入的字节
***************************************************************************/
int FiSerialWriteRS232( char *buf, int len )
{
	int ret = -1;
    
	g_serialRS232Mutex.Lock();
    
	if(NULL != buf && len>0 && len <MAX_SERIAL_PACK_SIZE && -1 != g_RS232.handle)
    {
    	ret = Writen( g_RS232.handle, buf, len );
    }
	g_serialRS232Mutex.Unlock();

	return ret;
}

/******************************************************************************
* function: 串口读
* return   : 读到的字节数
********************************************************************************/
int FiSerialReadRS232( char *buf, int len )
{
	int ret = -1;
    
	g_serialRS232Mutex.Lock();
    
	if(NULL != buf && len>0 && len <MAX_SERIAL_PACK_SIZE && -1 != g_RS232.handle)
    {
    	ret = read(g_RS232.handle,buf,len);
    }
	g_serialRS232Mutex.Unlock();

	return ret;
}

void FiSerialCloseRS232()
{
	g_serialRS232Mutex.Lock();

	if(g_RS232.openCount > 0)
    {        
    	if((--g_RS232.openCount == 0) && (-1 != g_RS232.handle))
        {
        	close(g_RS232.handle);
        	g_RS232.handle = -1;
        }
    }
    
	g_serialRS232Mutex.Unlock();    
}

void FiSerialCloseAllRS232()
{
	while((g_RS232.handle != -1))
    {        
    	SVPrint("close rs232\n");
    	FiSerialCloseRS232();
    	sleep(1);
    }
}

static CMutexLock g_serialRS485Mutex;

int FiSerialOpenRS485()
{
	int ret;
    
	g_serialRS485Mutex.Lock();
	ret = openSerialDev(SERIAL_TYPE_RS485);
	g_serialRS485Mutex.Unlock();

	return ret;
}

int FiSerialWriteRS485(char *buf, int len)
{
	int ret = -1;
    
	g_serialRS485Mutex.Lock();
    
	if(NULL != buf && len>0 && len <MAX_SERIAL_PACK_SIZE && -1 != g_RS485.handle)
    {
    	ret = Writen( g_RS485.handle, buf, len );
    }
	g_serialRS485Mutex.Unlock();

	return ret;
}

int FiSerialReadRS485(char *buf, int len)
{
	int ret = -1;
    
	g_serialRS485Mutex.Lock();
    
	if(NULL != buf && len>0 && len <MAX_SERIAL_PACK_SIZE && -1 != g_RS485.handle)
    {
    	ret = read(g_RS485.handle,buf,len);
    }
	g_serialRS485Mutex.Unlock();

	return ret;
}

void FiSerialCloseRS485()
{
	g_serialRS485Mutex.Lock();    
	if(g_RS485.openCount > 0)
    {        
    	if((--g_RS485.openCount == 0) && (-1 != g_RS485.handle))
        {
        	close(g_RS485.handle);
        	g_RS485.handle = -1;
        }
    }
	g_serialRS485Mutex.Unlock();    
}

void FiSerialCloseAllRS485()
{
	while((g_RS485.handle != -1))
    {        
    	SVPrint("close rs485\n");
    	FiSerialCloseRS485();
    	sleep(1);
    }
}

static CMutexLock g_serialThreegMutex;

int FiSerialOpenThreeg()
{
	int ret;
    
	g_serialThreegMutex.Lock();
	ret = openSerialDev(SERIAL_TYPE_THREEG_AT);
	g_serialThreegMutex.Unlock();

	return ret;
}

/****************************************************************************
*	function	: 往3G模块的 at 的ttyUSB中写入数据
*	buf	       : 要写入的数据缓冲区
*	len          : 要写入的长度
*	return      : 已经写入的长度
******************************************************************************/
int FiSerialWriteThreeg(char *buf, int len)
{
	int ret = -1;
    
	g_serialThreegMutex.Lock();
    
	if(NULL != buf && len>0 && len <MAX_SERIAL_PACK_SIZE_EX && -1 != g_RSTHREEG.handle)
    {
    	ret = Writen( g_RSTHREEG.handle, buf, len );
    }
	g_serialThreegMutex.Unlock();

	return ret;
}

int FiSerialReadThreeg(char *buf, int len)
{
	int ret = -1;
    
	g_serialThreegMutex.Lock();
    
	if(NULL != buf && len>0 && len <MAX_SERIAL_PACK_SIZE_EX && -1 != g_RSTHREEG.handle)
    {
    	ret = read( g_RSTHREEG.handle, buf, len );
    }
	g_serialThreegMutex.Unlock();

	return ret;
}

void FiSerialCloseThreeg()
{
	g_serialThreegMutex.Lock();    
	if(g_RSTHREEG.openCount > 0)
    {        
    	if((--g_RSTHREEG.openCount == 0) && (-1 != g_RSTHREEG.handle))
        {
        	close(g_RSTHREEG.handle);
        	g_RSTHREEG.handle = -1;
        }
    }
	g_serialThreegMutex.Unlock();    
}

static CMutexLock g_serialGpsMutex;

int FiSerialOpenGps()
{
	int ret;
    
	g_serialGpsMutex.Lock();
	ret = openSerialDev(SERIAL_TYPE_GPS);
	g_serialGpsMutex.Unlock();

	return ret;
}

int FiSerialWriteGps(char *buf, int len)
{
	int ret = -1;
    
	g_serialGpsMutex.Lock();
    
	if(NULL != buf && len>0 && len <MAX_SERIAL_PACK_SIZE_EX && -1 != g_RSGPS.handle)
    {
    	ret = Writen( g_RSGPS.handle, buf, len );
    }
	g_serialGpsMutex.Unlock();

	return ret;
}

int FiSerialReadGps(char *buf, int len)
{
	int ret = -1;
    
	g_serialGpsMutex.Lock();
    
	if(NULL != buf && len>0 && len <MAX_SERIAL_PACK_SIZE_EX && -1 != g_RSGPS.handle)
    {
    	ret = read(g_RSGPS.handle,buf,len);
    }
	g_serialGpsMutex.Unlock();

	return ret;
}

void FiSerialCloseGps()
{
	g_serialGpsMutex.Lock();    
	if(g_RSGPS.openCount > 0)
    {        
    	if((--g_RSGPS.openCount == 0) && (-1 != g_RSGPS.handle))
        {
        	close(g_RSGPS.handle);
        	g_RSGPS.handle = -1;
        }
    }
	g_serialGpsMutex.Unlock();    
}

/*************************************************************************
* function : 设置串口参数
***************************************************************************/
int FiSerialSetParam( int handle, SERIAL_BAUDRATE_ST baudrate, 
                                  SERIAL_DATABITS_ST databits, 
                                  SERIAL_STOPBITS_ST stopbits,
                                  SERIAL_PARITY_ST parity )
{
	int ret = -1;
    
	if(handle != -1 ) 
    {
    	struct termios options;
    	int tiocm;

    	const tcflag_t BAUDRATE[] = { B110, B300, B600, B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200 };
    	const tcflag_t DATABITS[] = { CS5, CS6, CS7, CS8 };
    	const tcflag_t STOPBITS[] = { 0, CSTOPB };
    	const tcflag_t PARITY[] = { 0, PARENB | PARODD, PARENB };        
        
    	tcgetattr(handle,&options);
        
    	cfsetispeed(&options,BAUDRATE[baudrate]);
    	cfsetospeed(&options,BAUDRATE[baudrate]);
    	options.c_lflag &= ~(ECHO|ICANON|IEXTEN|ISIG|NOFLSH);
    	options.c_cflag &= ~CSIZE;
    	options.c_cflag |= DATABITS[databits];
    	options.c_cflag |= STOPBITS[stopbits];
    	options.c_cflag &= ~(PARENB | PARODD);
    	options.c_cflag |= PARITY[parity];
    	options.c_cflag &= ~CRTSCTS;
    	options.c_iflag &= ~(BRKINT|ICRNL|ISTRIP);
    	options.c_iflag &= ~(IXON|IXOFF); // no XOR/XOFF flow control
    	options.c_cc[VMIN]=1;
    	options.c_cc[VTIME]=0;
    	options.c_line=0;
    	options.c_iflag &= ~(INLCR | ICRNL | IGNCR);
    	options.c_oflag &= ~(ONLCR | OCRNL | ONOCR | ONLRET);        

    	tcsetattr(handle,TCSANOW,&options);

    	ioctl(handle,TIOCMGET,&tiocm);
        
    	tiocm |= TIOCM_DTR|TIOCM_RTS;
    	tiocm &= ~(TIOCM_DSR|TIOCM_CTS);
        
    	ioctl(handle,TIOCMSET,&tiocm);
    
    	ret = 0;

    	SVPrint("FiSerialSetParam:Baudrate(%d) Databits(%d) Stopbits(%d) Parity(%d)!\r\n",                    
                	baudrate,databits,stopbits,parity);
    } 
    
	return ret;
}


int FiSerialSetParamRS485( int handle, RS_PARAM *rs485 )
{
	int ret = -1;
	SERIAL_BAUDRATE_ST baudrate = SERIAL_BAUDRATE_9600; 
 	SERIAL_DATABITS_ST databits = SERIAL_DATABITS_8; 
  	SERIAL_STOPBITS_ST stopbits = SERIAL_STOPBITS_1;
  	SERIAL_PARITY_ST   parity	= SERIAL_PARITY_NONE;
	int baudRateNum = (int)(sizeof(baudRate) / sizeof(baudRate[0]));
	int i;    
        
	RS_PARAM *serialParam = (RS_PARAM *)rs485;
	if( NULL != serialParam )
    {

    	for ( i = 0; i < baudRateNum; i++ )
        {
        	if ( serialParam->baudRate == baudRate[i] )
            {
            	baudrate = serialBuadRate[i];
            	ret = 0;
            	break;
            } 
        }

    	switch( serialParam->dataBits )
        {
    	case 5:
        	databits = SERIAL_DATABITS_5;
        	break;
    	case 6:
        	databits = SERIAL_DATABITS_6;
        	break;
    	case 7:
        	databits = SERIAL_DATABITS_7;
        	break;
    	case 8:                
        	databits = SERIAL_DATABITS_8;
        	break;
    	default:
        	ret = -1;
        	break;
        }

    	switch( serialParam->stopBits )
        {
    	case 1:                
        	stopbits = SERIAL_STOPBITS_1;
        	break;
    	case 2:
        	stopbits = SERIAL_STOPBITS_2;
        	break;
    	default:
        	ret = -1;
        	break;
        }

    	switch( serialParam->parity )
        {
    	case 0:                
        	parity = SERIAL_PARITY_NONE;
        	break;
    	case 1:    
        	parity = SERIAL_PARITY_ODD;
        	break;
    	case 2:
        	parity = SERIAL_PARITY_EVEN;
        	break;
    	default:
        	ret = -1;
        	break;
        }

    	if( ret != -1 )
        {
        	ret = FiSerialSetParam( handle, baudrate, databits, stopbits, parity );
        }        
    }
	return ret;
}

/*******************************************************************
*	func	: 3G 模块 ttyUsb专用参数设置接口
*********************************************************************/
int FiSerialSetParamThreeg( int handle, SERIAL_BAUDRATE_ST baudrate, 
                                  SERIAL_DATABITS_ST databits, 
                                  SERIAL_STOPBITS_ST stopbits,
                                  SERIAL_PARITY_ST parity )
{
	int ret = -1;
    
	if(handle != -1 ) 
    {
    	struct termios options;
    	int tiocm;

    	const tcflag_t BAUDRATE[] = { B110, B300, B600, B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200 };
    	const tcflag_t DATABITS[] = { CS5, CS6, CS7, CS8 };
    	const tcflag_t STOPBITS[] = { 0, CSTOPB };
//    	const tcflag_t PARITY[] = { 0, PARENB | PARODD, PARENB };        
        
    	tcgetattr(handle,&options);
        
    	cfsetispeed(&options,BAUDRATE[baudrate]);
    	cfsetospeed(&options,BAUDRATE[baudrate]);
    	options.c_cflag &= ~CSIZE;
    	options.c_cflag |= DATABITS[databits];
    	options.c_cflag |= STOPBITS[stopbits];
    	options.c_line=0;

    	options.c_iflag =  IGNBRK;
    	options.c_lflag = 0;
    	options.c_oflag = 0;
    	options.c_cflag |= CLOCAL | CREAD;
    	options.c_cc[VMIN] = 1;
    	options.c_cc[VTIME] = 5;
    	options.c_iflag &= ~(IXON|IXOFF|IXANY);
    	options.c_cflag &= ~(PARENB | PARODD);
    	options.c_cflag &= ~CSTOPB;
    	tcsetattr(handle,TCSANOW,&options);
    	ioctl(handle,TIOCMGET,&tiocm);
        
    	tiocm |= TIOCM_DTR|TIOCM_RTS;
    	tiocm &= ~(TIOCM_DSR|TIOCM_CTS);
        
    	ioctl(handle,TIOCMSET,&tiocm);
    
    	ret = 0;

    	SVPrint("FiSerialSetParam:Baudrate(%d) Databits(%d) Stopbits(%d) Parity(%d)!\r\n",                    
                	baudrate,databits,stopbits,parity);
    } 
    
	return ret;
}



/*****************************************************************
* 获取某个串口句柄
* type: 见SERIAL_TYPE_EN
*******************************************************************/
int FiSerialGetHandle( int type )
{
	int handle = -1;
	switch( type )
    {
	case SERIAL_TYPE_RS232:
    	handle = g_RS232.handle;
    	break;
        
	case SERIAL_TYPE_RS485:
    	handle = g_RS485.handle;
    	break;
	case SERIAL_TYPE_THREEG_AT:
    	handle = g_RSTHREEG.handle;
    	break;
	case SERIAL_TYPE_GPS:        
    	handle = g_RSGPS.handle;
    	break;        
    }

	return handle;
}

/****************************************************************************************
* 系统启动时先要初始化串口句柄
******************************************************************************************/
void FiSerialInit()
{
	g_RS232.handle = -1;
	g_RS232.openCount = 0;

	g_RS485.handle = -1;
	g_RS485.openCount = 0;    
    
	g_RSTHREEG.handle = -1;
	g_RSTHREEG.openCount = 0;

	g_RSGPS.handle = -1;
	g_RSGPS.openCount = 0;
}

