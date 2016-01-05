#ifndef __DCPINS_H__
#define __DCPINS_H__

#define  SV_MSG_REQ_BASE		0x6000
#define  SV_MSG_RES_BASE		0x8000

//1.认证, 相当于login 功能, 和logout 相对应
#define SV_MSG_REQ_VARIFY               (SV_MSG_REQ_BASE + 0x1)    
#define SV_MSG_RES_VARIFY	            (SV_MSG_RES_BASE + 0x1)    

//2.心跳
#define SV_MSG_REQ_HEARTBEAT	        (SV_MSG_REQ_BASE + 0x2)
#define SV_MSG_RES_HEARTBEAT	        (SV_MSG_RES_BASE + 0x2)

//3.流通道sessionID认证 
#define SV_MSG_REQ_STREAM_SESSIONID	    (SV_MSG_REQ_BASE + 0x3)
#define SV_MSG_RES_STREAM_SESSIONID	    (SV_MSG_RES_BASE + 0x3)

//4.实时流请求
#define SV_MSG_REQ_STREAM_START	        (SV_MSG_REQ_BASE + 0x4)
#define SV_MSG_RES_STREAM_START	        (SV_MSG_RES_BASE + 0x4)

//5.获取基本配置
#define SV_MSG_REQ_GET_BASE_INFO	    (SV_MSG_REQ_BASE + 0x5)
#define SV_MSG_RES_GET_BASE_INFO	    (SV_MSG_RES_BASE + 0x5)

//6.获取网络配置
#define SV_MSG_REQ_GET_NETWORK	        (SV_MSG_REQ_BASE + 0x6)
#define SV_MSG_RES_GET_NETWORK	        (SV_MSG_RES_BASE + 0x6)

//7.设置网络配置
#define SV_MSG_REQ_SET_NETWORK	        (SV_MSG_REQ_BASE + 0x7)
#define SV_MSG_RES_SET_NETWORK	        (SV_MSG_RES_BASE + 0x7)

//8.获取用户配置(全部用户)
#define SV_MSG_REQ_GET_CLIENT_USER	    (SV_MSG_REQ_BASE + 0x8)
#define SV_MSG_RES_GET_CLIENT_USER	    (SV_MSG_RES_BASE + 0x8)

//9.设置用户配置
#define SV_MSG_REQ_SET_CLIENT_USER	    (SV_MSG_REQ_BASE + 0x9)
#define SV_MSG_RES_SET_CLIENT_USER	    (SV_MSG_RES_BASE + 0x9)

//10.获取视频基本参数
#define SV_MSG_REQ_GET_VIDEO_BASE_PARAM	(SV_MSG_REQ_BASE + 0xA)
#define SV_MSG_RES_GET_VIDEO_BASE_PARAM	(SV_MSG_RES_BASE + 0xA)

//11.设置视频基本参数
#define SV_MSG_REQ_SET_VIDEO_BASE_PARAM	(SV_MSG_REQ_BASE + 0xB)
#define SV_MSG_RES_SET_VIDEO_BASE_PARAM	(SV_MSG_RES_BASE + 0xB)

//12.获取OSD logo
#define SV_MSG_REQ_GET_OSD_LOGO         (SV_MSG_REQ_BASE + 0xC)
#define SV_MSG_RES_GET_OSD_LOGO	        (SV_MSG_RES_BASE + 0xC)

//13.设置OSD logo
#define SV_MSG_REQ_SET_OSD_LOGO         (SV_MSG_REQ_BASE + 0xD)
#define SV_MSG_RES_SET_OSD_LOGO         (SV_MSG_RES_BASE + 0xD)

//14.获取OSD time
#define SV_MSG_REQ_GET_OSD_TIME         (SV_MSG_REQ_BASE + 0xE)
#define SV_MSG_RES_GET_OSD_TIME	        (SV_MSG_RES_BASE + 0xE)

//15.设置OSD time
#define SV_MSG_REQ_SET_OSD_TIME         (SV_MSG_REQ_BASE + 0xF)
#define SV_MSG_RES_SET_OSD_TIME	        (SV_MSG_RES_BASE + 0xF)

