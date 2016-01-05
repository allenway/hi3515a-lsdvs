/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 管理3G服务
**  date           :  2014.9.25
**
**  version       :  1.0
**  author        :  sven
********************************************************************************/

#include "debug.h"
#include "serial.h"
#include "const.h"

#include "threegAt.h"
#include "threegGps.h"
#include "gpsCom.h"

static int GpsStartViaLicence()
{    
	int ret = FI_FALSE;    
    
	return ret;    
}

void ThreegStartService()
{    
	int ret;

	ret = GpsComInit();
	if( ret != 0 ) 
    {
    	SVPrint( "error:GpsComInit failed,please check dev!\r\n" );
    	return;
    }    

	ThreegAtStartService();    
	ThreegAtInit();
	if( FI_TRUE == GpsStartViaLicence() )
    	ThreegGpsStartService();
        
}

void ThreegStopService()
{

	ThreegAtStopService();
    
	if( FI_TRUE == GpsStartViaLicence() )
    	ThreegGpsStopService();
}

