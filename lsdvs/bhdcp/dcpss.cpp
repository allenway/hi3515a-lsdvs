/********************************************************************************
**	Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**	author        :  sven
**	version       :  v1.0
**	date           :  2013.10.10
**	description  : dcp stream, 管理音视频流; 和 dcpSignal.cpp 相对了
********************************************************************************/

#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
    
#include "const.h"
#include "debug.h"
#include "thread.h"
#include "malloc.h"
#include "message.h"
#include "linuxFile.h"
#include "netSocket.h"
#include "mutex.h"
    
#include "dcpCom.h"
#include "dcpFuncList.h"
#include "dcpFuncManage.h"
#include "dcpSignal.h"
    
static CLIENT_CONNECT_T g_dcpssConnect;
static void DcpssInitClientConnect()
{
	memset( &g_dcpssConnect, 0x00, sizeof(g_dcpssConnect) );
}

static int DcpssAddClientCom( CLIENT_CONNECT_T *pClient, int socket, int state )
{
	int ret = -1;    

	if( pClient->num < MAX_CLIENT_SIZE
        && CLIENT_SOCKET_STATE_NONE == pClient->client[pClient->num].socketState )
    {
    	pClient->client[pClient->num].socket	    = socket;
    	pClient->client[pClient->num].socketState	= state;            
    	ret = 0;
    	if( CLIENT_SOCKET_STATE_ACCEPT == state )
        {
        	pClient->client[pClient->num].readBufSize	= MAX_MSG_DATA_LEN >> 1;
        	pClient->client[pClient->num].writeBufSize	= MAX_MSG_DATA_LEN >> 1;
        	pClient->client[pClient->num].readBuf	    
                = (char *)Malloc(pClient->client[pClient->num].readBufSize);
        	pClient->client[pClient->num].writeBuf	    
                = (char *)Malloc(pClient->client[pClient->num].writeBufSize);
        	if( NULL == pClient->client[pClient->num].readBuf 
                || NULL == pClient->client[pClient->num].writeBuf)
            {
            	Free( pClient->client[pClient->num].readBuf );                    
            	Free( pClient->client[pClient->num].writeBuf );
            	pClient->client[pClient->num].socket	    = 0;
            	pClient->client[pClient->num].socketState	= CLIENT_SOCKET_STATE_NONE;
            	ret = -1;
            }                
        }
    	if( 0 == ret )
        {
        	pClient->num++;
        }
    }

	return ret;
}

static int DcpssDelAllClientCom( CLIENT_CONNECT_T *pClient)
{
	int i;

    Close(pClient->client[0].socket);
	for( i = 1; i < pClient->num; i++ )
    {
    	if( pClient->client[i].socket > 0 )
        {        
            Close( pClient->client[i].socket );
            pClient->client[i].socket = -1;
        	Free( pClient->client[i].readBuf ); 
        	Free( pClient->client[i].writeBuf );
        	pClient->client[i].socketState = CLIENT_SOCKET_STATE_NONE;
        }
    }
    
	return 0;
}

static int DcpssDelClientCom(  CLIENT_CONNECT_T *pClient, int socket, int moveFlag )
{
	int i, j, k,left;
	for( i = 0; i < pClient->num; i++ )
    {
    	if( pClient->client[i].socket == socket )
        {        
        	if( 1 != moveFlag )
            {
            	Close( pClient->client[i].socket );
            	pClient->client[i].socket = -1;
            }
        	Free( pClient->client[i].readBuf ); 
        	Free( pClient->client[i].writeBuf );
        	pClient->client[i].socketState = CLIENT_SOCKET_STATE_NONE;
        	left = pClient->num - i - 1;
        	if( left > 0 )
            {
                for(k=0;k<left;k++)
                {
                    pClient->client[i+k] = pClient->client[i+k+1];
                }
                //有重叠，慎用
                //Memmove( &pClient->client[i], &pClient->client[i+1], sizeof(pClient->client[i]) * left );
            }

        	pClient->num--;

        	for( j = pClient->num; j < MAX_CLIENT_SOCKET_NUM; j++ )
            {
            	pClient->client[j].socketState = CLIENT_SOCKET_STATE_NONE;
            }

        	break;
        }
    }
    
	return 0;
}

