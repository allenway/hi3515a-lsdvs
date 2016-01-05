#ifndef __PCP_H__
#define __PCP_H__

#include <pthread.h>
#include "const.h"
#include "mutex.h"
#include "ttypes.h"

#define	MAX_PCP_DATA_NODE		16	// 一个通道最多缓存16 个节点
#define ASSUME_CONSUMER_SIZE	1	// 假定有2 个消费者消费速度比较慢,保证有8 个可用的节点

typedef struct _PcpHead_
{
	int users;        // 当前有多少个用户正在使用该节点
	int channel;    // 本节点的数据是属于哪个通道的
	int type;        // 数据类型,DATA_TYPE_EN 中的其中一种
	int len;        // 节点数据的大小	
} PCP_HEAD_T;

typedef struct _PcpNode_
{
	PCP_HEAD_T         	pcpHead;
	struct _PcpNode_     *next;
	char             	data[1];
} PCP_NODE_T;

typedef struct _PcpSt_
{
	int	            	writePos;
	pthread_mutex_t		dataMutex;
	PCP_NODE_T	        *dataPool[MAX_PCP_DATA_NODE];
} PCP_T;

typedef struct PcpReadSt
{
	int flag;
	int channel;
	int readBegin;
	int readEnd;
} PCP_READ_T;

typedef struct PcpWriteSt
{
	int flag;
	int channel;
} PCP_WRITE_T;

typedef struct _PcpPool_
{
	PCP_NODE_T *head;
	PCP_NODE_T *tail;
} PCP_POOL_T;

class CPcp
{
public:    
	CPcp( int blockFlag = BLOCK_NO, int dataType = DATA_TYPE_NOMAL );
    ~CPcp();    
	void Init( int chNum, int dataNode, uint maxSize );
	uint Open( int channel, int flag );
	void Close( uint fd );
	PCP_NODE_T *Read( uint fd );
	int Write( uint fd, DATA_PIECE_T proDataInfo );    
	int Write( uint fd, PCP_NODE_T *pExNode );
	void Tfree( PCP_NODE_T *pcpNode );    
	PCP_NODE_T *Talloc( int channel );    
private:    
	PCP_NODE_T *Tcopy( PCP_NODE_T *pcpNode );
	int GetWritePos( int channel );    
private:
	PCP_T *m_pcp;
	int	m_chNum;
	int	m_dataType;
	int m_blockFlag;
	int m_dataNode;
	uint m_maxPcpDataNodeSize;
	PCP_POOL_T *m_pool;     // producer list, 生产者列
	ClMutexLock m_lock;
};

#endif

