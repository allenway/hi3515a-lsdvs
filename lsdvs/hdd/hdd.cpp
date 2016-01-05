/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.
**	文件名: hdd.c
**  description  : 管理磁盘分区
**  date           :  2013.10.18
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <dirent.h>
#include <signal.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>
#include <string.h>
#include <pthread.h> 
#include <sys/vfs.h> 

#include "const.h"
#include "thread.h"
#include "debug.h"
#include "malloc.h"
#include "message.h"
#include "hdd.h"
#include "linuxFile.h"
#include "hddrw.h"
#include "log.h"
#include "message.h"
#include "dcpTypes.h"
#include "dcpInsLocal.h"

static pthread_t	g_hddHotplugId;
static int g_hddHotplugThreadFlag = FI_FALSE;
static HDD_MANAGE_ST	g_hddManage = {0}; 
static HOTPLUG_MAMAGE 	g_hotplug = {0,0,0,0};
static int g_usbNode = 0;

static int InitMount(void);
static void CalculateHddSize();
static CMutexLockRecursive g_mutexHdd;
static int g_hddState;

void HddStateSet( int state )
{
	g_mutexHdd.Lock();
	g_hddState = state;    
	g_mutexHdd.Unlock();    
}

int HddStateGet()
{
	int ret;
	g_mutexHdd.Lock();
	ret = g_hddState;    
	g_mutexHdd.Unlock();    

	return ret;
}

/******************************************************
* 初始化热插拔socket
*******************************************************/
static int InitHotlugSock(void)
{
    struct sockaddr_nl snl;
    const int bufferSize = 16 * 1024 * 1024;
    int retval;

    Memset(&snl, 0x00, sizeof(struct sockaddr_nl));
    snl.nl_family = AF_NETLINK;
    snl.nl_pid = getpid();
    snl.nl_groups = 1;

    int hotplugSock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (hotplugSock == -1) 
    {
        SVPrint("error getting socket: %s\r\n", STRERROR_ERRNO);
        return -1;
    }

    /* set receive bufferSize */
    setsockopt(hotplugSock, SOL_SOCKET, SO_RCVBUF, &bufferSize, sizeof(bufferSize));
    retval = bind(hotplugSock, (struct sockaddr *) &snl, sizeof(struct sockaddr_nl));
    if (retval < 0) 
    {
        SVPrint("bind failed: %s\r\n", STRERROR_ERRNO);
        close(hotplugSock);
        hotplugSock = -1;
        return -1;
    }
    return hotplugSock;
}

static int GetPartitionInfo(char *dev,long *totalSize,long *freeSize)
{
	struct statfs buf;
	long long size;    

	if(NULL==dev || NULL==totalSize || NULL==freeSize)
    {
    	SVPrint("NULL==dev || NULL==totalSize || NULL==freeSize!\r\n");
    	return FI_FAILED;
    }
    
	if( HddrwStatfs(dev, &buf) < 0 )
    {
    	SVPrint( "HddrwStatfs(%s) error:%s!\r\n", dev, STRERROR_ERRNO );
    	return FI_FAILED;
    }
	size = ((long long)buf.f_bsize) * ((long long)buf.f_blocks);
	size = size/1024/1024; // to MBytes
    *totalSize = (long)size;

	size = ((long long)buf.f_bsize) * ((long long)buf.f_bfree);
	size = size/1024/1024; // to MBytes
    *freeSize = (long)size;    
    
	return FI_SUCCESSFUL;
}

static int GetAvailablePartition( HDD_PARTITION_INFO_ST *part )
{
	int i;

	g_mutexHdd.Lock();
	for( i=0; i<MAX_HDD_PARTITION_NUM; i++ )
    {
    	if( HDD_MOUNTED == g_hddManage.partHead[i].mountFlag
            && g_hddManage.partHead[i].freeSize > MIN_PARTITION_SIZE )
        {
        	if(part) 
            {
            	Memcpy(part,&g_hddManage.partHead[i], sizeof(g_hddManage.partHead[i]));
            }    
        	break;
        }
    }
    
	g_mutexHdd.Unlock();
	return FI_SUCCESSFUL;
}

