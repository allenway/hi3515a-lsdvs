/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.01.17
**  description  : 
********************************************************************************/

#include "main.h"


static char g_mainloopFlag = 1;
static char g_devicerebootFlag = 0;

static void quit_main_loop( int )
{
    SVPrint("Press CTRL + C,quite application!\n");
	g_mainloopFlag = 0;
}

/*
static void DoNothing( int )
{    
}
*/
static void sys_init_signal()
{
	signal( SIGINT, quit_main_loop);
	signal( SIGTERM, SIG_IGN );
	signal( SIGQUIT, SIG_IGN );
	signal( SIGPIPE, SIG_IGN);
    //signal(SIGSEGV, dump);
}

static void sys_init_timer()
{
	int ret;
	ret = CreateRTimer( TIMER_PRECISION_SEC );
	if( 0 == ret )
    {
    	SysRunTimeAddTimer();
    	FitEncAddTimer();
        //AddSnapTimer();
    	RealTimeAddTimer();
    	SysConfigAddTimer();
        SysLogAddTimer();
        //DriverVideoStandardDetectAddTimer();
    }
}

static void sys_init_all_work_param()
{        
	ParamEasyInit();
	InitSnapWorkParam();
}

static void sys_init()
{
	sys_init_signal();    
    //GpioInit();//move at 20130926
	FiSerialInit();
	RealTimeSyncRtcWhenStartSys();
	InitParamConfig();
	sys_init_all_work_param();    
    WiredInitNetwork();//
    //DdnsAppRefresh();//add at 20131026
    LogInit();
	sys_init_timer();
	PtpoolInit();
	ProconInit();    
	FitFiOsdInitOsdLib();
    //InitRecordPool();//(录像相关)
//	InitIpc();    
//	LsppInitSysConfig();
//	FiPtzInit();
//	DriverInit();//ADc初始化应该在改板后添加
//	WifiInitNetwork();
}

static void sys_deinit()
{    
	DestroyRTimer();
	FitFiOsdDeinitOsdLib();
//	DeinitIpc();
	GpioDeinit();
    LogClean();
	DeinitRecordPool();
}


//////////////////////////////////////////////////////////////////
//follow is for test VIDEO RECORD
/////////////////////////////////////////////////////////////////
pthread_t 	test_venc_id;

void* thread_test_venc(void *arg)
{
    int ret = -1;
    uint fd;
    int channel = 2;
    PROCON_NODE_T *pH264;
    FILE* pfd = fopen("/nfsroot/yf.h264", "wb");

    if(!pfd)
    {
        ERRORPRINT( "failed:fopen!\r\n" );
        return NULL;
    }

    fd = ProconH264Open( channel, OPEN_RDONLY );
    if( fd > 0 )
    {
       
    }
    else
    {
        ret = -1;
        ERRORPRINT( "failed:ProconH264Open()!\r\n" );
        return NULL;
    }

    CORRECTPRINT("START YF TEST\n");
    while(1)
    {
        pH264 = ProconH264Read( fd );
        if( NULL != pH264 )
        {
            CORRECTPRINT("now write\n");
            fwrite((char*)pH264->data,pH264->proconHead.len,1,pfd);
        	ProconH264Free( pH264 );
            usleep(10);
        }
        else
        {
            CORRECTPRINT("HAVE NO DATA\n");
            usleep(1000);
        }
    }
}
void test_vec()
{
    int ret;
    ret = ThreadCreate(&test_venc_id,thread_test_venc,NULL);
    if( 0!= ret )
    {        
        SVPrint( "error:ThreadCreate:%s\r\n", STRERROR_ERRNO );
    }
}
/////////////////////////////////////////////////////////////////


static void sys_start_all_service()
{
    //TestServiceStart();//move at 20130926
    //FiHddStartHotplugService();//move at 20130926(热插拔服务开启)
    //FiRecStartRecordService(); //move at 20130926(录像相关)
	FitMpiServiceStart();
    //AlarmStartService();//move at 20130926
    StartBroadcastServerThread();
	BhdcpServiceStart();
    //RtspServiceStart();//move at 20130926
    //test_vec();
    WdtServiceStart();  //move at 20130926(正式版本记得打开)
}

static void sys_stop_all_service()
{
    WdtServiceStop();//move at 20130926(正式版本记得打开)
	//RtspServiceStop();//move at 20130926
	BhdcpServiceStop();
    StopBroadcastServerThread();
    
    //AlarmStopService();//move at 20130926
    // TestServiceStop();
    //StopSnapAppThread();
    //StopSnapJpgThread();
	FitMpiServiceStop();
    // StopFtpJpgThread();
    // StopNtpAppService();
//	LsppAppServiceSop();
    
	//FiHddStopHotplugService();//move at 20130926(热插拔，和录像相关)
    
    //FiRecStopRecordService();//move at 20130926
    // StopIcmpAppThread();
    // StopDtuAppThread();
}


static void sys_main_loop()
{
	while( g_mainloopFlag )
    {
        // 网络地址有改变, 需重启网络传输相关线程
        if ( MessageRecv( MSG_ID_NETWORK_CHANGE) >= 0 )
        {
            //sleep(1);
            BhdcpServiceStop();
            StopBroadcastServerThread();
            usleep(1000);//sleep(1);
            StartBroadcastServerThread();
            BhdcpServiceStart();
        }
#if 1
        if ( MessageRecv( MSG_ID_REBOOT) >= 0 )
        {
            g_devicerebootFlag = 1;
            break;
        }
#endif
    	usleep(2000);
    }
}


int main( int argc, char *argv[] )
{    
    ColorPrint(COLORBTLGREEN, "########Start dvs app,datetime:%s %s########\n", \
        __DATE__, __TIME__ );
    
	sys_init();
	sys_start_all_service();

    LogAdd(0xff, LOG_TYPE_SYSTEM, LOG_LEVEL_INFO, \
        "application start! %s enter sys_main_loop!", __FUNCTION__);
    
	sys_main_loop();
    
    LogAddAndWriteFlash(0xff, LOG_TYPE_SYSTEM, LOG_LEVEL_INFO, \
        "application stop! %s exit sys_main_loop!", __FUNCTION__);
    
	sys_stop_all_service();
	sys_deinit();

    if( g_devicerebootFlag )
    {
        ColorPrint(COLORPURPLE, "############system reboot.######\n");
        Reboot(NULL);
    }
	return 0;
}


