/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此头文件提供了对flash进行读写操作的函数接口
**  date           :  2013.11.11
**
**  version       :  1.0
**  author        :  sven

**  修改：添加系统日志读写函数
**  author        :  myf
**  日期：2013.04.10
*******************************************************************************
*/
#ifndef _FLASH_H
#define _FLASH_H

#include "norflash.h"   

#define FLASH_DEVICE_UBOOT	    	MTD_PARTITION_UBOOT
#define FLASH_DEVICE_UBOOT_ENV		MTD_PARTITION_UBOOT
#define FLASH_DEVICE_KERNEL	    	MTD_PARTITION_KERNEL
#define FLASH_DEVICE_ROOTFS	    	MTD_PARTITION_ROOTFS
#define FLASH_DEVICE_CONFIG	    	MTD_PARTITION_CONFIG
#define FLASH_DEVICE_LOG	    	MTD_PARTITION_CONFIG
#define FLASH_DEVICE_DATA	    	MTD_PARTITION_OTHER
#define FLASH_DEVICE_VSIP	    	MTD_PARTITION_OTHER

// start with FLASH_DEVICE_UBOOT
#define FLASH_SIZE_UBOOT_BIN	    (384*1024)        /* UBOOT程序 */
#define FLASH_OFFSET_UBOOT_BIN		0

// start with FLASH_DEVICE_UBOOT_ENV
#define FLASH_SIZE_UBOOT_ENV	    (128*1024)        /* UBOOT环境变量 */
#define FLASH_OFFSET_UBOOT_ENV	    (0+FLASH_SIZE_UBOOT_BIN)

// start with FLASH_DEVICE_KERNEL
#define FLASH_SIZE_KERNEL	        (2*1024*1024)    /* 内核 */
#define FLASH_OFFSET_KERNEL	    	0

// start with FLASH_DEVICE_ROOTFS
#if defined MCU_HI3515
#define FLASH_SIZE_ROOTFS	        (16*1024*1024)    /* 文件系统 */
#define FLASH_OFFSET_ROOTFS	    	0
#else
#define FLASH_SIZE_ROOTFS	        (11*1024*1024)    /* 文件系统 */
#define FLASH_OFFSET_ROOTFS	    	0
#endif


// start with FLASH_DEVICE_CONFIG
#define FLASH_SIZE_CONFIG	        (128*1024)        /* 配置文件 */
#define FLASH_OFFSET_CONFIG	    	0
#define FLASH_OFFSET_CONFIG_BACKUP	FLASH_SIZE_CONFIG

// start with FLASH_DEVICE_LOG
#define FLASH_SIZE_LOG	            (128*1024)        /* 日志文件 */
#define FLASH_OFFSET_LOG	        (FLASH_OFFSET_CONFIG_BACKUP + FLASH_SIZE_CONFIG)
#define FLASH_OFFSET_LOG_BACKUP	    (FLASH_OFFSET_LOG + FLASH_SIZE_LOG)



typedef struct FlashMsg
{
	char *	mtd;
	int		offset;
	char *	data;
	int		len;
}  FLASH_MSG;

// 写 FLASH消息类型
typedef enum _WriteFlashMsgType_
{
    WRITE_FLASH_MSG_PROGRESS = 0,   // 进度
    WRITE_FLASH_MSG_RESULT,         // 结果
} WRITE_FLASH_MSG_TYPE_EN;

// 写FLASH 消息类型
typedef struct _WriteFlashMsg_
{
    int msgType;
    int msgData;
} WRITE_FLASH_MSG_T;


int ReadFlash( char *mtd, int offset, char *pData, int nLen );
int WriteFlash( char *mtd, int offset, char *pData, int nLen );
int WriteFlashMsg( char *mtd, int offset, char *pData, int nLen );
void *WriteFlashImp( void *args );

//
// 系统配置
//
int ReadConfig( char* pConfigBuf, int nBufLen );
int ReadConfigBackup( char* pConfigBuf, int nBufLen );
int WriteConfig( char* pConfigBuf, int nBufLen );
int WriteConfigBackup( char* pConfigBuf, int nBufLen );
int WriteConfigMsg( char* pConfigBuf, int nBufLen );

//
// 系统日志
//
int ReadLog( char* PLogBuf, int nBufLen );
int ReadLogBackup( char* PLogBuf, int nBufLen );
int WriteLog( char* PLogBuf, int nBufLen );
int WriteLogBackup( char* PLogBuf, int nBufLen );
int WriteLogMsg( char* pLogBuf, int nBufLen );

//
// 软件升级
//
int UpgradeUBoot( char *pData, int nLen );
int UpgradeKernel( char *pData, int nLen );
int UpgradeRootfs( char *pData, int nLen );

//
// 操作u-boot环境变量
// 注意!!! GetUbootEnv函数为非线程安全接口
//
char* GetUbootEnv( char* pEnv );
int SetUbootEnv( char* pEnv );
// u-boot参数
void EraseUbootEnv();

#endif  // _FLASH_H

