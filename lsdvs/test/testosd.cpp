#if 0
#include "linuxFile.h"
#include "fit.h"

int TestSetOsd(void)
{
    
    int channel = 0;

    ConfigOsdLogo osdLogo;
    ConfigOsdTime osdTime;

	osdLogo.enable = 1;
	osdLogo.colorRed = 0;
	osdLogo.colorGreen = 0xff;
	osdLogo.colorBlue = 0;
	osdLogo.xPos = 10;
	osdLogo.yPos = 10;
    // osdLogo.logo[64];
    Strcpy(osdLogo.logo, "BHDVS terminal 12345");

	osdTime.enable = 1;
	osdTime.colorRed = 0;
	osdTime.colorGreen = 0;
	osdTime.colorBlue = 0xff;
	osdTime.xPos = 40;
	osdTime.yPos = 40;

    for( channel = 0; channel < REAL_CHANNEL_NUM; channel++)
    {
        osdLogo.colorRed = 0;
        osdLogo.colorGreen = 0;
        osdLogo.colorBlue = 0;

        osdTime.colorRed = 0;
        osdTime.colorGreen = 0;
        osdTime.colorBlue = 0;
        if(channel%4 == 1)
        {
            osdLogo.colorRed = 0xff;
            osdTime.colorRed = 0xff;
        }
        else if(channel%4 == 2)
        {
            osdLogo.colorGreen = 0xff;
            osdTime.colorGreen = 0xff;
        }
        else if(channel%4 == 3)
        {
            osdLogo.colorBlue = 0xff;
            osdTime.colorBlue = 0xff;
        }
        

        FitFiOsdSetLogoOsdConfig( channel, &osdLogo);
        FitFiOsdSetTimeOsdConfig( channel, &osdTime );
    }

	return 0;    
}
#endif
