/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**	文件名: osdVi.cpp
**  description  : 封装海思的VI OSD接口
**  date           :  2013.10.18
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/

#if defined MCU_HI3515A

#include <string.h>
#include <unistd.h>
#include "const.h"
#include "debug.h"
#include "malloc.h"

#include "hifb.h"
#include "loadbmp.h"
#include "hi_common.h"
#if defined MCU_HI3515
#include "hi_comm_vpp.h"
#endif
#if defined MCU_HI3515A
#include "hi_comm_vpss.h"
#endif
#if defined MCU_HI3515
#include "mpi_vpp.h"
#endif
#if defined MCU_HI3515A
#include "mpi_vpss.h"
#include "mpi_region.h"
#endif
#include "osdComm.h"
#include "osdVi.h"

extern unsigned    char    *g_asc8x16Buf;
extern unsigned    char    *g_ch16x16Buf;

#define unParamBufMax 120
static OSD_VI_PARAM_BUF unParamBuf[unParamBufMax];

static int ViDrawAsc(REGION_HANDLE handle, int color, int x, int y, unsigned char asc,
            	int font_size,int lace);
static int ViDrawChinese(REGION_HANDLE handle,  int color, int x, int y,unsigned char qu, unsigned char wei, 
            	int font_size,int lace);
static int ViBlacLace(unsigned char* lacemem,unsigned short* video, int font_width);


/*初始化OSD库 */
int FiOsdViInitOsdLib(void)
{
	int i;
	for(i = 0; i < unParamBufMax; i++)
    {
    	unParamBuf[i].bufHandle = 0;
    	unParamBuf[i].bufWidth = 0;
    	unParamBuf[i].bufHeight = 0;        
    	unParamBuf[i].bufData = NULL;
    }
	return 0;    
}

/*解初始化OSD库 */
int FiOsdViDeinitOsdLib(void)
{
	return 0;
}

int SVOsdViDeinitOsdLib(void)
{
	return 0;
}
/*
* fn: 从透明度转化成alpha
* bgTransparence: 透明度, // 背景透明度 0(不透明)~100(100%透明),标示0%~100%
* 返回 0(全透明)~128(不透明)
*/
static uint TransparenceToAlpha( uchar transparence )
{
	uint val;
	if( transparence > 100 )
    {
    	return 0;
    }    
	else
    {
    	val = 128 - 128 * transparence / 100;
    }

	return val;
}

