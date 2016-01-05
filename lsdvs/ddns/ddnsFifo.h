#ifndef __DDNSFIFO_H__
#define __DDNSFIFO_H__

#define DDNS_FIFO_DEVICE "/tmp/ddns.fifo"

typedef enum _DdnsFifoCmd_
{
	DDNS_FIFO_CMD_START = 11,    // 启动
	DDNS_FIFO_CMD_STOP,            // 停止
	DDNS_FIFO_CMD_RESTART,        // 重启	
	DDNS_FIFO_CMD_REFREASH,        // 刷新,我们把enable 直接写入/tmp/ddns.conf,由ddns.sh 自己决定是否启动ddns
} DDNS_FIFO_CMD_EN;

int DdnsFifoCmd( DDNS_FIFO_CMD_EN cmd );
int DdnsWriteConff( char *pathname );

#endif 

