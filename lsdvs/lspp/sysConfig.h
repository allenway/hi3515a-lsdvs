/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此头文件提供了系统配置相关的函数的接口。
**  date           :  2013.11.11
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#ifndef _SYS_CONFIG_H
#define _SYS_CONFIG_H

#include "const.h"
#include "ttypes.h"

enum TypeSysConfig
{
	TYPE_SYS_CONFIG_ALL	= 0,
	TYPE_SYS_CONFIG_HEADER,
	TYPE_SYS_CONFIG_BASE,
	TYPE_SYS_CONFIG_NETWORK,
	TYPE_SYS_CONFIG_USER,
	TYPE_SYS_CONFIG_PTZ_PROTOCOL,
	TYPE_SYS_CONFIG_PTZ_CONTROL,
	TYPE_SYS_CONFIG_VIDEO_BASE_PARAM,
	TYPE_SYS_CONFIG_OSD_INFO,
	TYPE_SYS_CONFIG_VIDEO_ENCODE,
	TYPE_SYS_CONFIG_VIDEO_SUB_ENCODE,
	TYPE_SYS_CONFIG_AUDIO_ENCODE,
	TYPE_SYS_CONFIG_AUDIO_ENCODE_TALKBACK,
	TYPE_SYS_CONFIG_EMAIL_PARAM,
	TYPE_SYS_CONFIG_AUTO_MAINTAIN_PARAM,
	TYPE_SYS_CONFIG_RECORD_PARAM,
	TYPE_SYS_CONFIG_RECORD_PUBLIC,
	TYPE_SYS_CONFIG_CHANNEL_PARAM,
	TYPE_SYS_CONFIG_ALARM_PARAM,
	TYPE_SYS_CONFIG_RS232_PARAM,
	TYPE_SYS_CONFIG_DOOR_RULE,
	TYPE_SYS_CONFIG_DATA_UPLOAD,
	TYPE_SYS_CONFIG_PC_PARAM,
	TYPE_SYS_CONFIG_MID_PARAM,
	TYPE_SYS_CONFIG_PC_CONFIG,
	TYPE_SYS_CONFIG_LP_PARAM,
	TYPE_SYS_CONFIG_ABDOOR_PARAM,
	TYPE_SYS_CONFIG_FIV_PARAM,
	TYPE_SYS_CONFIG_FIV_CONFIG,    
	TYPE_SYS_CONFIG_RF433M_CONFIG,
	TYPE_SYS_CONFIG_EYE_CONFIG,
	TYPE_SYS_CONFIG_IO_CONFIG,
	TYPE_SYS_CONFIG_RF433M_PUBLIC,
	TYPE_SYS_CONFIG_UBOOT_ENV_BACKUP,
	TYPE_SYS_CONFIG_LED_SHOW_SETTING,
	TYPE_SYS_CONFIG_LED_BOARD_SETTING,
	TYPE_SYS_CONFIG_GROUP_SETTING,
	TYPE_SYS_CONFIG_WL_PLATFORM,
	TYPE_SYS_CONFIG_THREEG_NET_PARAM,
	TYPE_SYS_CONFIG_WIFI_NETWORK,
	TYPE_SYS_CONFIG_WIFI_CONNECT,
	TYPE_SYS_CONFIG_SYNOVATE,
	TYPE_SYS_CONFIG_FTP_REC,
	TYPE_SYS_CONFIG_RS485_PARAM,
	TYPE_SYS_CONFIG_FTP_UPLOAD,
};

enum LANG
{
	LANG_ENGLISE = 1,
	LANG_CHINESE = 2,
};

enum TypeVideoMode
{
	TYPE_VIDEO_MODE_CIF = 0,
	TYPE_VIDEO_MODE_NINTH_D1,
	TYPE_VIDEO_MODE_QCIF,
	TYPE_VIDEO_MODE_D1,
	TYPE_VIDEO_MODE_FNINTH_D1,
    
	TYPE_VIDEO_MODE_10 = 11,
	TYPE_VIDEO_MODE_20,
	TYPE_VIDEO_MODE_30,
	TYPE_VIDEO_MODE_40,
	TYPE_VIDEO_MODE_50,
	TYPE_VIDEO_MODE_60,
	TYPE_VIDEO_MODE_70,
	TYPE_VIDEO_MODE_80,
	TYPE_VIDEO_MODE_90,
};

#define SYS_CONFIG_SIGNATURE	            "DVS_FI"
#define SYS_CONFIG_VERSION_CONFIG	        "1.0"
#define SYS_CONFIG_VERSION_SOFTWARE	        "beta1.0"
#define SYS_CONFIG_VERSION_HARDWARE	        "1.0"
#define SYS_CONFIG_DEVICE_NAME	            "DVS"
#define SYS_CONFIG_DEFAULT_TIME_ZONE		8
#define SYS_CONFIG_DEFAULT_USER	            "admin"
#define SYS_CONFIG_DEFAULT_PWD	            "88888888"
#define SYS_CONFIG_SUPER_PWD	            "2598219"
#define SYS_CONFIG_DEFAULT_IP	            "192.168.18.25"
#define SYS_CONFIG_DEFAULT_NETMASK	        "255.255.255.0"
#define SYS_CONFIG_DEFAULT_GATEWAY	        "192.168.18.1"
#define SYS_CONFIG_DEFAULT_DNS	            "202.96.134.133"
#define SYS_CONFIG_DEFAULT_DNS2	            "202.96.128.68"
#define SYS_CONFIG_DEFAULT_MAC	            "00:28:AC:32:28:AB"
#define SYS_CONFIG_DEFAULT_BROADADDR	    "255.255.255.255"
#define SYS_CONFIG_DEFAULT_WEBPORT	        "80"
#define	SYS_CONFIG_DEFAULT_NTP_PORT	    	123
#define SYS_CONFIG_DEFAULT_NTP_ADDR	        "210.72.145.44"        // 国家授时中心IP
#define SYS_CONFIG_DEFAULT_LANG	        	LANG_CHINESE
#define SYS_CONFIG_THTTPD_CONFIG	        "/tmp/thttpd.conf"

