#ifndef __CRC_H__
#define __CRC_H__

#ifdef __cplusplus
extern "C" {
#endif

unsigned short CRC16(unsigned char* buf, int len);
unsigned long CRC32(unsigned char* buf, int len);

#ifdef __cplusplus
}
#endif

#endif

