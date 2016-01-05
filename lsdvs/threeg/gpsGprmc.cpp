/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 解析 GPRMC 信息
$GPRMC,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>,<12>*hh<CR><LF>
<1> UTC时间，hhmmss（时分秒）格式
<2> 定位状态，A=有效定位，V=无效定位
<3> 纬度ddmm.mmmm（度分）格式（前面的0也将被传输）
<4> 纬度半球N（北半球）或S（南半球）
<5> 经度dddmm.mmmm（度分）格式（前面的0也将被传输）
<6> 经度半球E（东经）或W（西经）
<7> 地面速率（000.0~999.9节，前面的0也将被传输）
<8> 地面航向（000.0~359.9度，以真北为参考基准，前面的0也将被传输）
<9> UTC日期，ddmmyy（日月年）格式
<10> 磁偏角（000.0~180.0度，前面的0也将被传输）
<11> 磁偏角方向，E（东）或W（西）
<12> 模式指示（仅NMEA0183 3.00版本输出，A=自主定位，D=差分，E=估算，N=数据无效）

**  date           :  2014.9.25
**
**  version:1.0
**  author :sven
********************************************************************************/
#include <string>
#include "const.h"
#include "debug.h"
#include "gpsCom.h"
#include "gpsGprmc.h"
#include "sysRunTime.h"
#include "timeExchange.h"

/*
*	func: 解析gprmc源消息成本地消息
*	pGprmc: 源消息
* 	pLocal: out, 转化后的本地消息
*/
static int GpsGprmcOrgToLocal( char *pGprmc, GPS_GPRMC_MSG_ST *pLocal )
{
	char     *pSearch;
	int 	count = -1;
	GPS_GPRMC_MSG_ST gprmcMsg;
    
	memset( &gprmcMsg, 0x00, sizeof(gprmcMsg) );
	if( NULL == pGprmc || NULL == pLocal )    
    {
    	SVPrint( "NULL == pGprmc || NULL == pLocal!\r\n" );
    	return FI_FAILED;
    }
    
	while( NULL != (pSearch = strsep(&pGprmc, GPS_MSG_COMMA)) )
    {
    	count++;
    	if( '\0' == pSearch[0] ) continue; // 某个信息缺失了
        
    	switch( count )
        {
        	case GPS_GPRMC_MSG_HEAD:
            	gprmcMsg.mark |= GPS_GPRMC_MARK_HEAD;
            	break;
        	case GPS_GPRMC_MSG_TIME:
            	sscanf( pSearch, "%s", gprmcMsg.time );
            	gprmcMsg.mark |= GPS_GPRMC_MARK_TIME;
            	break;            
        	case GPS_GPRMC_MSG_STATE:    
            	sscanf( pSearch, "%c", &gprmcMsg.state );                
            	gprmcMsg.mark |= GPS_GPRMC_MARK_STATE;
            	break;            
        	case GPS_GPRMC_MSG_LATITUDE:    
            	sscanf( pSearch, "%f", &gprmcMsg.latitude );                
            	gprmcMsg.mark |= GPS_GPRMC_MARK_LATITUDE;
            	break;            
        	case GPS_GPRMC_MSG_LATITUDE_NS:        
            	sscanf( pSearch, "%c", &gprmcMsg.latitudeNs );                
            	gprmcMsg.mark |= GPS_GPRMC_MARK_LATITUDE_NS;
            	break;    
        	case GPS_GPRMC_MSG_LONGITUDE:        
            	sscanf( pSearch, "%f", &gprmcMsg.longitude );                
            	gprmcMsg.mark |= GPS_GPRMC_MARK_LONGITUDE;
            	break;    
        	case GPS_GPRMC_MSG_LONGITUDE_EW:
            	sscanf( pSearch, "%c", &gprmcMsg.longitudeEw );                
            	gprmcMsg.mark |= GPS_GPRMC_MARK_LONGITUDE_EW;
            	break;    
        	case GPS_GPRMC_MSG_SPEED:    
            	sscanf( pSearch, "%f", &gprmcMsg.speed );
            	gprmcMsg.mark |= GPS_GPRMC_MARK_SPEED;
            	break;    
        	case GPS_GPRMC_MSG_DIRECTION:    
            	sscanf( pSearch, "%f", &gprmcMsg.direction);
            	gprmcMsg.mark |= GPS_GPRMC_MARK_DIRECTION; 
            	break;    
        	case GPS_GPRMC_MSG_DATE:    
            	sscanf( pSearch, "%s", gprmcMsg.date );
            	gprmcMsg.mark |= GPS_GPRMC_MARK_DATE; 
            	break;        
        	case GPS_GPRMC_MSG_MAGNETIC_DECLINATION:    
            	sscanf( pSearch, "%f", &gprmcMsg.magneticDeclination );
            	gprmcMsg.mark |= GPS_GPRMC_MARK_MAGNETIC_DECLINATION; 
            	break;    
        	case GPS_GPRMC_MSG_MAGNETIC_DECLINATION_EW:        
            	sscanf( pSearch, "%c", &gprmcMsg.magneticDeclinationEw );
            	gprmcMsg.mark |= GPS_GPRMC_MARK_MAGNETIC_DECLINATION_EW; 
            	break;
        	case GPS_GPRMC_MSG_MODE:    
            	sscanf( pSearch, "%s", gprmcMsg.mode );
            	gprmcMsg.mark |= GPS_GPRMC_MARK_MODE;
            	break;
        	default:                
            	break;
        }
    } // end while()

    *pLocal = gprmcMsg;

	return FI_SUCCESSFUL;
}

