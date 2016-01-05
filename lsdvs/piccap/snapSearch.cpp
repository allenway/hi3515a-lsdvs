/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 搜索抓拍到的 jpg 图片
********************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "const.h"
#include "debug.h"
#include "public.h"
#include "malloc.h"
#include "timeExchange.h"
#include "linuxFile.h"
#include "snapSearch.h"
#include "hdd.h"

/*****************************************************************
* 抓怕类型的字符型到unsigned long转换
* snapName:不带路径的抓怕文件名;
* snapType:out
******************************************************************/
static int snapTypeFromCharToInt( char *snapName, uint *snapType )
{
	if( NULL == snapName || NULL == snapType )
    {
    	SVPrint( "error:NULL == snapTypeName || NULL == snapType!\r\n" );
    	return FI_FAILED;
    }
    
	if( NULL != strstr(snapName,"hand") )
    {
        *snapType = SNAP_TYPE_HAND;
    }
	else if( NULL != strstr(snapName,"timer") )
    {
        *snapType = SNAP_TYPE_TIMER;
    }
	else if( NULL != strstr(snapName,"alarmio") )
    {
        *snapType = SNAP_TYPE_ALARM_IO;
    }
	else if( NULL != strstr(snapName,"alarmlo") )
    {
        *snapType = SNAP_TYPE_ALARM_LOST;
    }
	else if( NULL != strstr(snapName,"alarmmd") )
    {
        *snapType = SNAP_TYPE_ALARM_MD;
    }
	else if( NULL != strstr(snapName,"alarmpc") )
    {
        *snapType = SNAP_TYPE_ALARM_PIC_COMPARE;
    }
	else if( NULL != strstr(snapName,"alarmshelter") )
    {
        *snapType = SNAP_TYPE_ALARM_SHELTER;
    }
    
	return FI_SUCCESSFUL;
}

/*
* 分析抓怕文件名
* snapName:不带路径的抓怕文件名;
* snapType:out,抓怕类型;
* snapTime:out,抓怕时间
*/
static int analysisSnapName( char *snapName, uint *snapType, int *hh, int *mm, int *ss )
{
	int  ret;
    
	if( NULL == snapName || NULL == snapType || NULL == hh || NULL == mm || NULL ==ss )
    {
    	SVPrint( "error:NULL == snapName || NULL == snapType || NULL == hh || NULL == mm || NULL ==ss!\r\n" );
    	return FI_FAILED;
    }

	ret = sscanf( snapName, "%02dh%02dm%02ds", hh, mm, ss );
	if( ret == 3 )
    {        
    	snapTypeFromCharToInt( snapName, snapType );
    	ret = 0;
    }
	else
    {
    	ret = -1;
    }
    
	return ret;    
}

/*
* 仲裁jpeg文件
* snapName:文件名, 不包括路径; 
* snapType: in, 查找的抓拍类型; out, 抓拍文件的类型; 
* snapTime:查找开始时间; 
* stopTime:查找停止时间;
* hh, mm, ss:out,抓怕时间;
* 返回:FI_TRUE-成功,FI_FALSE-失败
*/
static int IsMatchSnapFile( char *snapName, uint *snapType,
                        	int startTime, int stopTime, 
                        	int *hh, int *mm, int *ss )
{
	uint snapJpgType = 0;
	int	 snapJpgTime = 0;
	int  ret;
    
	if( NULL == snapName )
    {
    	SVPrint( "error:NULL == snapName || NULL == snapTime!\r\n" );
    	return FI_FALSE;
    }
    
	ret = analysisSnapName( snapName, &snapJpgType, hh, mm, ss );
	if( 0 == ret )
    {
    	if( !(snapJpgType & *snapType) )
        {
        	FiPrint2( "failed: snapJpgType(%X) - *snapType(%X)!\r\n", snapJpgType, *snapType );
        	return FI_FALSE;
        }
    	snapJpgTime = 3600 * (*hh) + 60 * (*mm) + (*ss);
    	if( snapJpgTime >= (startTime % 86400) && snapJpgTime <= (stopTime % 86400) )
        {
            *snapType = snapJpgType;
        	return FI_TRUE;
        }    
    }
	if( 0 != ret )
    {        
    	return FI_FALSE;
    }
	else
    {
    	return FI_FALSE;    
    }
}

