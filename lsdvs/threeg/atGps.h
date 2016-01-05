/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : AT GPS控制话接口
**  date           :  2014.9.25
**
**  version       :  1.0
**  author        :  sven
********************************************************************************/

#ifndef __ATGPS_H__
#define __ATGPS_H__

#define ATCMD_GSP_START_KEY	    "^GPSSTART"
#define ATCMD_GSP_STOP_KEY	    "^GPSSTOP"


typedef enum _AtGps_
{
	AT_GPS_OFF = 0,        // 关闭GPS
	AT_GPS_ON	        // 启动GPS
} AT_GPS_EN;

int FiAtGpsCtl( AT_GPS_EN flag );    
    
#endif

