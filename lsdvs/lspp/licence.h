/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此文件提供了操作licence文件所需要的相关函数接口
**  date           :  2011.04.06
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#ifndef _LICENCE_H
#define _LICENCE_H

#include "const.h"

#define LICENCE_FLAG	    	0x74CECEFA	        // LICENCE 标识

#define	OEM_FIRS	        	0x00	            // 
#define	OEM_STD	            	0x10	            // 标准版本
#define	OEM_LTY	            	0x11	            // 蓝泰源
#define OEM_SD	            	0x12	            // 桑达
#define OEM_PNM	            	0x13	            // 普诺玛
#define OEM_FT	            	0x14	            // 方通
#define OEM_ZHUNDIAN	    	0x15	            // 准点
#define	OEM_YW	            	0x16	            // 有为
#define	OEM_JSYD	        	0x17	            // 江苏移动
#define	OEM_WKP	            	0x18	            // 威康普

#define PF_HI3512	        	0	                // HiSi3512平台
#define PF_TI6446	        	1	                // TI6446平台
#define PF_LTY	            	2	                // lty 板子
#define PF_CIM	            	3	                // 老板一体机板子
#define PF_CAR	            	4	                // 车载dvs 板子
#define PF_NCIM	            	5	                // 带wifi 一体机板子
#define PF_WKP	            	6	                // 威康普合作板子

#define OD_TYPE_NULL	    	0x00	            // 不跑算法
#define OD_TYPE_PC	        	0x01	            // 数人算法
#define OD_TYPE_FIV	        	0x02	            // 运动目标检测算法
#define OD_TYPE_FOV	        	0x04	            // 失焦检测算法
#define OD_TYPE_FACE	    	0x08	            // 人脸检测算法
#define OD_TYPE_EYE	        	0x10	            // 人眼检测算法
#define OD_TYPE_FESMC	    	0x20	            // 人脸异常检测
#define OD_TYPE_LP	        	0x40	            // 车牌检测
#define OD_TYPE_ABDOOR	    	0x80	            // AB门

#define OD_FLAG_CLOSE	    	0	                // 关闭算法
#define OD_FLAG_OPEN	    	1	                // 打开算法

#define ENCODE_FLAG_CLOSE		0	                // 关闭编码
#define ENCODE_FLAG_OPEN		1	                // 打开编码

#define SERIAL_NORMAL	    	0	                // 串口类型 RS232
#define SERIAL_RS232	    	0	                // 串口类型 RS232
#define SERIAL_RS485	    	1	                // 串口类型 RS485 

#define PROTOCOL_NULL	    	0	                // 无效协议类型
#define PROTOCOL_FIRS	    	6	                // 通讯协议V2.1版(RS232)
#define PROTOCOL_LTY	    	1	                // 蓝泰源串口通讯协议(RS232)
#define PROTOCOL_YW_GPS	    	2	                // 友为GPS通讯协议(RS232)
#define PROTOCOL_JTX_GPS		3	                // 甲天行GPS通讯协议(RS232)
#define PROTOCOL_FIRS_COM		4	                // 通讯协议V3.0版(RS232)
#define PROTOCOL_FIRS_COUNTER	5	                // 计数器(TCP/IP RS232 RS485)
#define PROTOCOL_ZHUOYANG	PROTOCOL_FIRS_COUNTER	// 卓扬串口通讯协议(RS485)
#define PROTOCOL_NORMAL		PROTOCOL_FIRS_COUNTER	// 标准协议: 计数器(TCP/IP RS232 RS485)
#define PROTOCOL_FIRS_XS		7	                // -鑫盛通讯协议(RS485)
#define PROTOCOL_FIRS_ERRISSOM 	8	                // 乌鲁木齐车牌十张照片发送 RS232
#define PROTOCOL_JSYD_COM_GPS	9	                // 江苏移动客流统计 RS232
#define PROTOCOL_DLHZH         	10                     // 大连华展 RS485
#define PROTOCOL_JYZT         	11                     // 北京精英智通(RS232)
#define PROTOCOL_PCTY         	13                     // IPSOS项目，与通用平台通信(TCP)

