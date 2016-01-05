/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
    
**  description  : GPS一些公共的宏,以及一些对外的公共接口
**  date           :  2014.9.25
**  GPS信息:
	$GPGSA,A,3,03,11,13,19,23,24,,,,,,,3.5,2.0,2.8*32
	$GPVTG,,T,,M,0.0,N,0.0,K*4E
	$GPGSV,3,1,10,16,22,071,,06,14,043,,03,26,039,33,08,21,323,21*78
	$GPGSV,3,2,10,11,61,170,34,07,49,323,24,24,38,167,38,19,54,018,27*72
	$GPGSV,3,3,10,23,21,199,38,13,33,227,35*73
	$GPGGA,062131.0,2232.039330,N,11401.093446,E,1,07,1.7,57.5,M,,,,*3C
	$GPRMC,062131.0,A,2232.039330,N,11401.093446,E,,,110811,,,A*63
**  version:1.0
**  author :sven
********************************************************************************/

#ifndef __GPSCOM_H__
#define __GPSCOM_H__

#include <pthread.h>

#define GPS_MSG_GPGSV_KEY	"GPGSV"        // 可见卫星信息关键字
#define GPS_MSG_GPGLL_KEY     "GPGLL"        // 地理定位信息关键字
#define GPS_MSG_GPRMC_KEY     "GPRMC"         // 推荐最小定位信息关键字
#define GPS_MSG_GPVTG_KEY     "GPVTG"        // 地面速度信息关键字
#define GPS_MSG_GPGGA_KEY     "GPGGA"        // GPS定位信息关键字
#define GPS_MSG_GPGSA_KEY     "GPGSA"        // 当前卫星信息关键字

#define GPS_MSG_COMMA	","        // 逗号分隔符
#define GPS_MAX_SYNC_INTERVAL	300 // 有效的GPS同步时间间隔(秒)

typedef struct _GpsMsgSync_
{
	pthread_mutex_t		lock;
	int pts;                    // 时间戳,用来判断信息的有效性
	int interval;                // 有效的时间间隔,超过该时间无效,单位(秒)
} GPS_MSG_SYNC;

// GPS速度
typedef struct _GpsMsgSpeed_
{
	GPS_MSG_SYNC 	sync;
	float	    	speed;    // 单位(节)
} GPS_MSG_SPEED;

// GPS坐标
typedef struct _GpsMsgXyz_
{    
	float	    	latitude;        // 纬度 ddmm.mmmm（度分）
	char	    	latitudeNs;        // 纬度半球, == 'N'为北半球, == 'S'为南半球。
	float	    	longitude;        // 经度 ddmm.mmmm（度分）
	char	    	longitudeEw;    // 经度半球, == 'E'为东半球, 'W'为西半球
} GPS_MSG_XYZ;
typedef struct _GpsMsgXyzSt_
{
	GPS_MSG_SYNC 	sync;
	GPS_MSG_XYZ		xyz;
} GPS_MSG_XYZ_ST;

// GPS磁偏角
typedef struct _GpsMsgMagneticDeclination_
{
    
	float	    	magneticDeclination;        // 000.0~180.0度
	char	    	magneticDeclinationEw;        // 磁偏角方向，E（东）或W（西）
} GPS_MSG_MAGNETIC_DECLINATION;
typedef struct _GpsMsgMagneticDeclinationSt_
{
	GPS_MSG_SYNC 	sync;
	GPS_MSG_MAGNETIC_DECLINATION magDec;
} GPS_MSG_MAGNETIC_DECLINATION_ST;

int GpsComInit();
int GpsComDeInit();

// set 部分函数供模块内部使用
int GpsSetSpeed( float speed );
int FiGpsSetXyz( GPS_MSG_XYZ xyz );
int FiGpsSetMagDec( GPS_MSG_MAGNETIC_DECLINATION magDec );

// get 部分函数供所有模块调用
int FiGpsGetSpeed( float *pSpeed );
int FiGpsGetXyz( GPS_MSG_XYZ *pXyz );
int FiGpsGetMagDec( GPS_MSG_MAGNETIC_DECLINATION *pMagDec );

#endif 

