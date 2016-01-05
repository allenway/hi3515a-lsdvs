/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此头文件提供了DVS传输及配置协议的函数接口
**  参考文档: <<DVS传输及配置协议.doc>> V1.0
**  date           :  2013.11.11
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#ifndef _CONFIG_PROTOCOL_H
#define _CONFIG_PROTOCOL_H

#include "ttypes.h"
#include "sysConfig.h"

//
// 数据包类型
//
enum MsgCfgDataPackType
{
	MSG_CFG_HEART_BEAT	                = 0x0000,        // 心跳
	MSG_CFG_GET_SYS_INFO	            = 0x0100,        // 获取系统版本信息
	MSG_CFG_RESET_DEFAULT_PARAM	        = 0x0200,        // 恢复出厂设置
	MSG_CFG_GET_SYS_TIME	            = 0x0300,        // 获取系统时间
	MSG_CFG_SET_SYS_TIME	            = 0x0400,        // 设置系统时间
	MSG_CFG_GET_NET_PARAM	            = 0x0500,        // 获取网络参数
	MSG_CFG_SET_NET_PARAM	            = 0x0600,        // 设置网络参数
	MSG_CFG_GET_VIDEO_PARAM	            = 0x0700,        // 获取视频参数
	MSG_CFG_SET_VIDEO_PARAM	            = 0x0800,        // 设置视频参数
	MSG_CFG_GET_USER_PARAM	            = 0x0900,        // 获取用户参数
	MSG_CFG_SET_USER_PARAM	            = 0x0A00,        // 设置用户参数
	MSG_CFG_GET_PTZ_CONTROL	            = 0x0B00,        // 获取云台控制参数
	MSG_CFG_SET_PTZ_CONTROL	            = 0x0C00,        // 设置云台控制参数
	MSG_CFG_GET_PTZ_PROTOCOL	        = 0x0D00,        // 获取云台控制协议 
	MSG_CFG_SET_PTZ_PROTOCOL	        = 0x0E00,        // 设置云台控制协议 
	MSG_CFG_GET_NTP_PARAM	            = 0x0F00,        // 获取NTP配置 
	MSG_CFG_SET_NTP_PARAM	            = 0x1000,        // 设置NTP配置 
	MSG_CFG_GET_OSD_PARAM	            = 0x1100,        // 获取通道OSD 参数 
	MSG_CFG_SET_OSD_PARAM	            = 0x1200,        // 设置通道OSD 参数 
	MSG_CFG_GET_EMAIL_PARAM	            = 0x1300,        // 获取邮件参数 
	MSG_CFG_SET_EMAIL_PARAM	            = 0x1400,        // 设置邮件参数 
	MSG_CFG_GET_AUDIO_ENCODE	        = 0x1500,        // 获取音频编码参数
	MSG_CFG_SET_AUDIO_ENCODE	        = 0x1600,        // 设置音频编码参数
	MSG_CFG_GET_VIDEO_ENCODE	        = 0x1700,        // 获取视频编码参数
	MSG_CFG_SET_VIDEO_ENCODE	        = 0x1800,        // 设置视频编码参数
	MSG_CFG_GET_CHANNEL_PARAM	        = 0x1900,        // 获取通道参数
	MSG_CFG_SET_CHANNEL_PARAM	        = 0x1A00,        // 设置通道参数
	MSG_CFG_GET_AUTO_MAINTAIN	        = 0x1B00,        // 获取自动维护参数
	MSG_CFG_SET_AUTO_MAINTAIN	        = 0x1C00,        // 设置自动维护参数
	MSG_CFG_GET_RECORD_PARAM	        = 0x1D00,        // 获取录像参数
	MSG_CFG_SET_RECORD_PARAM	        = 0x1E00,        // 设置录像参数
	MSG_CFG_SEARCH_PTZ_PROTOCOL	        = 0x1F00,        // 搜索云台控制协议
	MSG_CFG_GET_SYS_BASE	            = 0x2000,        // 获取系统基本参数
	MSG_CFG_SET_SYS_BASE	            = 0x2100,        // 设置系统基本参数
	MSG_CFG_SET_PTZ_CMD	                = 0x2200,        // 设置云台控制命令
	MSG_CFG_GET_SD_CARD_INFO	        = 0x2300,        // 获取SD卡信息
	MSG_CFG_GET_USB_HD_INFO	            = 0x2400,        // 获取USB硬盘信息	
	MSG_CFG_GET_ALARM_PARAM	            = 0x2500,        // 获取告警信息
	MSG_CFG_SET_ALARM_PARAM	            = 0x2600,        // 设置告警信息
	MSG_CFG_GET_RS232_PARAM	            = 0x2700,        // 获取RS232参数
	MSG_CFG_SET_RS232_PARAM	            = 0x2800,        // 设置RS232参数
	MSG_CFG_REBOOT_SYSTEM	            = 0x2900,        // 系统重启
	MSG_CFG_SEND_EMAIL	                = 0x2A00,        // 发送Email
	MSG_CFG_AUTH_RAND_NUM	            = 0x2B00,        // 获取认证随机数
	MSG_CFG_LOGIN_AUTH	                = 0x2C00,        // 登录认证
	MSG_CFG_SEARCH_RECORD	            = 0x2D00,        // 搜索录像
	MSG_CFG_SEARCH_SNAP_FILE	        = 0x2E00,        // 搜索抓拍文件
	MSG_CFG_GET_PC_PARAM	            = 0x2F00,        // 获取PC参数
	MSG_CFG_SET_PC_PARAM	            = 0x3000,        // 设置PC参数
	MSG_CFG_GET_LICENCE_INFO	        = 0x3100,        // 获取licence信息
	MSG_CFG_SET_MIDSYS_CONFIG	        = 0x3200,        // 设置中间件参数
	MSG_CFG_GET_MIDSYS_CONFIG	        = 0x3300,        // 获取中间件参数
	MSG_CFG_GET_DOOR_RULE	            = 0x3400,        // 获取门规则信息
	MSG_CFG_SET_DOOR_RULE	            = 0x3500,        // 设置们规则信息
	MSG_CFG_GET_PC_CONFIG	            = 0x3600,        // 获取PC配置
	MSG_CFG_SET_PC_CONFIG	            = 0x3700,        // 设置PC配置
	MSG_CFG_GET_LP_PARAM	            = 0x3800,        // 获取LP参数
	MSG_CFG_SET_LP_PARAM	            = 0x3900,        // 设置LP参数
	MSG_CFG_GET_DATA_UPLOAD	            = 0x3A00,        // 获取数据上传参数
	MSG_CFG_SET_DATA_UPLOAD	            = 0x3B00,        // 设置数据上传参数
	MSG_CFG_GET_ABDOOR_PARAM	        = 0x3C00,        // 获取AB门参数
	MSG_CFG_SET_ABDOOR_PARAM	        = 0x3D00,        // 设置AB门参数
	MSG_CFG_GET_FIV_PARAM	            = 0x3E00,        // 获取FIV参数
	MSG_CFG_SET_FIV_PARAM	            = 0x3F00,        // 设置FIV参数
	MSG_CFG_GET_FIV_CONFIG	            = 0x4000,        // 获取FIV配置
	MSG_CFG_SET_FIV_CONFIG	            = 0x4100,        // 设置FIV配置	
	MSG_CFG_GET_RF433_CONFIG            = 0x4200,       // 获取RF433传感器参数
    MSG_CFG_SET_RF433_CONFIG            = 0x4300,       // 设置RF433传感器参数
	MSG_CFG_GET_EYE_CONFIG	            = 0x4400,        // 获取EYE配置
	MSG_CFG_SET_EYE_CONFIG	            = 0x4500,        // 设置EYE配置
	MSG_CFG_CLEAR_PC_NUM	            = 0x4600,        // 清零命令
	MSG_CFG_GET_IO_CONFIG	            = 0x4700,        // 获取IO配置
	MSG_CFG_SET_IO_CONFIG	            = 0x4800,        // 设置IO配置
	MSG_CFG_GET_RF433M_PUBLIC	        = 0x4900,        // 获取RF433M公共配置
	MSG_CFG_SET_RF433M_PUBLIC	        = 0x4A00,        // 设置RF433M公共配置
	MSG_CFG_GET_LED_SHOW_SETTING	    = 0x4B00,       // 获取LED显示设置
	MSG_CFG_SET_LED_SHOW_SETTINT	    = 0x4C00,        // 设置LED显示设置
	MSG_CFG_GET_LED_BOARD_SETTING	    = 0x4D00,        // 获取LED单板设置
	MSG_CFG_SET_LED_BOARD_SETTING	    = 0x4E00,        // 设置LED单板设置	
	MSG_CFG_GET_MASTERSLAVER_SETTING	= 0x4F00,        // 获取主从设备设置
	MSG_CFG_SET_MASTERSLAVER_SETTING	= 0x5000,        // 设置主从设备设置	
	MSG_CFG_SET_URUMCHILP_SERVER_PARAM	= 0x5400,        // 设置乌鲁木齐车牌对接服务器参数
	MSG_CFG_GET_URUMCHILP_SERVER_PARAM	= 0x5500,        // 获取乌鲁木齐车牌对接服务器参数	
	MSG_CFG_GET_THREEG_PARAM	        = 0x5100,        // 获取3G设备参数
	MSG_CFG_SET_THREEG_PARAM	        = 0x5200,        // 设置3G设备参数
	MSG_CFG_GET_THREEG_STATE	        = 0x5300,        // 获取3G设备状态
	MSG_CFG_GET_WIFI_SCAN_RESULT	    = 0x5600,        // 获取wifi搜索设备的结果
	MSG_CFG_GET_WIFI_NETWORK_CONFIG     = 0x5700,        // 获取wifi网络配置
	MSG_CFG_SET_WIFI_NETWORK_CONFIG     = 0x5800,        // 设置wifi网络配置
	MSG_CFG_GET_WIFI_CONNECT_CONFIG	    = 0x5900,        // 获取wifi连接配置
	MSG_CFG_SET_WIFI_CONNECT_CONFIG	    = 0x5A00,        // 设置wifi连接配置
	MSG_CFG_GET_SYNOVATE_CONFIG	          = 0x5B00,        // /*synovate_add*/
	MSG_CFG_SET_SYNOVATE_CONFIG	          = 0x5C00,        // /*synovate_add*/
	MSG_CFG_GET_FTP_REC                 = 0x5D00,         // 获取FTP录像相关参数 
	MSG_CFG_SET_FTP_REC                 = 0x5E00,         // 设置FTP录像相关参数 
 	MSG_CFG_GET_RS485_PARAM	            = 0x5F00,        // 获取RS485参数
	MSG_CFG_SET_RS485_PARAM	            = 0x6000,        // 设置RS485参数
	MSG_CFG_GET_FTP_UPLOAD	            = 0x6100,        // 获取FTP上传客流信息相关参数 
	MSG_CFG_SET_FTP_UPLOAD                 = 0x6200,        // 设置FTP上传客流信息相关参数 
	MSG_CFG_GET_RLW_TXTPATH             = 0x6300,       // 列车疲劳应用获取报警文件路径
 	MSG_CFG_UPLOAD_ALARM                 = 0x6400,        // 人数上限报警上传
};

