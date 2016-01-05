#ifndef __PTYPES_H__
#define __PTYPES_H__

#include "const.h"

#define PARAM_CONFIG_MAX_MEMBERS	200

#define PACK_ALIGN	__attribute__((packed))

//此结构体的大小固定为512bytes
typedef struct _ParamConfigStoreHead_
{
	uint	size;            // SYS_CONFIG 配置结构体的大小	
	uint 	totalMembers;    // SYS_CONFIG 有多少个成员
	ushort 	memberSize[PARAM_CONFIG_MAX_MEMBERS];    //每个结构体的大小
	char	mark[16];         //  标识
	char	reserved[80];
} PACK_ALIGN PARAM_CONFIG_STORE_HEAD;

// 
// 公共结构体
//
//1.时间
typedef struct _TimeSt_
{
	uchar 	hour;        
	uchar 	minute;
	uchar 	second;
	char	reserved[1];
} PACK_ALIGN TIME_ST;

//2.时间段
typedef struct _TimeSegSt_
{
	TIME_ST		timeStart;    
	TIME_ST		timeEnd;
} PACK_ALIGN TIME_SEG_ST;

//3.一天的工作时间选择
typedef struct _DayWorkTime_
{
	TIME_SEG_ST		timeSeg[MAX_DAY_TIME_SEG];    // 时间段 
	uchar	    	enableFlag;        // 1: 时间段有效, 0: 时间段无效
	char	    	reserved[3];
} PACK_ALIGN DAY_WORK_TIME;

//4.一周的工作时间
typedef struct _WeekWorkTime_
{
	DAY_WORK_TIME		day[MAX_WEEK_DAY];    // 一周的工作时间 
} PACK_ALIGN WEEK_WORK_TIME;

//5.串口配置
typedef struct _SerialSt_
{
	uint	baudRate;    // 波特率
	uchar	dataBits;    // 数据位
	uchar	stopBits;    // 停止位
	uchar	parity;        // 校验位
	uchar	comNum;        // 用哪一个串口
	char 	reserved[4];
} PACK_ALIGN SERIAL_ST;

// 配置结构体头
typedef struct _ParamConfigHead_
{
	char	signature[8];        // 配置文件标识	
	char	reserved[16];
} PACK_ALIGN PARAM_CONFIG_HEAD;

//
// 设备基本信息
//
typedef struct _ParamConfigBaseInfo_ 
{
	char	devModel[24];                // 设备型号	
	char	serialNo[16];                // 产品序列号	
	char	hardwareVersion[32];        // 硬件版本号
	char	softwareVersion[32];        // 软件版本号
	char	configureVersion[8];        // 配置版本号	
	char	devName[32];        // 设备名称
	uchar	chipNum;            // 3515a芯片数量
	uchar	channelNum;            // 实际的通道数
	uchar	videoInputNum;        // 视频输入端口个数
	uchar	videoOutputNum;        // 视频输出端口个数
	uchar	alarmInputNum;        // 告警输入端口个数
	uchar	alarmOutputNum;        // 告警输出端口个数	
	char	reserved[6];        // 
}PACK_ALIGN PARAM_CONFIG_BASE_INFO;

//
// 网络参数
//
// 1.DNS
typedef struct _ConfigNetworkDns_
{
	char	dns1[16];            // 主DNS
	char	dns2[16];            // 备用DNS
}PACK_ALIGN CONFIG_NETWORK_DNS;
// 2.端口
typedef struct _ConfigNetworkPort_
{
	ushort	searchPort;         // 搜索端口
	ushort	msgPort;            // 消息端口,流端口 = 消息端口 + 1.
	ushort	webPort;            // web端口
	char	reserved[14];
} PACK_ALIGN CONFIG_NETWORK_PORT;
// 3.网卡
typedef struct _ConfigNetworkCark_ 
{    
	char	mac[24];            // 物理地址
	char	ip[16];             // ip地址 
	char	netmask[16];        // 子网掩码
	char	gateway[16];        // 网关	
	char	broadAddr[16];      // 广播地址
	uchar	dhcpEnable;         // 1: DHCP, 0: STATIC
	uchar	enableFlag;         // 1: 启用, 0: 停用,有线网卡总是启用
	char	reserved[6];        
} PACK_ALIGN CONFIG_NETWORK_CARD;
//网络配置总结构体,没有考虑pppoe,3G,以后用到各自定义
typedef struct _ParamConfigNetwork_
{
	CONFIG_NETWORK_CARD     	wired;   // 有线
	CONFIG_NETWORK_CARD     	wifi;    // wifi
	CONFIG_NETWORK_DNS         	dns;     // dns
	CONFIG_NETWORK_PORT	    	port;    // 端口
} PACK_ALIGN PARAM_CONFIG_NETWORK;