static int SyncGpsTimeToRtc( char *gpsTime, char *gpsData )
{
	int zone = 8;
	int YY, MM, DD, hh, mm, ss, second;
	static int baseTime = -1;
	int curTime = SysRunTimeGet();
    
	if( -1 == baseTime )
    {
    	baseTime = SysRunTimeGet();
    }
	else
    {    
    	if( curTime - baseTime >= TEN_MINUTE )
        {
        	baseTime = curTime;
            
        	SVPrint("#sync gps time = %ld\n", curTime);
            
        	sscanf( gpsTime, "%02d%02d%02d", &hh, &mm, &ss );            
        	sscanf( gpsData, "%02d%02d%02d", &DD, &MM, &YY );
        	YY += 2000;
        	second = FiTimeHumanToUtc( YY, MM, DD, hh, mm, ss );
        	second += ( zone * 3600 );
        	FiTimeUtcToHuman( second, &YY, &MM, &DD, &hh, &mm, &ss );
            // TODO
            //SysSetRealTime( zone, YY, MM, DD, hh, mm, ss );        
        }
    }

	return 0;
}


int GpsGprmcParseLocalMsg( GPS_GPRMC_MSG_ST local )
{    
	float         	speed;
	GPS_MSG_XYZ 	xyz;
	GPS_MSG_MAGNETIC_DECLINATION magDec;
    
    // 同步GPS时间
	if( GPS_GPRMC_MARK_TIME_MIX == (local.mark & GPS_GPRMC_MARK_TIME_MIX) )
    {        
    	SyncGpsTimeToRtc( local.time, local.date );        
    }
    // 同步GPS速度
	if( GPS_GPRMC_MARK_SPEED_MIX == (local.mark & GPS_GPRMC_MARK_SPEED_MIX) )
    {
    	speed = local.speed;
    	GpsSetSpeed( speed );
    }
    // 同步GPS坐标
	if( GPS_GPRMC_MARK_XYZ_MIX == (local.mark & GPS_GPRMC_MARK_XYZ_MIX) )
    {
        //SVPrint("###gps set xyz.latitude = %f, xyz.longitude = %f\n", xyz.latitude, xyz.longitude);
    	xyz.latitude     = local.latitude;
    	xyz.latitudeNs     = local.latitudeNs;
    	xyz.longitude     = local.longitude;
    	xyz.longitudeEw = local.longitudeEw;
    	FiGpsSetXyz( xyz );
    }
    // 同步GPS磁偏角
	if( GPS_GPRMC_MARK_MAG_DEC_MIX == (local.mark & GPS_GPRMC_MARK_MAG_DEC_MIX) )
    {
    	magDec.magneticDeclination         = local.magneticDeclination;        
    	magDec.magneticDeclinationEw	= local.magneticDeclinationEw;
    	FiGpsSetMagDec( magDec );
    }    
    
	return FI_SUCCESSFUL;
}

int FiGpsParseGprmc( char *pGprmc )
{
	GPS_GPRMC_MSG_ST local;
	int ret;

	memset( &local, 0x00, sizeof(local) );

	if( NULL == pGprmc )    
    {
    	SVPrint( "NULL == pGprmc || NULL == pLocal!\r\n" );
    	return FI_FAILED;
    }

	ret = GpsGprmcOrgToLocal( pGprmc, &local );
	if( FI_SUCCESSFUL == ret ) ret = GpsGprmcParseLocalMsg( local );

	return ret;
}

