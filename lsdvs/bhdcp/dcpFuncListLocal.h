#ifndef __DPFUNCLISTLOCAL_H__
#define __DPFUNCLISTLOCAL_H__

#include "message.h"

int DflIavailableMsgType( MSG_CMD_T *pLocalMsg, CLIENT_CONNECT_T *pClient );
int	DflForward( MSG_CMD_T *pLocalMsg, CLIENT_CONNECT_T *pClient );
int DflAlarmUpload( MSG_CMD_T *pLocalMsg, CLIENT_CONNECT_T *pClient );
int DflHddStatErrReport( MSG_CMD_T *pLocalMsg, CLIENT_CONNECT_T *pClient );
int DflParamChangedUpload( MSG_CMD_T *pLocalMsg, CLIENT_CONNECT_T *pClient );

#endif 

