/********************************************************************************
**	Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**	author        :  sven
**	version       :  v1.0
**	date           :  2013.10.10
**	description  : 负责处理与客户端交互协议的信令部分
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
#include "dcpStream.h"

static CLIENT_CONNECT_T g_clientConnect = {0};
static void InitClientConnect()
{
	memset( &g_clientConnect, 0x00, sizeof(g_clientConnect) );
}

static int AddClientCom( CLIENT_CONNECT_T *pClient, int socket, int state )
{
	int ret = -1;    

	SVPrint( "pClient->num(%d), socketState(%d)!\r\n",
            	pClient->num,        
            	pClient->client[pClient->num].socketState );
	if( pClient->num < MAX_CLIENT_SIZE
        && CLIENT_SOCKET_STATE_NONE == pClient->client[pClient->num].socketState )
    {
    	pClient->client[pClient->num].socket         = socket;
    	pClient->client[pClient->num].socketState     = state;            
    	ret = 0;
    	if( CLIENT_SOCKET_STATE_ACCEPT == state )
        {
        	pClient->client[pClient->num].readBufSize     = MAX_MSG_DATA_LEN;
        	pClient->client[pClient->num].writeBufSize     = MAX_MSG_DATA_LEN;
        	pClient->client[pClient->num].readBuf         
                = (char *)Malloc(pClient->client[pClient->num].readBufSize);
        	pClient->client[pClient->num].writeBuf         
                = (char *)Malloc(pClient->client[pClient->num].writeBufSize);
        	if(	NULL == pClient->client[pClient->num].readBuf 
                || NULL == pClient->client[pClient->num].writeBuf)
            {
            	Free( pClient->client[pClient->num].readBuf );                    
            	Free( pClient->client[pClient->num].writeBuf );
            	pClient->client[pClient->num].socket         = -1;
            	pClient->client[pClient->num].socketState     = CLIENT_SOCKET_STATE_NONE;
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

/**********************************************************************
* 关闭所有连接
***********************************************************************/
static int DelAllClientCom( CLIENT_CONNECT_T *pClient)
{
	int i;

    Close(pClient->client[0].socket);
	for( i = 1; i < pClient->num; i++ )
    {
    	if( pClient->client[i].socket > 0 )
        {        
        	Close( pClient->client[i].socket );
        	pClient->client[i].socket = -1;
        	DcpDestoryStreamThread( pClient->client[i].threadId );
        	Free( pClient->client[i].readBuf );    
        	Free( pClient->client[i].writeBuf );
        	pClient->client[i].socketState = CLIENT_SOCKET_STATE_NONE;
        }
    }
    
	return 0;
}
static int DelClientCom( CLIENT_CONNECT_T *pClient, int socket, int moveFlag )
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
            	DcpDestoryStreamThread( pClient->client[i].threadId );
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
                //memset(&(pClient->client[i+k+1]),0,sizeof(CLIENT_COMMUNICATE_T))
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

static int AcceptClientCom( CLIENT_CONNECT_T *pClient, int listenSocket )
{
	int socket = -1, ret = -1;

	socket = AcceptNoblock( listenSocket );
	if( socket >= 0 )    
    {
    	ret = AddClientCom( pClient, socket, CLIENT_SOCKET_STATE_ACCEPT );
    	if( 0 != ret )
        {
        	Close( socket );
        }
    }

	return ret;
}

static int SelectClientComData( CLIENT_CONNECT_T *pClient, SELECT_CLIENT_DATA_T *pSelectResult )
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
  	timeout.tv_usec	= 0;
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

static int RecvClientComData( CLIENT_COMMUNICATE_T *pClientCom )
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

static int ClientComMsgCheck( CLIENT_COMMUNICATE_T *pClientCom, DCP_HEAD_T *pMsgHead )
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
        	Memmove( pClientCom->readBuf, pmem, left );//完全包含的关系，TBD
        	pClientCom->readBufDataSize = left;
        }

    	if( pClientCom->readBufDataSize >= headSize )
        {
        	memcpy( &msgHead, pClientCom->readBuf, headSize );
            ColorPrint(COLORYELLOW," %s msgType %x dataLen %d.\r\n", __FUNCTION__, msgHead.msgType, msgHead.len);
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
    
	if( FI_SUCCESS == ret )    *pMsgHead = msgHead;
    
	return ret;
}