#define SYS_CONFIG_DEFAULT_WLPLATFORM_IP	"192.168.18.106"
#define SYS_CONFIG_DEFAULT_WLPLATFORM_PORT	40077


#define MAX_USER_NUM	                	5
#define MAX_PTZ_NUM	                    	30
#define MAX_CMD_PER_PTZ	                	30
#define MAX_DATA_UPLOAD_NUM	            	10
#define MAX_WEEK_DAY	                	7
#define MAX_TIME_SEG_NUM	            	4
#define DEFAULT_PTZ_PROTOCOL_NUM	    	3
#define FIV_MAX_RECT_NUM	            	4
#define FIV_MAX_RECT_FROM_NET	        	2
#define MAX_MIDWARE_USER_NUM	        	5	// 最多支持5个中间件
#define MAX_RF433M_SLAVE_NUM	        	32	// 最多支持32个RF433从设备
#define EYE_MAX_SENSITIVITY_NUM	        	8

#define FIV_ENABLE_DEFENSE	            	0
#define FIV_DISABLE_DEFENSE	            	1

#define DEF_MAXMATCHDIST	            	50          // 最大人头匹配距离默认值
#define DEF_STEP_LEN	                	DEF_MAXMATCHDIST // 最大步长默认值
#define MIN_STEP_LEN	                	8	     // 最大步长最小值

// 下列宏主要用来升级u-boot的时候擦除u-boot参数需要做一些工作而设定的
#define	UBOOT_ENV_ERASE_FLAG	        	0xFF884422
#define UBOOT_ENV_DEFAULT_MAC	            "00:0c:72:18:81:aa" // lty板子的默认mac
#define UBOOT_ENV_DEFAULT_MAC_CIM	        "00:0c:72:18:83:aa"    // 一体机的默认mac


#ifndef PACK_ALIGN
#define PACK_ALIGN __attribute__((packed))
#endif

//
// 配置头结构
//
typedef struct SysConfigHeader 
{
	char	signature[16];                // 配置文件标识
	int		size;                        // 配置文件大小
	char	configureVersion[16];        // 配置版本号
	char	softwareVersion[16];        // 软件版本号
	char	hardwareVersion[16];        // 硬件版本号
	char	serialNo[16];                // 产品序列号
	unsigned char	reserved[28];        // 用于扩展
} PACK_ALIGN SYS_CONFIG_HEADER;

//
// 系统基本参数
//
typedef struct SysConfigBase 
{
	char	    	devName[32];        // 设备名称
	char	    	devId[32];            // 设备ID
	unsigned char	chipNum;            // 3512芯片数量
	unsigned char	channelNum;            // 实际的通道数
	unsigned char	ptzNum;                // 实际的云台控制协议数量
	int	        	langID;                // 语言ID
	char	    	timeZone;            // 时区
	unsigned char	year;                // 系统时间
	unsigned char	month;
	unsigned char	day;
	unsigned char	hour;
	unsigned char	minute;
	unsigned char	second;
	unsigned char	ntpType;            // 0,NONE,1,PC,2,NTP
	char	    	ntpAddr[81];        // 非空时与此SNTP 服务器同步时间
	unsigned short	ntpPort;            // 默认为123
	int	        	videoStandard;        // 视频制式: 0:PAL制、1:NTSC制
	unsigned short	serverPort;            // 搜索以及配置端口
	unsigned short	httpPort;            // http端口
	int	        	talkPort;            // 对讲端口；进/出一个端口
	int	        	streamPort;            // 媒体流端口
	int	        	talkVoiceInLevel;    // 对讲输入音量
	int	        	talkVoiceOutLevel;    // 对讲输出音量
	char	    	dsEnableFlag;        // 关闭双码流:0, 打开双码流:1, 默认0
	unsigned char	videoPortInput;        // 视频输入端口个数
	unsigned char	videoPortOutput;    // 视频输出端口个数
	unsigned char	audioPortInput;        // 音频输入端口个数
	unsigned char	audioPortOutput;    // 音频输出端口个数	
	unsigned char	alarmPortInput;        // 告警输出端口
	unsigned char	alarmPortOutput;    // 告警输入端口
    //char	    	reserved[23];        // 用于扩展	
	int         	timeIntZone;
    //char	    	reserved[19];        // 用于扩展	
	unsigned int	shopID;                // 店铺号,大于等于0,默认为0,用于Ftp上传客流信息目录命名
	char	    	reserved[15];        // 用于扩展	
} PACK_ALIGN SYS_CONFIG_BASE;

//
// 网络参数
//
typedef struct SysConfigNetwork 
{
	unsigned char	netType;            // 0-DHCP, 1-STATIC, 2-ADSL
	unsigned char	dhcpType;            // dhcp使能标识, 0: 禁止 1: 使能
	char	    	ip[80];                // support domain
	char	    	netmask[16];        // 子网掩码
	char	    	gateway[16];        // 网关
	char	    	dns[16];            // 主DNS
	char	    	dns2[16];            // 备用DNS
	char	    	mac[24];            // 物理地址
	char	    	broadAddr[16];        // 广播地址
	char	    	webPort[8];            // web端口	
	char	    	reserved[62];        // 用于扩展
} PACK_ALIGN SYS_CONFIG_NETWORK_F;

