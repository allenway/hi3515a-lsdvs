/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 管理3G的AT业务
**  date           :  2014.9.25
**
**  version       :  1.0
**  author        :  sven
********************************************************************************/
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "const.h"
#include "serial.h"
#include "thread.h"
#include "debug.h"
#include "linuxFile.h"
#include "malloc.h"
#include "atCom.h"
#include "atMsg.h"
#include "threegSerial.h"
#include "threegAt.h"
#include "atEcho.h"

#define THREEG_AT_BUFSIZE	(ATCMD_SIZE + 4)
static int g_threegAtFlag = FI_FALSE;

static int ThreegAtUnknow( THREEG_AT_COMMUNICATE *pThreegAt, char *pLine )
{
	AT_CMD_ST *pCmd = &g_atCmd;

	if( PRE_RECV_FLAG_NO != pCmd->preRecvFlag )
    {
    	if( PRE_RECV_FLAG_SMS == pCmd->preRecvFlag )
        {             
        	strcat( pCmd->rsp, ATCMD_CR_LF ); 
        }
    	strcat( pCmd->rsp, pLine );        
    }
	else
    {
    	SVPrint( "ThreegAtUnknow failed:recv unknow msg(%s)!\r\n", pLine );
    }    
    
	pThreegAt->anaDataSize = 0;
	return 0;
}

static int GetSmsPhoneNum( char *pLine, char *pPhoneNum )
{
	int len, ret = -1;
	const char phoneNumMarkStart[] = "\",\""; //从 +CMGR: "REC UNREAD","15818757328",,"11/11/16,21:53:15+32" 中找电话号码
	const char phoneNumMarkStop[] = "\",";
	char *ptrPhoneNumStart, *ptrPhoneNumStop;
    
	ptrPhoneNumStart = strstr( pLine, phoneNumMarkStart );
	if( NULL != ptrPhoneNumStart )
    {
    	ptrPhoneNumStart += strlen( phoneNumMarkStart );
    	ptrPhoneNumStop = strstr( ptrPhoneNumStart, phoneNumMarkStop );
    	if( NULL != ptrPhoneNumStop )
        {
        	len = ptrPhoneNumStop - ptrPhoneNumStart;
        	memcpy( pPhoneNum, ptrPhoneNumStart, len );
        	pPhoneNum[len] = '\0';

        	ret = 0;
        }
    }

	return ret;
}
static int ParseSmsContent( char *pSmsContent )
{
	int ret = -1;
	char phoneNum[16];    // 电话号码
	char smsBare[256] = { 0 };    // 仅仅短信消息内容 
	char *ptr;
	ptr = Strsep( &pSmsContent, ATCMD_CR_LF );
	if( NULL != ptr )
    {
    	ret = GetSmsPhoneNum( ptr, phoneNum );
    	if( ret == 0 )
        {
        	ptr = Strsep( &pSmsContent, ATCMD_CR_LF );
        	strcpy( smsBare, ptr );
        }
    }
    
	FiPrint2( "%s:%s, %s!\r\n", __FUNCTION__, phoneNum, smsBare );

	return ret;
}

static int ThreegAtAck( THREEG_AT_COMMUNICATE *pThreegAt, char *pLine )
{
	AT_CMD_ST *pCmd = &g_atCmd;

	if( PRE_RECV_FLAG_NO != pCmd->preRecvFlag )
    {
    	if( PRE_RECV_FLAG_SMS == pCmd->preRecvFlag )
        {             
        	strcat( pCmd->rsp, ATCMD_CR_LF ); 
        }
    	strcat( pCmd->rsp, pLine );
    }
	else
    {
    	strcpy( pCmd->rsp, pLine );
    }

	if( PRE_RECV_FLAG_SMS == pCmd->preRecvFlag )
    {
    	ParseSmsContent( pCmd->rsp );
    	FiAtDelSms( 0, FI_TRUE );
    }
	else
    {        
    	SemPost( &pCmd->sem );        
    }    

	if( PRE_RECV_FLAG_NO != pCmd->preRecvFlag )
    {
    	pCmd->preRecvFlag = PRE_RECV_FLAG_NO;
    }
    
	pThreegAt->anaDataSize = 0;
	return 0;
}

