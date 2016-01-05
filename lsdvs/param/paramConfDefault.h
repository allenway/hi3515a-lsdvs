#ifndef __SYSCONFDEFAULT_H__
#define __SYSCONFDEFAULT_H__

#include "const.h"

#define PARAM_CONFIG_SIGNATURE	"svconf1"//"LqJcon1"    //lqj config 1 ,该宏固定不变(长度为7)
#define MAX_CLIENT_USER_NUM		5	         //最多支持创建5个用户
#define MAX_CHANNEL_NUM	    	16	         //最多支持16个通道
#define DEV_MODEL	            "sj_DVS_00"         //产品型号 
#define DEV_SERIAL_NUM	        "000000000001"      //产品序列号
#define DEV_HARDWARE_VERSION    "v1.00"             //硬件版本	
#define DEV_SOFTWARE_VERSION	"v1.00"             //软件版本
#define DEV_CONFIG_VERSION	    "v1.00"             //配置版本
#define DEV_CHIP_NUM	            	1
#define DEV_VIDEO_IN_NUM	        	1
#define DEV_VIDEO_OUT_NUM	        	1
#define DEV_ALARM_IN_NUM	        	1
#define DEV_ALARM_OUT_NUM	        	1
#define MAX_OVERLAY_NUM_ECHE_CHANNEL	4	// 每个通道最多支持4个遮挡区域

#define PARAM_CONFIG_DEFAULT_MAC	"00:00:00:00:00:08" 
#define DEV_DEFAULT_IP	            "192.168.16.47"//司机室1和2:45&47  客室46
#define DEV_DEFAULT_NETMASK	        "255.255.255.0"
#define DEV_DEFAULT_GATEWAY	        "192.168.16.1" 
#define DEV_DEFAULT_DNS1	        "202.96.134.133"
#define DEV_DEFAULT_DNS2	        "202.96.128.68"
#define DEV_DEFAULT_BROADADDR	    "255.255.255.255"
#define DEV_DEFAULT_WEBPORT	    	80
#define DEV_DEFAULT_MSG_PORT		20000
#define DEV_DEFAULT_STREAM_PORT	    ( DEV_DEFAULT_MSG_PORT + 1 )
#define DEV_DEFAULT_SEARCH_PORT	    ( DEV_DEFAULT_MSG_PORT + 2 )


#define DEV_DEFAULT_USER_NAME	"admin"
#define DEV_DEFAULT_USER_PWD	"admin"
#define USER_PERMISSION_VIDEO	        (0x01 << 0)      // 浏览视频权限
#define USER_PERMISSION_RECORD	        (0x01 << 1)      // 录像操作权限
#define USER_PERMISSION_PARAM_SET	    (0x01 << 2)      // 参数设置权限
#define USER_PERMISSION_LOG	            (0x01 << 3)      // 日志查看权限
#define USER_PERMISSION_PTZ	            (0x01 << 4)      // 云台控制权限

#define USER_PERMISSION_ALL	( 	USER_PERMISSION_VIDEO | \
                            	USER_PERMISSION_RECORD | \
                            	USER_PERMISSION_PARAM_SET | \
                            	USER_PERMISSION_LOG	| \
                            	USER_PERMISSION_PTZ	)
typedef enum _UserLevel_
{
	USER_LEVEL_NONE = 0,
	USER_LEVEL_ADMIN,
	USER_LEVEL_BROWSE
} USER_LEVEL;

#define DEV_DEFAULT_USER_PERMISSION	USER_PERMISSION_ALL
#define DEV_DEFAULT_USER_LEVEL		USER_LEVEL_ADMIN

#define DEV_DEFAULT_VIDEO_BRIGHTNESS	128
#define DEV_DEFAULT_VIDEO_CONTRAST		128
#define DEV_DEFAULT_VIDEO_SATURATION	128
#define DEV_DEFAULT_VIDEO_EXPOSURE		128
#define DEV_DEFAULT_VIDEO_HUE	    	0

