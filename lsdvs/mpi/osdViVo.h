/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**	文件名: osdViVo.h
**  description  : for osdViVo.cpp
**  date           :  2013.10.18
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/

#ifndef __OSD_VI_VO_H__
#define __OSD_VI_VO_H__

#if defined MCU_HI3515
#include "hi_comm_vpp.h"
#endif

#if defined MCU_HI3515A
#include "hi_comm_vpss.h"
#include "hi_comm_region.h"
#endif
#include "osdComm.h"



int FiOsdInitOsdLib();

/*
* 解初始化编解码库
* type:0-VI;1-VO
*/
int FiOsdDeinitOsdLib(void);


int FiOsdInitOsd( int type, int ch, OSD_RECT rc, unsigned int color, 
                	uchar  bgTransparence, HI_U32  *handle );

/*
* 销毁指定OSD
* type:0-VI;1-VO
* ch:vegroup 0~31
*/
int FiOsdDestroyOsd(int type,int ch);

/*
* 设置当前叠加层显示还是隐藏
* type:0-VI;1-VO
* ch:vegroup 0~31
* bshow:0-隐藏;1-显示
*/
int FiOsdSetShow(int type,int ch, int bshow);

/*
* 写一个字符串
* type:0-VI;1-VO
* ch:vegroup 0~31
* color:颜色
* x、y:相对于OSD的坐标
* string:内容
* font_size:文字大小,目前只支持0
* lace:是否圆滑颜色
*/
int FiOsdDrawString( int type,int ch, int color, int x, int y, 
            	const char *string, int font_size, int lace);

/*加载文字库*/
int SVOsdInitFont(void);

/*卸载文字库*/
int SVOsdDestroyFont(void);

int FiOsdCheckAscFont();
int FiOsdCheckChineseFont();


int SVOsdInitOsd(int type,VI_CHN ViChannel,OSD_RECT rc,unsigned int color,
 RGN_HANDLE *handle ,eOSDTYPE eosdtype);

int SVOsdDestroyOsd(int type,int ch);

int SVOsdSetShow(int type,int ch, int bshow,OSD_RECT rc,VI_CHN ViChn,uchar  
bgTransparence,eOSDTYPE eosdtype);

int SVOsdDrawString( int type,int ch, int color, int x, int y, 
            	const char *string, int font_size, int lace);

#endif //__OSD_VI_VO_H__



