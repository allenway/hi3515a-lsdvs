/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 针对3G模块应用对串口接口函数做进一步封装
**  date           :  2014.9.25
**
**  version       :  1.0
**  author        :  sven
********************************************************************************/
#include "const.h"
#include "debug.h"
#include "netSocket.h"
#include "serial.h"


/*****************************************************************
*  function : add timeout function
*****************************************************************/
#define SELECT_THREEG_SERIAL_TIMEOUT_MS		500
static int SecectThreegSerial( int type )
{
	int fd = FiSerialGetHandle( type );
	if( fd < 0 ) return -1;

	return SelectRead( fd, SELECT_THREEG_SERIAL_TIMEOUT_MS );    
}


static int ReadThreegSerial( int type, char *pBuf, int len )
{
	int ret = FI_FAILED;
    
	if( SERIAL_TYPE_THREEG_AT == type )
    {
    	ret = FiSerialReadThreeg( pBuf, len );
    }
	else if( SERIAL_TYPE_GPS == type )
    {
    	ret = FiSerialReadGps( pBuf, len );
    }

	return ret;
}

/*********************************************************************
*  function: 从串口中接收数据。
*  type	: 见SERIAL_TYPE_EN。
*  pBuf	: 存放接收到的数据。
*  len	: 要接收多少数据。
*  return : 实际接收到的数据。
**************************************************************************/
int FiThreegSerialRecvData( int type, char *pBuf, int len )
{
	int ret;
	if( NULL == pBuf ) 
    {
    	SVPrint( "NULL == pBuf\r\n" );
    	return -1;
    }
    
	ret = SecectThreegSerial( type );
	if( ret > 0 )
    {
    	ret = ReadThreegSerial( type, pBuf, len );
    }

	return ret;
}

