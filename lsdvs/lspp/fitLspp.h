#ifndef __FITLSPP_H__
#define __FITLSPP_H__

#include "procon.h"
#include "avSendList.h"

#define PC_COUNT_TIME_NUM	2   // 每天支持设置两个时间段
#define PC_COUNT_TIME_LEN	12  // 12:20-13:30

unsigned short GetCheckSum( unsigned char *data, int len );
bool IsCheckSumOK( unsigned char *data, int len, unsigned short check );
bool IsCheckSumOK( unsigned char *data, int len );
void PassData( char * pData, int nLen );
int GetUpdateFlag();
int SetUpdateFlag( int flag );
void *GetUpgradeMemory( int size );
void ReleaseUpgradeMemory( void *addr );
int FiNetGetMacAddr( char *macAddr, int len );
int FiNetGetPppIpAddr( char *ipAddr, int len );
int GetWeekdaySeconds( int cTime );
int GetTimeZone();
void FiNetGenRandMac( char *pMac );
void RebootSystem();
void ProconH264ToVideoSendNode( PROCON_NODE_T *proconNode, VIDEO_SEND_NODE *pVideoSendNode );

#endif // __FITLSPP_H__

