/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**	文件名: osdApply.cpp
**  description  : 定义osd应用的接口函数
**  date           :  2013.10.18
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#if defined MCU_HI3515A


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "const.h"
#include "debug.h"
#include "malloc.h"
#if defined MCU_HI3515
#include "hi_comm_vpp.h"
#endif
#if defined MCU_HI3515A
#include "hi_comm_vpss.h"
#include "hi_comm_region.h"
#endif
#include "encComm.h"

#include "thread.h"
#include "const.h"
#include "osdComm.h"
#include "osdVi.h"
#include "osdViVo.h"
#include "osdApply.h"
#include "mutex.h"
#include "timeExchange.h"
#include "fitMpi.h"
#include "paramManage.h"

static pthread_mutex_t g_osd_mutex = PTHREAD_MUTEX_INITIALIZER;

//typedef unsigned int REGION_HANDLE;
CMutexLock g_osdIfMutex;

struct OsdRegion {
	RGN_HANDLE handle;
	int enable;
	int color;
	int fontsize;
	OSD_RECT rect;
    unsigned char  bgTransparence;
};

struct _OsdChannel {
	struct OsdRegion time;
	struct OsdRegion logo;
	int veGroupId;
	struct _OsdChannel *next;
};

static struct _OsdChannel *g_osd_list = NULL;
static OSD_LOGO	g_osdLogo[MAX_CHANNEL_NUM]; 
static OSD_TIME g_osdTime[REAL_CHANNEL_NUM];

void FiOsdInitTimeOsdStruct(void)
{
	int i,ret = -1;
	PARAM_CONFIG_OSD_TIME timeOsd;
    
	memset(g_osdTime, 0, sizeof(g_osdTime));
	for(i=0; i<REAL_CHANNEL_NUM; i++)
    {
    	ret = ParamGetOsdTime( i, &timeOsd );
    	if(0 == ret)
        {
        	g_osdTime[i].enable = timeOsd.enable;
        	g_osdTime[i].colorRed = timeOsd.colorRed;
        	g_osdTime[i].colorGreen = timeOsd.colorGreen;
        	g_osdTime[i].colorBlue = timeOsd.colorBlue;
        	g_osdTime[i].xPos = timeOsd.xPos;
        	g_osdTime[i].yPos = timeOsd.yPos;    
        	g_osdTime[i].bgTransparence = timeOsd.bgTransparence;    
        	SVPrint("time osd param from flash:ch(%d) enable(%d),x(%d),y(%d)!\r\n",i,g_osdTime[i].enable,g_osdTime[i].xPos,g_osdTime[i].yPos);
        }
    	else
        {
        	g_osdTime[i].enable = 1;
        	g_osdTime[i].colorRed = 255;
        	g_osdTime[i].colorGreen = 255;
        	g_osdTime[i].colorBlue = 255;
        	g_osdTime[i].xPos = 1;
        	g_osdTime[i].yPos = 1;        
        	ERRORPRINT("time osd param not from flash:ch(%d) enable(%d),x(%d),y(%d)!\r\n",i,g_osdTime[i].enable,g_osdTime[i].xPos,g_osdTime[i].yPos);
        }
    }
}

int FiOsdGetTimeOsdStruct(int channel,OSD_TIME *timeOsd)
{    
	if(NULL == timeOsd)
    {
    	SVPrint("error:NULL == timeOsd!\r\n");
    	return FI_FAILED;
    }
	g_osdIfMutex.Lock();
    *timeOsd = g_osdTime[channel];
	g_osdIfMutex.Unlock();
    
	return FI_SUCCESSFUL;
}

int FiOsdSetTimeOsdStruct(int channel,OSD_TIME *timeOsd)
{
	if(NULL == timeOsd)
    {
    	SVPrint("error:NULL == timeOsd!\r\n");
    	return FI_FAILED;
    }
	g_osdIfMutex.Lock();
	g_osdTime[channel] = *timeOsd;
	g_osdIfMutex.Unlock();

	return FI_SUCCESSFUL;
}

