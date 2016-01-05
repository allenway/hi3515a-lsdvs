/*
*********************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.11.06
**  description  : 封装抓拍的MPI
*********************************************************************
*/
#if defined MCU_HI3515

#include <stdio.h>
#include <string.h>
#include "const.h"
#include "debug.h"
#include "hi_comm_venc.h"
#include "mpi_venc.h"
#include "snapMpi.h"
#include "netSocket.h"
#include "vencParamEasy.h"
#include "osdApply.h"
#include "mutex.h"
#include "encComm.h"
#include "malloc.h"
#include "proconJpg.h"
#include "timeExchange.h"

static ClMutexLock g_snapMutex[REAL_CHANNEL_NUM];
/*
* fn: 创建抓怕通道组和抓怕通道
* snapCh: 哪个抓拍通道
* videoStandard: 视频标准
* resolution: 抓拍图像的分辨率
*/
static int CreateSnapGrpAndChn( int snapCh, int videoStandard, int resolution )
{
	int ret;
	int ViDevId = snapCh / MAX_CHANNEL_PER_DEV;
	int ViChn     = snapCh % MAX_CHANNEL_PER_DEV;
	int group     = SNAP_CHANNEL_START + snapCh;
	int physicsSnapCh	= group;
	VENC_ATTR_JPEG_S 	stJpegAttr;
	VENC_CHN_ATTR_S 	stAttr;

    /*JPEG编码属性*/
	if( videoStandard == HI_PAL )
    {
    	switch( resolution )
        {
    	case HI_QCIF:
        	stJpegAttr.u32BufSize     = 176 * 144 * 2;
        	stJpegAttr.u32PicWidth     = 176;
        	stJpegAttr.u32PicHeight = 144;
        	stJpegAttr.u32MCUPerECS = (176 * 144) / 256;
        	break;
    	case HI_CIF:
        	stJpegAttr.u32BufSize     = 352 * 288 * 2;
        	stJpegAttr.u32PicWidth     = 352;
        	stJpegAttr.u32PicHeight = 288;
        	stJpegAttr.u32MCUPerECS = (352 * 288) / 256;
        	break;
    	case HI_HD1:
        	stJpegAttr.u32BufSize     = 704 * 288 * 2;
        	stJpegAttr.u32PicWidth     = 704;
        	stJpegAttr.u32PicHeight = 288;
        	stJpegAttr.u32MCUPerECS = (704 * 288) / 256;
        	break;
    	case HI_D1:
        	stJpegAttr.u32BufSize     = 704 * 576 * 2;
        	stJpegAttr.u32PicWidth     = 704;
        	stJpegAttr.u32PicHeight = 576;
        	stJpegAttr.u32MCUPerECS = (704 * 576) / 256;
        	break;
    	case HI_QVGA:
        	stJpegAttr.u32BufSize     = 320 * 240 * 2;
        	stJpegAttr.u32PicWidth     = 320;
        	stJpegAttr.u32PicHeight = 240;
        	stJpegAttr.u32MCUPerECS = (320 * 240) / 256;
        	break;
    	case HI_VGA:
        	stJpegAttr.u32BufSize     = 640 * 480 * 2;
        	stJpegAttr.u32PicWidth     = 640;
        	stJpegAttr.u32PicHeight = 480;
        	stJpegAttr.u32MCUPerECS = (640 * 480) / 256;
        	break;
    	default:
        	break;
        }
    } // if( videoStandard == HI_PAL
	else
    {
    	switch( resolution )
        {
    	case HI_QCIF:
        	stJpegAttr.u32BufSize     = 176 * 128 * 2;
        	stJpegAttr.u32PicWidth     = 176;
        	stJpegAttr.u32PicHeight = 112; 
        	stJpegAttr.u32MCUPerECS = (176 * 112) / 256;
        	break;
    	case HI_CIF:
        	stJpegAttr.u32BufSize     = 352 * 240 * 2;
        	stJpegAttr.u32PicWidth     = 352;
        	stJpegAttr.u32PicHeight = 240;
        	stJpegAttr.u32MCUPerECS = (352 * 240) / 256;
        	break;
    	case HI_HD1:
        	stJpegAttr.u32BufSize     = 704 * 240 * 2;
        	stJpegAttr.u32PicWidth     = 704;
        	stJpegAttr.u32PicHeight = 240;
        	stJpegAttr.u32MCUPerECS = (704 * 240) / 256;
        	break;
    	case HI_D1:
        	stJpegAttr.u32BufSize     = 704 * 480 * 2;
        	stJpegAttr.u32PicWidth     = 704;
        	stJpegAttr.u32PicHeight = 480;
        	stJpegAttr.u32MCUPerECS = (704 * 480) / 256;
        	break;
    	case HI_QVGA:
        	stJpegAttr.u32BufSize     = 320 * 240 * 2;
        	stJpegAttr.u32PicWidth     = 320;
        	stJpegAttr.u32PicHeight = 240;
        	stJpegAttr.u32MCUPerECS = (320 * 240) / 256;
        	break;
    	case HI_VGA:
        	stJpegAttr.u32BufSize     = 640 * 480 * 2;
        	stJpegAttr.u32PicWidth     = 640;
        	stJpegAttr.u32PicHeight = 480;
        	stJpegAttr.u32MCUPerECS = (640 * 480) / 256;
        	break;
    	default:
        	break;
        }
    }
    
	stJpegAttr.bVIField = HI_FALSE;
	stJpegAttr.bByFrame = HI_TRUE;
	stJpegAttr.u32ImageQuality = 0; 
	memset( &stAttr, 0, sizeof(VENC_CHN_ATTR_S) );
	stAttr.enType = PT_JPEG;
	stAttr.pValue = (HI_VOID *)&stJpegAttr;

	ret = HI_MPI_VENC_CreateGroup( group );
	if( ret != 0 )
    {
    	SVPrint( "HI_MPI_VENC_CreateGroup err 0x%x\r\n", ret );
    	return -1;
    }
	ret = HI_MPI_VENC_CreateChn( physicsSnapCh, &stAttr, HI_NULL );
	if( ret != 0 )
    {
    	SVPrint( "HI_MPI_VENC_CreateChn err 0x%x\r\n", ret );
    	return -1;
    }
	ret = HI_MPI_VENC_BindInput( group, ViDevId, ViChn );
	if( ret != 0 )
    {
    	SVPrint( "HI_MPI_VENC_BindInput err 0x%x\r\n", ret );
    	return -1;
    }

	if( 0 == ret )
    {
    	FiOsdDoKindOfEvent( OSDEVENT_VECREATE, physicsSnapCh );
    }
    
	return ret;
}