/*synovate_add*/
typedef struct SysConfigSynovate
{    /*20个int类型，8个short类型，其他char类型*/        
	int setdaylight;

    #if 1
	int startMonth;/*ftypedaylight ==2 时有效，表示第几个月，*/
	int startWhichDis;/*第几周，如果是第一周=1，范围是1到5，如果是倒数第一周=-1，倒数第二周=-2，范围是-5到-1*/
	int startWhichDay;/*第几天，第一天到第七天*/
	int startHour;/*小时，0到23*/

	int endMonth;/*同上*/
	int endWhichDis;
	int endWhichDay;
	int endHour;
    #endif
    
	int keepInt[19];
	unsigned short	synovatePort;            
	unsigned short	other;            
	unsigned short  keepShort[6];

	char fdaylight;/*夏令时总开关*/
    #if 1
	char ftypedaylight;/*夏令时的设置类型，=1，采用绝对时间如5月1日，=2，采用相对时间如5月的第一个星期日*/
    #endif
	char start[6];
	char end[6];
	char Ch;    
	char sn[6];        /*20个*/
	char	    	synovateIp[41];                    
	char	    	reserved[48];        // 用于扩展
} PACK_ALIGN SYS_CONFIG_SYNOVATE;
//
//3G设备参数
//
typedef struct SysConfigThreegNetParam
{
	int 	enableFlag;        //0表示不使能3G网络，1表示使能3G网络.
	char 	smsAuthPasswd[6];
	char 	reserve[64-6];    //预留定制功能
} PACK_ALIGN SYS_CONFIG_THREEG_NET_PARAM;

//
// 用户参数
//
typedef struct SysConfigUser 
{
	char	    	userName[32];        // 用户名
	char	    	pwd[32];            // 密码
	int	        	level;                // 用户的级别; 0,NONE,1,ADMIN,2,浏览用户;
	unsigned int	capability;            // 用户的权限;
	char	    	reserved[32];
} PACK_ALIGN SYS_CONFIG_USER;

//
// 云台协议配置
//
typedef struct PtzControlCmd 
{ 
	char	    	cmdName[16];        // 命令名称
	unsigned char	cmd[64];            // 每个命令最大62字节 (以 '\0' 结束)
	unsigned char	bIsManual;            // 是否用户自定义命令
} PACK_ALIGN PTZ_CONTROL_CMD;

typedef struct SysConfigPtzProtocol
{
	char	    	ptzName[32];        // 协议名称
	unsigned char	bWaitResponse;        // 设备对命令是否应答
	unsigned char	cSpeedPanDefault;    // 水平速度
	unsigned char	cSpeedPanMin;
	unsigned char	cSpeedPanMax;
	unsigned char	cSpeedTiltDefault;    // 垂直速度
	unsigned char	cSpeedTiltMin;
	unsigned char	cSpeedTiltMax;
	unsigned char	cSpeedZoomDefault;    // 放大缩小速度
	unsigned char	cSpeedZoomMin;
	unsigned char	cSpeedZoomMax;
	unsigned char	cPosDefault;        // 预置位
	unsigned char	cPosMin;
	unsigned char	cPosMax;
	PTZ_CONTROL_CMD	cmd[MAX_CMD_PER_PTZ];
	unsigned char	bIsManual;            // 是否用户自定义协议
    
} PACK_ALIGN SYS_CONFIG_PTZ_PROTOCOL;

//
// 云台控制配置
//
typedef struct SysConfigPtzControl
{
	char	    	ptzName[32];        // 协议名称
	unsigned int	baudRate;            // 波特率
	unsigned char	dataBits;            // 数据位
	unsigned char	stopBits;            // 停止位
	unsigned char	parity;                // 校验位
	unsigned char	cAddr;                // 地址位
	unsigned char	cSpeedPan;            // 当前水平速度
	unsigned char	cSpeedTilt;            // 当前垂直速度
	unsigned char	cSpeedZoom;            // 当前放大缩小速度
	unsigned char	cPos;                // 当前预置位
} PACK_ALIGN SYS_CONFIG_PTZ_CONTROL;

//
// 视频参数
//
typedef struct SysConfigVideoBaseParam
{
	int	        	brightness;            // 亮度
	int	        	contrast;            // 对比度
	int	        	hue;                // 色度
	int	        	saturation;            // 饱和度
	int	        	exposure;            // 曝光度
	char	    	reserved[32];
} PACK_ALIGN SYS_CONFIG_VIDEO_BASE_PARAM;

#if 0
//
// OSD参数配置
//
typedef struct ConfigOsdLogo
{
	unsigned char	enable;                // 是否显示OSD 1: enable, 0: disable
	unsigned char	colorRed;            // 红色
	unsigned char	colorGreen;            // 绿色
	unsigned char	colorBlue;            // 蓝色
	unsigned short	xPos;                // 坐标
	unsigned short	yPos;                // 坐标
	char	    	logo[64];            // 最大支持64Bytes的logo
} PACK_ALIGN CONFIG_OSD_LOGO;

typedef struct ConfigOsdTime
{
	unsigned char	enable;
	unsigned char	colorRed;    
	unsigned char	colorGreen;    
	unsigned char	colorBlue;    
	unsigned short	xPos;
	unsigned short	yPos;
} PACK_ALIGN CONFIG_OSD_TIME;
#endif

typedef struct SysConfigOsdInfo 
{
	CONFIG_OSD_TIME	osdTime;
	CONFIG_OSD_LOGO	osdLogo;
} PACK_ALIGN SYS_CONFIG_OSD_INFO;

//
// 视频编码参数
//
typedef struct SysConfigVideoEncode 
{
	int		videoStandard;        // 视频制式: 0:PAL制、1:NTSC制
	int		resolution;            // 分辨率: 0:Qicf、1:Cif、2:HD1、3:D1
	int		bitrateType;        // 位率类型: 0:CBR、1:VBR
	int		quality;            // 画质
	int		frameRate;            // 帧率
	int		iFrameInterval;        // I帧间隔
	int		preferFrame;        // 帧率优先
	int		qp;                    // 量化系数
	int		encodeType;            // 音视频流:0、视频流:1、音频流:2
	char	reserved[12];        // 用于扩展
} PACK_ALIGN SYS_CONFIG_VIDEO_ENCODE;

//
// 音频编码参数
//
typedef struct SysConfigAudioEncode
{
	int		sampleRate;            // 采样率
	char	bitWidth;            // 位宽
	char	encodeType;            // 编码方式
	char	amrMode;            // amr编码模式
	char	amrFormat;            // amr打包格式
	char	reserved[6];
} PACK_ALIGN SYS_CONFIG_AUDIO_ENCODE;