#if defined MCU_HI3515
/*初始化指定区域 vegroup =ff 为公共区域*/
int FiOsdViInitOsd( VENC_GRP VeGroup,OSD_RECT rc, unsigned int color,  
                	uchar bgTransparence, REGION_HANDLE *handle )
{
	int ret=0;
	int i;
	REGION_ATTR_S stRgnAttr;    
	OSD_RECT rc_tmp;

	rc_tmp = rc;

	if((rc.x < 0)||(rc.y < 0)||(rc.width < 0)||(rc.height < 0))
    {
    	SVPrint("error!rc.x rc.y rc.width rc.height must be bigger than zero!\n");
    	return -1;    
    }    
    
	if(rc.x % OSD_ALIGN_X !=0)
    {
    	rc_tmp.x = rc.x + (OSD_ALIGN_X - rc.x % OSD_ALIGN_X);    
    }
	if(rc.y %OSD_ALIGN !=0)
    {
    	rc_tmp.y = rc.y + (OSD_ALIGN - rc.y % OSD_ALIGN);
    }
	if(rc.width % OSD_ALIGN != 0)
    {
    	rc_tmp.width = ++rc.width;        
    }      
	if(rc.height % OSD_ALIGN != 0)
    {
    	rc_tmp.height = ++rc.height;        
    }      

	stRgnAttr.enType = OVERLAY_REGION;
	if(VeGroup == 0xff)
    	stRgnAttr.unAttr.stOverlay.bPublic = (HI_BOOL)1;    
	else
    	stRgnAttr.unAttr.stOverlay.bPublic = (HI_BOOL)0;        
	stRgnAttr.unAttr.stOverlay.enPixelFmt = PIXEL_FORMAT_RGB_1555;
	stRgnAttr.unAttr.stOverlay.stRect.s32X = rc_tmp.x;
	stRgnAttr.unAttr.stOverlay.stRect.s32Y = rc_tmp.y;
	stRgnAttr.unAttr.stOverlay.stRect.u32Width = rc_tmp.width; 
	stRgnAttr.unAttr.stOverlay.stRect.u32Height = rc_tmp.height;
	SVPrint( "x(%d), y(%d), width(%d), height(%d)!\r\n", rc_tmp.x, rc_tmp.y, rc_tmp.width, rc_tmp.height );
	stRgnAttr.unAttr.stOverlay.u32BgAlpha = TransparenceToAlpha( bgTransparence );//30;  // [0,128] = [全透明,不透明]
	stRgnAttr.unAttr.stOverlay.u32FgAlpha = 128;     // [0,128] = [全透明,不透明]
	stRgnAttr.unAttr.stOverlay.u32BgColor = color&BGALPHAMASK;
	stRgnAttr.unAttr.stOverlay.VeGroup = VeGroup;
    
	ret = HI_MPI_VPP_CreateRegion(&stRgnAttr, handle);
	if(ret != 0)
    {
    	SVPrint("HI_MPI_VPP_CreateRegion err 0x%x!\n",ret);
    	SVPrint("VeGroup = %d,*handle = %d\n", VeGroup, *handle);        
    	return -1;
    }    
	for(i = 0; i < unParamBufMax; i++)
    {
    	if(unParamBuf[i].bufData == NULL)
        {
        	unParamBuf[i].bufHandle = *handle;
        	unParamBuf[i].bufWidth = rc_tmp.width;
        	unParamBuf[i].bufHeight = rc_tmp.height;
        	unParamBuf[i].bufData = (char *)Malloc(sizeof(short)*unParamBuf[i].bufWidth*unParamBuf[i].bufHeight+ 1);
        	if(NULL == unParamBuf[i].bufData)
            {
            	SVPrint("not enough memory to Malloc!\n");
            	ret = HI_MPI_VPP_DestroyRegion(*handle);
            	if(ret != 0)
                {
                	SVPrint("HI_MPI_VPP_DestroyRegion err 0x%x!\n",ret);
                	return -1;
                }                
               	return -1;
            }  
        	memset(unParamBuf[i].bufData,0,sizeof(short)*rc_tmp.height*rc_tmp.width+ 1);
        	return 0;
        }
    }
    
	return -1;
}
#endif