#define PC_TYPE_PC	        	0	                // 数人系统类型:非车载数人
#define PC_TYPE_PC_BUS	    	1	                // 数人系统类型:车载数人
#define DATA_UPLOAD_NO	    	0	                // 不支持实时数据上传
#define DATA_UPLOAD_YES	    	1	                // 支持实时数据上传

//0x01表示老一体机，0x02表示新一体机，0x03表示2路LTY DVS，0x04表示2路wkp DVS，0x05表示4路车载DVS，0x06表示3512单路D1
#define DVS_TYPE_OLD_WHOLE_ONE_ROAD         	0x01
#define DVS_TYPE_NEW_WHOLE_ONE_ROAD         	0x02
#define DVS_TYPE_LTY_TWO_ROAD                 	0x03
#define DVS_TYPE_WKP_TWO_ROAD                 	0x04
#define DVS_TYPE_CAR_FOUR_ROAD                 	0x05
#define DVS_TYPE_HI3512_D1_ONE_ROAD         	0x06

#ifndef PACK_ALIGN
#define PACK_ALIGN __attribute__((packed))
#endif

typedef struct Licence
{
	unsigned int	flag;                            // 文件头标记
	unsigned char	oemVersion;                        // OEM版本
	unsigned char	platform;                        // 平台
	unsigned int	odType[MAX_CHANNEL_NUM];        // 算法类型: 可以同时支持多种算法 OD_PC|OD_FIV
	unsigned char	odFlag[MAX_CHANNEL_NUM];        // 是否开启算法
	unsigned char	encodeFlag[MAX_CHANNEL_NUM];    // 是否开启编码
	unsigned char	serialType;                        // 串口类型
	unsigned char	protocolType;                    // 协议类型
	unsigned char	pcType;                            // 数人系统类型:车载数人,非车载数人(如果为1则是带门磁的)
	unsigned char	dataUploadFlag;                    // 是否支持实时数据上传
	unsigned char	fivRf433mFlag;                    // 是否带次声的FIV
	unsigned char   led             : 1;            // LED标志 -1
	unsigned char	mainSlave	    : 1;            // 主从标志 - 2
	unsigned char	fivType         : 1;            // fiv不同的协议类型 -4	
	unsigned char	threeg	        : 1;            // 3G模块标志 - 8
	unsigned char	wifi	        : 1;            // wifi模块标志
	unsigned char	wlPlatform	    : 1;            // 乌鲁木齐车牌捕捉仪,如果为0,则客户端不需要显示平台服务器的配置
	unsigned char	bsl             : 1;            // 百世龙协议,(这个不需要客户端管理)
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
} PACK_ALIGN LICENCE;

#ifndef PACK_ALIGN
#undef PACK_ALIGN
#endif

int SysGetLicence( LICENCE *pLicence );
int SysIsPcBus();
int SysGetProtocolType();
int SysGetDataUploadFlag();
int SysGetPcType();
int SysGetOdType( int channel );
int SysGetOdFlag( int channel );
int SysGetEncodeFlag( int channel );
int SysGetWifiFlag();
int SysGetThreegFlag();
int SysGetOemType();
int SysGetCzcfFlag();
int SysGetSynovateFlag();
int SysGetBslFlag();
int SysGetShf();

unsigned char SysGetDvsType();

int SysGetQiruiFlag();
int SysGetSerialType();
int SysGetOemVersion();
int SysGetJyzhtFlag();
int SysGetRlweyeFlag();//列车疲劳检测,add at 2013.07.31
int SysGetBjgqFlag();

#ifdef __cplusplus
extern "C" {
#endif

int SysGetFivRf433mFlag();
int SysGetLedFlag();
int SysGetMasterSlaverFlag();

#ifdef __cplusplus
}
#endif

#endif  // _LICENCE_H