/* osd logo */
void FiOsdInitLogoOsdStruct(void)
{
	int i,ret = -1;
	PARAM_CONFIG_OSD_LOGO logoOsd;
    
	memset(g_osdLogo, 0, sizeof(g_osdLogo));
	for(i=0; i<REAL_CHANNEL_NUM; i++)
    {
    	ret = ParamGetOsdLogo( i, &logoOsd );
    	if(0 == ret)
        {
        	g_osdLogo[i].enable     = logoOsd.enable;
        	g_osdLogo[i].colorRed     = logoOsd.colorRed;
        	g_osdLogo[i].colorGreen = logoOsd.colorGreen;
        	g_osdLogo[i].colorBlue     = logoOsd.colorBlue;
        	g_osdLogo[i].xPos         = logoOsd.xPos;
        	g_osdLogo[i].yPos         = logoOsd.yPos;
        	g_osdLogo[i].bgTransparence = logoOsd.bgTransparence;
        	snprintf( g_osdLogo[i].logo, 64, "%s", logoOsd.logo );
        	SVPrint("logo osd param from flash:ch(%d) enable(%d),x(%d),y(%d),logo(%s)!\r\n",i,g_osdLogo[i].enable,g_osdLogo[i].xPos,g_osdLogo[i].yPos,g_osdLogo[i].logo);        
        }
    	else
        {
        	g_osdLogo[i].enable     = 1;
        	g_osdLogo[i].colorRed     = 255;
        	g_osdLogo[i].colorGreen = 255;
        	g_osdLogo[i].colorBlue     = 255;
        	g_osdLogo[i].xPos         = 0;
        	g_osdLogo[i].yPos         = 100;
        	snprintf (g_osdLogo[i].logo,64,"dvs_ch%02d", i+1 );    
        	ERRORPRINT("logo osd param not from flash:ch(%d) enable(%d),x(%d),y(%d),logo(%s)!\r\n",i,g_osdLogo[i].enable,g_osdLogo[i].xPos,g_osdLogo[i].yPos,g_osdLogo[i].logo);        
        }
    }
}

int FiOsdGetLogoOsdStruct(int channel, OSD_LOGO *logoOsd)
{
	if(NULL == logoOsd)
    {
    	SVPrint("error:NULL == temp!\r\n");
    	return FI_FAILED;
    }
    
	g_osdIfMutex.Lock();    
    *logoOsd = g_osdLogo[channel];    
	g_osdIfMutex.Unlock();

	return FI_SUCCESSFUL;
}

int FiOsdSetLogoOsdStruct(int channel, OSD_LOGO *logoOsd)
{
	if(NULL == logoOsd)
    {
    	SVPrint("error:NULL == temp!\r\n");
    	return FI_FAILED;
    }    
	g_osdIfMutex.Lock();
	g_osdLogo[channel] = *logoOsd;    
	g_osdIfMutex.Unlock();
    
	return FI_SUCCESSFUL;
}

int rgb_to_int(unsigned char r,unsigned char g,unsigned char b)
{
    return (int)(((uint)b << 16) | (ushort)(((ushort)g << 8  ) | r));
}

char int_to_rgb(int color,unsigned char *r,unsigned char *g,unsigned char *b)
{
    if(NULL == r||NULL == g||NULL == b)
    {
        return 0;
    }
    
    *r = 0xFF & color;
    *g = ((0xFF00 & color) >> 8);
    *b = ((0xFF0000 & color) >> 16);
    
    return 1;
}