#if defined MCU_HI3515A
int SVOsdViInitOsd( VI_CHN VeGroup,OSD_RECT rc, unsigned int color,  
            	RGN_HANDLE *handle ,eOSDTYPE eosdtype)
{
	int ret=0;
	int i;
    RGN_ATTR_S stRegion;
    RGN_HANDLE temp_handle = (MAX_OSD_NUM*VeGroup)+ eosdtype+1;
	OSD_RECT rc_tmp;

	rc_tmp = rc;
    if(NULL == handle)
    {
        return -1;
    }

	if((rc.x < 0)||(rc.y < 0)||(rc.width < 0)||(rc.height < 0))
    {
    	SVPrint("error!rc.x rc.y rc.width rc.height must be bigger than zero!\n");
    	return -1;    
    }    
    
	if(rc.x % OSD_ALIGN_X !=0)
    {
    	rc_tmp.x = rc.x + (OSD_ALIGN_X - rc.x % OSD_ALIGN_X);    
    }
	if(rc.y %OSD_ALIGN !=0)
    {
    	rc_tmp.y = rc.y + (OSD_ALIGN - rc.y % OSD_ALIGN);
    }
	if(rc.width % OSD_ALIGN != 0)
    {
    	rc_tmp.width = ++rc.width;        
    }      
	if(rc.height % OSD_ALIGN != 0)
    {
    	rc_tmp.height = ++rc.height;        
    }      

    stRegion.enType = OVERLAYEX_RGN;
	stRegion.unAttr.stOverlay.enPixelFmt = PIXEL_FORMAT_RGB_1555;
	stRegion.unAttr.stOverlay.stSize.u32Width = rc_tmp.width; 
	stRegion.unAttr.stOverlay.stSize.u32Height = rc_tmp.height;
	SVPrint( "x(%d), y(%d), width(%d), height(%d)!\r\n", rc_tmp.x, rc_tmp.y, rc_tmp.width, rc_tmp.height );
	stRegion.unAttr.stOverlay.u32BgColor = color&BGALPHAMASK;
    
	ret = HI_MPI_RGN_Create(temp_handle,&stRegion);
	if(ret != 0)
    {
    	ERRORPRINT("HI_MPI_RGN_Create err 0x%x!\n",ret);
    	ERRORPRINT("VeGroup = %d,*handle = %d\n", VeGroup, temp_handle);        
    	return -1;
    }


    
	for(i = 0; i < unParamBufMax; i++)
    {
    	if(unParamBuf[i].bufData == NULL)
        {
        	unParamBuf[i].bufHandle = temp_handle;
        	unParamBuf[i].bufWidth = rc_tmp.width;
        	unParamBuf[i].bufHeight = rc_tmp.height;
        	unParamBuf[i].bufData = (char *)Malloc(sizeof(short)*unParamBuf[i].bufWidth*unParamBuf[i].bufHeight+ 1);
        	if(NULL == unParamBuf[i].bufData)
            {
            	ERRORPRINT("not enough memory to Malloc!\n");
            	ret = HI_MPI_RGN_Destroy(temp_handle);
            	if(ret != 0)
                {
                	ERRORPRINT("HI_MPI_VPP_DestroyRegion err 0x%x!\n",ret);
                	return -1;
                }                
               	return -1;
            }  
        	memset(unParamBuf[i].bufData,0,sizeof(short)*rc_tmp.height*rc_tmp.width+ 1);
            *handle = temp_handle;
        	return 0;
        }
    }
    
	return -1;
}
#endif
#if defined MCU_HI3515
int FiOsdViInitOsdBgClrEx( VENC_GRP VeGroup,OSD_RECT rc, unsigned int color,  
                        	uchar bgTransparence, REGION_HANDLE *handle )
{
	int ret=0;
	int i;
	REGION_ATTR_S stRgnAttr;    
	OSD_RECT rc_tmp;

	rc_tmp = rc;
	if((rc.x < 0)||(rc.y < 0)||(rc.width < 0)||(rc.height < 0))
    {
    	SVPrint("error!rc.x rc.y rc.width rc.height must be bigger than zero!\n");
    	return -1;    
    }    

	if(rc.x % OSD_ALIGN_X !=0)
    {
    	rc_tmp.x = rc.x + (OSD_ALIGN_X - rc.x % OSD_ALIGN_X);    
    }
	if(rc.y % OSD_ALIGN !=0)
    {
    	rc_tmp.y = rc.y + (OSD_ALIGN - rc.y % OSD_ALIGN);
    }
	if(rc.width % OSD_ALIGN != 0)
    {
    	rc_tmp.width = ++rc.width;        
    }      
	if(rc.height % OSD_ALIGN != 0)
    {
    	rc_tmp.height = ++rc.height;        
    }      

	stRgnAttr.enType = OVERLAY_REGION;
	if(VeGroup == 0xff)
    	stRgnAttr.unAttr.stOverlay.bPublic = (HI_BOOL)1;    
	else
    	stRgnAttr.unAttr.stOverlay.bPublic = (HI_BOOL)0;        
	stRgnAttr.unAttr.stOverlay.enPixelFmt = PIXEL_FORMAT_RGB_1555;
	stRgnAttr.unAttr.stOverlay.stRect.s32X= rc_tmp.x;
	stRgnAttr.unAttr.stOverlay.stRect.s32Y= rc_tmp.y;
	stRgnAttr.unAttr.stOverlay.stRect.u32Width = rc_tmp.width; 
	stRgnAttr.unAttr.stOverlay.stRect.u32Height = rc_tmp.height;
	stRgnAttr.unAttr.stOverlay.u32BgAlpha = TransparenceToAlpha( bgTransparence );        // [0,128] = [全透明,不透明]
	stRgnAttr.unAttr.stOverlay.u32FgAlpha = 128;    // [0,128] = [全透明,不透明]
	stRgnAttr.unAttr.stOverlay.VeGroup = VeGroup;
    
	ret = HI_MPI_VPP_CreateRegion(&stRgnAttr, handle);
	if(0 != ret)
    {
    	SVPrint("HI_MPI_VPP_CreateRegion err 0x%x!\n",ret);
    	SVPrint("VeGroup = %d,*handle = %d\n", VeGroup, *handle);        
    	return -1;
    }    
	for(i = 0; i < unParamBufMax; i++)
    {
    	if(unParamBuf[i].bufData == NULL)
        {
        	unParamBuf[i].bufHandle = *handle;
        	unParamBuf[i].bufWidth = rc_tmp.width;
        	unParamBuf[i].bufHeight = rc_tmp.height;
        	unParamBuf[i].bufData = (char *)Malloc(sizeof(short)*unParamBuf[i].bufWidth*unParamBuf[i].bufHeight+ 1);
        	if(NULL == unParamBuf[i].bufData)
            {
            	SVPrint("not enough memory to Malloc!\n");
            	ret = HI_MPI_VPP_DestroyRegion(*handle);
            	if(0 != ret)
                {
                	SVPrint("HI_MPI_VPP_DestroyRegion err 0x%x!\n",ret);
                	return -1;
                }
               	return -1;
            }  
        	memset(unParamBuf[i].bufData,0x0,sizeof(short)*rc_tmp.height*rc_tmp.width+ 1);
        	unsigned short *p = (unsigned short *)unParamBuf[i].bufData;
        	for(i=0; i<rc_tmp.height*rc_tmp.width+ 1; i++)
                *p++ = (~color)&BGALPHAMASK;
        	return 0;
        }
    }
    
	return -1;
}
#endif

