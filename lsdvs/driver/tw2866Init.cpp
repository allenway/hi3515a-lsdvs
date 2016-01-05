/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 初始化AD采集芯片
*******************************************************************************
*/

#include "paramManage.h"
#include "driver.h"
#include "vencParamEasy.h"
#include "tw2865.h"
#include "hicomm.h"

int Tw2866InitVideoBaseParam()
{
	int i, ret;
	PARAM_CONFIG_VIDEO_BASE_PARAM vpparam;

	for( i = 0; i < REAL_CHANNEL_NUM; ++i )
    {
    	ret = ParamGetVideoBaseParam( i, &vpparam );
    	if( 0 == ret )
        {
        	DriverSetAdVideoCapParam( i, vpparam.brightness, vpparam.contrast,
                                	vpparam.hue, vpparam.saturation );
        }
    }

	return 0;
}


int Tw2866InitVideoMode()
{
#if 1
	int ret;
	PARAM_CONFIG_VIDEO_ENCODE_PUBLIC param;
	uchar standard;

	standard = TW2865_PAL;
	ret = ParamGetVideoEncodePublic( &param );
	if ( FI_SUCCESS == ret ) 
    {
    	if ( VIDEO_ENCODING_MODE_PAL == param.videoStandard ) standard = TW2865_PAL;
    	else if ( VIDEO_ENCODING_MODE_NTSC == param.videoStandard ) standard = TW2865_NTSC;
    	else if ( VIDEO_ENCODING_MODE_AUTO == param.videoStandard ) standard = TW2865_AUTO;        
    	DriverSetAdVideoMode( 0, standard );
    	standard = (VIDEO_ENCODING_MODE_NTSC == param.videoStandard) ? (VIDEO_ENCODING_MODE_NTSC) : (VIDEO_ENCODING_MODE_PAL);
    	VencParamEasySetVideoStandard( standard );
    }

	return 0;
#endif	
}



