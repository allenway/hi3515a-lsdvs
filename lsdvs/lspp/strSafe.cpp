/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此文件实现更安全的字符串操作函数strlcpy和strlcat，主要用于替换
**            strcpy,strncpy和strcat,strncat函数，以提供更加安全的字符串操作。
**  date           :  2013.11.11
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "strSafe.h"

size_t strlcpy( char * dst, const char * src, size_t siz )
{
	char *        	d = dst;
	const char *	s = src;
	size_t	    	n = siz;

	if( NULL == dst || NULL == src )
    {
    	FiPrint( "error: NULL == dst || NULL == src!\r\n" );
    	return 0;
    }

	if ( n != 0 )
    {
        while ( --n != 0 )
        {
            if ( (*d++ = *s++) == '\0' )
            	break;
        }
    }

	if ( n == 0 )
    {
        if ( siz != 0 ) *d = '\0'; 
        while ( *s++ )
        {
            ;  // do nothing !
        }
    }

	return ( s - src - 1 ); 
}

size_t strlcat( char * dst, const char * src, size_t siz )
{
	char *        	d = dst;
	const char *	s = src;
	size_t	    	n = siz;
	size_t	    	dlen = 0;

	if( NULL == dst || NULL == src )
    {
    	FiPrint( "error: NULL == dst || NULL == src!\r\n" );
    	return 0;
    }

	while ( n-- != 0 && *d != '\0' )
    {
        ++d;
    }
	dlen = d - dst;
	n = siz - dlen;

	if ( n == 0 )
    {
    	return ( dlen + strlen(s) );
    }
    
	while ( *s != '\0' )
    {
    	if ( n != 1 )
        {
            *d++ = *s;
            --n;
        }
        ++s;
    }
    *d = '\0';

	return ( dlen + (s - src) );
}

