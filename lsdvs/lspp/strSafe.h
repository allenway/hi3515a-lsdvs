/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此头文件提供更安全的字符串操作函数strlcpy和strlcat的接口，
**            主要用于替换strcpy,strncpy和strcat,strncat函数，
**            以提供更加安全的字符串操作。
**  date           :  2013.11.11
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#ifndef _STR_SAFE_H
#define _STR_SAFE_H

size_t strlcpy( char * dst, const char * src, size_t siz );
size_t strlcat( char * dst, const char * src, size_t siz );

#endif  // _STR_SAFE_H

