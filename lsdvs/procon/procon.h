#ifndef __PROCON_H__
#define __PROCON_H__

#include <pthread.h>
#include "const.h"
#include "ttypes.h"


class CProcon
{
public:
	CProcon( int blockFlag = BLOCK_NO, int dataType = DATA_TYPE_NOMAL );
    ~CProcon();
	void Init( int chNum = 4, int dataNode = 64 );
	uint Open( int channel, int flag );
	void Close( uint fd );
	PROCON_NODE_T *Read( uint fd );
	int Write( uint fd, DATA_PIECE_T proDataInfo );    
	int Write( uint fd, PROCON_NODE_T *pExNode );
	void Tfree( PROCON_NODE_T *proconNode );
private:    
	int GetWritePos( int channel );
    
private:
	PROCON_T *m_procon;
	int	m_chNum;
	int m_dataNode;
	int m_blockFlag;
	int m_dataType;
};

#endif
