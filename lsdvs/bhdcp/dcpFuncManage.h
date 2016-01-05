#ifndef __DCPFUNCMANAGE_H__
#define __DCPFUNCMANAGE_H__

#include "dcpTypes.h"

// 处理网络指令的函数管理
typedef int (*DCP_CLIENT_COM_FUNC)( DCP_HEAD_T *msgHead, CLIENT_COMMUNICATE_T *clientCom );

typedef struct _DcpMsgFunc_
{
	unsigned short	    	msg;
	DCP_CLIENT_COM_FUNC		func;    
} DCP_MSG_FUNC_T;

DCP_CLIENT_COM_FUNC GetClientMsgFunction( unsigned short msg );

// 处理本地指令的函数管理
typedef int (*DCP_FUNC_LOCAL)( MSG_CMD_T *pLocalMsg, CLIENT_CONNECT_T *pClient );

typedef struct _DcpFuncLocal_
{
	unsigned short		msg;
	DCP_FUNC_LOCAL		func;    
} DCP_FUNC_LOCAL_T;

DCP_FUNC_LOCAL GetDcpFuncLocal( unsigned short msg );


#endif

