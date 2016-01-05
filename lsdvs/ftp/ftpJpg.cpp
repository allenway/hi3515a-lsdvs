/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 利用ftp 上传抓拍到的jpg 文件
*******************************************************************************
*/
#include <unistd.h>
#include <stdio.h>
#include "debug.h"
#include "condition.h"
#include "thread.h"
#include "message.h"
#include "public.h"
#include "ttypes.h"
#include "proconJpg.h"
#include "paramManage.h"
#include "ftpFile.h"
#include "sysRunTime.h"
#include "pcpIpcSnap.h"

typedef enum _FtpJpgThreadCmd_
{
	FTP_JPG_THREAD_CMD_CHANGE_PARAM = 10,
} FTP_JPG_THREAD_CMD_EN;

static int g_ftpDirChangeFlag[MAX_JPG_CHN_NUM];

static void GetJpgTypeName( int type, char typeNmae[16] )
{
	switch( type )
    {
	case SNAP_TYPE_HAND:
    	strcpy( typeNmae, "hand" );
    	break;
	case SNAP_TYPE_TIMER:
    	strcpy( typeNmae, "timer" );
    	break;
	case SNAP_TYPE_ALARM_IO:
    	strcpy( typeNmae, "alarmio" );
    	break;
	case SNAP_TYPE_ALARM_MD:
    	strcpy( typeNmae, "alarmmd" );
    	break;
	case SNAP_TYPE_ALARM_LOST:
    	strcpy( typeNmae, "alarmlo" );
    	break;
	case SNAP_TYPE_ALARM_PIC_COMPARE:
    	strcpy( typeNmae, "alarmpc" );
    	break;
#if 0	    
	case SNAP_TYPE_ALARM_SHELTER:
    	strcpy( typeNmae, "alarmshelter" );
    	break;        
#endif	    
	default:
    	strcpy( typeNmae, "hand" );
    	break;
    }
}

#define FTP_JPG_PREFIX_PATH "jpg"
static void GetPath( int channel, JPG_INFO_T *jpgInfo, char path[128] )
{
	char typeNmae[16];
	GetJpgTypeName( jpgInfo->type, typeNmae );
	sprintf( path, "%s/%s/ch%02d/%s_%04d_%s.jpg", 
    	FTP_JPG_PREFIX_PATH, jpgInfo->datel, channel, 
    	jpgInfo->timel, jpgInfo->num, typeNmae );
}
/* jpg/2013-11-08/ch00/18h08m08s_0001_timer.jpg */
static int FtpSendJpg( int channel, JPG_INFO_T *jpgInfo, netbuf *controlc )
{
	int ret;
	netbuf *datac = NULL;
	char path[128];

	GetPath( channel, jpgInfo, path );
	ret = FtpFileOpen( path, FTPLIB_FILE_WRITE, controlc, &datac, g_ftpDirChangeFlag[channel] );
	if( 0 == ret )
    {
    	g_ftpDirChangeFlag[channel] = 0;
    	ret = FtpFileWrite( ((char *)jpgInfo) + sizeof(JPG_INFO_T), jpgInfo->len, datac );
    	if( ret < 0 )
        {
        	SVPrint( "error:FtpFileWrite:%s!\r\n", STRERROR_ERRNO );
        }
    	ret = FtpFileClose( controlc, datac );
    }

	return ret;
}

