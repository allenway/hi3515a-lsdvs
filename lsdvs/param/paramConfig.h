//说明: 对于网络等特别重要的参数实行双备份的方式。

#ifndef __SYSCONFIG_H__
#define __SYSCONFIG_H__

//#define PACK_ALIGN	__attribute__((packed))

#include "const.h"
#include "paramConfDefault.h"
#include "ptypes.h"

#define MAX_PARAM_CONFIG_SIZE	    (128*1024)
#define PARAM_CONFIG_MAX_MEMBERS	200
#define PARAM_CONFIG_MARK	        "bhdvs--20130402"

#define PACK_ALIGN	__attribute__((packed))

//
// 系统配置结构,本结构体成员的顺序一旦定下来就不能做任何修改。
// 但是可以对某个成员结构体进行扩展
//
typedef struct _ParamConfig_	
{
#define		INDEX_PARAM_CONFIG_HEAD	            	0
           	PARAM_CONFIG_HEAD	                	head;
#define		INDEX_PARAM_CONFIG_BASE_INFO	    	1
        	PARAM_CONFIG_BASE_INFO                 	baseInfo;
#define		INDEX_PARAM_CONFIG_NETWORK             	2
        	PARAM_CONFIG_NETWORK                 	network;
#define		INDEX_PARAM_CONFIG_CLIENT_USER	    	3
        	PARAM_CONFIG_CLIENT_USER             	user[MAX_CLIENT_USER_NUM];
#define		INDEX_PARAM_CONFIG_VIDEO_ENCODE_PUBLIC	4
        	PARAM_CONFIG_VIDEO_ENCODE_PUBLIC		videoEncodePublic;

#define		INDEX_PARAM_CONFIG_VIDEO_ENCODE	    	5
        	PARAM_CONFIG_VIDEO_ENCODE             	videoEncode[MAX_CHANNEL_NUM];
#define		INDEX_PARAM_CONFIG_VIDEO_ENCODE_SLAVE	6
        	PARAM_CONFIG_VIDEO_ENCODE             	videoEncodeSlave[MAX_CHANNEL_NUM];
#define		INDEX_PARAM_CONFIG_VIDEO_BASE_PARAM		7
        	PARAM_CONFIG_VIDEO_BASE_PARAM         	videoBaseParam[MAX_CHANNEL_NUM];    
#define		INDEX_PARAM_CONFIG_OSD_LOGO	        	8
        	PARAM_CONFIG_OSD_LOGO                 	osdLogo[MAX_CHANNEL_NUM];
#define		INDEX_PARAM_CONFIG_OSD_TIME	        	9
        	PARAM_CONFIG_OSD_TIME                 	osdTime[MAX_CHANNEL_NUM];

#define		INDEX_PARAM_CONFIG_AUDIO	    	    10
        	PARAM_CONFIG_AUDIO_ENCODE               audioEncode;
#define		INDEX_PARAM_CONFIG_AUTO_MAINTAIN	    11
        	PARAM_CONFIG_AUTO_MAINTAIN         	    autoMaintion;
#define		INDEX_PARAM_CONFIG_RECORD_PUBLIC	    12
        	PARAM_CONFIG_RECORD_PUBLIC         	    recordPublic;
#define		INDEX_PARAM_CONFIG_RECORD_PARAM		    13
        	PARAM_CONFIG_RECORD_PARAM         	    recordParam[MAX_CHANNEL_NUM];
#define		INDEX_PARAM_CONFIG_ALARM_IO	    	    14
        	PARAM_CONFIG_ALARM_IO             	    alarmIo;

#define		INDEX_PARAM_CONFIG_SERIAL	        	15
        	PARAM_CONFIG_SERIAL                 	serial;
#define		INDEX_PARAM_CONFIG_NTP	            	16
        	PARAM_CONFIG_NTP	                	ntp;
#define		INDEX_PARAM_CONFIG_EMAIL	        	17
        	PARAM_CONFIG_EMAIL	                	email;
#define		INDEX_PARAM_CONFIG_ALARM_MOVE_DETECT	18
        	PARAM_CONFIG_ALARM_MOVE_DETECT	    	alarmMoveDetect[MAX_CHANNEL_NUM];
#define		INDEX_PARAM_CONFIG_VIDEO_OVERLAY		19
        	PARAM_CONFIG_VIDEO_OVERLAY	        	videoOverlay[MAX_CHANNEL_NUM];

#define 	INDEX_PARAM_CONFIG_ALARM_VIDEO_LOSE		20
        	PARAM_CONFIG_ALARM_VIDEO_LOSE	    	alarmVideoLose[MAX_CHANNEL_NUM];            
#define		INDEX_PARAM_CONFIG_FTP	            	21	
         	PARAM_CONFIG_FTP	                	ftpParam;
#define		INDEX_PARAM_CONFIG_SNAP_TIMER	    	22
        	PARAM_CONFIG_SNAP_TIMER	            	snapTimer[MAX_CHANNEL_NUM];
#define		INDEX_PARAM_CONFIG_ICMP	            	23
        	PARAM_CONFIG_ICMP	                	icmp;
#define		INDEX_PARAM_CONFIG_DDNS	            	24
        	PARAM_CONFIG_DDNS	                	ddns;
            
#define		INDEX_PARAM_CONFIG_DTU	            	25
        	PARAM_CONFIG_DTU	                	dtu;
#define 	INDEX_PARAM_CONFIG_WIFI_CONNECT	    	26
        	PARAM_CONFIG_WIFI_CONNECT_T	        	wifiConnect;
#define 	INDEX_PARAM_CONFIG_THREEG	        	27
        	PARAM_CONFIG_THREEG_T	            	threeg;
#define 	INDEX_PARAM_CONFIG_VIDEO_SHELTER		28
        	PARAM_CONFIG_ALARM_VIDEO_SHELTER		videoShelter[MAX_CHANNEL_NUM];

#define		INDEX_PARAM_CONFIG_TOTAL	        	29 //SYS_CONFIG 总共有多少个成员

} PACK_ALIGN SYS_CONFIG;

#undef  PACK_ALIGN

void InitParamConfig();
//其他模块调用该函数保存配置
int SaveParamConfig();
int SyncParamConfig();
void SysConfigAddTimer();
int GetParamConfig( int index, void *pBuf, int len, int n);
int SetParamConfig( int index, void *pBuf, int len, int n);

void SysConfigRestoreFactoryConf();
void SysConfigInitMemberAddr();

void SysConfigSetDefaultParam();
void ParamChangedReport(void);


//1 attention please:
/* !!! 注意, 为了实现参数配置的兼容: 要往 SYS_CONFIG 结构体里面添加成员, 
	一定要修改下面成员:
0. #define		INDEX_PARAM_CONFIG_TOTAL ***
1. static void SysConfigInitMemberAddr()
2. SysConfigpRestoreDefaultParam();
3. static void SysConfigInitStoreHead( PARAM_CONFIG_STORE_HEAD *pStoreHead )
4. int GetParamConfig( int index, char *pBuf, int len, int n)
5. int SetParamConfig( int index, char *pBuf, int len, int n)
6. paramManage.cpp 和 paramManage.h
*/

#endif