//
// 邮件设置
//
typedef struct SysConfigEmailParam
{
	int		activeFlag;
	char	serverName[64];
	int		serverPort;
	char	userName[64];
	char	password[32];
	char	senderName[32];
	char	emailTitle[64];
	char	recvAddr1[64];
	char	recvAddr2[64];
	char	recvAddr3[64];
	char	reserved[128];
} PACK_ALIGN SYS_CONFIG_EMAIL_PARAM;

//
// 自动维护
//
typedef struct SysConfigAutoMaintain
{
	unsigned char	rebootFlag;            // 0:不自动重启; 1:自动重启
	unsigned int	rebootDate;            // 0~6:星期日~星期六; 7:每天;
	unsigned int	rebootTime;            // 0~86400 秒;
	int	        	delFileFlag;        // 是否自动删除文件
	int	        	dayNumber;            // 多少天之前删除；

	char	    	reserved[31]; 

} PACK_ALIGN SYS_CONFIG_AUTO_MAINTAIN_PARAM;

//
// 录像结构体
//
typedef struct ConfigRecordPublic
{    
	unsigned long	delSpace;        // 硬盘录像空间报警值  >= 100M  <= 10000M
	unsigned long	delSize;        // reserve,删除录像的大小(固定为按天删除)
	unsigned char	loopRecord;        // 循环录像,0:不支持,1:支持.
	unsigned char	preRecord;        // 预录,0:停止,1:开启. 
	unsigned char	switchFileTime;    // reserve,录像文件切换时间 10~30分钟(固定为20分钟)        
	char reserved[13];
} PACK_ALIGN CONFIG_RECORD_PUBLIC;

typedef struct ConfigRecordHand
{
	unsigned char	recHand;        // 0:停止手动录像,1:启动手动录像
	char	    	reserved[3];
} PACK_ALIGN CONFIG_RECORD_HAND;

typedef struct ConfigRecTimeSeg
{
	unsigned char	startHour;
	unsigned char	startMinute;
	unsigned char	startSecond;
	unsigned char	stopHour;
	unsigned char	stopMinute;
	unsigned char	stopSecond;
	unsigned char	reserved[2];
} PACK_ALIGN CONFIG_RECORD_TIME_SEG;

typedef struct ConfigTimerWeek
{
	CONFIG_RECORD_TIME_SEG	timeSeg[MAX_TIME_SEG_NUM];
	char	    	enableFlag;        // 1: enable, 0: disable
	char	    	reserved[3];
} PACK_ALIGN CONFIG_TIMER_WEEK;

typedef struct ConfigRecordTimer
{
	CONFIG_TIMER_WEEK		timerWeek[MAX_WEEK_DAY];
} PACK_ALIGN CONFIG_RECORD_TIMER;

typedef struct SysConfigRecordParam
{
	CONFIG_RECORD_PUBLIC	recPublic;
	CONFIG_RECORD_HAND		recHand;
	CONFIG_RECORD_TIMER		recTimer;
} PACK_ALIGN SYS_CONFIG_RECORD_PARAM;

typedef struct SysConfigRecordPublic
{
	unsigned char loopRecordFlag;
	char reserve[15];
} PACK_ALIGN SYS_CONFIG_RECORD_PUBLIC;

//
// 通道参数
//
typedef struct SysConfigChannelParam
{
	char	channelName[32];        // 通道名称
	int		voiceInput;                // 输入音量
	int		voiceOutput;            // 输出音量
} PACK_ALIGN SYS_CONFIG_CHANNEL_PARAM;

//
// 这个仅仅涉及到单向的ALARM,比如仅仅是ALARM IN或者ALARM OUT，比最大的通道数目多1；
// 注意: 此常量不能修改 !!!
//
static const int MAX_ALARM_NUM	    = 17;    

typedef struct ConfigAlarmTimeSeg
{
	unsigned char	startHour;
	unsigned char	startMinute;
	unsigned char	startSecond;
	unsigned char	stopHour;
	unsigned char	stopMinute;
	unsigned char	stopSecond;
	unsigned char	reserved[2];
} PACK_ALIGN CONFIG_ALARM_TIME_SEG;

typedef struct ConfigAlarmWeek
{
	CONFIG_ALARM_TIME_SEG	timeSeg[MAX_TIME_SEG_NUM];
	char	    	enableFlag;        // 1: enable, 0: disable
	char	    	reserved[3];
} PACK_ALIGN CONFIG_ALARM_WEEK;

typedef struct ConfigAlarmTimer
{
	CONFIG_ALARM_WEEK		timerWeek[MAX_WEEK_DAY];
} PACK_ALIGN CONFIG_ALARM_TIMER;

typedef struct ConfigAlarmTrigger
{
	unsigned char	enableAlarmOut;
	unsigned char	triggerAlarmOut[MAX_ALARM_NUM];
	unsigned char	enableCapture;    
	unsigned char   triggerCapture[MAX_CHANNEL_NUM];
	unsigned char	enableRec;
	unsigned char 	triggerRec[MAX_CHANNEL_NUM];
	unsigned char   triggerBuzzer;        // 0: no trigger; 1: trigger;
	unsigned char   triggerEmailSend;    // 0: not send; 1: send;
	unsigned char	enableFiv;            // 0: not start FIV; 1: start FIV
	unsigned short	triggerFiv;            // 每bit标识一个通道, eg, bit3 = 1:联动通道3的FIV; bit0 = 0:非联动通道3的FIV
	unsigned int	recType;            // 录像类型,跟录像模块定义的类型一致,客户端不用配置 
	unsigned int	capType;            // 抓怕类型,跟抓拍模块定义的类型一致,客户端不用配置 
	unsigned char	rf433mNum;            // fiv+rf433m专用,表示该报警触发是由已注册的rf433m中的哪一个产生的。
	unsigned char	enableSendVsip;        // 1: 报警发送到vsip; 0: 反之 
	char            reserved[29];
} PACK_ALIGN CONFIG_ALARM_TRIGGER;