// 磁盘错误
static int g_hddPartitionFault;
int FiHddGetHddPartitionFault()
{
    int hddPartitionFault;
	g_mutexHdd.Lock();
    hddPartitionFault = g_hddPartitionFault;
	g_mutexHdd.Unlock();
    return hddPartitionFault;
}
void FiHddSetHddPartitionFault(int hddPartitionFault)
{
	g_mutexHdd.Lock();
    g_hddPartitionFault = hddPartitionFault;
	g_mutexHdd.Unlock();
}


static void ManageUsingPartition()
{
	g_mutexHdd.Lock();
    
	HDD_PARTITION_INFO_ST *cur = &g_hddManage.usingPart;
	if(HDD_MOUNTED != cur->mountFlag)
    {
    	CalculateHddSize();
    	GetAvailablePartition(cur);
    }
	else
    {
    	GetPartitionInfo(cur->mountPath,&cur->totalSize,&cur->freeSize);
    	if(cur->freeSize < MIN_PARTITION_SIZE)
        {
        	Memset(cur,0,sizeof(*cur));
        	CalculateHddSize();
        	GetAvailablePartition(cur);
        }
    }    
    
	g_mutexHdd.Unlock();
}

static void HddManageMount(void)
{
	int i;
	HDD_PARTITION_INFO_ST *cur;

	g_mutexHdd.Lock();
    
	for(i=0;i<MAX_HDD_PARTITION_NUM;i++)
    {
    	if(HDD_DETECT == g_hddManage.partHead[i].mountFlag)
        {
            //CORRECTPRINT("DETECT partition %d",i);
        	cur = &g_hddManage.partHead[i];
        	HddrwMkdir( cur->mountPath, 0777 );
        	HddrwUmount( cur->mountPath );
        	if( 0 == HddrwMount(cur->devPath,cur->mountPath,FAT32,0,"shortname=mixed") )        
            {                
            	g_hddManage.totalMounted++;
            	cur->mountFlag = HDD_MOUNTED;
            	GetPartitionInfo( cur->mountPath, &cur->totalSize, &cur->freeSize );
            	ManageUsingPartition();
            }
        	else
            {
            	cur->mountFlag = HDD_MOUNT_FAILED;
            	SVPrint( "mount(%s,%s,type(%s)) error::%d:%s\r\n",
                        	cur->devPath,cur->mountPath,FAT32,errno,STRERROR_ERRNO );

            	LogAdd( 0xff, LOG_TYPE_STORAGE, LOG_LEVEL_CRITICAL,
                            "mount(%s,%s,type(%s)) error:%d:%s\r\n",
                            cur->devPath,cur->mountPath,FAT32,errno,STRERROR_ERRNO );
            }
        }        
    }    

	g_mutexHdd.Unlock();
}

static int HddManageUmount( int type )
{
	int i;
	HDD_PARTITION_INFO_ST *cur;

	g_mutexHdd.Lock();
    
	for(i=0;i<MAX_HDD_PARTITION_NUM;i++)
    {
    	cur = &g_hddManage.partHead[i];
    	if(HDD_MOUNTED == cur->mountFlag && type == cur->type)
        {
        	if(0 == HddrwUmount(cur->mountPath))
            {
            	Memset(cur,0,sizeof(*cur));
            	cur->mountFlag = HDD_NONE;
            }
        	else
            {
            	cur->mountFlag = HDD_UMOUNT_FAILED;
            }
        	g_hddManage.totalMounted--;
        }        
    }
	cur = &g_hddManage.usingPart;
	if(HDD_MOUNTED == cur->mountFlag && type == cur->type)
    {        
    	cur->mountFlag = HDD_NONE;        
    }        

	g_mutexHdd.Unlock();
	return FI_SUCCESSFUL;
}

