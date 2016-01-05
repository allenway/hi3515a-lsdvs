#include "debug.h"
#include "rand.h"
#include "sysRunTime.h"
#include "log.h"
#include "testLog.h"

static const unsigned char s_logTypes[] =
{
LOG_TYPE_ALL,
LOG_TYPE_SYSTEM,
LOG_TYPE_DATABASE,
LOG_TYPE_STORAGE,
LOG_TYPE_MODULEx,
};

static const unsigned char s_logLevels[] =
{
LOG_LEVEL_DEBUG,
LOG_LEVEL_INFO,
LOG_LEVEL_WARN,
LOG_LEVEL_ERROR,
LOG_LEVEL_CRITICAL,
};

static int s_logWriteTime;
static int s_logPrintTime;
    
void TestLogInit(void)
{
    s_logWriteTime = SysRunTimeGet();
    s_logPrintTime = s_logWriteTime;
    SVPrint("TestLogInit\r\n");
    // PrintLog(LOG_TYPE_ALL,LOG_LEVEL_DEBUG);
}

void TestLog(void)
{
    int time;
    int level;
    int type;

    time = SysRunTimeGet();
    if(time - s_logWriteTime>9)
    {

        level = RandAb(0,4);
        type = RandAb(1,4);
        LogAdd(0xff, s_logTypes[type], s_logLevels[level], "test Log %d %d", level, type);
        s_logWriteTime = time;
    }
    if(s_logWriteTime - s_logPrintTime > 599)
    {
        LogPrint(LOG_TYPE_ALL,LOG_LEVEL_DEBUG);
        s_logPrintTime = s_logWriteTime;
    }
    
}