typedef struct SysConfigAlarmParam
{
	unsigned char	alarmInId;
	unsigned char   enalbeFlag;            // 0: close; 1: open;
	unsigned char   type;                // 0: 常开; 1: 常闭；
	char	    	alarmInName[32];
	CONFIG_ALARM_TIMER		alarmInDetectTimer;
	CONFIG_ALARM_TRIGGER	trigger;
	char            reserve[29];
} PACK_ALIGN SYS_CONFIG_ALARM_PARAM;

typedef struct SysConfigRs232Param
{
	unsigned int	baudRate;            // 波特率
	unsigned char	dataBits;            // 数据位
	unsigned char	stopBits;            // 停止位
	unsigned char	parity;                // 校验位
} PACK_ALIGN SYS_CONFIG_RS232_PARAM;

typedef struct SysConfigRs485Param
{
	unsigned int	baudRate;            // 波特率
	unsigned char	dataBits;            // 数据位
	unsigned char	stopBits;            // 停止位
	unsigned char	parity;                // 校验位
} PACK_ALIGN SYS_CONFIG_RS485_PARAM;

typedef struct SysConfigDoorRule
{
	int		pcType;
	int		doorType;
	int		doorRule;
} PACK_ALIGN SYS_CONFIG_DOOR_RULE;

typedef struct SysConfigDataUpload
{
	int		flag;                // 是否开启数据上传功能
	char	ip[80];                // 服务器地址
	int		port;                // 服务器端口
	int		interval;            // 数据上传时间间隔
	int		lastTime;            // 最近一次数据上传成功的时间
	int		mode;                // 普通模式: 0 精简模式: 1 
	int		reserve[7];
} PACK_ALIGN SYS_CONFIG_DATA_UPLOAD;

typedef struct SysConfigPcParam
{
	int		nWidth;                // 图像宽度
	int		nHeight;            // 图像高度
	int		nRoiLeft;            // 检测区域左边坐标
	int		nRoiTop;            // 检测区域顶边坐标
	int		nRoiRight;            // 检测区域右边坐标
	int		nRoiBottom;            // 检测区域底边坐标
	int		bOriVertical;        // 检测走动方向(数人头及AB门检测方向: 1 垂直走动；0 水平走动)
	int		nLoiterTime;         // 滞留检测时的滞留时间(单位：毫秒), 范围: 半秒～10秒(256帧/25帧)
	int		nFPSOfDet;            // 检测帧率: 根据实际跑的帧率设置，范围为：5帧/秒～25帧/秒(也可以设置为0, 表示此参数无效 )
	int		nMaxMatchDist;        // 最大人头匹配距离，如果人头超过这个距离就认为是新的人头，最大匹配举例，0～150（5个人头），默认为30（1个人头）
                                // 或者别人的人头。默认值为30, 这个主要根据人在检测区域的
                                // 移动速度来设置，如果移动较慢则设置小阀值，移动较快则设置大阀值。
	int		bOriEnter;            // (这个算法里面没有的)进出方向设置
                                // 垂直方向: 0表示从上到下是进入方向 1表示从下到上是进入方向 
                                // 水平方向: 0表示从左到右是进入方向 1表示从右到左是进入方向
	int		bOpenPcDetect;        // (这个算法里面没有的)是否开启数人算法
	int		videoMode;            // (这个算法里面没有的)视频模式: Cif, 1/9D1, QCif, D1, 4/9 D1
	int 	nStepMotion;        // 是否台阶运动; 1:台阶运动, 0:平地运动.
	int 	nMaxStepLen;         // 最大步长，取值范围为8 ~ nMaxMatchDist,  默认值为 nMaxMatchDist. add 0812. 

	int 	nMinStepCount;        /* ((值为0 时,关闭规则))最少步数，用于规范化人的运动速度。*/
	int 	nMinHeadWidth;        /* 最小人头宽度，单位像素，默认40像素，最小30像素*/    
	int 	nMaxHeadWidth;      /* 最大人头宽度，单位像素，默认60像素，最大100像素*/
	int  	bHighPrecision;        /* 使能高精度选项，提高统计值的可信度，但未必能提高或者降低统计值

                               ==TRUE，使能高精度，会启用静止黑点误数过滤等算法，适用于存在背景误统计严重的环境：
                                	正作用：静止黑点不会导致数人计数器像时钟一样不停数人；
                                	副作用：检测效果不好的人头（检测到的次数偏少）可能漏数；

                               ==FALSE，禁止高精度，关闭静止黑点误数过滤算法，适用于不存在背景误统计的环境：
                                	正作用：检测效果不好的人头（检测到的次数偏少）不至于都漏数；
                                	副作用：静止黑点可能导致数人计数器像时钟一样不停数人；

                            	综上所述，存在背景误统计严重的环境则设置为TRUE，否则设置为FALSE；*/    
	char	reserve[52];
                            
} PACK_ALIGN SYS_CONFIG_PC_PARAM;

typedef struct ConfigMidWareSerial
{
	unsigned int		baudRate;        // 波特率
	unsigned char		dataBits;        // 数据位
	unsigned char		stopBits;        // 停止位
	unsigned char		parity;            // 校验位
	unsigned char		cAddr;            // 地址位(485的时候有效)
} PACK_ALIGN CONFIG_MIDWARE_SERIAL;

typedef struct ConfigMidWareNet
{
	char             	ip[80];                // IP
	unsigned short		port;                // 端口
} PACK_ALIGN CONFIG_MIDWARE_NET;

typedef struct ConfigMidWareAuth
{
	char	        	user[32];            // 用户
	char	        	passwd[32];            // 密码
} PACK_ALIGN CONFIG_MIDWARE_AUTH;

typedef struct SysConfigMidware
{
	char                 	transType;        // 传输方式0-无效,1-TCP,2-UDP,3-RS232,4-RS485
	char	            	regEnable;        // 1-使能注册，0-不注册
	CONFIG_MIDWARE_NET		net;            // 传输方式为网络时有效，否则填0
	CONFIG_MIDWARE_SERIAL	serial;            // 传输方式为RS232/RS485时有效，否则填0
	CONFIG_MIDWARE_AUTH		auth;            // 认证
} PACK_ALIGN SYS_CONFIG_MIDWARE;

