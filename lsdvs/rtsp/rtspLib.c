#include "rtspLib.h"
#include "rtspSLib.h"

int st_rtsp_startRtspService()
{
    //return 0;
	return rtspServerStart( 554 );
}

int st_rtsp_stopRtspService()
{
	return rtspServerStop();
}

int RtspServiceStart()
{
	return st_rtsp_startRtspService();
}

int RtspServiceStop()
{
	return st_rtsp_stopRtspService();
}


