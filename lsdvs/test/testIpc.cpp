

#if 1
#include <unistd.h>
#include "rtc.h"
#include "debug.h"
#include "driver.h"
#include "gpio.h"

void TestTw2867()
{

#if 0   
	int mode;
	mode = DriverGetAdVideoMode( 0 );
	if (mode == -1) SVPrint("get mode error\n");
	else SVPrint("mode: %d\n", mode);
#endif

#if 0
	uint bri, ctr, hue, str;
	DriverGetAdVideoCapParam(0, &bri, &ctr, &hue, &str);
	SVPrint("bri: %d\n", bri);
	SVPrint("ctr: %d\n", ctr);
	SVPrint("hue: %d\n", hue);
	SVPrint("str: %d\n", str);
#endif

#if 0
	static int flag = 0;

	if (flag == 0) {
    	GpioRecordLedDisable();
    	flag = 1;
    }
	else {
    	GpioRecordLedEnable();
    	flag = 0;
    }
#endif	

#if 0   //  ÖÐÎçÆð´²ÄÖÖÓ
	int year; 
	int month; 
	int day;
	int hour;
	int minute;
	int second;
	int pweekday;

	RtcGetTime(&year, &month, &day, &hour, &minute, &second, &pweekday);

	if ( (hour == 13)&&(minute==21) ) {
    	GpioBuzzerEnable();
    } 
#endif

#if 0   //  ÉèÖÃ  ÁÁ¶È  ¶Ô±È¶È  ±¥ºÍ¶È  »Ò¶È  
	int i;

	SVPrint("\n\n\n\n ================================test 2867============================= \n\n\n\n\n");

	for (i=0; i<256; i++) {
    	DriverSetAdVideoCapParam(0, i, 200, 200, 200);
    	sleep( 1 );
    }
#endif	


#if 0	    //  ÊÓÆµ¶ªÊ§ ²âÊÔ
    	if ( 1 == DriverGetAdVideoLoss( 0 ) ) 
        {
        	GpioBuzzerEnable();
        	SVPrint("=========================loss===========================================================\n");
        }
    	else {
        	GpioBuzzerDisable();
        }
#endif	    

}
#endif