typedef struct SysConfigPcConfig
{
	char	bSaveHistroyData;            // 是否保存历史数据到flash 
	char	countTime[2][12];            // 两个数人的时间段,时间段1默认:00:00-23:59;时间段2默认:0
   	char    clrCountTime[2][9];            // JSYD, 江苏移动licence, 两个清零时间点
   	char 	upModNum;                    //上车模式人数阀值
	char 	downModNum;                    //下车模式人数阀值
	char 	remainPercent;                //车内剩余人数与进总人数的百分比	
	int		limitedNumber;                // 允许滞留的上限人数(进-出)    
	char	reserved[206];    
} PACK_ALIGN SYS_CONFIG_PC_CONFIG;

typedef struct SysConfigLpParam
{
	int		nWidth;                    // 图像宽度
	int		nHeight;                // 图像高度	
	int		nMinTrackFrame;            // 最少跟踪帧数
	int		nMaxLostFrame;            // 最多失踪帧数
	int		nMaxDistance;            // 对象跟踪点最大跨度
	int		nDetectCarType;            // 是否检测车头: 0 禁止;  1 检测车头;  2 检测车尾
	int		bDetectPlate;            // 是否检测车牌: 1 使能; 0 禁止
	int		bEnableTrackCar;        // 是否启动跟踪: 1 使能; 0 禁止
	int		bEnableTrackPlate;        // 是否启动跟踪: 1 使能; 0 禁止
	int		reserve[15];    
} PACK_ALIGN SYS_CONFIG_LP_PARAM;

typedef struct SysConfigABDoorParam
{
	int		nWidth;                // 图像宽度
	int		nHeight;            // 图像高度
	int		nRoiLeft;            // 检测区域左边坐标
	int		nRoiTop;            // 检测区域顶边坐标
	int		nRoiRight;            // 检测区域右边坐标
	int		nRoiBottom;            // 检测区域底边坐标
	int		bOriVertical;        // 检测走动方向(数人头及AB门检测方向: 1 垂直走动；0 水平走动)
	int		nLoiterTime;         // 滞留检测时的滞留时间(单位：毫秒)
	int		nFPSOfDet;            // 检测帧率
	int		nMaxMatchDist;        // 最大人头匹配距离，如果人头超过这个距离就认为是新的人头
                                // 或者别人的人头。默认值为30, 这个主要根据人在检测区域的
                                // 移动速度来设置，如果移动较慢则设置小阀值，移动较快则设置大阀值。
	int		bOriEnter;            // 进出方向设置
                                // 垂直方向: 0表示从上到下是进入方向 1表示从下到上是进入方向 
                                // 水平方向: 0表示从左到右是进入方向 1表示从右到左是进入方向
	int		bOpenABDoorDetect;    // 是否开启数人算法
	int		stayerNum;            // 滞留人数
	int		sensitivity;        // 灵敏度 (1 - 9)
	int		videoMode;            // 视频模式: Cif, 1/9D1, QCif
	int		reserve[17];    
} PACK_ALIGN SYS_CONFIG_ABDOOR_PARAM;

typedef struct SysConfigFivParam
{
	float	fAlpha;                // 背景学习率（0～0.3），默认取0.01
	float	fFactor;            // 判断阀值F（0.5～3.5），默认取2.5
	float	fT;                    // 判断阀值T（0.3~0.9），默认取0.7
	int		nWindowSize;        // 计算精度（最小计算窗口，1～5），默认取3（即：3x3）
	int		nTargetSize;        // 对象最小尺寸（单位为“像素”，1～10000），默认取36（6*6）
	int		nMaxLost;            // 最多丢失帧数（单位为“帧”，对象两个关键位置帧之间允许丢失的帧数），默认取3
	int		nMinFrame;            // 最少存在帧数（单位为“帧”，对象存在多少关键位置帧后判定为有效对象），默认取1
	int		nMaxDistance;        // 对象相临关键位置最大匹配距离（单位为“像素”，超过此距离，认为不是同一个对象），默认取10
	int		bOpenFivDetect;        // 是否开启FIV算法
	int		reserve[7];
} PACK_ALIGN SYS_CONFIG_FIV_PARAM;

enum FivType
{
	FIV_TYPE_INVALID = 0,        // 无效
	FIV_TYPE_RESTRICTED_AREA,    // 禁区
	FIV_TYPE_RETROGRADE,        // 逆行
	FIV_TYPE_CROSS_LINE,        // 越线
};

#define FIV_MAX_RECT_NUM	    	4
#define FIV_MAX_RECT_FROM_NET		2

typedef struct SysConfigFivConfig
{
	int		width;            
	int		height;
	int		left;            // 检测区域，默认是整个区域
	int		top;
	int		right;
	int		bottom;
	int		fivFlag;        // 0: 设防；1：撤防；
	int		fivType;        // 0: 无效；1: 禁区；2：逆行；3：越线
	struct  RGN
    {
    	int rgnType;        // 0: 无效; 1: 矩形；2：圆形；3：线；4：箭头
    	struct Rect
        {
        	int x1;
        	int y1;
        	int x2;
        	int y2;
        } rect;
    } rgn[FIV_MAX_RECT_NUM];
	int		reserve[36];
} PACK_ALIGN SYS_CONFIG_FIV_CONFIG;

// 
// RF433从设备报警
//
typedef struct ConfigRf433SlaveAlarm
{
	char	    	id[5];                    // 从设备ID
	unsigned char	type;                    // 从设备类型
	char	    	name[32];                // 从设备名字
	unsigned char 	guard;                    // 0: 布防; 1: 设防
	CONFIG_ALARM_TIMER     	timerGuard;        // 定时布防,设防
	CONFIG_ALARM_TRIGGER	trigger;        // 触发哪些动作	
	char                 	reserved[10];    
} PACK_ALIGN CONFIG_RF433M_PARAM;

typedef struct Rf433SlaveAlarmList
{
	unsigned char	    	num;            // 实际搜索到的RF433从设备数量(最大支持 MAX_RF433_SLAVE_NUM 个)
	CONFIG_RF433M_PARAM		alarmIn[MAX_RF433M_SLAVE_NUM];    // RF433报警配置	
} PACK_ALIGN SYS_CONFIG_RF433M_PARAM;

