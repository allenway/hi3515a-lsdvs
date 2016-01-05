/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 管理wifi模块
**  date           :  2013.10.21
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "debug.h"
#include "malloc.h"
#include "linuxFile.h"
#include "paramConfig.h"

#include "wifiCom.h"
#include "wifiFifo.h"
#include "wifiConf.h"
#include "wifi.h"
#include "sysRunTime.h"
#include "paramManage.h"
#include "network.h"
#include "netIp.h"
#include "netMask.h"
#include "netGateway.h"
/*
*	fn	    : 从一行中获取essid
	例如	: ESSID:"lisem1" [7]
*	pLine   : 一行数据
*	lineLen : 改行数据的长度
*	pEssid     : 获取到的id

*/
static void GetEssid( char *pLine, int lineLen, char *pEssid )
{
	int i;
	char *pStrSrc = pLine, *pStrDst = pEssid;    

	i = 0;
	while( *pStrSrc != '"' && i < lineLen ) 
    {
    	pStrSrc++;
    	i++;
    }
	pStrSrc++;

	for( i = 0; i < 32; i++ )
    {
    	if( *pStrSrc == '"' )
        {
        	break;
        }
        *pStrDst++ = *pStrSrc++;        
    }    
}

static const WIFI_SIGNAL_LEVEL_MAP g_wifiSignal[] = 
{
    { -20, WIFI_SIGNAL_LEVEL_5 },
    { -40, WIFI_SIGNAL_LEVEL_4 },
    { -60, WIFI_SIGNAL_LEVEL_3 }, 
    { -80, WIFI_SIGNAL_LEVEL_2 },
    { -100, WIFI_SIGNAL_LEVEL_1 }
};

/*
*	fn	: 从dbm转换成信号的格数
*	dbm	: 被转换的dbm值
*	返回: 转换后的格数
*/
int WifiSignalDbmToLevel( int dbm )
{
	unsigned int i;
	int level = 0;
    
	if( dbm >= 0) return 0;
    
	for( i = 0; i < (sizeof(g_wifiSignal) / sizeof(WIFI_SIGNAL_LEVEL_MAP)); i++ )
    {
    	if( dbm >= g_wifiSignal[i].signalVal )
        {
        	level = g_wifiSignal[i].level;
        	break;
        }
    }

	return level;
}

static int GetSignalLevel( char *pLine )
{ 
	int num, level = 0;
	char *ptr;

	ptr = strstr( pLine, WIFI_MARK_SIGNAL );
	if( NULL != ptr )
    {
    	ptr += strlen( WIFI_MARK_SIGNAL );
    	num = atoi( ptr );
    	level = WifiSignalDbmToLevel( num );
    }

	return level;
}

/*
*	fn: 从搜索结果的文件中获取wifi路由器的信息
*	pScanResultFile : 搜索的文件名
*	pWifiScanResult : out, 结果
*	返回:	搜索到的路由器的个数,如果返回-1表示出错
*/
int GetWifiScanInfo( char *pScanResultFile, WIFI_SCAN_RESULT **pWifiScanResult )
{    
	char *ptr;
	int ret = -1;
	int fd;
	int essidLineLen, offset, routeCount = 0;    
	WIFI_SCAN_RESULT wifiScanResult; 
	char *pResult = NULL;
	char wifiLine[WIFI_SCAN_MAX_LINE][WIFI_SCAN_LINE_SIZE];    
    
	if( NULL == pWifiScanResult || NULL == pScanResultFile )
    {
    	SVPrint( "error:NULL == pWifiScanResult || NULL == pScanResultFile!\r\n" );
    	return -1;
    }

	if( -1 != (fd = open( pScanResultFile, O_RDONLY)) )
    {        
    	while( (essidLineLen = Readline(fd, wifiLine[0], sizeof( wifiLine[0] ))) > 0)
        {            
        	if( (ptr = strstr( wifiLine[0], WIFI_MARK_ESSID)) != NULL )    // 检索ESSID
            {                
            	memset( &wifiScanResult, 0x00, sizeof(wifiScanResult) );
            	if((ret = Readline( fd, wifiLine[1], sizeof(wifiLine[1]) )) > 0) // 获取信号强度
                {
                	wifiScanResult.signalLevel = GetSignalLevel( wifiLine[1] );
                }
            	if( ret > 0 && (ret = Readline( fd, wifiLine[2], sizeof(wifiLine[2]) )) > 0 ) // 查看是否有加密
                {
                	if( strstr( wifiLine[2], WIFI_MARK_KEY_ON ) != NULL )
                    {
                    	wifiScanResult.encType = WIFI_ENC_WEP;
                    }
                }
            	if( ret > 0  && wifiScanResult.encType != WIFI_ENC_NONE
                    && (ret = Readline( fd, wifiLine[3], sizeof(wifiLine[3]) )) > 0 ) // 查看加密方式
                {
                	if( strstr( wifiLine[3], WIFI_MARK_ESSID ) != NULL )
                    {
                    	offset = 0 - ret;
                    	lseek( fd, offset, SEEK_CUR);
                    }
                	else if( strstr( wifiLine[3], WIFI_MARK_WIFI_ENC_WPA ) != NULL )
                    {
                    	wifiScanResult.encType = WIFI_ENC_WPA;
                    }
                	else if( strstr( wifiLine[3], WIFI_MARK_WIFI_ENC_WPAPSK) != NULL )
                    {
                    	wifiScanResult.encType = WIFI_ENC_WPAPSK;
                    }
                }
            	GetEssid( wifiLine[0], essidLineLen, wifiScanResult.essid );

            	FiPrint2( "wifiScanResult.encType = %d!\r\n", wifiScanResult.encType );

            	routeCount++;
            	if( NULL == pResult )
                {
                	pResult = (char *)Malloc( sizeof(WIFI_SCAN_RESULT) );
                }
            	else
                {
                	pResult = (char *)realloc( pResult, sizeof(WIFI_SCAN_RESULT) * routeCount );
                }    
            	memcpy( pResult + sizeof(WIFI_SCAN_RESULT) * (routeCount - 1), 
                                    &wifiScanResult, sizeof(wifiScanResult) );    
            } // if( ptr = strstr( wifiLine[0], WIFI_MARK_ESSID	        
        }
        *pWifiScanResult = (WIFI_SCAN_RESULT *)pResult;
    	close( fd );
    } // if( -1 != (fd == open( pScanResultFile, O_RDONLY))

	return routeCount;
    
}

