#ifndef __DCPERR_H__
#define __DCPERR_H__

#define DCP_RET_ERR_TIMEOUT	    (-2)

//协议全局错误
typedef enum _ErrorTypeEn_ 
{ 
   ERROR_TYPE_SUCCESSFUL             = 0,       // 成功 
   ERROR_TYPE_OTHER                  = 9,       // 其他错误 
   ERROR_TYPE_USER_WRONG             = 10,      // 用户名不对 
   ERROR_TYPE_PASSWD_WRONG,                     // 密码名不对 
   ERROR_TYPE_OVER_MAX_USER,                    // 超过最大的用户数 

   ERROR_TYPE_GET_PARAM_FAILED       = 13,      // 获取参数失败 
   ERROR_TYPE_SET_PARAM_FAILED,                 // 设置参数失败 
   ERROR_TYPE_CHANNEL,                          // 错误的通道号 
   ERROR_TYPE_PARAM_OUTOF_RANGE,                // 参数超出范围 
   ERROR_TYPE_UPDATE_FILE_WRONG_MD5,            // 升级文件MD5 校验错误 
    
   ERROR_TYPE_UPDATE_FILE_WRONG_TYPE = 18,      // 升级文件类型错误 
   ERROR_TYPE_UPDATE_FILE_SIZE,                 // 升级文件的大小不对 
   ERROR_TYPE_DEV_MEMORY_NOT_ENOUGH,            // 设备内存不足    
   ERROR_TYPE_UPDATE_PARAM_INVAL,               // 升级请求参数非法    
   ERROR_TYPE_upDATE_OTHER_UPDATE,              // 其他用户在升级    

   ERROR_TYPE_UPDATE_THREAD_FAIL	= 23,       // 启动升级线程失败    
   ERROR_TYPE_UPDATE_WRITE_FLASH,               // 升级文件写flash失败    
} ERROR_TYPE_EN;


// 客户端sdk 错误类型
typedef enum _CErrorTypeEn_
{
	CERROR_TYPE_CONNECT = 512,       // 连接错误	
	CERROR_TYPE_TIMEOUT,             // 超时
	CERROR_TYPE_ISNOT_LOGIN,         // 用户没登陆
	CERROR_TYPE_NOT_PERMISSION,      // 权限不够     
} CERROR_TYPE_EN;

#endif 

