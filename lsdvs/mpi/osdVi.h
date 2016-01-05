/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**	文件名: osdVi.h
**  description  : for osdVi.cpp
**  date           :  2013.10.18
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/

#ifndef __OSD_VI_H__
#define __OSD_VI_H__
#include "hi_comm_region.h"
#define OSD_ALIGN_X 8  // 纵坐标8 个像素对齐
#define OSD_ALIGN 	2  // 2 个像素对齐

#if defined MCU_HI3515A

typedef unsigned int REGION_HANDLE;
typedef struct OsdViParamBuf
{
	RGN_HANDLE bufHandle;
	int bufWidth;
	int bufHeight;    
	void *bufData;    
}OSD_VI_PARAM_BUF;


/*初始化OSD库*/
int FiOsdViInitOsdLib(void);
/*解初始化OSD库 */
int FiOsdViDeinitOsdLib(void);
int FiOsdViInitOsd( VENC_GRP VeGroup,OSD_RECT rc, unsigned int color,  
                	uchar bgTransparence, RGN_HANDLE *handle );

int FiOsdViDestroyOsd(RGN_HANDLE handle);

/*设置当前叠加层显示还是隐藏.
bshow :  1，显示 0，隐藏 */
int FiOsdViSetShow(RGN_HANDLE handle, int bshow);

/*写一个字符串.  */
int FiOsdViDrawString(RGN_HANDLE handle, int color, int x, int y, 
            	const char *string, int font_size, int lace);
int FiOsdViInitOsdBgClrEx( VENC_GRP VeGroup,OSD_RECT rc, unsigned int color,  
                        	uchar bgTransparence, REGION_HANDLE *handle );


int SVOsdViInitOsdBgClrEx( VI_CHN VeGroup,OSD_RECT rc, unsigned int color,  
            	RGN_HANDLE *handle ,eOSDTYPE eosdtype);

int SVOsdViInitOsd( VI_CHN VeGroup,OSD_RECT rc, unsigned int color,  
            	RGN_HANDLE *handle ,eOSDTYPE eosdtype);

int SVOsdViDestroyOsd(REGION_HANDLE handle);

int SVOsdViSetShow(RGN_HANDLE handle, int bshow,OSD_RECT rc,VI_CHN ViChn,
uchar  bgTransparence,eOSDTYPE eosdtype);

int SVOsdViDrawString(RGN_HANDLE handle, int color, int x, int y, 
            	const char *string, int font_size, int lace);

#endif //#if defined MCU_HI3515
#endif //__OSD_VI_H__



