/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此头文件提供了对消息协议的包数据进行打包和解包操作的函数接口
**  参考文档: <<DVS传输协议.doc>> V1.0
**  date           :  2013.12.07
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#ifndef _PACK_DATA_H
#define _PACK_DATA_H

#define PACK_ALIGN	__attribute__((packed))

//
// 数据头格式,长度14
//
typedef struct FirsPackHead
{
	unsigned int	msgFlag;    // 消息标识
	unsigned char	msgType;    // 消息类型
	unsigned short	packSn;        // 包序号
	unsigned int	packType;    // 包类型
	unsigned char	subType;    // 子类型
	unsigned short	len;        // 数据长度
} PACK_ALIGN FIRS_PACK_HEAD;

//
// 数据包格式
//
typedef struct FirsPackData
{
	FIRS_PACK_HEAD	head;        // 包头数据
	unsigned char	data[1];    // 包含数据部分和2字节校验和
} PACK_ALIGN FIRS_PACK_DATA;

#undef PACK_ALIGN

//
// 获取标记在数据中的偏移位置
//
int GetFlagOffset( unsigned char *data, int len, unsigned int flag );

//
// 获取数据包头
//
void FirsGetDataHead( FIRS_PACK_HEAD &head );

//
// 获取数据包长度: 包头长度+包数据长度+校验长度
//
int FirsGetPackLen( FIRS_PACK_DATA *pPackData );

//
// 生成数据包头
//
void FirsPackDataHead(	FIRS_PACK_HEAD &head,
                    	unsigned int	msgFlag,
                    	unsigned char	msgType,
                    	unsigned short	packSn,
                    	unsigned int	packType,
                    	unsigned char	subType,
                    	unsigned short	len );

//
// 生成数据包
//
void FirsPackDataPack(	unsigned int	msgFlag,
                    	unsigned char	msgType,
                    	unsigned short	packSn,
                    	unsigned int	packType, 
                    	unsigned char	subType,
                    	unsigned char *	dataBuf,
                    	unsigned short	dataLen,
                    	unsigned char *	packBuf,
                    	int &        	packLen	);

#endif  // _PACK_DATA_H

