#if 0
#include <unistd.h>
#include <stdio.h>
#include "debug.h"
#include "const.h"
#include "ttypes.h"
#include "mpiApp.h"
#include "proconH264.h"
#include "hton.h"

extern int StcPutStreamFrame( int channel, int type, int frameType, 
    	unsigned char *pAvData, unsigned int len, int frameRate );


typedef struct _TestProconH264Read_
{
	char flag[4];
	int frameType;    
	int len;
} TEST_PHRS_T;

#if 1 //
#define HI3512_RECORD_FILE_2 "./ch0_2.h264"
static void SaveH2642( char *pData0, int len0 )
{
	static FILE	*fp_record = NULL;
    
	if( NULL == fp_record )
    {
    	fp_record = fopen( HI3512_RECORD_FILE_2, "w+b" );
    	if(fp_record == NULL)
        {
        	SVPrint("fopen failed:%s!\r\n",STRERROR_ERRNO);
        	return;
        }
    }                            

	if( len0 > 0 )
    {
    	if( fwrite( pData0, len0, 1, fp_record) != 1 )
        {
        	perror( "fi_record write1" );
        }
    }
}
#endif

void TestProconH264Read()
{
	int i;
	PROCON_NODE_T *proconH264Node;
	static uint getLenTotal[REAL_CHANNEL_NUM]     = { 0, 0, 0, 0 };
	static uint proconH264Fd[REAL_CHANNEL_NUM]     = { 0, 0, 0, 0 };
	static uint getNotCount = 0;

	static TEST_PHRS_T saveHead;

	int localFrameLen;

    
	saveHead.flag[0] = 'B';
	saveHead.flag[1] = 'C';
	saveHead.flag[2] = 'D';
	saveHead.flag[3] = 'F';
    

	for( i = 0; i < REAL_CHANNEL_NUM; ++i )
    {
    	if( 0 == proconH264Fd[i] )
        {
        	proconH264Fd[i] = ProconH264Open( i, OPEN_RDONLY );
        }

    	proconH264Node = ProconH264Read( proconH264Fd[i] );
    	if( NULL != proconH264Node )
        {
        	getNotCount = 0;
        	localFrameLen = Ntohl(pFraemHead->videoHead.videoSize);
        	getLenTotal[i] += localFrameLen;
            //SVPrint( "now ch(%d) getLenTotal[i] = %u, thisFrame size = %u Bytes!\r\n", 
            //        	i, getLenTotal[i], localFrameLen );
            //if( 0 == i )
            {
            	saveHead.len         = localFrameLen;
            	saveHead.frameType     = pFraemHead->videoHead.videoType;
                //SaveH2642( (char *)&saveHead, sizeof(saveHead) );
                //SaveH2642( proconH264Node->data + sizeof(*pFraemHead), localFrameLen );
                //StcPutStreamFrame( i, 0, saveHead.frameType, (uchar *)(proconH264Node->data + sizeof(*pFraemHead)), localFrameLen, 25 );
            }
        }
    	else
        {
        	getNotCount++;
        }

    	ProconH264Free( proconH264Node );
    }

	if( getNotCount >= REAL_CHANNEL_NUM ) 
    {
    	usleep( 20000 );
    	getNotCount = 0;
    }    
}

#endif 

