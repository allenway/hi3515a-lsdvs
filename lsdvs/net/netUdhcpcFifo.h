#ifndef __NETUDHCPCFIFO_H__
#define __NETUDHCPCFIFO_H__

#define UDHCPC_FIFO	"/tmp/udhcpc.fifo" // 与 ppp.sh 同步的fifo
typedef enum _UdhcpcFifoCmd_
{
	UDHCPC_FIFO_CMD_START = 1,
	UDHCPC_FIFO_CMD_STOP,
	UDHCPC_FIFO_CMD_RESTART
} UDHCPC_FIFO_CMD;

int UdhcpcForkCtrl( UDHCPC_FIFO_CMD cmd );

#endif