/*
* 刷新时间OSD
*/
static int RefreshTimeOsd(struct _OsdChannel *posd)
{
	int ret = FI_SUCCESSFUL;
	int color = 0, resolution;
	int x,y,width,height;
	int year,month,day,hour,minute,second;
	char buffer[128];
	int fontsize = 0;
	size_t maxlen;
	OSD_TIME ot;
	maxlen = strlen("2012-10-15 15:30:28");
	height = 16;

    eOSDTYPE eosdtype = TIMEOSD;
    
	FiOsdGetTimeOsdStruct((posd->veGroupId)%8,&ot);
    
	color = ot.colorBlue>>3;
	color |= (ot.colorGreen >>3)<<5;
	color |= (ot.colorRed >>3)<<10;  
	//color = rgb_to_int(ot.colorRed,ot.colorGreen,ot.colorBlue);
	x = ot.xPos;
	y = ot.yPos;    

	resolution = FiEncGetOsdResolution(posd->veGroupId);
	if((resolution==HI_D1)||(resolution == HI_VGA))
    {
    	fontsize = 0;
    }
	else if(resolution == HI_HD1)
    {
        //y >>= 1; 
    	fontsize = 0;
    }
	else if(resolution == HI_CIF)
    {
        //x >>= 1; 
        //y >>= 1; 
    	fontsize = 0;
    }
	else if(resolution == HI_QCIF)
    {
        //x >>= 2; 
        //y >>= 2; 
    	fontsize = 0;
    }
	else if(resolution == HI_QVGA)
    {
        //x >>= 1; 
        //y >>= 1; 
    	fontsize = 0;
    }
	else 
    {
        //x = 16; 
        //y = 16; 
    	fontsize = 0;
    }
    
	if(fontsize == 1)
    {
    	width =  16 * maxlen;
    	height *= 2;
    }
	else if(fontsize == 0)
    {
     	width = 8 * maxlen;
    }
	else if(fontsize ==2)
    {    
     	width =  16 * maxlen;
    }
    
	posd->time.fontsize = fontsize;
	FiTimeUtcToHuman(time(NULL), &year, &month, &day, &hour, &minute, &second);
	snprintf(buffer, sizeof(buffer),"%04d-%02d-%02d %02d:%02d:%02d", 
                	year, month, day, hour, minute, second);    
	if(ot.enable != posd->time.enable)
    {
    	posd->time.enable = ot.enable;
    	if(posd->time.enable)
        {
        	posd->time.color          = color;
        	posd->time.rect.width     = width;
        	posd->time.rect.height    = height; 
        	posd->time.rect.x         = x;
        	posd->time.rect.y         = y;
            posd->time.bgTransparence = ot.bgTransparence;

            ret = SVOsdInitOsd(VI,posd->veGroupId,posd->time.rect,posd->time.color,
 &posd->time.handle,eosdtype);

        	if(0 == ret) 
            {        
                ret = SVOsdSetShow(VI, posd->time.handle, 
                1,posd->time.rect,posd->veGroupId,ot.bgTransparence,eosdtype);
            }
        	if(0 == ret) 
            {

                ret = SVOsdDrawString( VI, posd->time.handle,posd->time.color,
                                    	0, 0, buffer,fontsize, 0 );    
            }
        }
    	else  
        {
        	ret = SVOsdDestroyOsd(VI,posd->time.handle);
        }
    }
	else  
    {
    	if(posd->time.enable)
        {
        	if(posd->time.rect.x != x || posd->time.rect.y != y||\
                posd ->time.color != color||posd->time.bgTransparence != ot.bgTransparence)
            {
            	posd->time.color          = color;
            	posd->time.rect.x         = x;
            	posd->time.rect.y         = y;
            	posd->time.rect.width     = width;
            	posd->time.rect.height    = height; 
                posd->time.bgTransparence = ot.bgTransparence;
            	SVOsdDestroyOsd(VI, posd->time.handle);
            	if(0 == ret) 
                {
                	ret = SVOsdInitOsd( VI, posd->veGroupId, 
                                    	posd->time.rect,posd->time.color,&posd->time.handle 
                                        ,eosdtype);
                }
            	if(0 == ret)
                {
                    ret = SVOsdSetShow(VI, posd->time.handle, 
                    1,posd->time.rect,posd->veGroupId,ot.bgTransparence,eosdtype);
                }
            	if(0 == ret) 
                {
                	ret = SVOsdDrawString( VI, posd->time.handle, posd->time.color, 
                                        	0, 0, buffer, fontsize, 0 );    
                }
            }
        	else 
            {
            	posd->time.color = color;
            	ret = SVOsdDrawString(VI, posd->time.handle,posd->time.color,0,0, buffer ,fontsize,0);
            }
        }
    }

	return ret;
}