static uint GetSnapFileSize(char *fileName)
{
	int     	ret = 0;  
	struct stat buf = {0};
    
	if(fileName)
    {
    	stat( fileName, &buf );
    	ret = buf.st_size;
    }
    
	return (uint)ret;
}

/*
* 把索引文件索取到的抓怕文件插入抓怕查询链表中
* snapName: 索引文件名;
* snapTime: 该图片的抓怕时间;
* inquireList:out,查询到的抓怕索引列表
*/
static int InsertSnapFileToInquireList( char *snapName, int snapTime,
                                    	SNAP_INQUIRE_LIST *inquireList,uint snapType )
{
	SNAP_INQUIRE *inquireNode = NULL;

	if(NULL == snapName || NULL == inquireList)
    {
    	SVPrint("error:NULL == snapName || NULL == inquireList!\r\n");
    	return FI_FAILED;
    }
    
	if(NULL == (inquireNode = (SNAP_INQUIRE *)calloc(1,sizeof(SNAP_INQUIRE))))
    {
    	SVPrint("malloc() error:%s!\r\n",STRERROR_ERRNO);
    	return FI_FAILED;
    }    
	inquireNode->next = NULL;
	inquireNode->prev = NULL;
    
	snprintf( inquireNode->snapName, MAX_SNAP_JPG_PATH_LEN, "%s", snapName);
	inquireNode->snapTime	= snapTime; 
	inquireNode->snapLen	= GetSnapFileSize( inquireNode->snapName );
	inquireNode->snapType	= snapType;
    //start insert
	if(NULL == inquireList->head)
    {
    	inquireList->head     = inquireNode;
    	inquireList->cur     = inquireNode;
    	inquireList->tail     = inquireNode;
    	return FI_SUCCESSFUL;
    }
	inquireList->cur = inquireList->head;
	while(1)
    {
    	if( NULL == inquireList->cur )
        {
        	inquireList->tail->next	= inquireNode;
        	inquireNode->prev         = inquireList->tail;
        	inquireList->tail         = inquireNode;
        	break;
        }
    	if( inquireNode->snapTime < inquireList->cur->snapTime )
        {
        	if( inquireList->cur == inquireList->head )
            {
            	inquireNode->next         = inquireList->head;
            	inquireList->head->prev = inquireNode;
            	inquireList->head         = inquireNode;
            }
        	else
            {
            	inquireNode->next = inquireList->cur;
            	inquireNode->prev = inquireList->cur->prev;
            	inquireNode->prev->next = inquireNode;
            	inquireNode->next->prev = inquireNode;
            }
        	break;
        }
    	inquireList->cur = inquireList->cur->next;
    }
	return FI_SUCCESSFUL;
}

/*
* 找出某个路径下所有符合参数要求的抓怕
* path:文件的绝对路径;snapType:抓怕类型
* snapTime:要查找的开始时间;stopTime:要查找的结束时间
* inquireList:out,查询到的抓怕索引列表
*/
static int FindSnapFileInPath( char *path, uint snapType, int startTime, int stopTime, 
                            	int YY, int MM, int DD, SNAP_INQUIRE_LIST *inquireList )
{
	DIR     *handleDir;
	struct	dirent	*pdir;
	int		snapTime;    
	uint	localSnapType = snapType;
	char	fullSnapName[MAX_SNAP_JPG_PATH_LEN];
	int 	hh, mm, ss;
	int 	ret;
    
	if( NULL == path )
    {
    	SVPrint("error:NULL == path!\r\n");
    	return FI_FAILED;
    }
	if( NULL == (handleDir=opendir(path)) ) 
    {
    	SVPrint( "opendir(%s) error:%s\r\n", path, STRERROR_ERRNO );
    	return FI_FAILED;
    }
	while( NULL != (pdir=readdir(handleDir)) )
    {
    	if(0 == strcmp(pdir->d_name,".") || 0 == strcmp(pdir->d_name,".."))
        {
        	continue;
        }    
    	if( NULL == strstr(pdir->d_name,".jpg") )
        {
        	continue;
        }

    	ret = IsMatchSnapFile( pdir->d_name, &localSnapType, startTime, 
                            	stopTime, &hh, &mm, &ss );
    	if( FI_FALSE == ret )
        {
        	continue;
        }
    	snapTime = FiTimeHumanToUtc( YY, MM, DD, hh, mm, ss );
    	snprintf( fullSnapName, sizeof(fullSnapName)-1, "%s/%s", path, pdir->d_name );
        
    	InsertSnapFileToInquireList( fullSnapName, snapTime, inquireList, localSnapType );
    }
	return FI_SUCCESS;
}