static ClCondition g_jpgCondition;
static THREAD_MAINTAIN_T g_jpgTm;
static void *FtpJpgThread( void *arg )
{
	int ret, cmd, i, count;
	netbuf *controlc[MAX_JPG_CHN_NUM] = { NULL, NULL, NULL, NULL, NULL};
	PARAM_CONFIG_FTP confFtp;
	JPG_INFO_T *jpgInfo;
	static int btime[MAX_JPG_CHN_NUM] = { -10, -10, -10, -10, -10 };
	int ctime[MAX_JPG_CHN_NUM];    
	uint proconFd[MAX_JPG_CHN_NUM];
	PROCON_NODE_T *pProconJpg;
	PCP_NODE_T *pPcpJpg;
    
	ret = ParamGetFtp( &confFtp );
	if( 0 != ret )
    {        
    	SVPrint( "Failed: ParamGetFtp()!\r\n" );
    	return NULL;
    }
	for( i = 0; i < MAX_JPG_CHN_NUM; ++i )
    {
    	if( i == (MAX_JPG_CHN_NUM-1) )
        {
        	proconFd[i] = PcpIpcSnapOpen( 0, OPEN_RDONLY );
        }
    	else
        {
        	proconFd[i] = ProconJpgOpen( i, OPEN_RDONLY );
        }
    	if( proconFd[i] < 0 )
        {
        	SVPrint( "Failed: ProconJpgOpen() or PcpIpcSnapOpen()!\r\n" );
        	return NULL;
        }
    }
    
	SVPrint( "FtpJpgThread start!\r\n" );
	while( g_jpgTm.runFlag )
    {
    	if( MessageRecv( MSG_ID_FTP_JPG_THREAD, (char *)&cmd, sizeof(cmd)) > 0 )
        {
        	switch( cmd )
            {
        	case FTP_JPG_THREAD_CMD_CHANGE_PARAM:                
            	ret = ParamGetFtp( &confFtp );
            	if( 0 == ret ) // 修改了参数,需要重连
                {                    
                	for( i = 0; i < MAX_JPG_CHN_NUM; ++i ) 
                    {
                    	FtpFileQuit( controlc[i] );
                    	controlc[i] = NULL;
                    }
                }                
            	break;            
            }
        }
    	if( 0 == confFtp.enable )
        {
        	if( 0 == g_jpgTm.runFlag )
            {
            	break; // 买个保险,防止不能退出
            }
        	g_jpgCondition.Wait();
        }    
    	else
        {
        	for( i = 0; i < MAX_JPG_CHN_NUM; ++i ) // 短线重连
            {                
            	if( NULL == controlc[i] )
                {
                	ctime[i] = SysRunTimeGet();
                	if( ctime[i] - btime[i] >= 10 )
                    {
                    	btime[i] = ctime[i];
                    	ret = FtpFileInit( confFtp.ip, confFtp.user, 
                                	confFtp.passwd, &controlc[i] );    
                    	if( 0 == ret )
                        {
                        	g_ftpDirChangeFlag[i] = 1;
                        }
                    }                    
                } // if( NULL == controlc[i]
            }    
        }

    	count = 0;
    	for( i = 0; i < MAX_JPG_CHN_NUM; ++i )
        {            
        	if( NULL != controlc[i] )
            {
            	if( 1 == confFtp.jpgUpChannel[i] )
                {
                	if( i == (MAX_JPG_CHN_NUM-1) )
                    {
                    	pPcpJpg = PcpIpcSnapRead( proconFd[i] );    
                    	if( NULL == pPcpJpg )
                        {
                        	count++;
                        	if( count >= MAX_JPG_CHN_NUM )
                            {
                            	sleep( 2 );                            
                            }
                        }
                    	else
                        {                        
                        	jpgInfo = (JPG_INFO_T *)pPcpJpg->data;
                        	ret = FtpSendJpg( i, jpgInfo, controlc[i] );
                        	if( -1 == ret )
                            {
                            	FtpFileQuit( controlc[i] );
                            	controlc[i] = NULL;
                            }
                        	PcpIpcSnapFree( pPcpJpg );
                        }    
                    }
                	else
                    {
                    	pProconJpg = ProconJpgRead( proconFd[i] );    
                    	if( NULL == pProconJpg )
                        {
                        	count++;
                        	if( count >= MAX_JPG_CHN_NUM )
                            {
                            	sleep( 2 );                            
                            }
                        }
                    	else
                        {                        
                        	jpgInfo = (JPG_INFO_T *)pProconJpg->data;
                        	ret = FtpSendJpg( i, jpgInfo, controlc[i] );
                        	if( -1 == ret )
                            {
                            	FtpFileQuit( controlc[i] );
                            	controlc[i] = NULL;
                            }
                        	ProconJpgFree( pProconJpg );
                        }    
                    }
                } // if( 1 == confFtp.jpgUpChannel[i]
            } // if( NULL != controlc[i]
        }
    } // while( g_jpgTm.runFlag

	for( i = 0; i < MAX_JPG_CHN_NUM; ++i )
    {
    	if( 0 != proconFd[i] )
        {
        	if( i == (MAX_JPG_CHN_NUM-1) )
            {
            	PcpIpcSnapClose( proconFd[i] );
            }
        	else
            {
            	ProconJpgClose( proconFd[i] );
            }
        }
    }
    
	SVPrint( "FtpJpgThread stop!\r\n");
	return NULL;
}

void StartFtpJpgThread()
{
	int ret;
	g_jpgTm.runFlag = 1;
	ret = ThreadCreate( &g_jpgTm.id, FtpJpgThread, NULL );
	if( 0 != ret )
    {        
    	g_jpgTm.runFlag = 0;
    	SVPrint( "error:ThreadCreate:%S\r\n", STRERROR_ERRNO );
    }
}

void StopFtpJpgThread()
{
	int ret;
	g_jpgTm.runFlag = 0;    
	g_jpgCondition.Signal();
	ret = ThreadJoin( g_jpgTm.id, NULL );
	if( 0 != ret )
    {        
    	SVPrint( "error:ThreadJoin:%S\r\n", STRERROR_ERRNO );
    }}

/*
* fn: 当ftp 参数被修改后,通过此函数通知
*/
void FtpJpgSendParamChangeMessage()
{
	int cmd = FTP_JPG_THREAD_CMD_CHANGE_PARAM;
    
	MessageSend( MSG_ID_FTP_JPG_THREAD, (char *)&cmd, sizeof(cmd) );
	g_jpgCondition.Signal();
}