//
// 数据子类型
//
enum MsgCfgDataSubType
{
	MSG_CFG_DATA_REQUEST	            = 0x01,        // 请求
	MSG_CFG_DATA_RESPONSE	            = 0x02,        // 应答
	MSG_CFG_DATA_ERROR	                = 0x03,        // 出错
	MSG_CFG_DATA_ALARM	                = 0X04,        // 报警
};


#define PACK_ALIGN	__attribute__((packed))



//报警类型 
#define MSG_ALARM_PC_UPPER_LIMIT 0xA001 //人数上限告警 

//报警上传结构体 
typedef struct dvsAlarm 
{ 
	int 	channel;         //报警通道 
	int 	alarmType;         //报警类型
	char 	alarmTime[20];     //告警发生的时间,格式形如“2011-9-30 16:12:11” 
	char 	alarmInfo[64];     //报警描述 
	char 	reserved[128];

} PACK_ALIGN DVS_ALARM; 

//
// 系统配置信息
//
typedef struct ConfigSysInfo
{
	char	configureVersion[16];        // 配置版本号
	char	softwareVersion[16];        // 软件版本号
	char	hardwareVersion[16];        // 硬件版本号
	char	serialNo[16];                // 产品序列号
	int 	authorization;                // 为了与算法兼容采用int变量，表示是否授权
                                          /*0 表示初始化算法错误
                                          1	表示初始化算法成功，而且已经授权
                                          2 表示初始化算法成功，但是没有授权*/
} PACK_ALIGN CONFIG_SYS_INFO;

