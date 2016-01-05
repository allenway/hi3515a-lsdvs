/********************************************************************************
**	Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**	author        :  sven
**	version       :  v1.0
**	date           :  2013.10.10
**	description  : update
********************************************************************************/


#include <stdio.h>
#include "debug.h"
#include "md5.h"
#include "flash.h"
#include "writeUpdateFile.h"
#include "log.h"


/********************************************************************
* function:检查接收到的文件是否合法
* pUpdateHead: 文件头
* nUpgradeSize: 实际接收到文件的大小
* return: 1,合法; 否则,非法
********************************************************************/
static int IsValidUpgradeHead( UPGRADE_HEAD * pUpdateHead, int nUpgradeSize )
{    
	if ( pUpdateHead->upgradeNum < 1 || pUpdateHead->upgradeNum > 3 ) return 0;
    
	unsigned int headSize	= sizeof(UPGRADE_HEAD)
                            + (pUpdateHead->upgradeNum-1) * sizeof(FILE_INFO);
	if ( headSize != pUpdateHead->headSize ) return 0;    

    // 检查文件头 + 3个文件的大小是否等于实际接收到的数据大小
	unsigned int nFileSize = headSize;
	for ( unsigned int i = 0; i < pUpdateHead->upgradeNum; ++i )
    	nFileSize += pUpdateHead->upgradeInfo[i].size;
	if ( nFileSize != (unsigned int)nUpgradeSize ) return 0;
    
	return 1;
}

/******************************************************************
* function:检查要被升级的文件的md5 校验是否通过
* buf: 文件内容地址
* len: 文件长度
* return: 1,合法; 否则,非法
*******************************************************************/
static bool IsMd5CheckOK( char *buf, int len )
{
	bool bRet = false;
	unsigned char key[MD5_KEY_SIZE] = { 0 };
	MD5_CONTEXT mdc;
	char *pMd5Key;

	if ( buf == NULL || len < MD5_KEY_SIZE ) 
    {
    	return bRet;
    }
    
	Md5Init( &mdc );
	Md5Update( &mdc, (unsigned char *)buf, len - MD5_KEY_SIZE );
	Md5Final( key, &mdc );
    
	pMd5Key = buf + len - MD5_KEY_SIZE;
	if ( ! memcmp(pMd5Key, key, MD5_KEY_SIZE) ) 
    {
    	bRet = true;
    }
    
	return bRet;
}

