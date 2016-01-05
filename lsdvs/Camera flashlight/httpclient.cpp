/***********************************************************
author :sven
date    :2014年4月10日 
description:用来实现与手持摄像仪HTTP server的连接
*************************************************************/


int httpclient::httpclient()
{
    m_connectstatus = 0;
    m_initstatus = 0;
    m_httpsock = -1;
    m_port = 0;
    memset(m_IP,0,sizeof(m_IP));
    m_sndbuf = NULL;
}

int httpclient::~httpclient()
{
    if(!m_initstatus)
    {
        return 0;
    }

    //tbd
}

#define CAMERASENDLEN 200
int httpclient::clientinit(int ncameraPort,char scameraIP[16])
{
    if(m_initstatus)
    {
        return -1;
    }

    m_httpsock == socket( AF_INET, SOCK_STREAM, 0 );
    if(-1 == m_httpsock)
    {
        return -1;
    }

    m_sndbuf = (char*)malloc(CAMERASENDLEN);
    if( NULL == m_sndbuf)
    {
        return -1;
    }

    //u should check IP & PORT
    m_port = ncameraPort;
    memcpy(m_IP,scameraIP,sizeof(m_IP));
    m_initstatus = 1;

    return 0;
}

int httpclient::connectcamera()
{
    int nret;
    struct sockaddr_in remoteAddr = { 0 };

    if(!m_initstatus)
    {
        return -1;
    }

    if(m_connectstatus)
    {
        return 0;
    }
    
    remoteAddr.sin_family	    = AF_INET;
	remoteAddr.sin_port	        = htons( m_port);
	remoteAddr.sin_addr.s_addr	= inet_addr( m_IP );
    nret = connect(m_httpsock,(struct sockaddr *)&remoteAddr,sizeof(remoteAddr));

    if(nret)
    {
        return nret;
    }
    
    m_connectstatus = 1;
    return 0;
}

/**********************************
*func:if there are stream data
*return:0 no data ,1 have data ,other:error
**********************************/
//tbd




//now we do not know the identify key XXXXXX
//
typedef struct
{
    int length;
    char *virutaladdr;
}CameraVedio_t;

typedef struct
{
    int length;
    char *virtualaddr;
}CameraAudio_t;


//?????封装2:开放socket，此函数变为给camera发送开始发送流命令，
////////////// 在加一个获取socket函数//次设计不安全，但是不在外面关闭就行了
//封装3:针对封装2，不开放socket号。封装一个socket的select函数，判断是否有数据，
//////////在封装一个获取stream的函数
int httpclient::get_stream()
{
    if(!m_connectstatus)
    {
        return -1;
    }

    memset(m_sndbuf,0,CAMERASENDLEN);
    sprintf(m_sndbuf, "GET /server.stream&identify_key=xxxxx HTTP/1.0\r\n\r\n");
    //strcat(m_sndbuf, "Host: %s/n/r/n",);


    Sendn(m_httpsock, (char*)m_sndbuf, strlen(m_sndbuf));

    //get stream from camera the write to our sys
    //tbd
    
    return 0;
}
//step1:get jpeg data and mp3 data 

//step2:trans jpeg to h264 trans mp3 to HISI audio data(data->vdec(become VIDEO_FRAME_S)->venc(become h264))
//         use HI_MPI_VDEC_SendStream_TimeOut  &  HI_MPI_VENC_SendFrame
//step3:get h264data & audio data
//step 4:use AudioFromHisiAddrToMyAddr &  H264FromHisiAddrToMyAddr

//2       takecare
#define DECCHNFORFLASH 4 //tbd this channel is not create    
#define ENCGRPFORFLASH 4 //tbd this GRP is not create   
#define ENCCHNFORFLASH 4 //tbd this CHN is not create   
#define CAMERADEFAULTCHN 4
//2       takecare


