#ifndef __DRIVER_H__
#define __DRIVER_H__

#include "const.h"


#define TW2865_FILE     "/dev/tw2865dev"  
#define CX26828_FILE    "/dev/cx268xx"//"/dev/cx26828"//

#if defined MCU_HI3515A
#define AD_DRVFILE CX26828_FILE
#else
#define AD_DRVFILE TW2865_FILE
#endif

#define GPIOI2C "/dev/gpioi2c"


int DriverSetAdVideoCapParam( int channel, uint brightness, 
                    	uint contrast, short hue, uint saturation );
int DriverGetAdVideoCapParam( int channel, uint *brightness, 
                    	uint *contrast, uint *hue, uint *saturation );
int DriverGetAdVideoLoss( int channel );
int DriverSetAdVideoMode( int channel, int mode );
int DriverGetAdVideoMode( int channel );
void DriverVideoStandardDetectAddTimer(void);
void DriverInit();

#endif // __DRIVER_H__

