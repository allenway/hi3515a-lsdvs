/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 支持一个生产者->多个消费者的类
         图片抓拍专用的生产者消费者模块
********************************************************************************/
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "debug.h"
#include "procon.h"
#include "malloc.h"
#include "thread.h"

static CProcon g_jpgProcon( BLOCK_NO, DATA_TYPE_NOMAL );

void ProconJpgInit( int chNum, int dataNode )
{
	g_jpgProcon.Init( chNum, dataNode );
}

uint ProconJpgOpen( int channel, int flag )
{
	return g_jpgProcon.Open( channel, flag );
}

void ProconJpgClose( uint fd )
{
	g_jpgProcon.Close( fd );
}

int ProconJpgWrite( uint fd, DATA_PIECE_T proDataInfo )
{
	int ret;    
    
	ret = g_jpgProcon.Write( fd, proDataInfo );    

	return ret;
}

PROCON_NODE_T *ProconJpgRead( uint fd )
{
	return g_jpgProcon.Read( fd );
}

void ProconJpgFree( PROCON_NODE_T *pcpNode )
{
	g_jpgProcon.Tfree( pcpNode );
}