/*
* 插入一个分区信息到分区信息列表
* head:out,返回的链表
* insertPart:要插入的节点
*/
static int InsertPartitionNodeToList(HDD_PARTITION_INFO_ST *insertPart)
{
	int i;
	HDD_PARTITION_INFO_ST *cur;

	g_mutexHdd.Lock();
	for(i=0;i<MAX_HDD_PARTITION_NUM;i++)
    {
    	cur = &g_hddManage.partHead[i];
    	if(HDD_MOUNTED != cur->mountFlag && HDD_DETECT != cur->mountFlag && insertPart)
        {
        	Memcpy(cur,insertPart,sizeof(*cur));
        	break;
        }
    }    

	g_mutexHdd.Unlock();
	return FI_SUCCESSFUL;
}

/*
* 从分区信息列表中删除分区的信息
*/
static int RemoveNodeFromHddList(int type)
{    
	return FI_SUCCESSFUL;    
}

/*
* 从设备路径生成mount路径
* type:见 enum _HddDevType
* devPath:设备路径
* mountPath:out,mount路径
*/
static int DevPathToMountPath(int type,char *devPath,char *mountPath)
{
	char *ptr = devPath;
	int num;
	int ret = FI_FAILED;
	char path[MAX_HDD_PATH_NAMESIZE] = {0};
	char partMark[MAX_HDD_PATH_NAMESIZE] = {0};
    
	if(NULL==devPath || NULL==mountPath)
    {
    	SVPrint("error:NULL==devPath || NULL==mountPath!\r\n");
    	return FI_FAILED;
    }
	if(HDD_TYPE_MMC == type)
    {
    	strcpy(path,MMC_MOUNT_PATH);
    	strcpy(partMark,MMC_DEV_PARTITION_MARK);
    }    
	else if(HDD_TYPE_USB == type)
    {
    	strcpy(path,USB_MOUNT_PATH);
    	strcpy(partMark,USB_DEV_PARTITION_MARK);
    }
        
	if(NULL != (ptr=Strstr(ptr,partMark)))
    {
    	ptr += Strlen(partMark);
    	num = atoi(ptr);
    	sprintf(mountPath,"%s%d",path,num);
    	ret = FI_SUCCESSFUL;
    }
    
	return ret;    
}

/*
* 把MMC设备路径加入链表
*/
static int AddMmcNodeToHddList(void)
{
	DIR     *dp;
	struct  dirent  *dirp;
	HDD_PARTITION_INFO_ST *curPart;
	char devPath[MAX_HDD_PATH_NAMESIZE] = {0};
	char mountPath[MAX_HDD_PATH_NAMESIZE] = {0};    
    
	if(NULL == (dp=HddrwOpendir(MMC_DEV_PATH)))
    {
    	SVPrint("HddrwOpendir(%s) failed:%s\r\n",MMC_DEV_PATH,STRERROR_ERRNO);
    	return FI_FAILED;
    }
	while( NULL != (dirp=HddrwReaddir(dp)) )
    {
    	if( (0==Strcmp(dirp->d_name,".")) || (0==Strcmp(dirp->d_name,"..")) )
        {
        	continue;
        }
        if( NULL == Strstr(dirp->d_name, MMC_DEV_PARTITION_MARK) )
        {
        	continue;
        }
        if( Strlen(dirp->d_name) == Strlen(MMC_DEV_PARTITION_MARK) )
        {
        	continue;
        }
            
        snprintf( devPath, MAX_HDD_PATH_NAMESIZE-1, "%s/%s", MMC_DEV_PATH, dirp->d_name );
        DevPathToMountPath( HDD_TYPE_MMC, devPath, mountPath );
        
        if(NULL != (curPart = (HDD_PARTITION_INFO_ST *)Calloc(1,sizeof(HDD_PARTITION_INFO_ST))))
        {
        	strcpy(curPart->devPath,devPath);
        	strcpy(curPart->mountPath,mountPath);
        	curPart->type = HDD_TYPE_MMC;
        	curPart->mountFlag = HDD_DETECT;
            
        	InsertPartitionNodeToList(curPart);            
        	Free(curPart);
                
        }   
    }
	HddrwClosedir(dp);

	return FI_SUCCESSFUL;
}

