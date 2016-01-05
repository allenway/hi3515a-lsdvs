#ifndef __ICMPAPP_H__
#define __ICMPAPP_H__

#include "const.h"

typedef struct _IcmpWorkParam_
{
	int 	enable;                    // 是否使能icmp
	char	ipAddr[NET_ADDRSIZE];    // 被ping 的ip 地址
	int		interval;                // 正常情况下每隔多长时间发送一次心跳包 5～100s
	int		timeOut;                // 发送ping 包后等待接收的超时时间 5～100s
	int		fcount;                    // failed count, ping不通的情况下ping 多少次也就是ping 的-c 参数了 5～100 次
	int		finterval;                // 存在虚链路时，发送ICMP包的间隔时间 10～100s
} ICMP_WORK_PARAM_T;

typedef enum _IcmpCurState_
{
	ICMP_CUR_STATE_UNKNOW = 0,        // 状态未知
	ICMP_CUR_STATE_PING_SUCCESS,    // ping 成功
	ICMP_CUR_STATE_PING_FAILED,        // ping 失败
	ICMP_CUR_STATE_PING_N_FAILED,    // 尝试了 n 次ping 后失败
} ICMP_CUR_STATE_EN;

typedef struct _IcmpWorkMaintain_
{
	int	pts;        // 时间戳,相对的
	int curState;    // 见 ICMP_CUR_STATE_EN
	int	fcount;        // failed count ping 失败的次数	
} ICMP_WORK_MAINTAIN_T;

void StartIcmpAppThread();
void StopIcmpAppThread();
void IcmpSendParamChangeMessage();
int IcmpGetNetworkState();

#endif 