int SVOsdViInitOsdBgClrEx( VI_CHN VeGroup,OSD_RECT rc, unsigned int color,  
            	RGN_HANDLE *handle ,eOSDTYPE eosdtype)
{
	int ret=0;
	int i;
	RGN_ATTR_S stRegion;
    RGN_HANDLE temp_handle = (MAX_OSD_NUM*VeGroup)+ eosdtype+1;
	OSD_RECT rc_tmp;

	rc_tmp = rc;
    if(NULL == handle)
    {
        return -1;
    }

	if((rc.x < 0)||(rc.y < 0)||(rc.width < 0)||(rc.height < 0))
    {
    	SVPrint("error!rc.x rc.y rc.width rc.height must be bigger than zero!\n");
    	return -1;    
    }    
    
	if(rc.x % OSD_ALIGN_X !=0)
    {
    	rc_tmp.x = rc.x + (OSD_ALIGN_X - rc.x % OSD_ALIGN_X);    
    }
	if(rc.y %OSD_ALIGN !=0)
    {
    	rc_tmp.y = rc.y + (OSD_ALIGN - rc.y % OSD_ALIGN);
    }
	if(rc.width % OSD_ALIGN != 0)
    {
    	rc_tmp.width = ++rc.width;        
    }      
	if(rc.height % OSD_ALIGN != 0)
    {
    	rc_tmp.height = ++rc.height;        
    }      

    stRegion.enType = OVERLAYEX_RGN;
	stRegion.unAttr.stOverlay.enPixelFmt = PIXEL_FORMAT_RGB_1555;
	stRegion.unAttr.stOverlay.stSize.u32Width = rc_tmp.width; 
	stRegion.unAttr.stOverlay.stSize.u32Height = rc_tmp.height;
	SVPrint( "x(%d), y(%d), width(%d), height(%d)!\r\n", rc_tmp.x, rc_tmp.y, rc_tmp.width, rc_tmp.height );
    //stRegion.unAttr.stOverlay.u32BgColor = color&BGALPHAMASK;
    
	ret = HI_MPI_RGN_Create(temp_handle,&stRegion);
	if(ret != 0)
    {
    	ERRORPRINT("HI_MPI_RGN_Create err 0x%x!\n",ret);
    	ERRORPRINT("VeGroup = %d,*handle = %d\n", VeGroup, temp_handle);        
    	return -1;
    }

    
	for(i = 0; i < unParamBufMax; i++)
    {
    	if(unParamBuf[i].bufData == NULL)
        {
        	unParamBuf[i].bufHandle = temp_handle;
        	unParamBuf[i].bufWidth = rc_tmp.width;
        	unParamBuf[i].bufHeight = rc_tmp.height;
        	unParamBuf[i].bufData = (char *)Malloc(sizeof(short)*unParamBuf[i].bufWidth*unParamBuf[i].bufHeight+ 1);
        	if(NULL == unParamBuf[i].bufData)
            {
            	ERRORPRINT("not enough memory to Malloc!\n");
            	ret = HI_MPI_RGN_Destroy(temp_handle);
            	if(ret != 0)
                {
                	ERRORPRINT("HI_MPI_VPP_DestroyRegion err 0x%x!\n",ret);
                	return -1;
                }                
               	return -1;
            }  
        	memset(unParamBuf[i].bufData,0,sizeof(short)*rc_tmp.height*rc_tmp.width+ 1);
            unsigned short *p = (unsigned short *)unParamBuf[i].bufData;
        	for(i=0; i<rc_tmp.height*rc_tmp.width+ 1; i++)
                *p++ = (~color)&BGALPHAMASK;
            *handle = temp_handle;
        	return 0;
        }
    }
    
	return -1;
}