/*
* 刷新logo OSD
*/
static int RefreshLogoOsd(struct _OsdChannel *posd)
{
	int ret = FI_SUCCESSFUL;
	int color, resolution;
	int x,y,width,height;
	char buffer[MAX_LOGO_LEN];
	int fontsize;
	OSD_LOGO ol;

    if(NULL == posd)
    {
        return ret;
    }
	FiOsdGetLogoOsdStruct((posd->veGroupId)%8,&ol);    
    
	ol.logo[sizeof(ol.logo) - 1]= '\0';
    
	color = 0;
	color = ol.colorBlue>>3;
	color |= (ol.colorGreen >>3)<<5;
	color |= (ol.colorRed >>3)<<10;
    //color = rgb_to_int(ol.colorRed,ol.colorGreen,ol.colorBlue);
	x = ol.xPos;
	y = ol.yPos;
	fontsize = 1;

    eOSDTYPE eosdtype = WORDOSD1;

	resolution = FiEncGetOsdResolution(posd->veGroupId);    
	if((resolution==HI_D1)||(resolution==HI_VGA))
    {
    	fontsize = 0;
    }
	else if(resolution == HI_HD1)
    {
    	fontsize = 0;
    }
	else if(resolution == HI_CIF)
    {
    	fontsize = 0;
    }
	else if(resolution == HI_QCIF)
    {
    	fontsize =0;
    }
	else if(resolution == HI_QVGA)
    {
    	fontsize =0;
    }
	else
    {
    	x = 10; 
    	y = 10; 
    	fontsize = 0;
    }
    
	if(fontsize == 1)
    {
    	width =  8 * strlen(ol.logo);
    	height = 16;
    }
	else if(fontsize == 0)
    {
    	width = 8 * strlen(ol.logo);
    	height = 16;
    }
	else if(fontsize ==2)
    {    
    	width =  16 * strlen(ol.logo);
    	height = 16;
    }
    
	posd->logo.fontsize = fontsize;
	strncpy(buffer,ol.logo,sizeof(buffer));
    
	if(ol.enable != posd->logo.enable)
    {    
    	posd->logo.enable = ol.enable;
    	if(posd->logo.enable)
        { 
        	posd->logo.color          = color;
        	posd->logo.rect.x         = x;
        	posd->logo.rect.y         = y;
        	posd->logo.rect.width     = width;
        	posd->logo.rect.height    = height;
            posd->logo.bgTransparence = ol.bgTransparence;
            
        	if(width >= 8) 
            {
            	ret = SVOsdViInitOsdBgClrEx(posd->veGroupId, posd->logo.rect,
                    	posd->logo.color, &posd->logo.handle ,eosdtype);
            }
            
        	if(0 == ret && width >= 8) 
            {
                ret = SVOsdSetShow(VI, posd->logo.handle, 
                1,posd->logo.rect,posd->veGroupId,ol.bgTransparence,eosdtype);
            }
            
        	if(0 == ret && width >= 8) 
            {
            	ret = SVOsdDrawString(VI, posd->logo.handle,posd->logo.color,0,0,buffer,fontsize,0);    
            }
        }
    	else  
        {    
        	ret = SVOsdDestroyOsd(VI,posd->logo.handle);
        }
    }
	else  
    {        
    	if( posd->logo.enable )
        {
        	if(posd->logo.rect.x != x || posd->logo.rect.y != y || \
                posd->logo.rect.width != width||posd->logo.color != color||\
                posd->logo.bgTransparence != ol.bgTransparence)
            {                
            	posd->logo.color          = color;
            	posd->logo.rect.x         = x;
            	posd->logo.rect.y         = y;
            	posd->logo.rect.width     = width;
            	posd->logo.rect.height    = height;
                posd->logo.bgTransparence = ol.bgTransparence;
                
                if(width >= 8)
            	    ret = SVOsdDestroyOsd(VI, posd->logo.handle);
                
            	if(0 == ret) 
                {
                    ret = SVOsdViInitOsdBgClrEx(posd->veGroupId, posd->logo.rect,
                    	posd->logo.color, &posd->logo.handle ,eosdtype);
                	//ret = SVOsdViInitOsd(posd->veGroupId, posd->logo.rect,
                                        	//posd->logo.color,
                                            //&posd->logo.handle ,eosdtype);//SVOsdViInitOsdBgClrEx

                    if(0 == ret) 
                    { 
                        ret = SVOsdSetShow(VI, posd->logo.handle, 
                        1,posd->logo.rect,posd->veGroupId,ol.bgTransparence,eosdtype);

                        if(0 == ret) 
                        { 
                        	ret = SVOsdDrawString( VI, posd->logo.handle, posd->logo.color, 
                                                    	0, 0, buffer, fontsize, 0 );

                            if(0 != ret)
                            {
                                ERRORPRINT("SVOsdDrawString return:%d\n",ret);
                            }
                        }
                        else
                        {
                            ERRORPRINT("SVOsdSetShow return:%d\n",ret);
                        }
                    }
                    else
                    {
                        ERRORPRINT("SVOsdViInitOsdBgClrEx return:%d\n",ret);
                    }
                }
                else
                {
                    ERRORPRINT("SVOsdDestroyOsd return:%d\n",ret);
                }
                                
            }
        	else 
            { 
            	posd->logo.color = color;
            	ret = SVOsdDrawString(0, posd->logo.handle,posd->logo.color, 
                                            	0, 0, buffer, fontsize, 0 );    
            }
        }
    }
	return ret;
}

