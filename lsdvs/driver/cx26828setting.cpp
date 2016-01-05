
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "cx26828.h"
#include <sys/ioctl.h>

#include "cx26828setting.h"  
#include "debug.h"

int set_image_parameter_cx26828(int video_fd, int chn, U8 brightness, U8 contrast, U8 hue, U8 saturation)
{
    cx26828_image_adjust stImageAdjust;

	if( chn < 0 || chn >= 4 ) 
    {
    	return -1;
    }

	stImageAdjust.chip	        = 0;
	stImageAdjust.chn	        = chn;
	stImageAdjust.brightness    = brightness;
	stImageAdjust.contrast      = contrast;
	stImageAdjust.saturation    = saturation;
	stImageAdjust.hue           = hue;
    stImageAdjust.sharpness     = 128;
	stImageAdjust.item_sel	    = CX26828_SET_HUE|CX26828_SET_CONTRAST|CX26828_SET_BRIGHT|CX26828_SET_SATURATION;

    if( ioctl(video_fd, _IOC_NR(CX26828_SET_IMAGE_ADJUST), &stImageAdjust) )
    {
    	ERRORPRINT( "set CX26828(%d) video mode fail\r\n", chn );
        return -1;
    }

	return 0;
}

int get_image_parameter_cx26828(int video_fd, int chn, U8 *brightness, U8 *contrast, U8 *hue, U8 *saturation)
{
    cx26828_image_adjust stImageAdjust;

	if( chn < 0 || chn >= 4 ) 
    {
    	return -1;
    }

	stImageAdjust.chip	        = 0;
	stImageAdjust.chn	        = chn;
	stImageAdjust.item_sel	    = CX26828_SET_HUE|CX26828_SET_CONTRAST|CX26828_SET_BRIGHT|CX26828_SET_SATURATION;

    if( ioctl(video_fd, _IOC_NR(CX26828_GET_IMAGE_ADJUST), &stImageAdjust) )
    {
    	ERRORPRINT( "get CX26828(%d) video mode fail\r\n", chn );
        return -1;
    }
    *brightness = stImageAdjust.brightness;
    *contrast = stImageAdjust.contrast;
    *hue = stImageAdjust.hue;
    *saturation = stImageAdjust.saturation;

	return 0;
}

int get_video_loss_cx26828(int video_fd, int chn)
{
    FiPrint2("notice:cx26828 chip is not here !\n");
    return 0;
    
	cx26828_video_loss loss;

	if ( (chn<0) || (chn>3) ) 
    {
        return -1;
    }   

	loss.chip = 0;
    loss.ch = chn;
    if ( ioctl(video_fd, _IOC_NR(CX26828_GET_VIDEO_LOSS), &loss) )
    {
    	ERRORPRINT("get cx26828(%d) video loss fail\n", chn);
        return -1;
    }

	if (loss.is_lost == 0) return 0;
	else return 1;

}

int get_video_norm_cx26828(int video_fd, int chn)
{
    cx26828_video_norm stVideoMode;
    
	if (chn<0||chn>3) 
    {
        return -1;
    }   
        

	stVideoMode.chip = 0;
    if (ioctl(video_fd, _IOC_NR(CX26828_GET_VIDEO_NORM), &stVideoMode))
    {
    	ERRORPRINT("get CX26828(%d) video mode fail\n", chn);
        return -1;
    }
    
	return stVideoMode.mode;
}

int set_video_norm_cx26828(int video_fd, int chn, int mode)
{
    cx26828_video_norm stVideoMode;
    
	if (chn<0||chn>3)
    {
        return -1;
    }
    
	if ( !(mode == CX26828_MODE_PAL) || (mode == CX26828_MODE_NTSC) ) 
    {
        return -1;
    }

	stVideoMode.chip    = 0;
    stVideoMode.mode    = mode;
    if (ioctl(video_fd, _IOC_NR(CX26828_SET_VIDEO_NORM), &stVideoMode))
    {
    	ERRORPRINT("set CX26828(%d) video mode fail\n", chn);
        return -1;
    }

	return 0;
}