//16.获取视频编码公共参数
#define SV_MSG_REQ_GET_VIDEO_ENCODE_PUBLIC	    (SV_MSG_REQ_BASE + 0x10)
#define SV_MSG_RES_GET_VIDEO_ENCODE_PUBLIC	    (SV_MSG_RES_BASE + 0x10)

//17.设置视频编码公共参数
#define SV_MSG_REQ_SET_VIDEO_ENCODE_PUBLIC      (SV_MSG_REQ_BASE + 0x11)
#define SV_MSG_RES_SET_VIDEO_ENCODE_PUBLIC	    (SV_MSG_RES_BASE + 0x11)

//18.获取音频编解码参数
#define SV_MSG_REQ_GET_AUDIO	                (SV_MSG_REQ_BASE + 0x12)
#define SV_MSG_RES_GET_AUDIO	                (SV_MSG_RES_BASE + 0x12)

//19.设置音频编解码参数
#define SV_MSG_REQ_SET_AUDIO	            (SV_MSG_REQ_BASE + 0x13)
#define SV_MSG_RES_SET_AUDIO	            (SV_MSG_RES_BASE + 0x13)

//20.获取自动维护参数
#define SV_MSG_REQ_GET_AUTO_MAINTAIN	    (SV_MSG_REQ_BASE + 0x14)
#define SV_MSG_RES_GET_AUTO_MAINTAIN	    (SV_MSG_RES_BASE + 0x14)

//21.设置自动维护参数
#define SV_MSG_REQ_SET_AUTO_MAINTAIN	    (SV_MSG_REQ_BASE + 0x15)
#define SV_MSG_RES_SET_AUTO_MAINTAIN	    (SV_MSG_RES_BASE + 0x15)

//22.获取录像公共参数
#define SV_MSG_REQ_GET_RECORD_PUBLIC	    (SV_MSG_REQ_BASE + 0x16)
#define SV_MSG_RES_GET_RECORD_PUBLIC	    (SV_MSG_RES_BASE + 0x16)

//23.设置录像公共参数
#define SV_MSG_REQ_SET_RECORD_PUBLIC	    (SV_MSG_REQ_BASE + 0x17)
#define SV_MSG_RES_SET_RECORD_PUBLIC	    (SV_MSG_RES_BASE + 0x17)

//24.获取各个通道录像参数
#define SV_MSG_REQ_GET_RECORD_PARAM	    (SV_MSG_REQ_BASE + 0x18)
#define SV_MSG_RES_GET_RECORD_PARAM	    (SV_MSG_RES_BASE + 0x18)

//25.设置各个通道录像参数
#define SV_MSG_REQ_SET_RECORD_PARAM	    (SV_MSG_REQ_BASE + 0x19)
#define SV_MSG_RES_SET_RECORD_PARAM	    (SV_MSG_RES_BASE + 0x19)

//26.获取IO报警参数
#define SV_MSG_REQ_GET_ALARM_IO	    (SV_MSG_REQ_BASE + 0x1A)
#define SV_MSG_RES_GET_ALARM_IO	    (SV_MSG_RES_BASE + 0x1A)

//27.设置IO报警参数
#define SV_MSG_REQ_SET_ALARM_IO	    (SV_MSG_REQ_BASE + 0x1B)
#define SV_MSG_RES_SET_ALARM_IO	    (SV_MSG_RES_BASE + 0x1B)

//28.获取NTP服务参数
#define SV_MSG_REQ_GET_NTP	            (SV_MSG_REQ_BASE + 0x1C)
#define SV_MSG_RES_GET_NTP	            (SV_MSG_RES_BASE + 0x1C)

//29.设置NTP服务参数
#define SV_MSG_REQ_SET_NTP	            (SV_MSG_REQ_BASE + 0x1D)
#define SV_MSG_RES_SET_NTP	            (SV_MSG_RES_BASE + 0x1D)

//30.获取EMAIL服务参数
#define SV_MSG_REQ_GET_EMAIL	        (SV_MSG_REQ_BASE + 0x1E)
#define SV_MSG_RES_GET_EMAIL	        (SV_MSG_RES_BASE + 0x1E)

