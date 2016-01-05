#ifndef __SNAPMPI_H__
#define __SNAPMPI_H__

#define SNAP_CHANNEL_START	8

int SnapMpiGetJpg( int snapCh, char *buf, unsigned int *len );
void SnapMpiInit();
void SnapMpiDeinit();
int SnapMpiGetJpgAndToProcon( int snapCh, uint snapType );
void InitProconJpgFd();
void DeinitProconJpgFd();

#endif // __SNAPMPI_H__