typedef struct ConfigBase 
{
	char	    	devName[32];        // 设备名称
	char	    	devId[32];            // 设备ID
	unsigned char	chipNum;            // 3512芯片数量
	unsigned char	channelNum;            // 实际的通道数
	unsigned char	ptzNum;                // 实际的云台控制协议数量
	int	        	langID;                // 语言ID
	char	timeZone;            // 时区 废掉
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
	int         	timeIntZone;
	int             iDevMACNum;            // 设备号,由MAC地址生成,生成函数为:FiNetConvertMacToInt(),不写入Flash
	unsigned int	shopID;                // 店铺号,大于等于0,默认为0,用于Ftp上传客流信息目录命名
} PACK_ALIGN CONFIG_BASE;

//
// 视频参数
//
typedef struct ConfigVideoParam
{
	unsigned char	channel;            // 通道号
	int	        	brightness;            // 各通道的亮度值 0-255
	int	        	contrast;            // 各通道的对比度值
	int	        	hue;                // 各通道的色差
	int	        	saturation;            // 各通道的饱和度
	int	        	exposure;            // 各通道的爆光度
} PACK_ALIGN CONFIG_VIDEO_PARAM;

//
// 网络参数列表
//
typedef struct ConfigNetwork
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

} PACK_ALIGN CONFIG_NETWORK;

//
// 3G设备参数
//
typedef struct ConfigThreegNetParam
{
	int 	enableFlag;        //0表示不使能3G网络，1表示使能3G网络.
	int 	connectState;    //0表示3G网络已断开，1表示3G网络已连接.--只读
	char 	wanIP[16];         //拨号成功后获取的公网IP，如果拨号失败，则该值填"0.0.0.0".--只读
	char 	reserve[64];    //预留定制功能
} PACK_ALIGN CONFIG_THREEG_NET_PARAM;

//
//3G设备状态
//
typedef struct ThreegDevState
{
	int 	signalState;     //3G网络信号状态，0表示无信号，1 ~ 5 共5格信号等级，5格信号表示信号强度最强.--只读
	char 	reserve[64];    //预留定制功能
} PACK_ALIGN CONFIG_THREEG_DEV_STATE;

//
// 用户参数
//
typedef struct ConfigUser 
{
	char	    	userName[32];        // 用户名
	char	    	pwd[32];            // 密码
	int	        	level;                // 用户的级别; 0,NONE,1,ADMIN,2,浏览用户;
	unsigned int	capability;            // 用户的权限;
} PACK_ALIGN CONFIG_USER;

typedef char PTZ_NAME[32];
typedef struct ConfigPtzName
{
	int	        	ptzNum;                // 协议数量
	PTZ_NAME		ptzName[1];            // 协议名称

} PACK_ALIGN CONFIG_PTZ_NAME;

//
// 云台协议配置
//
typedef struct ConfigPtzProtocol
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
} PACK_ALIGN CONFIG_PTZ_PROTOCOL;

//
// 云台控制配置
//
typedef struct ConfigPtzControl
{
	unsigned char	channel;            // 通道号
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
} PACK_ALIGN CONFIG_PTZ_CONTROL;

//
// NTP参数
//
typedef struct ConfigNtp
{
	unsigned char	ntpType;            // 0,NONE,1,PC,2,NTP
	unsigned short	ntpPort;            // 默认为123	
	char	    	ntpAddr[81];        // 非空时与此SNTP 服务器同步时间

} PACK_ALIGN CONFIG_NTP;
    
//
// OSD参数配置
//
typedef struct ConfigOsdInfo 
{
	unsigned char	channel;            // 通道号
	CONFIG_OSD_TIME	osdTime;            // OSD 时间
	CONFIG_OSD_LOGO	osdLogo;            // OSD Logo
} PACK_ALIGN CONFIG_OSD_INFO;

