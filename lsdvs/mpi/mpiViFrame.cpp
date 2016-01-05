/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.06.14
**  description  : 获得YUV视频流  并做视频遮挡判断
	理想状态完全被遮挡 U 值会是0x80  
	做一个范围的判断，如果大于最小并且小于最大，则认为被遮挡了
*******************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>

#include "mpiCommon.h"
#include "hi_comm_vi.h"
#include "mpi_vi.h"
#include "const.h"
#include "mympi.h"
#include "paramManage.h"
#include "ttypes.h"
#include "const.h"
#include "debug.h"
#include "thread.h"
#include "mympiAvenc.h"
#include "mpiApp.h"
#include "paramManage.h"
#include "mpiViFrame.h"
#include "malloc.h"
#include "linuxFile.h"
#include "mutex.h"
#include "paramConfDefault.h"

#include "mpi_comm.h"
#include "condition.h"
#include "message.h"



static SHELTER_DETECT_T g_ShelterDetect[REAL_CHANNEL_NUM];
static ShelterThreadStatus_t g_ShelterThreadStatus[REAL_CHANNEL_NUM];
//static ClMutexLock s_videoShelterMutex;


static void VideoShelterStatInit()
{
	int i;

	for ( i=0; i<REAL_CHANNEL_NUM; i++ )
    {
    	g_ShelterDetect[i].shelterStat = 0;
    	g_ShelterDetect[i].shelterCnt = 0;
    	g_ShelterDetect[i].noShelterCnt = 0;
    }
    
}

int VideoGetShelterDetect( int channel )
{
	int ret;
	ret = g_ShelterDetect[channel].shelterStat;
	return ret;
}

/*
* 灵敏度级别定义  
*  0: 需要用整个手掌将摄像头盖住
*  1:
*  2:
*  3:
*  4: 照着天花板即认为被遮挡 (天花板颜色相差不大)
*/
void VideoShelterSensitivityConvert( PARAM_CONFIG_ALARM_VIDEO_SHELTER param, SHELTER_SENSITIVITY_T *sensitivity )
{
	switch (param.sensitivity)
    {
    	case VIDEO_SHELTER_SENSITIVITY_0:
        	sensitivity->rowDotNum = 50;
        	sensitivity->frameNum = 10;
        	sensitivity->variance = 0;
        	break;
    	case VIDEO_SHELTER_SENSITIVITY_1:
        	sensitivity->rowDotNum = 40;
        	sensitivity->frameNum = 10;
        	sensitivity->variance = 1;
        	break;
    	case VIDEO_SHELTER_SENSITIVITY_2:
        	sensitivity->rowDotNum = 30;
        	sensitivity->frameNum = 10;
        	sensitivity->variance = 2;
        	break;
    	case VIDEO_SHELTER_SENSITIVITY_3:
        	sensitivity->rowDotNum = 20;
        	sensitivity->frameNum = 10;
        	sensitivity->variance = 3;
        	break;
    	case VIDEO_SHELTER_SENSITIVITY_4:
        	sensitivity->rowDotNum = 10;
        	sensitivity->frameNum = 10;
        	sensitivity->variance = 4;
        	break;
    	default:
        	sensitivity->rowDotNum = 50;
        	sensitivity->frameNum = 10;
        	sensitivity->variance = 0;
        	break;
        
    }
}