#if defined MCU_HI3515
/*销毁指定OSD. */
int FiOsdViDestroyOsd(REGION_HANDLE handle)
{
	int ret = FI_FAILED;
	int i;     

	for(i = 0; i < unParamBufMax; i++)
    {
    	if((unParamBuf[i].bufHandle == handle)&&(unParamBuf[i].bufData != NULL))
        {
        	unParamBuf[i].bufHandle = 0;
        	unParamBuf[i].bufWidth = 0;
        	unParamBuf[i].bufHeight = 0;
        	Free(unParamBuf[i].bufData);
        	unParamBuf[i].bufData = NULL;            
        }
    }    
	ret = HI_MPI_VPP_DestroyRegion(handle);
	if(ret != 0)
    {
    	SVPrint("HI_MPI_VPP_DestroyRegion,handle=%d,err:0x%x!\n",handle,ret);
    	ret = FI_FAILED;
    }
    
	return ret;
}
#endif

int SVOsdViDestroyOsd(REGION_HANDLE handle)
{
	int ret = FI_FAILED;
	int i;     

	for(i = 0; i < unParamBufMax; i++)
    {
    	if((unParamBuf[i].bufHandle == handle)&&(unParamBuf[i].bufData != NULL))
        {
        	unParamBuf[i].bufHandle = 0;
        	unParamBuf[i].bufWidth = 0;
        	unParamBuf[i].bufHeight = 0;
        	Free(unParamBuf[i].bufData);
        	unParamBuf[i].bufData = NULL;            
        }
    }    
	ret = HI_MPI_RGN_Destroy(handle);
	if(ret != 0)
    {
    	SVPrint("HI_MPI_RGN_Destroy,handle=%d,err:0x%x!\n",handle,ret);
    	ret = FI_FAILED;
    }
    
	return ret;
}

/*获得初始化VPP的句柄*/
static int GetBufIndex(REGION_HANDLE handle,int *index)
{
	int i;
    
	for(i = 0; i < unParamBufMax; i++)
    {
    	if((unParamBuf[i].bufHandle == handle)&&(unParamBuf[i].bufData != NULL))
        {
            *index = i;            
        	return 0;
        }
    }
	return -1;    
}

