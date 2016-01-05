/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.09.16
**  description  : 带数据缓冲池的pcp, 支持一个生产者->多个消费者的类
    pcp: producer consumer
    pcp: producer consumer pool
********************************************************************************/

#include "pcp.h"
#include "malloc.h"
#include "debug.h" 

/**************************************************************************
* chNum: 最多支持多少个通道
* blockFlag: 0, 非阻塞; 1, 阻塞;
* maxSize: 每个节点的最大;
* dataType: 数据类型(DATA_TYPE_EN)
*****************************************************************************/
CPcp::CPcp( int blockFlag, int dataType )
{
	m_blockFlag = blockFlag;
	m_dataType	= dataType;    
}

CPcp::~CPcp()
{
	int i;    
	PCP_T *pcp;

	if( NULL != m_pcp )
    {
    	for( i = 0; i< m_chNum; ++i )
        {
        	pcp = m_pcp + i;
        	pthread_mutex_destroy( &pcp->dataMutex );        
        }
    }
}

void CPcp::Init( int chNum, int dataNode, uint maxSize )
{
	int i, j;
	PCP_T *pcp;
	PCP_NODE_T *pcpNode;
	pthread_mutexattr_t mutexattr;
    
	m_chNum = chNum;
	m_maxPcpDataNodeSize = maxSize;
	m_dataNode = dataNode;
    
	m_pool = (PCP_POOL_T *)Malloc( sizeof(PCP_POOL_T) * m_chNum );
	if( NULL != m_pool )
    {
    	memset( m_pool, 0x00, sizeof(PCP_POOL_T) * m_chNum );
    	for( i = 0; i < m_chNum; ++i )
        {
        	for( j = 0; j < m_dataNode + ASSUME_CONSUMER_SIZE; ++j ) 
            {
            	pcpNode = (PCP_NODE_T *)Malloc( sizeof(PCP_NODE_T) + 
                                        	m_maxPcpDataNodeSize - 1 );
            	if( NULL != pcpNode )
                {
                	pcpNode->pcpHead.users         = 0;
                	pcpNode->pcpHead.channel     = i;
                	pcpNode->pcpHead.type         = m_dataType;
                	pcpNode->next	            = NULL;
                	if( NULL == m_pool[i].tail )
                    {
                    	m_pool[i].tail = pcpNode;
                    	m_pool[i].head = pcpNode;
                    }
                	else
                    {
                    	m_pool[i].tail->next = pcpNode;
                    	m_pool[i].tail = pcpNode;
                    }
                }
            } // for( j = 0; j < m_dataNode
        } // for( i = 0; i < m_chNum;
    } // if( NULL != m_pool
    
	m_pcp = (PCP_T *)Malloc( sizeof(PCP_T) * m_chNum );
	if( NULL != m_pcp )
    {        
    	pthread_mutexattr_init( &mutexattr );
    	pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_DEFAULT );
    	for( i = 0; i < m_chNum; ++i )
        {
        	pcp = m_pcp + i;
            
        	pcp->writePos = 0;            
        	pthread_mutex_init( &pcp->dataMutex, &mutexattr );
        	for( j = 0; j < m_dataNode; ++j )
            {
            	pcp->dataPool[j] = NULL;
            }            
        }        
    	pthread_mutexattr_destroy( &mutexattr );
    } // if( NULL != m_pcp
}

/***************************************************************************
* function: 打开一个pcp 句柄
* channel: 所打开的通道
* flag     : 方式(OPEN_RDONLY / OPEN_WRONLY)
* return : 0, 打开失败; 大于0, 成功
*****************************************************************************/
uint CPcp::Open( int channel, int flag )
{
	uint ret = 0;
    
	if( 0 <= channel && channel < m_chNum )
    {
    	if( NULL != m_pcp )
        {
        	if( OPEN_RDONLY == flag )
            {
            	PCP_READ_T *pRead = (PCP_READ_T *)Malloc( sizeof(PCP_READ_T) );
            	if( NULL != pRead )
                {
                	pRead->channel	    = channel;
                	pRead->flag         = flag;
                	pRead->readBegin	= GetWritePos( channel );
                	pRead->readEnd	    = pRead->readBegin;
                	ret = (uint)pRead;
                }
            }
        	else if( OPEN_WRONLY == flag )
            {
            	PCP_WRITE_T *pWrite = (PCP_WRITE_T *)Malloc( sizeof(PCP_WRITE_T) );
            	if( NULL != pWrite )
                {
                	pWrite->channel = channel;
                	pWrite->flag	= flag;
                	ret = (uint)pWrite;
                }
            }
        } // if( NULL != m_pcp
    } // if( 0 <= channel && channel
    
	return ret;
}