static int VideoShelterDetect( int channel, VIDEO_FRAME_INFO_S stFrame )
{
	int             	i;
	unsigned long     	len = 0;
	unsigned long     	invalidCnt;
	unsigned char         *ptTmp;
	PARAM_CONFIG_ALARM_VIDEO_SHELTER param;
	SHELTER_SENSITIVITY_T sensitivity;
	unsigned int row, column;
	int step, index, row_offset;
	int sum, avg;
    //int ch;

	ParamGetAlarmVideoShelter( channel, &param );
    param.sensitivity = 0;    
	VideoShelterSensitivityConvert( param, &sensitivity );
    
	char *sampleData = (char *)Malloc( (sensitivity.rowDotNum+1)*stFrame.stVFrame.u32Height );
	if (sampleData == NULL) {
    	SVPrint("malloc error\n");
    	return -1;
    }

	invalidCnt = 0;
	len = stFrame.stVFrame.u32Width*stFrame.stVFrame.u32Height/4;
	ptTmp = (unsigned char*)HI_MPI_SYS_Mmap(stFrame.stVFrame.u32PhyAddr[1], len);    


    //采样
	step = stFrame.stVFrame.u32Width/(sensitivity.rowDotNum+1);
	index = 0;
	for ( row=5; row<(stFrame.stVFrame.u32Height/2-5); row++)
    {
    	row_offset = row*stFrame.stVFrame.u32Width/2;
    	for ( column=0; column<sensitivity.rowDotNum; column++ )
        {
        	sampleData[index] = ptTmp[row_offset+column*step];
        	index ++;
        }
    }
	HI_MPI_SYS_Munmap( ptTmp, len );

    //计算方差
	sum = 0;
	for ( i=0; i<index; i++ )
    {
    	sum += sampleData[i];
    }
	avg = sum / index;
	sum = 0;
	for ( i=0; i<index; i++ )
    {
    	sum += (avg - sampleData[i]) * (avg - sampleData[i]);
    }
	avg = sum/index;
    
	Free( sampleData );

	if (channel == 0)
        SVPrint( "--------------------------------2avg :%d-----------------------------------\n", avg );
    
    //s_videoShelterMutex.Lock();
    //ch = channel;
    //没有被遮挡
	if ( avg>sensitivity.variance )
    {
    	g_ShelterDetect[channel].shelterCnt = 0;
    	g_ShelterDetect[channel].noShelterCnt ++;
    	if ( g_ShelterDetect[channel].noShelterCnt > sensitivity.frameNum )
        {
        	g_ShelterDetect[channel].shelterStat = 0;
        	g_ShelterDetect[channel].noShelterCnt = 0;
        	if ( channel == 0 )
            {
                //SVPrint( "++++++++++++++++++++++++++++++++++444+no shelter:%d+++++++++++++++++++++++++++++++++++\n", avg);
            }
        }
    }
	else  //被遮挡 
    {
    	g_ShelterDetect[channel].noShelterCnt = 0;
    	g_ShelterDetect[channel].shelterCnt ++;
    	if ( g_ShelterDetect[channel].shelterCnt > sensitivity.frameNum )
        {
        	g_ShelterDetect[channel].shelterCnt = 0;
        	g_ShelterDetect[channel].noShelterCnt = 0;
        	g_ShelterDetect[channel].shelterStat = 1;
        	if ( channel == 0 )
            {
                //SVPrint( "================================shelter:%d================================\n", avg);
            }
        }        
    }
    //s_videoShelterMutex.Unlock();

	return 0;
}


static void *ViFrameThread( void *args )
{
	HI_S32 s32ret;
	VIDEO_FRAME_INFO_S stFrame;
	int channel;

	sleep( 5 );
	channel = *(int *)args;
	Free( args );

	SVPrint( "%!!!!!!!!!!s start!\r\n", __FUNCTION__ );
	while( g_ShelterThreadStatus[channel].runFlag )
    {
        /* get video frame from vi chn */
        s32ret = HI_MPI_VI_GetFrame(channel, &stFrame);
    	if (0 == s32ret)
        {
        	VideoShelterDetect( channel, stFrame );
            HI_MPI_VI_ReleaseFrame(channel, &stFrame);
        }
    	else 
        {
        	SVPrint("get vi frame err:0x%x\n", s32ret);
        }

    	usleep( 50*1000 );
        //sleep( 1 );
    }    
    
	SVPrint( "%s stop!\r\n", __FUNCTION__ );
	return NULL;
}

void StartViFrameThread()
{
	int ret, i;
	int *pCh;

    ERRORPRINT("CAN NOT USE IN HI3151A!!!!\n");
    return;
	VideoShelterStatInit();
#if 1
	for ( i=0; i<REAL_CHANNEL_NUM; i++ )
    {
    	g_ShelterThreadStatus[i].runFlag = 1;
    	pCh = (int *)Malloc( sizeof(int) );
    	if( NULL != pCh )
        {
            *pCh = i;
        	ret = ThreadCreate( &g_ShelterThreadStatus[i].threadId, ViFrameThread, (void *)pCh );
        	if( 0!= ret )
            {        
            	g_ShelterThreadStatus[i].runFlag = 0;
            	SVPrint( "error:ThreadCreate:%s!\r\n", STRERROR_ERRNO );
            }
        }
    }
#endif	
}

