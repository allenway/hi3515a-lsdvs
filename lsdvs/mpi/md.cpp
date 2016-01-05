/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.10.10
**  description  : 利用mpi 实现移动侦测 
********************************************************************************/


#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "const.h"
#include "thread.h"
#include "md.h"
#include "debug.h"
#include "message.h"
#include "condition.h"
#include "paramManage.h"
#if defined MCU_HI3515A
#include "hi_comm_vda.h"
#endif
#include "hi_common.h"
#include "encComm.h"
#if defined MCU_HI3515A
#include "mpi_vda.h"
#endif
#include "proconMd.h"
#include "mpi_comm.h"


static MD_ATTR_T g_mdAttr[REAL_CHANNEL_NUM];
static THREAD_MAINTAIN_T g_mdTm;
static int g_mdThreadRunFlag = 0;
static ClCondition g_mdCondition;

static int SetMDArea ( int ch, MD_AREA_T mdArea )
{
	int i, j, r, c;
	PARAM_CONFIG_VIDEO_ENCODE         	pcve;
	PARAM_CONFIG_VIDEO_ENCODE_PUBLIC 	pcvep;

	bzero( g_mdAttr[ch].mask, sizeof(g_mdAttr[ch].mask) );    
	ParamGetVideoEncode( ch, &pcve );    
	ParamGetVideoEncodePublic( &pcvep );

	if( pcvep.videoStandard == HI_PAL )
    {
    	if( pcve.resolution == HI_D1 ) //d1
        {
        	g_mdAttr[ch].macroRatio     = 2;//10;
        	g_mdAttr[ch].hBlock         = 18*2;
        	g_mdAttr[ch].wBlock         = 45;
        	for( i=0; i<12; i++ )
            {
            	for( j=0; j<15; j++ )
                {
                	if( (mdArea.area[i]>>(15-j))&1 )
                    {
                    	for( r=0; r<3; r++ )
                        	for( c=0; c<3; c++ )
                            	g_mdAttr[ch].mask[(i*3+r)*(g_mdAttr[ch].wBlock) + j*3+c] = 1;
                    }                
                }
            }
        	for( i=0; i<12; i++ )
            {
            	j = 15;
                {
                	if( (mdArea.area[i]>>(15-j))&1 )
                    {
                    	for(r=0; r<3; r++)
                        	for(c=0; c<3; c++)
                            	g_mdAttr[ch].mask[(i*3+r)*(g_mdAttr[ch].wBlock) + (j-1)*3+c] = 1;
                    }
                
                }
            }
        }
    	else if( pcve.resolution == HI_HD1 )
        {
        	g_mdAttr[ch].macroRatio = 4;//10;
        	g_mdAttr[ch].hBlock = 18;
        	g_mdAttr[ch].wBlock = 45;
        	for( i=0; i<12; i++ )
            {
            	for( j=0; j<15; j++ )
                {
                	if( (mdArea.area[i]>>(15-j))&1 )
                    {
                    	for( r=0; r<2; r++ )
                        	for( c=0; c<3; c++ )
                            	g_mdAttr[ch].mask[(i*3/2+r)*(g_mdAttr[ch].wBlock) + j*3+c] = 1;
                    }
                
                }
            }
        	for( i=0; i<12; i++ )
            {
            	j = 15;
                {
                	if( (mdArea.area[i]>>(15-j))&1 )
                    {
                    	for( r=0; r<2; r++ )
                        	for( c=0; c<3; c++ )
                            	g_mdAttr[ch].mask[(i*3/2+r)*(g_mdAttr[ch].wBlock) + j*3+c] = 1;
                    }
                
                }
            }
        }
    	else if( pcve.resolution == HI_CIF )
        {
        	g_mdAttr[ch].macroRatio = 6;//10;
        	g_mdAttr[ch].hBlock = 18;
        	g_mdAttr[ch].wBlock = 22;
            
        	for(i=0; i<12; i++ )
            {
            	for(j=0; j<14; j++)
                {
                	if((mdArea.area[i]>>(15-j))&1)
                    {
                    	for(r=0; r<2; r++)
                        	for(c=0; c<2; c++)
                            	g_mdAttr[ch].mask[(i*3/2+r)*(g_mdAttr[ch].wBlock) + j*3/2+c] = 1;
                    }
                
                }
            }
        	for( i=0; i<12; i++ )
            {
            	for( j=14; j<16; j++ )
                {
                	if( (mdArea.area[i]>>(15-j))&1 )
                    {
                    	for(r=0; r<2; r++)
                        	for(c=0; c<2; c++)
                            	g_mdAttr[ch].mask[(i*3/2+r)*(g_mdAttr[ch].wBlock) + 20+c] = 1;
                    }
                
                }
            }
            
        }
    	else if( pcve.resolution == HI_QCIF )
        {
        	g_mdAttr[ch].macroRatio     = 8;//10;
        	g_mdAttr[ch].hBlock         = 9;
        	g_mdAttr[ch].wBlock         = 11;
        	for( i=0; i<12; i++ )
            {
            	for( j=0; j<15; j++ )
                {
                	if( (mdArea.area[i]>>(15-j))&1 )
                    {
                    	for( r=0; r<1; r++ )
                        	for( c=0; c<1; c++ )
                            	g_mdAttr[ch].mask[(i*3/4+r)*(g_mdAttr[ch].wBlock) + j*3/4+c] = 1;
                    }
                
                }
            }
        	for( i=0; i<12; i++ )
            {
            	j = 15;
                {
                	if( (mdArea.area[i]>>(15-j))&1 )
                    {
                    	for( r=0; r<1; r++ )
                        	for( c=0; c<1; c++ )
                            	g_mdAttr[ch].mask[(i*3/4+r)*(g_mdAttr[ch].wBlock) + 10+c] = 1;
                    }
                
                }
            }
        
        }
    	else if( pcve.resolution == HI_VGA )
        {
        	g_mdAttr[ch].macroRatio     = 3;//10;
        	g_mdAttr[ch].hBlock         = 30;
        	g_mdAttr[ch].wBlock         = 40;
        	for( i=0; i<12; i++ )
            {
            	for( j=0; j<16; j++)
                {
                	if( (mdArea.area[i]>>(16-j))&1 )
                    {
                    	for( r=0; r<3; r++ )
                        	for( c=0; c<3; c++ )
                            	g_mdAttr[ch].mask[(i*5/2+r)*(g_mdAttr[ch].wBlock) + j*5/2+c] = 1;
                    }
                
                }
            }
        
        }
    	else if( pcve.resolution == HI_QVGA )
        {
        	g_mdAttr[ch].macroRatio     = 6;//10;
        	g_mdAttr[ch].hBlock         = 15;
        	g_mdAttr[ch].wBlock         = 20;
        	for(i=0; i<12; i++ )
            {
            	for( j=0; j<16; j++)
                {
                	if( (mdArea.area[i]>>(16-j))&1 )
                    {
                    	for( r=0; r<2; r++ )
                        	for( c=0; c<2; c++ )
                            	g_mdAttr[ch].mask[(i*5/4+r)*(g_mdAttr[ch].wBlock) + j*5/4+c] = 1;
                    }
                
                }
            }        
        }
    }
	else
    {
    	if( pcve.resolution == HI_D1 )
        {
        	g_mdAttr[ch].macroRatio     = 2;//10;
        	g_mdAttr[ch].hBlock         = 15*2;
        	g_mdAttr[ch].wBlock         = 45;
        	for( i=0; i<12; i++ )
            {
            	for( j=0; j<15; j++ )
                {
                	if( (mdArea.area[i]>>(15-j))&1 )
                    {
                    	for( r=0; r<3; r++)
                        	for( c=0; c<3; c++ )
                            	g_mdAttr[ch].mask[(i*5/2+r)*(g_mdAttr[ch].wBlock) + j*3+c] = 1;
                    }
                
                }
            }
        	for( i=0; i<12; i++ )
            {
            	j = 15;
                {
                	if((mdArea.area[i]>>(15-j))&1)
                    {
                    	for(r=0; r<3; r++)
                        	for(c=0; c<3; c++)
                            	g_mdAttr[ch].mask[(i*5/2+r)*(g_mdAttr[ch].wBlock) + (j-1)*3+c] = 1;
                    }
                
                }
            }
        }
    	else if( pcve.resolution == HI_HD1 )
        {
        	g_mdAttr[ch].macroRatio = 4;//10;
        	g_mdAttr[ch].hBlock = 15;
        	g_mdAttr[ch].wBlock = 45;
        	for(i=0; i<12; i++ )
            {
            	for(j=0; j<15; j++)
                {
                	if((mdArea.area[i]>>(15-j))&1)
                    {
                    	for(r=0; r<2; r++)
                        	for(c=0; c<3; c++)
                            	g_mdAttr[ch].mask[(i*5/4+r)*(g_mdAttr[ch].wBlock) + j*3+c] = 1;
                    }
                
                }
            }
        	for(i=0; i<12; i++ )
            {
            	j = 15;
                {
                	if((mdArea.area[i]>>(15-j))&1)
                    {
                    	for(r=0; r<2; r++)
                        	for(c=0; c<3; c++)
                            	g_mdAttr[ch].mask[(i*5/4+r)*(g_mdAttr[ch].wBlock) + j*3+c] = 1;
                    }
                
                }
            }
        }
    	else if( pcve.resolution == HI_CIF )
        {
        	g_mdAttr[ch].macroRatio = 6;//10;
        	g_mdAttr[ch].hBlock = 15;
        	g_mdAttr[ch].wBlock = 22;
            
        	for(i=0; i<12; i++ )
            {
            	for(j=0; j<14; j++)
                {
                	if((mdArea.area[i]>>(15-j))&1)
                    {
                    	for(r=0; r<2; r++)
                        	for(c=0; c<2; c++)
                            	g_mdAttr[ch].mask[(i*5/4+r)*(g_mdAttr[ch].wBlock) + j*3/2+c] = 1;
                    }
                
                }
            }
        	for(i=0; i<12; i++ )
            {
            	for(j=14; j<16; j++)
                {
                	if((mdArea.area[i]>>(15-j))&1)
                    {
                    	for(r=0; r<2; r++)
                        	for(c=0; c<2; c++)
                            	g_mdAttr[ch].mask[(i*5/4+r)*(g_mdAttr[ch].wBlock) + 20+c] = 1;
                    }
                
                }
            }            
        }
    	else if( pcve.resolution == HI_QCIF )
        {
        	g_mdAttr[ch].macroRatio     = 8;//10;
        	g_mdAttr[ch].hBlock         = 8;
        	g_mdAttr[ch].wBlock         = 11;
        	for( i=0; i<12; i++ )
            {
            	for( j=0; j<15; j++ )
                {
                	if( (mdArea.area[i]>>(15-j))&1 )
                    {
                    	for( r=0; r<1; r++ )
                        	for( c=0; c<1; c++ )
                            	g_mdAttr[ch].mask[(i*2/3+r)*(g_mdAttr[ch].wBlock) + j*3/4+c] = 1;
                    }
                
                }
            }
        	for( i=0; i<12; i++ )
            {
            	j = 15;
                {
                	if((mdArea.area[i]>>(15-j))&1)
                    {
                    	for(r=0; r<1; r++)
                        	for(c=0; c<1; c++)
                            	g_mdAttr[ch].mask[(i*2/3+r)*(g_mdAttr[ch].wBlock) + 10+c] = 1;
                    }
                
                }
            }        
        }
    	else if( pcve.resolution == HI_VGA )
        {
        	g_mdAttr[ch].macroRatio     = 3;//10;
        	g_mdAttr[ch].hBlock         = 30;
        	g_mdAttr[ch].wBlock         = 40;
        	for(i=0; i<12; i++ )
            {
            	for( j=0; j<16; j++ )
                {
                	if( (mdArea.area[i]>>(16-j))&1 )
                    {
                    	for( r=0; r<3; r++ )
                        	for( c=0; c<3; c++ )
                            	g_mdAttr[ch].mask[(i*5/2+r)*(g_mdAttr[ch].wBlock) + j*5/2+c] = 1;
                    }
                
                }
            }        
        }
    	else if( pcve.resolution == HI_QVGA )
        {
        	g_mdAttr[ch].macroRatio     = 6;//10;
        	g_mdAttr[ch].hBlock         = 15;
        	g_mdAttr[ch].wBlock         = 20;
        	for(i=0; i<12; i++ )
            {
            	for( j=0; j<16; j++ )
                {
                	if( (mdArea.area[i]>>(16-j))&1 )
                    {
                    	for( r=0; r<2; r++ )
                        	for( c=0; c<2; c++ )
                            	g_mdAttr[ch].mask[(i*5/4+r)*(g_mdAttr[ch].wBlock) + j*5/4+c] = 1;
                    }
                
                }
            }        
        }
    }//end else

	return 0;
}

