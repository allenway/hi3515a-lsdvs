/********************************************************************************
**	Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**	author        :  sven
**	version       :  v1.0
**	date           :  2013.10.10
**	description  : bhdcp 音视频流发送
********************************************************************************/

#include "const.h"
#include "debug.h"
#include "fit.h"
#include "thread.h"
#include "malloc.h"
#include "dcpStream.h"
#include "linuxFile.h"
#include "proconH264.h"
#include "dcpStreamMessage.h"
#include "dcpStreamSend.h"

static void CloseAllStream( DCP_STREAM_T *pDcpStream )
{
	int i;
    
	for( i = 0; i < pDcpStream->csnum; i++ )
    {
    	ProconH264Close( pDcpStream->streamCh[i].fd );
    	Close( pDcpStream->streamCh[i].socket );
    }

	pDcpStream->csnum = 0;
}

/*************************************************************************
* fn: 检查要操作的通道是否已经存在
* 返回: < 0, 不存在;
        >= 0, 存在,而且该返回值是该通道在数组中的下标
**************************************************************************/
static int CheckChannelExist( int channel, DCP_STREAM_T *pDcpStream )
{
	int ret = -1;
	int i;

    
	for( i = 0; i < pDcpStream->csnum; i++ )
    {
    	if( pDcpStream->streamCh[i].ch == channel )
        {
        	ret = i;
        	break;
        }
    }

	return ret;
}

static int StartChannelStream( MSG_CMD_T *pMsgCmd, DCP_STREAM_T *pDcpStream )
{
	int ret = -1;
	uint fd;
	CHANNEL_START_STREAM_T css;

	Memcpy( &css, pMsgCmd->pData, sizeof(css) );

	ret = CheckChannelExist( css.channel, pDcpStream );
	if( ret >= 0 )
    {
    	SVPrint( "warning:ch(%d) is exist!\r\n", css.channel );        
    	Close( css.socket );
    	return DCP_STREAM_ERR_CHANNEL_IS_EXIST;
    }

    //pDcpStream->streamCh[pDcpStream->csnum].ch         = css.channel;
    //pDcpStream->streamCh[pDcpStream->csnum].socket     = css.socket;
	fd = ProconH264Open( css.channel, OPEN_RDONLY );
	if( fd > 0 )
    {
    	ret = 0;
    	pDcpStream->streamCh[pDcpStream->csnum].fd      = fd;
        pDcpStream->streamCh[pDcpStream->csnum].ch         = css.channel;
        pDcpStream->streamCh[pDcpStream->csnum].socket     = css.socket;
    	pDcpStream->csnum++;
    	FitMympiForceIframe( css.channel );
        //SVPrint("ch:%d,sock:%d,fd:%d add to arry to start send!\n",css.channel,css.socket,fd);
    }
	else
    {
    	ret = -1;
    	Close( css.socket );
    	ERRORPRINT( "failed:ProconH264Open()!\r\n" );
    }

	return ret;
}

static int StopChannelStream( MSG_CMD_T *pMsgCmd, DCP_STREAM_T *pDcpStream )
{
	int ret,i;
    //int left;
	CHANNEL_START_STREAM_T css;

	Memcpy( &css, pMsgCmd->pData, sizeof(css) );
    
	ret = CheckChannelExist( css.channel, pDcpStream );
    SVPrint("CheckChannelExist return (%d)\n",ret);
	if( ret >= 0 )
    {    
        SVPrint("find channel:%d,socket:%d,fd:%d,and then stop it!!!\n",css.channel,pDcpStream->streamCh[ret].socket,
            pDcpStream->streamCh[ret].fd);
    	ProconH264Close( pDcpStream->streamCh[ret].fd );
        pDcpStream->streamCh[ret].fd = 0;
    	Close( pDcpStream->streamCh[ret].socket );
        pDcpStream->streamCh[ret].socket = -1;
        for(i = ret;i<(pDcpStream->csnum)-1;i++)
        {
            pDcpStream->streamCh[i] = pDcpStream->streamCh[i+1];
        }
        //慎用
        //left = pDcpStream->csnum - ret - 1;
        //if( left > 0 && left < pDcpStream->csnum )
        //{
        //	Memmove( &pDcpStream->streamCh[ret], &pDcpStream->streamCh[ret+1], left * sizeof(pDcpStream->streamCh) );
        //}
    	pDcpStream->csnum--;
    }

	return 0;
}

static int QuitStream( MSG_CMD_T *pMsgCmd, DCP_STREAM_T *pDcpStream )
{
	return DCP_STREAM_ERR_QUIT;
}



static int HandleDsmInternalCmd( MSG_CMD_T *pMsgCmd, DCP_STREAM_T *pDcpStream )
{
	int ret = 0;
    
	switch( pMsgCmd->cmd )
    {
    	case DSM_CMD_START_STREAM:
        	ret = StartChannelStream( pMsgCmd, pDcpStream );
        	break;
    	case DSM_CMD_STOP_STREAM:
        	ret = StopChannelStream( pMsgCmd, pDcpStream );
        	break;
    	case DSM_CMD_QUIT_STREAM:
        	ret = QuitStream( pMsgCmd, pDcpStream );
        	break;
    	default:
        	break;
    }

	Free( pMsgCmd->pData );

	return ret;
}