//31.设置EMAIL服务参数
#define SV_MSG_REQ_SET_EMAIL	        (SV_MSG_REQ_BASE + 0x1F)
#define SV_MSG_RES_SET_EMAIL	        (SV_MSG_RES_BASE + 0x1F)

//32.获取视频编码参数
#define SV_MSG_REQ_GET_VIDEO_ENCODE_PARAM	(SV_MSG_REQ_BASE + 0x20)
#define SV_MSG_RES_GET_VIDEO_ENCODE_PARAM	(SV_MSG_RES_BASE + 0x20)

//33.设置视频编码参数
#define SV_MSG_REQ_SET_VIDEO_ENCODE_PARAM	(SV_MSG_REQ_BASE + 0x21)
#define SV_MSG_RES_SET_VIDEO_ENCODE_PARAM	(SV_MSG_RES_BASE + 0x21)

//32.获取从码流视频编码参数
#define SV_MSG_REQ_GET_VIDEO_ENCODE_PARAM_SLAVE	(SV_MSG_REQ_BASE + 0x22)
#define SV_MSG_RES_GET_VIDEO_ENCODE_PARAM_SLAVE	(SV_MSG_RES_BASE + 0x22)

//33.设置从码流视频编码参数
#define SV_MSG_REQ_SET_VIDEO_ENCODE_PARAM_SLAVE	(SV_MSG_REQ_BASE + 0x23)
#define SV_MSG_RES_SET_VIDEO_ENCODE_PARAM_SLAVE	(SV_MSG_RES_BASE + 0x23)

//34.一次性获取logo OSD和时间OSD
#define SV_MSG_REQ_GET_OSD	    (SV_MSG_REQ_BASE + 0x24)
#define SV_MSG_RES_GET_OSD	    (SV_MSG_RES_BASE + 0x24)

//35.一次性设置logo OSD和时间OSD
#define SV_MSG_REQ_SET_OSD     (SV_MSG_REQ_BASE + 0x25)
#define SV_MSG_RES_SET_OSD     (SV_MSG_RES_BASE + 0x25)

//36.注销登录
#define SV_MSG_REQ_LOGIN_OUT     (SV_MSG_REQ_BASE + 0x26)

//37. 恢复出厂设置
#define SV_MSG_REQ_RESET_FACTORY	(SV_MSG_REQ_BASE + 0x27)
#define SV_MSG_RES_RESET_FACTORY	(SV_MSG_RES_BASE + 0x27)

//38. 恢复默认参数设置
#define SV_MSG_REQ_RESET_DEFAULT_PARAM	(SV_MSG_REQ_BASE + 0x28)
#define SV_MSG_RES_RESET_DEFAULT_PARAM	(SV_MSG_RES_BASE + 0x28)

//39. 恢复默认通道编码参数设置
#define SV_MSG_REQ_RESET_DEFAULT_CHANNEL_ENCODE_PARAM	(SV_MSG_REQ_BASE + 0x29)
#define SV_MSG_RES_RESET_DEFAULT_CHANNEL_ENCODE_PARAM	(SV_MSG_RES_BASE + 0x29)

//40. 获取时间
#define SV_MSG_REQ_GET_TIME	(SV_MSG_REQ_BASE + 0x2A)
#define SV_MSG_RES_GET_TIME	(SV_MSG_RES_BASE + 0x2A)

//41. 设置时间
#define SV_MSG_REQ_SET_TIME	(SV_MSG_REQ_BASE + 0x2B)
#define SV_MSG_RES_SET_TIME	(SV_MSG_RES_BASE + 0x2B)

//42.搜索录像
#define SV_MSG_REQ_SEARCH_RECORD     (SV_MSG_REQ_BASE + 0x2C)
#define SV_MSG_RES_SEARCH_RECORD     (SV_MSG_RES_BASE + 0x2C)

//43.请求升级系统
#define SV_MSG_REQ_UPDATE	    (SV_MSG_REQ_BASE + 0x2D)
#define SV_MSG_RES_UPDATE	    (SV_MSG_RES_BASE + 0x2D)

//44. 实时流停止
#define SV_MSG_REQ_STREAM_STOP	    (SV_MSG_REQ_BASE + 0x2E)