// 设置灵敏度
static int SetMDSensitive( int channel )
{
	int	ret;
	MD_AREA_T mdArea;
	PARAM_CONFIG_ALARM_MOVE_DETECT pcamd;
    
	ret = ParamGetAlarmMoveDetect( channel, &pcamd );
	if( 0 == ret )
    {
    	switch( pcamd.sensitiveLevel )  
        {
        	case 9: 
            	g_mdAttr[channel].macroThreshold = 10;
            	break;  
        	case 8: 
            	g_mdAttr[channel].macroThreshold = 25; 
            	break;     
        	case 7: 
            	g_mdAttr[channel].macroThreshold = 40; 
            	break;     
        	case 6: 
            	g_mdAttr[channel].macroThreshold = 55; 
            	break; 
        	case 5: 
            	g_mdAttr[channel].macroThreshold = 70; 
            	break; 
        	case 4: 
            	g_mdAttr[channel].macroThreshold = 85; 
            	break;     
        	case 3: 
            	g_mdAttr[channel].macroThreshold = 105; 
            	break; 
        	case 2: 
            	g_mdAttr[channel].macroThreshold = 125; 
            	break;    
        	case 1: 
            	g_mdAttr[channel].macroThreshold = 150; 
            	break;     
        	default:
            	g_mdAttr[channel].macroThreshold = 40; 
            	break; 
        }
    	memcpy( mdArea.area, pcamd.detectArea.area, sizeof(mdArea.area) );
    	ret = SetMDArea( channel, mdArea );/*设置移侦区域*/
    }
    
	return ret;
}


