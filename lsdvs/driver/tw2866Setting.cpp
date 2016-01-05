#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tw2865.h"
#include <sys/ioctl.h>

#define TW2865_FILE     "/dev/tw2865dev"  
    
typedef unsigned char U8; 

/*  set_video_norm 设置视频制式 NTSC PAL
    @int chn 
    	取值0 1号通道
    	取值1 2号通道
    	取值2 3号通道
    	取值3 4号通道

    @int mode
    	取值  TW2865_PAL
    	取值  TW2865_NTSC
*/

int set_video_norm(int video_fd, int chn, int mode)
{
    tw2865_video_norm stVideoMode;
	if (chn<0||chn>3) return -1;
	if ( (mode<TW2865_NTSC) || (mode>TW2865_AUTO) ) return -1;

	stVideoMode.chip    = 0;
    stVideoMode.mode    = mode;
    if (ioctl(video_fd, TW2865_SET_VIDEO_NORM, &stVideoMode))
    {
    	printf("set tw2865(%d) video mode fail\n", chn);
        return -1;
    }

	return 0;
}


/*
* return : mode 
*/
int get_video_norm(int video_fd, int chn)
{
    tw2865_video_norm stVideoMode;
	if (chn<0||chn>3) return -1;

	stVideoMode.chip    = 0;
    if (ioctl(video_fd, TW2865_GET_VIDEO_NORM, &stVideoMode))
    {
    	printf("get tw2865(%d) video mode fail\n", chn);
        return -1;
    }
    
	return stVideoMode.mode;
}


/*
*  fn:  获得视频丢失信息
*   chn: 通道号  0---3
*  return: 0:正常   1:丢失   -1:错误
*/
int get_video_loss(int video_fd, int chn)
{
	tw2865_video_loss loss;

	if ( (chn<0) || (chn>3) ) return -1;

	loss.chip = 0;
    loss.ch = chn;
    if ( ioctl(video_fd, TW2865_GET_VIDEO_LOSS, &loss) )
    {
    	printf("set tw2865(%d) video mode fail\n", chn);
        return -1;
    }

	if (loss.is_lost == 0) return 0;
	else return 1;

}



/* set_image_parameter 
	U8 brightness      0-255
	U8 contrast         0-255
	U8 hue               0-255
	U8 saturation      0-255
*/

int set_image_parameter(int video_fd, int chn, U8 brightness, U8 contrast, U8 hue, U8 saturation)
{
    tw2865_image_adjust stImageAdjust;

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
	stImageAdjust.item_sel	    = TW2865_SET_HUE|TW2865_SET_CONTRAST|TW2865_SET_BRIGHT|TW2865_SET_SATURATION;

    if( ioctl(video_fd, TW2865_SET_IMAGE_ADJUST, &stImageAdjust) )
    {
    	printf( "set tw2865(%d) video mode fail\r\n", chn );
        return -1;
    }

	return 0;
}


int get_image_parameter(int video_fd, int chn, U8 *brightness, U8 *contrast, U8 *hue, U8 *saturation)
{
    tw2865_image_adjust stImageAdjust;

	if( chn < 0 || chn >= 4 ) 
    {
    	return -1;
    }

	stImageAdjust.chip	        = 0;
	stImageAdjust.chn	        = chn;
	stImageAdjust.item_sel	    = TW2865_SET_HUE|TW2865_SET_CONTRAST|TW2865_SET_BRIGHT|TW2865_SET_SATURATION;

    if( ioctl(video_fd, TW2865_GET_IMAGE_ADJUST, &stImageAdjust) )
    {
    	printf( "get tw2865(%d) video mode fail\r\n", chn );
        return -1;
    }
    *brightness = stImageAdjust.brightness;
    *contrast = stImageAdjust.contrast;
    *hue = stImageAdjust.hue;
    *saturation = stImageAdjust.saturation;

	return 0;
}



#if 0
int main()
{
	int fd;
	fd = open(TW2865_FILE, O_RDWR);
    if (fd < 0)
    {
        printf("open %s fail\n", TW2865_FILE);
        return -1;
    }

	printf("Setting Video Parameter ... !\n");
	set_image_parameter(fd, 0, 128, 128, 128, 128);
	sleep(3);
	close(fd);
    
	return 0;
}
#endif

//视频丢失 参考sample_common.c中的SampleVLossDetProc 视频丢失后关闭MP 可以省电



