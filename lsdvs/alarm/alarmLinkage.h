#ifndef __ALARMLINKAGE_H__
#define	__ALARMLINKAGE_H__

typedef struct _AlarmLinkageOutput_
{
	unsigned int	linkageAlarmOut;    // 联动报警输出,每1bit标识一个报警输出
	unsigned char	alarmOutNum;        // 总共有多少个报警输出
	unsigned char	alarmOutDuration;    // 报警输出时长, 单位秒
	char	    	reserved[2];
} ALARM_LINKAGE_OUTPUT;

typedef struct _AlarmLinkageBuzzer_
{
	unsigned char   linkageBuzzer;        // 联动蜂鸣器 0: no linkage; 1: linkage;
	unsigned char	buzzerDuration;        // buzzer响输出时长, 单位秒
	char	    	reserved[2];
} ALARM_LINKAGE_BUZZER;

typedef struct _AlarmLinkageRecord_
{
	unsigned int	linkageRecord;        // 联动录像,每1bit标识一个通道	    
	unsigned int 	recordType;            // 录像类型
	unsigned char	recordDuration;        // 报警录像时长, 单位秒	
	char	    	reserved[3];
} ALARM_LINKAGE_RECORD;

typedef struct _AlarmLinkageCapture_
{
	unsigned int   	linkageCapture;        // 联动抓怕,每1bit标识一个通道
	unsigned int 	captureType;        // 抓拍类型
	unsigned char	captureNum;            // 抓怕张数
	char	    	reserved[3];
} ALARM_LINKAGE_CAPTURE;

typedef struct _AlarmLinkageEmail
{
	unsigned char   linkageEmail;        //联动eamil. 0: not send; 1: send;
	char	    	reserved[3];
} ALARM_LINKAGE_EMAIL;

void *AlarmLinkageOutput( void *args );
void *AlarmLinkageCapture( void *args );
void *AlarmLinkageRecord( void *args );
void *AlarmLinkageBuzzer( void *args );
void *AlarmLinkageEmail( void *args );

#endif 