static int HandLeClientComMsg( CLIENT_COMMUNICATE_T *pClientCom )
{
	int ret = 0;
	unsigned int leftSize;
	DCP_CLIENT_COM_FUNC func;
	DCP_HEAD_T msgHead = { 0 };
	uint headSize = sizeof( msgHead );
	uint dataLen;
     
	leftSize = pClientCom->readBufDataSize;
    //SVPrint(" %s letfSzie : %x, headSize %x.\r\n", __FUNCTION__, leftSize, headSize);
	while( leftSize  >= headSize )
    {        
    	ret = ClientComMsgCheck( pClientCom, &msgHead );        
    	//FiPrint2( "leftSize = %d!\r\n", leftSize );
    	//FiPrint2( "ret = %d!\r\n", ret );
    	if( FI_SUCCESS == ret )
        {
        	dataLen = msgHead.len;
            ERRORPRINT("get %d message!!!!!!!\n",msgHead.msgType);
        	func = GetClientMsgFunction( msgHead.msgType );            
        	ret = func( &msgHead, pClientCom );    
            
        	pClientCom->readBufDataSize = pClientCom->readBufDataSize - (headSize+dataLen);
//            SVPrint(" ret %x, leftSize %x.\r\n", ret, pClientCom->readBufDataSize);
            //memmove( pClientCom->readBuf, pClientCom->readBuf + (headSize+dataLen), pClientCom->readBufDataSize );
            unsigned int i = 0;
            for(i = 0;i < pClientCom->readBufDataSize;i++)
            {
                pClientCom->readBuf[i]= pClientCom->readBuf[i+headSize+dataLen];
            }
        	if( DCP_COM_ERR_NEED_CLOSE_SOCKET == ret     // 如果发生错误, 则关闭socket
                || DCP_COM_ERR_NEED_MOVE_SOCKET == ret )     // socket 被其他线程接收了
            {
            	break;
            }
        }
        
        // 如果不加判端只赋值，在readBufDataSize不消耗时，会死循环，
        // 收到数据头和不完整数据会出现这种情况.
        if(leftSize > pClientCom->readBufDataSize)
        {
            leftSize = pClientCom->readBufDataSize;
        }
        else
        {
            break;
        }
    }

	return ret;
}

