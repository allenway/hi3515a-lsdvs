#if 0
#define TEST_JPG_SIZE	(1024 * 1024)
#include <stdio.h>
#include "proconJpg.h"
#include "debug.h"
#include "public.h"
#include "linuxFile.h"
#include "ttypes.h"
#include "timeExchange.h"
#include "malloc.h"
#include "paramManage.h"
#include "ftpJpg.h"

void TestProconJpg()
{
	int fileFd, ret;
	char filename[MAX_JPG_CHN_NUM][16] = { "./ch0.jpg", "./ch1.jpg", "./ch2.jpg", "./ch3.jpg", "./ch4.jpg" };
	static uint proconJpgFd[MAX_JPG_CHN_NUM] = { 0, 0, 0, 0, 0 };
	int i;
	static unsigned int jpgCount = 0;
	static char buf[TEST_JPG_SIZE];
	PROCON_NODE_T *pExNode;
	JPG_INFO_T *jpgInfo;
	int YY, MM, DD, hh, mm, ss;

	for( i = 0; i < MAX_JPG_CHN_NUM; i++ )
    {
    	if( 0 == proconJpgFd[i] )
        {
        	proconJpgFd[i] = ProconJpgOpen( i, OPEN_WRONLY );
        	if( proconJpgFd[i] == 0 )
            {
            	SVPrint( "faile ProconJpgOpen!\r\n" );
            	return;
            }
        }

    	fileFd = Open( filename[i], O_RDONLY );
    	if( -1 == fileFd )
        {
        	SVPrint( "error ch(%d) Open(%s): %s!\r\n", i, filename[i], STRERROR_ERRNO );
        	return;
        }
    	ret = Read( fileFd, buf, sizeof(buf) );
    	SVPrint( "ret(%d) error: %s!\r\n", ret, STRERROR_ERRNO);
    	if( ret > 0 )
        {
        	pExNode = (PROCON_NODE_T *)ShareMalloc( sizeof(PROCON_NODE_T) + sizeof(JPG_INFO_T) + ret );
        	if( NULL != pExNode )
            {
            	pExNode->proconHead.len = sizeof(JPG_INFO_T) + ret;
            	pExNode->proconHead.type = DATA_TYPE_NOMAL;
            	jpgInfo = (JPG_INFO_T *)pExNode->data;
            	jpgInfo->type = RECORD_TYPE_TIMER;
            	jpgInfo->num = ++jpgCount;
            	jpgInfo->len = ret;
            	memcpy( jpgInfo->buf, buf, ret );
            	FiTimeUtcToHuman( time(NULL), &YY, &MM, &DD, &hh, &mm, &ss );
            	sprintf( jpgInfo->datel, "%04d-%02d-%02d", YY, MM, DD );
            	sprintf( jpgInfo->timel, "%02dh02dm02ds", hh, mm, ss );

            	ret = ProconJpgWrite( proconJpgFd[i], pExNode );
            	SVPrint( "channel(%d) ret(%d) = ProconJpgWrite()!\r\n", i, ret );
            }
        }

    	Close( fileFd );
    }
}

void TestFtpJpgParam()
{
	int ret;
	PARAM_CONFIG_FTP ftpParam;
    
	ret = ParamGetFtp(&ftpParam );
	if( 0 == ret )
    {
    	ftpParam.enable = 1;
    	ret = ParamSetFtp( &ftpParam );
    	if( 0 == ret )
        {
        	FtpJpgSendParamChangeMessage();
        }
    }
}

#endif 

