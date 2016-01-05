/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**	文件名: osdViVo.cpp
**  description  : 兼容VI VO 的OSD
**  date           :  2013.10.18
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/

#if defined MCU_HI3515A

#include <fcntl.h>
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
#endif
#include "osdComm.h"
#include "osdVi.h"
#include "linuxFile.h"
#include "osdViVo.h"
#include "osdApply.h"

unsigned char *g_asc8x16Buf = NULL;
unsigned char *g_ch16x16Buf = NULL;

int FiOsdCheckAscFont()
{
	int ret = 0;
    
	if(NULL != g_asc8x16Buf) ret = 1;

	return ret;
}

int FiOsdCheckChineseFont()
{
	int ret = 0;
    
	if(NULL != g_ch16x16Buf) ret = 1;

	return ret;
}

int FiOsdInitOsdLib()
{
	int ret = 0;

	FiOsdInitTimeOsdStruct();    
	FiOsdInitLogoOsdStruct();
    
	SVOsdInitFont();
    ret = FiOsdViInitOsdLib();    
	return ret;
}

int FiOsdDeinitOsdLib()
{
	int ret = 0;
    //if(VI == type)
    	ret = FiOsdViDeinitOsdLib();    
	SVOsdDestroyFont();
	return ret;
}

/*
* fn: 初始化指定区域
* type:0-VI;1-VO
* ch:vegroup 0~31，如果ch==0xff，则为设置公共通道
* rc:矩形
* color:颜色
* handle:out,创建的osd句柄
* bgTransparence: 背景透明度 0~100,标示0%~100%
*/
#if defined MCU_HI3515
int FiOsdInitOsd( int type, int ch, OSD_RECT rc, unsigned int color, 
                	uchar  bgTransparence, HI_U32  *handle )
{
	int ret = 0;
	if(VI == type)
    	ret = FiOsdViInitOsd( ch, rc, color, bgTransparence, handle );        
	return ret;
}
#endif
#if defined MCU_HI3515A
int SVOsdInitOsd(int type,VI_CHN ViChannel,OSD_RECT rc,unsigned int color,
 RGN_HANDLE *handle ,eOSDTYPE eosdtype)
{
    int ret = 0;
    
    if(VI == type)
    	ret = SVOsdViInitOsd(ViChannel,rc,color, handle ,eosdtype);        
	return ret;
}
#endif
#if defined MCU_HI3515
int FiOsdDestroyOsd(int type,int ch)
{
	int ret = FI_FAILED;
	if(VI == type)
    	ret = FiOsdViDestroyOsd(ch);    
	return ret;    
}
#endif
int SVOsdDestroyOsd(int type,int ch)
{
	int ret = FI_FAILED;
	if(VI == type)
    	ret = SVOsdViDestroyOsd(ch);    
	return ret;    
}
#if defined MCU_HI3515
int FiOsdSetShow(int type,int ch, int bshow)
{
	int ret = FI_FAILED;
	if(VI == type)
    	ret = FiOsdViSetShow(ch,bshow);        
	return ret;
}
#endif
int SVOsdSetShow(int type,int ch, int bshow,OSD_RECT rc,VI_CHN ViChn,uchar  
                bgTransparence,eOSDTYPE eosdtype)
{
    int ret = FI_FAILED;
	if(VI == type)
    	ret = SVOsdViSetShow(ch, bshow,rc,ViChn, bgTransparence,eosdtype);        
	return ret;
}
#if defined MCU_HI3515
int FiOsdDrawString( int type,int ch, int color, int x, int y, 
            	const char *string, int font_size, int lace)
{
	int ret = FI_FAILED;
	if(VI == type)
    	ret = FiOsdViDrawString(ch,color, x, y, string,font_size,lace);        
	return ret;
}
#endif
int SVOsdDrawString( int type,int ch, int color, int x, int y, 
            	const char *string, int font_size, int lace)
{
	int ret = FI_FAILED;
	if(VI == type)
    	ret = SVOsdViDrawString(ch,color, x, y, string,font_size,lace);        
	return ret;
}

/*
int SVOsdSetAttr(int type,RGN_HANDLE handle,const MPP_CHN_S *pstChn,const RGN_CHN_ATTR_S *pstChnAttr)
{
    int ret = FI_FAILED;
	if(VI == type)
    	ret = SVOsdViSetAttr(handle,pstChn,pstChnAttr);        
	return ret;
}

int SVOsdGetAttr(int type,RGN_HANDLE handle,const MPP_CHN_S *pstChn,RGN_CHN_ATTR_S *pstChnAttr)
{
    int ret = FI_FAILED;
	if(VI == type)
    	ret = SVOsdViGetAttr(handle,pstChn,pstChnAttr);        
	return ret;
}
*/
int SVOsdInitFont(void)
{
	int fd;
	int ret = 0;    
	struct stat st;
	unsigned int file_size = 0;
    
	if(g_asc8x16Buf == NULL)
    {
    	if ((fd = open(FONT_8EN_FILE ,O_RDONLY)) < 0)
        {
        	ERRORPRINT("open(%s) error:%s\r\n",FONT_8EN_FILE,STRERROR_ERRNO);
        	return(-1);
        }
    	if(fstat(fd, &st) != 0)
        {
        	ERRORPRINT("fstat %s failed!\r\n",FONT_8EN_FILE);            
        	close(fd);
        	return -1;
        }
    	file_size = st.st_size;    
    	if((g_asc8x16Buf = (unsigned char *)Calloc(1,file_size)) == NULL)
        {
        	ERRORPRINT("g_asc8x16Buf Malloc \r\n");
        	close(fd);
        	return(-1);
        }
        
    	ret = Readn(fd, (char *)g_asc8x16Buf, file_size);
    	if(-1 == ret)
        {
        	ERRORPRINT("read() error:%s\r\n",STRERROR_ERRNO);
        	close(fd);
        	return(-1);        
        }
    	close(fd);    
    }
	if(g_ch16x16Buf == NULL)
    {                
    	if ((fd = open(FONT_16CH_FILE, O_RDONLY)) < 0)
        {
        	ERRORPRINT("open(%s) error:%s\r\n",FONT_16CH_FILE,STRERROR_ERRNO);
        	return(-1);
        }
    	if(fstat(fd, &st) != 0)
        {
        	ERRORPRINT("fstat %s failed!\r\n",FONT_16CH_FILE);            
        	close(fd);
        	return -1;
        }
    	file_size = st.st_size;    
    	if ((g_ch16x16Buf = (unsigned char *)Calloc(1,file_size)) == NULL)
        {
        	ERRORPRINT("g_asc_16x32_buf Malloc\r\n");            
        	close(fd);
        	return(-1);
        }
    	ret = Readn(fd, (char *)g_ch16x16Buf, file_size);
    	if(-1 == ret)
        {
        	ERRORPRINT("read FONT_32CH_FILE\r\n");                
        	close(fd);
        	return(-1);        
        }
    	close(fd);
    }
	return(0);
}

int SVOsdDestroyFont(void)
{    
	if(g_asc8x16Buf != NULL)
    {
    	Free(g_asc8x16Buf);
    	g_asc8x16Buf = NULL;
    }
	if(g_ch16x16Buf != NULL)
    {
    	Free(g_ch16x16Buf);
    	g_ch16x16Buf = NULL;
    }
	return 0;
}

#endif //#if defined MCU_HI3515