//
// 用户参数
//
typedef struct _ParamConfigUser_ 
{
	char	name[32];          // 用户名
	char	pwd[32];           // 密码
	uint	permission;        // 每1 bit标识一种权限;
	uchar	level;             // 用户的级别:0,NONE,1,ADMIN,2,浏览用户;
	uchar	enable;            // 该用户是否生效 
	char	reserved[6];    
} PACK_ALIGN PARAM_CONFIG_CLIENT_USER;

//
// 视频基本参数
//
typedef struct _ParamConfigVideoBaseParam_
{
	uchar	brightness;            // 亮度, 0~255
	uchar	contrast;              // 对比度, 0~255
	uchar	saturation;            // 饱和度, 0~255
	uchar	exposure;              // 曝光度, 0~255
	short	hue;                   // 色度, -128~128
	char	reserved[6];
} PACK_ALIGN PARAM_CONFIG_VIDEO_BASE_PARAM;

//
// OSD参数配置
//
typedef struct _ParamConfigOsdLogo_
{
	uchar	enable;                // 是否显示OSD 1: enable, 0: disable
	uchar	colorRed;              // 红色
	uchar	colorGreen;            // 绿色
	uchar	colorBlue;             // 蓝色
	ushort	xPos;                  // 坐标
	ushort	yPos;                  // 坐标
	char	logo[64];              // 最大支持64Bytes的logo
	uchar	bgTransparence;        // 背景透明度 0(不透明)~100(100%透明),标示0%~100%
	char	reserved[7];
} PACK_ALIGN PARAM_CONFIG_OSD_LOGO;

typedef struct _ParamConfigOsdTime_
{
	uchar	enable;
	uchar	colorRed;    
	uchar	colorGreen;    
	uchar	colorBlue;    
	ushort	xPos;    // 单位(像素)
	ushort	yPos;
	uchar	bgTransparence;        // 背景透明度 0(不透明)~100(100%透明),标示0%~100%
	char	reserved[7];
} PACK_ALIGN PARAM_CONFIG_OSD_TIME;

//
// 视频编码参数
//
typedef struct _ParamConfigVideoEncodePublic_
{
	uchar		videoStandard;        // 视频制式: 0:PAL制、1:NTSC制   2:AUTO
	char		reserved[3];        // 
} PACK_ALIGN PARAM_CONFIG_VIDEO_ENCODE_PUBLIC;

typedef struct _ParamConfigVideoEncode_
{    
	uchar		resolution;            // 分辨率: 0:Qicf、1:Cif、3:HD1、4:D1 参考PIC_SIZE_E
	uchar		bitrateType;        // 位率类型: 0:VBR、1:CBR, 参考RC_MODE_E
	uchar		frameRate;            // 帧率,低6位标识具体帧率。bit7 和 bit8 标识帧率最大值,00:最大值为25,01:最大值为30,其它保留
	uchar		iFrameInterval;        // I帧间隔, [1,1000]
	uchar		preferFrame;        // 帧率优先, 0 or 1
	uchar		encodeType;            // 0:音视频流, 1:视频流
	ushort		bitrate;            // 位率, 单位kbits/秒, 200~3000
	char		reserved[4];        // 
} PACK_ALIGN PARAM_CONFIG_VIDEO_ENCODE;