//
// 视频编码参数
//
typedef struct ConfigVideoEncode 
{
	unsigned char	channel;            // 通道号
	unsigned char	chType;                // 0:主码流, 1:子码流
	int	        	videoStandard;        // 视频制式: 0:PAL制、1:NTSC制
	int	        	resolution;            // 分辨率: 0:Qicf、1:Cif、2:HD1、3:D1
	int	        	bitrateType;        // 位率类型: 0:CBR、1:VBR
	int	        	quality;            // 画质
	int	        	frameRate;            // 帧率
	int         	iFrameInterval;        // I帧间隔
	int         	preferFrame;        // 帧率优先
	int         	qp;                    // 量化系数
	int         	encodeType;            // 音视频流:0、视频流:1、音频流:2
} PACK_ALIGN CONFIG_VIDEO_ENCODE;

//
// 音频编码参数
//
typedef struct ConfigAudioEncode
{
	unsigned char	channel;            // 通道号
	int	        	sampleRate;            // 采样率
	char	    	bitWidth;            // 位宽
	char	    	encodeType;            // 编码方式
	char	    	amrMode;            // amr编码模式
	char	    	amrFormat;            // amr打包格式
} PACK_ALIGN CONFIG_AUDIO_ENCODE;

//
// 邮件设置
//
typedef struct ConfigEmailParam
{
	int	        	activeFlag;
	char	    	serverName[64];
	int	        	serverPort;
	char	    	userName[64];
	char	    	password[32];
	char	    	senderName[32];
	char	    	emailTitle[64];
	char	    	recvAddr1[64];
	char	    	recvAddr2[64];
	char	    	recvAddr3[64];
} PACK_ALIGN CONFIG_EMAIL_PARAM;

//
// 自动维护
//
typedef struct ConfigAutoMaintain
{
	unsigned char	rebootFlag;            // 重启标识
	unsigned int	rebootDate;            // 0:none; 1～7:星期一～天; 8:everyDay;
	unsigned int	rebootTime;
	int	        	delFileFlag;        // 是否自动删除文件
	int	        	dayNumber;            // 多少天之前删除；

} PACK_ALIGN CONFIG_AUTO_MAINTAIN_PARAM;

//
// 录像参数
//
typedef struct ConfigRecordParam
{
	unsigned char	    	channel;    // 通道号
	CONFIG_RECORD_PUBLIC	recPublic;    // 公共参数
	CONFIG_RECORD_HAND		recHand;    // 手动录像参数
	CONFIG_RECORD_TIMER		recTimer;    // 定时录像参数
} PACK_ALIGN CONFIG_RECORD_PARAM;

//
// 通道参数
//
typedef struct ConfigChannelParam
{
	unsigned char	channel;            // 通道号
	char	    	channelName[32];    // 通道名称
	int	        	voiceInput;            // 输入音量
	int	        	voiceOutput;        // 输出音量
} PACK_ALIGN CONFIG_CHANNEL_PARAM;

//
// PTZ命令参数
//
typedef struct ConfigPtzCmd
{
	unsigned char	channel;            // 通道号
	char	    	cmd[32];            // 命令
	int	        	param;                // 参数
} PACK_ALIGN CONFIG_PTZ_CMD;

//
// 告警参数
//
typedef struct ConfigAlarmParam
{
	unsigned char	alarmInId;
	unsigned char   enalbeFlag;            // 0: close; 1: open;
	unsigned char   type;                // 0: 常开; 1: 常闭；
	char	    	alarmInName[32];
	CONFIG_ALARM_TIMER		alarmInDetectTimer;
	CONFIG_ALARM_TRIGGER	trigger;
	char            reserve[29];
} PACK_ALIGN CONFIG_ALARM_PARAM;

//
// SD卡信息
//
typedef struct ConfigSdCardInfo
{
	unsigned char   enalbeFlag;
} PACK_ALIGN CONFIG_SD_CARD_INFO;

//
// USB硬盘信息
//
typedef struct ConfigUsbHdInfo
{
	unsigned char   enalbeFlag;
} PACK_ALIGN CONFIG_USB_HD_INFO;

//
// RS232参数
//
typedef struct ConfigRs232Param
{
	unsigned int	baudRate;            // 波特率
	unsigned char	dataBits;            // 数据位
	unsigned char	stopBits;            // 停止位
	unsigned char	parity;                // 校验位
} PACK_ALIGN CONFIG_RS232_PARAM;

//
// RS485参数
//
typedef struct ConfigRs485Param
{
	unsigned int	baudRate;            // 波特率
	unsigned char	dataBits;            // 数据位
	unsigned char	stopBits;            // 停止位
	unsigned char	parity;                // 校验位
} PACK_ALIGN CONFIG_RS485_PARAM;
//
// 登录认证
//
typedef struct ConfigLoginAuth 
{
	char	    	userName[32];        // 用户名
	char	    	auth[32];            // 认证信息: md5(认证随机数+密码)
	int	        	level;                // 用户的级别: 0,NONE; 1,ADMIN; 2,浏览用户;
	unsigned int	capability;            // 用户的权限;
} PACK_ALIGN CONFIG_LOGIN_AUTH;

//
// 搜索请求信息
//
typedef struct ConfigSearchInfo
{
	unsigned char	channel;            // 通道号
	char	    	startTime[19];        // 开始时间(YYYY-MM-DD hh:mm:ss)
	char	    	endTime[19];        // 结束时间(YYYY-MM-DD hh:mm:ss)
	unsigned long	fileType;            // 文件类型
	int	        	offset;                // 偏移
} PACK_ALIGN CONFIG_SEARCH_INFO;

//
// 录像列表
//
typedef struct ConfigRecordNode
{
	char	    	recordName[128];    // 录像文件名
	char	    	startTime[19];        // 开始时间(YYYY-MM-DD hh:mm:ss)
	char	    	endTime[19];        // 结束时间(YYYY-MM-DD hh:mm:ss)
	unsigned long	recordLen;            // 录像长度
	unsigned long	recordType;            // 录像类型	
} PACK_ALIGN CONFIG_RECORD_NODE;