/*
* 把USB设备路径加入链表
* driverNode:-1 表示刚上电的时候检测;其它 指定 host(driverNode)检测
*/
static int AddUsbNodeToHddList(int driverNode)
{
	int fd, i;
	HDD_PARTITION_INFO_ST *curPart;
	char devPath[MAX_HDD_PATH_NAMESIZE] = {0};
	char mountPath[MAX_HDD_PATH_NAMESIZE] = {0};    
    
	if(-1 == driverNode)    
    {    
    	for( i = 0; i < 20; ++i )
        {                
            snprintf( devPath, MAX_HDD_PATH_NAMESIZE-1, "%s%d", "/dev/sda", i );
        	if( -1 != (fd = Open(devPath, O_RDONLY)) )
            {
                Close( fd );
             	snprintf( mountPath, MAX_HDD_PATH_NAMESIZE-1,"%s%d", "/hdd/sda", i );
                if(NULL != (curPart = (HDD_PARTITION_INFO_ST *)Calloc(1,sizeof(HDD_PARTITION_INFO_ST))))
                {
                	strcpy(curPart->devPath,devPath);
                	strcpy(curPart->mountPath,mountPath);
                	curPart->mountFlag     = HDD_DETECT;
                	curPart->type         = HDD_TYPE_USB;

                	InsertPartitionNodeToList( curPart );            
                	Free(curPart);                     
                }  
            } // if( -1 != (fd = Open(devPath, O_RDONLY
        } //for( i = 0; i < 20; ++i
    }
	return FI_SUCCESSFUL;
}

static int AddNodeToHddList(int type,int driverNode)
{
	int ret = FI_FAILED;
    
	if(HDD_TYPE_MMC == type)
    	ret = AddMmcNodeToHddList();
	else if(HDD_TYPE_USB == type)
    	ret = AddUsbNodeToHddList(driverNode);

	return ret;
}

static void HandleHotplug(void)
{    
	if(FI_TRUE == g_hotplug.mmcInsertFlag)
    {
        LogAdd(0xff, LOG_TYPE_STORAGE, LOG_LEVEL_ERROR, "SD CARD Inserted!");
    	AddNodeToHddList(HDD_TYPE_MMC,0);
    	HddManageMount();
    	g_hotplug.mmcInsertFlag = FI_FALSE;
    }
	if(FI_TRUE == g_hotplug.mmcRemoveFlag)
    {
        LogAdd(0xff, LOG_TYPE_STORAGE, LOG_LEVEL_ERROR, "SD CARD Removed!");
        FiHddHddStatErrReport(DCP_HDD_STAT_ERR_REMOVED);
    	HddManageUmount(HDD_TYPE_MMC);
    	RemoveNodeFromHddList(HDD_TYPE_MMC);                    
    	g_hotplug.mmcRemoveFlag = FI_FALSE;
    }
	if(FI_TRUE == g_hotplug.usbInsertFlag)
    {
        LogAdd(0xff, LOG_TYPE_STORAGE, LOG_LEVEL_ERROR, "USB Storage Inserted.");
    	AddNodeToHddList(HDD_TYPE_USB,g_usbNode);
    	HddManageMount();
    	g_hotplug.usbInsertFlag = FI_FALSE;
    }
	if(FI_TRUE == g_hotplug.usbRemoveFlag)
    {
        LogAdd(0xff, LOG_TYPE_STORAGE, LOG_LEVEL_ERROR, "USB Storage Removed.");
    	HddManageUmount(HDD_TYPE_USB);
    	RemoveNodeFromHddList(HDD_TYPE_USB);                    
    	g_hotplug.usbRemoveFlag = FI_FALSE;
    }
}

static void CalculateHddSize()
{
	int i;
	HDD_PARTITION_INFO_ST *cur;

	g_mutexHdd.Lock();
    
	for(i=0;i<MAX_HDD_PARTITION_NUM;i++)
    {
    	cur = &g_hddManage.partHead[i];
    	if(HDD_MOUNTED == cur->mountFlag)
        	GetPartitionInfo(cur->mountPath,&cur->totalSize,&cur->freeSize);        
    }        
	g_mutexHdd.Unlock();
}

