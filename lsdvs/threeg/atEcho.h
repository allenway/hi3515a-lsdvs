/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 控制3G模块的回显
**  date           :  2014.9.25
**
**  version       :  1.0
**  author        :  sven
********************************************************************************/


#ifndef __ATECHO_H__
#define __ATECHO_H__

typedef enum _AtEcho_
{
	AT_ECHO_OFF = 0,    // 关闭回显
	AT_ECHO_ON	        // 打开回显
} AT_ECHO_EN;

int FiAtEchoCtl( AT_ECHO_EN flag );    
    
#endif