static unsigned char UpgradeFile( char *upgradeFile, int upgradeSize )
{
	unsigned char errorNo = UPGRADE_OK;    

	if( IsMd5CheckOK( upgradeFile, upgradeSize ) )
    {
    	int	            	nRet	= 0;
    	char *            	file	= upgradeFile;
    	int	            	size	= upgradeSize - sizeof(UPGRADE_FILE_INFO);
    	UPGRADE_FILE_INFO *	info	= (UPGRADE_FILE_INFO *)( file + size );
        /*        
    	if ( strncmp((char *)info->id, UPDATE_FILE_ID, sizeof(info->id)) )
        {
        	errorNo = FILE_ID_ERROR;
        	SVPrint( "Upgrade file id error !\r\n" );
        	nRet = -1;
        }
        */        
    	if ( nRet != -1 )
        {
        	int ret = 0;

            // 升级之前退出获取视频线程
            // 由于升级会导致操作系统死机，原因未明!!!
            // 初步怀疑是由于获取原始视频的驱动导致操作系统挂死，故升级之前先退出。
            //extern int QuitVideoServiceThread();
            //QuitVideoServiceThread();
            
            // 如果写flash出错，则最多支持重写10次；
            // 如果升级文件跟flash里的文件完全一致则退出；
            // 如果写flash成功，则再写一次用于校验是否写成功。
        	for ( int cnt = 0; cnt < 10; ++cnt )
            {    
            	switch ( info->type )
                {
                	case UPGRADE_FILE_TYPE_ROOTFS:
                        LogAddAndWriteFlash(0xff, LOG_TYPE_SYSTEM, LOG_LEVEL_CRITICAL, "Start UpgradeRootfs, version:%d.", info->version);
                    	ret = UpgradeRootfs( file, size );
                    	if ( -1 == ret )
                        {
                        	errorNo = WRITE_FLASH_FAIL;
                        	SVPrint( "Upgrade root file system write flash failed %d !\r\n", cnt );
                        }
                    	break;
                	case UPGRADE_FILE_TYPE_UBOOT:
                        LogAddAndWriteFlash(0xff, LOG_TYPE_SYSTEM, LOG_LEVEL_CRITICAL, "Start UpgradeUBoot, version:%d.", info->version);
                    	ret = UpgradeUBoot( file, size );
                    	if ( ret == -1 )
                        {
                        	errorNo = WRITE_FLASH_FAIL;
                        	SVPrint( "Upgrade uboot write flash failed %d !\r\n", cnt );
                        }
                    	break;
                	case UPGRADE_FILE_TYPE_KERNEL:
                        LogAddAndWriteFlash(0xff, LOG_TYPE_SYSTEM, LOG_LEVEL_CRITICAL, "Start UpgradeKernel, version:%d.", info->version);
                    	ret = UpgradeKernel( file, size );
                    	if ( ret == -1 )
                        {
                        	errorNo = WRITE_FLASH_FAIL;
                        	SVPrint( "Upgrade kernel write flash failed %d !\r\n", cnt );
                        }
                    	break;
                	default:
                        LogAddAndWriteFlash(0xff, LOG_TYPE_SYSTEM, LOG_LEVEL_CRITICAL, "Upgrade file type error, type:%d, version:%d.", info->type, info->version);
                    	errorNo = FILE_TYPE_ERROR;
                    	SVPrint( "Upgrade file type error !\r\n" );
                    	ret = -1;
                    	break;
                }
                
            	if ( ret != 0 )
                {
                    nRet = ret;
                }
                
            	if ( ret >= 0 ) 
                {
                    break;  //myf 把 == 改为 >=. 升级后不用再读出一次比较一次，
                }
            }
            
        	if ( nRet > 0 )
            {
            	errorNo = UPGRADE_OK;
            	SVPrint( "Upgrade successful, Reboot system !\r\n" );
            }
        	else 
        	if ( nRet == 0 )
            {
            	errorNo = UPGRADE_SAME_FILE;
            	SVPrint( "Upgrade file as same as current flash file !\r\n" );
            }
        }
    }
	else
    {
    	errorNo = MD5_CHECK_ERROR;    
    	SVPrint( "Upgrade file md5 check error !\r\n" );
    }
    
	return errorNo;
}

/**********************************************************************************
* function: 升级接收到的文件
* upgradeFile: 通过网络接收到的文件,或者通过串口接收到的文件
                     或者在SD 卡里面的文件
* upgradeSize: 文件的大小
***********************************************************************************/
unsigned char UpgradeSystem( char *upgradeFile, int upgradeSize )
{
	unsigned char errorNo = UPGRADE_OTHER_ERROR;    
	UPGRADE_HEAD * pUpdateHead = ( UPGRADE_HEAD * )( upgradeFile );

    // 根据包头标识来判断是否为多文件升级包
	if ( ! memcmp(pUpdateHead->headFlag, UPDATE_HEAD_FLAG, sizeof(UPDATE_HEAD_FLAG)) )
    {
    	if ( IsValidUpgradeHead( pUpdateHead, upgradeSize ) )
        {
        	char *	pUpgradeFile = upgradeFile + pUpdateHead->headSize;
        	int		nUpgradeSize = 0;
            
        	for ( unsigned int i = 0; i < pUpdateHead->upgradeNum; ++i )
            {
            	nUpgradeSize = pUpdateHead->upgradeInfo[i].size;
            	int nRet = UpgradeFile( pUpgradeFile, nUpgradeSize );
                
            	if ( nRet == UPGRADE_OK )
                {
                	errorNo = nRet;
                }
            	else 
            	if ( nRet == UPGRADE_SAME_FILE )
                {
                	if ( errorNo != UPGRADE_OK )
                    {
                        errorNo = nRet;
                    }   
                }
            	else
                {
                	errorNo = nRet;
                	break;
                }
                
            	pUpgradeFile += nUpgradeSize;
            }
        }
    }
	else
    {
    	errorNo = UpgradeFile( upgradeFile, upgradeSize );
    }

	return errorNo;
}

