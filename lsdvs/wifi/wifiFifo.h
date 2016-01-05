/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 管理wifi fifo,主要是和wifi.sh脚本通信
**  date           :  2013.10.21
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/

#ifndef __WIFIFIFO_H__
#define __WIFIFIFO_H__

#include "wifiCom.h"

#define WIFI_FIFO_DEVICE "/tmp/wifi.fifo"

int WifiFifoCmd( WIFI_FIFO_CMD_EN cmd );
int WifiFifoReStart();

#endif // __WIFIFIFO_H__