/***********************************************************************
* function: 关闭PcpOpen 所打开的句柄
* fd: PcpOpen 的返回值
*************************************************************************/
void CPcp::Close( uint fd )
{
	void *ptr = (void *)fd;
	Free( ptr ) ;
}

/******************************************************************************
* function: 从数据缓冲池获取一个节点的数据
* fd: PcpOpen() 以只读方式打开的的句柄
* return  : 如果成功,则返回获取节点的指针; 否则, 返回NULL;
*******************************************************************************/
PCP_NODE_T *CPcp::Read( uint fd )
{
	PCP_T *pcp;
	int readBegin, readEnd, writePos;
	PCP_NODE_T *pPcpNode = NULL;    
	PCP_READ_T *pRead =( PCP_READ_T *)fd;
    
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
	if( NULL != m_pcp )
    {
    	pcp = m_pcp + pRead->channel; 

    	pthread_mutex_lock( &pcp->dataMutex );    
    	if ( pRead->readBegin == pRead->readEnd && pRead->readEnd == pcp->writePos )
        {            
        	pthread_mutex_unlock( &pcp->dataMutex );
        	return NULL;
        }

    	readBegin	= pRead->readBegin;
    	readEnd     = pRead->readEnd;
    	writePos	= pcp->writePos;
        
    	if( (readBegin <= readEnd && readEnd <= writePos)
            || (readEnd <= writePos && writePos <= readBegin)
            || (writePos <= readBegin && readBegin <= readEnd) )
        {
        	pRead->readEnd	= writePos;
        	pPcpNode         = Tcopy( pcp->dataPool[readBegin] );        
        	pRead->readBegin = (readBegin + 1) % m_dataNode;
        }
    	else
        {
        	SVPrint( "failed:lost frame,rb(%d),re(%d),wp(%d)!\r\n",
                                    	readBegin,readEnd,writePos);
        	pRead->readEnd	= writePos;
        	pRead->readBegin = writePos;
        }
    	pthread_mutex_unlock(&pcp->dataMutex);        
    } // end if( NULL != m_pcp
    
	return pPcpNode;
}

/*****************************************************************
* fn: 往从数据缓冲池添加一个节点的数据
* fd: PcpOpen() 以只写方式打开的的句柄
********************************************************************/
int CPcp::Write( uint fd, DATA_PIECE_T proDataInfo )
{
	uint	allConSize = 0; 
	int 	ret = -1;
	int 	i, writePos, offset;
	PCP_T     *pcp;    
	PCP_WRITE_T *pWrite =( PCP_WRITE_T *)fd;

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
	if( NULL != m_pcp )
    {
    	pcp = m_pcp + pWrite->channel;    

    	for( i = 0; i < proDataInfo.count; i++ )
        {
        	allConSize += proDataInfo.len[i];
        }    
    	if( allConSize > m_maxPcpDataNodeSize )    
        {
        	allConSize = m_maxPcpDataNodeSize; 
        }    
            
    	pthread_mutex_lock( &pcp->dataMutex );
        
    	writePos = pcp->writePos;
    	Tfree( pcp->dataPool[writePos] );    
    	pcp->dataPool[writePos] = Talloc( pWrite->channel );
    	if( NULL != pcp->dataPool[writePos] )
        {
        	pcp->dataPool[writePos]->pcpHead.len = allConSize;
                    
        	offset = 0;
        	for( i = 0; i < proDataInfo.count; ++i )
            {
            	if ( offset + proDataInfo.len[i] > allConSize )
                {
                	memcpy( pcp->dataPool[writePos]->data + offset, proDataInfo.buf[i], 
                                        	allConSize - offset - proDataInfo.len[i] );
                	break;
                }
            	else
                {
                	memcpy( pcp->dataPool[writePos]->data + offset, 
                            	proDataInfo.buf[i], proDataInfo.len[i] );    
                }
            	offset += proDataInfo.len[i];
            }
        	writePos = (writePos + 1) % m_dataNode;
        	pcp->writePos = writePos;
        	ret = 0;
        }    
    	else
        {
        	SVPrint( "pcp pool is all in used, pelease check!\r\n" );
        }
    	pthread_mutex_unlock( &pcp->dataMutex );
    } 

	return ret;
}