typedef struct ConfigRecordList
{
	int	            	totalNum;        // 总个数
	int	            	recordNum;        // 录像个数
	CONFIG_RECORD_NODE  recordList[1];    // 录像列表
} PACK_ALIGN CONFIG_RECORD_LIST;

//
// 抓拍列表
//
typedef struct ConfigSnapNode
{
	char	    	snapName[128];        // 抓拍文件名
	char	    	snapTime[19];        // 抓拍时间(YYYY-MM-DD hh:mm:ss)
	unsigned long	snapLen;            // 抓拍文件长度
	unsigned long	snapType;            // 抓拍文件类型	
} PACK_ALIGN CONFIG_SNAP_NODE;

typedef struct ConfigSnapList
{
	int	            	totalNum;        // 总个数
	int	            	snapNum;        // 抓拍文件个数
	CONFIG_SNAP_NODE	snapList[1];    // 抓拍文件列表
} PACK_ALIGN CONFIG_SNAP_LIST;

//
// PC参数
//
typedef struct ConfigPcParam
{
	unsigned char	channel;        // 通道号
	int	        	nWidth;            // 图像宽度
	int	        	nHeight;        // 图像高度
	int	        	nRoiLeft;        // 检测区域左边坐标
	int	        	nRoiTop;        // 检测区域顶边坐标
	int	        	nRoiRight;        // 检测区域右边坐标
	int	        	nRoiBottom;        // 检测区域底边坐标
	int	        	bOriVertical;    // 检测走动方向(数人头及AB门检测方向: 1 垂直走动；0 水平走动)
	int	        	nLoiterTime;     // 滞留检测时的滞留时间(单位：毫秒), 范围: 半秒～10秒(256帧/25帧)
	int	        	nFPSOfDet;        // 检测帧率: 根据实际跑的帧率设置，范围为：5帧/秒～25帧/秒(也可以设置为0, 表示此参数无效 )
	int	        	nMaxMatchDist;    // 最大人头匹配距离，如果人头超过这个距离就认为是新的人头，最大匹配举例，0～150（5个人头），默认为30（1个人头）
                                    // 或者别人的人头。默认值为30, 这个主要根据人在检测区域的
                                    // 移动速度来设置，如果移动较慢则设置小阀值，移动较快则设置大阀值。
	int	        	bOriEnter;        // 进出方向设置
	int	        	bOpenPcDetect;    // 是否开启数人算法
	int	        	videoMode;        // 视频模式: Cif, 1/9D1, QCif
	int	        	nStepMotion;    // 是否台阶运动; 1:台阶运动, 0:平地运动.
	int         	nMinHeadWidth;    /* 最小人头宽度，单位像素，默认40像素，最小30像素*/
    
	int         	nMaxHeadWidth;  /* 最大人头宽度，单位像素，默认60像素，最大100像素*/
	int         	bHighPrecision;    /* 使能高精度选项，提高统计值的可信度，但未必能提高或者降低统计值

                               ==TRUE，使能高精度，会启用静止黑点误数过滤等算法，适用于存在背景误统计严重的环境：
                                	正作用：静止黑点不会导致数人计数器像时钟一样不停数人；
                                	副作用：检测效果不好的人头（检测到的次数偏少）可能漏数；

                               ==FALSE，禁止高精度，关闭静止黑点误数过滤算法，适用于不存在背景误统计的环境：
                                	正作用：检测效果不好的人头（检测到的次数偏少）不至于都漏数；
                                	副作用：静止黑点可能导致数人计数器像时钟一样不停数人；

                            	综上所述，存在背景误统计严重的环境则设置为TRUE，否则设置为FALSE；*/
} PACK_ALIGN CONFIG_PC_PARAM;

//
// Licence信息
//
typedef struct ConfigLicenceInfo
{
	unsigned int	flag;                            // 文件头标记
	unsigned char	oemVersion;                        // OEM版本
	unsigned char	platform;                        // 平台
	unsigned int	odType[MAX_CHANNEL_NUM];        // 算法类型: 可以同时支持多种算法 OD_PC|OD_FIV
	unsigned char	odFlag[MAX_CHANNEL_NUM];        // 是否开启算法
	unsigned char	encodeFlag[MAX_CHANNEL_NUM];    // 是否开启编码
	unsigned char	serialType;                        // 串口类型
	unsigned char	protocolType;                    // 协议类型
	unsigned char	pcType;                            // 数人系统类型:车载数人,非车载数人
	unsigned char	dataUploadFlag;                    // 是否支持实时数据上传
	unsigned char	fivRf433mFlag;                    // 是否带次声的FIV
	unsigned char   led             : 1;            // 1 - LED标志 - 1
	unsigned char	mainSlave	    : 1;            // 2 - 主从标志 - 2
	unsigned char	fivType         : 1;            // 4 - fiv不同的协议类型 - 4	
	unsigned char	threeg	        : 1;            // 8 - 3G模块标志 - 8
	unsigned char	wifi	        : 1;            // 1 - wifi模块标志 - 1
	unsigned char	wlPlatform	    : 1;            // 2 - 乌鲁木齐车牌捕捉仪,如果为0,则客户端不需要显示平台服务器的配置
	unsigned char	bsl             : 1;            // 4 - 百世龙协议,(这个不需要客户端管理)
	unsigned char	synovate	    : 1;            // 思纬协议
	unsigned char	czcf	        : 1;            // 城中村人脸抓拍
	unsigned char	shelterf	    : 1;            // 遮挡人脸抓拍	
    unsigned char 	qirui	        : 1;            // 奇瑞汽车
    unsigned char   jyzht             : 1;            // 精英智通
    unsigned char   rlweye             : 1;            // 列车疲劳检测,add at 2013.07.31(无每十分钟的身份验证)
	unsigned char	bjgq	        : 1;             // 北京光桥(led)&扬州数人
	unsigned char	reserved	    : 2;            // 保留


	unsigned char	devType;                        // 设备类型, 0x01表示老一体机，0x02表示新一体机，0x03表示2路LTY DVS，0x04表示3路wkp DVS，0x05表示4路车载DVS，0x06表示3512单路D1
	char	    	unused[2];                        // 扩展	

} PACK_ALIGN CONFIG_LICENCE_INFO;