//
// 音频编码参数
//
typedef struct _ParamConfigAudioEncode_
{
	uchar	sampleRate;            // 采样率, 0:8k, 1:16k, 默认值 0
	uchar	encodeType;            // 编码方式,0:adpcm, 1:g711, 2:g726, 默认值 0
	uchar	bitWidth;            // 位宽,0:8bits, 1:16bits, 默认值 1
	uchar	chMode;                // 通道模式, 0:单通道, 1:立体声, 默认值 0
	uchar	amrMode;            // amr编码模式
	uchar	amrFormat;            // amr打包格式
	char	reserved[10];
} PACK_ALIGN PARAM_CONFIG_AUDIO_ENCODE;

//
// 自动维护
//
typedef struct _ParamConfigAutoMaintain_
{
	TIME_ST	    	rebootTime[MAX_WEEK_DAY];    // 重启时间
	uchar	    	rebooDatetFlag;                // bit 0~6 分别标识星期日到星期六,该bit为1标识重启时间有效,反之无效
	char	    	reserved[7]; 
} PACK_ALIGN PARAM_CONFIG_AUTO_MAINTAIN;

//
// 录像参数
//
//1.录像公共参数
typedef struct _ParamConfigRecordPublic_
{    
	uint	delSpace;        // 硬盘录像空间报警值  >= 50M  <= 10000M
	uint	delSize;        // reserve,单位 M
	uchar	loopRecord;        // 循环录像,0:不支持,1:支持.
	uchar	preRecord;        // 预录,0:停止,1:开启. 
	uchar	switchFileTime;    // reserve,录像文件切换时间 10~30分钟(固定为20分钟)        
	char 	reserved[13];
} PACK_ALIGN PARAM_CONFIG_RECORD_PUBLIC;

//2.手动录像
typedef struct _ParamConfigRecordHand_
{
	uchar	recFlag;        // 0:停止手动录像,1:启动手动录像
	char	reserved[11];
} PACK_ALIGN PARAM_CONFIG_RECORD_HAND;

//3.录像总结构体
typedef struct _ParamConfigRecordParam_
{
	PARAM_CONFIG_RECORD_HAND	recHand;    //手动录像 
	WEEK_WORK_TIME	        	recTimer;    //定时录像
} PACK_ALIGN PARAM_CONFIG_RECORD_PARAM;

//
// 报警配置
//
//报警联动
typedef struct _ConfigAlarmlinkage_
{
	uint	linkageAlarmOut;    //联动报警输出,每1bit标识一个报警输出
	uchar	alarmOutDuration;    //报警输出时长, 单位秒, 1 ~ 255 
	char	unused[3];            // 对齐
	uint   	linkageCapture;        //联动抓怕,每1bit标识一个通道
	uchar	captureNum;            //抓怕张数, 1 ~ 255     
	char	unused1[3];            // 对齐
	uint	linkageRecord;        //联动录像,每1bit标识一个通道	
	uchar	recordDuration;        //报警录像时长, 单位秒, 1 ~ 255 
	uchar   linkageBuzzer;        //联动蜂鸣器 0: no linkage; 1: linkage;
	uchar	buzzerDuration;        //buzzer响输出时长, 单位秒, 1 ~ 255     
	uchar   linkageEmail;        //联动eamil. 0: not send; 1: send;
	uint	linkageEmailPicture;//联动email发送图片,每1bit标识一个通道
	char    reserve[36];
} PACK_ALIGN CONFIG_ALARM_LINKAGE_T;

//2.IO(可接探头)报警
typedef struct SysConfigAlarmIo
{    
	char	            	alarmDevName[32];    // 该IO所接设备的名字
	WEEK_WORK_TIME	    	armTimer;            // 定时布防
	CONFIG_ALARM_LINKAGE_T	linkage;            // 报警联动	
	uchar               	normalcy;            // 常态, 0:常闭; 1:常开	
	uchar               	armFlag;            // 0: 撤防; 1: 布防。若此字段为1,则“定时 布防”字段无效 
	char                	unused[2];    
	uchar                 	scoutInterval;        // 侦察间隔时间,单位秒 (10~255)
	char	            	reserved[7];
} PACK_ALIGN PARAM_CONFIG_ALARM_IO;

