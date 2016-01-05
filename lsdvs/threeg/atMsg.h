/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 发送短信接口
**  date           :  2014.9.25
**
**  version       :  1.0
**  author        :  sven
********************************************************************************/

#ifndef __ATMSG_H__
#define __ATMSG_H__

typedef enum _AtMsgFormat_
{
	AT_MSG_FORMAT_PDU = 0,    // PDU 方式收发短信
	AT_MSG_FORMAT_TEXT	    // TEST 方式收发短信
} AT_MSG_FORMAT;


#define ATCMD_MSG_CMGF	    "+CMGF"    // 设置短信的收发模式,pdu or text
#define ATCMD_MSG_CNMI	    "+CNMI"    // 设置接收短信的属性
#define ATCMD_MSG_CMGR	    "+CMGR"    // 读取某条短信命令
#define ATCMD_MSG_CMGD	    "+CMGD"    // 删除短信命令

int FiAtSetSmsFormat( AT_MSG_FORMAT format );
int FiAtSendSms( char *pPhone, char *pMsgConnect );
int FiAtSmsInit();
int FiAtSendReadSmsCmd( int smsIndex );
int FiAtDelSms( int index, int allFlag );

#endif