/*
* 处理OSD创建事件
*/
static int HandleOsdCreateEvent(int channel)
{    
	int ret = FI_FAILED;
	struct _OsdChannel *posd = NULL;

	if(0 == FiOsdCheckAscFont() )//|| 0 == FiOsdCheckChineseFont())//暂时不支持中文OSD，待添加TBD
    {
        ERRORPRINT("font buffer is null\n");
        return -1;
    }   
        
    //
	SVPrint( "creat osd, vegropid is %d\r\n", channel);
	for(posd = g_osd_list; posd != NULL; posd = posd->next)
    {
    	if(posd->veGroupId == channel) 
        {
        	ERRORPRINT("osd create failed:It seems that this osd channel already exist!\r\n");
        	return FI_FAILED;
        }
    }
    
	posd = (struct _OsdChannel *)Malloc(sizeof(struct _OsdChannel));
	memset(posd,0,sizeof(struct _OsdChannel));
	posd->veGroupId = channel;
    
	ret = RefreshTimeOsd(posd);    
	if(FI_SUCCESSFUL != ret)
    {
    	ERRORPRINT("failed:ch(%d) time osd create failed!\r\n",channel);
    }
    
	ret = RefreshLogoOsd(posd);    
	if(FI_SUCCESSFUL != ret)
    {
    	ERRORPRINT("failed:ch(%d) logo osd create failed!\r\n",channel);
    }
	posd->next = g_osd_list;
	g_osd_list = posd;

	return ret;
}

