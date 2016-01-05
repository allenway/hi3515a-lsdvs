/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : procon 应用
********************************************************************************/

#include "public.h"
#include "proconApp.h"
#include "proconH264.h"
#include "proconJpg.h"
#include "pcpIpcSnap.h"
#include "proconMd.h"
#include "pcpRecord.h"

void ProconInit()
{
    //ProconJpgInit( MAX_JPG_CHN_NUM, MAX_JPG_DATA_NODE );
	ProconH264Init( REAL_CHANNEL_NUM, MAX_H264_DATA_NODE );
    //PcpIpcSnapInit( IPC_JPG_CHN_NUM, MAX_IPC_SNAP_DATA_NODE, MAX_IPC_SNAP_DATA_SIZE);
    //PcpRecordInit( REAL_CHANNEL_NUM, MAX_RECORD_DATA_NODE, MAX_RECORD_DATA_SIZE );
    //ProconMdInit( REAL_CHANNEL_NUM, MAX_MD_DATA_NODE );
}