/*
*	func	: 对于某些多于1行的回复,用这个函数接收结束关键行(大部分为 \r\n"OK"\r\n) 前的行
*/
static int ThreegAtAckPreRecv( THREEG_AT_COMMUNICATE *pThreegAt, char *pLine )
{
	AT_CMD_ST *pCmd = &g_atCmd;

	strcpy( pCmd->rsp, pLine );    
	pCmd->preRecvFlag = PRE_RECV_FLAG_SYNC;
    
	pThreegAt->anaDataSize = 0;
	return 0;
}

static int ThreegAtRing( THREEG_AT_COMMUNICATE *pThreegAt, char *pLine )
{
	pThreegAt->anaDataSize = 0;
	return 0;
}

static int ThreegAtRingNum( THREEG_AT_COMMUNICATE *pThreegAt, char *pLine )
{
	pThreegAt->anaDataSize = 0;
	return 0;
}

/*
*	fn : 从 +CMTI: "SM",10 此类消息中得到新接收到sms的index
*	返回新sms的index
*/
static int GetSmsIndex( char *pLine )
{
	int smsIndex = 0;
	char *ptr;

	ptr = strstr( pLine, "," );
	if( NULL != ptr ) 
    {
    	ptr++;
    	smsIndex = atoi( ptr );
    }
    
	return smsIndex;
}
static int ThreegAtRecvNewSms( THREEG_AT_COMMUNICATE *pThreegAt, char *pLine )
{
	int smsIndex;
    //AT_CMD_ST *pCmd = &g_atCmd;
	SVPrint( "note:dvs recv new sms, pLine = %s!\r\n", pLine );
	smsIndex = GetSmsIndex( pLine );    
	SVPrint( "note:dvs recv new sms, index = %d!\r\n", smsIndex );
	FiAtSendReadSmsCmd( smsIndex );

	pThreegAt->anaDataSize = 0;
	return 0;
}

static int ThreegAtRecvSmsContent( THREEG_AT_COMMUNICATE *pThreegAt, char *pLine )
{
    //int smsIndex;
	AT_CMD_ST *pCmd = &g_atCmd;
	strcpy( pCmd->rsp, pLine ); 
	SVPrint( "recv sms, content:%s!\r\n", pCmd->rsp );
	pCmd->preRecvFlag = PRE_RECV_FLAG_SMS; // 继续接收短息的其他内容
    
	pThreegAt->anaDataSize = 0;
	return 0;
}

/*
*  func	        : 检查接收到的里面的数据是否有合格的行，两个 ATCMD_CR_LF 之间的行为合格行
*  checkedSize	: out,已经检查过的数据大小。
*  pLine	    : 把找到的有效行拷贝到 pLine  
*  返回         : 0,找到合格行; -1,找不到合格行.
*/
static int ThreegAtCheck( THREEG_AT_COMMUNICATE *pThreegAt, char *pLine, int *pCheckedSize )
{        
	int ret = -1;
	int checkedSize = 0, cpySize, secondKeySize, offset;
	int rcLfLen = strlen(ATCMD_CR_LF);
	char *pRcLfFirst, *pRcLfSecond;    
	AT_CMD_ST *pCmd = &g_atCmd;

	if( NULL == pThreegAt || NULL == pLine || NULL == pCheckedSize )
    {
    	SVPrint( "NULL == pThreegAt || NULL == pLine || NULL == pCheckedSize\r\n" );
    	return -1;
    }
    
	if( PRE_RECV_FLAG_SMS == pCmd->preRecvFlag )
    {
    	pRcLfFirst         = pThreegAt->sockBuf;
    }
	else
    {
    	pRcLfFirst      = strstr( pThreegAt->sockBuf, ATCMD_CR_LF );
    }    
	if( NULL == pRcLfFirst )
    {        
    	checkedSize += pThreegAt->sockDataSize;
    }
	else
    {
    	if( PRE_RECV_FLAG_SMS != pCmd->preRecvFlag )
        {
        	do	// 去掉连续两个 "\r\n"
            {
            	pRcLfFirst += rcLfLen;
            }while( 0 == memcmp( pRcLfFirst, ATCMD_CR_LF, rcLfLen ) );
        }
        
    	offset = pRcLfFirst - pThreegAt->sockBuf;
    	checkedSize += (offset > 0 ? offset : 0);
    	ret = 0;
    }

	if( ret != -1)
    {
    	ret = -1;
    	if( NULL != (pRcLfSecond = strstr( pRcLfFirst, ATCMD_INPUT )) )
        {
        	pRcLfSecond += strlen( ATCMD_INPUT );
        	secondKeySize = 0;
        }
    	else
        {
        	pRcLfSecond = strstr( pRcLfFirst, ATCMD_CR_LF );
        	secondKeySize = strlen( ATCMD_CR_LF );
        }
    	if( NULL != pRcLfSecond )    // 找到第二个"\r\n"
        {
        	cpySize = pRcLfSecond - pRcLfFirst;
        	cpySize = (cpySize > (ATCMD_SIZE - 1)) ? (ATCMD_SIZE - 1) : cpySize;
        	memcpy( pLine, pRcLfFirst, cpySize );
        	pLine[cpySize] = '\0';
        	checkedSize += ( cpySize + secondKeySize); 
        	ret = 0;
        }
    	else if( pThreegAt->sockDataSize >= ATCMD_SIZE ) // 没有找到第二个"\r\n",同时接收缓冲区又满了。
        {
        	checkedSize = pThreegAt->sockDataSize;
        }
    }
    
    *pCheckedSize = checkedSize;
    
	return ret;
} 

