/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**	文件名: hdd.h
**  description  : for hdd.cpp
**  date           :  2013.10.18
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/

#ifndef __HDD_H__
#define __HDD_H__

#include "const.h"

#define FAT32	"vfat"

#define MIN_PARTITION_SIZE	    	50	//录像最小可用分区大小 50MBytes
#define MIN_PARTITION_SIZE_OTHER	10	//其他应用最小可用分区大小 10MBytes

#define MAX_HDD_PARTITION_NUM	32	    //最多支持32个分区
#define MAX_HDD_PATH_NAMESIZE	64	    //硬盘路径最大字节数

#define UEVENT_BUFFER_SIZE 2048

#define MMC_INSERT_MARK	    "add@/block/mmcblk0/mmcblk0p"
#define MMC_REMOVE_MARK	    "remove@/block/mmcblk"

//usb insert mark:add@/devices/platform/hisilicon-ohci.0/usb1/1-1/1-1:1.0/host*/target*:0:0/0:0:0:0
//note:but "/target*" is not used.
#define USB_INSERT_MARK_FIRST	"add@/devices"
#define USB_INSERT_MARK_SECOND	"/host"
#define USB_REMOVE_MARK_FIRST	"remove@/devices"
#define USB_REMOVE_MARK_SECOND	"/host"

//MMC 的设备目录
// #define MMC_DEV_PATH	        "/dev/mmc/blk0"
#define MMC_DEV_PATH	        "/dev/"


//USB的设备目录
//usb dev path:/dev/scsi/host*/bus0/target0/lun0/part*
#define USB_DEV_PATH_FIRST	    "/dev/scsi"
#define USB_DEV_PATH_SECOND	    "bus0/target0/lun0"


#define MMC_MOUNT_PATH	        "/hdd/mmcp"    //EXAMPLE:hdd/mmcp0
#define USB_MOUNT_PATH	        "/hdd/sda"    //EXAMPLE:/hdd/sda0

//已mount的分区标识
#define MMC_MOUNT_PARTITION_MARK	        "mmcp"
#define USB_MOUNT_PARTITION_MARK	        "sda"

// 分区标识
#define MMC_DEV_PARTITION_MARK	        "mmcblk0p"
#define USB_DEV_PARTITION_MARK	        "part"

//磁盘或MMC/SD卡mount的根目录
#define HDD_ROOT_DIR	"/hdd"    

typedef enum _HddState_
{
	HDD_STATE_START = 0,    // 开始
	HDD_STATE_NORMAL,    // 常规
	HDD_STATE_STOP,            // 停止
	HDD_STATE_SLEEP,        // 休眠
} HDD_STATE_EN;

typedef enum _HddDevTypeSt
{
	HDD_TYPE_MMC = 0,
	HDD_TYPE_USB
}HDD_DEV_TYPE_ST;

typedef enum _HddStatusSt
{
	HDD_NONE = 0,            //没有检测到/移除
	HDD_DETECT,                //检测到
	HDD_MOUNTED,            //已经mount上
	HDD_MOUNT_FAILED,        //mount失败
	HDD_UMOUNT_FAILED	    //umount失败
}HDD_STATUS_ST;

typedef struct _HotPlugManage
{
	int mmcInsertFlag;
	int mmcRemoveFlag;
	int usbInsertFlag;
	int usbRemoveFlag;
}HOTPLUG_MAMAGE;

typedef struct _HddPartitionInfoSt
{
	char	devPath[MAX_HDD_PATH_NAMESIZE];            //分区设备路径
	char 	mountPath[MAX_HDD_PATH_NAMESIZE];        //分区mount路径
	int 	mountFlag;                                //是否已经mount上 HDD_STATUS_ST
	int 	type;                                    //分区设备类型 HDD_DEV_TYPE_ST
	long 	totalSize;                                //分区总大小 单位MBytes
	long 	freeSize;                                //分区剩余空间 单位MBytes	
}HDD_PARTITION_INFO_ST;

typedef struct _HddManageSt
{
	int totalMounted;                        //所有已经mount上去的分区	
	HDD_PARTITION_INFO_ST	usingPart;        //当前分区列表
	HDD_PARTITION_INFO_ST	partHead[MAX_HDD_PARTITION_NUM];//所有分区列表
}HDD_MANAGE_ST;

int FiHddGetMaxPartitionNum(void);
int FiHddGetHddPartitionPath(int partition,char *path);
int FiHddGetUsingPartition(char *hddPath);
int FiHddGetHddExistFlag(int type,int *flag);
int FiHddGetTotalMounted();
int FiHddGetPartitionFreeSize(int partition);

int FiHddStartHotplugService();
int FiHddStopHotplugService();

void HddStateSet( int state );
int HddStateGet();

int FiHddStopHotPlugForDelRecord();
int FiHddRestartHotPlugForDelRecord();

int FiHddGetHddManage(HDD_MANAGE_ST *pHddManageSt);

int FiHddGetHddPartitionFault();
void FiHddSetHddPartitionFault(int hddPartitionFault);
int FiHddHddStatErrReport(int errCode);

#endif //__HDD_H__

