// vi: ts=4

#ifndef __PTZ_H__
#define __PTZ_H__

#include <termios.h>
#include <sys/ioctl.h>
#include <string.h>

#include "const.h"
/////////////////////////////////////////////////////////////////////////
#ifndef PACK_ALIGN
#define PACK_ALIGN __attribute__((packed))
#endif

#define MAX_PTZ_NUM	                    	30
#define MAX_CMD_PER_PTZ	                	30
//
// 云台协议配置
//
typedef struct PtzControlCmd 
{ 
	char	    	cmdName[16];        // 命令名称
	unsigned char	cmd[64];            // 每个命令最大62字节 (以 '\0' 结束)
	unsigned char	bIsManual;            // 是否用户自定义命令
} PACK_ALIGN PTZ_CONTROL_CMD;

typedef struct SysConfigPtzProtocol
{
	char	    	ptzName[32];        // 协议名称
	unsigned char	bWaitResponse;        // 设备对命令是否应答
	unsigned char	cSpeedPanDefault;    // 水平速度
	unsigned char	cSpeedPanMin;
	unsigned char	cSpeedPanMax;
	unsigned char	cSpeedTiltDefault;    // 垂直速度
	unsigned char	cSpeedTiltMin;
	unsigned char	cSpeedTiltMax;
	unsigned char	cSpeedZoomDefault;    // 放大缩小速度
	unsigned char	cSpeedZoomMin;
	unsigned char	cSpeedZoomMax;
	unsigned char	cPosDefault;        // 预置位
	unsigned char	cPosMin;
	unsigned char	cPosMax;
	PTZ_CONTROL_CMD	cmd[MAX_CMD_PER_PTZ];
	unsigned char	bIsManual;            // 是否用户自定义协议
    
} PACK_ALIGN SYS_CONFIG_PTZ_PROTOCOL;

//
// 云台控制配置
//
typedef struct SysConfigPtzControl
{
	char	    	ptzName[32];        // 协议名称
	unsigned int	baudRate;            // 波特率
	unsigned char	dataBits;            // 数据位
	unsigned char	stopBits;            // 停止位
	unsigned char	parity;                // 校验位
	unsigned char	cAddr;                // 地址位
	unsigned char	cSpeedPan;            // 当前水平速度
	unsigned char	cSpeedTilt;            // 当前垂直速度
	unsigned char	cSpeedZoom;            // 当前放大缩小速度
	unsigned char	cPos;                // 当前预置位
} PACK_ALIGN SYS_CONFIG_PTZ_CONTROL;
#undef PACK_ALIGN
/////////////////////////////////////////////////////////////////////////
#define BOOL int

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

#define _MAX_PTZ	    	30
#define _MAX_CMD_PER_PTZ  	30

#define PORT_NO_RS232	0
#define PORT_NO_RS485	1

#define PORT_RS232	(char*)"/dev/ttyAMA1"
#define PORT_RS485	(char*)"/dev/ttyAMA1"

#define DEFAULT_PTZ_SERIAL	"/dev/ttyAMA1" 

typedef enum _E_BAUDRATE 
{ 
	eB110, 
	eB300, 
	eB600, 
	eB1200, 
	eB2400, 
	eB4800, 
	eB9600, 
	eB19200, 
	eB38400, 
	eB57600, 
	eB115200 
}PTZ_SERIAL_BAUDRATE;
typedef enum _E_DATABITS 
{ 
	eCS5, 
	eCS6, 
	eCS7, 
	eCS8 
}PTZ_SERIAL_DATABITS;
typedef enum _E_STOPBITS 
{
	eSTOP1, 
	eSTOP2 
}PTZ_SERIAL_STOPBITS;
typedef enum _E_PARITY 
{ 
	ePARNONE, 
	ePARODD, 
	ePAREVEN 
}PTZ_SERIAL_PARITY;