// 处理磁盘错误
int HandlePartitionFault()
{
	g_mutexHdd.Lock();

	HDD_PARTITION_INFO_ST *cur = &g_hddManage.usingPart;
    HddManageUmount(cur->type);
	AddNodeToHddList(cur->type, -1);
    Memset(cur, 0, sizeof(*cur));
	HddManageMount();

	g_mutexHdd.Unlock();

	return FI_SUCCESSFUL;
}

/*
* 热插拔管理
*/
static void *HddHotplugThread(void *argc)
{
	int ret;
	char *ptr;
	int initFlag = FI_FALSE;
	fd_set	readFds;    
	struct timeval tv;
	char buf[UEVENT_BUFFER_SIZE] = {0};        
    
	int hotplugSock = InitHotlugSock();

    FiHddSetHddPartitionFault(0);
	HddStateSet( HDD_STATE_NORMAL );
    
	SVPrint( "start HddHotplugThread:%d!\r\n", ThreadSelf() );
    
	while( g_hddHotplugThreadFlag )
    {    
    	switch( HddStateGet() )
        {
        	case HDD_STATE_START:            
            	CalculateHddSize();
            	HddStateSet( HDD_STATE_NORMAL );
            	Usleep( 10 * 1000 );
            	break;
        	case HDD_STATE_NORMAL:
            	if ( MessageRecv( MSG_ID_CALCULATE_HDD_SIZE ) >= 0 )
                {
                	SVPrint("hdd get msg:MSG_ID_CALCULATE_HDD_SIZE\r\n");
                    //CalculateHddSize();
                }
                if( -1 == FiHddGetHddPartitionFault() )
                {
                    HandlePartitionFault();
                    LogAdd(0xff, LOG_TYPE_STORAGE, LOG_LEVEL_CRITICAL, "HandlePartitionFault.");
                    ERRORPRINT("HandlePartitionFault.\n");
                    FiHddHddStatErrReport(DCP_HDD_STAT_ERR_OTHER);
                    FiHddSetHddPartitionFault(0);
                }

            	CalculateHddSize();
            	if( FI_FALSE == initFlag )
                {
                	tv.tv_sec = 0;
                }
            	else
                {
                	tv.tv_sec = 2;
                }
            	tv.tv_usec = 0;

            	FD_ZERO(&readFds);
            	FD_SET(hotplugSock,&readFds);
            	ret = select(hotplugSock+1, &readFds, NULL, NULL, &tv);
            	if(ret == 0)
                {
                	if(initFlag == FI_FALSE)
                    {
                    	InitMount();//get partition information etc 。。。
                    	initFlag = FI_TRUE;
                        if(g_hddManage.totalMounted < 1)
                        {
                            ERRORPRINT("no storage!\n");
                            LogAdd(0xff, LOG_TYPE_STORAGE, LOG_LEVEL_CRITICAL, "InitMount no Storage.");
                        }
                    }
                	ManageUsingPartition();
                	HandleHotplug();    

                	sleep(2);
                }
            	else if(ret > 0)
                {        
                	recv(hotplugSock, &buf, sizeof(buf), 0);
                	SVPrint("msg from hotplug driver:%s\n", buf); 
                	if(NULL != strstr(buf,MMC_INSERT_MARK))
                    {                
                    	g_hotplug.mmcInsertFlag = FI_TRUE;                
                    }
                	else if(NULL !=strstr(buf,MMC_REMOVE_MARK))
                    {                                
                    	g_hotplug.mmcRemoveFlag = FI_TRUE;                
                    }
                	else if(NULL != (ptr=strstr(buf,USB_INSERT_MARK_SECOND)) 
                         && NULL != strstr(buf,USB_INSERT_MARK_FIRST))
                    {
                    	g_usbNode = atoi(ptr+strlen(USB_INSERT_MARK_SECOND));
                    	g_hotplug.usbInsertFlag = FI_TRUE;
                    }
                	else if(NULL != strstr(buf,USB_REMOVE_MARK_SECOND) 
                         && NULL != strstr(buf,USB_REMOVE_MARK_FIRST))
                    {
                    	g_hotplug.usbRemoveFlag = FI_TRUE;
                    }
                }
            	else
                {
                	SVPrint("HddHotplugThread select error:%s\r\n",STRERROR_ERRNO);
                	break;
                }
            	break; // case HDD_STATE_START:
        	case HDD_STATE_STOP:
            	HddStateSet( HDD_STATE_SLEEP );
            	break;
        	case HDD_STATE_SLEEP:
        	default:
            	Ssleep( 1 );
            	break;
        } // switch( g_hddState )        
    }

	SVPrint("quit HddHotplugThread!\r\n");
	return NULL;
}