static int GetStreamAndSend( uint threadId, DCP_STREAM_T *pDcpStream )
{
	int i, ret = -1, sendRet;
	PROCON_NODE_T *pH264;

    //SVPrint("there is %d chn\n",pDcpStream->csnum);
    
	for( i = 0; i < pDcpStream->csnum; i++ )
    {
    	pH264 = ProconH264Read( pDcpStream->streamCh[i].fd );
    	if( NULL != pH264 )
        {
            /*STREAM_HEAD_T *temp_p = (STREAM_HEAD_T *)(&(pH264->data[0]));
            if(temp_p->packHead.packType == 0&&pDcpStream->streamCh[i].ch == 0)
            {
                printf("send frame no is :%d!!!!!!!!\n",temp_p->frameHead.frameNo);
            }*/
            
        	sendRet = DssSendStream( pDcpStream->streamCh[i].socket, pH264 );
        	if( -1 == sendRet ) // 发送出错,关闭
            {
                ERRORPRINT("send error!!!!!!!!!!!!!!!\n");
            	DcpStopChannelStream( threadId, pDcpStream->streamCh[i].ch );    
            }
            
        	ProconH264Free( pH264 );
        	ret = 0;
        }        
    }

	return ret;
}

void *DcpStreamThread( void *args )
{
	int 	ret;
	uint 	myid;
	DCP_STREAM_T     *pDcpStream;
	MSG_CMD_T     	msgCmd;

	myid = ThreadSelf();
	ThreadDetach( myid );
    
	pDcpStream = (DCP_STREAM_T *)Calloc( 1, sizeof(DCP_STREAM_T) );
	if( NULL == pDcpStream )
    {
    	ERRORPRINT( "failed: Calloc()!\r\n" );
    	return NULL;
    }
    
	CORRECTPRINT( "########%s start, threadId=%u!\r\n", __FUNCTION__, myid  );
	while( 1 )
    {
    	if( 0 <=  DsmMessageRecv( myid, (char *)&msgCmd, sizeof(msgCmd)) )
        {
        	ret = HandleDsmInternalCmd( &msgCmd, pDcpStream );
        	if( DCP_STREAM_ERR_QUIT == ret )
            {
            	break;
            }
        }
        
    	if( 0 == pDcpStream->csnum )
        {
        	Usleep(200);
        }
    	else
        {
        	ret = GetStreamAndSend( myid, pDcpStream );
        	if( -1 == ret ) // 没有获取到视频
            {
            	Usleep(10000);
            }
        	else
            {
            	//Usleep(10*1000);//yfchanged
            }
        }
    }

	CloseAllStream( pDcpStream );
	Free( pDcpStream );
    
	ERRORPRINT( "#############%s stop, threadId=%u!\r\n", __FUNCTION__, myid  );
	return NULL;
}

/********************************************************************************
* fn: 创建一个流线程
* threadId: out, 成功创建后的线程id, 这个id 会作为LOGIN_DVS_RES_T 中的
	userStreamMark 成员发给客户端,客户端在请求某个通道流的时候带上它
********************************************************************************/
int DcpCreateStreamThread( uint *threadId )
{
	int ret;
	if( NULL == threadId )
    {
    	SVPrint( "error:NULL == threadId!\r\n" );
    	return -1;
    }
	ret = ThreadCreate( (pthread_t *)threadId, DcpStreamThread, NULL );
	if( 0 != ret )
    {    
    	SVPrint( "failed:ThreadCreate()!\r\n" );        
    }

	return ret;
}

/*******************************************************************************
* fn: 销毁某个客户端对应的流线程
* threadId: DcpCreateStreamThread() 中的threadId 输出
*******************************************************************************/
void DcpDestoryStreamThread( uint threadId )
{
	MSG_CMD_T MsgCmd;

	Memset( &MsgCmd, 0x00, sizeof(MsgCmd) );

	MsgCmd.cmd = DSM_CMD_QUIT_STREAM;
    MsgCmd.pData = NULL;

	DsmMessageSend( threadId, (char *)&MsgCmd, sizeof(MsgCmd) );
}

/******************************************************************************
* fn: 开始某个通道的流
* threadId: DcpCreateStreamThread() 中的threadId 输出
* channel: 被停止的通道
* socket: 客户端请求流的时候所建立的链接
*******************************************************************************/
int DcpStartChannelStream( uint threadId, int channel, int socket )
{
	int ret;
	CHANNEL_START_STREAM_T css;
	MSG_CMD_T MsgCmd;

	css.channel = channel;
	css.socket     = socket;

	MsgCmd.pData = (char *)Malloc( sizeof(css) );
	if( NULL != MsgCmd.pData )
    {
    	MsgCmd.cmd = DSM_CMD_START_STREAM;
    	Memcpy( MsgCmd.pData, &css, sizeof(css) );

    	ret = DsmMessageSend( threadId, (char *)&MsgCmd, sizeof(MsgCmd) );
    	if( ret > 0 )
        {
        	ret = 0;
        }
    }
	else
    {
    	ret = -1;
    }

	return ret;    
}

/****************************************************************************
* fn: 停止某个通道的流
* threadId: DcpCreateStreamThread() 中的threadId 输出
* channel: 被停止的通道
******************************************************************************/
int DcpStopChannelStream( uint threadId, int channel )
{
	int ret;
	CHANNEL_START_STREAM_T css;
	MSG_CMD_T MsgCmd;

    memset(&css,0,sizeof(css));//
    memset(&MsgCmd,0,sizeof(MsgCmd));//

	css.channel = channel;
	MsgCmd.pData = (char *)Malloc( sizeof(css) );
	if( NULL != MsgCmd.pData )
    {
    	MsgCmd.cmd = DSM_CMD_STOP_STREAM;
    	Memcpy( MsgCmd.pData, &css, sizeof(css) );

    	ret = DsmMessageSend( threadId, (char *)&MsgCmd, sizeof(MsgCmd) );
    }
	else
    {
    	ret = -1;
    }
    
	return ret;    
}

