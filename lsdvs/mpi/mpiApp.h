#ifndef __MPIAPP_H__
#define __MPIAPP_H__

#include "const.h"

//int FiOsdInitOsdLib();
//int FiOsdDeinitOsdLib();

#ifdef __cplusplus
extern "C"{
#endif

void FiEncSendRestartMessage();
void EncAddTimer();

//void RestartVideoEncThread();
int H264FromHisiAddrToMyAddr( int channel, void *hisiH264 );
int H264FromHisiAddrToMyAddrAv( int channel, void *hisiH264, void *hisiAudio );
int AudioFromHisiAddrToMyAddr( int channel, void *hisiAudio );

int MpiServiceStart();
int MpiServiceStop();

#ifdef __cplusplus
}
#endif


#endif //