typedef enum __SPEED 
{
	PTZ_SPEED_PAN = 0,
	PTZ_SPEED_TITL,
	PTZ_SPEED_ZOOM
}PTZ_SPEED;

// 云台控制命令
typedef struct __CMD 
{ 
	char CmdName[16]; // 命令名称
	unsigned char Cmd[64]; // 每个命令最大62字节 (以 '\0' 结束)
	unsigned char bIsManual; // 是否用户自定义命令
} PTZ_CMD; // 每种协议最多支持64命令

typedef struct DEV_PARAM
{
	char DevName[32]; // 协议名称
	unsigned char cBaudrate, cDatabits, cStopbits, cParity; // 端口设置
	unsigned char bWaitResponse; // 设备对命令是否应答
	unsigned char cAddr, cSpeedPan, cSpeedTilt, cSpeedZoom, cPos; // 协议参数
	unsigned char cAddrMin, cAddrMax, cSpeedPanMin, cSpeedPanMax, cSpeedTiltMin, cSpeedTiltMax,
    	cSpeedZoomMin, cSpeedZoomMax, cPosMin, cPosMax;
	PTZ_CMD cmd[_MAX_CMD_PER_PTZ];
	unsigned char bIsManual; // 是否用户自定义协议
} * LPDEV_PARAM;

typedef struct __ptz_serial
{
	PTZ_SERIAL_BAUDRATE baudrate;
	PTZ_SERIAL_DATABITS databits;
	PTZ_SERIAL_STOPBITS stopbits;
	PTZ_SERIAL_PARITY 	parity;
}ptz_serial_t;

typedef struct __ptz_protocol
{    
	unsigned char cAddr;
	unsigned char cSpeedPan;
	unsigned char cSpeedTilt;
	unsigned char cSpeedZoom;
	unsigned char cPos;
}ptz_protocol_t;

typedef struct __ptz_control
{
	char DevName[32]; // 协议名称
	ptz_serial_t com;//about串口配置
	ptz_protocol_t pro;//about协议配置	
}ptz_control_t;

class CPTZ
{
	DEV_PARAM m_Devs[_MAX_PTZ];
	int m_nTty, m_nDev, m_tiocm;
	struct termios m_termios;
	BOOL m_bUseRs232;

public:
	CPTZ();
    ~CPTZ();

	int Open(char* pPort=PORT_RS485);
	void Close();
	int SelectDev(int nDev=0);
	int GetDev();
	int GetDevParam(LPDEV_PARAM lpDevs);
	int SetDevParam(LPDEV_PARAM lpDevs);
	void SetChannelCurProtocol(int channel,LPDEV_PARAM lpDevs);
	void SetSerial(int channel,unsigned char cBaudrate,unsigned char cDatabits,
                	unsigned char cStopbits,unsigned char cParity);
	void SetProtocolVariable(int channel,unsigned char cAddr,unsigned char cSpeedPan,
                    	unsigned char cSpeedTilt,unsigned char cSpeedZoom,unsigned char cPos);
	char* List(void);

	int SetParameters(int nDev, char* pParam);
	int ExecCmd(char* pCmd);
};
#if 0
class CPort
{
	int m_nTty, m_tiocm;
	struct termios m_termios;

public:
	CPort();
    ~CPort();

	int Open(char cBaudrate=eB9600, char cDatabits=eCS8, char cStopbits=eSTOP1, char cParity=ePARNONE, char* pPort=PORT_RS232);
	int GetHandle();
	void Close();
};
#endif
int FiPtzExecCmd(int channel,char *pCmd,int param);

void FiPtzInit();

void FiParamGetPtzProtocol(DEV_PARAM *temp);
//int FiPtzSetChnPtzControlParam(int channel,PARAM_CONFIG_PTZ_CONTROL *ptzCtrl); // TODO
int FiPtzGetComFlag();
int FiPtzSetComFlag(int val);


#endif // __PTZ_H__

