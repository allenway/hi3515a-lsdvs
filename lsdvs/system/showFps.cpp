/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 显示帧率
********************************************************************************/

#include "const.h"
#include "debug.h"
#include "sysRunTime.h"

/*************************************************************************
* function: 计算帧率
* message: 显示帧率前的内容
* channel: 计算那个通道
**************************************************************************/
int ShowFps( const char *message, int channel )
{
	static int		nFps	                    = 0;
    static int		fps[MAX_CHANNEL_NUM]        = { 0 };
    static int		lastTime[MAX_CHANNEL_NUM]    = { 0 };
    int	        	curTime;
	int         	difTime;

	curTime = SysRunTimeGet();
    if ( channel < MAX_CHANNEL_NUM )
    {
    	fps[channel]++;
        
    	difTime = curTime - lastTime[channel];
       	if ( difTime < 0 || difTime > 60 )
        {
        	lastTime[channel] = curTime;
        	fps[channel] = 1;
        }
        else
        if ( difTime >= 10 ) 
        {
            nFps = (fps[channel] * 10) / difTime;
            fps[channel] = 0;
            lastTime[channel] = curTime;
            if ( message != NULL ) Print( "%s, ", message );
            Print( "Channel:%d, Fps:%d.%d\r\n", channel, nFps/10, nFps%10 );
        }
    }
	return nFps/10;
}

