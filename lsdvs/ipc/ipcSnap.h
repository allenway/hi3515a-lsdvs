#ifndef __IPCSNAP_H__
#define __IPCSNAP_H__

#include "const.h"

int IpcSnapAndToPcp( int snapCh, uint snapType );
void InitIpcSnapPcpFd();
void DeinitIpcSnapPcpFd();

#endif

