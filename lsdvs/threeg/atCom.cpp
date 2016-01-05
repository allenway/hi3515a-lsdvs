/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 封装at 指令集使用的公共函数,初始化AT指令集使用的资源
**  date           :  2014.9.25
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "debug.h"
#include "serial.h"
#include "sem.h"
#include "gb2312.h"
#include "atCom.h"

AT_CMD_ST g_atCmd;

/*
*  func:初始化AT库的全局结构体
*/
int AtCmdInitStruct( AT_CMD_ST *pAtCmd )
{
	int ret = -1;
	int serialFd;

	if( NULL == pAtCmd )
    {
    	SVPrint( "error:NULL == pAtCmd!\r\n" );
    	return -1;
    }
    
	pthread_mutexattr_t	mutexattr;
	memset( pAtCmd, 0, sizeof(AT_CMD_ST) );    
	pthread_mutexattr_init( &mutexattr );
	pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_RECURSIVE_NP );
	pthread_mutex_init( &pAtCmd->lock, &mutexattr);
	SemInit( &(pAtCmd->sem), 0, 0 );        
    
	serialFd = FiSerialOpenThreeg();
	if( serialFd != -1 )
    {
    	ret = FiSerialSetParamThreeg( serialFd, SERIAL_BAUDRATE_115200,
                            	SERIAL_DATABITS_8, SERIAL_STOPBITS_1,SERIAL_PARITY_NONE );
    }

	pAtCmd->recvTimeout.tv_sec = 0;
	pAtCmd->recvTimeout.tv_usec = 100*1000;

	pthread_mutexattr_destroy( &mutexattr );
    
	return ret;
}

/*
*  func: 去初始化AT库的全局结构体
*/
int AtCmdDeinitStruct( AT_CMD_ST *pAtCmd )
{
	if( NULL == pAtCmd )
    {
    	SVPrint( "error:NULL == pAtCmd!\r\n" );
    	return -1;
    }
    
	pthread_mutex_destroy( &(pAtCmd->lock) );    
	SemDestroy( &pAtCmd->sem );

	FiSerialCloseThreeg();

	return 0;
}

/*
*  func: 发送AT指令
*/
int AtCmdSend( AT_CMD_ST *pAtCmd )
{
	if( NULL == pAtCmd )
    {
    	SVPrint( "error:NULL == pAtCmd!\r\n" );
    	return -1;
    }
    
	return FiSerialWriteThreeg( pAtCmd->req, pAtCmd->reqSize );
}

/*
*  func: 同步发送指令
*  返回: == 0: 同步成功;否则同步失败
*/
int AtCmdSendSync( AT_CMD_ST *pAtCmd )
{
	if( NULL == pAtCmd )
    {
    	SVPrint( "error:NULL == pAtCmd!\r\n" );
    	return -1;
    }

	return SemTimedWait( &pAtCmd->sem, AT_WAIT_SEC, AT_WAIT_NSEC );
}

/*
*  func: 同步发送指令, 适用于只有一个关键字的同步
*  pKey: 同步的关键字
*  返回: == 0: 同步成功;否则同步失败
*/
int AtCmdSendSync( AT_CMD_ST *pAtCmd, char *pKey )
{
	int ret;
	if( NULL == pAtCmd || NULL == pKey )
    {
    	SVPrint( "error:NULL == pAtCmd || NULL == pKey!\r\n" );
    	return -1;
    }

	ret = SemTimedWait( &pAtCmd->sem, AT_WAIT_SEC, AT_WAIT_NSEC );
	if( ret == 0 )
    {
    	if( strstr( pAtCmd->rsp, pKey ) == NULL )
        {
        	ret = -1;
        }
    }

	return ret;
}

/*
* func: 把一个字符串转成unicode码串
* 注意: 该函数主要用于短信, srcStr 不能超过512个bytes
*/
int FiAtStrToUnicodeStr( char *pUnicode, char *srcStr )
{
	int i, count;
	char tmpBuf[1024];
	int len = strlen( srcStr );
	unsigned short unicodeWord;    
    
	count = 0;
	memset(tmpBuf,0,sizeof(tmpBuf));
    
	for (i = 0;i < len;)
    {
    	if ( (unsigned char)(*(srcStr + i)) > 0x80 )
        {
        	Gb2312ToUnicode( &unicodeWord, srcStr + i );
        	tmpBuf[count]         = (char)( (unicodeWord >> 8) & 0xff );
        	tmpBuf[count+1]     = (char)( unicodeWord&0xff );
        	i += 2;
        }
    	else
        {
        	tmpBuf[count]     = 0x00;
        	tmpBuf[count+1] = *(srcStr + i);
        	i++;
        }
    	count += 2;
    }
    
	for ( i = 0; i < count; i++)
    {
    	sprintf( pUnicode + strlen(pUnicode), "%02x", tmpBuf[i]);
    }
    
	return count;
}


