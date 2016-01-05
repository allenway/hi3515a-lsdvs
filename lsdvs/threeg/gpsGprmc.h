/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司
**  All rights reserved.

**  description  : 解析 GPRMC 信息
**  date           :  2014.9.25

**  version:1.0
**  author :sven
********************************************************************************/
#ifndef __GPSGPRMC_H__
#define __GPSGPRMC_H__

#define TEN_MINUTE (10*60)

typedef enum _GpsGprmcMsgEn_
{
	GPS_GPRMC_MSG_HEAD = 0,            // <0> 消息头，"$GPRMC"    
	GPS_GPRMC_MSG_TIME,                // <1> UTC时间，hhmmss（时分秒）格式
	GPS_GPRMC_MSG_STATE,            // <2> 定位状态，A=有效定位，V=无效定位
	GPS_GPRMC_MSG_LATITUDE,            // <3> 纬度ddmm.mmmm（度分）格式（前面的0也将被传输）
	GPS_GPRMC_MSG_LATITUDE_NS,        // <4> 纬度半球N（北半球）或S（南半球）
	GPS_GPRMC_MSG_LONGITUDE,        // <5> 经度dddmm.mmmm（度分）格式（前面的0也将被传输）
	GPS_GPRMC_MSG_LONGITUDE_EW,        // <6> 经度半球E（东经）或W（西经）
	GPS_GPRMC_MSG_SPEED,            // <7> 地面速率（000.0~999.9节，前面的0也将被传输）
	GPS_GPRMC_MSG_DIRECTION,        // <8> 地面航向（000.0~359.9度，以真北为参考基准，前面的0也将被传输）
	GPS_GPRMC_MSG_DATE,                // <9> UTC日期，ddmmyy（日月年）格式
	GPS_GPRMC_MSG_MAGNETIC_DECLINATION,        // <10> 磁偏角（000.0~180.0度，前面的0也将被传输）
	GPS_GPRMC_MSG_MAGNETIC_DECLINATION_EW,    // <11> 磁偏角方向，E（东）或W（西）
	GPS_GPRMC_MSG_MODE,                        // <12> 模式指示（仅NMEA0183 3.00版本输出，A=自主定位，D=差分，E=估算，N=数据无效）
} GPS_GPRMC_MSG_EN;

typedef enum _GpsGprmcMarkEn_
{
	GPS_GPRMC_MARK_HEAD     = (0x01 << 1),            // <0> 消息头，"$GPRMC"    
	GPS_GPRMC_MARK_TIME     = (0x01 << 2),            // <1> UTC时间，hhmmss（时分秒）格式
	GPS_GPRMC_MARK_STATE     = (0x01 << 3),            // <2> 定位状态，A=有效定位，V=无效定位
	GPS_GPRMC_MARK_LATITUDE = (0x01 << 4),            // <3> 纬度ddmm.mmmm（度分）格式（前面的0也将被传输）
	GPS_GPRMC_MARK_LATITUDE_NS	= (0x01 << 5),        // <4> 纬度半球N（北半球）或S（南半球）
	GPS_GPRMC_MARK_LONGITUDE     = (0x01 << 6),        // <5> 经度dddmm.mmmm（度分）格式（前面的0也将被传输）
	GPS_GPRMC_MARK_LONGITUDE_EW = (0x01 << 7),        // <6> 经度半球E（东经）或W（西经）
	GPS_GPRMC_MARK_SPEED         = (0x01 << 8),        // <7> 地面速率（000.0~999.9节，前面的0也将被传输）
	GPS_GPRMC_MARK_DIRECTION     = (0x01 << 9),        // <8> 地面航向（000.0~359.9度，以真北为参考基准，前面的0也将被传输）
	GPS_GPRMC_MARK_DATE         = (0x01 << 10),                // <9> UTC日期，ddmmyy（日月年）格式
	GPS_GPRMC_MARK_MAGNETIC_DECLINATION     = (0x01 << 11),        // <10> 磁偏角（000.0~180.0度，前面的0也将被传输）
	GPS_GPRMC_MARK_MAGNETIC_DECLINATION_EW     = (0x01 << 12),    // <11> 磁偏角方向，E（东）或W（西）
	GPS_GPRMC_MARK_MODE                     = (0x01 << 13),                        // <12> 模式指示（仅NMEA0183 3.00版本输出，A=自主定位，D=差分，E=估算，N=数据无效）
} GPS_GPRMC_MARK_EN;
#define GPS_GPRMC_MARK_TIME_MIX	    ( GPS_GPRMC_MARK_TIME | GPS_GPRMC_MARK_STATE | GPS_GPRMC_MARK_DATE )
#define GPS_GPRMC_MARK_SPEED_MIX	( GPS_GPRMC_MARK_STATE | GPS_GPRMC_MARK_SPEED )
#define GPS_GPRMC_MARK_XYZ_MIX	    ( GPS_GPRMC_MARK_STATE \
                                    | GPS_GPRMC_MARK_LATITUDE | GPS_GPRMC_MARK_LATITUDE_NS \
                                    | GPS_GPRMC_MARK_LONGITUDE | GPS_GPRMC_MARK_LONGITUDE_EW )
#define GPS_GPRMC_MARK_MAG_DEC_MIX	( GPS_GPRMC_MARK_STATE \
                                    | GPS_GPRMC_MARK_MAGNETIC_DECLINATION \
                                    | GPS_GPRMC_MARK_MAGNETIC_DECLINATION_EW )

typedef struct _GpsGprmcMsgSt_
{
	char	head[16];
	char 	time[9];
	char 	state;
	float 	latitude;
	char 	latitudeNs;
	float 	longitude;
	char  	longitudeEw;
	float 	speed;
	float 	direction;
	char  	date[9];
	float	magneticDeclination;
	char	magneticDeclinationEw;
	char	mode[16];
	int		mark;        //掩码, 对应GPS_GPRMC_MARK_EN
} GPS_GPRMC_MSG_ST;

int FiGpsParseGprmc( char *pGprmc );

#endif 

