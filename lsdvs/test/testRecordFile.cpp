#if 0
// ¶ÁÈ¡Â¼ÏñÎÄ¼s£¬¼ì²é¸ÃÎÄ¼sÂ¼ÏñµÄÍêÕûGÔ
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define REC_FILENAME "./rec.h264"

typedef struct _PackHead_
{
	unsigned char 	packType;    // °üÀàGÍ, 0:ÊÓÆµ°ü; 1:ÒôÆµ°ü; 2:ÖÇÄÜ°ü
	unsigned char	frameHeadLen;    // Ö¡Í·³¤¶È,¸Ã³¤¶È¸ù¾IpackType µÄÖµ±ä»¯¶øÈ¡ sizeof(²»Í¬µÄ½á¹¹Ìå)    
	char	reserved[2];
} PACK_HEAD_T;

// ÒôÊÓÆµÖ¡Í·ÃèÊö
typedef struct _FrameHead_
{
    unsigned int  frameLen;            // Ö¡ÂãÁ÷³¤¶È
    unsigned int  frameNo;            // Ö¡ºÅ,µ¥µ÷µIÔö
    unsigned char videoStandard;    // ÊÓÆµ:±ê×¼ 0,PAL; 1,NTSC. // ÒôÆµ:Í¨µÀÄ£Ê½, 0:µ¥Í¨µÀ, 1:Á¢ÌåÉù, Ä¬ÈÏÖµ 0
    unsigned char videoResolution;    // ÊÓÆµ:·Ö±æÂÊ 0,qcif; 1,cif; 2,hd1; 3,d1. // ÒôÆµ:²ÉÑùÂÊ, 0:8k, 1:16k, Ä¬ÈÏÖµ 0
    unsigned char frameRate;          // ÊÓÆµ:Ö¡ÂÊ. // ÒôÆµ:Î»¿í,0:8bits, 1:16bits, Ä¬ÈÏÖµ 1
    unsigned char frameType;         // ÊÓÆµ:Ö¡ÀàGÍ 5,IÖ¡; 1,PÖ¡ . // ÒôÆµ::±àÂë·½Ê½,0:adpcm, 1:g711, 2:g726, Ä¬ÈÏÖµ 0
    unsigned int  sec;                // ´Ó±¾µØ»ñÈ¡µÄÊ±¼ä,Ãë
    unsigned int  usec;                // ´Ó±¾µØ»ñÈ¡µÄÊ±¼ä,Î¢Ãî
    unsigned long long  pts;                // ´Óº£Ë¼µ×²ã»ñÈ¡µÄÊ±¼ä´Á,(Î¢Ãë)
} FRAME_HEAD_ST;

 
 typedef struct _StreamHead_
 {
 	PACK_HEAD_T 	packHead;
 	FRAME_HEAD_ST	frameHead;
 } STREAM_HEAD_T;

static void PrintFrameHead( STREAM_HEAD_T *streamHead )
{
	printf( "pt(%u), fhl(%u), fl(%u), fn(%u), vs(%u), vr(%u), fr(%u), ft(%u)!\r\n",
    	streamHead->packHead.packType,
    	streamHead->packHead.frameHeadLen,
    	streamHead->frameHead.frameLen,
    	streamHead->frameHead.frameNo,
    	streamHead->frameHead.videoStandard,
    	streamHead->frameHead.videoResolution,
    	streamHead->frameHead.frameRate,
    	streamHead->frameHead.frameType
        );
}
 
 int main()
 {
	int fd, ret;
	unsigned int lastFrameNo, totalReadSize = 0;
	static char buf[1024*1024];
	STREAM_HEAD_T streamHead;
	fd = open( REC_FILENAME, O_RDONLY );

	ret = read( fd, &streamHead, sizeof(streamHead) );
	if( ret <= 0 )
    {
    	return 0;
    }
	totalReadSize += ret;
    
	printf( "totalReadSize = %u\r\n", totalReadSize );
	ret = read( fd, buf, streamHead.frameHead.frameLen );
	if( ret <= 0 )
    {
    	return 0;
    }
	totalReadSize += ret;
    
	printf( "totalReadSize = %u\r\n", totalReadSize );
	lastFrameNo = streamHead.frameHead.frameNo;
	printf( "start test\r\n" );
	while( 1 )
    {        
    	ret = read( fd, &streamHead, sizeof(streamHead) );
    	if( ret <= 0 )
        {
        	printf( "ret(%d) = read(streamHead)\r\n", ret );
        	break;
        }
    	totalReadSize += ret;
        
    	printf( "totalReadSize = %u\r\n", totalReadSize );
    	PrintFrameHead( &streamHead );
    	ret = read( fd, buf, streamHead.frameHead.frameLen );
    	if( ret <= 0 )
        {            
        	printf( "ret(%d) = read(buf)\r\n", ret );
        	break;
        }
    	totalReadSize += ret;
        
    	printf( "totalReadSize = %u\r\n", totalReadSize );
        #if 0
    	if( 1 != (streamHead.frameHead.frameNo - lastFrameNo) )
        {
        	printf( "1 != (streamHead.frameHead.frameNo - lastFrameNo)\r\n" );
        	break;
        }
        #endif
    	lastFrameNo = streamHead.frameHead.frameNo;
    }
    
	printf( "stop test\r\n" );

	return 0;
}

#endif //