/*
* fn: 销毁抓怕通道组和抓怕通道
*/
static int DestorySnapGrpAndChn( int snapCh )
{
	int ret = 0;
	int group	= SNAP_CHANNEL_START + snapCh;
	int physicsSnapCh = group;

	ret = HI_MPI_VENC_UnbindInput( group );
	if (ret != 0)
    {
    	SVPrint( "HI_MPI_VENC_UnbindInput err 0x%x\n", ret );
    	return -1;
    }
	ret = HI_MPI_VENC_DestroyChn( physicsSnapCh );
	if( ret != 0 )
    {
    	SVPrint( "HI_MPI_VENC_DestroyChn err 0x%x\n", ret );
    	return -1;
    }
	ret = HI_MPI_VENC_DestroyGroup( group );
	if( ret != 0 )
    {
    	SVPrint( "HI_MPI_VENC_DestroyGroup err 0x%x\n", ret );
    	return -1;
    }
	if( 0 == ret )
    {
    	FiOsdDoKindOfEvent(OSDEVENT_VEDESTROY, physicsSnapCh );
    }
    
	return ret;
}

/*
* 开始抓怕
*/
static int StartSnapPic( int snapCh )
{
	int ret = 0;
	int group	= SNAP_CHANNEL_START + snapCh;
	int physicsSnapCh	= group;

	ret = HI_MPI_VENC_RegisterChn( group, physicsSnapCh );
	if (ret != 0)
    {
    	SVPrint("HI_MPI_VENC_RegisterChn err 0x%x\n", ret);
    	return -1;
    }
	ret = HI_MPI_VENC_StartRecvPic( physicsSnapCh );
	if (ret != 0)
    {
    	SVPrint("HI_MPI_VENC_StartRecvPic err 0x%x\n", ret);
    	return -1;
    }
	return ret;
}