static int HandleOsdRefreshEvent(int channel)
{
	int ret = FI_FAILED;
	struct _OsdChannel *posd = NULL;

	if(0 == FiOsdCheckAscFont() || 0 == FiOsdCheckChineseFont())
    	return -1;
        
	for(posd = g_osd_list; posd != NULL; posd = posd->next)
    {
    	if(posd->veGroupId == channel) 
    	break;
    }
	if(posd == NULL)
    {
    	SVPrint("osd refrech failed:ch(%d) osd had not been created!\r\n",channel);
    	return FI_FAILED;
    }
    
	ret = RefreshTimeOsd(posd);
	if(FI_SUCCESSFUL != ret)
    {
    	SVPrint("failed:ch(%d) time osd refrech failed!\r\n",channel);
    }
	ret = RefreshLogoOsd(posd);
	if(FI_SUCCESSFUL != ret)
    {
    	SVPrint("failed:ch(%d) logo osd refrech failed!\r\n",channel);
    }

	return ret;
}

/*
* OSD销毁
*/
static int HandleOsdDestoryEvent(int channel)
{
	struct _OsdChannel *posd = NULL,*prev=NULL;
    
	for(prev=NULL,posd = g_osd_list; posd != NULL; )
    {
    	if(posd->veGroupId == channel) 
        {
        	if(posd->time.enable)
            {
            	SVOsdDestroyOsd(VI,posd->time.handle);
            }
        	if(posd->logo.enable)
            {
            	SVOsdDestroyOsd(VI,posd->logo.handle);
            }            
        	if(prev==NULL) 
            {
            	g_osd_list = posd->next;
            }
        	else
            {
            	prev->next = posd->next;
            }
        	Free(posd);
        	posd = NULL;
        	return FI_SUCCESSFUL;
        }
    	prev = posd;
    	posd = posd->next;
    }
	if(posd == NULL)
    {
    	SVPrint("failed:can't find ch(%) osd to destory!\r\n",channel);
    }    
	return FI_FAILED;
}

/*
* 时间OSD刷新
*/
static int HandleTimeOsdRefreshEvent(int param1)
{
	struct _OsdChannel *posd = NULL;
	int ret = FI_FAILED;

	if(0 == FiOsdCheckAscFont())
    	return -1;
    
	for(posd = g_osd_list; posd != NULL; posd = posd->next)
    {
    	if(posd->veGroupId<0 || posd->veGroupId>32)
        	return FI_FAILED;
            
    	ret = RefreshTimeOsd(posd);
    }

	return ret;
}

/*
* Logo OSD刷新
*/
static int HandleLogoOsdRefreshEvent(int channel)
{
	int ret = FI_FAILED;
	struct _OsdChannel *posd = NULL;

	if(0 == FiOsdCheckAscFont() )//|| 0 == FiOsdCheckChineseFont())
    	return -1;
    
	for(posd = g_osd_list; posd != NULL; posd = posd->next)
    {
    	if(posd->veGroupId == channel) 
        	break;
    }
	if(posd == NULL)
    {
    	SVPrint("failed:ch(%d) logoOsd hadn't been created!\r\n",channel);
    	return FI_FAILED;
    }
	ret = RefreshLogoOsd(posd);

    if(ret != FI_SUCCESS)
    {
        ERRORPRINT("RefreshLogoOsd return:%d\n",ret);
    }

	return ret;
}

int FiOsdDoKindOfEvent(int event,int channel)
{
	int ret = FI_FAILED;
    
	pthread_mutex_lock(&g_osd_mutex);
	switch(event)
    {
    	case OSDEVENT_VECREATE :   
        	ret = HandleOsdCreateEvent(channel);    
        	break; 
    	case OSDEVENT_VEUPDATE :   
        	ret = HandleOsdRefreshEvent(channel);    
        	break; 
    	case OSDEVENT_VEDESTROY:   
        	ret = HandleOsdDestoryEvent(channel);  
        	break; 
    	case OSDEVENT_TIMEUPDATE:  
        	ret = HandleTimeOsdRefreshEvent(channel);  
        	break; 
    	case OSDEVENT_LOGOUPDATE:  
        	ret = HandleLogoOsdRefreshEvent(channel);  
        	break; 
    	default : 
        	break;
    }
	pthread_mutex_unlock(&g_osd_mutex);

	return ret;
}

