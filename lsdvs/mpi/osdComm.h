/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**	文件名: osdComm.h
**  description  : 定义一些OSD共用的宏
**  date           :  2013.10.18
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/

#ifndef __OSD_COMM_H__
#define __OSD_COMM_H__

#define BIT_I(ch, i)    ((ch)&(1<<i))

#define	FONT_EN_NUM     128
#define	FONT_CH_NUM     8836
#define FONT_16EH_SIZE  (16*32/8)
#define FONT_32CH_SIZE  (32*32/8)
#define FONT_8EH_SIZE	(8*16/8)
#define FONT_16CH_SIZE  (16*16/8)

#define TUXING_ZK_NUM		20
#define TUXING_32TX_SIZE	(32*32/8)


#define FONT_32CH_FILE	"/bin/32x32heich.bin"
#define FONT_16EN_FILE	"/bin/16x32heien.bin"
#define FONT_16CH_FILE	"/root/resource/font/GBK16"
#define FONT_8EN_FILE	"/root/resource/font/ASC16"

#define LAYER0         0
#define LAYER1         1

#define VI     	0
#define VO     	1

#define NOCOLOR  0x0fff 

#define FGALPHAMASK 0x8000     /*前景色掩码*/
#define BGALPHAMASK 0x7fff     /*背景色掩码*/

#define SCREENWIDTH 800
#define SCREENHIGH 32           /*32大小*/

#define  BLACK_COLOR      	0x0000
#define  BLUE_COLOR      	0x101f
#define  RED_COLOR      	0x7C00
#define  MAGENTA_COLOR  	0x7c1f
#define  GREEN_COLOR      	0x03e0
#define  CYAN_COLOR      	0x03ff
#define  YELLOW_COLOR      	0x7fe0
#define  WHITE_COLOR      	0x7fff

typedef struct _OsdRect
{
	int x;
	int y;
	int width;
	int height;
}OSD_RECT;

typedef enum
{
    TIMEOSD = 0,
    WORDOSD1,
    MAX_OSD_NUM
}eOSDTYPE;

#endif //__OSD_COMM_H__