static int DcpssAcceptClientCom( CLIENT_CONNECT_T *pClient, int listenSocket )
{
	int socket, ret = -1;

	socket = AcceptNoblock( listenSocket );
	if( socket >= 0 )    
    {
        SVPrint("socket(%d)= AcceptNoblock( listenSocket );\n",socket);
    	ret = DcpssAddClientCom( pClient, socket, CLIENT_SOCKET_STATE_ACCEPT );
    	if( 0 != ret )
        {
        	Close( socket );
        }
    }

	return ret;
}

static int SelectDcpssComData( CLIENT_CONNECT_T *pClient, SELECT_CLIENT_DATA_T *pSelectResult )
{        
	fd_set fd; 
	int maxfd = 0;
	struct timeval timeout;
	int ret, i;

	memset( pSelectResult, 0x00, sizeof(*pSelectResult) );
	FD_ZERO( &fd );
	for( i = 0; i < pClient->num; ++i )
    {        
    	FD_SET( pClient->client[i].socket, &fd );
    	if ( pClient->client[i].socket > maxfd )
        {
        	maxfd = pClient->client[i].socket;
        }        
    }
	timeout.tv_sec	= 2;
	timeout.tv_usec = 0;
	ret = select( maxfd + 1, &fd, NULL, NULL, &timeout );
	if ( ret > 0 )
    {
    	for( i = 0; i < pClient->num; ++i )
        {
        	if( FD_ISSET(pClient->client[i].socket, &fd) ) 
            {
            	pSelectResult->index[pSelectResult->num++] = i;                 
            }            
        }
    }
    
	return ret; 
}

static int DcpssRecvClientComData( CLIENT_COMMUNICATE_T *pClientCom )
{
	int ret;
	int toReadLen = pClientCom->readBufSize - pClientCom->readBufDataSize;

	ret = Read( pClientCom->socket, pClientCom->readBuf + pClientCom->readBufDataSize, toReadLen );
	if( ret > 0 )
    {
    	pClientCom->readBufDataSize += ret;
    	ret = pClientCom->readBufDataSize;
    }
	return ret;
}

static int DcpssClientComMsgCheck( CLIENT_COMMUNICATE_T *pClientCom, DCP_HEAD_T *pMsgHead )
{
	int ret = FI_FAIL;
	int mark = CLIENT_MSG_MARK;
	DCP_HEAD_T msgHead;
	unsigned int headSize = sizeof( msgHead );    
	unsigned int left;
	void *pmem;

	pmem = Memmem( pClientCom->readBuf, pClientCom->readBufDataSize, &mark, sizeof(mark) );
	if( NULL != pmem )
    {
    	if( pmem != pClientCom->readBuf )
        {
        	left = pClientCom->readBufDataSize - ((unsigned int)pmem - (unsigned int)pClientCom->readBuf);
        	Memmove( pClientCom->readBuf, pmem, left );
        	pClientCom->readBufDataSize = left;
        }

    	if( pClientCom->readBufDataSize >= headSize )
        {
        	memcpy( &msgHead, pClientCom->readBuf, headSize );
            ColorPrint(COLORBLUE," %s msgType %x dataLen %d.\r\n", __FUNCTION__, msgHead.msgType, msgHead.len);
        	if( msgHead.len <= (pClientCom->readBufSize - headSize) )
            {
            	if( pClientCom->readBufDataSize >= (headSize + msgHead.len) )
                {
                	ret = FI_SUCCESS;
                }
            	else
                {
                	ret = FI_FAIL;
                }
            }
        	else // 出错了
            {
            	pClientCom->readBufDataSize = 0;
            	ret = FI_FAIL;
            }
        }
    }
	else
    {
    	left = sizeof(mark) - 1; // 可能还没有收完一个mark
    	pmem = pClientCom->readBuf + pClientCom->readBufDataSize - left;
    	Memmove( pClientCom->readBuf, pmem, left );
    	pClientCom->readBufDataSize = left;
    }    
    
	if( FI_SUCCESS == ret ) *pMsgHead = msgHead;
    
	return ret;
}