static THREEG_AT_MSG_FUNC g_threegAtMsgFunc[] = 
{
    { AT_CHARS_OK,                	ThreegAtAck	            },    // 通用回复
    { AT_KEY_SHORT_MSG_ENTRY,    	ThreegAtAck	            },    // 短信输入
    { AT_KEY_RING_NUMBER_DISPLAY, 	ThreegAtRingNum         },    // 来电号码显示
    { AT_KEY_RING,                	ThreegAtRing             },    // 有电话打进来
    { AT_KEY_SIGNAL_VALUE,        	ThreegAtAckPreRecv	    },    // 查看信号格数
    { AT_KEY_RECV_NEW_SMS,        	ThreegAtRecvNewSms	    },    // 收到新短信通知
    { AT_KEY_RECV_SMS_CONTENT,    	ThreegAtRecvSmsContent     },    // 开始接收短信内容	
    { AT_KEY_SIGNAL_VALUE_HW,    	ThreegAtAckPreRecv	    },    // 查看信号格数
};
/*
* func	: 解析一条命令完整的命令
* pLine	: 从3G模块中获取到的一条完整的命令
*/
static THREEG_AT_FUNC ThreegAtGetFunc( char *pLine)
{
	THREEG_AT_FUNC func = ThreegAtUnknow;
	int loopTimes, i;
	loopTimes = sizeof( g_threegAtMsgFunc ) / sizeof( THREEG_AT_MSG_FUNC ) ;
    
	for( i = 0; i < loopTimes; ++i )
    {
    	if( NULL != strstr( pLine, g_threegAtMsgFunc[i].msgKey ) )
        {
        	func = g_threegAtMsgFunc[i].func;
        	break;
        }
    }

	return func;
}
/*
* func	: 解析一条命令完整的命令
* pLine	: 从3G模块中获取到的一条完整的命令
* 返回	: 根据该命令返回要处理它的函数
*/
static THREEG_AT_FUNC ThreegAtParseCmdLine( char *pLine )
{
	THREEG_AT_FUNC func;
	if( NULL == pLine )
    {
    	SVPrint( "NULL == pLine\r\n" );
    	return NULL;
    }

	func = ThreegAtGetFunc( pLine );
    
	return func;
}

/*
*  func	: 处理接收到的数据。
*  phandledSize	: out, 处理过的数据的大小 
*  返回	: 0,成功处理了一条命令; -1,没有成功处理的命令。
*/
static int ThreegAtHandleRecvMsg( THREEG_AT_COMMUNICATE *pThreegAt, int *phandledSize )
{
	int 	ret;
	char   	line[ATCMD_SIZE];
	int		checkedSize = 0;
	THREEG_AT_FUNC	doFunc;
    
	if( NULL == pThreegAt )
    {
    	SVPrint( "NULL == pThreegAt!\r\n" );
    	return -1;
    }

	ret = ThreegAtCheck( pThreegAt, line, &checkedSize);
	if( ret == 0 ) // 找到一条完成的消息
    {
    	doFunc = ThreegAtParseCmdLine( line );
    	if( NULL != doFunc ) ret = doFunc( pThreegAt, line );
    }
    
    *phandledSize = checkedSize;
    
	return ret;
}