/*
* 停止抓怕
*/
int StopSnapPic( int snapCh )
{
	int ret = 0;
	int group = SNAP_CHANNEL_START + snapCh;
	int physicsSnapCh = group;

	ret = HI_MPI_VENC_StopRecvPic( physicsSnapCh );
	if( ret != 0 )
    {
    	SVPrint( "HI_MPI_VENC_StopRecvPic err 0x%x\n", ret );
    	return -1;
    }
	ret = HI_MPI_VENC_UnRegisterChn( physicsSnapCh );
	if( ret != 0 )
    {
    	SVPrint( "HI_MPI_VENC_UnRegisterChn err 0x%x\n", ret );
    	return -1;
    }
	return ret;
}

/*
 * 把抓怕到的数据存到buf里面
 */
static int SaveSnapPicDataToBuf( int snapCh, void *buf, unsigned int *len )
{
	unsigned int i;
	int size = 0;
	int ret;
	int s32VencFd;
	VENC_CHN_STAT_S 	stStat;
	VENC_STREAM_S     	stStream;
	char *jpegBuf = (char *) buf;
	int group = SNAP_CHANNEL_START + snapCh;
	int physicsSnapCh = group;

	s32VencFd = HI_MPI_VENC_GetFd( physicsSnapCh );
    
	if (s32VencFd < 0)
    {
    	SVPrint( "failed:HI_MPI_VENC_GetFd err \r\n" );
    	return -1;
    }

	ret = SelectRead( s32VencFd, 5000 );
	if( ret < 0 )
    {
    	SVPrint( "SelectRead err:%s!\r\n", STRERROR_ERRNO );
    	return -1;
    }
	else if( 0 == ret )
    {
    	SVPrint( "SelectRead time out!\r\n" );
    	return -1;
    }
	else
    {    
    	ret = HI_MPI_VENC_Query( physicsSnapCh, &stStat );
    	if( ret != 0 )
        {
        	SVPrint( "failed: I_MPI_VENC_Query:0x%x\r\n", ret );
        	return -1;
        }
    	stStream.pstPack = (VENC_PACK_S*) Malloc( sizeof(VENC_PACK_S) * stStat.u32CurPacks );
    	if( NULL == stStream.pstPack )
        {
        	SVPrint( "error: malloc memory!\r\n" );
        	return -1;
        }
    	stStream.u32PackCount = stStat.u32CurPacks;
    	SVPrint( "stStream.u32PackCount = %d, %d!\r\n", stStream.u32PackCount, stStat.u32CurPacks );
    	ret = HI_MPI_VENC_GetStream( physicsSnapCh, &stStream, HI_FALSE );
    	if (0 != ret)
        {
        	SVPrint( "failed:HI_MPI_VENC_GetStream:0x%x\n", ret );
        	Free( stStream.pstPack );
        	stStream.pstPack = NULL;
        	return -1;
        }

    	for( i = 0, size = 0; i < stStream.u32PackCount; i++ )
        {
        	size += stStream.pstPack[i].u32Len[0];
        	size += stStream.pstPack[i].u32Len[1];
        }
    	if( size > MAX_JPEG_SIZE )
        {
        	SVPrint( "JPEG is %d too len, and lost it\n", size );
        	return -1;
        }
    	size = 0;
        /*get JPEG data*/
    	jpegBuf[0] = 0xFF;
    	jpegBuf[1] = 0xD8;
    	size += 2;
    	for (i = 0; i < stStream.u32PackCount; i++)
        {
        	memcpy(	jpegBuf + size, stStream.pstPack[i].pu8Addr[0],
                                	stStream.pstPack[i].u32Len[0]);
        	size += stStream.pstPack[i].u32Len[0];
        	memcpy(	jpegBuf + size, stStream.pstPack[i].pu8Addr[1],
                                	stStream.pstPack[i].u32Len[1]);
        	size += stStream.pstPack[i].u32Len[1];
        }
    	jpegBuf[size++] = 0xFF;
    	jpegBuf[size++] = 0xD9;
        *len = size;

    	ret = HI_MPI_VENC_ReleaseStream( physicsSnapCh, &stStream );
    	if( 0 != ret )
        {
        	SVPrint( "HI_MPI_VENC_ReleaseStream err:0x%x\n", ret );
        	Free( stStream.pstPack );
        	stStream.pstPack = NULL;
        	return -1;
        }
    	Free( stStream.pstPack );
    	stStream.pstPack = NULL;
        
    }    
    
	return ret;
}