int FiOsdSetLogoOsdConfig(int channel,CONFIG_OSD_LOGO *setVal)
{
	int ret = FI_FAILED;
	OSD_LOGO logoOsd = {0};
    
	if(NULL == setVal)
    {
    	SVPrint( "NULL == setVal\r\n" );
    	return FI_FAILED;
    }
	logoOsd.colorRed = setVal->colorRed;
	logoOsd.colorGreen = setVal->colorGreen;
	logoOsd.colorBlue = setVal->colorBlue;
	logoOsd.enable = setVal->enable;
	logoOsd.xPos = setVal->xPos;
	logoOsd.yPos = setVal->yPos;
	snprintf( logoOsd.logo, 64, "%s", setVal->logo );    
    
	logoOsd.bgTransparence = setVal->bgTransparence;
	ret = FiOsdSetLogoOsdStruct(channel,&logoOsd);
	if(0 == ret) ret = FiOsdDoKindOfEvent( OSDEVENT_LOGOUPDATE, channel );
    
	return ret;
}

int FiOsdSetTimeOsdConfig(int channel,CONFIG_OSD_TIME *setVal)
{
	int ret = FI_FAILED;
	OSD_TIME timeOsd;
    
	if(NULL == setVal)
    {
    	SVPrint("NULL == setVal\r\n");
    	return FI_FAILED;
    }
	timeOsd.colorRed = setVal->colorRed;
	timeOsd.colorGreen = setVal->colorGreen;
	timeOsd.colorBlue = setVal->colorBlue;
	timeOsd.enable = setVal->enable;
	timeOsd.xPos = setVal->xPos;
	timeOsd.yPos = setVal->yPos;    
	timeOsd.bgTransparence = setVal->bgTransparence;
	ret = FiOsdSetTimeOsdStruct(channel,&timeOsd);
	if(0 == ret) ret = FiOsdDoKindOfEvent(OSDEVENT_TIMEUPDATE,channel);

	return ret;    
}

/*Osd time */
void FiRefreshOsdTime(void)
{
	OSD_TIME ot;
	int i;
	int enable = 0;
	static time_t oldt;
	for(i=0; i < MAX_OSD_CHN; i++ )    
    {
    	FiOsdGetTimeOsdStruct(i, &ot);        
    	if(ot.enable) 
        {
        	enable =1;
        	break;
        }
    }
    
	if(enable)
    {
    	if(oldt != time(NULL))
        {
        	oldt = time(NULL);
        	FiOsdDoKindOfEvent(OSDEVENT_TIMEUPDATE,0);
        }
    }
}

static pthread_t g_timeOsdThreadFlag;
static void *OsdRealTimeAppThread(void *arg)
{    
	ThreadDetach(ThreadSelf());
	SVPrint("OsdRealTimeAppThread:%d!\r\n",ThreadSelf());
	while(g_timeOsdThreadFlag)
    {
    	usleep(1000*100);    
    	if(FiOsdCheckAscFont())
        	FiRefreshOsdTime();                    
    }
	return NULL;
}

static pthread_t g_time_osd_id;
int FiOsdStartTimeOsdThread()
{        
	g_timeOsdThreadFlag = 1;
	if(ThreadCreateCommonPriority(&g_time_osd_id,
                    	OsdRealTimeAppThread, 
                    	NULL) < 0)
    {
    	g_timeOsdThreadFlag = 0;
    	SVPrint("pthread_create() error:%s!\r\n",STRERROR_ERRNO);
    	return FI_FAILED;
    }
	return FI_SUCCESSFUL;
}

int FiOsdStopTimeOsdThread()
{
	g_timeOsdThreadFlag = 0;
	return FI_SUCCESSFUL;
}

#endif //defined MCU_HI3515

