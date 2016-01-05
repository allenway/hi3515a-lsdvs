/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.10.10
**  description  : 
*******************************************************************************
*/

#include "debug.h"
#include "const.h"
#include "osdApply.h"

int FiEncGetOsdResolution( int vencGroup )
{
	return 0;
}

void FitMpiInitOsdChannel()
{
	int i, ret;
	for( i = 0; i < REAL_CHANNEL_NUM; ++i )
    {
    	ret = FiOsdDoKindOfEvent( OSDEVENT_VECREATE, i );
    	SVPrint( "ret(%d) = FiOsdDoKindOfEvent(%d)!\r\n", ret, i );
    }
}

void FitMpiDeinitOsdChannel()
{
	int i, ret;
	for( i = 0; i < REAL_CHANNEL_NUM; ++i )
    {
    	ret = FiOsdDoKindOfEvent( OSDEVENT_VEDESTROY, i );
    	SVPrint( "ret(%d) = FiOsdDoKindOfEvent(%d)!\r\n", ret, i );
    }
}