//
// 门规则参数
//
typedef struct ConfigDoorRule
{
	int		pcType;
	int		doorType;
	int		doorRule;
} PACK_ALIGN CONFIG_DOOR_RULE;

//
// 数人参数
//
typedef struct ConfigPcConfig
{
	char	bSaveHistoryData;
	char	countTime[2][12];            // 两个数人的时间段,时间段1默认:00:00-23:59;时间段2默认:0
    char    clrCountTime[2][9];         // JSYD， 江苏移动licence, 两个清零时间点。时间点形如：12:00:00.
   	char 	upModNum;                    //上车模式人数阀值
	char 	downModNum;                    //下车模式人数阀值
	char 	remainPercent;                //车内剩余人数与进总人数的百分比
	int		limitedNumber;            // 允许滞留的上限人数(进-出)    
	char	reserved[206];
} PACK_ALIGN CONFIG_PC_CONFIG;

//
// 车牌检测算法参数
//
typedef struct ConfigLpParam
{
	unsigned char	channel;                // 通道号
	int	        	nWidth;                    // 图像宽度
	int	        	nHeight;                // 图像高度	
	int	        	nMinTrackFrame;            // 最少跟踪帧数
	int	        	nMaxLostFrame;            // 最多失踪帧数
	int	        	nMaxDistance;            // 对象跟踪点最大跨度
	int	        	nDetectCarType;            // 是否检测车头: 0 禁止;  1 检测车头;  2 检测车尾
	int	        	bDetectPlate;            // 是否检测车牌: 1 使能; 0 禁止
	int	        	bEnableTrackCar;        // 是否启动跟踪: 1 使能; 0 禁止
	int	        	bEnableTrackPlate;        // 是否启动跟踪: 1 使能; 0 禁止
} PACK_ALIGN CONFIG_LP_PARAM;

//
// 数据上传协议
//
typedef struct ConfigDataUpload
{
	int		flag;            // 1为开启, 0为关闭
	char	ip[80];            // 服务器IP
	int		port;            // 服务器端口
	int		interval;        // 必须是10的整数倍
	int		mode;            // 1: 表示精简模式, 0: 表示普通模式
} PACK_ALIGN CONFIG_DATA_UPLOAD;

//
// AB门参数
//
typedef struct ConfigABDoorParam
{
	unsigned char	channel;            // 通道号
	int	        	nWidth;                // 图像宽度
	int	        	nHeight;            // 图像高度
	int	        	nRoiLeft;            // 检测区域左边坐标
	int	        	nRoiTop;            // 检测区域顶边坐标
	int	        	nRoiRight;            // 检测区域右边坐标
	int	        	nRoiBottom;            // 检测区域底边坐标
	int	        	bOriVertical;        // 检测走动方向(数人头及AB门检测方向: 1 垂直走动；0 水平走动)
	int	        	nLoiterTime;         // 滞留检测时的滞留时间(单位：毫秒)
	int	        	nFPSOfDet;            // 检测帧率
	int	        	nMaxMatchDist;        // 最大人头匹配距离，如果人头超过这个距离就认为是新的人头
                                        // 或者别人的人头。默认值为30, 这个主要根据人在检测区域的
                                        // 移动速度来设置，如果移动较慢则设置小阀值，移动较快则设置大阀值。
	int	        	bOriEnter;            // 进出方向设置
                                        // 垂直方向: 0表示从上到下是进入方向 1表示从下到上是进入方向 
                                        // 水平方向: 0表示从左到右是进入方向 1表示从右到左是进入方向
	int	        	bOpenABDoorDetect;    // 是否开启数人算法
	int	        	stayerNum;            // 滞留人数
	int	        	sensitivity;        // 灵敏度 (1 - 9)
	int	        	videoMode;            // 视频模式
} PACK_ALIGN CONFIG_ABDOOR_PARAM;

//
// Fiv参数
//
typedef struct ConfigFivParam
{
	unsigned char	channel;            // 通道号
	float	    	fAlpha;                // 背景学习率（0～0.3），默认取0.01
	float	    	fFactor;            // 判断阀值F（0.5～3.5），默认取2.5
	float	    	fT;                    // 判断阀值T（0.3~0.9），默认取0.7
	int	        	nWindowSize;        // 计算精度（最小计算窗口，1～5），默认取3（即：3x3）
	int	        	nTargetSize;        // 对象最小尺寸（单位为"像素"，1～10000），默认取36（6*6）
	int	        	nMaxLost;            // 最多丢失帧数（单位为"帧"，对象两个关键位置帧之间允许丢失的帧数），默认取3
	int	        	nMinFrame;            // 最少存在帧数（单位为"帧"，对象存在多少关键位置帧后判定为有效对象），默认取1
	int	        	nMaxDistance;        // 对象相临关键位置最大匹配距离（单位为“像素”，超过此距离，认为不是同一个对象），默认取10
} PACK_ALIGN CONFIG_FIV_PARAM;

