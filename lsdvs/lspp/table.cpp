/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此文件实现对哈希表基类，哈希表基类主要提供关键字和内容都是字符串
**            类型的数据进行插入，查找，删除和清除哈希表等操作。
**  date           :  2013.11.11
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "malloc.h"
#include "strSafe.h"
#include "table.h"

CTable::CTable()
{
	m_nHash = NHASH;
	m_ppTable = ( LP_TABLE * )Malloc( m_nHash * sizeof( LP_TABLE ) );
	if ( m_ppTable != NULL )
    {
    	for ( int i = 0; i < m_nHash; i++ )
        	m_ppTable[i] = NULL;
    }
	else
    {
    	m_nHash = 0;
    }
}

CTable::CTable( int nHash )
{
	m_nHash = nHash;
	if ( m_nHash < NHASH ) m_nHash = NHASH;

	m_ppTable = ( LP_TABLE * )Malloc( m_nHash * sizeof( LP_TABLE ) );
	if ( m_ppTable != NULL )
    {
    	for ( int i = 0; i < m_nHash; i++ )
        	m_ppTable[i] = NULL;
    }
	else
    {
    	m_nHash = 0;
    }    
}

CTable::~CTable()
{
	if ( m_ppTable != NULL )
    {
    	Clear();
    	Free( m_ppTable );
    	m_nHash = 0;
    }
}

/*
* 根据关键字查找哈希表返回元素的值，如果无符合关键字的元素返回NULL
*/
char *CTable::Get( char *pKey )
{
	LP_TABLE pTable = NULL;
    
	if ( m_ppTable != NULL && pKey != NULL )
    {
    	int idx = Hash( pKey );
    	for ( pTable = m_ppTable[idx]; pTable != NULL; pTable = pTable->next )
        {
        	if ( strcmp( pTable->key, pKey ) == 0 )
            	return pTable->val;
        }
    }

	return NULL;
}

/*
** 将关键字和值插入哈希表，如果关键字已经
** 存在返回1；否则插入成功返回0，插入失败返回-1。
*/
int CTable::Put( char *pKey, char *pVal )
{
	LP_TABLE pTable = NULL;
	int nRet = -1;

	if ( m_ppTable != NULL && pKey != NULL && pVal != NULL )
    {
    	int idx = Hash( pKey );
    	for ( pTable = m_ppTable[idx]; pTable != NULL; pTable = pTable->next )
        {
            /* 关键字已经存在 */
        	if ( strcmp( pTable->key, pKey ) == 0 )
            {
            	nRet = 1;
            	break;
            }
        }
        
    	if ( nRet != 1 )
        {
        	LP_TABLE pNew = ( LP_TABLE )Malloc( sizeof( TABLE ) );
        	if ( pNew != NULL )
            {
            	pNew->key = StrCopy( pKey );
            	if ( pNew->key == NULL )
                {
                	Free( pNew );
                }
            	else
                {
                	pNew->val = StrCopy( pVal );
                	if ( pNew->val == NULL )
                    {
                    	Free( pNew->key );
                    	Free( pNew );
                    }
                	else /* 插入成功 */
                    {
                    	pNew->next = m_ppTable[idx];
                    	m_ppTable[idx] = pNew;
                    	nRet = 0;
                    }
                }
            } // if ( pNew != NULL
        }
    }

	return nRet;
}

int CTable::Delete( char *pKey )
{
	int nRet = -1;
	LP_TABLE pTable = NULL;
    
	if ( m_ppTable != NULL && pKey != NULL )
    {
    	int idx = Hash( pKey );
    	pTable = m_ppTable[idx];
    	if ( pTable != NULL )
        {
        	if ( strcmp( pTable->key, pKey ) == 0 )
            {
            	m_ppTable[idx] = pTable->next;
            	Free( pTable->key );
            	Free( pTable->val );
            	Free( pTable );
            	nRet = 0;
            }        
        }
    	else
        {
        	LP_TABLE pPreTable = m_ppTable[idx];
        	LP_TABLE pCurTable = pPreTable->next;
        	while ( pCurTable != NULL )
            {
            	if ( strcmp( pCurTable->key, pKey ) == 0 )
                {
                	pPreTable->next = pCurTable->next;
                	Free( pCurTable->key );
                	Free( pCurTable->val );
                	Free( pCurTable );
                	nRet = 0;
                	break;
                }

            	pPreTable = pCurTable;
            	pCurTable = pPreTable->next;
            }
        }
    }
	return nRet;    
}

/*
** 清空哈希表
*/
void CTable::Clear()
{
	LP_TABLE pTable = NULL, pNext = NULL;
    
	if ( m_ppTable != NULL )
    {
    	for( int idx = 0; idx < m_nHash; idx++ )
        {
        	pTable = m_ppTable[idx];
        	while( pTable != NULL )
            {
            	pNext = pTable->next;
            	Free( pTable->key );
            	Free( pTable->val );
            	Free( pTable );
            	pTable = pNext;
            }
        	m_ppTable[idx] = NULL;
        }
    }
}

/* 字符串拷贝函数，动态分配内存保存拷贝的字符串 */
char *CTable::StrCopy( const char *pStr ) 
{
	int len = strlen( pStr );
	char *pBuf = ( char * )Malloc( sizeof( char ) * ( len + 1 ) );
	if ( pBuf != NULL ) strlcpy( pBuf, pStr, len + 1 );
	return pBuf;
}

/* 计算关键字的哈希值 */
int CTable::Hash( char *pKey )
{
	unsigned int h = 0;
	unsigned char *pCh = NULL;

	for ( pCh = (unsigned char *)pKey; *pCh != '\0'; pCh++ )
    	h = MULTIPLTER * h + *pCh;

	return h % (unsigned int)m_nHash;
}

#ifdef _TABLE_DEBUG

/* 遍历哈希表，计算元素个数 */    
int CTable::Size()
{
	LP_TABLE pTable = NULL;
	int cnt = -1;
    
	if ( m_ppTable != NULL )
    {
    	cnt = 0;
    	for( int idx = 0; idx < m_nHash; idx++ )
        {
        	pTable = m_ppTable[idx];
        	while ( pTable != NULL )
            {
            	cnt++;        
            	pTable = pTable->next;
            }
        }
    }
    
	return cnt;
}

/* 遍历哈希表，打印出各个元素 */
void CTable::Print( FILE *fp )
{
	if ( m_ppTable != NULL && fp != NULL )
    {
    	for ( int idx = 0; idx < m_nHash; idx++ )
        {
        	LP_TABLE pTable = m_ppTable[idx];
        	if ( pTable != NULL )
            {
            	fprintf( fp, "%d:|", idx );
            	for ( ; pTable != NULL; pTable = pTable->next )
                {
                	fprintf( fp, "%s->%s|", pTable->key, pTable->val );    
                }
            	fprintf( fp, "\r\n" );
            }
        }
    }

	fflush( fp );
}

#endif