/*******************************************************************************
* fn: 往从数据缓冲池添加一个节点的数据
* fd: PcpOpen() 以只写方式打开的的句柄
* pExNode: Talloc() 的返回值
*********************************************************************************/
int CPcp::Write( uint fd, PCP_NODE_T *pExNode )
{
	int 	ret = -1;
	int 	writePos;
	PCP_T     *pcp;    
	PCP_WRITE_T *pWrite =( PCP_WRITE_T *)fd;

	if( NULL == pWrite || NULL ==  pExNode )
    {
    	SVPrint( "error:NULL == pWrite || NULL ==  pExNode!\r\n" );
    	return -1;
    }    
	if( OPEN_WRONLY != pWrite->flag )
    {
    	SVPrint( "PCP_OPEN_WRONLY != pWrite->flag, please check!\r\n" );
    	return -1;
    }
	if( NULL != m_pcp )
    {
    	pcp = m_pcp + pWrite->channel;            
    	pthread_mutex_lock( &pcp->dataMutex );
        
    	writePos = pcp->writePos;
    	Tfree( pcp->dataPool[writePos] );    
    	pcp->dataPool[writePos] = pExNode;
    	if( NULL != pcp->dataPool[writePos] )
        {    
        	writePos         = (writePos + 1) % m_dataNode;
        	pcp->writePos     = writePos;
        	ret = 0;
        }    
        
    	pthread_mutex_unlock( &pcp->dataMutex );
    } 

	return ret;
}

PCP_NODE_T *CPcp::Talloc( int channel )
{
	PCP_NODE_T *pres = NULL;

	m_lock.Lock();
	if( NULL != m_pool && NULL != m_pool[channel].head )
    {
    	pres = m_pool[channel].head;
    	m_pool[channel].head = m_pool[channel].head->next;
    	if( NULL == m_pool[channel].head )
        {
        	m_pool[channel].tail = NULL;
        }
    	pres->next = NULL;
    	pres->pcpHead.users = 1;
    	pres->pcpHead.len	= 0;
    	pres->pcpHead.channel = channel;
    }
	m_lock.Unlock();

	return pres;
}

PCP_NODE_T *CPcp::Tcopy( PCP_NODE_T *pcpNode )
{
	PCP_NODE_T *pres = NULL;
    
	m_lock.Lock();
	if( NULL != pcpNode )
    {
    	pcpNode->pcpHead.users++;        
    	pres = pcpNode;
    }
	m_lock.Unlock();

	return pres;
}

/***************************************************************************
* fn: 释放read 到的资源
* 如果是最后一次调用, 那么把该节点插回到生产者队列
****************************************************************************/
void CPcp::Tfree( PCP_NODE_T *pcpNode )
{        
	m_lock.Lock();
	if( NULL != pcpNode )
    {
    	if( pcpNode->pcpHead.users > 1 ) 
        {
        	pcpNode->pcpHead.users--;
        }
    	else // 当前只有一个用户在用了
        {
        	pcpNode->pcpHead.users = 0;
        	if( NULL == m_pool[pcpNode->pcpHead.channel].tail )
            {
            	m_pool[pcpNode->pcpHead.channel].head = pcpNode;
            	m_pool[pcpNode->pcpHead.channel].tail = pcpNode;
            }
        	else
            {
            	m_pool[pcpNode->pcpHead.channel].tail->next = pcpNode;
            	m_pool[pcpNode->pcpHead.channel].tail	       = pcpNode;
            }
        }
    } 
	m_lock.Unlock();
}

/**********************************************************
* fn: 获取当前写的位置
**********************************************************/
int CPcp::GetWritePos( int channel )
{
	int ret = 0;    
	PCP_T *pcp;

	if( NULL != m_pcp )
    {
    	pcp = m_pcp + channel;        
    	ret = pcp->writePos;         
    }

	return ret;
}



