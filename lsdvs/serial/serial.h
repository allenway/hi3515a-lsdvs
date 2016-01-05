/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : for serial.cpp        
**  date           :  2014.10.15
**
**  version       :  1.0
**  author        :  sven
********************************************************************************/

#ifndef __SERIAL_H__
#define __SERIAL_H__

#define MAX_SERIAL_PACK_SIZE	(64*1024)
#define MAX_SERIAL_PACK_SIZE_EX	(2048)


#define SERIAL_DEV_RS232	    "/dev/ttyO4"    // RS232端口
#define SERIAL_DEV_RS485	    "/dev/ttyS2"    // RS485端口
#define SERIAL_DEV_PPP	        "/dev/ttyUSB0"    // 3G pppd 拨号端口(ppp.sh使用).
#define SERIAL_DEV_GPS	        "/dev/ttyUSB1"    // GPS 端口
#define SERIAL_DEV_THREEG_AT	"/dev/ttyUSB2"    // AT 指令端口

typedef enum _SerialType_
{
	SERIAL_TYPE_RS232 = 0,
	SERIAL_TYPE_RS485,
	SERIAL_TYPE_THREEG_AT,    // 3G AT指令集端口
	SERIAL_TYPE_GPS	        // GPS端口
} SERIAL_TYPE_EN;

typedef enum _SerialBaudrate_
{
	SERIAL_BAUDRATE_110 = 0,
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
}SERIAL_BAUDRATE_ST;

typedef enum _SerialDatabits_
{
	SERIAL_DATABITS_5 = 0,
	SERIAL_DATABITS_6,
	SERIAL_DATABITS_7,
	SERIAL_DATABITS_8	
}SERIAL_DATABITS_ST;

typedef enum _SerialStopbits_
{
	SERIAL_STOPBITS_1 = 0,
	SERIAL_STOPBITS_2
}SERIAL_STOPBITS_ST;

typedef enum _SerialParity_
{
	SERIAL_PARITY_NONE = 0,
	SERIAL_PARITY_ODD,
	SERIAL_PARITY_EVEN
}SERIAL_PARITY_ST;

// 串口参数 rs232 rs485
typedef struct RsParam
{
	unsigned int	baudRate;            // 波特率
	unsigned char	dataBits;            // 数据位
	unsigned char	stopBits;            // 停止位
	unsigned char	parity;                // 校验位
}RS_PARAM;


void FiSerialInit();
int FiSerialSetParam(int handle, SERIAL_BAUDRATE_ST baudrate,SERIAL_DATABITS_ST databits,
                    	SERIAL_STOPBITS_ST stopbits,SERIAL_PARITY_ST parity);
int FiSerialGetHandle( int type );

int FiSerialOpenRS485();
int FiSerialWriteRS485(char *buf, int len);
int FiSerialReadRS485(char *buf, int len);
void FiSerialCloseRS485();
void FiSerialCloseAllRS485();

int FiSerialSetParamRS485( int handle, RS_PARAM *rs485 );

// RS232
int FiSerialOpenRS232();
int FiSerialWriteRS232(char *buf, int len);
int FiSerialReadRS232(char *buf, int len);
void FiSerialCloseRS232();
void FiSerialCloseAllRS232();

// 3G专用串口
int FiSerialOpenThreeg();
int FiSerialWriteThreeg(char *buf, int len);
int FiSerialReadThreeg(char *buf, int len);
void FiSerialCloseThreeg();

// GPS专用串口
int FiSerialOpenGps();
int FiSerialWriteGps(char *buf, int len);
int FiSerialReadGps(char *buf, int len);
void FiSerialCloseGps();


int FiSerialSetParamThreeg( int handle, SERIAL_BAUDRATE_ST baudrate, 
                                  SERIAL_DATABITS_ST databits, 
                                  SERIAL_STOPBITS_ST stopbits,
                                  SERIAL_PARITY_ST parity );

#endif //__SERIAL_H__