/*设置当前叠加层显示还是隐藏.
bshow :  1，显示 0，隐藏 */
#if defined MCU_HI3515
int FiOsdViSetShow(REGION_HANDLE handle, int bshow)
{
	int ret = FI_FAILED;
	REGION_CRTL_CODE_E ctrshow;
	REGION_CTRL_PARAM_U unParam;    

	if( 1 == bshow)
    	ctrshow = REGION_SHOW;
	else if(0 == bshow)
    	ctrshow = REGION_HIDE;
	else
    {
    	SVPrint("failed:osd parameter bshow(%d) is wrong!\n",bshow);    
    	return -1;
    }
    
	ret = HI_MPI_VPP_ControlRegion(handle,ctrshow, &unParam);
	if(0 != ret)
    {
    	SVPrint("error:HI_MPI_VPP_ControlRegion ret=0x%x!\n",ret);
    	ret = FI_FAILED;
    }    
	return ret;
}
#endif

#if defined MCU_HI3515A
int SVOsdViSetShow(RGN_HANDLE handle, int bshow,OSD_RECT rc,VI_CHN ViChn,
uchar  bgTransparence,eOSDTYPE eosdtype)
{
	int ret = FI_FAILED;

    MPP_CHN_S stOverlayExChn;
    RGN_CHN_ATTR_S stOverlayExChnAttr;
    OSD_RECT rc_tmp = rc;

	if( 1 == bshow)
    	stOverlayExChnAttr.bShow = HI_TRUE;
	else if(0 == bshow)
    	stOverlayExChnAttr.bShow = HI_FALSE;
	else
    {
    	ERRORPRINT("failed:osd parameter bshow(%d) is wrong!\n",bshow);    
    	return -1;
    }
    

    if(rc.x % OSD_ALIGN_X !=0)
    {
    	rc_tmp.x = rc.x + (OSD_ALIGN_X - rc.x % OSD_ALIGN_X);    
    }
	if(rc.y %OSD_ALIGN !=0)
    {
    	rc_tmp.y = rc.y + (OSD_ALIGN - rc.y % OSD_ALIGN);
    }
	if(rc.width % OSD_ALIGN != 0)
    {
    	rc_tmp.width = ++rc.width;        
    }      
	if(rc.height % OSD_ALIGN != 0)
    {
    	rc_tmp.height = ++rc.height;        
    } 
    
    stOverlayExChnAttr.enType = OVERLAYEX_RGN;
	stOverlayExChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = rc_tmp.x;
	stOverlayExChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = rc_tmp.y;
	stOverlayExChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha = TransparenceToAlpha( bgTransparence );
	stOverlayExChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha = 128;
	stOverlayExChnAttr.unChnAttr.stOverlayExChn.u32Layer = eosdtype;//type

    stOverlayExChn.enModId = HI_ID_VIU;
    stOverlayExChn.s32DevId = 0;
    stOverlayExChn.s32ChnId = ViChn;

	ret = HI_MPI_RGN_AttachToChn(handle,&stOverlayExChn,&stOverlayExChnAttr);
    if(0 != ret)
    {
    	ERRORPRINT("error:HI_MPI_RGN_AttachToChn ret=0x%x!\n",ret);
    	ret = FI_FAILED;
    }
	return ret;
}
#endif
#if defined MCU_HI3515
/*写一个字符串.  */
int FiOsdViDrawString(REGION_HANDLE handle, int color, int x, int y, 
            	const char *string, int font_size, int lace)
{
	int i = 0;
	int bufIndex = 0;
	int ret = 0;
	REGION_CTRL_PARAM_U unParam;    
    
	memset(&unParam,0,sizeof(unParam));
    
	ret = GetBufIndex(handle,&bufIndex);
	if(ret != 0)
    {
    	SVPrint("failed:GetBufIndex,no buf init!\n");
    	return -1;
    }     
	while(*(string + i)  != 0)
    {
    	if((unsigned char)*(string + i) < 0x81)
        {
        	ViDrawAsc(handle,color,x,y,*(string + i), font_size, lace);
        	if(font_size == 0)
            	x += 8;                            
        }
    	else if((unsigned char)*(string + i + 1) >= 0x81)
        {
        	ViDrawChinese(handle,color,x,y,*(string + i),*(string + i + 1),font_size, lace);
        	if(font_size == 0)
            	x += 16;                            
        	i ++;         
        }
    	else
        {
        	SVPrint("read font from lib error!\n");
        }
    	i++;    
    }        
	unParam.stBitmap.u32Width = unParamBuf[bufIndex].bufWidth;
	unParam.stBitmap.u32Height = unParamBuf[bufIndex].bufHeight;
//	SVPrint("unParam.stBitmap.u32Width=%d,unParam.stBitmap.u32Height=%d\r\n",
//    	unParam.stBitmap.u32Width,unParam.stBitmap.u32Height);
	unParam.stBitmap.enPixelFormat = PIXEL_FORMAT_RGB_1555;
	unParam.stBitmap.pData = (void *)unParamBuf[bufIndex].bufData;    
	ret = HI_MPI_VPP_ControlRegion(handle, REGION_SET_BITMAP,&unParam);
	if(0 != ret)
    {
    	SVPrint("error:HI_MPI_VPP_ControlRegion ret=0x%X!\n",ret);
    	ret = FI_FAILED;
    }    
	return ret;
}