//
// 串口配置, 最多支持10哥对象使用串口(包括RS232和RS485) 
//
typedef struct _ParamConfigSerial_
{
	SERIAL_ST	ptz;        //同一个设备的所有ptz都使用同一个串口配置 
	SERIAL_ST	reserved[9];
} PACK_ALIGN PARAM_CONFIG_SERIAL;

//
// NTP配置
//
typedef struct _ParamConfigNtp_
{
	uchar	    	enable;        // 是否使能
	char	    	zone;        // 当地时区
	ushort	    	interval;    // 隔多久对时一次,单位秒	
	char	    	host[64];    // 支持域名
	char	    	reserved[12];        // 
} PACK_ALIGN PARAM_CONFIG_NTP;

//
// Email配置
//
typedef struct _ParamConfigEmail_
{    
	char serverIP[64];        // 发送邮件服务器, 如 smtp.163.com	
	ushort  port;            // 服务器端口, 默认是25
	char align[2];
	char loginName[64];      // 登录ID, 如你注册邮箱 abc@163.com,那么名字可以为abc,也可以为abc@163.com
	char password[32];        // 登录邮箱的密码
    char fromAddr[64];        // 邮件从该邮箱发出,        
    char userName[32];        // 别人收到邮件时看到的名字
    char toAddr[256];        // 接收人地址, 支持多个收件人    
	char subject[64];       // 邮件主题	
	char reserved[12];        // 
} PACK_ALIGN PARAM_CONFIG_EMAIL;

// 
// 移动侦测报警
//
// 移动侦测区域
typedef struct _ConfigMoveDetectArea_
{
	ushort	area[12];    //把一帧分成(12*16)个宏块,area[n]中的每一个bit标识第n行的每一个宏块是否被移动侦测检测
	char	reserved[8];
} PACK_ALIGN CONFIG_MOVE_DETECT_AREA;

typedef struct _ParamConfigAlarmMoveDetect_
{
	WEEK_WORK_TIME	    	armTimer;        // 定时布防
	CONFIG_ALARM_LINKAGE_T	linkage;        // 报警联动	
	CONFIG_MOVE_DETECT_AREA detectArea;        // 侦测区域
	uchar     	sensitiveLevel;        // 灵敏度,1~9,值越大灵敏度越高
	uchar     	scoutInterval;        // 侦察间隔时间,单位秒 
	uchar   	armFlag;            // 0: 撤防; 1: 布防。若此字段为1,则“定时 布防”字段无效 
	char		reserved[9];                
} PACK_ALIGN PARAM_CONFIG_ALARM_MOVE_DETECT;

//
// 视频遮挡
//
// 单个遮挡区域的参数
typedef struct _ConfigVideoOverlay_
{
	uint	color;
	ushort	x_start;
	ushort	y_start;
	ushort	width;
	ushort	height;
} PACK_ALIGN CONFIG_VIDEO_OVERLAY;//

typedef struct _ParamConfigVideoOverlay_
{
	uchar enable;
	uchar num;
	uchar align[2];    
	CONFIG_VIDEO_OVERLAY overlay[4];
} PACK_ALIGN PARAM_CONFIG_VIDEO_OVERLAY;

//
// 视频丢失报警
//
typedef struct _ParamConfigAlarmVideoLose_
{
	WEEK_WORK_TIME	    	armTimer;            // 定时布防
	CONFIG_ALARM_LINKAGE_T	linkage;            // 报警联动	
	uchar               	armFlag;            // 0: 撤防; 1: 布防。若此字段为1,则“定时 布防”字段无效 
	uchar                 	scoutInterval;        // 侦察间隔时间,单位秒 (10~255)
	char	            	reserved[5];
    
} PACK_ALIGN PARAM_CONFIG_ALARM_VIDEO_LOSE;

//
// ftp 参数
//
typedef struct _ParamConfigFtp_
{
	ushort 	enable;        // 是否使能
	ushort 	port;        // 服务器端口
	char	ip[64];                // 服务器ip
	char 	user[32];        // 用户名
	char   	passwd[32];        // 密码
	char	jpgUpChannel[16]; // 哪些通道通过ftp 上传jpg 图片, 0, 不上传; 1, 上传
	char	reserved[8];                
} PACK_ALIGN PARAM_CONFIG_FTP;