/* 判断单个侦测区域是否报警 */
static int ShouldAlarm( unsigned int avgSadValue[MAX_MACROCELL_NUM], int ch )
{
	int r,c;
	int Area_macrocell_num           = 0;    /*侦测区域中的宏块个数 */
	int Area_Alarm_macrocell_num     = 0;    /*侦测区域中报警宏块个数 */

	for( r=0; r<g_mdAttr[ch].hBlock; r++ )
    {
    	for( c=0; c<g_mdAttr[ch].wBlock; c++ )
        {
        	if( 1 ==  g_mdAttr[ch].mask[r*(g_mdAttr[ch].wBlock)+c] )
            {
            	Area_macrocell_num++;
            	if( avgSadValue[r*(g_mdAttr[ch].wBlock)+c] >= g_mdAttr[ch].macroThreshold )
                {
                	Area_Alarm_macrocell_num++;
                }
            } 
        } 
    }


	if( Area_macrocell_num == 0 || Area_Alarm_macrocell_num == 0 )
    {
    	return 0;
    }
	if( Area_Alarm_macrocell_num >= 5 )
    {
    	return 1;
    }
	else 
    {
    	return 0;
    }
}

static int CheckMdNeed()
{
	int i, j, ret = 0;
    
	PARAM_CONFIG_ALARM_MOVE_DETECT pcamd;
    
	for( i = 0; i < REAL_CHANNEL_NUM; i++ )
    {
    	ret = ParamGetAlarmMoveDetect( i, &pcamd );
    	if( 0 == ret )
        {
        	if( 1 == pcamd.armFlag )
            {
            	ret = 1;
            	break;
            }
        	else
            {
            	for( j = 0; j < MAX_WEEK_DAY; j++ )
                {
                	if( 1 == pcamd.armTimer.day[i].enableFlag )
                    {
                    	ret = 1;
                    	break;
                    }
                } 
            }
        }
    }
	return ret;
}

