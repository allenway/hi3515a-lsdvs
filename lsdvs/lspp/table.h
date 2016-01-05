/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此头文件提供哈希表基类的接口。
**  date           :  2013.11.11
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#ifndef _TABLE_H
#define _TABLE_H

#include <stdio.h>

#define _TABLE_DEBUG

/*
** 哈希表大小NHASH应该是下面质数primes[]中其中一个
** int primes[] = { 509, 1021, 2053, 4093, 8191, 16381, 32771, 65521 };
*/
const int NHASH = 509;
const int MULTIPLTER = 37;

typedef struct table 
{
	char *key;
	char *val;
	struct table *next;

} TABLE, *LP_TABLE;

class CTable
{
private:
	int m_nHash;
	LP_TABLE *m_ppTable;
	char *StrCopy( const char *pStr );    
	int Hash( char *pKey );
    
public:
	CTable();
	CTable( int nHash );
    ~CTable();
	char *Get( char *pKey );
	int Put( char *pKey, char *pVal );
	int Delete( char *pKey );    
	void Clear();

#ifdef _TABLE_DEBUG
	int Size();    
	void Print( FILE *fp );
#endif
};

#endif  // _TABLE_H