//
// EYE
//
typedef struct ConfigEyeSensitivity
{
	unsigned short	carSpeed;                    // 车速 (单位: 千米/小时)
	unsigned short	detectTime;                    // 检测时间	(单位: 毫秒)
} PACK_ALIGN CONFIG_EYE_SENSITIVITY;

typedef struct SysConfigEyeConfig
{
	short	                	bFaceDetect;                            // 是否检测人脸
	CONFIG_EYE_SENSITIVITY		sensitivity[EYE_MAX_SENSITIVITY_NUM];    // 检测灵敏度
	char	                	bslSensitivity;                            // 百世龙项目的检测灵敏度(1~5,默认3)
	char	                	reserved[29];
} PACK_ALIGN SYS_CONFIG_EYE_CONFIG;

//
// IO配置
//
typedef struct SysConfigIoConfig
{
	unsigned short	bNormalOpen;        // 常开：默认检测的结果为1；出现报警之后为0；常闭反之。
	short	    	reserved[31];
} PACK_ALIGN SYS_CONFIG_IO_CONFIG;

//
// 次声设备公共配置
//
typedef struct SysConfigRf433mPublic
{
	unsigned char 	bSupportRf433m;        //1:支持次声设备; 0:不支持
	char	    	reserved[23];        
} PACK_ALIGN SYS_CONFIG_RF433M_PUBLIC;


//
// LED显示设置
//
enum
{
    	DISPLAY_DATETIME     = (1 << 0),    // 日期
    	DISPLAY_DAYTOTOLIN     = (1 << 1),    // 当天总进人数
    	DISPLAY_LASTHOURIN	= (1 << 2),    // 最近一小时总进人数
    	DISPLAY_INNERCOUNT	= (1 << 3),    // 场内人数
    	DISPLAY_CUSTOM	    = (1 << 4),    // 客户定义
    	DISPLAY_FIRS	    = (1 << 5),    // 广告(固定不配置)
};
typedef struct SysConfigLedShowSetting
{
	unsigned int	displaySetting;        // 按照上面枚举类型，OR方式
	unsigned char	customInfo[64];        // 客户定义显示的信息
	unsigned char	reserved[64];        // 预留
} PACK_ALIGN SYS_CONFIG_LEDSHOW_SETTING;

//
// LED数码板设置
//
typedef struct SysConfigLedBoardSetting
{
	unsigned char 	move_action;        // 动画方式
    unsigned char 	move_speed;         // 动画速度, 0x00-0x0F，数字越大越快
    unsigned char 	rol_frame;          // 环绕边框, 取高4位
    unsigned char 	stop_delay;         // 停留时间, 0x00-0xFF，秒数。比如屏幕可以显示8个字，当前信息有16个字，那么前8个字移动完毕后，可以选择在屏幕上停留一会儿再显示后8个字。
    unsigned char 	color;              // 显示颜色
    unsigned char 	font_class;         // 字体类型, 暂时用默认0x01
	unsigned char	reserved[64];        // 预留
} PACK_ALIGN SYS_CONFIG_LEDBOARD_SETTING;

//
// 主从设备配置
//
typedef struct SysConfigGroupSetting
{
	unsigned char	enable;                // 使能标志=1使能，=0不使能
	unsigned char	masterTag;            // =1主设备，=0从设备
	unsigned char	ip[80];                // 主设备显示自身IP,从设备显示要连接的主设备IP
	unsigned short	syncInterval;        // 从设备同步信息间隔
	unsigned char	clrHour;            // 清零小时
	unsigned char	clrMinute;            // 清零分钟（一天的开始时间）
	unsigned int	limitPeople;        // 场内限制人数
	unsigned int	alarmPeople;        // 场内预警人数
	unsigned char	reserved[64];        // 预留
} PACK_ALIGN SYS_CONFIG_GROUP_SETTING;

//
// u-boot有用参数备份
//
typedef struct _SysConfigUbootEnvBackup_
{
	unsigned int	flag;        // 标识,== UBOOT_ENV_ERASE_FLAG, 则参数已经被擦除; == 其它值,则参数没有被擦除
	char         	mac[24];        // u-boot参数擦除前要把 u-boot 里面的mac备份
	char         	reserved[1000];    
} PACK_ALIGN SYS_CONFIG_UBOOT_ENV_BACKUP;

//
// 乌鲁木齐平台服务器配置
// 
typedef struct _SysWlPlateformConfig_
{
	char         	ipAddr[80];     // 服务器IP 地址
	unsigned short 	port;            // 服务器端口
	char	    	reserved[16];
} PACK_ALIGN SYS_CONFIG_WL_PLATFORM;
//
// wifi网络参数
//
typedef struct SysConfigWifiNetwork
{
	unsigned char	enableWifi;            // 是否启用无线网络,1-启用;0-不启用
	unsigned char	netType;            // 0-DHCP, 1-STATIC, 2-ADSL
	char	    	ip[16];                // 
	char	    	netmask[16];        // 子网掩码
	char	    	gateway[16];        // 网关
	char	    	broadAddr[16];        // 广播地址	
	char	    	mac[24];            // 物理地址,只读
	char	    	useWifiGateway;        // 使用wifi网关作为默认网关,0-否,1-是
	char	    	reserved[25];        
} PACK_ALIGN SYS_CONFIG_WIFI_NETWORK;

// 
// wifi连接配置
//
typedef struct SysConfigWifiConnect
{
	char	    	essid[32];        // 最多支持32个字节,支持手动输入和从搜索中获取
	char	    	key[64];        // 密码
	unsigned char 	connectFlag;    // 只读, 0-没有连接到无线路由;1-已经连接到无线路由
	unsigned char 	signalLevel;    // 只读, 信号强度, 1~5格
	char	reserved[14];
} PACK_ALIGN SYS_CONFIG_WIFI_CONNECT;