#endif

int SVOsdViDrawString(RGN_HANDLE handle, int color, int x, int y, 
            	const char *string, int font_size, int lace)
{
	int i = 0;
	int bufIndex = 0;
	int ret = 0;
    //REGION_CTRL_PARAM_U unParam;    

    BITMAP_S stBitmap;
	memset(&stBitmap,0,sizeof(stBitmap));
    
	ret = GetBufIndex(handle,&bufIndex);
	if(ret != 0)
    {
    	SVPrint("failed:GetBufIndex,no buf init!\n");
    	return -1;
    }     
	while(*(string + i)  != 0)
    {
    	if((unsigned char)*(string + i) < 0x81)
        {
        	ViDrawAsc(handle,color,x,y,*(string + i), font_size, lace);
        	if(font_size == 0)
            	x += 8;                            
        }
    	else if((unsigned char)*(string + i + 1) >= 0x81)
        {
        	ViDrawChinese(handle,color,x,y,*(string + i),*(string + i + 1),font_size, lace);
        	if(font_size == 0)
            	x += 16;                            
        	i ++;         
        }
    	else
        {
        	SVPrint("read font from lib error!\n");
        }
    	i++;    
    }    

    stBitmap.enPixelFormat = PIXEL_FORMAT_RGB_1555;
    stBitmap.u32Width = unParamBuf[bufIndex].bufWidth;
    stBitmap.u32Height = unParamBuf[bufIndex].bufHeight;
    stBitmap.pData = (void *)unParamBuf[bufIndex].bufData;
    
    ret = HI_MPI_RGN_SetBitMap(handle,&stBitmap);
    
	if(0 != ret)
    {
    	ERRORPRINT("error:HI_MPI_VPP_ControlRegion ret=0x%X!\n",ret);
    	ret = FI_FAILED;
    }    
	return ret;
}

/*
int SVOsdViSetAttr(RGN_HANDLE Handle,const MPP_CHN_S *pstChn,const RGN_CHN_ATTR_S *pstChnAttr)
{
    int ret = -1;

    ret = HI_MPI_RGN_SetDisplayAttr(Handle,pstChn,pstChnAttr);
    return ret;
}

int SVOsdViGetAttr(RGN_HANDLE Handle,const MPP_CHN_S *pstChn,RGN_CHN_ATTR_S *pstChnAttr)
{
    int ret = -1;

    ret = HI_MPI_RGN_GetDisplayAttr(Handle,pstChn,pstChnAttr);
    
    return ret;
}
*/

