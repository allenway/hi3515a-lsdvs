#include <stdio.h>
#include "debug.h"

#if 0
static void SaveYuv422( int channel,char *data, int len )
{
	int ret;
	static int count = 0;
	char name[32] = {0};
	sprintf(name, "./yuv-ch%02d-%03d.yuv", channel, ++count);
	int fd = open(name, O_CREAT | O_WRONLY );

	if( fd == -1 )
    {
    	printf( "open error, exit!\r\n" );
    	exit(-1);
    }

	ret = writen( fd, data, len );
	SVPrint( "writen ret(%d):%s==========\r\n", ret, STRERROR_ERRNO );
	sync();
	close(fd);
}


#endif