static int SendClientComBack( CLIENT_COMMUNICATE_T *pClientCom )
{
	int ret = 0;

	if( pClientCom->writeBufDataSize > 0 
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

static int InitSignalListenSocket( CLIENT_CONNECT_T *pClient )
{
	int ret, socket;

    // signal listen socket
	ret = SocketTcpListen( &socket, LISTEN_CLIENT_MSG_PORT, SOCKET_NOBLOCK );    
	if( 0 == ret )
    {
    	ret = AddClientCom( pClient, socket, CLIENT_SOCKET_STATE_LISTEN );
    	if( 0 != ret )
        {
        	ERRORPRINT( "failed: Add signal listen socket failed!\r\n" );
        	Close( socket );
        }
    }
	else
    {
    	ERRORPRINT( "failed: Create signal listen socket failed!\r\n" );
    }    
    
	return ret;
}

static int HandleDcpInsLocal( MSG_CMD_T *pLocalMsg, CLIENT_CONNECT_T *pClient )
{
	int ret;
	DCP_FUNC_LOCAL func; 
    
	func = GetDcpFuncLocal( (ushort)pLocalMsg->cmd );         
	ret = func( pLocalMsg, pClient );             

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
//该线程主要进行协议数据的处理
static void *DcpSignalThread( void *arg )
{
	int ret, retSend, i;
	SELECT_CLIENT_DATA_T selectResult;
	CLIENT_CONNECT_T *pClient = &g_clientConnect;
	MSG_CMD_T localCmd;
	SOCKET_ERR_T SignalsocketErr;

	InitClientConnect();
	ret = InitSignalListenSocket( pClient );
	if(0 != ret) 
    {
    	ERRORPRINT("!!!!!!!!failed: InitSignalListenSocket()!\r\n");
    	return NULL;    
    }    
    
	CORRECTPRINT( "$$$$$$$$%s start$$$$$$$$\r\n", __FUNCTION__ );

	while( g_dcpSignalTm.runFlag )
    {
        // local order
    	if ( MessageRecv(MSG_ID_DCP_SIGNAL, (char *)&localCmd, sizeof(localCmd)) >= 0 )
        {    
        	Memset( &SignalsocketErr, 0x00, sizeof(SignalsocketErr) );
        	ret = HandleDcpInsLocal( &localCmd, pClient );

            // 因为数据包是间歇性的，所以没有考虑大数据量的传输。
        	for ( i=1; i<pClient->num; i++)
            {
            	if(ret == 0 && pClient->client[i].writeBufDataSize > 0)
                {
                	retSend = SendClientComBack( &pClient->client[i] );
                	if( retSend < 0 )
                    {
                    	SignalsocketErr.socket[SignalsocketErr.num] = pClient->client[i].socket;
                    	SignalsocketErr.moveFlag[SignalsocketErr.num] = 0;
                    	SignalsocketErr.num++;
                    }
                	SVPrint( "local command transit, client:%d, len:%d\n", i, ret );
                }

            }
        	Free( localCmd.pData ); // 在指令发送的时候,一定要给localCmd.pData 赋有效值,NULL 也可以.

        	for( i = 0; i < SignalsocketErr.num; i++ )
            {                
            	DelClientCom( pClient, SignalsocketErr.socket[i], SignalsocketErr.moveFlag[i] );
            }
        }
        // net order
    	Memset( &SignalsocketErr, 0x00, sizeof(SignalsocketErr) );
    	ret = SelectClientComData( pClient, &selectResult );    
    	if(ret > 0)    
        {
        	for( i = 0; i < selectResult.num; ++i )
            {    
            	pClient->client[selectResult.index[i]].index = selectResult.index[i];                 
            	if( CLIENT_SOCKET_STATE_LISTEN == pClient->client[selectResult.index[i]].socketState )
                {                    
                	ret = AcceptClientCom( pClient, pClient->client[selectResult.index[i]].socket );
                	CORRECTPRINT( " ret(%d) = AcceptClientCom\r\n", ret );
                }
            	else
                {
                	ret = RecvClientComData( &pClient->client[selectResult.index[i]] );
                	if( ret <= 0 )
                    {                        
                    	SignalsocketErr.socket[SignalsocketErr.num] = \
                        	pClient->client[selectResult.index[i]].socket;                        
                    	SignalsocketErr.moveFlag[SignalsocketErr.num] = 0;
                    	SignalsocketErr.num++;
                    }
                	else
                    {
                    	ret     = HandLeClientComMsg( &pClient->client[selectResult.index[i]] );     // 处理所有接收到的消息
                    	retSend = SendClientComBack( &pClient->client[selectResult.index[i]] );             // 返回处理结果
                    	if( DCP_COM_ERR_NEED_CLOSE_SOCKET == ret
                            || retSend < 0 )
                        {                            
                        	SignalsocketErr.socket[SignalsocketErr.num] = 
                            	pClient->client[selectResult.index[i]].socket;
                        	SignalsocketErr.moveFlag[SignalsocketErr.num] = 0;
                        	SignalsocketErr.num++;
                        }
                    	else if( DCP_COM_ERR_NEED_MOVE_SOCKET == ret )
                        {
                        	SignalsocketErr.socket[SignalsocketErr.num] = 
                            	pClient->client[selectResult.index[i]].socket;
                        	SignalsocketErr.moveFlag[SignalsocketErr.num] = 1;
                        	SignalsocketErr.num++;
                        }
                    }
                }
            } 
        	for( i = 0; i < SignalsocketErr.num; i++ )
            {            
            	DelClientCom( pClient, SignalsocketErr.socket[i], SignalsocketErr.moveFlag[i] );
            }
        } 
        
    	Usleep(10);
    }
    
	ERRORPRINT( "$$$$$$$$%s stop$$$$$$$$\r\n", __FUNCTION__ );
    DelAllClientCom(pClient);

	return NULL;
}

void StartDcpSignalThread()
{
	int ret;
	g_dcpSignalTm.runFlag = 1;
	ret = ThreadCreate( &g_dcpSignalTm.id, DcpSignalThread, NULL );
	if( 0!= ret )
    {        
    	g_dcpSignalTm.runFlag = 0;
    	SVPrint( "error:ThreadCreate:%s\r\n", STRERROR_ERRNO );
    }
}

void StopDcpSignalThread()
{
	int ret;
	g_dcpSignalTm.runFlag = 0;
	ret = ThreadJoin( g_dcpSignalTm.id, NULL );
	if( 0 != ret )
    {
    	SVPrint( "error:ThreadJoin:%s\r\n", STRERROR_ERRNO );
    }
}

ushort DcpSignalConnectedUserNum()
{
	return g_clientConnect.num;
}

