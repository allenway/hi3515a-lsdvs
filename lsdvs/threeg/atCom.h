/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : at 指令集
**  date           :  2014.9.25
**  version       :  1.0
**  author        :  sven
********************************************************************************/

#ifndef __ATCMD_H__
#define __ATCMD_H__

#include "sem.h"
#include <sys/times.h>

#define AT_CHARS_OK	    "OK"
#define AT_CALL_PREFIX	"ATD"

#define AT_KEY_SHORT_MSG_ENTRY	        ">"            // 短信输入关键字
#define AT_KEY_RING_NUMBER_DISPLAY	    "+CLIP:"    // 来电显示关键字
#define AT_KEY_RING	                    "RING"        // 电话打入关键字	
#define AT_KEY_SIGNAL_VALUE	            "+CSQLVL"    // 信号格数查询与回复关键字
#define AT_KEY_RECV_NEW_SMS	            "+CMTI:"    // 收到新短信命令
#define AT_KEY_RECV_SMS_CONTENT	        "+CMGR:"    // 收到短信内容

// for huawei wcdma
#define AT_KEY_SIGNAL_VALUE_HW	        "+CSQ"    // 信号格数查询与回复关键字


#define ATCMD_PRE	"AT"     
#define ATCMD_CR_LF	"\r\n"
#define ATCMD_INPUT	"> "

#define AT_WAIT_SEC		20
#define AT_WAIT_NSEC	0

#define ATCMD_SIZE		1024

typedef enum _AtRecvFlag_
{
	PRE_RECV_FLAG_NO = 0,    // 没有预接收
	PRE_RECV_FLAG_SYNC,        // 接收到需要同步的预接收
	PRE_RECV_FLAG_ASYC,        // 接收到不需要同步的预接收
	PRE_RECV_FLAG_SMS	    // 接收到短信的预接收
} AT_RECV_FLAG;

typedef struct _AtCmdSt_
{
	pthread_mutex_t		lock;                // 锁,主要是防止多个AT指令同时请求
	sem_t	        	sem;                // 线程同步
	char             	req[ATCMD_SIZE];    // AT发送命令缓冲区
	int	            	reqSize;            // 接收到的数据大小
	char             	rsp[ATCMD_SIZE];    // 从3G模块接收到的消息
	struct timeval		recvTimeout;        // 同步超时
	char	        	preRecvFlag;        // 在接收结束关键行(大部分为 \r\n"OK"\r\n) 前有没有接收其他行
} AT_CMD_ST;

extern AT_CMD_ST g_atCmd;

int AtCmdInitStruct( AT_CMD_ST *pAtCmd );
int AtCmdDeinitStruct( AT_CMD_ST *pAtCmd );
int AtCmdSend( AT_CMD_ST *pAtCmd );
int AtCmdSendSync( AT_CMD_ST *pAtCmd );
int AtCmdSendSync( AT_CMD_ST *pAtCmd, char *pKey );
int FiAtStrToUnicodeStr( char *pUnicode, char *srcStr );

#endif
 
