/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.10.10
**  description  : 把一些应用层用到的海思的宏、枚举共享出来
********************************************************************************/
#ifndef __HI_COMMON_H__
#define __HI_COMMON_H__

// 编码分辨率
typedef enum hiPIC_SIZE_E
{
    PIC_QCIF = 0,
    PIC_CIF,    
    PIC_2CIF,    
    PIC_HD1, 
    PIC_D1,
    
    PIC_QVGA,    /* 320 * 240 */
    PIC_VGA,     /* 640 * 480 */    
    PIC_XGA,     /* 1024 * 768 */   
    PIC_SXGA,    /* 1400 * 1050 */    
    PIC_UXGA,    /* 1600 * 1200 */    
    PIC_QXGA,    /* 2048 * 1536 */

    PIC_WVGA,    /* 854 * 480 */
    PIC_WSXGA,   /* 1680 * 1050 */      
    PIC_WUXGA,   /* 1920 * 1200 */
    PIC_WQXGA,   /* 2560 * 1600 */
    
    PIC_HD720,   /* 1280 * 720 */
    PIC_HD1080,  /* 1920 * 1080 */
    
    PIC_BUTT
}PIC_SIZE_E;

// 视频制式
typedef enum hiVIDEO_NORM_E
{
	VIDEO_ENCODING_MODE_PAL=0,
	VIDEO_ENCODING_MODE_NTSC,
	VIDEO_ENCODING_MODE_AUTO,
	VIDEO_ENCODING_MODE_BUTT
} VIDEO_NORM_E;

// 码率类型
typedef enum hiRC_MODE_E
{
    RC_MODE_VBR = 0,    /* VBR must be 0 for compatible with 3511 */
    RC_MODE_CBR,
    RC_MODE_ABR,
    RC_MODE_FIXQP,
    RC_MODE_BUTT,
} RC_MODE_E;

#endif // __HI_COMMON_H__

#ifndef __HI_COMM_VENC_H__
#define __HI_COMM_VENC_H__

/*the nalu type of H264E*/
typedef enum hiH264E_NALU_TYPE_E
{/*一个 IDR 帧出来的顺序是7 8 6 5*/
     H264E_NALU_PSLICE = 1, /*PSLICE types*/
     H264E_NALU_ISLICE = 5, /*ISLICE types*/
     H264E_NALU_SEI    = 6, /*SEI types*/
     H264E_NALU_SPS    = 7, /*SPS types*/
     H264E_NALU_PPS    = 8, /*PPS types*/
     H264E_NALU_BUTT        
} H264E_NALU_TYPE_E;

#endif  