static int ViDrawAsc(REGION_HANDLE handle, int color, int x, int y,unsigned char asc, 
                	int font_size, int lace)
{
	int	        	i, j;
	int         	ret = 0;
	int         	bufIndex = 0;
	unsigned  char     *zkbuf = NULL;         
 	unsigned short     *pvideo = NULL;
	unsigned  char  lacemem[50] = {0}, *lacememlp = NULL;        
    
	ret = GetBufIndex(handle,&bufIndex);
	if(ret != 0)
    {
    	SVPrint("error:no buf init!\n");
    	return -1;
    }  
	if(NULL == g_asc8x16Buf)
    {
    	FiPrint2("error:NULL == g_asc8x16Buf,maybe can not open font file!\n");
    	sleep(2);
    	return -1;
    }
    
    if(font_size == 0)
       {
    	zkbuf = g_asc8x16Buf + (asc)*16*1;           
    	for (i = 0; i < 16; i++)
        {
        	lacememlp = &lacemem[0];
        	memset(lacemem,0,sizeof(lacemem));        

        	pvideo = (unsigned short*)(unParamBuf[bufIndex].bufData);                
        	pvideo += x + (y + i)*unParamBuf[bufIndex].bufWidth;                
        	for (j = 0; j < 8 ; j++)    
            {
                if (BIT_I(*zkbuf, j))
                {
                    *(pvideo + 7 - j) = color|FGALPHAMASK;
                    *lacememlp = 1;
                }
            	else
                {
                    *(pvideo + 7 - j) = (~color)&BGALPHAMASK;
                }
            	lacememlp++;
            }
        	zkbuf++;
        	if(1 == lace)
            	ViBlacLace(&lacemem[0],pvideo + unParamBuf[bufIndex].bufWidth*2, 8); /*blace lace */                                
        }    
       }                              
	return 0;
}

static int ViDrawChinese(REGION_HANDLE handle, int color, int x,int y,unsigned char qu, unsigned char wei,
            	int font_size,int lace)
{

	int     i, j;
	unsigned int pos;
	unsigned  char    *zkbuf; 
	unsigned short * pvideo; 
	int bufIndex = 0;
	unsigned  char lacemem[500], *lacememlp;
	int ret = 0;
//	unsigned char k,temp;
    
	ret = GetBufIndex(handle,&bufIndex);
	if(ret != 0)
    {
    	SVPrint("error!no buf init!\n");
    	return -1;
    }    
	if(NULL == g_ch16x16Buf)
    {
    	SVPrint("error:NULL == g_ch16x16Buf,maybe can not open font file!\n");
    	return -1;
    }
    
   	if(font_size == 0)
    {                
    	pos=((qu-0x81)*192+(wei-0x40))*32;        
         
    	zkbuf = g_ch16x16Buf + pos;        

    	lacememlp = &lacemem[0];
    	memset(lacemem,0,sizeof(lacemem));                
    	for (i = 0; i < 16; i++)
        {                                            
        	pvideo = (unsigned short*)(unParamBuf[bufIndex].bufData);                
        	pvideo += x + (y + i)*unParamBuf[bufIndex].bufWidth;        
        	for (j=0;j<8;j++)
            {
            	if (BIT_I(*zkbuf, j))
                {
                    *(pvideo + 7 - j) = color|FGALPHAMASK;    /*ql explain:使用mirror提供的字库,如果不是这样读的话,读到的东西会倒过来的*/
                    *lacememlp = 1;
                }
            	else
                {
                    *(pvideo + 7 - j) = (~color)&BGALPHAMASK;
                }
            	lacememlp++;                
            }
        	zkbuf++;
        	for (j=0;j<8;j++)
            {
            	if (BIT_I(*zkbuf, j))
                {
                    *(pvideo + 15 - j) = color|FGALPHAMASK;/*ql explain:使用mirror提供的字库,如果不是这样读的话,读到的东西会倒过来的*/
                    *lacememlp = 1;
                }
            	else
                {
                    *(pvideo + 15 - j) = (~color)&BGALPHAMASK;
                }
                *lacememlp++;                
            }
        	zkbuf++;
        	if(1 == lace)
            {
            	ViBlacLace(&lacemem[0],pvideo + unParamBuf[bufIndex].bufWidth*2, 16);                                 
            }        
        }                                                        
    }               
	return 0;
}
static int ViBlacLace(unsigned char* lacemem,unsigned short* video,int font_width)
{
	int j;
    //return 0;
	for (j = 0; j < font_width; j++)
    {
    	if (*(lacemem + j) == 0 )
        	continue;
    	else
              *(video + j -1) = BLACK_COLOR|FGALPHAMASK;
    }

	return 0;
}
#endif //#if defined MCU_HI3515