#if defined MCU_HI3515
static int EnableMD( int channel )
{
	int ret;
	MD_CHN_ATTR_S stMdAttr;
	MD_REF_ATTR_S stRefAttr;
	VENC_CHN VeChn = channel << 1;

	g_mdAttr[channel].frameCount = 0;
	bzero( g_mdAttr[channel].sadSum, sizeof(g_mdAttr[channel].sadSum) );
	SetMDSensitive( channel );

    /*set MD attribute*/
	stMdAttr.stMBMode.bMBSADMode    = HI_TRUE;
	stMdAttr.stMBMode.bMBMVMode     = HI_FALSE;
	stMdAttr.stMBMode.bMBPelNumMode = HI_FALSE;
	stMdAttr.stMBMode.bMBALARMMode  = HI_FALSE;
	stMdAttr.u16MBALSADTh           = g_mdAttr[channel].macroThreshold;
	stMdAttr.u8MBPelALTh            = 20;
	stMdAttr.u8MBPerALNumTh         = 20;
	stMdAttr.enSADBits              = MD_SAD_8BIT;
	stMdAttr.stDlight.bEnable       = HI_FALSE;
	stMdAttr.u32MDInternal          = 0;
	stMdAttr.u32MDBufNum            = 16;
    /*set MD frame*/
	stRefAttr.enRefFrameMode        = MD_REF_AUTO;
	stRefAttr.enRefFrameStat        = MD_REF_DYNAMIC;

	ret =  HI_MPI_MD_SetChnAttr( VeChn, &stMdAttr );
	if( HI_SUCCESS != ret )
    {
        SVPrint( "HI_MPI_MD_SetChnAttr Err 0x%x\n", ret );
        return ret;
    }
	ret = HI_MPI_MD_SetRefFrame( VeChn, &stRefAttr );
	if( HI_SUCCESS != ret )
    {
        SVPrint( "HI_MPI_MD_SetRefFrame Err 0x%x\n", ret );
        return ret;
    }
	ret = HI_MPI_MD_EnableChn( VeChn );
	if( HI_SUCCESS != ret )
    {
        SVPrint( "HI_MPI_MD_EnableChn Err 0x%x\n", ret );
        return ret;
    }    
    
	return ret;
}

