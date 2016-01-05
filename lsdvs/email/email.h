#ifndef __EMAIL_H__
#define __EMAIL_H__

#define EMAIL_SEND_TYPE_IO_TEST     	1	// 测试邮件发送
#define EMAIL_SEND_TYPE_IO_PROBER     	2	// IO探头报警发送
#define EMAIL_SEND_TYPE_VIDEO_LOSE     	3	// 视频丢失报警发送
#define EMAIL_SEND_TYPE_MOTION         	4	// 视频移动报警发送邮件
#define EMAIL_SEND_TYPE_TIMER_CAPTURE 	5	// 定时抓怕发送邮件

typedef struct __send_mail_info_t_ 
{
	int ch;
	int emailSendType;
} send_mail_info_t;

#ifdef __cplusplus
extern "C" {
#endif

int hh_mail_startSendMail(int ch, int emailSendType);
int hh_mail_sendMail(send_mail_info_t *send_mail_info,char *param);

#ifdef __cplusplus
}
#endif 

#endif 

