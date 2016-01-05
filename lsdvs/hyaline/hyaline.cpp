/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 透明传输//rs232
*******************************************************************************
*/
#include "serial.h"
#include "thread.h"
#include "debug.h"
#include "netSocket.h"
#include "hyaline.h"
#include "message.h"

#define HYALINE_BUF_SIZE	1024
static THREAD_MAINTAIN_T g_hyalineTm;
static void *HyalineThread( void *arg )
{
	int ttyFd, ret;
	char buf[HYALINE_BUF_SIZE];

	ttyFd = FiSerialOpenRS232();
	if( -1 == ttyFd )
    {
    	SVPrint( "failed: FiSerialOpenRS232()!\r\n" );
    	return NULL;
    }

	ret = FiSerialSetParam( ttyFd, SERIAL_BAUDRATE_9600, SERIAL_DATABITS_8,
                            	SERIAL_STOPBITS_1, SERIAL_PARITY_NONE );
	if( 0 != ret )
    {
    	SVPrint( "failed: FiSerialSetParam()!\r\n" );
    	return NULL;
    }
	SVPrint( "%s start!\r\n", __FUNCTION__ );

	while( g_hyalineTm.runFlag )
    {
    	ret = SelectRead( ttyFd, 2 );
    	if( ret > 0 )
        {
        	ret = FiSerialReadRS232( buf, sizeof(buf) );
        	buf[ret] = '\0';
        	MessageSend( MSG_ID_DTU_HYALINE, buf, ret );
        }
    }
    
	SVPrint( "%s stop!\r\n", __FUNCTION__ );
	return NULL;
}

void StartHyalineThread()
{
	int ret;
	g_hyalineTm.runFlag = 1;
	ret = ThreadCreate( &g_hyalineTm.id, HyalineThread, NULL );
	if( 0!= ret )
    {        
    	g_hyalineTm.runFlag = 0;
    	SVPrint( "error:ThreadCreate:%s\r\n", STRERROR_ERRNO );
    }
}

void StopHyalineThread()
{
	int ret;
	g_hyalineTm.runFlag = 0;
	ret = ThreadJoin( g_hyalineTm.id, NULL );
	if( 0 != ret )
    {
    	SVPrint( "error:ThreadJoin:%s\r\n", STRERROR_ERRNO );
    }
}

