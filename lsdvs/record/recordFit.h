#ifndef __RECORDFIT_H__
#define __RECORDFIT_H__

#include <sys/time.h>

int IsRecordingFile(const char *checkName);

/*Ë÷ÒıÎÄ¼ş½á¹¹Ìå*/
typedef struct _RecordIndex
{
	uint indexVersion;    ////Ö¡±êÊ¶,0x1FDF0301,ËÄ¸ö×Ö½ÚµÄÒâÒå | 31 | 255-31È¡×î½üÖÊÊı | 3 | 1 | //Ç°Á½¸ö×Ö½ÚÊÇ¹Ì¶¨µÄ,ºóÁ½¸ö×Ö½ÚÊÇ¿ÉÀ©Õ¹µÄ
	uint recordType;    //Â¼ÏñÀàĞÍ
	uint fileLen;        //ÎÄ¼ş³¤¶È
	uint videoFrameSum;        //ÊÓÆµ×ÜÖ¡Êı
	time_t firstFrameTimestamp; //µÚÒ»Ö¡IÖ¡µÄÊ±¼ä´Á
	time_t lastFrameTimestamp;    //×îºóÒ»Ö¡µÄÊ±¼ä´Á
}RECORD_INDEX;


#define RECORD_FILENAME_LEN		128

#endif // __RECORDFIT_H__