/*
* fn: 获取一张抓怕图片并把数据存到buf里面
*/
static int GetOneSnapPic( int snapCh, void *buf, unsigned int *len )
{
	int ret = 0;

	ret = StartSnapPic( snapCh );
	if( ret != 0 )
    {
    	SVPrint( "Start Snap Pic err 0x%x\n", ret );
    	return -1 ;
    }

	ret = SaveSnapPicDataToBuf( snapCh, buf, len );
	if (ret != 0)
    {
    	StopSnapPic( snapCh );
    	return -1;
    }

	ret = StopSnapPic( snapCh );
	if( ret != 0 )
    {
    	SVPrint( "Stop Snap Pic err 0x%x\r\n", ret );
    	return -1;
    }

	return 0;
}

/*
* fn: 抓拍接口
* snapCh: 用户通道 0 ~ x
* len: out, 抓拍到图片的长度
*/
int SnapMpiGetJpg( int snapCh, char *buf, unsigned int *len )
{
	int ret;
	if( snapCh < 0 || snapCh >= REAL_CHANNEL_NUM )
    {
    	SVPrint( "error:snapCh < 0 || snapCh >= REAL_CHANNEL_NUM!\r\n" );
    	return -1;
    }
    
	g_snapMutex[snapCh].Lock();
    
	ret = GetOneSnapPic( snapCh, buf, len );
	if(  ret < 0 )
    {
    	SVPrint( "GetOneSnapPic err !\r\n" );
    }
    
	g_snapMutex[snapCh].Unlock();
    
	return ret;
}

void SnapMpiInit()
{
	int i;
	uchar videoStandard = VencParamEasyGetVideoStandard();
	uchar resolutin[REAL_CHANNEL_NUM];

	for( i = 0; i < REAL_CHANNEL_NUM; ++i )
    {    
    	resolutin[i] = VencParamEasyGetResolution( i );
    	CreateSnapGrpAndChn( i, videoStandard, resolutin[i] );        
    }
}

void SnapMpiDeinit()
{
	int i;

	for( i = 0; i < REAL_CHANNEL_NUM; ++i )    
    {                
    	DestorySnapGrpAndChn( i );
    }
}

/* ===== 函数重载, 主要是满足 procon 设计 ===== */

static uint g_proconJpgFd[REAL_CHANNEL_NUM];
int GetChnProconJpgFd( int channel )
{
	return g_proconJpgFd[channel];
}

void InitProconJpgFd()
{
	int i;
	for( i = 0; i < REAL_CHANNEL_NUM; ++i )
    {
    	g_proconJpgFd[i] = ProconJpgOpen( i, OPEN_WRONLY );
    	if( 0 == g_proconJpgFd[i] )
        {
        	SVPrint( "ch(%d) ProconJpgOpen failed!\r\n", i );
        }
    }
}