//
// 定时抓拍参数
//
typedef struct _ParamConfigSnapTimer_
{
	int	  enable;        // 是否使能
	int   interval;        // 抓拍间隔 10～36000 秒
	char  startTime[TIME_LEN];     // 开始时间	
	char  stopTime[TIME_LEN];     // 结束时间	    
} PACK_ALIGN PARAM_CONFIG_SNAP_TIMER;

//
// icmp 参数
//
typedef struct _ParamConfigIcmp_
{
	uchar 	enable;                    // 是否使能icmp
	char	unused[3];
	char	ipAddr[NET_ADDRSIZE];    // 被ping 的ip 地址
	int		interval;                // 正常情况下每隔多长时间发送一次心跳包 5～100s
	int		timeOut;                // 发送ping 包后等待接收的超时时间 5～100s
	int		fcount;                    // failed count, ping不通的情况下ping 多少次也就是ping 的-c 参数了 5～100 次
	int		finterval;                // 存在虚链路时，发送ICMP包的间隔时间 10～100s	
	char	reserve[8];
} PACK_ALIGN PARAM_CONFIG_ICMP;

//
// ddns 参数
//
typedef struct _ParamConfigDdns_
{
	uchar 	enable;                            // 是否使能ddns
	uchar	vender;                            // 见 DDNS_VENDER_EN
	ushort	port;                            // 服务器的端口
	char	userName[NORMAL_USER_NAME_LEN];    // 登陆的用户名
	char	passwd[NORMAL_PASSWD_LEN];        // 密码
	char	url[NORMAL_URL_LEN];            // 所使用的域名
	char	ifName[MAX_PATH_LEVEL];            // 接口名字, eth0/eth1/ppp0/rw0
	int		interval;                        // 更新的时间间隔 120~65535 s	
	char	reserve[8];
} PACK_ALIGN PARAM_CONFIG_DDNS;

//
// dtu
//
typedef struct _ParamConfigDtu_
{
	uchar 	enable;                        // 0,非使能; 1,使能
	uchar 	transProtocol;                // 传输协议, 0,tcp; 1,udp;
	ushort	serverPort;                    // 服务器端口
	char	severIp[NET_ADDRSIZE];        // 服务器IP 地址
	char	heartbeatContent[76];        // 心跳包的内容,最多32 个汉字,可包含文字、数字、英文大小写字母等
	ushort	interval;                    // 心跳包的发送间隔1～65535s
	char	reserve[10];
} PACK_ALIGN PARAM_CONFIG_DTU;

//
// wifi连接配置
//
typedef struct _ParamConfigWifiConnect_
{
	char	    	essid[32];        // 最多支持32个字节,支持手动输入和从搜索中获取
	char	    	key[64];        // 密码
	unsigned char 	connectFlag;    // 只读, 0-没有连接到无线路由;1-已经连接到无线路由
	unsigned char 	signalLevel;    // 只读, 信号强度, 1~5格
	char	    	reserved[14];
} PACK_ALIGN PARAM_CONFIG_WIFI_CONNECT_T;

typedef struct _ParamConfigThreeg_
{
	uchar 	enableFlag;    // 1,使用3G作为默认网络
	char	reserved[63];     
} PARAM_CONFIG_THREEG_T;

//
// 视频遮挡
//
typedef struct _ParamConfigVideoShelter_
{
	WEEK_WORK_TIME	    	armTimer;            // 定时布防
	CONFIG_ALARM_LINKAGE_T	linkage;            // 报警联动	
	uchar               	armFlag;            // 0: 撤防; 1: 布防。若此字段为1,则“定时 布防”字段无效 
	uchar                 	scoutInterval;        // 侦察间隔时间,单位秒 (10~255)
	uchar	sensitivity;   //灵敏度  见VIDEO_SHELTER_SENSITIVITY_EN
	char	reserved[5];     
} PARAM_CONFIG_ALARM_VIDEO_SHELTER;


#undef PACK_ALIGN

#endif 

