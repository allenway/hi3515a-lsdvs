#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "driver.h"
#include "nvp1914.h"
#include "debug.h"
#include "linuxFile.h"

#define GPIO_I2C_MAGIC_BASE	 'I'
#define GPIO_I2C_READ_BYTE   _IOR(GPIO_I2C_MAGIC_BASE,0x01,int)
#define GPIO_I2C_WRITE_BYTE  _IOW(GPIO_I2C_MAGIC_BASE,0x02,int)

#define GPIO_I2C_READ_DWORD  _IOR(GPIO_I2C_MAGIC_BASE,0x03,int)
#define GPIO_I2C_WRITE_DWORD _IOR(GPIO_I2C_MAGIC_BASE,0x04,int)

#define BRIGHTNESSBASEREG 0X0C
#define CONTRASTBASEREG   0X10
#define HUEBASEREG        0X40
#define SATURATIONBASEREG 0X3c
int set_image_parameter_nvp1914(int chn, unsigned char brightness, unsigned char contrast, unsigned char hue, unsigned char saturation)
{

    int value;
    int ret = -1;
    unsigned int device_addr, reg_addr, reg_value;
    
    int gpiofd = -1;
    gpiofd = Open( GPIOI2C, 0 );
    if( gpiofd < 0 )
    {
    	ERRORPRINT( "Open(%s) fail!\r\n", GPIOI2C );
    	return -1;
    }
    else
    {
        //CORRECTPRINT("Open(%s) SUCCESS!\r\n", GPIOI2C );
    }

    device_addr = 0x60;
    
    reg_addr = BRIGHTNESSBASEREG+chn;//(0x10*chn);
    reg_value = brightness;
    value = ((device_addr&0xff)<<24) | ((0xff)<<16) | (0x0000);
    ret = ioctl(gpiofd, GPIO_I2C_WRITE_BYTE, &value);
    if(ret != 0)
    {
        ERRORPRINT("set 0x0000 failed\n");
        Close(gpiofd);
        return ret;
    }
    
    value = ((device_addr&0xff)<<24) | ((reg_addr&0xff)<<16) | (reg_value&0xffff);
    ret = ioctl(gpiofd, GPIO_I2C_WRITE_BYTE, &value);

    if(ret != 0)
    {
        ERRORPRINT("set bright failed\n");
    }
    else
    {
        //CORRECTPRINT("brightness is:%d\n",reg_value);
    }

///////////////////////////////////////////////////////////////////////////////
    reg_addr = CONTRASTBASEREG+chn;
    reg_value = contrast;
    value = ((device_addr&0xff)<<24) | ((0xff)<<16) | (0x0);
    ret = ioctl(gpiofd, GPIO_I2C_WRITE_BYTE, &value);
    value = ((device_addr&0xff)<<24) | ((reg_addr&0xff)<<16) | (reg_value&0xffff);
    ret = ioctl(gpiofd, GPIO_I2C_WRITE_BYTE, &value);

    if(ret != 0)
    {
        ERRORPRINT("set contrast failed\n");
    }

    reg_addr = HUEBASEREG+chn;
    reg_value = hue;
    value = ((device_addr&0xff)<<24) | ((0xff)<<16) | (0x0);
    ret = ioctl(gpiofd, GPIO_I2C_WRITE_BYTE, &value);
    value = ((device_addr&0xff)<<24) | ((reg_addr&0xff)<<16) | (reg_value&0xffff);
    ret = ioctl(gpiofd, GPIO_I2C_WRITE_BYTE, &value);

    if(ret != 0)
    {
        ERRORPRINT("set hue failed\n");
    }


    reg_addr = SATURATIONBASEREG+chn;
    reg_value = saturation;
    value = ((device_addr&0xff)<<24) | ((0xff)<<16) | (0x0);
    ret = ioctl(gpiofd, GPIO_I2C_WRITE_BYTE, &value);
    value = ((device_addr&0xff)<<24) | ((reg_addr&0xff)<<16) | (reg_value&0xffff);
    ret = ioctl(gpiofd, GPIO_I2C_WRITE_BYTE, &value);

    if(ret != 0)
    {
        ERRORPRINT("set saturation failed\n");
    }

    Close(gpiofd);

    return ret;
}
