#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/reboot.h>
#include "timer.h"
#include "message.h"


void *Reboot(void *args)
{
    fflush(NULL);
    sync();
    sleep(1);
    reboot(RB_AUTOBOOT);
    return NULL;
}

void RebootAddTimer()
{
	AddRTimer( Reboot, NULL, 3 );
}

void RebootSendMessage()
{
    RebootAddTimer();
    //MessageSend(MSG_ID_REBOOT); //³öÏÖ¶Î´íÎó,tbd
}