void StopViFrameThread()
{

ERRORPRINT("CAN NOT USE IN HI3151A!!!!\n");
    return;
#if 1
	int ret, i;
	for ( i=0; i<REAL_CHANNEL_NUM; i++ )
    {
    	g_ShelterThreadStatus[i].runFlag = 0;
    	ret = ThreadJoin( g_ShelterThreadStatus[i].threadId, NULL );
    	if( 0 != ret )
        {
        	SVPrint( "error:ThreadJoin:%s\r\n", STRERROR_ERRNO );
        }
    }
#endif	
}

/////////////////////////////////////////////////////////////////////////////////
#define OD

#ifdef OD

static int g_odThreadRunFlag = 0;
static THREAD_MAINTAIN_T g_odTm;
static ClCondition g_OdCondition;


static int CheckOdNeed()
{
    int i,j;
    PARAM_CONFIG_ALARM_VIDEO_SHELTER param;


    for(i = 0;i < REAL_CHANNEL_NUM;i++)
    {
        memset(&param,0,sizeof(param));
        ParamGetAlarmVideoShelter( i, &param );
        
        if(param.armFlag == 1)
        {
            return 1;
        }
        else
        {
            for(j = 0;j < MAX_WEEK_DAY;j++)
            {
                if(param.armTimer.day[i].enableFlag)
                {
                    return 1;
                }
            }
            
        }
    }
    

    return 0;
}

static void OdSendMsgChangeParam()
{
	g_OdCondition.Signal();
	MessageSend( MSG_ID_SHELTER_ALARM_PARAM_CHANGE );
}

static int EnableOD( int channel )
{
	//SetODSensitive( channel );
    HI_S32 s32Ret = HI_SUCCESS;
    VDA_CHN_ATTR_S stVdaChnAttr;
    MPP_CHN_S stSrcChn, stDestChn;
    VDA_CHN VdaChn = channel + REAL_CHANNEL_NUM;
    VI_CHN ViChn   = channel;

    VIDEO_NORM_E enNorm  = VIDEO_ENCODING_MODE_PAL;
    PIC_SIZE_E enPicSize = PIC_D1;
    SIZE_S  stSize;
    
    memset(&stSize,0,sizeof(stSize));
    s32Ret = mpi_comm_sys_get_pic_size(enNorm, enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        ERRORPRINT("mpi_comm_sys_get_pic_size failed!\n");
        return s32Ret;
    }
    
    if (VDA_MAX_WIDTH < stSize.u32Width || VDA_MAX_HEIGHT < stSize.u32Height)
    {
        ERRORPRINT("Picture size invaild!\n");
        return HI_FAILURE;
    }
    
    /* step 1 create vda channel */
    stVdaChnAttr.enWorkMode = VDA_WORK_MODE_OD;
    stVdaChnAttr.u32Width   = stSize.u32Width;
    stVdaChnAttr.u32Height  = stSize.u32Height;

    stVdaChnAttr.unAttr.stOdAttr.enVdaAlg      = VDA_ALG_REF;
    stVdaChnAttr.unAttr.stOdAttr.enMbSize      = VDA_MB_16PIXEL;
    stVdaChnAttr.unAttr.stOdAttr.enMbSadBits   = VDA_MB_SAD_8BIT;
    stVdaChnAttr.unAttr.stOdAttr.enRefMode     = VDA_REF_MODE_DYNAMIC;
    stVdaChnAttr.unAttr.stOdAttr.u32VdaIntvl   = 4;  
    stVdaChnAttr.unAttr.stOdAttr.u32BgUpSrcWgt = 128;
    
    stVdaChnAttr.unAttr.stOdAttr.u32RgnNum = 1;
    
    stVdaChnAttr.unAttr.stOdAttr.astOdRgnAttr[0].stRect.s32X = 0;
    stVdaChnAttr.unAttr.stOdAttr.astOdRgnAttr[0].stRect.s32Y = 0;
    stVdaChnAttr.unAttr.stOdAttr.astOdRgnAttr[0].stRect.u32Width  = stSize.u32Width;
    stVdaChnAttr.unAttr.stOdAttr.astOdRgnAttr[0].stRect.u32Height = stSize.u32Height;

    stVdaChnAttr.unAttr.stOdAttr.astOdRgnAttr[0].u32SadTh      = 200;
    stVdaChnAttr.unAttr.stOdAttr.astOdRgnAttr[0].u32AreaTh     = 70;
    stVdaChnAttr.unAttr.stOdAttr.astOdRgnAttr[0].u32OccCntTh   = 6;
    stVdaChnAttr.unAttr.stOdAttr.astOdRgnAttr[0].u32UnOccCntTh = 2;

    SVPrint("sensitivity is a constant,take care!\n");
    s32Ret = HI_MPI_VDA_CreateChn(VdaChn, &stVdaChnAttr);
    if(s32Ret != HI_SUCCESS)
    {
        ERRORPRINT("err!\n");
        return s32Ret;
    }

    /* step 2: vda chn bind vi chn */
    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32ChnId = ViChn;
    stSrcChn.s32DevId = 0;

    stDestChn.enModId = HI_ID_VDA;
    stDestChn.s32ChnId = VdaChn;
    stDestChn.s32DevId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if(s32Ret != HI_SUCCESS)
    {
        ERRORPRINT("err!\n");
        return s32Ret;
    }

    /* step 3: vda chn start recv picture */
    s32Ret = HI_MPI_VDA_StartRecvPic(VdaChn);
    if(s32Ret != HI_SUCCESS)
    {
        ERRORPRINT("err!\n");
        return s32Ret;
    }
    
	return s32Ret;
}