void DeinitProconJpgFd()
{
	int i;
	for( i = 0; i < REAL_CHANNEL_NUM; ++i )
    {
    	ProconJpgClose( g_proconJpgFd[i] );
    	g_proconJpgFd[i] = 0;
    }
}

/*
* fn: 打开jpg 通道后获取jpg 信息
* snapCh: 用户通道 0 ~ x
* stStream: out, 从mpi 直接获取的信息
*/
static int GetJpgStream( int snapCh, VENC_STREAM_S *stStream )
{
	int ret;
	int s32VencFd;
	VENC_CHN_STAT_S 	stStat;
	int group = SNAP_CHANNEL_START + snapCh;
	int physicsSnapCh = group;    

	s32VencFd = HI_MPI_VENC_GetFd( physicsSnapCh );    
	if (s32VencFd < 0)
    {
    	SVPrint( "failed:HI_MPI_VENC_GetFd err \r\n" );
    	return -1;
    }

	ret = SelectRead( s32VencFd, 1000 );
	if( ret < 0 )
    {
    	SVPrint( "SelectRead err:%s!\r\n", STRERROR_ERRNO );
    	return -1;
    }
	else if( 0 == ret )
    {
    	SVPrint( "SelectRead time out!\r\n" );
    	return -1;
    }
	else
    {    
    	ret = HI_MPI_VENC_Query( physicsSnapCh, &stStat );
    	if( ret != 0 )
        {
        	SVPrint( "failed: I_MPI_VENC_Query:0x%x\r\n", ret );
        	return -1;
        }
    	stStream->pstPack = (VENC_PACK_S*)Malloc( sizeof(VENC_PACK_S) * stStat.u32CurPacks );
    	if( NULL == stStream->pstPack )
        {
        	SVPrint( "error: malloc memory!\r\n" );
        	return -1;
        }
    	stStream->u32PackCount = stStat.u32CurPacks;
    	ret = HI_MPI_VENC_GetStream( physicsSnapCh, stStream, HI_FALSE );
    	if (0 != ret)
        {
        	SVPrint( "failed:HI_MPI_VENC_GetStream:0x%x\n", ret );
        	Free( stStream->pstPack );
        	stStream->pstPack = NULL;
        	return -1;
        }    
    }    
    
	return ret;
}

/*
* fn: 释放从 GetOneSnapPicInfo() 获取到的资源
*/
static int ReleaseJpgStreamAndStopSnapCh( int snapCh, VENC_STREAM_S *stStream )
{
	int ret;
	int group = SNAP_CHANNEL_START + snapCh;
	int physicsSnapCh = group;    
    
	ret = HI_MPI_VENC_ReleaseStream( physicsSnapCh, stStream );
	if( 0 != ret )
    {
    	SVPrint( "failed:HI_MPI_VENC_ReleaseStream err:0x%x\n", ret );
    	Free( stStream->pstPack );
    	stStream->pstPack = NULL;
    	StopSnapPic( snapCh );
    	return -1;
    }
	Free( stStream->pstPack );
	stStream->pstPack = NULL;

	ret = StopSnapPic( snapCh );
	if( ret != 0 )
    {
    	SVPrint( "failed: stop Snap Pic err 0x%x\r\n", ret );
    	return -1;
    }

	return ret;
}

/*
* fn: 获取一张抓怕图片
* snapCh: 用户通道 0 ~ x
* stStream: out, 从mpi 直接获取的信息
*/
static int GetOneSnapPicInfo( int snapCh, VENC_STREAM_S *stStream )
{
	int ret = 0;

	ret = StartSnapPic( snapCh );
	if( ret != 0 )
    {
    	SVPrint( "Start Snap Pic err 0x%x\n", ret );
    	return -1 ;
    }

	ret = GetJpgStream( snapCh, stStream );
	if (ret != 0)
    {
    	StopSnapPic( snapCh );
    	return -1;
    }

	return 0;
}

