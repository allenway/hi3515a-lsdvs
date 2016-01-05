/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 报警公共接口
********************************************************************************/

#include "const.h"
#include "alarm.h"
#include "timeExchange.h"

/*
* 检查当前时间是否处于布防状态
* guard:布防配置
* 放回: 1-当前时间是否处于布防状态; 0-反之
*/
int CheckIsGuard( ALARM_GUARD_SCHEME_T guard )
{
	int ret = FI_FALSE, i, timeStart, timeStop;
	int curTime, timeNum;
    
	if(FI_TRUE == guard.guardHand.guardFlag)
    {
    	ret = FI_TRUE;
    }
	else if(guard.guardTimer.timeNum > 0)
    {
    	curTime = FiTimeUtcToWeekSecond( Time() );
    	timeNum = guard.guardTimer.timeNum;
    	for ( i = 0; i < timeNum; ++i )
        {
        	timeStart	= guard.guardTimer.timeSeg[i].timeStart;
        	timeStop	= guard.guardTimer.timeSeg[i].timeStop; 
        	if ( timeStart <= curTime && curTime < timeStop ) 
            {
            	ret = FI_TRUE;
            	break;
            }
        }    
    }

	return ret;
}