//
// Fiv配置
//
typedef struct ConfigFivConfig
{
	unsigned char	channel;        // 通道号
	int	        	width;            
	int	        	height;
	int	        	left;            // 检测区域，默认是整个区域
	int	        	top;
	int	        	right;
	int	        	bottom;
	int	        	fivFlag;        // 0: 设防；1：撤防；
	int	        	fivType;        // 0: 无效；1: 禁区；2：逆行；3：越线
	struct RGN
    {
    	int rgnType;                // 0: 无效; 1: 矩形；2：圆形；3：线；4：箭头
    	struct Rect
        {
        	int x1;
        	int y1;
        	int x2;
        	int y2;
        } rect;
    } rgn[FIV_MAX_RECT_NUM];
} PACK_ALIGN CONFIG_FIV_CONFIG;

//
// RF433M配置
//
// 客户端请求的结构体
typedef struct ConfigRf433mReq
{
	unsigned char 	totalFlag;    // 是否请求所有的传感器配置, 0: 不是; 1: 是
	char	    	id[5];        // 请求这个ID号得传感器配置,如果totalFlag = 1,该字段无效
} PACK_ALIGN CONFIG_RF433M_REQ;

// 单个RF433M回应的结构体
typedef struct ConfigRf433mConfig
{
	char	    	id[5];                    // 从设备ID
	unsigned char	type;                    // 从设备类型
	char	    	name[32];                // 从设备名字
	unsigned char 	guard;                    // 0: 布防; 1: 设防
	CONFIG_ALARM_TIMER     	timerGuard;        // 定时布防,设防
	CONFIG_ALARM_TRIGGER	trigger;        // 触发哪些动作	
	char                 	reserved[10];    
} PACK_ALIGN CONFIG_RF433M_CONFIG;

//
// 疲劳检测配置
//
typedef struct ConfigEyeConfig
{
	unsigned char	        	channel;        // 通道号
	short	                	bFaceDetect;    // 是否检测人脸
    //CONFIG_EYE_SENSITIVITY	sensitivity[8];    // 检测灵敏度
	unsigned char             	sensitivity;
	char                         reserved[40];
} PACK_ALIGN CONFIG_EYE_CONFIG;

//
// IO配置
//
typedef struct ConfigIoConfig
{
	unsigned short	bNormalOpen;    // 常开：默认检测的结果为1；出现报警之后为0；常闭反之。

} PACK_ALIGN CONFIG_IO_CONFIG;

//
// 次声设备公共配置
//
typedef struct _ConfigRf433mPublic_
{
	unsigned char 	bSupportRf433m;        //1:支持次声设备; 0:不支持
	char	    	reserved[23];        
} PACK_ALIGN CONFIG_RF433M_PUBLIC;

//
// LED显示设置
//
typedef struct ConfigLedShowSetting
{    
	unsigned int	displaySetting;        // 按照上面枚举类型，OR方式
	char	    	customInfo[64];        // 客户定义显示的信息
	unsigned char	reserved[64];        // 预留
} PACK_ALIGN CONFIG_LEDSHOW_SETTING;

//
// LED数码板设置
//
enum MOVE_ACTION_STYLE
{
    	NOW         = 0x00, //-立刻显示
    	LEFT	    = 0x01, //-左移
    	RIGHT	    = 0x02, //-右移
    	UP	        = 0x03, //-上移
    	DOWN	    = 0x04, //-下移
    	L_CURTAIN	= 0x05, //-左拉幕
    	D_CURTAIN	= 0x06, //-下拉幕
    	R_CURTAIN	= 0x07, //-右拉幕
    	ARAB_R	    = 0x08, //-阿拉伯右移
    	H_WIN	    = 0x09, //-水平百叶窗
    	CABINET	    = 0x0A,    //-堆积木
    	V_WIN	    = 0x0B, //-垂直百叶窗
    	MID_CLOSE	= 0x0C, //-中合
    	MID_OPEN	= 0x0D, //-中开
    	UD_CLOSE	= 0x0E, //-上下合
    	UD_OPEN	    = 0x0F, //-上下开
    	H_CROSS	    = 0x10, //-水平穿插
    	V_CROSS	    = 0x11, //-上下交错
    	RAND	    = 0x12, //-随机
};
    
enum ROL_FRAME_STYLE
{
    	NONE	    = 0x00, //-无环绕闪烁
    	RED4	    = 0x10, //-红4点
    	GREEN4	    = 0x20, //-绿4点
    	YELLOW4	    = 0x30, //-黄4点
    	RED1	    = 0x40, //-红1点
     	GREEN1	    = 0x50, //-绿1点
    	YELLOW1	    = 0x60, //-黄1点
    	RED_S_F	    = 0x70, //-红单线闪烁
    	GREEN_S_F	= 0x80, //-绿单线闪烁
    	YELLOW_S_F	= 0x90, //-黄单线闪烁
    	RED_S_ROL	= 0xA0, //-红单线环绕
    	GREEN_S_ROL	= 0xB0, //-绿单线环绕
    	YELLOW_S_ROL= 0xC0, //-黄单线环绕
    	RED_D_ROL	= 0xD0, //-红双线环绕
    	GREEN_D_ROL	= 0xE0, //-绿双线环绕
    	YELLOW_D_ROL= 0xF0, //-黄双线环绕
};

enum DISPLAY_COLOR
{
    	SINGLE_DISPLAY	= 0x01,     // 单色屏
    	D_REG	        = 1 << 0,     // 红色
    	D_GREEN	        = 1 << 1,     // 绿色
    	D_YELLOW	    = D_REG|D_GREEN,// 黄色
};