/*
*	fn : 检索wifi搜索结果
*	pWifiScanResult : out, 存放返回结果的结构体
*	pScanSize	    : out, pWifiScanResult所指向内存的长度
* 	返回	        : 搜索到的路由器的个数,如果出错则返回-1.
*/
int WifiScanResult( WIFI_SCAN_RESULT **pWifiScanResult, int *pScanSize )
{
	int i;
	static int baseTime = -100;
	int curTime;
	int routeCount;
    
	if( NULL == pWifiScanResult || NULL == pScanSize )
    {
    	SVPrint( "error:NULL == pWifiScanResult || NULL == pScanSize!\r\n" );
    	return -1;
    }
    
	curTime = SysRunTimeGet();

	if( curTime - baseTime > WIFI_SCAN_INTERVAL ) 
    {
    	WifiFifoCmd( WIFI_FIFO_CMD_SCAN );
    	baseTime = curTime;
    	for( i = 0; i < 5; i++ )
        {
        	if( 0 == access( WIFI_SCAN_FINISH_FLAG, F_OK) )
            {
            	unlink( WIFI_SCAN_FINISH_FLAG );
            	break;
            }
        	else
            {
            	sleep( 1 );
            }        
        }
    }

	routeCount = GetWifiScanInfo( (char *)WIFI_SCAN_RESULT_FILENAME, pWifiScanResult );
    *pScanSize = routeCount * sizeof( WIFI_SCAN_RESULT );  

	return routeCount;    
}

// 初始化wifi网络
int WifiInitNetwork()
{
	int 	ret;
	PARAM_CONFIG_THREEG_T threeg;
	PARAM_CONFIG_NETWORK network;    
	PARAM_CONFIG_WIFI_CONNECT_T wifiConnect;

	memset( &threeg, 0x00, sizeof(threeg) );            
	ParamGetThreeg( &threeg );
	ret = ParamGetNetwork( &network );    
	if( FI_SUCCESSFUL == ret )
    {
    	ret = WifiSetNetworkInfoToFile( NULL, NULL, network.wifi.gateway );    
    	if( 0 == (ret = ParamGetWifiConnect(&wifiConnect)) )
        {
        	WifiSetConncetInfoToFile( wifiConnect.essid, wifiConnect.key );
        	sync();
        }
        
    	if( 1 == network.wifi.enableFlag )
        {            
        	if( 0 == network.wifi.dhcpEnable ) // 静态 IP
            {            
            	ret = NetSetIpAddr( (char *)NET_WIFI_NAME, network.wifi.ip );
            	if( ret == 0 ) ret = NetSetMask( (char *)NET_WIFI_NAME, network.wifi.netmask );

                //if( 1 == network.wifi.beDefaultGateway && FI_TRUE != threeg.enableFlag ) 
                //{
                //	NetRouteAddDefaultGateway( (char *)NET_WIFI_NAME, network.wifi.gateway );
                //}

            	WifiFifoCmd( WIFI_FIFO_CMD_START );
            }
        	else if( 1 == network.wifi.dhcpEnable ) // 动态 IP
            {                
                //if( 1 == network.wifi.beDefaultGateway ) // 删除其他区默认的网关,使用默认网关
                //{
                //	NetRouteDelDefaultGateway();
                //}
            	WifiFifoCmd( WIFI_FIFO_CMD_START_DHCP );
            }
        }
    }
	return ret;
}