int InitMount(void)
{    
	DIR     *dp;
	struct  dirent  *dirp;    
	char hddDir[MAX_HDD_PATH_NAMESIZE] = {0};    
    
	if(NULL == (dp=HddrwOpendir(HDD_ROOT_DIR)))
    {
    	SVPrint("HddrwOpendir(%s) error:%s\r\n",HDD_ROOT_DIR,STRERROR_ERRNO);
    	return FI_FAILED;
    }
	while(NULL != (dirp=HddrwReaddir(dp)))
    {
    	if((0==Strcmp(dirp->d_name,".")) || (0==Strcmp(dirp->d_name,"..")))
        	continue;

        sprintf(hddDir,"%s/%s",HDD_ROOT_DIR,dirp->d_name);
        if(0 != HddrwUmount(hddDir))
        {
        	FiPrint2("%s had not been mount!\r\n",hddDir);
        }
    }
	HddrwClosedir(dp);
	Usleep(10);

	AddNodeToHddList(HDD_TYPE_MMC, 0);
	AddNodeToHddList(HDD_TYPE_USB, -1);
	HddManageMount();
    
	return FI_SUCCESSFUL;
}

/*
* 启动热插拔服务
*/
int FiHddStartHotplugService()
{
	int ret;

	g_hddHotplugThreadFlag = FI_TRUE;

	Memset(&g_hddManage,0,sizeof(g_hddManage));
    
	ret = ThreadCreateCommonPriority(&g_hddHotplugId,
                        	HddHotplugThread,
                        	NULL );
	if (ret)
    {
    	g_hddHotplugThreadFlag = FI_FALSE;
    	SVPrint( "ThreadCreateCommonPriority() error:%s\r\n", STRERROR_ERRNO );
    	return FI_FAILED;
    }

	//Memset(&g_hddManage,0,sizeof(g_hddManage));//remove at 5.6
    CORRECTPRINT("start hot plug service!!!!\n");
	return FI_SUCCESSFUL;
}

/***************************************************
* 停止热插拔服务
*****************************************************/
int FiHddStopHotplugService()
{
	g_hddHotplugThreadFlag = FI_FALSE;
	if( 0 != ThreadJoin(g_hddHotplugId,NULL) )
    {
    	SVPrint("error:ThreadJoin(g_hddHotplugThreadFlag,NULL)\r\n");
    	return FI_FAILED;
    }    
	return FI_SUCCESSFUL;
}

/*
* 获取最大支持的硬盘分区数目
*/
int FiHddGetMaxPartitionNum(void)
{    
	return MAX_HDD_PARTITION_NUM;
}

/*
* 获取指定分区的路径
*/
int FiHddGetHddPartitionPath(int partition,char *path)
{    
	int ret;
	HDD_PARTITION_INFO_ST *cur;    
    
	if(NULL == path || partition > MAX_HDD_PARTITION_NUM)
    {
    	SVPrint("failed:NULL==path || partition(%d)>MAX_HDD_PARTITION_NUM!\r\n",partition);
    	return FI_FAILED;
    }

	g_mutexHdd.Lock();
    
	cur = &g_hddManage.partHead[partition];    
	if(HDD_MOUNTED == cur->mountFlag)
    {
    	strcpy(path,cur->mountPath);
    	ret = FI_SUCCESSFUL;
    }
	else
    {
    	ret = FI_FAILED;
    }

	g_mutexHdd.Unlock();
    
	return ret;
}

