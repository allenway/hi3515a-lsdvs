/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**	文件名: norflash.h
**  description  : 封转norflsh MTD接口
**  date           :  2013.12.03
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#ifndef __NORFLASH_H__
#define __NORFLASH_H__

////////////////////////////////////////////////////////////
//HI3515A use spi flash block:64kB,size:16MB
//0x82000000-0x820fffff    MTD0:UBOOT,1M
//0x82100000-0x824fffff    MTD1:KENERL,4M
//0x82500000-0x82efffff    MTD2:FS,10M
//0x82f00000-0x82ffffff     MTD3:conf,1M
#define 	MTD_PARTITION_UBOOT	                "/dev/mtd0"    // uboot - 1M
#define 	MTD_PARTITION_KERNEL	            "/dev/mtd1"    // 内核 - 4M
#define 	MTD_PARTITION_ROOTFS	            "/dev/mtd2"    // 文件系统 - 10M
#define 	MTD_PARTITION_CONFIG	            "/dev/mtd3"    // 系统配置 - 1M
//no use follow
#define 	MTD_PARTITION_OTHER	                "/dev/mtd4"    // 其他 - 13M

///////////////////////////////////////////////////////////
/*
** 下面函数注意:
** 1.非线程安全;
** 2.开发板上flash一个扇区的大小为64KBytes,
**     读写擦除的起始地址要以扇区对齐,擦除的时候要整个扇区擦除
** 3.mtdname参数对应 MTD_PARTITION_*的宏
*/
int mtd_erase(const char *mtdname, unsigned int start, unsigned int size);
int mtd_write(const char *mtdname, unsigned int start, unsigned int size, char *buffer);
int mtd_read(const char *mtdname, unsigned int start, unsigned int size, char *buffer);

#endif //__NORFLASH_H__