typedef enum _VideoEncodeStandard_
{
	VIDEO_ENCODE_STANDARD_PAL = 0,
	VIDEO_ENCODE_STANDARD_NTSC,
	VIDEO_ENCODE_STANDARD_AUTO
} VIDEO_ENCODE_STANDARD;

typedef enum _VideoEncodeResolution_
{
	VIDEO_ENCODE_RESOLUTION_QCIF = 0,   
	VIDEO_ENCODE_RESOLUTION_CIF,        	
	VIDEO_ENCODE_RESOLUTION_2CIF,        
	VIDEO_ENCODE_RESOLUTION_HD1,       
	VIDEO_ENCODE_RESOLUTION_D1	       
} VIDEO_ENCODE_RESOLUTION;

typedef enum _VideoEncodeBitrateType_
{
	VIDEO_ENCODE_BITRATE_TYPE_CBR = 0,    // 固定位率
	VIDEO_ENCODE_BITRATE_TYPE_VBR,        // 可变位率
} VIDEO_ENCODE_BITRATE_TYPE;

typedef enum _VideoEncodeStreamType_
{
	VIDEO_ENCODE_TYPE_AV = 0,    //音视频
	VIDEO_ENCODE_TYPE_V,         //视频
	VIDEO_ENCODE_TYPE_A,         //音频
} VIDEO_ENCODE_STREAM_TYPE;

#define DEV_DEFAULT_VIDEO_ENCODE_STANDARD	    	VIDEO_ENCODE_STANDARD_PAL
#define DEV_DEFAULT_VIDEO_ENCODE_RESOLUTION	    	VIDEO_ENCODE_RESOLUTION_D1
#define DEV_DEFAULT_VIDEO_ENCODE_BITRATETYPE		VIDEO_ENCODE_BITRATE_TYPE_CBR//tbd...
#define DEV_DEFAULT_VIDEO_ENCODE_FRAMERATE	    	30//25
#define DEV_DEFAULT_VIDEO_ENCODE_IFRAMEINTERVAL		50  //gop  tbd
#define DEV_DEFAULT_VIDEO_ENCODE_PREFERFRAME		FI_FALSE
#define DEV_DEFAULT_VIDEO_ENCODE_TYPE	        	VIDEO_ENCODE_TYPE_AV//VIDEO_ENCODE_TYPE_V//
#define DEV_DEFAULT_VIDEO_ENCODE_BITRATE	    	1024	//tbd default bps 

/******音频参数******/
typedef enum _FiAudioBitWidth_
{
    FI_AUDIO_BIT_WIDTH_8   = 0,   /* 8bit width */
    FI_AUDIO_BIT_WIDTH_16  = 1,   /* 16bit width*/
    FI_AUDIO_BIT_WIDTH_32  = 2,   /* 32bit width*/
    FI_AUDIO_BIT_WIDTH_BUTT,
} FI_AUDIO_BIT_WIDTH_E;


/******音频采样率******/
typedef enum _FiAudioSampleRate_
{
	FI_AUDIO_SAMPLE_R8     = 0,   /* 8K Sample rate     */
	FI_AUDIO_SAMPLE_R16    = 1,   /* 16K Sample rate    */    
} FI_AUDIO_SAMPLE_RATE_E;

/******音频编码格式******/
typedef enum FiAudioCodecFormat_
{
	FI_AUDIO_FORMAT_ADPCM	= 0,   /* ADPCM     */    
	FI_AUDIO_FORMAT_G711	= 1,   /* G.711     */
	FI_AUDIO_FORMAT_G726	= 2,   /* G.7126    */
} FI_AUDIO_CODEC_FORMAT_E;

/******音频通道模式******/
typedef enum _FiAudioChannelMode_
{
	FI_AUDIO_CHANNEL_MODE_MOMO = 0,
	FI_AUDIO_CHANNEL_MODE_STEREO,
} FI_AUDIO_CHANNEL_MODE_E;

/******AMR包类型******/
typedef enum stAMR_PACKAGE_TYPE_E
{
	FI_AMR_PACKAGE_MIME = 0,  /* Using for file saving        */
	FI_AMR_PACKAGE_IF_1 = 1,  /* Using for wireless translate */
	FI_AMR_PACKAGE_IF_2 = 2,  /* Using for wireless translate */
}AMR_PACKAGE_TYPE;

