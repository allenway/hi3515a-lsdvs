#if 0
#include "debug.h"
#include "mpiApp.h"
#include "mympi.h"
#include "paramManage.h"
#include "hicomm.h"

void TestEncSetResolution()
{
	static int count = 0;
	int ret;
	PARAM_CONFIG_VIDEO_ENCODE_PUBLIC vepp;

	vepp.videoStandard = VIDEO_ENCODING_MODE_NTSC;

	if( 0 == count )
    {
    	ParamSetVideoEncodePublic( &vepp );    
    	MympiSetVideoStandard();
    }
}
#endif