static int DcpssHandLeClientComMsg( CLIENT_COMMUNICATE_T *pClientCom )
{
	int ret = 0;
	unsigned int leftSize;
	DCP_CLIENT_COM_FUNC func;
	DCP_HEAD_T msgHead = { 0 };
	uint headSize = sizeof( msgHead );
	uint dataLen;
    
	leftSize = pClientCom->readBufDataSize;
	while( leftSize  >= headSize )
    {
    	ret = DcpssClientComMsgCheck( pClientCom, &msgHead );        
//    	FiPrint2( "leftSize = %d!\r\n", leftSize );
//    	FiPrint2( "ret = %d!\r\n", ret );
    	if( FI_SUCCESS == ret )
        {
        	dataLen = msgHead.len;
        	func = GetClientMsgFunction( msgHead.msgType );         
        	ret = func( &msgHead, pClientCom ); 
            
        	pClientCom->readBufDataSize = pClientCom->readBufDataSize - (headSize+dataLen);
        	memmove( pClientCom->readBuf, pClientCom->readBuf + (headSize+dataLen), pClientCom->readBufDataSize );                
        	if( DCP_COM_ERR_NEED_CLOSE_SOCKET == ret	 // 如果发生错误, 则关闭socket
                || DCP_COM_ERR_NEED_MOVE_SOCKET == ret ) // 发生请求也是马上返回
            {
            	break;
            }
        }
    	if( leftSize > pClientCom->readBufDataSize ) // 数据有被处理过
        {
        	leftSize = pClientCom->readBufDataSize; 
        }
    	else
        {
        	break; // 剩余的数据长度不足一条指令的长度
        }
    }

	return ret;
}

static int DcpssSendClientComBack( CLIENT_COMMUNICATE_T *pClientCom )
{
	int ret = 0;

	if( pClientCom->writeBufDataSize> 0 
        && pClientCom->writeBufDataSize <= pClientCom->writeBufSize )
    {
    	ret = SelectWrite( pClientCom->socket, 1 );
    	if( ret > 0 )
        {            
        	ret = Write( pClientCom->socket, pClientCom->writeBuf, pClientCom->writeBufDataSize );
        	if( ret > 0 )
            {
            	pClientCom->writeBufDataSize -= ret;    
            	if( pClientCom->writeBufDataSize > 0 ) // 没有发送完
                {
                	Memmove( pClientCom->writeBuf, pClientCom->writeBuf+ret, pClientCom->writeBufDataSize);
                }                
            }
        	else
            {                
            	SVPrint( "Error: writen ret = %d, %s!\r\n", ret, STRERROR_ERRNO );

            	ret = -1;
            }
        }
    }
	else // 异常处理
    {
    	pClientCom->writeBufDataSize = 0;
    }

	return ret;
}

static int DcpssInitSignalListenSocket( CLIENT_CONNECT_T *pClient )
{
	int ret, socket;

    // signal listen socket
	ret = SocketTcpListen( &socket, LISTEN_CLIENT_STREAM_PORT, SOCKET_NOBLOCK );    
	if( 0 == ret )
    {
    	ret = DcpssAddClientCom( pClient, socket, CLIENT_SOCKET_STATE_LISTEN );
    	if( 0 != ret )
        {
        	SVPrint( "failed: Add signal listen socket failed!\r\n" );
        	Close( socket );
        }
    }
	else
    {
    	SVPrint( "failed: Create signal listen socket failed!\r\n" );
    }    
    
	return ret;
}

#if 0
static void RefreshHeartbeatTimestamp( CLIENT_COMMUNICATE_T *pClientCom )
{
    //pClientCom->timestamp = SysRunTimeGet(); // TODO
}