/******AMR比特率类型******/
typedef enum stAUDIO_AMR_MODE_E
{
	FI_AMR_MODE_R475 = 0,     /* AMR bit rate as  4.75k */
	FI_AMR_MODE_R515,         /* AMR bit rate as  5.15k */
	FI_AMR_MODE_R59,          /* AMR bit rate as  5.90k */
	FI_AMR_MODE_R67,          /* AMR bit rate as  6.70k */
	FI_AMR_MODE_R74,          /* AMR bit rate as  7.40k */
	FI_AMR_MODE_R795,         /* AMR bit rate as  7.95k */
	FI_AMR_MODE_R102,         /* AMR bit rate as 10.20k */
	FI_AMR_MODE_R122,         /* AMR bit rate as 12.20k */
}AUDIO_AMR_MODE;

#define DEV_DEFAULT_AUDIO_ENCODE_SAMPLE_RATE	FI_AUDIO_SAMPLE_R8	    // 如果改为16 k 采样率, 系统就很不正常,不知道为什么
#define DEV_DEFAULT_AUDIO_ENCODE_SAMPLE_WIDTH	FI_AUDIO_BIT_WIDTH_16
#define DEV_DEFAULT_AUDIO_ENCODE_TYPE	    	FI_AUDIO_FORMAT_G711
#define DEV_DEFAULT_AUDIO_ENCODE_CHANNEL_MODE	FI_AUDIO_CHANNEL_MODE_MOMO
#define DEV_DEFAULT_AUDIO_ENCODE_AMR_MODE		FI_AMR_PACKAGE_MIME
#define DEV_DEFAULT_AUDIO_ENCODE_AMR_FORMAT		FI_AMR_MODE_R475

#define DEV_DEFAULT_RECORD_PUBLIC_DEL_SPACE	    (100)           //剩余100M开始删盘	
#define DEV_DEFAULT_RECORD_PUBLIC_DEL_SIZE	    (300)           //删除300M
#define DEV_DEFAULT_RECORD_PUBLIC_SWITCH_FILE_TIME		20	    //20分钟切换一个文件

typedef enum _Baudrate_
{
	BAUDRATE_110 = 0,
	BAUDRATE_300,
	BAUDRATE_600,
	BAUDRATE_1200,
	BAUDRATE_2400,
	BAUDRATE_4800,
	BAUDRATE_9600,
	BAUDRATE_19200,
	BAUDRATE_38400,
	BAUDRATE_57600,
	BAUDRATE_115200	
}BAUDRATE_EN;

typedef enum _Databits_
{
	DATABITS_5 = 0,
	DATABITS_6,
	DATABITS_7,
	DATABITS_8	
}DATABITS_EN;

typedef enum _Stopbits_
{
	STOPBITS_1 = 0,
	STOPBITS_2
}ENOPBITS_EN;

typedef enum _Parity_
{
	PARITY_NONE = 0,
	PARITY_ODD,
	PARITY_EVEN
}PARITY_EN;

#define DEV_DEFAULT_SERIAL_BAUDRATE		BAUDRATE_9600
#define DEV_DEFAULT_SERIAL_DATABITS		DATABITS_8
#define DEV_DEFAULT_SERIAL_STOPBITS		STOPBITS_1
#define DEV_DEFAULT_SERIAL_PARITY		PARITY_NONE	

// for ntp
#define	PARAM_CONFIG_DEFAULT_NTP_ENABLE_FLAG	1     // 使能
#define	PARAM_CONFIG_DEFAULT_NTP_INTERVAL		120   // 120 秒尝试一次
#define	PARAM_CONFIG_DEFAULT_NTP_ZONE	    	8     // 时区
#define PARAM_CONFIG_DEFAULT_NTP_ADDR	        "210.72.145.44"    

