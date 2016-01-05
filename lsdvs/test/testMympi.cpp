#if 0
#include "hicomm.h"
#include "const.h"
#include "mympi.h"

void TestMympi()
{
	int i;

	for( i = 0; i < REAL_CHANNEL_NUM; ++i )
    {
    	MympiSetLevel( i, 3 );
    	MympiSetBitrateType( i, RC_MODE_VBR );
    	MympiSetBitrate( i, 534 );
    	MympiSetFramerate( i, 20 );
    	MympiSetIframeInterval( i, 10 );
    }
}

// ÄÚÈÝ±¸·Ý
int FiOpenVideoLed(int channel);
int FiCloseVideoLed(int channel);

static VIDEO_LED_STATUS g_videoLed;
/*
* ´ò¿ªÂ¼ÏñÖ¸Ê¾µÆ
*/
int FiOpenVideoLed(int channel)
{
	int ret = FI_FAILED;
	if(channel>=0 && channel<=REAL_CHANNEL_NUM)
    {
    	g_videoLed.recStatus[channel] = 1;
    	if(FI_FALSE == g_videoLed.openFlag)
        {            
//        	ret = OpenLedVideo();
        	if(FI_SUCCESSFUL == ret) g_videoLed.openFlag = FI_TRUE;
        }
    }

	return ret;
}

/*
* ¹Ø±ÕÂ¼ÏñÖ¸Ê¾µÆ
*/
int FiCloseVideoLed(int channel)
{
	int i;
	int ret = FI_FAILED;
	int closeFlag = 1;
    
	if(channel>=0 && channel<=REAL_CHANNEL_NUM)
    {
    	g_videoLed.recStatus[channel] = 0;
        
    }
	for(i=0;i<REAL_CHANNEL_NUM;i++)
    {
    	if(g_videoLed.recStatus[i] != 0)
        {
        	closeFlag = 0;
        	break;
        }
    }
	if(1==closeFlag && FI_TRUE==g_videoLed.openFlag)
    {
//    	ret = CloseLedVideo();
    	g_videoLed.openFlag = FI_FALSE; 
    }

	return ret;
}

#endif 

