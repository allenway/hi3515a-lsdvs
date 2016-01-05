/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 支持一个生产者->多个消费者的类
                        目前只支持非阻塞的方式,但设计时考虑了阻塞
                        的扩展
                        procon: producer&consumer
********************************************************************************/


#include "procon.h"
#include "malloc.h"
#include "debug.h"

CProcon::CProcon( int blockFlag, int dataType )
{    
	m_blockFlag = blockFlag;
	m_dataType	= dataType;
}

void CProcon::Init( int chNum, int dataNode )
{
	int i, j;
	PROCON_T *procon;
	pthread_mutexattr_t mutexattr;

	m_chNum     = chNum;
	m_dataNode  = dataNode;
    
	m_procon = (PROCON_T *)Malloc( sizeof(PROCON_T) * m_chNum );
	if( NULL != m_procon )
    {        
    	pthread_mutexattr_init( &mutexattr );
    	pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_DEFAULT );
    	for( i = 0; i < m_chNum; ++i )
        {
        	procon = m_procon + i;
            
        	procon->writePos = 0;            
        	pthread_mutex_init( &procon->dataMutex, &mutexattr );
        	for( j = 0; j < m_dataNode; ++j )
            {
            	procon->dataPool[j] = NULL;
            }            
        }        
    	pthread_mutexattr_destroy( &mutexattr );
    }
}

CProcon::~CProcon()
{
	int i, j;    
	PROCON_T *procon;

	if( NULL != m_procon )
    {
    	for( i = 0; i< m_chNum; ++i )
        {
        	procon = m_procon + i;
            	pthread_mutex_destroy( &procon->dataMutex );    
            	for( j = 0; j < m_dataNode; ++j )
                {
            	Tfree( procon->dataPool[j] );
                }
           }
       }
}

/*
* fn: 打开一个procon 句柄
* channel: 所打开的通道
* flag: 方式(OPEN_RDONLY / OPEN_WRONLY)
* 返回: 0, 打开失败; 大于0, 成功
*/
uint CProcon::Open( int channel, int flag )
{
	uint ret = 0;    

	if( 0 <= channel && channel < m_chNum )
    {
    	if( NULL != m_procon )
        {//SVPrint("NULL != m_procon\n");
        	if( OPEN_RDONLY == flag )
            {
            	PROCON_READ_T *pRead = (PROCON_READ_T *)Malloc( sizeof(PROCON_READ_T) );
            	if( NULL != pRead )
                {
                	pRead->channel         = channel;
                	pRead->flag	        = flag;
                	pRead->readBegin     = GetWritePos( channel );
                	pRead->readEnd	     = pRead->readBegin;
                	ret	= (uint)pRead;
                }
            }
        	else if( OPEN_WRONLY == flag )
            {
            	PROCON_WRITE_T *pWrite = (PROCON_WRITE_T *)Malloc( sizeof(PROCON_WRITE_T) );
            	if( NULL != pWrite )
                {//SVPrint("success!!!!!\n");
                	pWrite->channel = channel;
                	pWrite->flag	= flag;
                	ret	= (uint)pWrite;
                }
                else
                {
                    ERRORPRINT("malloc failed\n");
                }
            }
        } 
    	else
        {
            ERRORPRINT("NULL == m_procon\n");
        }      
    } 
	else
    {
        ERRORPRINT("channel is :%d,m_chNum is :%d\n",channel,m_chNum);
    }   
	return ret;
}

/*
* fn: 关闭ProconOpen 所打开的句柄
* fd: ProconOpen 的返回值
*/
void CProcon::Close( uint fd )
{
	void *ptr = (void *)fd;
	Free( ptr ) ;
}

/*
* fn: 从数据缓冲池获取一个节点的数据
* fd: ProconOpen() 以只读方式打开的的句柄
* 返回: 如果成功,则返回获取节点的指针; 否则, 返回NULL;
*/
PROCON_NODE_T *CProcon::Read( uint fd )
{
	PROCON_T *procon;
	int readBegin, readEnd, writePos;
	PROCON_NODE_T *pProconNode = NULL;    
	PROCON_READ_T *pRead =( PROCON_READ_T *)fd;

	if( NULL == pRead )
    {
    	SVPrint( "error:NULL == pRead!\r\n" );
    	return NULL;
    }
    
	if( OPEN_RDONLY != pRead->flag )
    {
    	SVPrint( "failed:PCP_OPEN_RDONLY != pRead->flag!\r\n" );
    	return NULL;
    }
	if( NULL != m_procon )
    {
    	procon = m_procon + pRead->channel;    

    	pthread_mutex_lock( &procon->dataMutex );    
    	if ( pRead->readBegin == pRead->readEnd && pRead->readEnd == procon->writePos )
        {            
        	pthread_mutex_unlock( &procon->dataMutex );
        	return NULL;
        }

    	readBegin	= pRead->readBegin;
    	readEnd     = pRead->readEnd;
    	writePos	= procon->writePos;
        
    	if( (readBegin <= readEnd && readEnd <= writePos)
            || (readEnd <= writePos && writePos <= readBegin)
            || (writePos <= readBegin && readBegin <= readEnd) )
        {
        	pRead->readEnd     = writePos;
        	pProconNode     = (PROCON_NODE_T *)ShareCopy( procon->dataPool[readBegin] );        
        	pRead->readBegin = (readBegin + 1) % m_dataNode;
        }
    	else
        {
        	SVPrint( "failed:lost frame,rb(%d),re(%d),wp(%d)!\r\n",
                                    	readBegin,readEnd,writePos);
        	pRead->readEnd     = writePos;
        	pRead->readBegin = writePos;
        }
    	pthread_mutex_unlock(&procon->dataMutex);        
       }
       
	return pProconNode;
}