// for move detect
#define PARAM_CONFIG_DEFAULT_MOVE_DETECT_SENSITIVITY		3
#define PARAM_CONFIG_DEFAULT_MOVE_DETECT_INTERVAL	    	1
#define PARAM_CONFIG_DEFAULT_MOVE_DETECT_LINKALARMOUT		0x1F // 5个报警输出
#define PARAM_CONFIG_DEFAULT_MOVE_DETECT_ALARMOUTDURATION 	3
#define PARAM_CONFIG_DEFAULT_MOVE_DETECT_LINKCAPTURE		0x1F // 5个抓拍通道
#define PARAM_CONFIG_DEFAULT_MOVE_DETECT_CAPTURENUM	    	3
#define PARAM_CONFIG_DEFAULT_MOVE_DETECT_LINKRECORD	    	0x0F
#define PARAM_CONFIG_DEFAULT_MOVE_DETECT_RECORDDURATION		20

// for ftp
#define	PARAM_CONFIG_DEFAULT_FTP_ENABLE		1
#define	PARAM_CONFIG_DEFAULT_FTP_PORT		21
#define	PARAM_CONFIG_DEFAULT_FTP_IP	        "192.168.16.195" 
#define	PARAM_CONFIG_DEFAULT_FTP_USER	    "sven" 
#define	PARAM_CONFIG_DEFAULT_FTP_PASSWD	    "123456789" 
#define	PARAM_CONFIG_DEFAULT_FTP_UP	    	1 

// for icmp
#define PARAM_CONFIG_DEFAULT_ICMP_IPADDR    "58.63.236.49" // www.sina.com
#define PARAM_CONFIG_DEFAULT_ICMP_INTERVAL 	32
#define PARAM_CONFIG_DEFAULT_ICMP_TIMEOUT	10
#define PARAM_CONFIG_DEFAULT_ICMP_FCOUNT	5
#define PARAM_CONFIG_DEFAULT_ICMP_FINTERVAL 32

// for ddns
typedef enum _DdnsVender_
{
	DDNS_VENDER_QDNS = 0,        // members.3322.org 希网,中国
	DDNS_VENDER_DYNDNS,          // www.dyns.cx 英国
	DDNS_VENDER_ORAY,            // 花生壳
} DDNS_VENDER_EN;

#define PARAM_CONFIG_DEFAULT_DNS_VENDER 	DDNS_VENDER_ORAY // DDNS_VENDER_QDNS// 
#define PARAM_CONFIG_DEFAULT_DNS_PORT     	6060
#define PARAM_CONFIG_DEFAULT_DNS_INTERVAL	120
#define PARAM_CONFIG_DEFAULT_DNS_USERNAME	"sven328"
#define PARAM_CONFIG_DEFAULT_DNS_PASSWD     "sven328"
#define PARAM_CONFIG_DEFAULT_DNS_URL	    "sven328.3322.org" //"sven328.gicp.net"
#define PARAM_CONFIG_DEFAULT_DNS_IFNAME	    "ppp0"

// for dtu
#define PARAM_CONFIG_DEFAULT_DTU_SERVERIP	"192.168.16.218"
#define PARAM_CONFIG_DEFAULT_DTU_SERVERPORT	22222
#define PARAM_CONFIG_DEFAULT_DTU_HEARTBEAT	10
#define PARAM_CONFIG_DEFAULT_DTU_PROTOCOL	0	// 0,tcp; 1, udp
#define PARAM_CONFIG_DEFAULT_DTU_CONTENT	"rhadvs"

// for video shelter
typedef enum _VideoShelterSensitivity_
{
	VIDEO_SHELTER_SENSITIVITY_0 = 0,   // 灵敏度低，需要完全遮挡才会感应
	VIDEO_SHELTER_SENSITIVITY_1,
	VIDEO_SHELTER_SENSITIVITY_2,
	VIDEO_SHELTER_SENSITIVITY_3,
	VIDEO_SHELTER_SENSITIVITY_4,       //灵敏度最高，背景差别不大就会认为被遮挡
} VIDEO_SHELTER_SENSITIVITY_EN;


//OSD PARAM
#define TIME_OSD_TRANSPARENCE 50
#define LOGO_OSD_TRANSPARENCE 50

#endif