static int DisableMD( int ch )
{
	int ret;
	VENC_CHN VeChn = ch << 1;
    
	ret = HI_MPI_MD_DisableChn( VeChn );
	if( HI_SUCCESS != ret )
    {
        SVPrint( "HI_MPI_MD_DisableChn Err 0x%x\n", ret );
        return ret;
    }
	return ret;
}
static void *MdThread( void *arg )
{
	int ret = 0;    
	MD_DATA_S mdData;
	int i, j, k;
	ushort *pTmp = NULL;
	static int oldStatus[MAX_CHANNEL_NUM]  = {0};
	static int status[MAX_CHANNEL_NUM]     = {0};
	int mdFd[8]={0};
	int maxfd = 0;
	int chNum = 0;
	fd_set readFds;
	struct timeval tv;
	int vencNo;
	chNum = REAL_CHANNEL_NUM;
	uint fdmd[REAL_CHANNEL_NUM];
	DATA_PIECE_T dataPiece;
	char bufT[4];

	dataPiece.buf[0] = bufT;
	dataPiece.len[0] = sizeof(bufT);
	dataPiece.count  = 1;

	while( g_mdThreadRunFlag )
    {
    	if( 0 == CheckMdNeed() )     // 如果全部通道都没有打开移动侦测
        {        
        	g_mdCondition.Wait();     // 则进入休眠
        	MessageRecv( MSG_ID_MD_PARAM_CHANGE );
        	continue;
        }
        
    	for( i = 0; i < chNum; i++ )
        {
        	ret = EnableMD( i );
        	if(ret < 0)
            {
            	SVPrint( "Enable channel(%d) MD err!\n", i );
            	return NULL;
            }
        	vencNo = i << 1;
        	mdFd[i] = HI_MPI_MD_GetFd( vencNo );
        	if( mdFd[i] <= 0 )
            {
            	SVPrint("HI_MPI_VENC_GetFd err\n");
            	return NULL;
            }
        	if( maxfd <= mdFd[i] ) 
            {
            	maxfd = mdFd[i];
            }

        	fdmd[i] = ProconMdOpen( i, OPEN_WRONLY );
        }
        
    	SVPrint( "%s start!\r\n", __FUNCTION__ );
    	while( g_mdTm.runFlag )
        {
        	if( MessageRecv( MSG_ID_MD_PARAM_CHANGE ) >= 0 )
            {
            	break; 
            }
        	FD_ZERO( &readFds );
        	for( i=0; i<chNum; i++ )
            {
            	FD_SET( mdFd[i], &readFds );
            }
        	tv.tv_sec     = 1;
        	tv.tv_usec     = 0;
            
        	ret = select( maxfd+1, &readFds, NULL, NULL, &tv );
        	if( ret < 0 )
            {
            	SVPrint( "error: select()!\r\n" );
            	break;
            }
        	else if( 0 == ret )
            {
            	SVPrint("failed: MD select time out!\n");
            	break;
            }
        	else
            {
            	for( i = 0; i < chNum; i++ )
                {
                	vencNo = i << 1;
                    /*get md data*/
                	memset( &mdData, 0, sizeof(MD_DATA_S) );
                	if( FD_ISSET(mdFd[i], &readFds) )
                    {    
                    	ret = HI_MPI_MD_GetData( vencNo, &mdData, HI_IO_NOBLOCK );
                    	if( ret != HI_SUCCESS )
                        {
                        	SVPrint( "HI_MPI_MD_GetData Err 0x%x\n", ret );
                        	continue;
                        }
                        /*process md data*/
                    	if( mdData.stMBMode.bMBSADMode )
                        {
                        	if( g_mdAttr[i].frameCount < MOVE_FRAME_INTERVAL )
                            {
                            	for( j=0; j<mdData.u16MBHeight; j++ )
                                {
                                                            
                                	pTmp = (HI_U16 *)((HI_U32)mdData.stMBSAD.pu32Addr + 
                                                        	j*mdData.stMBSAD.u32Stride);                                    
                                	for( k=0; k<mdData.u16MBWidth; k++ )
                                    {
                                    	g_mdAttr[i].sadSum[j*mdData.u16MBWidth+k] += (*pTmp); 
                                    	pTmp++;                                    
                                    }    
                                }
                            	g_mdAttr[i].hBlock = mdData.u16MBHeight;
                            	g_mdAttr[i].wBlock = mdData.u16MBWidth;
                            	g_mdAttr[i].frameCount++;
                            }
                        	else
                            {
                            	if( 1 == ShouldAlarm(g_mdAttr[i].sadSum, i) )
                                {
                                	status[i] = 1;
                                	if( oldStatus[i] != status[i] )
                                    {                                        
                                    	ProconMdWrite( fdmd[i], dataPiece );
                                    	SVPrint( "ch(%d)=====>SEND MOVE ALARM!\r\n", i );
                                    }
                                }
                            	else
                                {
                                	status[i] = 0;
                                	if( oldStatus[i] != status[i] )
                                    {                                        
                                    	SVPrint( "ch(%d)=====>STOP MOVE ALARM!\r\n", i );
                                    }
                                }
                            	oldStatus[i] = status[i];
                                
                            	g_mdAttr[i].frameCount = 0;
                            	for( j=0; j<mdData.u16MBHeight; j++ )
                                {
                                	for( k=0; k<mdData.u16MBWidth; k++ )
                                    {
                                    	g_mdAttr[i].sadSum[j*mdData.u16MBWidth+k] = 0; 
                                    }    
                                }
                            }
                        }
                        /*release md data*/
                    	ret = HI_MPI_MD_ReleaseData( vencNo, &mdData );
                    	if( ret != 0 )
                        {
                        	SVPrint( "Md Release Data Err 0x%x\n", ret );
                        	continue;
                        }
                    }
                } 
            }        
        } 	    
    	for( i = 0; i < chNum; i++ )
        {
        	DisableMD( i );
        }
    } 
    
	SVPrint( "%s stop!\r\n", __FUNCTION__ );
    
	return 0;
}
#endif

