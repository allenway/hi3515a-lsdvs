#if 0



#define UBOOT_MTD     "/dev/mtdblock0"
#define UBOOT_NAME     "./uboot.mtd"
#define UBOOT_SIZE  1048576

#define KERNEL_MTD "/dev/mtdblock1"
#define KERNEL_NAME "./uImage.mtd"
#define KERNEL_SIZE 1572864

#define FS_MTD "/dev/mtdblock2"
#define FS_NAME "./fs.mtd"
#define FS_SIZE 6291456


#include "norflash.h"
#include "debug.h"
#include "linuxFile.h"

static char bufUboot[UBOOT_SIZE];
static char bufKernel[KERNEL_SIZE];
static char bufFs[FS_SIZE];

void TestReadMtd1()
{
	int ret;
	int fd;

	ret = mtd_read( UBOOT_MTD, 0, UBOOT_SIZE, bufUboot );
	SVPrint( "ret(%d) = mtd_read UBOOT\r\n", ret );
	if( 0 == ret )
    {
    	fd = Open( UBOOT_NAME, O_RDWR|O_CREAT );
    	if( -1 != fd )
        {
        	ret = Writen( fd, bufUboot, UBOOT_SIZE );
            
        	SVPrint( "ret(%d) = Writen UBOOT\r\n", ret );
        	Close(fd);
        }
    }
    
	ret = mtd_read( KERNEL_MTD, 0, KERNEL_SIZE, bufKernel );
	SVPrint( "ret(%d) = mtd_read KERNEL\r\n", ret );
	if( 0 == ret )
    {
    	fd = Open( KERNEL_NAME, O_RDWR|O_CREAT );
    	if( -1 != fd )
        {
        	ret = Writen( fd, bufKernel, KERNEL_SIZE );
            
        	SVPrint( "ret(%d) = Writen KERNEL\r\n", ret );
        	Close(fd);
        }
    }

	ret = mtd_read( FS_MTD, 0, FS_SIZE, bufFs );
	SVPrint( "ret(%d) = mtd_read FS\r\n", ret );
	if( 0 == ret )
    {
    	fd = Open( FS_NAME, O_RDWR|O_CREAT );
    	if( -1 != fd )
        {
        	ret = Writen( fd, bufFs, FS_SIZE );
            
        	SVPrint( "ret(%d) = Writen FS\r\n", ret );
        	Close(fd);
        }
    }
}
#endif

