/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此头文件实现了对消息协议的包数据进行打包和解包操作的函数
**  参考文档: <<DVS传输协议.doc>> V1.0
**  date           :  2013.12.07
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "debug.h"
#include "netSocket.h"
#include "packData.h"
#include "fitLspp.h"

//
// 获取标记在数据中的偏移位置
//
int GetFlagOffset( unsigned char *data, int len, unsigned int flag )
{
	len -= ( sizeof(flag) - 1 );
	int offset = 0;
	for ( ; offset < len; ++offset )
    {
    	if ( !memcmp(data+offset, &flag, sizeof(flag)) )
        	break;
    }
	return offset;
}

//
// 获取数据包头
//
void FirsGetDataHead( FIRS_PACK_HEAD &head )
{
	head.msgFlag	= ntohl( head.msgFlag );
	head.packSn	    = ntohs( head.packSn );
	head.packType	= ntohl( head.packType );
	head.len	    = ntohs( head.len );
}

//
// 获取数据包长度: 包头长度+包数据长度+校验长度
//
int FirsGetPackLen( FIRS_PACK_DATA *pPackData )
{
	unsigned short check = 0;
	return sizeof(pPackData->head) + ntohs(pPackData->head.len) + sizeof(check);
}

//
// 生成数据包头
//
void FirsPackDataHead(	FIRS_PACK_HEAD &head,
                    	unsigned int	msgFlag,
                    	unsigned char	msgType,
                    	unsigned short	packSn,
                    	unsigned int	packType,
                    	unsigned char	subType,
                    	unsigned short	len )
{
	head.msgFlag	= htonl( msgFlag );
	head.msgType	= msgType;
	head.packSn	    = htons( packSn );
	head.packType	= htonl( packType );
	head.subType	= subType;
	head.len	    = htons( len );
}

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
                    	int &        	packLen	)
{
	if ( packBuf != NULL )
    {
    	FIRS_PACK_DATA *pFirsPackData = ( FIRS_PACK_DATA * )packBuf;
    	FirsPackDataHead( pFirsPackData->head, msgFlag, msgType,
                        	packSn, packType, subType, dataLen );

    	packLen = sizeof( pFirsPackData->head );
    	if ( dataBuf != NULL && dataLen > 0 )
        {
        	memcpy( (char *)packBuf + packLen, dataBuf, dataLen );
        	packLen += dataLen;
        }
    	unsigned short check = htons( GetCheckSum( packBuf, packLen ) );
    	memcpy( (char *)packBuf + packLen, (char *)&check, sizeof(check) );
    	packLen += sizeof(check);
    }
}

