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
#ifndef __NORFLASH3512_H__
#define __NORFLASH3512_H__


#if defined MCU_HI3515
/*
注意: norflash 的映射地址从 0x0200000 开始
**	0x00000000-0x00080000 : MTD 0	512K
**	0x00080000-0x00280000 : MTD 1	2M
**	0x00280000-0x01280000 : MTD 2	16M
**	0x01280000-0x01300000 : MTD 3	512K
**	0x01300000-0x02000000 : MTD 4	13M
*/
#define 	MTD_PARTITION_UBOOT	                "/dev/mtd0"    // uboot - 512K
#define 	MTD_PARTITION_KERNEL	            "/dev/mtd1"    // 内核 - 1.5
#define 	MTD_PARTITION_ROOTFS	            "/dev/mtd2"    // 文件系统 - 16M
#endif
#define 	MTD_PARTITION_CONFIG	            "/dev/mtd3"    // 系统配置 - 512K
#define 	MTD_PARTITION_OTHER	                "/dev/mtd4"    // 其他 - 13M


#if defined MCU_HI3515A
////////////////////////////////////////////////////////////
//HI3515A use spi flash block:64kB,size:16MB
//0x82000000-0x820fffff    MTD0:UBOOT,1M
//0x82100000-0x824fffff    MTD1:KENERL,4M
//0x82500000-0x82fffffff    MTD2:FS,11M
#define 	MTD_PARTITION_UBOOT	                "/dev/mtd0"    // uboot - 1M
#define 	MTD_PARTITION_KERNEL	            "/dev/mtd1"    // 内核 - 4M
#define 	MTD_PARTITION_ROOTFS	            "/dev/mtd2"    // 文件系统 - 11M
#endif


///////////////////////////////////////////////////////////
/*
** 下面函数注意:
** 1.非线程安全;
** 2.开发板上norflash一个扇区的大小为128KBytes,
**     读写擦除的起始地址要以扇区对齐,擦除的时候要整个扇区擦除
** 3.mtdname参数对应 MTD_PARTITION_*的宏
*/
int mtd_erase(const char *mtdname, unsigned int start, unsigned int size);
int mtd_write(const char *mtdname, unsigned int start, unsigned int size, char *buffer);
int mtd_read(const char *mtdname, unsigned int start, unsigned int size, char *buffer);

#endif //__NORFLASH3512_H__