/*
* 获取指定分区的路径
* 返回:该分区剩余空间大小
*/
int FiHddGetPartitionFreeSize(int partition)
{    
	int ret = 0;
	HDD_PARTITION_INFO_ST *cur;    

	g_mutexHdd.Lock();
    
	if(partition >= 0 && partition < MAX_HDD_PARTITION_NUM)
    {        
    	cur = &g_hddManage.partHead[partition];    
    	if(HDD_MOUNTED == cur->mountFlag)
        {        
        	ret = cur->freeSize;
        	if(ret <= MIN_PARTITION_SIZE_OTHER)
            	ret = 0;
        }        
    }
	g_mutexHdd.Unlock();
    
	return ret;
}

/*
* 获取当前可用的分区
* 返回:FI_SUCCESSFUL-有可用分区,路径存放在hdd_path;FI_FAILED-无当前可用分区
*/
int FiHddGetUsingPartition(char *hddPath)
{    
	HDD_PARTITION_INFO_ST usingPartition;
	int ret = FI_FAILED;
    
	if(NULL == hddPath)
    {
    	SVPrint("error:NULL == path!\r\n");
    	return FI_FAILED;
    }

	g_mutexHdd.Lock();
    
	Memcpy(&usingPartition,&g_hddManage.usingPart,sizeof(usingPartition));
	if(HDD_MOUNTED == usingPartition.mountFlag)
    {
    	sprintf(hddPath,"%s",usingPartition.mountPath);
    	ret = FI_SUCCESSFUL;
    }
	g_mutexHdd.Unlock();
    
	return ret;
}

/*
* 检查存储设备是否存在
* type:HDD_TYPE_MMC or HDD_TYPE_USB
* Flag:out,1-存在;0-不存在
*/
int FiHddGetHddExistFlag(int type,int *flag)
{
	int i;
	int existFlag = FI_FALSE;
	HDD_PARTITION_INFO_ST *cur;

	g_mutexHdd.Lock();
    
	if(type>=HDD_TYPE_MMC && type<=HDD_TYPE_USB)
    {    
    	for(i=0;i<MAX_HDD_PARTITION_NUM;i++)
        {
        	cur = &g_hddManage.partHead[i];
        	if(HDD_MOUNTED == cur->mountFlag && type == cur->type)
            	existFlag = FI_TRUE;        
        }    
    }

	if(NULL != flag) *flag = existFlag;

	g_mutexHdd.Unlock();
    
	return FI_SUCCESSFUL;
}

int FiHddGetTotalMounted()
{
	int totalMount;
    
	g_mutexHdd.Lock();

	totalMount = g_hddManage.totalMounted;
    
	g_mutexHdd.Unlock();
    
	return totalMount;
}

// 专门为删除录像做这个接口
int FiHddStopHotPlugForDelRecord()
{    
	HddStateSet( HDD_STATE_STOP );
	while( 1 )
    {
    	if( HDD_STATE_SLEEP == HddStateGet() )
        {
        	break;
        }
    	Usleep(1);
    }

	return 0;
}

int FiHddRestartHotPlugForDelRecord()
{
	HddStateSet( HDD_STATE_START );
	while( 1 )
    {
    	if( HDD_STATE_NORMAL == HddStateGet() )
        {
        	break;
        }
    	Usleep(1);
    }
	return 0;
}

int FiHddGetHddManage(HDD_MANAGE_ST *pHddManageSt)
{
    if( pHddManageSt )
    {
        Memcpy( pHddManageSt, &g_hddManage, sizeof(g_hddManage) );
    }
    return 0;
}

int FiHddHddStatErrReport(int errCode)
{
	MSG_CMD_T msgCmd;
    DCP_HDD_STAT_ERR_T *pDcpHddStatErr;

    //发送到客户端
    msgCmd.cmd = DIL_HDD_STAT_ERR_REPORT;
    msgCmd.dataLen = sizeof(DCP_HDD_STAT_ERR_T);
    msgCmd.pData = (char *)Malloc(msgCmd.dataLen);
    if(NULL != msgCmd.pData)
    {
        pDcpHddStatErr = (DCP_HDD_STAT_ERR_T *)msgCmd.pData;
        pDcpHddStatErr->errCode = errCode;
        MessageSend( MSG_ID_DCP_SIGNAL, (char *)&msgCmd, sizeof(msgCmd) );
    }

	return 0;
}