/*
* fn: 往从数据缓冲池添加一个节点的数据
* fd: ProconOpen() 以只写方式打开的的句柄
* proDataInfo: 指向数据的指针,里面指针的数据将会被复制到
	ShareMalloc 出来的内存。
*/
int CProcon::Write( uint fd, DATA_PIECE_T proDataInfo )
{
	uint 	allConSize = 0;    
	int 	ret = -1;
	int 	i, writePos, offset;
	PROCON_T *procon;    
	PROCON_WRITE_T *pWrite =( PROCON_WRITE_T *)fd;
    
	if( NULL == pWrite )
    {
    	ERRORPRINT( "error:NULL == pWrite!\r\n" );
    	return -1;
    }
	if( OPEN_WRONLY != pWrite->flag )
    {
    	ERRORPRINT( "PCP_OPEN_WRONLY != pWrite->flag, please check!\r\n" );
    	return -1;
    }
	if( NULL != m_procon )
    {
    	procon = m_procon + pWrite->channel;    

    	for( i = 0; i < proDataInfo.count; i++ )
        {
        	allConSize += proDataInfo.len[i];
        }    
    	if( allConSize > MAX_PROCON_DATA_NODE_SIZE )    
        {
        	allConSize = MAX_PROCON_DATA_NODE_SIZE;    
        }    
            
    	pthread_mutex_lock( &procon->dataMutex );
        
    	writePos = procon->writePos;
    	ShareFree( procon->dataPool[writePos] );    
    	procon->dataPool[writePos] = (PROCON_NODE_T *)ShareMalloc( allConSize + sizeof(PROCON_HEAD_T) );
    	if( NULL != procon->dataPool[writePos] )
        {
        	procon->dataPool[writePos]->proconHead.len     = allConSize;
        	procon->dataPool[writePos]->proconHead.type	   = m_dataType;
        	procon->dataPool[writePos]->proconHead.nalInfo = proDataInfo.nalInfo;
        	offset = 0;
        	for( i = 0; i < proDataInfo.count; ++i )
            {
            	if ( offset + proDataInfo.len[i] > allConSize )
                {
                	memcpy( procon->dataPool[writePos]->data + offset, proDataInfo.buf[i], 
                                        	allConSize - offset );
                	break;
                }
            	else
                {
                	memcpy( procon->dataPool[writePos]->data + offset, 
                            	proDataInfo.buf[i], proDataInfo.len[i] );    
                }
            	offset += proDataInfo.len[i];
            }
        	writePos = (writePos + 1) % m_dataNode;
        	procon->writePos = writePos;
        	ret = 0;
        }    
        else
        {
            ret = -1;
            ERRORPRINT("ShareMalloc failed \n");
        }
    	pthread_mutex_unlock( &procon->dataMutex );
    } 
	return ret;
}

/*
* fn: 把已经分配好的节点插入队列
* fd: ProconOpen() 以只写方式打开的的句柄
* pExNode: 用ShareMalloc() 分配好的节点
*/
int CProcon::Write( uint fd, PROCON_NODE_T *pExNode )
{
	int 	ret = -1;
	int 	writePos;
	PROCON_T *procon;    
	PROCON_WRITE_T *pWrite =( PROCON_WRITE_T *)fd;
    
	if( NULL == pWrite )
    {
    	SVPrint( "error:NULL == pWrite!\r\n" );
    	return -1;
    }
	if( OPEN_WRONLY != pWrite->flag )
    {
    	SVPrint( "PCP_OPEN_WRONLY != pWrite->flag, please check!\r\n" );
    	return -1;
    }
	if( NULL != m_procon )
    {
    	procon = m_procon + pWrite->channel;                
    	pthread_mutex_lock( &procon->dataMutex );
        
    	writePos = procon->writePos;
    	ShareFree( procon->dataPool[writePos] );    
    	procon->dataPool[writePos] = pExNode; //(PROCON_NODE_T *)ShareMalloc( allConSize + sizeof(PROCON_HEAD_T) );
    	if( NULL != procon->dataPool[writePos] )
        {
        	writePos = (writePos + 1) % m_dataNode;
        	procon->writePos = writePos;
        	ret = 0;
        }        
    	pthread_mutex_unlock( &procon->dataMutex );
    }

	return ret;
}


/*
* fn: 释放read 到的资源
*/
void CProcon::Tfree( PROCON_NODE_T *proconNode )
{
	ShareFree( proconNode );
}

/*
* fn: 获取当前写的位置
*/
int CProcon::GetWritePos( int channel )
{
	int ret = 0;    
	PROCON_T *procon;

	if( NULL != m_procon )
    {
    	procon = m_procon + channel;        
        	ret = procon->writePos;                
       }

   	return ret;
}


