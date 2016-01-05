/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.10.10
**  description  : 当key是线性,并单调递增的时候使用该hash 表类
********************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "debug.h" 
#include "malloc.h" 
#include "hashLinear.h" 

/**************************************************************************
* fn: 字符形式的hash算法封装, H = H(key)
* key: 用来计算hash 值的关键字
* max: hash的成员格数,index的个数
* 返回: 由key 经过算法产生的hash值,
****************************************************************************/
unsigned int LinearHash( unsigned int key, unsigned int max )
{
	return key % max; 
}

CHashLinear::CHashLinear( int n )
{
	int tableSize;
    
	m_maxNode      = n;
	tableSize	= sizeof(m_ppTable) * m_maxNode;
	m_ppTable     = (HASH_LINEAR_T **)malloc( tableSize );    
	if( NULL != m_ppTable )
    {
    	memset( m_ppTable, 0x00, tableSize );
    }
	else
    {
    	m_maxNode = 0;
    }
}

CHashLinear::~CHashLinear()
{
	int index;
	HASH_LINEAR_T *pTable, *pNext;
    
	if ( m_ppTable != NULL )
    {
    	for( index = 0; index < m_maxNode; index++ )
        {            
        	pTable = m_ppTable[index];
        	while( pTable != NULL )
            {
            	pNext = pTable->next;
            	FreeNode( pTable );                
            	pTable = pNext;
            }
        	m_ppTable[index] = NULL;
        }

    	Free( m_ppTable );
    }    
}

void CHashLinear::FreeNode( HASH_LINEAR_T *pNode )
{
	if( NULL != pNode )
    {        
    	FreeVal( pNode->pVal );
    	Free( pNode );
    }
}

/********************************************************************
* fn: 往linear hash 插入一个节点
* key: 关键字
* pVal: 插入结构体的地址
* valSize: 插入的大小
*********************************************************************/
int CHashLinear::insert( unsigned int key, void *pVal, int valSize )
{
	int ret = -1, index;
	HASH_LINEAR_T *pTable;
    
	mutex.WriteLock();
	if ( NULL != m_ppTable && NULL != pVal )
    {
    	index = LinearHash( key, m_maxNode );
    	for( pTable = m_ppTable[index]; pTable != NULL; pTable = pTable->next ) //查看是否已经纯在该key
        {
        	if( pTable->key == key )
            {
            	ShareFree( pTable->pVal );  // 释放旧的val
            	pTable->pVal = pVal;        // 更新到新的val
            	ret = 0;
            	break;
            }
        }
    	if( -1 == ret ) // 哈希表里面不存在该key,则插入
        {
        	pTable = (HASH_LINEAR_T *)malloc( sizeof(HASH_LINEAR_T) );
        	if( NULL != pTable )
            {
            	pTable->key  = key;                
            	pTable->pVal = pVal;        
            	pTable->next = m_ppTable[index]; // 解决冲突
            	m_ppTable[index] = pTable;
            	ret = 0;                            
            } // if( NULL != pTable
        }
    } // if ( NULL != m_ppTable && NULL != key 
    
	mutex.Unlock();

	return ret;
}

/************************************************************************
* fn: 获取指定key 的节点的值所指向的地址
* 返回: 如果key 存在,则返回值的地址; 否则返回 NULL
* 说明: 该函数返回的结果要用本类的 FreeVal 释放
************************************************************************/
void *CHashLinear::GetVal( unsigned int key )
{
	HASH_LINEAR_T *pTable;
	int index;
	void *pRet;

    //FiPrint2( "%s, key(%u)!\r\n", __FUNCTION__, key );

	mutex.ReadLock();    
	pRet = NULL;
	if ( m_ppTable != NULL )
    {
    	index = LinearHash( key, m_maxNode );        
        
    	for ( pTable = m_ppTable[index]; pTable != NULL; pTable = pTable->next )
        {
            //FiPrint2( "%s, key(%u), pTable->key(%u)!\r\n", __FUNCTION__, key, pTable->key );
        	if ( pTable->key == key )
            {
            	pRet = ShareCopy( pTable->pVal );
            }
        }
    }
	mutex.Unlock();
    
	return pRet;
}

/*******************************************************************
* fn: 移除指定key 的节点
*********************************************************************/
int CHashLinear::remove( unsigned int key )
{
	int ret = -1, index;
	HASH_LINEAR_T *pPreTable, *pCurTable;
    
	mutex.WriteLock();
	if ( m_ppTable != NULL )
    {
    	index = LinearHash( key, m_maxNode );    
    	if( NULL != m_ppTable[index] )
        {
        	pPreTable = m_ppTable[index];
        	pCurTable = pPreTable->next;
            
        	if ( m_ppTable[index]->key == key ) // 头节点的key就等于要删除节点的key
            {
            	pCurTable = m_ppTable[index];
            	m_ppTable[index] = m_ppTable[index]->next;
            	FreeNode( pCurTable );            
            	ret = 0;
            }        
        	else
            {
            	pPreTable = m_ppTable[index];
            	pCurTable = pPreTable->next;
            	while ( pCurTable != NULL )
                {
                	if ( pCurTable->key == key )
                    {
                    	pPreTable->next = pCurTable->next;
                    	FreeNode( pCurTable );
                    	ret = 0;
                    	break;
                    }

                	pPreTable = pCurTable;
                	pCurTable = pCurTable->next;
                }
            }
        } // if( NULL != m_ppTable[index] 
    } // if ( m_ppTable != NULL && key != NULL
    
	mutex.Unlock();
    
	return ret;    
}

void CHashLinear::FreeVal( void *pVal )
{
	ShareFree( pVal );
}

