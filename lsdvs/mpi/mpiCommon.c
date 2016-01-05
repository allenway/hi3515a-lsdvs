/******************************************************************************

  Copyright (C), 2014, RHA

******************************************************************************
  File Name    : mpicommon.c
  Version       : 
  Author        : sven
  Created      : 2014/10/04
  Description : 
  History       :
******************************************************************************/
#ifdef __cplusplus
 #if __cplusplus
extern "C"{
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>

#include "const.h"
#include "debug.h"
#include "mpiCommon.h"
#include "paramManage.h"
#include "mpiApp.h"

#include "mpi_comm.h"

#define TW2865_FILE   "/dev/tw2865dev"

#define TW2864_A_FILE "/dev/tw2864adev"
#define TW2864_B_FILE "/dev/tw2864bdev"
#define TW2864_C_FILE "/dev/tw2864cdev"
#define TW2864_D_FILE "/dev/tw2864ddev"
#define ADV7441_DEV_FILE "/dev/adv7441"

/************!!!!attention:********
The test code havn't check whether vi dev is out of  range(0-3)
*********************************/
#define G_VIDEV_START 0

/* video input into VI.
** Only valid when input from ADC, such as TW2864, TW2815, etc.
** VI samples that run for PAL will change to NTSC if modify it
** to VIDEO_ENCODING_MODE_NTSC.
*/
#if 1
    VIDEO_NORM_E   gs_enViNorm   = VIDEO_ENCODING_MODE_PAL;
    VO_INTF_SYNC_E gs_enSDTvMode = VO_OUTPUT_PAL;
#else
    VIDEO_NORM_E   gs_enViNorm   = VIDEO_ENCODING_MODE_NTSC;
    VO_INTF_SYNC_E gs_enSDTvMode = VO_OUTPUT_NTSC;
#endif

void enableOut(void)
{
	int fd,ret;
	fd = open("/dev/hi_gpio", O_RDWR);
	if(fd < 0)
    {
    	printf("open gpio device error!\n");
    	return ;
    }
	ret = ioctl(fd, 0x05, NULL);
	if(ret < 0)
    {
    	printf("gpio ctrl  error !\n");
    }
	close(fd);
}
/************************************************************************************/
const static HI_U8 g_SOI[2] = {0xFF, 0xD8};
const static HI_U8 g_EOI[2] = {0xFF, 0xD9};

#if defined MCU_HI3515
/*****************************************************************************
 Prototype       : SAMPLE_TW2865_CfgV
 Description     : config video of tw2865
 Input           : enVideoMode  **
                   enWorkMode   **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
 Remark          : in default, TW2865 works in 4D1 mode.
*****************************************************************************/
HI_S32 SAMPLE_TW2865_CfgV(VIDEO_NORM_E enVideoMode,VI_WORK_MODE_E enWorkMode)
{
    int fd, i;
    int video_mode;
    tw2865_video_norm stVideoMode;
    tw2865_work_mode work_mode;
#ifdef hi3515
    int chip_cnt = 2;
#else
    int chip_cnt = 4;
#endif

    fd = open(TW2865_FILE, O_RDWR);
    if (fd < 0)
    {
        printf("open %s fail\n", TW2865_FILE);
        return -1;
    }

    video_mode = (VIDEO_ENCODING_MODE_PAL == enVideoMode) ? TW2865_PAL : TW2865_NTSC ;

    for (i=0; i<chip_cnt; i++)
    {
        stVideoMode.chip    = i;
        stVideoMode.mode    = video_mode;
        if (ioctl(fd, TW2865_SET_VIDEO_NORM, &stVideoMode))
        {
            printf("set tw2865(%d) video mode fail\n", i);
            close(fd);
            return -1;
        }
    }

    for (i=0; i<chip_cnt; i++)
    {
        work_mode.chip = i;
        if (VI_WORK_MODE_4D1 == enWorkMode)
        {
            work_mode.mode = TW2865_4D1_MODE;
        }
        else if (VI_WORK_MODE_2D1 == enWorkMode)
        {
            work_mode.mode = TW2865_2D1_MODE;
        }
        else
        {
            printf("work mode not support\n");
            return -1;
        }
        ioctl(fd, TW2865_SET_WORK_MODE, &work_mode);
    }

    close(fd);
    return 0;
}

typedef struct 
{
    int start;
    pthread_t Pid;   
    int chn_cnt;
} SAMPLE_VIDEO_LOSS;

static SAMPLE_VIDEO_LOSS s_stVideoLoss;

void *SampleVLossDetProc(void *parg)
{  
    int fd, i;
    VI_DEV ViDev;
    VI_CHN ViChn;
    tw2865_video_loss video_loss;
    SAMPLE_VIDEO_LOSS *ctl = (SAMPLE_VIDEO_LOSS*)parg;
    
    fd = open(TW2865_FILE, O_RDWR);
    if (fd < 0)
    {
        printf("open %s fail\n", TW2865_FILE);
        ctl->start = 0;
        return NULL;
    }
    
    while (ctl->start)
    {
        for (i = 0; i < ctl->chn_cnt; i++)
        {
            video_loss.chip = i/4;
            video_loss.ch   = i%4;
            ioctl(fd, TW2865_GET_VIDEO_LOSS, &video_loss);

#ifdef hi3515
            ViDev = (i/4)*2;
#else
            ViDev = i/4;
#endif
            ViChn = i%4;
            if (video_loss.is_lost)
            {
                HI_MPI_VI_EnableUserPic(ViDev, ViChn);
            }
            else
            {
                HI_MPI_VI_DisableUserPic(ViDev, ViChn);
            }                
        }
        usleep(500000);
    }
    
    close(fd);
    ctl->start = 0;
    return NULL;
}

void SAMPLE_StartVLossDet(int chn_cnt)
{              
    s_stVideoLoss.start = 1;
    s_stVideoLoss.chn_cnt = chn_cnt;
    pthread_create(&s_stVideoLoss.Pid, 0, SampleVLossDetProc, &s_stVideoLoss);
}

void SAMPLE_StopVLossDet()
{
    if (s_stVideoLoss.start)
    {
        s_stVideoLoss.start = 0;
        pthread_join(s_stVideoLoss.start, 0);
    }
}

/*****************************************************************************
 Prototype       : SAMPLE_InitMpp
 Description     : Init Mpp
 Input           : pstVbConf  **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2009/7/4
    Author       : c55300
    Modification : Created function

*****************************************************************************/
HI_S32 SAMPLE_InitMPP(VB_CONF_S *pstVbConf)
{
    MPP_SYS_CONF_S stSysConf = {0};

    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit();

    if (HI_MPI_VB_SetConf(pstVbConf))
    {
        printf("HI_MPI_VB_SetConf failed!\n");
        return -1;
    }

    if (HI_MPI_VB_Init())
    {
        printf("HI_MPI_VB_Init failed!\n");
        return -1;
    }

    stSysConf.u32AlignWidth = 16;
    if (HI_MPI_SYS_SetConf(&stSysConf))
    {
        printf("conf : system config failed!\n");
        return -1;
    }

    if (HI_MPI_SYS_Init())
    {
        printf("sys init failed!\n");
        return -1;
    }

    return 0;
}

/*****************************************************************************
 Prototype       : SAMPLE_ExitMpp
 Description     : Exit Mpp
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
*****************************************************************************/
HI_S32 SAMPLE_ExitMPP(HI_VOID)
{
    if (HI_MPI_SYS_Exit())
    {
        printf("sys quit fail\n");
        return -1;
    }

    if (HI_MPI_VB_Exit())
    {
        printf("vb quit fail\n");
        return -1;
    }

    return 0;
}

/*****************************************************************************
 Prototype       : SampleVichnPerDev
 Description     : get number of channel of VI device with difference mode.
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
*****************************************************************************/
static HI_S32 SampleVichnPerDev(VI_INPUT_MODE_E enViInputMode, VI_WORK_MODE_E enViWorkMode)
{
    if ((VI_MODE_BT656 == enViInputMode) && (VI_WORK_MODE_1D1 == enViWorkMode))
    {
        return 1;
    }
    else if ((VI_MODE_BT656 == enViInputMode) && (VI_WORK_MODE_2D1 == enViWorkMode))
    {
        return 2;
    }
    else if ((VI_MODE_BT656 == enViInputMode) && (VI_WORK_MODE_4D1 == enViWorkMode))
    {
        return 4;
    }
    else if ((VI_MODE_BT656 == enViInputMode) && (VI_WORK_MODE_4HALFD1 == enViWorkMode))
    {
        return 4;
    }
    else if ((VI_MODE_BT601 == enViInputMode) || (VI_MODE_DIGITAL_CAMERA == enViInputMode)
             || (VI_MODE_BT1120_PROGRESSIVE == enViInputMode) || (VI_MODE_BT1120_INTERLACED == enViInputMode))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*****************************************************************************
 Prototype       : SAMPLE_GetViChnPerDev
 Description     : with different mode, vi device can enter different number of
                   channel. call this function to get.
 Input           : ViDev  **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  Remark         : vi device attribute must be set first.
*****************************************************************************/
HI_S32 SAMPLE_GetViChnPerDev(VI_DEV ViDev)
{
    VI_PUB_ATTR_S stPubAttr;
    HI_S32 s32ViChnPerDev;
    HI_S32 s32Ret;

    s32Ret = HI_MPI_VI_GetPubAttr(ViDev, &stPubAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("Get vi(%d) channel per dev fail\n", ViDev);
        return 0;
    }

    s32ViChnPerDev = SampleVichnPerDev(stPubAttr.enInputMode, stPubAttr.enWorkMode);
    return s32ViChnPerDev;
}

HI_S32 SAMPLE_PicSize2WH(PIC_SIZE_E enPicSize, HI_S32* s32Width, HI_S32* s32Height)
{
    switch ( enPicSize )
    {
        case PIC_QCIF:
            *s32Width = 176;
            *s32Height = (VIDEO_ENCODING_MODE_PAL == gs_enViNorm) ? 144 : 120;
            break;
        case PIC_CIF:
            *s32Width = 352;
            *s32Height = (VIDEO_ENCODING_MODE_PAL == gs_enViNorm) ? 288 : 240;
            break;
        case PIC_2CIF:
            *s32Width = 352;
            *s32Height = (VIDEO_ENCODING_MODE_PAL == gs_enViNorm) ? 576 : 480;
            break;
        case PIC_HD1:
            *s32Width = 704;
            *s32Height = (VIDEO_ENCODING_MODE_PAL == gs_enViNorm) ? 288 : 240;
            break;
        case PIC_D1:
            *s32Width = 704;
            *s32Height = (VIDEO_ENCODING_MODE_PAL == gs_enViNorm) ? 576 : 480;
            break;
        case PIC_QVGA:
            *s32Width = 320;
            *s32Height = 240;
            break;
        case PIC_VGA:
            *s32Width = 640;
            *s32Height = 480;
            break;
        case PIC_XGA:
            *s32Width = 1024;
            *s32Height = 768;
            break;
        case PIC_SXGA:
            *s32Width = 1400;
            *s32Height = 1050;
            break;
        case PIC_UXGA:
            *s32Width = 1600;
            *s32Height = 1200;
            break;
        case PIC_WSXGA:
            *s32Width = 1680;
            *s32Height = 1050;
            break;
        default:
            printf("err pic size\n");
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}

/*****************************************************************************
 Prototype       : SampleParseVoOutput
 Description     : parse vo output, to decide dispaly widht,height and rate.
 Input           : enVoOutput     **
 Output          : s32Width       **
                   s32Height      **
                   s32Rate        **
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2009/7/27
    Author       : c55300
    Modification : Created function

*****************************************************************************/
HI_S32 SampleParseVoOutput(VO_INTF_SYNC_E enVoOutput,
    HI_S32* s32Width, HI_S32* s32Height, HI_S32* s32DisplayRate)
{
    switch ( enVoOutput )
    {
        case VO_OUTPUT_PAL:
            *s32Width = 720;
            *s32Height = 576;
            *s32DisplayRate = 25;
            break;
        case VO_OUTPUT_NTSC:
            *s32Width = 720;
            *s32Height = 480;
            *s32DisplayRate = 30;
            break;
        case VO_OUTPUT_720P60:
            *s32Width = 1280;
            *s32Height = 720;
            *s32DisplayRate = 60;
            break;
        case VO_OUTPUT_1080I60:
            *s32Width = 1920;
            *s32Height = 1080;
            *s32DisplayRate = 60;
            break;
        case VO_OUTPUT_1080P30:
            *s32Width = 1920;
            *s32Height = 1080;
            *s32DisplayRate = 30;
            break;
        case VO_OUTPUT_800x600_60:
            *s32Width = 800;
            *s32Height = 600;
            *s32DisplayRate = 60;
            break;
        case VO_OUTPUT_1024x768_60:
            *s32Width = 1024;
            *s32Height = 768;
            *s32DisplayRate = 60;
            break;
        case VO_OUTPUT_1280x1024_60:
            *s32Width = 1280;
            *s32Height = 1024;
            *s32DisplayRate = 60;
            break;
        case VO_OUTPUT_1366x768_60:
            *s32Width = 1366;
            *s32Height = 768;
            *s32DisplayRate = 60;
            break;
        case VO_OUTPUT_1440x900_60:
            *s32Width = 1440;
            *s32Height = 900;
            *s32DisplayRate = 60;
            break;
        case VO_OUTPUT_USER:
            printf("Why call me? You should set display size in VO_OUTPUT_USER.\n");
            break;
        default:
            *s32Width = 720;
            *s32Height = 576;
            *s32DisplayRate = 25;
            printf(" vo display size is (720, 576).\n");
            break;
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_StartViDev(VI_INPUT_MODE_E enInputMode)
{
    HI_S32 s32Ret;
    VI_DEV ViDev;
    VI_PUB_ATTR_S stViDevAttr;

    stViDevAttr.enInputMode = enInputMode;
    if (VI_MODE_BT656 == stViDevAttr.enInputMode)
    {
        stViDevAttr.enWorkMode = VI_WORK_MODE_4D1;
    }
    else
    {
        printf("%s now not support this input mode\n", __FUNCTION__);
        return HI_FAILURE;
    }

    SAMPLE_TW2865_CfgV(gs_enViNorm, VI_WORK_MODE_4D1);

    for (ViDev=0; ViDev < VIU_MAX_DEV_NUM; ViDev++)
    {
        s32Ret = HI_MPI_VI_SetPubAttr(ViDev, &stViDevAttr);
        if (s32Ret)
        {
            printf("set vi dev %d attr fail,%x\n", ViDev, s32Ret);
            return HI_FAILURE;
        }

        s32Ret = HI_MPI_VI_Enable(ViDev);
        if (s32Ret)
        {
            printf("VI dev %d enable fail \n", ViDev);
            return s32Ret;
        }
    }
    return HI_SUCCESS;
}

HI_S32 SAMPLE_StartViChn(VI_DEV ViDev, VI_CHN ViChn, PIC_SIZE_E enPicSize)
{
    HI_S32 s32Ret;
    VI_CHN_ATTR_S stViChnAttr;

    stViChnAttr.stCapRect.u32Width  = 704;
    stViChnAttr.stCapRect.u32Height =
        (VIDEO_ENCODING_MODE_PAL == gs_enViNorm) ? 288 : 240;
    stViChnAttr.stCapRect.s32X = 8;
    stViChnAttr.stCapRect.s32Y = 0;
    stViChnAttr.enViPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stViChnAttr.bHighPri = HI_FALSE;
    stViChnAttr.bChromaResample = HI_FALSE;

    /* different pic size has different capture method for BT656. */
    stViChnAttr.enCapSel = (PIC_D1 == enPicSize || PIC_2CIF == enPicSize) ? \
                             VI_CAPSEL_BOTH : VI_CAPSEL_BOTTOM;
    stViChnAttr.bDownScale = (PIC_D1 == enPicSize || PIC_HD1 == enPicSize) ? \
                               HI_FALSE : HI_TRUE;

    s32Ret = HI_MPI_VI_SetChnAttr(ViDev, ViChn, &stViChnAttr);
    if (s32Ret)
    {
        printf("set vi(%d,%d) attr fail,%x\n", ViDev, ViChn, s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_VI_EnableChn(ViDev, ViChn);
    if (s32Ret)
    {
        printf("set vi(%d,%d) attr fail,%x\n", ViDev, ViChn, s32Ret);
        return s32Ret;
    }
    return HI_SUCCESS;
}

HI_S32 SAMPLE_StopViChn(VI_DEV ViDev, VI_CHN ViChn)
{
    HI_S32 s32Ret;
    s32Ret = HI_MPI_VI_DisableChn(ViDev, ViChn);
    if (s32Ret)
    {
        printf("disable (%d,%d) fail,%x\n", ViDev, ViChn, s32Ret);
        return s32Ret;
    }
    return HI_SUCCESS;
}

/*****************************************************************************
 Prototype       : SAMPLE_StartViByChn
 Description     : start vi. Just input how many channels you want to start.
                   It decide how many devices will be start. vi devices
                   begin from 0.
 Input           : s32ChnTotal   ** how many vi channels you want to start.
                   pstViDevAttr  ** attribute of vi device
                   pstViChnAttr  ** attribute of vi channel
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
 Remark          : Used together with SAMPLE_StopViByChn().
                   All vi devices/channles must have the same attribute.
*****************************************************************************/
HI_S32 SAMPLE_StartViByChn(HI_S32 s32ChnTotal, VI_PUB_ATTR_S* pstViDevAttr, VI_CHN_ATTR_S* pstViChnAttr)
{
    HI_S32 i;
    VI_DEV ViDev,vi_start;
    HI_S32 s32ChnPerDev;
    HI_S32 s32DevTotal;
    HI_S32 ViChn;
    HI_S32 s32Ret;
    HI_S32 s32ChnCnt;
    HI_U32 u32SrcFrmRate;

    s32ChnPerDev = SampleVichnPerDev(pstViDevAttr->enInputMode, pstViDevAttr->enWorkMode);
    s32DevTotal = (s32ChnTotal - 1) / s32ChnPerDev + 1;
    u32SrcFrmRate = (VIDEO_ENCODING_MODE_PAL == gs_enViNorm) ? 25 : 30;

    s32ChnCnt = 0;
    vi_start = G_VIDEV_START;
    for (i = 0; i < s32DevTotal; i++)
    {
#ifdef hi3515
        ViDev = 2 * i;/* hi3515 fpga板 设备0和设备2使用4D1模式 设备1和设备3无效 */
#else
        ViDev = i;
#endif
        s32Ret = HI_MPI_VI_SetPubAttr(ViDev, pstViDevAttr);
        if (s32Ret)
        {
            printf("set vi dev %d attr fail,%x\n", ViDev, s32Ret);
            return HI_FAILURE;
        }

        s32Ret = HI_MPI_VI_Enable(ViDev);
        if (s32Ret)
        {
            printf("VI dev %d enable fail \n", ViDev);
            return s32Ret;
        }

        printf("enable vi dev %d ok\n", ViDev);

        for (ViChn = 0; ViChn < s32ChnPerDev; ViChn++)
        {
            s32Ret = HI_MPI_VI_SetChnAttr(ViDev, ViChn, pstViChnAttr);
            if (s32Ret)
            {
                printf("set vi(%d,%d) attr fail,%x\n", ViDev, ViChn, s32Ret);
                return s32Ret;
            }

            s32Ret = HI_MPI_VI_EnableChn(ViDev, ViChn);
            if (s32Ret)
            {
                printf("set vi(%d,%d) attr fail,%x\n", ViDev, ViChn, s32Ret);
                return s32Ret;
            }

            s32Ret = HI_MPI_VI_SetSrcFrameRate(ViDev, ViChn, u32SrcFrmRate);
            if (s32Ret)
            {
                return s32Ret;
            }

            printf("enable vi(%d,%d) ok\n", ViDev, ViChn);

            /* return when all channel ok. */
            if(++s32ChnCnt >= s32ChnTotal)
            {
                return HI_SUCCESS;
            }
        }
    }
    return HI_SUCCESS;
}

HI_S32 SAMPLE_StartViByChn_ls(HI_S32 s32ChnTotal, VI_PUB_ATTR_S* pstViDevAttr, VI_CHN_ATTR_S* pstViChnAttr)
{
    HI_S32 i;
    VI_DEV ViDev,vi_start;
    HI_S32 s32ChnPerDev;
    HI_S32 s32DevTotal;
    HI_S32 ViChn;
    HI_S32 s32Ret;
    HI_S32 s32ChnCnt;
    HI_U32 u32SrcFrmRate;

    s32ChnPerDev = SampleVichnPerDev(pstViDevAttr->enInputMode, pstViDevAttr->enWorkMode);
    s32DevTotal = (s32ChnTotal - 1) / s32ChnPerDev + 1;
    u32SrcFrmRate = (VIDEO_ENCODING_MODE_PAL == gs_enViNorm) ? 25 : 30;

    s32ChnCnt = 0;
    vi_start = G_VIDEV_START;
    for (i = 0; i < s32DevTotal; i++)
    {
#ifdef hi3515
        ViDev = 2 * i;/* hi3515 fpga板 设备0和设备2使用4D1模式 设备1和设备3无效 */
#else
        ViDev = i;
#endif
        s32Ret = HI_MPI_VI_SetPubAttr(ViDev, pstViDevAttr);
        if (s32Ret)
        {
            printf("set vi dev %d attr fail,%x\n", ViDev, s32Ret);
            return HI_FAILURE;
        }

        s32Ret = HI_MPI_VI_Enable(ViDev);
        if (s32Ret)
        {
            printf("VI dev %d enable fail \n", ViDev);
            return s32Ret;
        }

        printf("enable vi dev %d ok\n", ViDev);

        for (ViChn = 0; ViChn < s32ChnPerDev; ViChn++)
        {
            s32Ret = HI_MPI_VI_SetChnAttr(ViDev, ViChn, &pstViChnAttr[ViChn]);
            if (s32Ret)
            {
                printf("set vi(%d,%d) attr fail,%x\n", ViDev, ViChn, s32Ret);
                return s32Ret;
            }

            s32Ret = HI_MPI_VI_EnableChn(ViDev, ViChn);
            if (s32Ret)
            {
                printf("set vi(%d,%d) attr fail,%x\n", ViDev, ViChn, s32Ret);
                return s32Ret;
            }

            s32Ret = HI_MPI_VI_SetSrcFrameRate(ViDev, ViChn, u32SrcFrmRate);
            if (s32Ret)
            {
                return s32Ret;
            }

            printf("enable vi(%d,%d) ok\n", ViDev, ViChn);

            /* return when all channel ok. */
            if(++s32ChnCnt >= s32ChnTotal)
            {
                return HI_SUCCESS;
            }
        }
    }
    return HI_SUCCESS;
}


/*****************************************************************************
 Prototype       : SAMPLE_StopViByChn
 Description     : stop vi. Just input how many channels you want to stop.
                   It decide how many devices will be stop. vi devices
                   begin from 0.
 Input           : s32ChnTotal  ** how many vi channels you want to stop.
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
 Remark          : Used together with SAMPLE_StartViByChn().
                   All vi devices/channles must have the same attribute.
*****************************************************************************/
HI_S32 SAMPLE_StopViByChn(HI_S32 s32ChnTotal)
{
    VI_DEV ViDev,vi_start;
    HI_S32 s32ChnPerDev;
    HI_S32 s32DevTotal;
    HI_S32 ViChn;
    HI_S32 s32Ret;
    HI_S32 s32ChnCnt;

    s32ChnPerDev = SAMPLE_GetViChnPerDev(0);
    s32DevTotal = (s32ChnTotal - 1) / s32ChnPerDev + 1;

    s32ChnCnt = 0;
    vi_start = G_VIDEV_START;
    for (ViDev = vi_start; ViDev <vi_start+ s32DevTotal; ViDev++)
    {
        for (ViChn = 0; ViChn < s32ChnPerDev; ViChn++)
        {
            s32Ret = HI_MPI_VI_DisableChn(ViDev, ViChn);
            if (HI_SUCCESS != s32Ret)
            {
                printf("HI_MPI_VI_DisableChn(%d,%d) fail, err code: 0x%08x\n",
                    ViDev, ViChn, s32Ret);
            }

            /* return when all channel ok. */
    	s32ChnCnt++;
        }

        HI_MPI_VI_Disable(ViDev);
        if(s32ChnCnt >= s32ChnTotal)
        {
            return HI_SUCCESS;
        }

    }

    return HI_SUCCESS;
}

/*****************************************************************************
 Prototype       : SAMPLE_StartViByDev
 Description     : start vi device and its channel.
 Input           : s32ChnTotal   ** how many channel start on this device
                   ViDev         ** which vi device you want to start
                   pstViDevAttr  ** attribute of this device
                   pstViChnAttr  ** attribute of all channels on this device
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
 Remark          : Used together with SAMPLE_StopViByDev().
*****************************************************************************/
HI_S32 SAMPLE_StartViByDev(HI_S32 s32ChnTotal, VI_DEV ViDev, VI_PUB_ATTR_S* pstViDevAttr, VI_CHN_ATTR_S* pstViChnAttr)
{
    HI_S32 ViChn;
    HI_S32 s32Ret;

    s32Ret = HI_MPI_VI_SetPubAttr(ViDev, pstViDevAttr);
    if (s32Ret)
    {
        printf("set vi dev %d attr fail,%x\n", ViDev, s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VI_Enable(ViDev);
    if (s32Ret)
    {
        printf("VI dev %d enable fail \n", ViDev);
        return s32Ret;
    }

    printf("enable vi dev %d ok\n", ViDev);

    for (ViChn = 0; (ViChn < s32ChnTotal); ViChn++)
    {
        s32Ret = HI_MPI_VI_SetChnAttr(ViDev, ViChn, pstViChnAttr);
        if (s32Ret)
        {
            printf("set vi(%d,%d) attr fail,%x\n", ViDev, ViChn, s32Ret);
            return s32Ret;
        }

        s32Ret = HI_MPI_VI_EnableChn(ViDev, ViChn);
        if (s32Ret)
        {
            printf("set vi(%d,%d) attr fail,%x\n", ViDev, ViChn, s32Ret);
            return s32Ret;
        }

        printf("enable vi(%d,%d) ok\n", ViDev, ViChn);
    }

    return HI_SUCCESS;
}

/*****************************************************************************
 Prototype       : SAMPLE_StopViByDev
 Description     : Stop vi device and all channls on this device.
 Input           : ViDev        **
                   s32ChnTotal  **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
 Remark          : Used together with SAMPLE_StartViByDev().
*****************************************************************************/
HI_S32 SAMPLE_StopViByDev(HI_S32 s32ChnTotal, VI_DEV ViDev)
{
    HI_S32 ViChn;
    HI_S32 s32Ret;

    for (ViChn = 0; ViChn < s32ChnTotal; ViChn++)
    {
        s32Ret = HI_MPI_VI_DisableChn(ViDev, ViChn);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_VI_DisableChn(%d, %d) fail, err no: 0x%08x.\n",
                ViDev, ViChn, s32Ret);
            return HI_FAILURE;
        }

    }

    s32Ret = HI_MPI_VI_Disable(ViDev);
    if ( HI_SUCCESS != s32Ret )
    {
        printf("HI_MPI_VI_Disable(%d) fail, err no: 0x%08x.\n",
            ViDev, s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/*****************************************************************************
 Prototype       : SAMPLE_StartVoDevice
 Description     : Start vo device as you specified.
 Input           : VoDev       **
                   pstDevAttr  **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
*****************************************************************************/
HI_S32 SAMPLE_StartVoDevice(VO_DEV VoDev, VO_PUB_ATTR_S* pstDevAttr)
{
    HI_S32 ret;

    /*because we will change vo device attribution,
      so we diable vo device first*/
	ret = HI_MPI_VO_Disable(VoDev);
    if (HI_SUCCESS != ret)
    {
        printf("HI_MPI_VO_Disable fail 0x%08x.\n", ret);
        return HI_FAILURE;
    }

    ret = HI_MPI_VO_SetPubAttr(VoDev, pstDevAttr);
    if (HI_SUCCESS != ret)
    {
        printf("HI_MPI_VO_SetPubAttr fail 0x%08x.\n", ret);
        return HI_FAILURE;
    }

    ret = HI_MPI_VO_Enable(VoDev);
    if (HI_SUCCESS != ret)
    {
        printf("HI_MPI_VO_Enable fail 0x%08x.\n", ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_StartVoVideoLayer(VO_DEV VoDev, VO_VIDEO_LAYER_ATTR_S* pstVideoLayerAttr)
{
    HI_S32 ret;

    /* set public attr of VO*/
    ret = HI_MPI_VO_SetVideoLayerAttr(VoDev, pstVideoLayerAttr);
    if (HI_SUCCESS != ret)
    {
        printf("set video layer of dev %u failed %#x!\n", VoDev, ret);
        return HI_FAILURE;
    }

    /* enable VO device*/
    ret = HI_MPI_VO_EnableVideoLayer(VoDev);
    if (HI_SUCCESS != ret)
    {
        printf("enable video layer of dev %d failed with %#x !\n", VoDev, ret);
        return HI_FAILURE;
    }
	enableOut();

    return HI_SUCCESS;
}

HI_S32 SAMPLE_SetVoChnMScreen(VO_DEV VoDev, HI_U32 u32ChnCnt, HI_U32 u32Width, HI_U32 u32Height)
{
    HI_U32 i, div, w, h, ret;
    VO_CHN_ATTR_S stChnAttr;

    /* If display 32 vo channels, should use 36-screen split. */
    u32ChnCnt = (u32ChnCnt == VO_MAX_CHN_NUM) ? 36 : u32ChnCnt;

    div = sqrt(u32ChnCnt);
    w = (u32Width / div);
    h = (u32Height / div);

    for (i = 0; i < u32ChnCnt; i++)
    {
        if (i >= VO_MAX_CHN_NUM)
        {
            break;
        }

        stChnAttr.u32Priority = 0;
        stChnAttr.bZoomEnable = HI_TRUE;
        stChnAttr.bDeflicker  = HI_FALSE;
        stChnAttr.stRect.s32X = w * (i % div);
        stChnAttr.stRect.s32Y = h * (i / div);
        stChnAttr.stRect.u32Width  = w;
        stChnAttr.stRect.u32Height = h;

        if (stChnAttr.stRect.s32X % 2 != 0)
        {
            stChnAttr.stRect.s32X++;
        }

        if (stChnAttr.stRect.s32Y % 2 != 0)
        {
            stChnAttr.stRect.s32Y++;
        }

        if (stChnAttr.stRect.u32Width % 2 != 0)
        {
            stChnAttr.stRect.u32Width++;
        }

        if (stChnAttr.stRect.u32Height % 2 != 0)
        {
            stChnAttr.stRect.u32Height++;
        }

        ret = HI_MPI_VO_SetChnAttr(VoDev, i, &stChnAttr);
        if (ret != HI_SUCCESS)
        {
            printf("In %s set channel %d attr failed with %#x!\n", __FUNCTION__, i, ret);
            return ret;
        }
    }

    return HI_SUCCESS;
}

/*****************************************************************************
 Prototype       : SAMPLE_StartVo
 Description     : Start VO to output CVBS signal
 Input           : VoDev       **  SD, AD or HD
                   s32ChnTotal **  how many channel display on screen.
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
*****************************************************************************/
HI_S32 SAMPLE_StartVo(HI_S32                 s32ChnTotal,
                            VO_DEV                 VoDev,
                            VO_PUB_ATTR_S*         pstVoDevAttr,
                            VO_VIDEO_LAYER_ATTR_S* pstVideoLayerAttr)
{
    HI_S32 s32Ret;
    HI_U32 u32Width;
    HI_U32 u32Height;
    VO_CHN VoChn;

    s32Ret = SAMPLE_StartVoDevice(VoDev, pstVoDevAttr);
    if (HI_SUCCESS != s32Ret)
    {
        return HI_FAILURE;
    }

    /* set vo video layer attribute and start video layer. */
    s32Ret = SAMPLE_StartVoVideoLayer(VoDev, pstVideoLayerAttr);
    if (HI_SUCCESS != s32Ret)
    {
        return HI_FAILURE;
    }

    /* set vo channel attribute and start each channel. */
    u32Width  = pstVideoLayerAttr->stImageSize.u32Width;
    u32Height = pstVideoLayerAttr->stImageSize.u32Height;

    if (s32ChnTotal == 1)
    {
        ;
    }
    else if (s32ChnTotal <= 4)
    {
        s32ChnTotal = 4;
    }
    else if (s32ChnTotal <= 9)
    {
        s32ChnTotal = 9;
    }
    else if (s32ChnTotal <= 16)
    {
        s32ChnTotal = 16;
    }
    else if (s32ChnTotal <= 36)
    {
        /* 36-screen split is support. But only 32 vo channels display. */
        s32ChnTotal = VO_MAX_CHN_NUM;
    }
    else
    {
        printf("too many vo channels!\n");
        return HI_FAILURE;
    }

    s32Ret = SAMPLE_SetVoChnMScreen(VoDev, s32ChnTotal, u32Width, u32Height);
    if (HI_SUCCESS != s32Ret)
    {
        return HI_FAILURE;
    }

    for (VoChn = 0; VoChn < s32ChnTotal; VoChn++)
    {
        s32Ret = HI_MPI_VO_EnableChn(VoDev, VoChn);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_VO_EnableChn(%d, %d) failed, err code:0x%08x\n\n",
                VoDev, VoChn, s32Ret);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_StopVo(HI_S32 s32ChnTotal, VO_DEV VoDev)
{
    VO_CHN VoChn;
    HI_S32 s32Ret;

    for (VoChn = 0; VoChn < s32ChnTotal; VoChn++)
    {
        s32Ret = HI_MPI_VO_DisableChn(VoDev, VoChn);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_VO_DisableChn(%d, %d) fail, err code: 0x%08x.\n",
                VoDev, VoChn, s32Ret);
            return HI_FAILURE;
        }
    }
    s32Ret = HI_MPI_VO_DisableVideoLayer(VoDev);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VO_DisableVideoLayer(%d) fail, err code: 0x%08x.\n",
            VoDev, s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VO_Disable(VoDev);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VO_Disable(%d) fail, err code: 0x%08x.\n",
            VoDev, s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}
/*****************************************************************************
 Prototype       : SAMPLE_ViBindVo
 Description     : Bind VI to VO sequently, i.e, VI channe is same with VO channel
                   binded.
 Input           : u32ViChnCnt  **
                   VoDev        **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
 Remark          : frames captured by different vi device may display on one vo device.
*****************************************************************************/
HI_S32 SAMPLE_ViBindVo(HI_S32 s32ViChnTotal, VO_DEV VoDev)
{
    HI_S32 ViChnCnt;
    HI_S32 s32ViChnPerDev;
    VI_DEV ViDev;
    VO_CHN VoChn;
    VI_CHN ViChn;
    HI_S32 s32Ret;

    s32ViChnPerDev = SAMPLE_GetViChnPerDev(0);

    /* Bind VI channel to VO channel */
    ViDev = G_VIDEV_START;
    VoChn = 0;
    ViChnCnt = 0;
    while (1)
    {
        for (ViChn = 0; ViChn < s32ViChnPerDev; ViChn++, VoChn++)
        {
            s32Ret = HI_MPI_VI_BindOutput(ViDev, ViChn, VoDev, VoChn);
            if (0 != s32Ret)
            {
                printf("bind vi2vo failed, vi(%d,%d),vo(%d,%d)!\n",
                       ViDev, ViChn, VoDev, VoChn);
                return s32Ret;
            }

            ViChnCnt++;
            if (ViChnCnt >= s32ViChnTotal)
            {
                return HI_SUCCESS;
            }
        }
#ifdef hi3515
        ViDev += 2;
#else
        ViDev ++;
#endif
    }

    return HI_SUCCESS;
}




/***************Level 2 TEST CODE************************/

/*****************************************************************************
 Prototype       : SAMPLE_StartVi_SD
 Description     : start vi to input standard-definition video.
 Input           : s32ViChnTotal  **
                   enPicSize      **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
*****************************************************************************/
HI_S32 SAMPLE_StartVi_SD(HI_S32 s32ViChnTotal, PIC_SIZE_E enPicSize)
{
#if 0
    VI_PUB_ATTR_S stViDevAttr;
    VI_CHN_ATTR_S stViChnAttr;
    HI_S32 s32Ret;

    SAMPLE_TW2865_CfgV(gs_enViNorm, VI_WORK_MODE_4D1);

    stViDevAttr.enInputMode = VI_MODE_BT656;
    stViDevAttr.enWorkMode = VI_WORK_MODE_4D1;
    stViDevAttr.enViNorm = gs_enViNorm;

    stViChnAttr.stCapRect.u32Width  = 704;
    stViChnAttr.stCapRect.u32Height =
        (VIDEO_ENCODING_MODE_PAL == gs_enViNorm) ? 288 : 240;
    stViChnAttr.stCapRect.s32X = 8;
    stViChnAttr.stCapRect.s32Y = 0;
    stViChnAttr.enViPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stViChnAttr.bHighPri = HI_FALSE;
    stViChnAttr.bChromaResample = HI_FALSE;

    /* different pic size has different capture method for BT656. */
    stViChnAttr.enCapSel = (PIC_D1 == enPicSize || PIC_2CIF == enPicSize) ? \
                             VI_CAPSEL_BOTH : VI_CAPSEL_BOTTOM;
    stViChnAttr.bDownScale = (PIC_D1 == enPicSize || PIC_HD1 == enPicSize) ? \
                               HI_FALSE : HI_TRUE;

    s32Ret = SAMPLE_StartViByChn(s32ViChnTotal, &stViDevAttr, &stViChnAttr);
    if(HI_SUCCESS != s32Ret)
    {
        return HI_FAILURE;
    }
#endif
    return HI_SUCCESS;
}

HI_S32 SAMPLE_StartVi_SD_ql(HI_S32 s32ViChnTotal, PIC_SIZE_E enPicSize[REAL_CHANNEL_NUM][2])
{
    VI_PUB_ATTR_S stViDevAttr;
    VI_CHN_ATTR_S stViChnAttr[REAL_CHANNEL_NUM];
    HI_S32 s32Ret;
	int i;

    SAMPLE_TW2865_CfgV(gs_enViNorm, VI_WORK_MODE_4D1);

    stViDevAttr.enInputMode     = VI_MODE_BT656;
    stViDevAttr.enWorkMode         = VI_WORK_MODE_4D1;
    stViDevAttr.enViNorm         = gs_enViNorm;

    stViChnAttr[0].stCapRect.u32Width  = 704;
    stViChnAttr[0].stCapRect.u32Height =
        (VIDEO_ENCODING_MODE_PAL == gs_enViNorm) ? 288 : 240;
    stViChnAttr[0].stCapRect.s32X = 8;
    stViChnAttr[0].stCapRect.s32Y = 0;
    stViChnAttr[0].enViPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stViChnAttr[0].bHighPri = HI_FALSE;
    stViChnAttr[0].bChromaResample = HI_FALSE;

    /* different pic size has different capture method for BT656. */
    stViChnAttr[0].enCapSel = (PIC_D1 == enPicSize[0][0] || PIC_2CIF == enPicSize[0][0]) ? \
                             VI_CAPSEL_BOTH : VI_CAPSEL_BOTTOM;
    stViChnAttr[0].bDownScale = (PIC_D1 == enPicSize[0][0] || PIC_HD1 == enPicSize[0][0]) ? \
                               HI_FALSE : HI_TRUE;
                      
	for( i = 1; i < REAL_CHANNEL_NUM; ++i )
    {
    	stViChnAttr[i] = stViChnAttr[0];
    	stViChnAttr[i].enCapSel = (PIC_D1 == enPicSize[i][0] || PIC_2CIF == enPicSize[i][0]) ? \
                             VI_CAPSEL_BOTH : VI_CAPSEL_BOTTOM;
    	stViChnAttr[i].bDownScale = (PIC_D1 == enPicSize[i][0] || PIC_HD1 == enPicSize[i][0]) ? \
                               HI_FALSE : HI_TRUE;        
    }
    
    s32Ret = SAMPLE_StartViByChn_ls(s32ViChnTotal, &stViDevAttr, stViChnAttr);
    if(HI_SUCCESS != s32Ret)
    {
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/*****************************************************************************
 Prototype       : SAMPLE_StartVo_SD
 Description     : start vo to display standard-definition video.
 Input           : s32VoChnTotal  **
                   VoDev          **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
 Remark          : Using different vo device, you can display video on SDTV or VGA.
*****************************************************************************/
HI_S32 SAMPLE_StartVo_SD(HI_S32 s32VoChnTotal, VO_DEV VoDev)
{
    VO_PUB_ATTR_S stVoDevAttr;
    VO_VIDEO_LAYER_ATTR_S stVideoLayerAttr;
    HI_U32 u32Width;
    HI_U32 u32Height;
    HI_U32 u32DisplayRate = -1;
    HI_S32 s32Ret;

    switch (VoDev)
    {
        case VO_DEV_HD:
            stVoDevAttr.enIntfType = VO_INTF_VGA;
            stVoDevAttr.enIntfSync = VO_OUTPUT_800x600_60;
            stVoDevAttr.u32BgColor = VO_BKGRD_BLACK;
            u32DisplayRate = (VO_OUTPUT_PAL == gs_enSDTvMode) ? 25 : 30;
            break;
        case VO_DEV_AD:
            stVoDevAttr.enIntfType = VO_INTF_CVBS;
            stVoDevAttr.enIntfSync = gs_enSDTvMode;
            stVoDevAttr.u32BgColor = VO_BKGRD_BLACK;
            u32DisplayRate = (VO_OUTPUT_PAL == gs_enSDTvMode) ? 25 : 30;
            break;
        case VO_DEV_SD:
            stVoDevAttr.enIntfType = VO_INTF_CVBS;
            stVoDevAttr.enIntfSync = gs_enSDTvMode;
            stVoDevAttr.u32BgColor = VO_BKGRD_BLACK;
            u32DisplayRate = (VO_OUTPUT_PAL == gs_enSDTvMode) ? 25 : 30;
            break;
        default:
            return HI_FAILURE;
    }

    u32Width = 720;
    u32Height = (VO_OUTPUT_PAL == gs_enSDTvMode) ? 576 : 480;
    stVideoLayerAttr.stDispRect.s32X = 0;
    stVideoLayerAttr.stDispRect.s32Y = 0;
    stVideoLayerAttr.stDispRect.u32Width   = u32Width;
    stVideoLayerAttr.stDispRect.u32Height  = u32Height;
    stVideoLayerAttr.stImageSize.u32Width  = u32Width;
    stVideoLayerAttr.stImageSize.u32Height = u32Height;
    stVideoLayerAttr.u32DispFrmRt = u32DisplayRate;
    stVideoLayerAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stVideoLayerAttr.s32PiPChn = VO_DEFAULT_CHN;

    s32Ret = SAMPLE_StartVo(s32VoChnTotal, VoDev, &stVoDevAttr, &stVideoLayerAttr);
    if(HI_SUCCESS != s32Ret)
    {
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/*****************************************************************************
 Prototype       : SAMPLE_StartViVo_SD
 Description     : start vio (i.e. preview) . input and output are both standart-
                   definition.
 Input           : s32ChnTotal  **
                   enPicSize    **
                   VoDev        **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :

*****************************************************************************/
HI_S32 SAMPLE_StartViVo_SD(HI_S32 s32ChnTotal, PIC_SIZE_E enViSize, VO_DEV VoDev)
{
    VI_DEV ViDev;
    VI_CHN ViChn;
    VO_CHN VoChn;
    HI_S32 s32ChnCnt;
    HI_S32 s32ViChnPerDev;
    HI_S32 s32Ret;

    s32ViChnPerDev = 4;
    ViDev = G_VIDEV_START;
    ViChn = 0;
    VoChn = 0;

    SAMPLE_StartVi_SD(s32ChnTotal, enViSize);
    SAMPLE_StartVo_SD(s32ChnTotal, VoDev);

    s32ChnCnt = 0;
    while(s32ChnTotal--)
    {
        s32Ret = HI_MPI_VI_BindOutput(ViDev, ViChn, VoDev, VoChn);
        if (HI_SUCCESS != s32Ret)
        {
            printf("bind Vi(%d,%d) to Vo(%d,%d) fail! \n",
                ViDev, ViChn, VoDev, VoChn);
            return HI_FAILURE;
        }
        ViChn++;
        VoChn++;
        if (++s32ChnCnt == s32ViChnPerDev)
        {
            s32ChnCnt = 0;
            ViChn = 0;
            #ifdef hi3515
            ViDev+=2;
            #else
            ViDev++;
            #endif
        }
    }
    return HI_SUCCESS;
}

HI_S32 SAMPLE_StartViVo_SD_ls(HI_S32 s32ChnTotal, PIC_SIZE_E enViSize[REAL_CHANNEL_NUM][2], VO_DEV VoDev)
{
    VI_DEV ViDev;
    VI_CHN ViChn;
    VO_CHN VoChn;
    HI_S32 s32ChnCnt;
    HI_S32 s32ViChnPerDev;
    HI_S32 s32Ret;

    s32ViChnPerDev = 4;
    ViDev = G_VIDEV_START;
    ViChn = 0;
    VoChn = 0;

    SAMPLE_StartVi_SD_ql(s32ChnTotal, enViSize);
    SAMPLE_StartVo_SD(s32ChnTotal, VoDev);

    s32ChnCnt = 0;
    while(s32ChnTotal--)
    {
        s32Ret = HI_MPI_VI_BindOutput(ViDev, ViChn, VoDev, VoChn);
        if (HI_SUCCESS != s32Ret)
        {
            printf("bind Vi(%d,%d) to Vo(%d,%d) fail! \n",
                ViDev, ViChn, VoDev, VoChn);
            return HI_FAILURE;
        }
        ViChn++;
        VoChn++;
        if (++s32ChnCnt == s32ViChnPerDev)
        {
            s32ChnCnt = 0;
            ViChn = 0;
            #ifdef hi3515
            ViDev+=2;
            #else
            ViDev++;
            #endif
        }
    }
    return HI_SUCCESS;
}


HI_S32 SAMPLE_StopViVo_SD(HI_S32 s32ChnTotal, VO_DEV VoDev)
{
    VI_DEV ViDev;
    VI_CHN ViChn;
    VO_CHN VoChn;
    HI_S32 s32ChnCnt;
    HI_S32 s32ViChnPerDev;
    HI_S32 s32Ret;

    s32ViChnPerDev = 4;
    ViDev = G_VIDEV_START;
    ViChn = 0;
    VoChn = 0;

    SAMPLE_StopViByChn(s32ChnTotal);
    SAMPLE_StopVo(s32ChnTotal, VoDev);

    s32ChnCnt = 0;
    while(s32ChnTotal--)
    {
        s32Ret = HI_MPI_VI_UnBindOutput(ViDev, ViChn, VoDev, VoChn);
        if (HI_SUCCESS != s32Ret)
        {
            printf("bind Vi(%d,%d) to Vo(%d,%d) fail! \n",
                ViDev, ViChn, VoDev, VoChn);
            return HI_FAILURE;
        }
        ViChn++;
        VoChn++;
        if (++s32ChnCnt == s32ViChnPerDev)
        {
            s32ChnCnt = 0;
            ViChn = 0;
            ViDev++;
        }
    }
    return HI_SUCCESS;
}

/*****************************************************************************
 Prototype       : SAMPLE_StopAllVi
 Description     : Stop all vi
 Input           : HI_VOID  **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
 Remark          : simplified method to stop vi.
*****************************************************************************/
HI_S32 SAMPLE_StopAllVi(HI_VOID)
{
    HI_S32 i;

    for (i = 0; i < VIU_MAX_CHN_NUM; i++)
    {
        HI_MPI_VI_DisableChn(i / 4, i % 4);
    }

    for (i = 0; i < VIU_MAX_DEV_NUM; i++)
    {
        HI_MPI_VI_Disable(i);
    }

    return 0;
}

HI_S32 SAMPLE_StopAllVo(HI_VOID)
{
    VO_DEV VoDev;
    VO_CHN VoChn;

    for (VoDev = 0; VoDev < VO_MAX_DEV_NUM; VoDev++)
    {
        for (VoChn = 0; VoChn < VO_MAX_CHN_NUM; VoChn++)
        {
            HI_MPI_VO_DisableChn(VoDev, VoChn);
        }

        HI_MPI_VO_DisableVideoLayer(VoDev);
        HI_MPI_VO_Disable(VoDev);
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_VoPicSwitch(VO_DEV VoDev, HI_U32 u32VoPicDiv)
{
    VO_CHN VoChn;
    VO_CHN_ATTR_S VoChnAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    HI_S32 s32Ret;
    HI_S32 i, div, u32ScreemDiv, u32PicWidth, u32PicHeight;

    u32ScreemDiv = u32VoPicDiv;

    s32Ret = HI_MPI_VO_SetAttrBegin(VoDev);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VO_SetAttrBegin(%d) errcode: 0x%08x\n",
            VoDev, s32Ret);
        return HI_FAILURE;
    }
#if 1
    for (i = 0; i < VO_MAX_CHN_NUM; i++)
    {
        if (i < 16)
            HI_MPI_VO_ChnHide(VoDev, i);
    }
#endif
    div = sqrt(u32ScreemDiv);

    s32Ret = HI_MPI_VO_GetVideoLayerAttr(VoDev, &stLayerAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_VO_GetVideoLayerAttr(%d) errcode: 0x%08x\n",
            VoDev, s32Ret);
        return HI_FAILURE;
    }
    u32PicWidth  = stLayerAttr.stDispRect.u32Width / div;
    u32PicHeight = stLayerAttr.stDispRect.u32Height / div;

    for (i = 0; i < u32ScreemDiv; i++)
    {
        VoChn = i;
        VoChnAttr.stRect.s32X = (i % div) * u32PicWidth;
        VoChnAttr.stRect.s32Y = (i / div) * u32PicHeight;
        VoChnAttr.stRect.u32Width  = u32PicWidth;
        VoChnAttr.stRect.u32Height = u32PicHeight;
        VoChnAttr.u32Priority = 1;
        VoChnAttr.bZoomEnable = HI_TRUE;
        VoChnAttr.bDeflicker  = HI_FALSE;
        if (0 != HI_MPI_VO_SetChnAttr(VoDev, VoChn, &VoChnAttr))
        {
            printf("set VO Chn %d attribute(%d,%d,%d,%d) failed !\n",
                   VoChn, VoChnAttr.stRect.s32X, VoChnAttr.stRect.s32Y,
                   VoChnAttr.stRect.u32Width, VoChnAttr.stRect.u32Height);
            return -1;
        }
#if 1
        if (0 != HI_MPI_VO_ChnShow(VoDev, VoChn))
        {
            return -1;
        }
#endif
    }

    if (0 != HI_MPI_VO_SetAttrEnd(VoDev))
    {
        return -1;
    }

    return 0;
}
#endif
HI_S32 SAMPLE_GetJpegeCfg(PIC_SIZE_E enPicSize, VENC_ATTR_JPEG_S *pstJpegeAttr)
{
    VENC_ATTR_JPEG_S stJpegAttr;

    if (PIC_D1 == enPicSize)
    {
        stJpegAttr.u32PicWidth          = 704;
        stJpegAttr.u32PicHeight         = (VIDEO_ENCODING_MODE_PAL==gs_enViNorm)?576:480;
    }
    else if (PIC_CIF == enPicSize)
    {
        stJpegAttr.u32PicWidth          = 352;
        stJpegAttr.u32PicHeight         = (VIDEO_ENCODING_MODE_PAL==gs_enViNorm)?288:240;
    }
    else
    {
        printf("%s: not support this payload type\n", __FUNCTION__);
        return -1;
    }

    stJpegAttr.u32MaxPicWidth = stJpegAttr.u32PicWidth;
    stJpegAttr.u32MaxPicHeight= stJpegAttr.u32PicHeight;
    stJpegAttr.u32BufSize   = stJpegAttr.u32PicWidth * stJpegAttr.u32PicHeight * 2;
    stJpegAttr.bVIField     = HI_TRUE;
    stJpegAttr.bByFrame     = HI_TRUE;
    //stJpegAttr.u32MCUPerECS = 0;
    stJpegAttr.u32Priority  = 0;
    //stJpegAttr.u32ImageQuality = 3;

    memcpy(pstJpegeAttr, &stJpegAttr, sizeof(VENC_ATTR_JPEG_S));

    return 0;
}

HI_S32 mpi_comm_get_Mjpegeconfig(PIC_SIZE_E enPicSize, HI_BOOL bMainStream,
        VENC_ATTR_MJPEG_S *pstMjpegeAttr)
{
    VENC_ATTR_MJPEG_S stMjpegAttr;

    if (PIC_D1 == enPicSize)
    {
        stMjpegAttr.u32PicWidth          = 704;
        stMjpegAttr.u32PicHeight         = (VIDEO_ENCODING_MODE_PAL==gs_enViNorm)?576:480;
        //stMjpegAttr.u32TargetBitrate     = 8192;
    }
    else if (PIC_CIF == enPicSize)
    {
        stMjpegAttr.u32PicWidth          = 352;
        stMjpegAttr.u32PicHeight         = (VIDEO_ENCODING_MODE_PAL==gs_enViNorm)?288:240;
        //stMjpegAttr.u32TargetBitrate     = 4096;
    }
    else if (PIC_QCIF == enPicSize)
    {
        stMjpegAttr.u32PicWidth          = 176;
        stMjpegAttr.u32PicHeight         = (VIDEO_ENCODING_MODE_PAL==gs_enViNorm)?144:120;;
        //stMjpegAttr.u32TargetBitrate     = 2048;
    }
    else
    {
        printf("%s: not support this payload type\n", __FUNCTION__);
        return -1;
    }

    stMjpegAttr.bMainStream             = bMainStream;
    stMjpegAttr.bByFrame                = HI_TRUE;
    stMjpegAttr.bVIField                = HI_TRUE;
    //stMjpegAttr.u32ViFramerate          = (VIDEO_ENCODING_MODE_PAL==gs_enViNorm)?25:30;
    //stMjpegAttr.u32TargetFramerate      = stMjpegAttr.u32ViFramerate;
    //stMjpegAttr.u32MCUPerECS            = 0;
    stMjpegAttr.u32BufSize              = stMjpegAttr.u32PicWidth * stMjpegAttr.u32PicHeight * 2;

    memcpy(pstMjpegeAttr, &stMjpegAttr, sizeof(VENC_ATTR_MJPEG_S));

    return 0;
}

HI_S32 SAMPLE_GetH264eCfg(PIC_SIZE_E enPicSize, HI_BOOL bMainStream,
        VENC_ATTR_H264_S *pstH264eAttr)
{
    VENC_ATTR_H264_S stH264Attr;

    if (PIC_D1 == enPicSize)
    {
        stH264Attr.u32PicWidth          = 704;
        stH264Attr.u32PicHeight         = (VIDEO_ENCODING_MODE_PAL==gs_enViNorm)?576:480;
        //stH264Attr.u32Bitrate           = 1024;
    }
    else if (PIC_HD1 == enPicSize)
    {
        stH264Attr.u32PicWidth          = 704;
        stH264Attr.u32PicHeight         = (VIDEO_ENCODING_MODE_PAL==gs_enViNorm)?288:240;
        //stH264Attr.u32Bitrate           = 1024;
    }
    else if (PIC_CIF == enPicSize)
    {
        stH264Attr.u32PicWidth          = 352;
        stH264Attr.u32PicHeight         = (VIDEO_ENCODING_MODE_PAL==gs_enViNorm)?288:240;
        //stH264Attr.u32Bitrate           = 512;
    }
    else if (PIC_QCIF == enPicSize)
    {
        stH264Attr.u32PicWidth          = 176;
        stH264Attr.u32PicHeight         = (VIDEO_ENCODING_MODE_PAL==gs_enViNorm)?144:120;
        //stH264Attr.u32Bitrate           = 256;
    }
    else if (PIC_VGA == enPicSize)
    {
        stH264Attr.u32PicWidth          = 640;
        stH264Attr.u32PicHeight         = 480;
        //stH264Attr.u32Bitrate           = 1024;
    }
    else if (PIC_QVGA == enPicSize)
    {
        stH264Attr.u32PicWidth          = 320;
        stH264Attr.u32PicHeight         = 240;
        //stH264Attr.u32Bitrate           = 512;
    }
    else if (PIC_HD720 == enPicSize)
    {
        stH264Attr.u32PicWidth          = 1280;
        stH264Attr.u32PicHeight         = 720;
        //stH264Attr.u32Bitrate           = 2048;
    }
    else if (PIC_HD1080 == enPicSize)
    {
        stH264Attr.u32PicWidth          = 1920;
        stH264Attr.u32PicHeight         = 1072;
        //stH264Attr.u32Bitrate           = 4000;
    }
    else
    {
        printf("%s: not support this payload type\n", __FUNCTION__);
        return -1;
    }

    stH264Attr.bMainStream          = bMainStream;
    stH264Attr.bByFrame             = HI_TRUE;
    //stH264Attr.enRcMode             = RC_MODE_CBR;
    stH264Attr.bField               = HI_FALSE;
    stH264Attr.bVIField             = HI_TRUE;
    //stH264Attr.u32ViFramerate       = (VIDEO_ENCODING_MODE_PAL==gs_enViNorm)?25:30;
    //stH264Attr.u32TargetFramerate   = stH264Attr.u32ViFramerate;
    //stH264Attr.u32Gop               = 100;
    //stH264Attr.u32MaxDelay          = 100;
    //stH264Attr.u32PicLevel          = 0;
    stH264Attr.u32Priority          = 0;
    stH264Attr.u32BufSize           = stH264Attr.u32PicWidth * stH264Attr.u32PicHeight * 2;

    memcpy(pstH264eAttr, &stH264Attr, sizeof(VENC_ATTR_H264_S));

    return 0;
}

HI_S32 SAMPLE_GetH264eCfg_ls(PIC_SIZE_E enPicSize[REAL_CHANNEL_NUM][2], HI_BOOL bMainStream,
        VENC_ATTR_H264_S pstH264eAttr[REAL_CHANNEL_NUM][2], int channel )
{
	int ret;
    VENC_ATTR_H264_S stH264Attr;
    PARAM_CONFIG_VIDEO_ENCODE vep;
    int j = (HI_TRUE == bMainStream)?0:1;

	ret = ParamGetVideoEncode( channel, &vep );
	if( 0 == ret )
    {
        SVPrint("ParamGetVideoEncode successed \
        ;FILE:%s,LINE:%d\n",__FILE__,__LINE__);
    	if (PIC_D1 == enPicSize[channel][j])
        {
            stH264Attr.u32PicWidth          = 704;
            stH264Attr.u32PicHeight         = (VIDEO_ENCODING_MODE_PAL==gs_enViNorm)?576:480;
        }
        else if (PIC_HD1 == enPicSize[channel][j])
        {
            stH264Attr.u32PicWidth          = 704;
            stH264Attr.u32PicHeight         = (VIDEO_ENCODING_MODE_PAL==gs_enViNorm)?288:240;
        }
        else if (PIC_CIF == enPicSize[channel][j])
        {
            stH264Attr.u32PicWidth          = 352;
            stH264Attr.u32PicHeight         = (VIDEO_ENCODING_MODE_PAL==gs_enViNorm)?288:240;
        }
        else if (PIC_QCIF == enPicSize[channel][j])
        {
            stH264Attr.u32PicWidth          = 176;
            stH264Attr.u32PicHeight         = (VIDEO_ENCODING_MODE_PAL==gs_enViNorm)?144:120;
        }    
        //stH264Attr.u32Bitrate             = vep.bitrate;    
        stH264Attr.bMainStream          = bMainStream;
        stH264Attr.bByFrame             = HI_TRUE;
        //stH264Attr.enRcMode             = vep.bitrateType;
        stH264Attr.bField               = HI_FALSE;
        stH264Attr.bVIField             = HI_TRUE;
        //stH264Attr.u32ViFramerate       = vep.frameRate;
        //stH264Attr.u32TargetFramerate   = stH264Attr.u32ViFramerate;
        //stH264Attr.u32Gop               = vep.iFrameInterval;
        //stH264Attr.u32MaxDelay          = 100;
        //stH264Attr.u32PicLevel          = 0;
        stH264Attr.u32Priority          = 0;
        stH264Attr.u32BufSize           = stH264Attr.u32PicWidth * stH264Attr.u32PicHeight * 2;
        stH264Attr.u32Profile           = 0;
    }
	else
    {
        SVPrint("ParamGetVideoEncode failed,use default param \
        ;FILE:%d,LINE:%d\n",__FILE__,__LINE__);
        if (PIC_D1 == enPicSize[channel][j])
        {
            stH264Attr.u32PicWidth          = 704;
            stH264Attr.u32PicHeight         = (VIDEO_ENCODING_MODE_PAL==gs_enViNorm)?576:480;
            //stH264Attr.u32Bitrate           = 1024;
        }
        else if (PIC_HD1 == enPicSize[channel][j])
        {
            stH264Attr.u32PicWidth          = 704;
            stH264Attr.u32PicHeight         = (VIDEO_ENCODING_MODE_PAL==gs_enViNorm)?288:240;
            //stH264Attr.u32Bitrate           = 1024;
        }
        else if (PIC_CIF == enPicSize[channel][j])
        {
            stH264Attr.u32PicWidth          = 352;
            stH264Attr.u32PicHeight         = (VIDEO_ENCODING_MODE_PAL==gs_enViNorm)?288:240;
            //stH264Attr.u32Bitrate           = 512;
        }
        else if (PIC_QCIF == enPicSize[channel][j])
        {
            stH264Attr.u32PicWidth          = 176;
            stH264Attr.u32PicHeight         = (VIDEO_ENCODING_MODE_PAL==gs_enViNorm)?144:120;
            //stH264Attr.u32Bitrate           = 256;
        }
        else if (PIC_VGA == enPicSize[channel][j])
        {
            stH264Attr.u32PicWidth          = 640;
            stH264Attr.u32PicHeight         = 480;
            //stH264Attr.u32Bitrate           = 1024;
        }
        else if (PIC_QVGA == enPicSize[channel][j])
        {
            stH264Attr.u32PicWidth          = 320;
            stH264Attr.u32PicHeight         = 240;
            //stH264Attr.u32Bitrate           = 512;
        }
        else if (PIC_HD720 == enPicSize[channel][j])
        {
            stH264Attr.u32PicWidth          = 1280;
            stH264Attr.u32PicHeight         = 720;
            //stH264Attr.u32Bitrate           = 2048;
        }
        else if (PIC_HD1080 == enPicSize[channel][j])
        {
            stH264Attr.u32PicWidth          = 1920;
            stH264Attr.u32PicHeight         = 1072;
            //stH264Attr.u32Bitrate           = 4000;
        }
        else
        {
            printf("%s: not support this payload type\n", __FUNCTION__);
            return -1;
        }

        stH264Attr.bMainStream          = bMainStream;
        stH264Attr.bByFrame             = HI_TRUE;
        //stH264Attr.enRcMode             = RC_MODE_CBR;
        stH264Attr.bField               = HI_FALSE;
        stH264Attr.bVIField             = HI_TRUE;
        //stH264Attr.u32ViFramerate       = (VIDEO_ENCODING_MODE_PAL==gs_enViNorm)?25:30;
        //stH264Attr.u32TargetFramerate   = stH264Attr.u32ViFramerate;
        //stH264Attr.u32Gop               = 100;
        //stH264Attr.u32MaxDelay          = 100;
        //stH264Attr.u32PicLevel          = 0;
        stH264Attr.u32Priority          = 0;
        stH264Attr.u32BufSize           = stH264Attr.u32PicWidth * stH264Attr.u32PicHeight * 2;
    }
    
    memcpy(&pstH264eAttr[channel][j], &stH264Attr, sizeof(VENC_ATTR_H264_S));

    return 0;
}

HI_S32 mpi_comm_get_h264config_3515a(PIC_SIZE_E enPicSize[REAL_CHANNEL_NUM][2], HI_BOOL bMainStream,
        VENC_CHN_ATTR_S pstH264eAttr[REAL_CHANNEL_NUM][2], int channel )
{
	int ret;
    //VENC_ATTR_H264_S stH264Attr;
    PARAM_CONFIG_VIDEO_ENCODE vep;
    int j = (HI_TRUE == bMainStream)?0:1;

	ret = ParamGetVideoEncode( channel, &vep );
    
    if (PIC_D1 == enPicSize[channel][j])
    {
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32PicWidth   = 704;
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32PicHeight  = (VIDEO_ENCODING_MODE_PAL==gs_enViNorm)?576:480;
    }
    else if (PIC_HD1 == enPicSize[channel][j])
    {
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32PicWidth   = 704;
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32PicHeight  = (VIDEO_ENCODING_MODE_PAL==gs_enViNorm)?288:240;
    }
    else if (PIC_CIF == enPicSize[channel][j])
    {
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32PicWidth   = 352;
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32PicHeight  = (VIDEO_ENCODING_MODE_PAL==gs_enViNorm)?288:240;
    }
    else if (PIC_QCIF == enPicSize[channel][j])
    {
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32PicWidth   = 176;
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32PicHeight  = (VIDEO_ENCODING_MODE_PAL==gs_enViNorm)?144:120;
    }
    else if (PIC_VGA == enPicSize[channel][j])
    {
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32PicWidth          = 640;
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32PicHeight         = 480;
    }
    else if (PIC_QVGA == enPicSize[channel][j])
    {
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32PicWidth          = 320;
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32PicHeight         = 240;
    }
    else if (PIC_HD720 == enPicSize[channel][j])
    {
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32PicWidth          = 1280;
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32PicHeight         = 720;
    }
    else if (PIC_HD1080 == enPicSize[channel][j])
    {
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32PicWidth          = 1920;
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32PicHeight         = 1072;
    }
    else
    {
        printf("%s: not support this payload type\n", __FUNCTION__);
        return -1;
    }
    
	if( 0 == ret )
    {
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32MaxPicWidth       = \
            pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32PicWidth;
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32MaxPicHeight      = \
            pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32PicHeight;
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.bMainStream  = bMainStream;
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.bByFrame     = HI_TRUE;
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.bField       = HI_FALSE;
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.bVIField     = HI_TRUE;

        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32Priority  = 0;
        #if 1
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32BufSize   = \
            pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32PicWidth * \
            pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32PicHeight*2;
        #endif
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32Profile   = 2;
        pstH264eAttr[channel][j].stRcAttr.enRcMode = vep.bitrateType ;
        //vep.bitrate = 
        if(pstH264eAttr[channel][j].stRcAttr.enRcMode == VENC_RC_MODE_H264CBR)
        {
            CORRECTPRINT("VENC_RC_MODE_H264CBR");
            pstH264eAttr[channel][j].stRcAttr.stAttrH264Cbr.u32BitRate = vep.bitrate;
            pstH264eAttr[channel][j].stRcAttr.stAttrH264Cbr.fr32TargetFrmRate = vep.frameRate;
            pstH264eAttr[channel][j].stRcAttr.stAttrH264Cbr.u32Gop = vep.iFrameInterval;
            pstH264eAttr[channel][j].stRcAttr.stAttrH264Cbr.u32ViFrmRate = vep.frameRate;
            pstH264eAttr[channel][j].stRcAttr.stAttrH264Cbr.u32FluctuateLevel = 0;
            pstH264eAttr[channel][j].stRcAttr.stAttrH264Cbr.u32StatTime = 1;
        }
        else
        {
            
            pstH264eAttr[channel][j].stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
            pstH264eAttr[channel][j].stRcAttr.stAttrH264Vbr.fr32TargetFrmRate = vep.frameRate;
            pstH264eAttr[channel][j].stRcAttr.stAttrH264Vbr.u32Gop = vep.iFrameInterval;
            pstH264eAttr[channel][j].stRcAttr.stAttrH264Vbr.u32StatTime = 1;
            pstH264eAttr[channel][j].stRcAttr.stAttrH264Vbr.u32ViFrmRate = vep.frameRate;
            pstH264eAttr[channel][j].stRcAttr.stAttrH264Vbr.u32MaxBitRate = vep.bitrate;
            pstH264eAttr[channel][j].stRcAttr.stAttrH264Vbr.u32MinQp = 30;
            pstH264eAttr[channel][j].stRcAttr.stAttrH264Vbr.u32MaxQp = 50;//图片质量控制过大时PROCON会出问题，TBD
            CORRECTPRINT("VENC_RC_MODE_H264VBR,bitrate is %d\n",vep.bitrate);
        }
        
    } 
	else
    {
        SVPrint("ParamGetVideoEncode failed,use default param \
        ;FILE:%d,LINE:%d\n",__FILE__,__LINE__);

        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32MaxPicWidth       = \
            pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32PicWidth;
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32MaxPicHeight      = \
            pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32PicHeight;
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.bMainStream  = bMainStream;
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.bByFrame     = HI_TRUE;
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.bField       = HI_FALSE;
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.bVIField     = HI_TRUE;

        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32Priority  = 0;
        #if 1
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32BufSize   = \
            pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32PicWidth * \
            pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32PicHeight*2;
        #endif
        pstH264eAttr[channel][j].stVeAttr.stAttrH264e.u32Profile   = 2;
        pstH264eAttr[channel][j].stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
        pstH264eAttr[channel][j].stRcAttr.stAttrH264Cbr.u32BitRate = 512;
        pstH264eAttr[channel][j].stRcAttr.stAttrH264Cbr.fr32TargetFrmRate = 30;
        pstH264eAttr[channel][j].stRcAttr.stAttrH264Cbr.u32Gop = vep.iFrameInterval;
        pstH264eAttr[channel][j].stRcAttr.stAttrH264Cbr.u32ViFrmRate = 30;
        pstH264eAttr[channel][j].stRcAttr.stAttrH264Cbr.u32FluctuateLevel = 0;
        pstH264eAttr[channel][j].stRcAttr.stAttrH264Cbr.u32StatTime = 1;
    }
    
    return 0;
}



#if defined MCU_HI3515
HI_S32 SAMPlE_GetGrpViMap(VENC_GRP VeGrp, VI_DEV *pViDev, VI_CHN *pViChn)
{
    HI_S32 s32ViChnPerDev;

    s32ViChnPerDev = SAMPLE_GetViChnPerDev(0);
    *pViDev = VeGrp / s32ViChnPerDev;
    *pViChn = VeGrp % s32ViChnPerDev;
    return 0;
}


HI_S32 SAMPLE_StartOneVenc(VENC_GRP VeGrp, VI_DEV ViDev, VI_CHN ViChn,
    PAYLOAD_TYPE_E enType, PIC_SIZE_E enSize, HI_S32 s32FrmRate)
{
    HI_S32 s32Ret;
    VENC_CHN VeChn;
    VENC_ATTR_H264_S stH264eAttr;
    VENC_ATTR_MJPEG_S stMjpegAttr;
    VENC_CHN_ATTR_S stVencAttr;

    if (PT_H264 == enType)
    {
        /* you should get config by your method */
        SAMPLE_GetH264eCfg(enSize, HI_TRUE, &stH264eAttr);
        //stH264eAttr.u32TargetFramerate = s32FrmRate;
    }
    else if (PT_MJPEG == enType)
    {
        mpi_comm_get_Mjpegeconfig(enSize, HI_TRUE, &stMjpegAttr);
        //stMjpegAttr.u32TargetFramerate = s32FrmRate;
    }
    else
    {
        printf("venc not support this payload type :%d\n", enType);
        return -1;
    }

    s32Ret = HI_MPI_VENC_CreateGroup(VeGrp);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_VENC_CreateGroup(%d) err 0x%x\n", VeGrp, s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VENC_BindInput(VeGrp, ViDev, ViChn);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_VENC_BindInput err 0x%x\n", s32Ret);
        return HI_FAILURE;
    }
    printf("grp %d bind vi(%d,%d) ok\n", VeGrp, ViDev, ViChn);

    VeChn = VeGrp;
    stVencAttr.enType = enType;
    if (PT_H264 == stVencAttr.enType)
    {
        stVencAttr.pValue = &stH264eAttr;
    }
    else
    {
        stVencAttr.pValue = &stMjpegAttr;
    }
    s32Ret = HI_MPI_VENC_CreateChn(VeChn, &stVencAttr, HI_NULL);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_VENC_CreateChn(%d) err 0x%x\n", VeChn, s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VENC_RegisterChn(VeGrp, VeChn);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_VENC_RegisterChn err 0x%x\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VENC_StartRecvPic(VeChn);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_VENC_StartRecvPic err 0x%x\n", s32Ret);
        return HI_FAILURE;
    }
    printf("venc chn%d start ok\n", VeChn);

    return HI_SUCCESS;
}

HI_S32 SAMPLE_StartVenc(HI_U32 u32GrpCnt, HI_BOOL bHaveMinor,
    PAYLOAD_TYPE_E aenType[2], PIC_SIZE_E aenSize[2])
{
    HI_S32 i, j;
    HI_S32 s32Ret, s32GrpChn;
    VI_DEV ViDev;
    VI_CHN ViChn;
    VENC_GRP VeGroup;
    VENC_CHN VeChn;
    VENC_ATTR_H264_S stH264eAttr[2];
    VENC_ATTR_MJPEG_S stMjpegAttr[2];
    VENC_CHN_ATTR_S stVencAttr;

    s32GrpChn = bHaveMinor ? 2 : 1;

    for (j=0; j<s32GrpChn; j++)
    {
        HI_BOOL bMainChn = (0==j)?HI_TRUE:HI_FALSE;
        if (PT_H264 == aenType[j])
        {
             SAMPLE_GetH264eCfg(aenSize[j], bMainChn, &stH264eAttr[j]);
        }
        else if (PT_MJPEG == aenType[j])
        {
            mpi_comm_get_Mjpegeconfig(aenSize[j], bMainChn, &stMjpegAttr[j]);
        }
        else
        {
            printf("venc not support this payload type :%d\n", aenType[j]);
            return -1;
        }

        if( (aenSize[1] == PIC_QCIF) && (VIDEO_ENCODING_MODE_NTSC == gs_enViNorm))
        {
            HI_U32 u32HeightTmp = stH264eAttr[j].u32PicHeight;

            stH264eAttr[j].u32PicHeight += (((u32HeightTmp)/120)*8);
        }
    }

    for (i=0; i<u32GrpCnt; i++)
    {
        VeGroup = i;

        s32Ret = HI_MPI_VENC_CreateGroup(VeGroup);
        if (s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_VENC_CreateGroup(%d) err 0x%x\n", VeGroup, s32Ret);
            return HI_FAILURE;
        }

        for (j=0; j<s32GrpChn; j++)
        {
            VeChn = i*2 + j;
            //stVencAttr.enType = aenType[j];
            stVencAttr.stVeAttr.enType = aenType[j];
            if (PT_H264 == stVencAttr.stVeAttr.enType)
            {
                stVencAttr.stVeAttr.stAttrH264e = stH264eAttr[j];
            }
            else
            {
                stVencAttr.stVeAttr.stAttrMjpeg= stMjpegAttr[j];
            }

            s32Ret = HI_MPI_VENC_CreateChn(VeChn, &stVencAttr);//HI_MPI_VENC_CreateChn(VeChn, &stVencAttr, HI_NULL);
            if (s32Ret != HI_SUCCESS)
            {
                printf("HI_MPI_VENC_CreateChn(%d) err 0x%x\n", VeChn, s32Ret);
                return HI_FAILURE;
            }

            s32Ret = HI_MPI_VENC_RegisterChn(VeGroup, VeChn);
            if (s32Ret != HI_SUCCESS)
            {
                printf("HI_MPI_VENC_RegisterChn err 0x%x\n", s32Ret);
                return HI_FAILURE;
            }

            s32Ret = HI_MPI_VENC_StartRecvPic(VeChn);
            if (s32Ret != HI_SUCCESS)
            {
                printf("HI_MPI_VENC_StartRecvPic err 0x%x\n", s32Ret);
                return HI_FAILURE;
            }
        }

#if defined MCU_HI3515
        /* After group is ready, then allow vi send picture to this group. */
        SAMPlE_GetGrpViMap(VeGroup, &ViDev, &ViChn);

        s32Ret = HI_MPI_VENC_BindInput(VeGroup, ViDev, ViChn);
        if (s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_VENC_BindInput err 0x%x\n", s32Ret);
            return HI_FAILURE;
        }
#endif        
    }

    return 0;
}

#endif


HI_S32 mpi_InitMPP(VB_CONF_S *pstVbConf)
{
    MPP_SYS_CONF_S stSysConf = {0};

    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit();

    if (HI_MPI_VB_SetConf(pstVbConf))
    {
        printf("HI_MPI_VB_SetConf failed!\n");
        return -1;
    }

    if (HI_MPI_VB_Init())
    {
        printf("HI_MPI_VB_Init failed!\n");
        return -1;
    }

    stSysConf.u32AlignWidth = 16;
    if (HI_MPI_SYS_SetConf(&stSysConf))
    {
        printf("conf : system config failed!\n");
        return -1;
    }

    if (HI_MPI_SYS_Init())
    {
        printf("sys init failed!\n");
        return -1;
    }

    return 0;
}

/*****************************************************************************
 Prototype       : SAMPLE_ExitMpp
 Description     : Exit Mpp
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
*****************************************************************************/
HI_S32 mpi_ExitMPP(HI_VOID)
{
    if (HI_MPI_SYS_Exit())
    {
        printf("sys quit fail\n");
        return -1;
    }

    if (HI_MPI_VB_Exit())
    {
        printf("vb quit fail\n");
        return -1;
    }

    return 0;
}
//add for hi 3515a
HI_S32 mpi_comm_start_venc_3515a(HI_U32 u32GrpCnt, HI_BOOL bHaveMinor,
    PAYLOAD_TYPE_E aenType[2], PIC_SIZE_E aenSize[REAL_CHANNEL_NUM][2])
{
    HI_S32 i, j;
    HI_S32 s32Ret, s32GrpChn;
    VENC_GRP VeGroup;
    VENC_CHN VeChn;
    VENC_ATTR_MJPEG_S stMjpegAttr[2];
    
    VENC_CHN_ATTR_S stH264eVencAttr[REAL_CHANNEL_NUM][2];
    VPSS_GRP VpssGrp;

    GROUP_CROP_CFG_S stGrpCropCfg;

    stGrpCropCfg.bEnable = HI_TRUE;
    stGrpCropCfg.stRect.s32X = 0;
    stGrpCropCfg.stRect.s32Y = 0;
    stGrpCropCfg.stRect.u32Height = 576;
    stGrpCropCfg.stRect.u32Width  =704;

    s32GrpChn = bHaveMinor ? 2 : 1;
    for( i = 0; i < u32GrpCnt; ++i )
    {
        for (j=0; j<s32GrpChn; j++)
        {
            HI_BOOL bMainChn = (0==j)?HI_TRUE:HI_FALSE;
            if (PT_H264 == aenType[j])
            {
                 mpi_comm_get_h264config_3515a(aenSize, bMainChn, stH264eVencAttr, i );
            }
            else if (PT_MJPEG == aenType[j])
            {
                mpi_comm_get_Mjpegeconfig(aenSize[i][j], bMainChn, &stMjpegAttr[j]);
            }
            else
            {
                ERRORPRINT("venc not support this payload type :%d\n", aenType[j]);
                return -1;
            }
        } 
    }

    for (i=0; i<u32GrpCnt; i++)
    {
        VeGroup = i;
        VpssGrp = i;

        s32Ret = HI_MPI_VENC_CreateGroup(VeGroup);
        if (s32Ret != HI_SUCCESS)
        {
            ERRORPRINT("HI_MPI_VENC_CreateGroup(%d) err 0x%x;\n", VeGroup, s32Ret);
            return HI_FAILURE;
        }

        for (j=0; j<s32GrpChn; j++)
        {
            VeChn = i*2 + j;            
            stH264eVencAttr[i][j].stVeAttr.enType = aenType[j];
#if 0
            if (PT_H264==stVencAttr.stVeAttr.enType)
            {
                stVencAttr.stVeAttr.stAttrH264e= stH264eAttr[i][j];
            }
            else
            {
                stVencAttr.stVeAttr.stAttrMjpeg= stMjpegAttr[j];
            }
#endif
            s32Ret = HI_MPI_VENC_CreateChn(VeChn, &(stH264eVencAttr[i][j]));
            if (s32Ret != HI_SUCCESS)
            {
                ERRORPRINT("HI_MPI_VENC_CreateChn(%d) err 0x%x\n", VeChn, s32Ret);
                return HI_FAILURE;
            }
            s32Ret = HI_MPI_VENC_RegisterChn(VeGroup, VeChn);
            if (s32Ret != HI_SUCCESS)
            {
                ERRORPRINT("HI_MPI_VENC_RegisterChn err 0x%x\n", s32Ret);
                return HI_FAILURE;
            }

            s32Ret = HI_MPI_VENC_StartRecvPic(VeChn);
            if (s32Ret != HI_SUCCESS)
            {
                ERRORPRINT("HI_MPI_VENC_StartRecvPic err 0x%x\n", s32Ret);
                return HI_FAILURE;
            }

        }
        
///////////////////////////////////////////tbd
        s32Ret = HI_MPI_VENC_SetGrpCrop(VeGroup, &stGrpCropCfg);
        if (s32Ret != HI_SUCCESS)
        {
            ERRORPRINT("HI_MPI_VENC_SetGrpCrop(%d) ERR 0X%x\n",VeGroup,s32Ret);
        }
///////////////////////////////////////////////////

        s32Ret = mpi_comm_venc_bind_vpss(VeGroup, VpssGrp, VPSS_BSTR_CHN);
        if (HI_SUCCESS != s32Ret)
        {
            ERRORPRINT("Vencgrp:%d bind vpssgrp:%d failed!\n",VeGroup,VpssGrp);
            return s32Ret;
        }
    }

    return 0;
}

HI_S32 mpi_comm_stop_venc(HI_U32 u32GrpCnt, HI_BOOL bHaveMinor)
{
    HI_S32 s32Ret, i, j;
    HI_U32 u32ChnCntPerGrp;
    VENC_GRP VeGroup;
    VENC_CHN VeChn;

    VPSS_GRP VpssGrp;

    u32ChnCntPerGrp = bHaveMinor?2:1;

    for (i=0; i<u32GrpCnt; i++)
    {
        VeGroup = i;
        VpssGrp = i;

        //s32Ret = HI_MPI_VENC_UnbindInput(VeGroup);
        s32Ret = mpi_comm_venc_unbind_vpss(VeGroup,VpssGrp,VPSS_BSTR_CHN);
        if (s32Ret != HI_SUCCESS)
        {
            printf("mpi_comm_venc_unbind_vpss(%d) err 0x%x\n",
                VeGroup, s32Ret);
            return HI_FAILURE;
        }

        for (j=u32ChnCntPerGrp-1; j>=0; j--)/* first unregister minor chn */
        {
            VeChn = i*2 + j;
            s32Ret = HI_MPI_VENC_StopRecvPic(VeChn);
            if (s32Ret != HI_SUCCESS)
            {
                printf("HI_MPI_VENC_StopRecvPic vechn %d err 0x%x\n", VeChn, s32Ret);
                return HI_FAILURE;
            }

            s32Ret = HI_MPI_VENC_UnRegisterChn(VeChn);
            if (s32Ret != HI_SUCCESS)
            {
                printf("HI_MPI_VENC_UnRegisterChn vechn %d err 0x%x\n", VeChn, s32Ret);
                return HI_FAILURE;
            }

            s32Ret = HI_MPI_VENC_DestroyChn(VeChn);
            if (s32Ret != HI_SUCCESS)
            {
                printf("HI_MPI_VENC_DestroyChn vechn %d err 0x%x\n", VeChn, s32Ret);
                return HI_FAILURE;
            }
        }
        s32Ret = HI_MPI_VENC_DestroyGroup(VeGroup);
        if (s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_VENC_DestroyGroup grp %d err 0x%x\n", VeGroup, s32Ret);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}
#if defined MCU_HI3515
HI_S32 SAMPLE_CreateJpegChn(VENC_GRP VeGroup, VENC_CHN SnapChn, PIC_SIZE_E enPicSize)
{
    HI_S32 s32Ret;
    VENC_CHN_ATTR_S stVencAttr;
    VENC_ATTR_JPEG_S stJpegeAttr;

    SAMPLE_GetJpegeCfg(enPicSize, &stJpegeAttr);
    stVencAttr.stVeAttr.enType = PT_JPEG;
    stVencAttr.stVeAttr.stAttrJpeg= stJpegeAttr;

    s32Ret = HI_MPI_VENC_CreateGroup(VeGroup);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_VENC_CreateGroup err 0x%x\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VENC_CreateChn(SnapChn, &stVencAttr);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_VENC_CreateChn err 0x%x\n", s32Ret);
        return HI_FAILURE;
    }

    /* This is recommended if you snap one picture each time. */
    s32Ret = HI_MPI_VENC_SetMaxStreamCnt(SnapChn, 1);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_VENC_SetMaxStreamCnt(%d) err 0x%x\n", SnapChn, s32Ret);
        return HI_FAILURE;
    }
    
    return HI_SUCCESS;
}

HI_S32 Payload2FilePostfix(PAYLOAD_TYPE_E enPayload, HI_U8* szFilePostfix)
{
    if (PT_H264 == enPayload)
    {
        strcpy(szFilePostfix, ".h264");
    }
    else if (PT_JPEG == enPayload)
    {
        strcpy(szFilePostfix, ".jpg");
    }
    else if (PT_MJPEG == enPayload)
    {
        strcpy(szFilePostfix, ".mjp");
    }
    else
    {
        printf("payload type err!\n");
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

/* save jpeg stream */
HI_S32 SampleSaveJpegStream(FILE* fpJpegFile, VENC_STREAM_S *pstStream)
{
    VENC_PACK_S*  pstData;
    HI_U32 i;

    fwrite(g_SOI, 1, sizeof(g_SOI), fpJpegFile);

    for (i = 0; i < pstStream->u32PackCount; i++)
    {
        pstData = &pstStream->pstPack[i];
        fwrite(pstData->pu8Addr[0], pstData->u32Len[0], 1, fpJpegFile);
        fwrite(pstData->pu8Addr[1], pstData->u32Len[1], 1, fpJpegFile);
    }

    fwrite(g_EOI, 1, sizeof(g_EOI), fpJpegFile);

    return HI_SUCCESS;
}

/* save h264 stream */
HI_S32 SampleSaveH264Stream(FILE* fpH264File, VENC_STREAM_S *pstStream)
{
    HI_S32 i;

    for (i = 0; i < pstStream->u32PackCount; i++)
    {
        fwrite(pstStream->pstPack[i].pu8Addr[0],
               pstStream->pstPack[i].u32Len[0], 1, fpH264File);

        fflush(fpH264File);

        if (pstStream->pstPack[i].u32Len[1] > 0)
        {
            fwrite(pstStream->pstPack[i].pu8Addr[1],
                   pstStream->pstPack[i].u32Len[1], 1, fpH264File);

            fflush(fpH264File);
        }
    }

    return HI_SUCCESS;
}

HI_S32 SampleSaveVencStream(PAYLOAD_TYPE_E enType,FILE *pFd, VENC_STREAM_S *pstStream)
{
    HI_S32 s32Ret;

    if (PT_H264 == enType)
    {
        s32Ret = SampleSaveH264Stream(pFd, pstStream);
    }
    else if (PT_MJPEG == enType)
    {
        s32Ret = SampleSaveJpegStream(pFd, pstStream);
    }
    else
    {
        return HI_FAILURE;
    }
    return s32Ret;
}

/* get stream from each channels and save them..
 * only for H264/MJPEG, not for JPEG.  */
HI_VOID* SampleGetVencStreamProc(HI_VOID *p)
{
    HI_S32 i;
    HI_S32 s32ChnTotal;
    PAYLOAD_TYPE_E enPayload;
    VENC_CHN VeChnStart;
    GET_STREAM_S* pstGet;
    HI_S32 maxfd = 0;
    struct timeval TimeoutVal;
    fd_set read_fds;
    HI_S32 VencFd[VENC_MAX_CHN_NUM];
    HI_CHAR aszFileName[VENC_MAX_CHN_NUM][64];
    FILE *pFile[VENC_MAX_CHN_NUM];
    HI_U8 szFilePostfix[10];
    VENC_CHN_STAT_S stStat;
    VENC_STREAM_S stStream;
    HI_S32 s32ret;
    
    pstGet = (GET_STREAM_S*)p;
    VeChnStart = pstGet->VeChnStart;
    s32ChnTotal = pstGet->s32ChnTotal;
    enPayload = pstGet->enPayload;

    printf("VeChnStart:%d, s32ChnTotal:%d\n",VeChnStart,s32ChnTotal);

    /* Prepare for all channel. */
    Payload2FilePostfix(enPayload, szFilePostfix);

    for (i = VeChnStart; i < VeChnStart + s32ChnTotal; i++)
    {
        /* decide the stream file name, and open file to save stream */
        sprintf(aszFileName[i], "stream_%s%d%s", "chn", i, szFilePostfix);
        pFile[i] = fopen(aszFileName[i], "wb");
        if (!pFile[i])
        {
            printf("open file err!\n");
            return NULL;
        }
        VencFd[i] = HI_MPI_VENC_GetFd(i);
        if (VencFd[i] <= 0)
        {
            return NULL;
        }
        if (maxfd <= VencFd[i])
        {
            maxfd = VencFd[i];
        }
    }

    /* Start to get streams of each channel. */
    while (HI_TRUE == pstGet->bThreadStart)
    {
        FD_ZERO(&read_fds);
        for (i = VeChnStart; i < VeChnStart + s32ChnTotal; i++)
        {
            FD_SET(VencFd[i], &read_fds);
        }

        TimeoutVal.tv_sec  = 2;
        TimeoutVal.tv_usec = 0;
        s32ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32ret < 0)
        {
            printf("select err\n");
            break;
        }
        else if (s32ret == 0)
        {
            printf("get venc stream time out, quit thread\n");
            break;
        }
        else
        {
            for (i = VeChnStart; i < VeChnStart + s32ChnTotal; i++)
            {
                if (FD_ISSET(VencFd[i], &read_fds))
                {
                    /* step 1: query how many packs in one-frame stream. */
                    memset(&stStream, 0, sizeof(stStream));
                    s32ret = HI_MPI_VENC_Query(i, &stStat);
                    if (s32ret != HI_SUCCESS)
                    {
                        printf("HI_MPI_VENC_Query:0x%x, chn:%d\n", s32ret, i);
                        pstGet->bThreadStart = HI_FALSE;
                        return NULL;
                    }

                    /* step 2: malloc corresponding number of pack nodes. */
                    stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
                    if (NULL == stStream.pstPack)
                    {
                        printf("malloc stream pack err!\n");
                        pstGet->bThreadStart = HI_FALSE;
                        return NULL;
                    }

                    /* step 3: call mpi to get one-frame stream. */
                    stStream.u32PackCount = stStat.u32CurPacks;
                    s32ret = HI_MPI_VENC_GetStream(i, &stStream, HI_IO_BLOCK);
                    if (s32ret != HI_SUCCESS)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        printf("HI_MPI_VENC_GetStream err 0x%x\n", s32ret);
                        pstGet->bThreadStart = HI_FALSE;
                        return NULL;
                    }

                    /* step 4: save stream. */ 
                    SampleSaveVencStream(pstGet->enPayload, pFile[i], &stStream);

                    /* step 5: release these stream */
                    s32ret = HI_MPI_VENC_ReleaseStream(i, &stStream);
                    if (s32ret != HI_SUCCESS)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        pstGet->bThreadStart = HI_FALSE;
                        return NULL;
                    }

                    /* step 6: free pack nodes */
                    free(stStream.pstPack);
                    stStream.pstPack = NULL;
                }
            }
        }
    };

    for (i = VeChnStart; i < VeChnStart + s32ChnTotal; i++)
    {
        fclose(pFile[i]);
    }
    pstGet->bThreadStart = HI_FALSE;
    return NULL;
}

HI_VOID* SampleGetVencStreamProc_ls(HI_VOID *p)
{
    HI_S32 i;
    HI_S32 s32ChnTotal;
    PAYLOAD_TYPE_E enPayload;
    VENC_CHN VeChnStart;
    GET_STREAM_S* pstGet;
    HI_S32 maxfd = 0;
    struct timeval TimeoutVal;
    fd_set read_fds;
    HI_S32 VencFd[VENC_MAX_CHN_NUM];
    HI_U8 szFilePostfix[10];
    VENC_CHN_STAT_S stStat;
    VENC_STREAM_S stStream;
    HI_S32 s32ret;
    int mailChannel;
    
    pstGet         = (GET_STREAM_S*)p;
    VeChnStart     = pstGet->VeChnStart;
    s32ChnTotal = pstGet->s32ChnTotal;
    enPayload     = pstGet->enPayload;

    printf("VeChnStart:%d, s32ChnTotal:%d\n",VeChnStart,s32ChnTotal);

    /* Prepare for all channel. */
    Payload2FilePostfix(enPayload, szFilePostfix);

    for (i = VeChnStart; i < VeChnStart + s32ChnTotal; i++)
    {        
    	mailChannel = i << 1;
        /* decide the stream file name, and open file to save stream */
        
        VencFd[i] = HI_MPI_VENC_GetFd(mailChannel);
        if (VencFd[i] <= 0)
        {
            return NULL;
        }
        if (maxfd <= VencFd[i])
        {
            maxfd = VencFd[i];
        }
    }

    /* Start to get streams of each channel. */
    while (HI_TRUE == pstGet->bThreadStart)
    {
        FD_ZERO(&read_fds);
        for (i = VeChnStart; i < VeChnStart + s32ChnTotal; i++)
        {
            FD_SET(VencFd[i], &read_fds);
        }

        TimeoutVal.tv_sec  = 2;
        TimeoutVal.tv_usec = 0;
        s32ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32ret < 0)
        {
            printf("select err\n");
            break;
        }
        else if (s32ret == 0)
        {
            printf("get venc stream time out, quit thread\n");
            break;
        }
        else
        {
            for (i = VeChnStart; i < VeChnStart + s32ChnTotal; i++)
            {
            	mailChannel = i << 1;
                if (FD_ISSET(VencFd[i], &read_fds))
                {
                    /* step 1: query how many packs in one-frame stream. */
                    memset(&stStream, 0, sizeof(stStream));
                    s32ret = HI_MPI_VENC_Query(mailChannel, &stStat);
                    if (s32ret != HI_SUCCESS)
                    {
                        printf("HI_MPI_VENC_Query:0x%x, chn:%d\n", s32ret, i);
                        pstGet->bThreadStart = HI_FALSE;
                        return NULL;
                    }

                    /* step 2: malloc corresponding number of pack nodes. */
                    stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
                    if (NULL == stStream.pstPack)
                    {
                        printf("malloc stream pack err!\n");
                        pstGet->bThreadStart = HI_FALSE;
                        return NULL;
                    }

                    /* step 3: call mpi to get one-frame stream. */
                    stStream.u32PackCount = stStat.u32CurPacks;
                    s32ret = HI_MPI_VENC_GetStream(mailChannel, &stStream, HI_IO_BLOCK);
                    if (s32ret != HI_SUCCESS)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        printf("HI_MPI_VENC_GetStream err 0x%x\n", s32ret);
                        pstGet->bThreadStart = HI_FALSE;
                        return NULL;
                    }

                    /* step 4: save stream. */
                   // SampleSaveVencStream(pstGet->enPayload, pFile[i], &stStream);
                	H264FromHisiAddrToMyAddr( i, &stStream);

                    /* step 5: release these stream */
                    s32ret = HI_MPI_VENC_ReleaseStream(mailChannel, &stStream);
                    if (s32ret != HI_SUCCESS)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        pstGet->bThreadStart = HI_FALSE;
                        return NULL;
                    }

                    /* step 6: free pack nodes */
                    free(stStream.pstPack);
                    stStream.pstPack = NULL;
                }
            } 
        }
    }
    
    pstGet->bThreadStart = HI_FALSE;
    return NULL;
}
#endif

//获取音视频流
//这里只是获取HISI芯片出来的音视频。
//对于网络传输过来的音视频，可以通过socket的select来进行音视频的
//获取
#define vedioENC 1
HI_VOID* mpi_comm_get_stream_proc(HI_VOID *p)
{
    HI_S32 i;
    HI_S32 s32ChnTotal;
    //PAYLOAD_TYPE_E enPayload;
    VENC_CHN VeChnStart;
    GET_STREAM_S* pstGet;
    HI_S32 maxfd = 0;
    struct timeval TimeoutVal;
    fd_set read_fds;
    HI_S32 VencFd[VENC_MAX_CHN_NUM];
    VENC_CHN_STAT_S stStat;
    VENC_STREAM_S stStream;
    HI_S32 s32ret;
    int mailChannel;


    HI_S32 AencFd[MAX_AENC_CHN_NUM];
    AUDIO_STREAM_S stAudioStream;
    
    pstGet         = (GET_STREAM_S*)p;
    VeChnStart     = pstGet->VeChnStart;
    s32ChnTotal    = pstGet->s32ChnTotal;
    //enPayload      = pstGet->enPayload;
    
#if vedioENC
    for (i = VeChnStart; i < VeChnStart + s32ChnTotal; i++)
    {        
    	mailChannel = i << 1;
        
        VencFd[i] = HI_MPI_VENC_GetFd(mailChannel);
        if (VencFd[i] <= 0)
        {
            return NULL;
        }
        
        if (maxfd <= VencFd[i])
        {
            maxfd = VencFd[i];
        }
    }
#endif

#ifdef AUDIOENC
    if(pstGet->bAudioEnable)
    {
        for(i =0;i < MAX_AENC_CHN_NUM;i++)
        {
            AencFd[i] = HI_MPI_AENC_GetFd(i);
            if(AencFd[i] <= 0)
            {
                ERRORPRINT("get AENC chn(%d) FD error\n",i);
                return NULL;
            }
            
            if(maxfd <= AencFd[i])
            {
                maxfd = AencFd[i];
            }
        }
    }
    
#endif

    CORRECTPRINT("########thread mpi_comm_get_stream_proc start;VeChnStart:%d, s32ChnTotal:%d########\n",VeChnStart,s32ChnTotal);
    /* Start to get streams of each channel. */
    while (HI_TRUE == pstGet->bThreadStart)
    {
        FD_ZERO(&read_fds);
        
#if vedioENC        
        for (i = VeChnStart; i < VeChnStart + s32ChnTotal; i++)
        {
            FD_SET(VencFd[i], &read_fds);
        }
#endif

#ifdef AUDIOENC
        if(pstGet->bAudioEnable)
        {
            for(i = 0;i<MAX_AENC_CHN_NUM;i++)
            {
                FD_SET(AencFd[i], &read_fds);
            }
        }
#endif
        TimeoutVal.tv_sec  = 2;
        TimeoutVal.tv_usec = 0;
        s32ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32ret < 0)
        {
            ERRORPRINT("select err\n");
            break;
        }
        else if (s32ret == 0)
        {
            ERRORPRINT("get venc stream time out, continue\n");
            continue;
        }
        else
        {
            for (i = VeChnStart; i < VeChnStart + s32ChnTotal; i++)
            {
#ifdef AUDIOENC
                if((pstGet->bAudioEnableFlag[i]))
                {
                    //continue;
                    if(FD_ISSET(AencFd[i],&read_fds))
                    {
                        memset(&stAudioStream,0,sizeof(stAudioStream));
                        s32ret = HI_MPI_AENC_GetStream(i, &stAudioStream, HI_FALSE);
                        if (HI_SUCCESS != s32ret )
                        {
                            ERRORPRINT("%s: HI_MPI_AENC_GetStream(%d), failed with %#x!\n",\
                                   __FUNCTION__, i, s32ret);
                            pstGet->bThreadStart = HI_FALSE;
                            return NULL;
                        }
    //if(2 == i||3 == i)                    
    //printf("we get chn %d audio frame!\n",i);
                        s32ret = AudioFromHisiAddrToMyAddr(i,&stAudioStream);
                        if(s32ret != HI_SUCCESS)
                        {
                            ERRORPRINT("AudioFromHisiAddrToMyAddr error\n");
                            pstGet->bThreadStart = HI_FALSE;
                            return NULL;
                        }

                        s32ret = HI_MPI_AENC_ReleaseStream(i, &stAudioStream);
                        if (s32ret != HI_SUCCESS)
                        {
                            ERRORPRINT("HI_MPI_AENC_ReleaseStream error \n");
                            pstGet->bThreadStart = HI_FALSE;
                            return NULL;
                        }
                    }
                }
                
                
#endif

#if vedioENC            
            	mailChannel = i << 1;
                if (FD_ISSET(VencFd[i], &read_fds))
                {
                    /* step 1: query how many packs in one-frame stream. */
                    memset(&stStream, 0, sizeof(stStream));
                    s32ret = HI_MPI_VENC_Query(mailChannel, &stStat);
                    if (s32ret != HI_SUCCESS)
                    {
                        ERRORPRINT("HI_MPI_VENC_Query:0x%x, chn:%d\n", s32ret, i);
                        pstGet->bThreadStart = HI_FALSE;
                        return NULL;
                    }

                    /* step 2: malloc corresponding number of pack nodes. */
                    stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
                    if (NULL == stStream.pstPack)
                    {
                        printf("malloc stream pack err!\n");
                        pstGet->bThreadStart = HI_FALSE;
                        return NULL;
                    }

                    /* step 3: call mpi to get one-frame stream. */
                    stStream.u32PackCount = stStat.u32CurPacks;
                    s32ret = HI_MPI_VENC_GetStream(mailChannel, &stStream, HI_IO_BLOCK);
                    if (s32ret != HI_SUCCESS)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        ERRORPRINT("HI_MPI_VENC_GetStream err 0x%x\n", s32ret);
                        pstGet->bThreadStart = HI_FALSE;
                        return NULL;
                    }
//printf("we get a video frame!\n");
                    /* step 4: save stream. */
                	s32ret = H264FromHisiAddrToMyAddr( i, &stStream);
                    if(s32ret != 0)
                    {
                        ERRORPRINT("H264FromHisiAddrToMyAddr return:%d\n",s32ret);
                    }

                    /* step 5: release these stream */
                    s32ret = HI_MPI_VENC_ReleaseStream(mailChannel, &stStream);
                    if (s32ret != HI_SUCCESS)
                    {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        pstGet->bThreadStart = HI_FALSE;
                        return NULL;
                    }

                    /* step 6: free pack nodes */
                    free(stStream.pstPack);
                    stStream.pstPack = NULL;
                }
#endif 
            }    
        }
    } 
    
    pstGet->bThreadStart = HI_FALSE;
    ERRORPRINT("!!!!!!!!thread mpi_comm_get_stream_proc quit!!!!!!!!\n");
    return NULL;
}

static GET_STREAM_S s_stGetVeStream;
HI_S32 mpi_comm_start_get_venc_stream_3515a(GET_STREAM_S *pstGetVeStream)
{
    memcpy(&s_stGetVeStream, pstGetVeStream, sizeof(GET_STREAM_S));

    s_stGetVeStream.bThreadStart = HI_TRUE;
    return pthread_create(&s_stGetVeStream.pid, 0, mpi_comm_get_stream_proc, (HI_VOID*)&s_stGetVeStream);
}

HI_S32 mpi_comm_stop_get_stream()
{
    if (HI_TRUE == s_stGetVeStream.bThreadStart)
    {
        s_stGetVeStream.bThreadStart = HI_FALSE;
        pthread_join(s_stGetVeStream.pid, 0);
    }
    return HI_SUCCESS;
}

HI_VOID HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo)
    {
        mpi_ExitMPP();
        printf("MPP quit\n");
    }
    exit(0);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif 

