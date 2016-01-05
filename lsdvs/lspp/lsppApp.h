#ifndef __LSPPAPP_H__
#define __LSPPAPP_H__

void *DealMessageDataThread( void *args );
void *DealAVSendThread( void *args );

void LsppAppServiceStart();
void LsppAppServiceSop();


#endif // __LSPPAPP_H__