static void GetJpgDatetimeInfo( int sec, char *date, char *time )
{
	int YY, MM, DD, hh, mm, ss;
	FiTimeUtcToHuman( sec, &YY, &MM, &DD, &hh, &mm, &ss );
	sprintf( date, "%04d-%02d-%02d", YY, MM, DD );    
	sprintf( time, "%02dh%02dm%02ds", hh, mm, ss );
}

/*
* fn: 抓拍接口
* snapCh: 用户通道 0 ~ x
* snapType: 抓怕类型
*/
int SnapMpiGetJpgAndToProcon( int snapCh, uint snapType )
{
	int ret;
	uint i;
	static uint 	num[REAL_CHANNEL_NUM];
	JPG_INFO_T		jpgInfo;
	VENC_STREAM_S 	stStream;
	DATA_PIECE_T	jpgPiece;
	char jpgPrefix[2], jpgSuffix[2];
    
	if( snapCh < 0 || snapCh >= REAL_CHANNEL_NUM )
    {
    	SVPrint( "error:snapCh < 0 || snapCh >= REAL_CHANNEL_NUM!\r\n" );
    	return -1;
    }

	g_snapMutex[snapCh].Lock();
    
	ret = GetOneSnapPicInfo( snapCh, &stStream );
	if(  ret == 0 )
    {
    	jpgInfo.num     = num[snapCh]++;
    	jpgInfo.type     = snapType;
    	GetJpgDatetimeInfo( time(NULL), jpgInfo.datel, jpgInfo.timel );    
    	jpgInfo.len	    = 0;
        
    	jpgPiece.count     = 0;
    	jpgPiece.buf[jpgPiece.count] = (char *)&jpgInfo;
    	jpgPiece.len[jpgPiece.count] = sizeof( jpgInfo );
    	jpgPiece.count++;

    	jpgPrefix[0] = 0xFF;
    	jpgPrefix[1] = 0xD8;
    	jpgPiece.buf[jpgPiece.count] = jpgPrefix;
    	jpgPiece.len[jpgPiece.count] = sizeof( jpgPrefix );
    	jpgPiece.count++;
    	jpgInfo.len	+= sizeof( jpgPrefix );

    	for( i = 0; i < stStream.u32PackCount; ++i )
        {
        	jpgPiece.buf[jpgPiece.count] = (char *)stStream.pstPack[i].pu8Addr[0];
        	jpgPiece.len[jpgPiece.count] = stStream.pstPack[i].u32Len[0];
        	jpgPiece.count++;
        	jpgInfo.len += stStream.pstPack[i].u32Len[0];
        	if( stStream.pstPack[i].u32Len[1] > 0 )
            {
            	jpgPiece.buf[jpgPiece.count] = (char *)stStream.pstPack[i].pu8Addr[1];
            	jpgPiece.len[jpgPiece.count] = stStream.pstPack[i].u32Len[1];
            	jpgPiece.count++;
            	jpgInfo.len += stStream.pstPack[i].u32Len[1];
            }
        }
        
    	jpgSuffix[0] = 0xFF;
    	jpgSuffix[1] = 0xD9;
    	jpgPiece.buf[jpgPiece.count] = jpgSuffix;
    	jpgPiece.len[jpgPiece.count] = sizeof( jpgSuffix );
    	jpgPiece.count++;
    	jpgInfo.len	+= sizeof( jpgSuffix );
    	ret = ProconJpgWrite( GetChnProconJpgFd(snapCh), jpgPiece );
    	ReleaseJpgStreamAndStopSnapCh( snapCh, &stStream );
    } // if(  ret == 0
    
	g_snapMutex[snapCh].Unlock();
    
	return ret;
}

/* ===== end 函数重载, 主要是瞒住 procon 设计 ===== */
#endif //#if defined MCU_HI3515