static int EnableMD( int channel )
{
	//int ret;
	//MD_CHN_ATTR_S stMdAttr;
	//MD_REF_ATTR_S stRefAttr;

	g_mdAttr[channel].frameCount = 0;
	bzero( g_mdAttr[channel].sadSum, sizeof(g_mdAttr[channel].sadSum) );
	SetMDSensitive( channel );

    /*set MD attribute
	stMdAttr.stMBMode.bMBSADMode    = HI_TRUE;
	stMdAttr.stMBMode.bMBMVMode     = HI_FALSE;
	stMdAttr.stMBMode.bMBPelNumMode = HI_FALSE;
	stMdAttr.stMBMode.bMBALARMMode  = HI_FALSE;
	stMdAttr.u16MBALSADTh           = g_mdAttr[channel].macroThreshold;////'''''''''''''''''''''''''''''
	stMdAttr.u8MBPelALTh            = 20;
	stMdAttr.u8MBPerALNumTh         = 20;
	stMdAttr.enSADBits              = MD_SAD_8BIT;
	stMdAttr.stDlight.bEnable       = HI_FALSE;
	stMdAttr.u32MDInternal          = 0;
	stMdAttr.u32MDBufNum            = 16;*/
    /*set MD frame
	stRefAttr.enRefFrameMode        = MD_REF_AUTO;
	stRefAttr.enRefFrameStat        = MD_REF_DYNAMIC;*/

    
    HI_S32 s32Ret = HI_SUCCESS;
    VDA_CHN_ATTR_S stVdaChnAttr;
    MPP_CHN_S stSrcChn, stDestChn;
    VDA_CHN VdaChn = channel;
    VI_CHN ViChn = channel;

    VIDEO_NORM_E enNorm = VIDEO_ENCODING_MODE_PAL;
    PIC_SIZE_E enPicSize = PIC_D1;
    SIZE_S  stSize;
    memset(&stSize,0,sizeof(stSize));
    s32Ret = mpi_comm_sys_get_pic_size(enNorm, enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        ERRORPRINT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        return s32Ret;
    }
    
    if (VDA_MAX_WIDTH < stSize.u32Width || VDA_MAX_HEIGHT < stSize.u32Height)
    {
        ERRORPRINT("Picture size invaild!\n");
        return HI_FAILURE;
    }
    
    /* step 1 create vda channel */
    stVdaChnAttr.enWorkMode = VDA_WORK_MODE_MD;
    stVdaChnAttr.u32Width   = stSize.u32Width;
    stVdaChnAttr.u32Height  = stSize.u32Height;

    stVdaChnAttr.unAttr.stMdAttr.enVdaAlg      = VDA_ALG_REF;
    stVdaChnAttr.unAttr.stMdAttr.enMbSize      = VDA_MB_16PIXEL;
    stVdaChnAttr.unAttr.stMdAttr.enMbSadBits   = VDA_MB_SAD_8BIT;
    stVdaChnAttr.unAttr.stMdAttr.enRefMode     = VDA_REF_MODE_DYNAMIC;
    stVdaChnAttr.unAttr.stMdAttr.u32MdBufNum   = 8;
    stVdaChnAttr.unAttr.stMdAttr.u32VdaIntvl   = 4;  
    stVdaChnAttr.unAttr.stMdAttr.u32BgUpSrcWgt = 128;
    stVdaChnAttr.unAttr.stMdAttr.u32SadTh      = 100;
    stVdaChnAttr.unAttr.stMdAttr.u32ObjNumMax  = 128;

    s32Ret = HI_MPI_VDA_CreateChn(VdaChn, &stVdaChnAttr);
    if(s32Ret != HI_SUCCESS)
    {
        ERRORPRINT("err!\n");
        return s32Ret;
    }

    /* step 2: vda chn bind vi chn */
    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32ChnId = ViChn;
    stSrcChn.s32DevId = 0;

    stDestChn.enModId = HI_ID_VDA;
    stDestChn.s32ChnId = VdaChn;
    stDestChn.s32DevId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if(s32Ret != HI_SUCCESS)
    {
        ERRORPRINT("err!\n");
        return s32Ret;
    }

    /* step 3: vda chn start recv picture */
    s32Ret = HI_MPI_VDA_StartRecvPic(VdaChn);
    if(s32Ret != HI_SUCCESS)
    {
        ERRORPRINT("err!\n");
        return s32Ret;
    }
    
	return s32Ret;
}