typedef struct ConfigLedBoardSetting
{
	unsigned char 	move_action;        // 动画方式
    unsigned char 	move_speed;         // 动画速度, 0x00-0x0F，数字越大越快
    unsigned char 	rol_frame;          // 环绕边框, 取高4位
    unsigned char 	stop_delay;         // 停留时间, 0x00-0xFF，秒数。比如屏幕可以显示8个字，当前信息有16个字，那么前8个字移动完毕后，可以选择在屏幕上停留一会儿再显示后8个字。
    unsigned char 	color;              // 显示颜色
    unsigned char 	font_class;         // 字体类型, 暂时用默认0x01
	unsigned char	reserved[64];        // 预留
} PACK_ALIGN CONFIG_LEDBOARD_SETTING;

//
// (主从设备)
//
typedef struct ConfigGroupSetting
{
	unsigned char	enable;                // 使能标志=1使能，=0不使能
	unsigned char	masterTag;            // =1主设备，=0从设备
	char	    	ip[80];                // 主设备显示自身IP,从设备显示要连接的主设备IP
	unsigned short	syncInterval;        // 从设备同步信息间隔	
	unsigned char	clrHour;            // 清零小时
	unsigned char	clrMinute;            // 清零分钟（一天的开始时间）
	unsigned int	limitPeople;        // 场内限制人数
	unsigned int	alarmPeople;        // 场内预警人数
	unsigned char	reserved[64];        // 预留
} PACK_ALIGN CONFIG_GROUP_SETTING;

// 
// 乌鲁木齐平台
//
typedef struct _WlPlateformServerParam_
{
	char	    	ip[80];         // IP
	unsigned short	port;            // 端口
	int	        	devId;            // 由MAC生成的设备ID,供协议使用,在客户端显示,让客知道设备与图片的对应关系。
	char	    	reserved[60];    // 保留
} PACK_ALIGN WLPLATFORM_SERVER_PARAM;

//
// (1)wifi网络参数
//
typedef struct _ConfigWifiNetwork_
{
	unsigned char	enableWifi;            // 是否启用无线网络,1-启用;0-不启用
	unsigned char	netType;            // 0-DHCP, 1-STATIC, 2-ADSL
	char	    	ip[16];                // ip
	char	    	netmask[16];        // 子网掩码
	char	    	gateway[16];        // 网关
	char	    	broadAddr[16];        // 广播地址	
	char	    	mac[24];            // 物理地址,只读
	char	    	useWifiGateway;        // 使用wifi网关作为默认网关,0-否,1-是
	char	    	reserved[25];        
} PACK_ALIGN CONFIG_WIFI_NETWORK;

/*synovate_add*/
typedef struct _NetConfigSynovate
{    /*20个int类型，8个short类型，其他char类型*/        
	int setdaylight;
	int keepInt[19];
	unsigned short	synovatePort;            
	unsigned short	other;            
	unsigned short  keepShort[6];

	char fdaylight;
	char start[6];
	char end[6];
	char Ch;    
	char sn[6];        /*20个*/
	char	    	synovateIp[41];                    
	char	    	reserved[48];        // 用于扩展
} PACK_ALIGN NETWORK_SYNOVATE;


// 
// ftp录像相关参数
//
typedef struct configFtpRec
{
	char recFlag;        //1 启用ftp录像，0 不启用ftp录像
	char host[80];        //ftp服务器url
	char user[16];          //ftp用户名
	char passwd[16];    //ftp用户密码
	char reserved[16];  //保留
} PACK_ALIGN CONFIG_FTP_REC;


//
// 数人清零时间段参数(江苏移动)
//
typedef struct ConfigClearPcPeriod
{
	char firstPeriod[9];  //第一个时间段，形如：12:00:00.
	char secondPeriod[9]; //第二个时间段，形如：12:00:00.
} PACK_ALIGN CONFIG_CLEAR_PC_PERIOD;

// 
// ftp上传客流信息相关参数
//
typedef struct ConfigFtpUpload
{
	int nUploadInterval;        /* 上传文件间隔(单位:秒)(前台允许选择范围 |1|5|10|20|30| 分钟,需转化为秒) */
	int nUploadFileFormat;        /* 上传文件格式(用数值表示  1=时间段方式, 2= 详细数据方式) */

	int  nRunFlag;                /* 是否开启Ftp上传客流信息功能, 0=关闭, 1=开启 */
	int  nPort;                    /* ftp服务器端口( 1 <= iPort <= 65535 ) */
	char strHost[64+1];            /* ftp服务器ip(形如: 192.168.1.10) */
	char strUserName[64+1];        /* ftp用户名 (不允许为空)*/
	char strPassword[64+1];        /* ftp用户密码 (不允许为空) */

	char reserved[16];            /* 保留 */
} PACK_ALIGN CONFIG_FTP_UPLOAD;

//
//列车疲劳检测
//
typedef struct
{
    char txtpath[50];//存放报警信息报表文件的路径
    int  nFileLen;   //文件长度
} PACK_ALIGN RLW_TXT_PATH;

#undef PACK_ALIGN

#define DATA_READ	0
#define DATA_WRITE	1

//void ChangeTimeToInt( char tBuf[19], time_t &t );
int DealConfigDataProcess( unsigned char *dataBuf, int &dataLen, const int bufSize );
int CheckConfigDataProcess( unsigned char *dataBuf, int dataLen, int bufSize, int &offset );
int CfgManageSetPcClsCountTime( CONFIG_CLEAR_PC_PERIOD *clsPeriod );
extern int WifiScanResult( WIFI_SCAN_RESULT **pWifiScanResult, int *pScanSize );
int GetAlarmPack( unsigned char *dataBuf, int &dataLen, const int bufSize );

int ConfigProtocolEmpowerment( int type, int *pData );

#endif  // _CONFIG_PROTOCOL_H

