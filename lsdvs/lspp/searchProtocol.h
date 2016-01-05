#ifndef _DEV_SEARCH_H
#define _DEV_SEARCH_H

enum MsgDevSearchPackType
{
	MSG_DEV_SEARCH_COM	                = 0xFF00,
};

enum MsgDevSearchSubType
{
	MSG_DEV_SEARCH_REQUEST	            = 0x01,        // 请求
	MSG_DEV_SEARCH_RESPONSE	            = 0x02,        // 应答
	MSG_DEV_SEARCH_ERROR	            = 0x03,        // 出错
};

#define PACK_ALIGN	__attribute__((packed))

//
// 设备搜索数据格式
//
typedef struct MessageDevSearch
{
	int		devType;            // 设备类型
	char	devTypeName[32];    // 设备类型名称
	char	devName[32];        // 设备名称
	int		channelNum;            // 通道数量
	char	ip[80];                // IP地址
	char	netmask[16];        // 子网掩码
	char	gateway[16];        // 网关
	char	dns[16];            // 主DNS
	char	dns2[16];            // 备用DNS
	char	mac[24];            // 物理地址	
	unsigned short	protocolPort;    // 消息协议端口
	unsigned short	httpPort;        // http端口
} PACK_ALIGN MESSAGE_DEV_SEARCH;

#undef PACK_ALIGN

int DealDevSearchProcess( unsigned char *dataBuf, int &dataLen, const int bufSize );
int CheckDevSearchProcess( unsigned char *dataBuf, int dataLen, int bufSize, int &offset );

#endif  // _DEV_SEARCH_H

