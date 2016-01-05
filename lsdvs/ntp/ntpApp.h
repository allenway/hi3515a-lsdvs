#ifndef __NTPAPP_H__
#define __NTPAPP_H__

typedef enum _NtpMaintain_
{
	NTP_MAINTAIN_WORK = 10,
	NTP_MAINTAIN_RESTART,
	NTP_MAINTAIN_NOT_ENABLE,
} NTP_MAINTAIN_EN;

typedef struct _NtpApp_
{
	unsigned char	enable;        // 是否使能
	char	    	zone;        // 当地时区
	unsigned short	interval;    // 隔多久对时一次,单位秒	
	char	    	host[64];    // 支持域名
	int	        	count;        // 当count == 0, 则把ntp时间写入系统和rtc,否则仅仅写入rtc	
} NTP_APP_T;

int NtpAppGetRunFlag();
int NtpAppMaintain();
int NtpAppSaveTime( int second );
int NtpAppGetParam( int *pInterval, char *pHost );

void StartNtpAppService();
void StopNtpAppService();
void NtpAppOutMaintain();

#endif // __NTPAPP_H__

