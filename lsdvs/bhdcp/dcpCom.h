#ifndef __DCPCOM_H__
#define __DCPCOM_H__

#include "dcpIns.h"
#include "dcpTypes.h"

#define CLIENT_MSG_MARK		0x5AA58BB8
#define MAX_MSG_DATA_LEN    (100*1024)
#define MAX_NET_PACK_SIZE 	1400

#define SUPER_USER	    "shenzhenrha"
#define SUPER_PASSWD    "1asdrtyjkl1"


#define LISTEN_CLIENT_MSG_PORT		20000
#define LISTEN_CLIENT_STREAM_PORT	20001

typedef enum _ClientSocketStateEn_
{
	CLIENT_SOCKET_STATE_NONE	= 0,     // 空
	CLIENT_SOCKET_STATE_LISTEN,            // 监听
	CLIENT_SOCKET_STATE_ACCEPT,            // 已经被accept的, 但没有通过了用户名和密码登陆的
	CLIENT_COCKET_STATE_LOAD,            // 用户已经通过了用户名和密码登陆的
}CLIENT_SOCKET_STATE_EN;

#define MAX_CLIENT_SIZE	        	44//16(16->44,上位机有很多不明的连接，需要扩大连接数才行)  //最大支持的客户数量
#define MAX_CLIENT_SOCKET_NUM	    (MAX_CLIENT_SIZE + 1)    //还有一个socket用来存放listen的socket

typedef enum _DcpComErr_
{
	DCP_COM_ERR_NEED_CLOSE_SOCKET = -10000,    
	DCP_COM_ERR_NEED_MOVE_SOCKET,
} DCP_COM_ERR_EN;

#define MAX_SEND_DATA_SIZE	(1*1024)
typedef enum _SeqTypeEn_
{
	SEQ_TYPE_ONE_PACK = 0,     // 开始和结束都是同一个包,也就是只有一个包
	SEQ_TYPE_START,            // 开始包
	SEQ_TYPE_STOP,            // 结束包
	SEQ_TYPE_MID,            // 中间包
} SEQ_TYPE_EN;

typedef struct _SocketErr_
{
	int num;                                // 总共有多少个socket 出错了
	int socket[MAX_CLIENT_SOCKET_NUM];         // 出错了socket 的值
	int moveFlag[MAX_CLIENT_SOCKET_NUM];    // =1,表示该socket 并没有发生错误,而是被其他线程接手,不需要Close; 否则需要Close, 仅仅dcpss.cpp 中用到
} SOCKET_ERR_T;

#endif 