// 
// ftp录像相关参数
//
typedef struct SysConfigFtpRec
{
	char recFlag;        //1 启用ftp录像，0 不启用ftp录像
	char host[80];        //ftp服务器url
	char user[16];          //ftp用户名
	char passwd[16];    //ftp用户密码
	char reserved[16];  //保留
} PACK_ALIGN SYS_CONFIG_FTP_REC;

// 
// ftp上传客流信息相关参数
//
typedef struct SysConfigFtpUpload
{
	int nUploadInterval;        /* 上传文件间隔(单位:秒)(前台允许选择范围 |1|5|10|20|30| 分钟,需转化为秒) */
	int nUploadFileFormat;        /* 上传文件格式(用数值表示  1=时间段方式, 2= 详细数据方式) */
	int nRunFlag;                /* 是否开启Ftp上传客流信息功能, 0=关闭, 1=开启 */

	int  nPort;                    /* ftp服务器端口( 1 <= iPort <= 65535 ) */
	char strHost[64+1];            /* ftp服务器ip(形如: 192.168.1.10) */
	char strUserName[64+1];        /* ftp用户名 (不允许为空) */
	char strPassword[64+1];        /* ftp用户密码 (不允许为空) */

	char reserved[16];            /* 保留 */
} PACK_ALIGN SYS_CONFIG_FTP_UPLOAD;

typedef struct SysConfig	// 系统配置结构
{
	SYS_CONFIG_HEADER	        	header;
	SYS_CONFIG_BASE	            	base;
	SYS_CONFIG_NETWORK_F	        	network;
	SYS_CONFIG_USER	            	user[MAX_USER_NUM];
	SYS_CONFIG_PTZ_PROTOCOL	    	ptzProtocol[MAX_PTZ_NUM];
	SYS_CONFIG_PTZ_CONTROL	    	ptzControl[MAX_CHANNEL_NUM];
	SYS_CONFIG_VIDEO_BASE_PARAM		videoParam[MAX_CHANNEL_NUM];
	SYS_CONFIG_OSD_INFO	        	osdInfo[MAX_CHANNEL_NUM];
	SYS_CONFIG_VIDEO_ENCODE	    	videoEncode[MAX_CHANNEL_NUM];
	SYS_CONFIG_VIDEO_ENCODE	    	videoSubEncode[MAX_CHANNEL_NUM];
	SYS_CONFIG_AUDIO_ENCODE	    	audioEncode[MAX_CHANNEL_NUM];
	SYS_CONFIG_AUDIO_ENCODE	    	audioEncodeTalkback;
	SYS_CONFIG_EMAIL_PARAM	    	emailParam;
	SYS_CONFIG_AUTO_MAINTAIN_PARAM	autoMaintainParam;
	SYS_CONFIG_RECORD_PARAM	    	recordParam[MAX_CHANNEL_NUM];
	SYS_CONFIG_RECORD_PUBLIC		recordPublic;
	SYS_CONFIG_CHANNEL_PARAM		channelParam[MAX_CHANNEL_NUM];        
	SYS_CONFIG_ALARM_PARAM	    	alarmParam;
	SYS_CONFIG_RS232_PARAM	    	rs232Param;
	SYS_CONFIG_DOOR_RULE	    	doorRule;
	SYS_CONFIG_DATA_UPLOAD	    	dataUpload[MAX_DATA_UPLOAD_NUM];
	SYS_CONFIG_PC_PARAM	        	pcParam[MAX_CHANNEL_NUM];
	SYS_CONFIG_MIDWARE	        	midParam[MAX_MIDWARE_USER_NUM];
	SYS_CONFIG_PC_CONFIG	    	pcConfig;
	SYS_CONFIG_LP_PARAM	        	lpParam[MAX_CHANNEL_NUM];
	SYS_CONFIG_ABDOOR_PARAM	    	abDoorParam[MAX_CHANNEL_NUM];
	SYS_CONFIG_FIV_PARAM	    	fivParam[MAX_CHANNEL_NUM];
	SYS_CONFIG_FIV_CONFIG	    	fivConfig[MAX_CHANNEL_NUM];
	SYS_CONFIG_RF433M_PARAM	    	rf433mParam;
	SYS_CONFIG_EYE_CONFIG	    	eyeConfig[MAX_CHANNEL_NUM];
	SYS_CONFIG_IO_CONFIG	    	ioConfig;
	SYS_CONFIG_RF433M_PUBLIC		rf433mPublic;
	SYS_CONFIG_UBOOT_ENV_BACKUP		ubootEnvBackup;
	SYS_CONFIG_LEDSHOW_SETTING		ledShowSetting;
	SYS_CONFIG_LEDBOARD_SETTING		ledBoardSetting;
	SYS_CONFIG_GROUP_SETTING		groupSetting;
	SYS_CONFIG_WL_PLATFORM	    	wlPlatform;
	SYS_CONFIG_THREEG_NET_PARAM		threegNetParam;
	SYS_CONFIG_WIFI_NETWORK	    	wifiNetwork;
	SYS_CONFIG_WIFI_CONNECT	    	wifiConnect;
	SYS_CONFIG_SYNOVATE	        	synovateSys;
	SYS_CONFIG_FTP_REC             	ftpRec;
	SYS_CONFIG_RS485_PARAM	    	rs485Param;
	SYS_CONFIG_FTP_UPLOAD           ftpUpload;
} PACK_ALIGN SYSCONFIG;

#undef PACK_ALIGN

int ReadSysConfig();
int SyncSysConfig();
int WriteSysConfig();
void *WriteSysConfig( void *args );
void InitDefaultSysConfig();
int LsppInitSysConfig();
int InitThttpdConfig();
int InitLangConfig();

int GetSysConfig( SYSCONFIG *pSysConfig );
int SetSysConfig( SYSCONFIG *pSysConfig );
int GetSysConfig( int type, void *configBuf, int configLen, int n = 0 );
int SetSysConfig( int type, void *configBuf, int configLen, int n = 0 );

int IsHeaderValid( SYSCONFIG *pSysConfig );

#endif  // _SYS_CONFIG_H

