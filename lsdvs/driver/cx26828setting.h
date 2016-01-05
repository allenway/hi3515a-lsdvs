#ifndef __CX26828_SETTING_H__
#define __CX26828_SETTING_H__

typedef unsigned char U8; 

int set_image_parameter_cx26828(int video_fd, int chn, U8 brightness, U8 contrast, U8 hue, U8 saturation);
int get_image_parameter_cx26828(int video_fd, int chn, U8 *brightness, U8 *contrast, U8 *hue, U8 *saturation);
int get_video_loss_cx26828(int video_fd, int chn);
int get_video_norm_cx26828(int video_fd, int chn);
int set_video_norm_cx26828(int video_fd, int chn, int mode);


#endif