static void HeartbeatMaintain( CLIENT_COMMUNICATE_T *clientCom )
{
    // int timeCur = SysRunTimeGet(); // TODO

    // 超过8 秒没有登录
    // 两分钟没有心跳,从新注册
}
#endif
static THREAD_MAINTAIN_T g_dcpSignalTm;
//该线程主要收发进行视频流的发送相关的协议
static void *DcpssThread( void *arg )
{
	int ret, retSend, i;
	SOCKET_ERR_T socketErr;
	SELECT_CLIENT_DATA_T selectResult;
	CLIENT_CONNECT_T *pClient = &g_dcpssConnect;

	DcpssInitClientConnect();
	ret = DcpssInitSignalListenSocket( pClient );
	if(0 != ret) 
    {
    	ERRORPRINT("failed: DcpssInitSignalListenSocket()!\r\n");
    	return NULL;    
    }    
    
	CORRECTPRINT( "????????%s start????????\r\n", __FUNCTION__ );

	while( g_dcpSignalTm.runFlag )
    {
    #if 0
    	if ( MessageRecv(MSG_ID_CLIENT_PROTOCOL_THREAD, (char *)&internalCmd, sizeof(internalCmd)) >= 0 )
        {
        	ret = HandleClientCmdFromInternal(internalCmd,&clientCom);
        	if(ret == 0 && clientCom.writeBufSize > 0)
            	ret = DcpssSendClientComBack(&clientCom);
        }
    #endif
    	ret = SelectDcpssComData( pClient, &selectResult );    
    	if(ret > 0) 
        {
        	Memset( &socketErr, 0x00, sizeof(socketErr) );
        	for( i = 0; i < selectResult.num; ++i )
            {    
            	if( CLIENT_SOCKET_STATE_LISTEN == pClient->client[selectResult.index[i]].socketState )
                {                    
                	ret = DcpssAcceptClientCom( pClient, pClient->client[selectResult.index[i]].socket ); // 接收连接并加入socket 维护数组
                }
            	else
                {
                	ret = DcpssRecvClientComData( &pClient->client[selectResult.index[i]] ); // 接收客户端的数据
                	if( ret <= 0 )
                    {
                    	socketErr.socket[socketErr.num]     = pClient->client[selectResult.index[i]].socket;
                    	socketErr.moveFlag[socketErr.num]     = 0;
                    	socketErr.num++;
                    }
                	else
                    {                        
                    	ret = DcpssHandLeClientComMsg( &pClient->client[selectResult.index[i]] );    // 处理所有接收到的消息
                    	retSend = DcpssSendClientComBack( &pClient->client[selectResult.index[i]] );            // 返回处理结果
                    	if( -1 == retSend )
                        {
                        	socketErr.socket[socketErr.num] = pClient->client[selectResult.index[i]].socket;
                        	socketErr.moveFlag[socketErr.num] = 0;
                        	socketErr.num++;
                        }
                    	else
                        {
                        	if( DCP_COM_ERR_NEED_CLOSE_SOCKET == ret )
                            {
                            	socketErr.socket[socketErr.num] = pClient->client[selectResult.index[i]].socket;
                            	socketErr.moveFlag[socketErr.num] = 0;
                            	socketErr.num++;
                            }
                        	else if( DCP_COM_ERR_NEED_MOVE_SOCKET == ret )
                            {    
                            	SVPrint( "socket:%d ---DCP_COM_ERR_NEED_MOVE_SOCKET!\r\n", 
                            	pClient->client[selectResult.index[i]].socket,DCP_COM_ERR_NEED_MOVE_SOCKET );
                            	socketErr.socket[socketErr.num] = pClient->client[selectResult.index[i]].socket;
                            	socketErr.moveFlag[socketErr.num] = 1;
                            	socketErr.num++;
                            }
                        }
                    }
                }
            }     

        	for( i = 0; i < socketErr.num; i++ )
            {
            	DcpssDelClientCom( pClient, socketErr.socket[i], socketErr.moveFlag[i] );                                
            }
        }
        
    	usleep(10);
    } 

	ERRORPRINT( "????????%s stop????????\r\n", __FUNCTION__ );
    DcpssDelAllClientCom(pClient);

	return NULL;
}

void StartDcpssThread()
{
	int ret;
	g_dcpSignalTm.runFlag = 1;
	ret = ThreadCreate( &g_dcpSignalTm.id, DcpssThread, NULL );
	if( 0!= ret )
    {        
    	g_dcpSignalTm.runFlag = 0;
    	SVPrint( "error:ThreadCreate:%s\r\n", STRERROR_ERRNO );
    }
}

void StopDcpssThread()
{
	int ret;
	g_dcpSignalTm.runFlag = 0;
	ret = ThreadJoin( g_dcpSignalTm.id, NULL );
	if( 0 != ret )
    {
    	SVPrint( "error:ThreadJoin:%s\r\n", STRERROR_ERRNO );
    }
}
    