//////////////////just for logical????????????//////////////////////////////////////////////
int cameradatadec(CameraVedio_t *vediobuff)
{
    VDEC_CHN VdChn = DECCHNFORFLASH;
    VDEC_STREAM_S stStream;
    HI_U32 u32MilliSec;
    
    if(NULL == vediobuff)
    {
        return -1;
    }

    stStream.u32Len = vediobuff->length;
    stStream.pu8Addr = vediobuff->virutaladdr;
    stStream.u64PTS  = 0;
    u32MilliSec = 0;
    HI_MPI_VDEC_SendStream_TimeOut(VdChn,  &stStream, u32MilliSec);
}


int cameravideodatatrans()
{
    VDEC_CHN VdChn = DECCHNFORFLASH;
    VENC_GRP VeGroup = ENCGRPFORFLASH;
    VENC_CHN VeChn = ENCCHNFORFLASH;
    HI_U32 u32BlockFlag = 0;
    VIDEO_FRAME_INFO_S stFrameInfo;
    VENC_STREAM_S stStream;
    HI_BOOL bBlockFlag = 0;
    
    HI_MPI_VDEC_GetImage(VdChn, &stFrameInfo, u32BlockFlag);

    HI_MPI_VENC_SendFrame(VeGroup, &stFrameInfo);

    HI_MPI_VDEC_ReleaseImage(VdChn, &stFrameInfo);

    HI_MPI_VENC_GetStream(VeChn, &stStream, bBlockFlag);

    H264FromHisiAddrToMyAddr(CAMERADEFAULTCHN,(void *)&stStream);

    HI_MPI_VENC_ReleaseStream(VeChn, &stStream);

    
}
//////////////////just for logical????????????//////////////////////////////////////////////






int httpclient::get_video()
{
    if(!m_connectstatus)
    {
        return -1;
    }
    
    return 0;
}

int httpclient::get_audio()
{
    if(!m_connectstatus)
    {
        return -1;
    }
    
    return 0;
}


int httpclient::get_snapshot()
{
    if(!m_connectstatus)
    {
        return -1;
    }
    
    return 0;
}


int httpclient::storage_check()
{
    if(!m_connectstatus)
    {
        return -1;
    }
    
    return 0;
}


int httpclient::record_start()
{
    if(!m_connectstatus)
    {
        return -1;
    }
    
    return 0;
}


int httpclient::record_stop()
{
    if(!m_connectstatus)
    {
        return -1;
    }
    
    return 0;
}

int httpclient::record_status_get(e_recordstatus *recordstatus)
{
    if(!m_connectstatus)
    {
        return -1;
    }

    if(NULL == recordstatus)
    {
        return 1;
    }
    
    return 0;
}


int httpclient::mutestatus_set(e_mutestatus mutestatus)
{
    if(!m_connectstatus)
    {
        return -1;
    }
    
    return 0;
}

int httpclient::resolution_set(e_cfresolution cfreseolution)
{
    if(!m_connectstatus)
    {
        return -1;
    }
    
    return 0;
}

int httpclient::resolution_get(e_cfresolution *pcfreseolution)
{
    if(NULL == pcfreseolution)
    {
        return -2;
    }
    
    if(!m_connectstatus)
    {
        return -1;
    }
    
    return 0;
}

int httpclient::quality_set(int quality)
{
    if(quality < 0||quality > 15)
    {
        return -1;
    }

    if(!m_connectstatus)
    {
        return -1;
    }
    
    return 0;
}

int httpclient::quality_get(int* pquality)
{
    if(NULL == pquality)
    {
        return -1;
    }

    if(!m_connectstatus)
    {
        return -1;
    }
    
    *pquality = 1;
    return 0;
}


//u shuld set a range of fps
int httpclient::maxfps_set(int fps)
{
    if(fps < 24||fps > 111111115)
    {
        return -1;
    }

    if(!m_connectstatus)
    {
        return -1;
    }
    
    return 0;
}

int httpclient::maxfps_get(int *fps)
{
    if(fps == NULL)
    {
        return -1;
    }

    if(!m_connectstatus)
    {
        return -1;
    }
    
    fps = 24;
    return 0;
}

int httpclient::currentfps_get(int *fps)
{
    if(fps == NULL)
    {
        return -1;
    }

    if(!m_connectstatus)
    {
        return -1;
    }
    
    fps = 24;
    return 0;
}