void *ThreegAtRecvThread( void *arg )
{
	char *pRecvBuf;
	THREEG_AT_COMMUNICATE threegAt;        
	int ret, recvSize, handedSize;

	threegAt.sockBuf     = (char *)Malloc( THREEG_AT_BUFSIZE );
	threegAt.anaBuf     = (char *)Malloc( THREEG_AT_BUFSIZE );
	if( NULL == threegAt.sockBuf || NULL == threegAt.anaBuf )
    {        
    	Free( threegAt.sockBuf );
    	Free( threegAt.anaBuf );
    	return NULL;
    }
	threegAt.sockBuf[0] = '\0';
	threegAt.sockDataSize = 0;
    
	SVPrint( "start ThreegAtRecvThread(%d)!\r\n", ThreadSelf() );
	while( FI_TRUE == g_threegAtFlag )
    {
    	pRecvBuf = threegAt.sockBuf + threegAt.sockDataSize; // -1 是因为所有的字符串都要放到'\0'前
    	recvSize = THREEG_AT_BUFSIZE - threegAt.sockDataSize - 1;
    	ret = FiThreegSerialRecvData( SERIAL_TYPE_THREEG_AT, pRecvBuf,  recvSize );
    	if( ret > 0 )
        {    
        	threegAt.sockDataSize += ret;
        	threegAt.sockBuf[threegAt.sockDataSize] = '\0';                    
        	do
            {
            	ret = ThreegAtHandleRecvMsg( &threegAt, &handedSize );
            	if( ret == 0 )
                {                        
                	threegAt.sockDataSize -= handedSize;
                	memmove( threegAt.sockBuf, threegAt.sockBuf + handedSize, threegAt.sockDataSize );
                	threegAt.sockBuf[threegAt.sockDataSize] = '\0';                    
                }                
            	usleep(5);
            }while( ret == 0 );
        }    
    	usleep(100000);
    } // end while( FI_TRUE == g_threegAtFlag )

	Free( threegAt.sockBuf );
	Free( threegAt.anaBuf );
	SVPrint( "stop ThreegAtRecvThread!\r\n" );

	return NULL;
}

static pthread_t	g_threegAtId;
int ThreegAtStartService()
{
	int ret;
    
	ret = AtCmdInitStruct( &g_atCmd );
	if( ret != 0 ) 
    {        
    	SVPrint( "error:AtCmdInitStruct failed,please check dev!\r\n" );
    	return ret;
    }
    
	g_threegAtFlag = FI_TRUE;
	ret = ThreadCreateCommonPriority( &g_threegAtId, ThreegAtRecvThread, NULL );
	if ( ret != 0 )
    {
    	g_threegAtFlag = FI_FALSE;
    	SVPrint( "ThreadCreateCommonPriority() error:%s\r\n", STRERROR_ERRNO );
    }

	return ret;
}

int ThreegAtStopService()
{    
	g_threegAtFlag = FI_FALSE;
	if( 0 != ThreadJoin( g_threegAtId, NULL ) )
    {
    	SVPrint( "ThreadJoin error:%s!\r\n", STRERROR_ERRNO );
    	return FI_FAILED;
    }
	AtCmdDeinitStruct( &g_atCmd );
	return FI_SUCCESSFUL;
}

static void *ThreegAtInitThread( void *arg )
{
	int i, ret;

	for( i = 0; i < 10; i++ )
    {
    	if( 0 == FiAtSmsInit() )
        {
        	break;
        }        
    	usleep(10000);        
    	SVPrint( "FiAtSmsInit usleep(10000);\r\n" );
    }

	for( i = 0; i < 10; i++ )
    {
    	if( 0 == FiAtSetSmsFormat( AT_MSG_FORMAT_TEXT ) )
        {
        	break;
        }
    	usleep(10000);
    	SVPrint( "FiAtSetSmsFormat usleep(10000)!\r\n" );
    }        
	ret = FiAtEchoCtl( AT_ECHO_OFF);
	SVPrint( "FiAtEchoCtl( AT_ECHO_OFF ) = ret(%d)!\r\n", ret );
    
	return NULL;
}

/*
*	fn : 通过AT指令初始化3G模块的参数,例如短信的CNMI
*         由于这些工作需要在线程 ThreegAtStartService() 正常工作的时候才能有正确的返回值,所以
         独立一个线程来初始化
*/
int ThreegAtInit()
{
	int ret;    
	pthread_t threegAtInitId;
    
	ret = ThreadCreateCommonPriority( &threegAtInitId, ThreegAtInitThread, NULL );
	if ( ret != 0 )
    {
    	SVPrint( "ThreadCreateCommonPriority() error:%s\r\n", STRERROR_ERRNO );
    }

	return ret;
}