//45. 录像下载请求
#define SV_MSG_REQ_RECORD	    (SV_MSG_REQ_BASE + 0x2F)

//46. 客户端向dvs 注销
#define SV_MSG_REQ_LOGOUT	    (SV_MSG_REQ_BASE + 0x30)

//47. 升级文件传输
#define SV_MSG_REQ_TXUPDATEFILE	            (SV_MSG_REQ_BASE + 0x31)

//48. 升级文件写FLASH进度
#define SV_MSG_RES_WRITE_FLASH_PROGRESS	    (SV_MSG_RES_BASE + 0x32)

//49. 升级结果
#define SV_MSG_RES_UPDATE_RESULT	            (SV_MSG_RES_BASE + 0x33)

//50. dvs向客户端发送报警消息
#define SV_MSG_RES_ALARM	                    (SV_MSG_RES_BASE + 0x34)

//51. 日志下载
#define SV_MSG_REQ_GET_LOG                     (SV_MSG_REQ_BASE + 0x35)
#define SV_MSG_RES_GET_LOG                     (SV_MSG_RES_BASE + 0x35)

//52. SD卡信息下载
#define SV_MSG_REQ_STORAGE_INFO                (SV_MSG_REQ_BASE + 0x36)
#define SV_MSG_RES_STORAGE_INFO                (SV_MSG_RES_BASE + 0x36)

//53.获取视频丢失报警参数
#define SV_MSG_REQ_GET_ALARM_VLOSS	    (SV_MSG_REQ_BASE + 0x37)
#define SV_MSG_RES_GET_ALARM_VLOSS	    (SV_MSG_RES_BASE + 0x37)

//54.设置视频丢失报警参数
#define SV_MSG_REQ_SET_ALARM_VLOSS	    (SV_MSG_REQ_BASE + 0x38)
#define SV_MSG_RES_SET_ALARM_VLOSS	    (SV_MSG_RES_BASE + 0x38)

//55.获取视频遮挡报警参数
#define SV_MSG_REQ_GET_ALARM_SHELTER	    (SV_MSG_REQ_BASE + 0x39)
#define SV_MSG_RES_GET_ALARM_SHELTER	    (SV_MSG_RES_BASE + 0x39)

//56.设置视频遮挡报警参数
#define SV_MSG_REQ_SET_ALARM_SHELTER	    (SV_MSG_REQ_BASE + 0x3A)
#define SV_MSG_RES_SET_ALARM_SHELTER	    (SV_MSG_RES_BASE + 0x3A)

//57.获取移动侦测报警参数
#define SV_MSG_REQ_GET_ALARM_MD	        (SV_MSG_REQ_BASE + 0x3B)
#define SV_MSG_RES_GET_ALARM_MD	        (SV_MSG_RES_BASE + 0x3B)

//58.设置移动侦测报警参数
#define SV_MSG_REQ_SET_ALARM_MD	        (SV_MSG_REQ_BASE + 0x3C)
#define SV_MSG_RES_SET_ALARM_MD	        (SV_MSG_RES_BASE + 0x3C)

//59. dvs向客户端发送硬盘满,或异常报警消息, 与其他报警分开
#define SV_MSG_RES_HDD_STAT_ERR	        (SV_MSG_RES_BASE + 0x3D)

//60. 终端系统重启
#define SV_MSG_REQ_SYS_REBOOT	            (SV_MSG_REQ_BASE + 0x3E)
#define SV_MSG_RES_SYS_REBOOT	            (SV_MSG_RES_BASE + 0x3E)

//61. dvs向客户端发送
#define SV_MSG_RES_PARAM_CHANGED	        (SV_MSG_RES_BASE + 0x3F)

//62. 设置所有参数
#define SV_MSG_REQ_GET_ALL_PARAM	           (SV_MSG_REQ_BASE + 0x40)
#define SV_MSG_RES_GET_ALL_PARAM	           (SV_MSG_RES_BASE + 0x40)

//63. 获取所有参数
#define SV_MSG_REQ_SET_ALL_PARAM	           (SV_MSG_REQ_BASE + 0x41)
#define SV_MSG_RES_SET_ALL_PARAM	           (SV_MSG_RES_BASE + 0x41)


#endif // __DCPINS_H__