static int DisableMD( int ch )
{
	HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn, stDestChn;
    VDA_CHN VdaChn = ch;
    VI_CHN  ViChn  = ch;

	
    /* vda stop recv picture */
    s32Ret = HI_MPI_VDA_StopRecvPic(VdaChn);
    if(s32Ret != HI_SUCCESS)
    {
        ERRORPRINT("err(0x%x)!!!!\n", s32Ret);
        return s32Ret;
    }

    /* unbind vda chn & vi chn */
    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32ChnId = ViChn;
    stSrcChn.s32DevId = 0;
    stDestChn.enModId = HI_ID_VDA;
    stDestChn.s32ChnId = VdaChn;
    stDestChn.s32DevId = 0;
    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if(s32Ret != HI_SUCCESS)
    {
        ERRORPRINT("err(0x%x)!!!!\n", s32Ret);
        return s32Ret;
    }

    /* destroy vda chn */
    s32Ret = HI_MPI_VDA_DestroyChn(VdaChn);
    if(s32Ret != HI_SUCCESS)
    {
        ERRORPRINT("err(0x%x)!!!!\n",s32Ret);
    }
    
    return s32Ret;
}

static void *MdThread( void *arg )
{
	int ret = 0;    
	unsigned int i, j, k;
	HI_VOID *pTmp = NULL;
	static int oldStatus[MAX_CHANNEL_NUM]  = {0};
	static int status[MAX_CHANNEL_NUM]     = {0};
	int mdFd[8]={0};
	int maxfd = 0;
	unsigned int chNum = REAL_CHANNEL_NUM;
	fd_set readFds;
	struct timeval tv;

	uint fdmd[REAL_CHANNEL_NUM];
	DATA_PIECE_T dataPiece;
	char bufT[4];
	dataPiece.buf[0] = bufT;
	dataPiece.len[0] = sizeof(bufT);
	dataPiece.count  = 1;

    VDA_CHN VdaChn;
    VDA_DATA_S stVdaData;

	while( g_mdThreadRunFlag )
    {
    	if( 0 == CheckMdNeed() )     // 如果全部通道都没有打开移动侦测
        {   
            SVPrint("MD close all\n");
        	g_mdCondition.Wait();     // 则进入休眠
        	MessageRecv( MSG_ID_MD_PARAM_CHANGE );
        	continue;
        }
        
    	for( i = 0; i < chNum; i++ )
        {
        	ret = EnableMD( i );
        	if(ret < 0)
            {
            	ERRORPRINT( "Enable channel(%d) MD err!\n", i );
            	return NULL;
            }
        	VdaChn = i;
        	mdFd[i] = HI_MPI_VDA_GetFd(VdaChn);
        	if( mdFd[i] <= 0 )
            {
            	ERRORPRINT("HI_MPI_VDA_GetFd err\n");
            	return NULL;
            }
        	if( maxfd <= mdFd[i] ) 
            {
            	maxfd = mdFd[i];
            }

        	fdmd[i] = ProconMdOpen( i, OPEN_WRONLY );
        }
        
    	CORRECTPRINT( "%s start!\r\n", __FUNCTION__ );
    	while( g_mdTm.runFlag )
        {
        	if( MessageRecv( MSG_ID_MD_PARAM_CHANGE ) >= 0 )
            {
            	break; 
            }
            
        	FD_ZERO( &readFds );
        	for( i = 0; i < chNum; i++ )
            {
            	FD_SET( mdFd[i], &readFds );
            }
            
        	tv.tv_sec  = 1;
        	tv.tv_usec = 0;
        	ret = select( maxfd+1, &readFds, NULL, NULL, &tv );
        	if( ret < 0 )
            {
            	ERRORPRINT( "error: select()!\r\n" );
            	break;
            }
        	else if( 0 == ret )
            {
            	ERRORPRINT("failed: MD select time out!\n");
            	break;
            }
        	else
            {
            	for( i = 0; i < chNum; i++ )
                {
                	VdaChn = i;
                    /*get md data*/
                	memset( &stVdaData, 0, sizeof(stVdaData) );
                	if( FD_ISSET(mdFd[i], &readFds) )
                    {    
                    	ret = HI_MPI_VDA_GetData( VdaChn, &stVdaData, HI_IO_NOBLOCK );
                    	if( ret != HI_SUCCESS )
                        {
                        	ERRORPRINT( "HI_MPI_VDA_GetData Err 0x%x\n", ret );
                        	continue;
                        }
                        /*process md data*/
                        if(stVdaData.unData.stMdData.bMbSadValid)
                        {
                        	if( g_mdAttr[i].frameCount < MOVE_FRAME_INTERVAL )
                            {
                            	for( j = 0; j < stVdaData.u32MbHeight; j++ )
                                {
                                                            
                                	pTmp = (HI_VOID *)((HI_U32)stVdaData.unData.stMdData.stMbSadData.pAddr+
                                	                        j*stVdaData.unData.stMdData.stMbSadData.u32Stride);  
                                	for( k = 0; k < stVdaData.u32MbWidth; k++ )
                                    {
                                    	if(VDA_MB_SAD_8BIT == stVdaData.unData.stMdData.stMbSadData.enMbSadBits)
                            	        {
                                            g_mdAttr[i].sadSum[j * stVdaData.u32MbWidth + k] += (*((HI_U8 *)pTmp + k));

                            	        }
                            	        else
                            	        {
                                            g_mdAttr[i].sadSum[j * stVdaData.u32MbWidth + k] += (*((HI_U16 *)pTmp + k));
                            	        }
                                    }    
                                }
                            	g_mdAttr[i].hBlock = stVdaData.u32MbHeight;
                            	g_mdAttr[i].wBlock = stVdaData.u32MbWidth;
                            	g_mdAttr[i].frameCount++;
                            }
                        	else
                            {
                            	if( 1 == ShouldAlarm(g_mdAttr[i].sadSum, i) )
                                {
                                	status[i] = 1;
                                	if( oldStatus[i] != status[i] )
                                    {                                        
                                    	ProconMdWrite( fdmd[i], dataPiece );
                                    	SVPrint( "ch(%d)=====>SEND MOVE ALARM!\r\n", i );
                                    }
                                }
                            	else
                                {
                                	status[i] = 0;
                                	if( oldStatus[i] != status[i] )
                                    {                                        
                                    	SVPrint( "ch(%d)=====>STOP MOVE ALARM!\r\n", i );
                                    }
                                }
                            	oldStatus[i] = status[i];
                                
                            	g_mdAttr[i].frameCount = 0;
                            	for( j = 0; j < stVdaData.u32MbHeight; j++ )
                                {
                                	for( k = 0; k < stVdaData.u32MbWidth; k++ )
                                    {
                                    	g_mdAttr[i].sadSum[j*stVdaData.u32MbWidth + k] = 0; 
                                    }    
                                }
                            }
                        }
                        else
                        {
                            SVPrint("is not valid\n");
                        }
                        /*release md data*/
                    	ret = HI_MPI_VDA_ReleaseData( VdaChn, &stVdaData );
                    	if( ret != 0 )
                        {
                        	SVPrint( "Md Release Data Err 0x%x\n", ret );
                        	continue;
                        }
                    }
                } 
            }        
        } 	    
    	for( i = 0; i < chNum; i++ )
        {
        	DisableMD(i);
            ProconMdClose(fdmd[i]);
        }
    } 
    
	ERRORPRINT( "!!!!!!!!!!!!!!!!!!!!!!%s stop!\r\n", __FUNCTION__ );
    
	return 0;
}


void StartMdThread()
{
	int ret;
	g_mdThreadRunFlag	= 1;
	g_mdTm.runFlag         = 1;
	ret = ThreadCreate( &g_mdTm.id, MdThread, NULL );
	if( 0!= ret )
    {        
    	g_mdTm.runFlag = 0;
    	ERRORPRINT( "error:ThreadCreate:%s\r\n", STRERROR_ERRNO );
    }
}

void StopMdThread()
{
	int ret;
	g_mdThreadRunFlag     = 0;
	g_mdTm.runFlag        = 0;
	MdSendMsgChangeParam();
	ret = ThreadJoin( g_mdTm.id, NULL );
	if( 0 != ret )
    {
    	SVPrint( "error:ThreadJoin:%s\r\n", STRERROR_ERRNO );
    }
}

void MdSendMsgChangeParam()
{
	g_mdCondition.Signal();
	MessageSend( MSG_ID_MD_PARAM_CHANGE );
}

