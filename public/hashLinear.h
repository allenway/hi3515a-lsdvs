#ifndef __HASHLINEAR_H__
#define __HASHLINEAR_H__

#include "mutex.h"

typedef struct _HashLinearSt_ 
{
	unsigned int key;
	void *pVal;
	struct _HashLinearSt_ *next;

} HASH_LINEAR_T;

class CHashLinear    
{
public:
	CHashLinear( int maxNode );
    ~CHashLinear();        
	int insert( unsigned int key, void *pVal, int valSize );
	int remove( unsigned int key );
	void *GetVal( unsigned int key );     
	void FreeVal( void *pVal );
private:
	void FreeNode( HASH_LINEAR_T *pNode );
private:
	HASH_LINEAR_T **m_ppTable;
	int		m_maxNode;
	CRwLock mutex;
};

unsigned int LinearHash( unsigned int key, unsigned int max );

#endif  


