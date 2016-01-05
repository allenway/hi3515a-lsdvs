/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : GPS一些公共的宏,以及一些对外的公共接口
**  date           :  2014.9.25
**
**  当前版本"1.0
**  author"sven
********************************************************************************/


#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include "const.h"
#include "debug.h"
#include "serial.h"

#include "gpsCom.h"

static GPS_MSG_SPEED 	g_gpsSpeed;
static GPS_MSG_XYZ_ST	g_gpsXyz;
static GPS_MSG_MAGNETIC_DECLINATION_ST 	g_gpsMagDec;

int GpsComInit()
{
	int ret = FI_FAILED;
	int serialFd;
    
	memset( &g_gpsSpeed, 0, sizeof(g_gpsSpeed) );     
	memset( &g_gpsXyz, 0, sizeof(g_gpsXyz) ); 
	memset( &g_gpsMagDec, 0, sizeof(g_gpsMagDec) ); 
    
	pthread_mutexattr_t mutexattr;
	pthread_mutexattr_init( &mutexattr );
	pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_RECURSIVE_NP );
	pthread_mutex_init( &g_gpsSpeed.sync.lock, &mutexattr );
	pthread_mutex_init( &g_gpsXyz.sync.lock, &mutexattr );
	pthread_mutex_init( &g_gpsMagDec.sync.lock, &mutexattr );    
	pthread_mutexattr_destroy( &mutexattr );

	g_gpsSpeed.sync.pts         = -1000;
	g_gpsSpeed.sync.interval     = GPS_MAX_SYNC_INTERVAL;    
    
	g_gpsXyz.sync.pts             = -1000;
	g_gpsXyz.sync.interval         = GPS_MAX_SYNC_INTERVAL;
    
	g_gpsMagDec.sync.pts         = -1000;    
	g_gpsMagDec.sync.interval     = GPS_MAX_SYNC_INTERVAL;
    
	serialFd = FiSerialOpenGps();
	if( serialFd != -1 )
    {
    	ret = FiSerialSetParamThreeg( serialFd, SERIAL_BAUDRATE_115200,
                            	SERIAL_DATABITS_8, SERIAL_STOPBITS_1,SERIAL_PARITY_NONE );
    }
    
	return ret;
}

int GpsComDeInit()
{    
	pthread_mutex_destroy( &g_gpsSpeed.sync.lock );        
	pthread_mutex_destroy( &g_gpsXyz.sync.lock );    
	pthread_mutex_destroy( &g_gpsMagDec.sync.lock );
    
	FiSerialCloseGps();

	return FI_SUCCESSFUL;
}

int GpsSetSpeed( float speed )
{
	pthread_mutex_lock( &g_gpsSpeed.sync.lock );
    
	g_gpsSpeed.speed     = speed;
	g_gpsSpeed.sync.pts = time( NULL );
    
	pthread_mutex_unlock( &g_gpsSpeed.sync.lock );

	return FI_SUCCESSFUL;
}

/********************************************************************
*  pSpeed: out, 返回有效的速度;
*  return  : 0,成功; -1,失败
********************************************************************/
int FiGpsGetSpeed( float *pSpeed )
{
	int ret = FI_FAILED;
	int curTime, interval;
    
	if( NULL == pSpeed )
    {
    	SVPrint( "NULL == pSpeed\r\n" ); 
    	return FI_FAILED;
    }
	pthread_mutex_lock( &g_gpsSpeed.sync.lock );    
	curTime     = time( NULL );
	interval     =  curTime - g_gpsSpeed.sync.pts;
	if( (interval >= 0) && interval < g_gpsSpeed.sync.interval )
    {
        *pSpeed = g_gpsSpeed.speed;
    	ret = FI_SUCCESSFUL;
    }
    
	pthread_mutex_unlock( &g_gpsSpeed.sync.lock );

	return ret;
}

int FiGpsSetXyz( GPS_MSG_XYZ xyz )
{
	pthread_mutex_lock( &g_gpsXyz.sync.lock );
    
	g_gpsXyz.xyz = xyz;
	g_gpsXyz.sync.pts = time( NULL );
    
	pthread_mutex_unlock( &g_gpsXyz.sync.lock );

	return FI_SUCCESSFUL;
}

/***************************************************************
*  function: 获取GPS坐标
*  pXyz: out, 返回有效的坐标;
*  return   : 0,成功; -1,失败
*****************************************************************/
int FiGpsGetXyz( GPS_MSG_XYZ *pXyz )
{
	int ret = FI_FAILED;
	int curTime, interval;
    
	if( NULL == pXyz )
    {
    	SVPrint( "NULL == pXyz\r\n" ); 
    	return FI_FAILED;
    }
	pthread_mutex_lock( &g_gpsXyz.sync.lock );    
	curTime     = time( NULL );
	interval     =  curTime - g_gpsXyz.sync.pts;
	if( (interval >= 0) && interval < g_gpsXyz.sync.interval )
    {
        *pXyz = g_gpsXyz.xyz;
    	ret = FI_SUCCESSFUL;
    }
    
	pthread_mutex_unlock( &g_gpsXyz.sync.lock );

	return ret;
}

int FiGpsSetMagDec( GPS_MSG_MAGNETIC_DECLINATION magDec )
{
	pthread_mutex_lock( &g_gpsMagDec.sync.lock );
    
	g_gpsMagDec.magDec = magDec;
	g_gpsMagDec.sync.pts = time( NULL );
    
	pthread_mutex_unlock( &g_gpsMagDec.sync.lock );

	return FI_SUCCESSFUL;
}

/**********************************************************************
*  function: 获取GPS磁偏角
*  pMagDec: out, 返回有效的磁偏角;
*  return   : 0,成功; -1,失败
**********************************************************************/
int FiGpsGetMagDec( GPS_MSG_MAGNETIC_DECLINATION *pMagDec )
{
	int ret = FI_FAILED;
	int curTime, interval;
    
	if( NULL == pMagDec )
    {
    	SVPrint( "NULL == pMagDec\r\n" ); 
    	return FI_FAILED;
    }
    
	pthread_mutex_lock( &g_gpsMagDec.sync.lock );    
	curTime     = time( NULL );
	interval     =  curTime - g_gpsMagDec.sync.pts;
	if( (interval >= 0) && interval < g_gpsMagDec.sync.interval )
    {
        *pMagDec     = g_gpsMagDec.magDec;
    	ret         = FI_SUCCESSFUL;
    }    
	pthread_mutex_unlock( &g_gpsMagDec.sync.lock );

	return ret;
}