static int DisableOD( int ch )
{
	HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn, stDestChn;
    VDA_CHN VdaChn = ch + REAL_CHANNEL_NUM;
    VI_CHN  ViChn  = ch;

	
    /* vda stop recv picture */
    s32Ret = HI_MPI_VDA_StopRecvPic(VdaChn);
    if(s32Ret != HI_SUCCESS)
    {
        ERRORPRINT("err(0x%x)!!!!\n", s32Ret);
        return s32Ret;
    }

    /* unbind vda chn & vi chn */
    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32ChnId = ViChn;
    stSrcChn.s32DevId = 0;
    stDestChn.enModId = HI_ID_VDA;
    stDestChn.s32ChnId = VdaChn;
    stDestChn.s32DevId = 0;
    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if(s32Ret != HI_SUCCESS)
    {
        ERRORPRINT("err(0x%x)!!!!\n", s32Ret);
        return s32Ret;
    }

    /* destroy vda chn */
    s32Ret = HI_MPI_VDA_DestroyChn(VdaChn);
    if(s32Ret != HI_SUCCESS)
    {
        ERRORPRINT("err(0x%x)!!!!\n",s32Ret);
    }
    
    return s32Ret;
}

static void *OdThread( void *arg )
{
	int ret = 0;    
	unsigned int i;
	int odFd[8]={0};
	int maxfd = 0;
	unsigned int chNum = REAL_CHANNEL_NUM;
	fd_set readFds;
	struct timeval tv;
    VDA_CHN VdaChn;
    VDA_DATA_S stVdaData;

    CORRECTPRINT( "!!!!!!!!!%s start!\r\n", __FUNCTION__ );
	while( g_odThreadRunFlag )
    {
    	if( 0 == CheckOdNeed() )     // 如果全部通道都没有打开移动侦测
        {   
            SVPrint("MD close all\n");
        	g_OdCondition.Wait();     // 则进入休眠
        	MessageRecv( MSG_ID_SHELTER_ALARM_PARAM_CHANGE );
        	continue;
        }
        
        SVPrint("DO not care about areas,take care!\n");
        VideoShelterStatInit();
        
    	for( i = 0; i < chNum; i++ )
        {
        	ret = EnableOD( i );
            
        	if(ret < 0)
            {
            	ERRORPRINT( "Enable channel(%d) MD err!\n", i );
            	return NULL;
            }
            
        	VdaChn = i + REAL_CHANNEL_NUM;
        	odFd[i] = HI_MPI_VDA_GetFd(VdaChn);
            
        	if( odFd[i] <= 0 )
            {
            	ERRORPRINT("HI_MPI_VDA_GetFd err\n");
            	return NULL;
            }
            
        	if( maxfd <= odFd[i] ) 
            {
            	maxfd = odFd[i];
            }
        }
        
    	CORRECTPRINT( "loop of %s start!\r\n", __FUNCTION__ );
    	while( g_odTm.runFlag )
        {
        	if( MessageRecv( MSG_ID_SHELTER_ALARM_PARAM_CHANGE ) >= 0 )
            {
                SVPrint("rcv MSG_ID_SHELTER_ALARM_PARAM_CHANGE!!!\n");
            	break; 
            }
            
        	FD_ZERO( &readFds );
        	for( i = 0; i < chNum; i++ )
            {
            	FD_SET( odFd[i], &readFds );
            }
            
        	tv.tv_sec  = 1;
        	tv.tv_usec = 0;
        	ret = select( maxfd+1, &readFds, NULL, NULL, &tv );
        	if( ret < 0 )
            {
            	ERRORPRINT( "error: select()!\r\n" );
            	break;
            }
        	else if( 0 == ret )
            {
            	ERRORPRINT("failed: MD select time out!\n");
            	break;
            }
        	else
            {
            	for( i = 0; i < chNum; i++ )
                {
                	VdaChn = i + REAL_CHANNEL_NUM;
                    /*get md data*/
                	memset( &stVdaData, 0, sizeof(stVdaData) );
                	if( FD_ISSET(odFd[i], &readFds) )
                    {    
                    	ret = HI_MPI_VDA_GetData( VdaChn, &stVdaData, HI_IO_NOBLOCK );
                    	if( ret != HI_SUCCESS )
                        {
                        	ERRORPRINT( "HI_MPI_VDA_GetData Err 0x%x\n", ret );
                        	continue;
                        }
                        /*process md data*/
                        g_ShelterDetect[i].shelterStat = stVdaData.unData.stOdData.abRgnAlarm[0];
                        if(g_ShelterDetect[i].shelterStat)
                        {
                            SVPrint("channel(%d) OD happened!\n",i);
                            ret = HI_MPI_VDA_ResetOdRegion(VdaChn,0);
                            if(ret != HI_SUCCESS)
                            {
            		            SAMPLE_PRT("HI_MPI_VDA_ResetOdRegion failed with %#x!\n", ret);
                                continue;
                            }
                        }
                        
                        /*release md data*/
                    	ret = HI_MPI_VDA_ReleaseData( VdaChn, &stVdaData );
                    	if( ret != 0 )
                        {
                        	SVPrint( "Md Release Data Err 0x%x\n", ret );
                        	continue;
                        }
                    }
                } 
            }  

            usleep(20);            
        } 	    
    	for( i = 0; i < chNum; i++ )
        {
        	DisableOD( i );
        }
    } 
    
	ERRORPRINT( "!!!!!!!!!!!!!!!!!!!!!!%s stop!\r\n", __FUNCTION__ );
    
	return 0;
}


void StartOdThread()
{
	int ret;
	g_odThreadRunFlag	= 1;
	g_odTm.runFlag      = 1;
	ret = ThreadCreate( &g_odTm.id, OdThread, NULL );
	if( 0!= ret )
    {        
    	g_odTm.runFlag = 0;
    	ERRORPRINT( "error:ThreadCreate:%s\r\n", STRERROR_ERRNO );
    }
}

void StopOdThread()
{
	int ret;
	g_odThreadRunFlag     = 0;
	g_odTm.runFlag        = 0;
	OdSendMsgChangeParam();
	ret = ThreadJoin( g_odTm.id, NULL );
	if( 0 != ret )
    {
    	SVPrint( "error:ThreadJoin:%s\r\n", STRERROR_ERRNO );
    }
}
#endif