/*
* 从给定的参数生成查询路径
* partition:分区;year,month,day:年-月-日;channel:通道
* inquirePath:out,查询路径
* 返回:FI_FAILED-失败,FI_SUCCESSFUL-成功
*/
static int GetSnapInquirePath( char *partition, int year, int month, 
                        	int day, int channel, char *inquirePath )
{
	sprintf( inquirePath, "%s/jpg/%04d-%02d-%02d/ch%02d",
                    	partition, year, month, day, channel );

	return FI_SUCCESSFUL;
}

/*
* fn: 抓怕查询
* channel: 通道
* snapType:	抓怕类型
* startTime: 开始时间
* stopTime: 结束时间
* 返回:符合参数要求的抓怕文件列表
* 注意:调用完该函数后要释放一个指针(返回的指针)和一个链表(返回指针的head变量所指向的链表)
	使用 FiSnapFreeInquireSnap() 释放
*/
SNAP_INQUIRE_LIST *FiSnapInquireSnapFile( int channel, uint snapType, 
                                    	int startTime, int stopTime )
{
	char inquirePartition[32] = {0};
	char inquirePath[MAX_SNAP_JPG_PATH_LEN] = {0};     
	int partitionNum;
	int i,j;
	int YY, MM, DD;
	int startSecond,stopSecond;
	SNAP_INQUIRE_LIST *inquireList;    
    
	if(	startTime > stopTime 
        || channel < 0 || channel >= REAL_CHANNEL_NUM
        || (snapType & SNAP_TYPE_ALL) == 0 )
    {
    	SVPrint( "snap find failed:please check your find condition!\r\n" );
    	return NULL;
    }

	if(NULL == (inquireList = (SNAP_INQUIRE_LIST *)Malloc(sizeof(SNAP_INQUIRE_LIST))))
    {
    	SVPrint( "malloc error:%s!\r\n", STRERROR_ERRNO );
    	return NULL;
    }    
	inquireList->head     = NULL;
	inquireList->cur     = NULL;
	inquireList->tail     = NULL;

	partitionNum = FiHddGetMaxPartitionNum();
	for( j = 0; j < partitionNum; j++ )
    {
    	if( FI_FAILED == FiHddGetHddPartitionPath( j, inquirePartition) )
        {
        	continue;
        }
    	startSecond = startTime;
    	stopSecond     = stopTime + ONE_DAY_SECOND - stopTime%ONE_DAY_SECOND;
        
    	for( i = startSecond; i < stopSecond; i += ONE_DAY_SECOND ) 
        {
        	FiTimeUtcToHuman( i, &YY, &MM, &DD, NULL, NULL, NULL );                 
        	if(FI_FAILED == GetSnapInquirePath( inquirePartition, YY, MM, DD, channel, inquirePath) )
            {
            	continue;
            }        
        	FindSnapFileInPath( inquirePath, snapType, startTime, stopTime, YY, MM, DD, inquireList );            
        }
    }
    
	return inquireList;    
}

/**********************************************************
* fn: 释放FiSnapInquireSnapFile() 的返回结果
************************************************************/
void FiSnapFreeInquireSnap( SNAP_INQUIRE_LIST *snapInquireResult )
{
	SNAP_INQUIRE *del;
	if( NULL == snapInquireResult )
    {
    	return;
    }
	snapInquireResult->cur = snapInquireResult->head;
	while( NULL != snapInquireResult->cur )
    {
    	del = snapInquireResult->cur;
    	snapInquireResult->cur = snapInquireResult->cur->next;
    	Free( del );
    }
    
	Free(snapInquireResult);
    
	return; 
}

