/********************************************************************************
**	Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**	author        :  sven
**	version       :  v1.0
**	date           :  2013.10.10
**	description  : ddns 应用接口
********************************************************************************/
#include "debug.h"
#include "ddnsFifo.h"
#include "ddnsApp.h"

/*
* fn: 刷新ddns的工作参数
      此函数在设备初始化和修改了ddns参数后被调用。      
*/
int DdnsAppRefresh()
{
	int ret;
    
	ret = DdnsWriteConff( (char *)DDNS_CONF_PATHNAME );
	if( 0 == ret )
    {
    	ret = DdnsFifoCmd( DDNS_FIFO_CMD_REFREASH );
    }
	return ret;
}

