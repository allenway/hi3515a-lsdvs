/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此文件实现了对多语言进行支持的相关类
**  date           :  2013.11.11
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "debug.h"
#include "malloc.h"
#include "strSafe.h"
#include "lang.h"

void CLang::ClearCfg( LP_LANGCFG *ppLangCfg )
{
	if ( ppLangCfg ) 
    {
    	for ( int i = 0; i < m_nLangNum; i++ )
        {
        	Free( ppLangCfg[i]->fontLib );
        	Free( ppLangCfg[i]->charSet );
        	Free( ppLangCfg[i]->langFile );
        	Free( ppLangCfg[i]->langSufix );
        	Free( ppLangCfg[i]->langName );
        	Free( ppLangCfg[i] );
        }
    	Free( ppLangCfg );
    }    
}

/*
** 读取语言配置文件
** 配置文件格式: 语言ID=字库名称,字符集名称,语言包名称,语言后缀,语言名称
*/
int CLang::ReadCfg( char *cfgFile )
{
	int nRet = -1;
	char *pCfgBuf = NULL;
     
	int len = ReadFile( &pCfgBuf, cfgFile );
	if ( len > 0 && pCfgBuf != NULL )
    {
    	int lineCnt = 0;

        // 计算配置文件行数
    	for( char *pCfgTmp = pCfgBuf; pCfgTmp != NULL; pCfgTmp++ )
        {
        	pCfgTmp = strchr( pCfgTmp, '\n' );
        	if ( pCfgTmp != NULL ) lineCnt++;
        	else break;
        }

    	if ( lineCnt > 0 )
        {
        	if ( m_ppLangCfg ) ClearCfg( m_ppLangCfg );
        	m_ppLangCfg = ( LP_LANGCFG * )Malloc( lineCnt * sizeof(LP_LANGCFG) );
        	if ( m_ppLangCfg )
            {
            	for ( int i = 0; i < lineCnt; i++ ) m_ppLangCfg[i] = NULL;

            	char *pCfg = pCfgBuf;
            	char *pLine = NULL;
            	int langCnt = 0;
            	while ( (langCnt < lineCnt) && (pLine = GetLine( &pCfg )) )
                {
                	m_ppLangCfg[langCnt] = GetLangCfg( pLine );
                	if ( m_ppLangCfg[langCnt] != NULL ) langCnt++;
                	Free( pLine );
                }
                
            	if ( langCnt > 0 ) // 读取配置文件成功
                {
                	m_nLangNum = langCnt;
                	nRet = 0;
                	FiPrint( "Load language config file success !\r\n" );                    
                }
            }
        	else
            {
            	FiPrint( "Load language config file error: Malloc memory failed !\r\n" );                
            }
        }
    }
	else
    {
    	if ( len == 0 )
        	FiPrint( "Load language config file error: Empty file !\r\n" );
    	else if ( len == -1 )
        	FiPrint( "Load language config file error: Read file error !\r\n" );
    	else
        	FiPrint( "Load language config file error: Malloc memory failed !\r\n" );
    }
    
	Free( pCfgBuf );
	return nRet;
}

/*
** 将语言包读取到哈希表中
*/
int CLang::LoadLangFile( char *file )
{
	int nRet = -1;
	char *pLangBuf = NULL;

    // 将语言包中的数据保存到缓冲区中
	int len = ReadFile( &pLangBuf, file );

	if ( len > 0 && pLangBuf != NULL )
    {
    	m_lang.Clear(); // 清空哈希表

    	char *pKey = NULL;
    	char *pVal = NULL;
    	char *pLine = NULL;

    	char *pLang = pLangBuf;

        // 从缓冲区中反复读取一行数据
        // 并从该行数据中提取关键字和值保存到哈希表中
    	while ( (pLine = GetLine( &pLang )) != NULL )
        {
        	GetKeyVal( &pKey, &pVal, pLine );
        	if ( pKey != NULL && pVal != NULL ) m_lang.Put( pKey, pVal );
        	Free( pLine );
        }

    	nRet = 0;
    }
	else
    {
    	if ( len == 0 )
        	FiPrint( "Load language file error: Empty file !\r\n" );
    	else if ( len == -1 ) 
        	FiPrint( "Load language file error: Read file error !\r\n" );
    	else
        	FiPrint( "Load language file error: Malloc memory failed !\r\n" );
    }
    
	Free( pLangBuf );
	return nRet;
}

/* 配置文件格式: 语言ID=字库名称,字符集名称,语言包名称,语言后缀,语言名称 */
LP_LANGCFG CLang::GetLangCfg( char *pCfgBuf )
{
	int flag = 1;
	if ( pCfgBuf == NULL ) return NULL;

	LP_LANGCFG pLangCfg = ( LP_LANGCFG )Malloc( sizeof( LANGCFG ) );
	if ( pLangCfg == NULL ) return NULL;

	pLangCfg->langID = 0;
	pLangCfg->fontLib = NULL;
	pLangCfg->charSet = NULL;
	pLangCfg->langFile = NULL;
	pLangCfg->langSufix = NULL;
	pLangCfg->langName = NULL;        

	int nBufLen = strlen(pCfgBuf) + 1;
    // 处理一行配置文件信息
	char *pCfgTmp = ( char * )Malloc( sizeof(char) * nBufLen );
	if ( pCfgTmp != NULL )
    {
    	strlcpy( pCfgTmp, pCfgBuf, nBufLen );

    	char *pTmp = pCfgTmp;
    	char *pStr = NULL;
        
        // 读取语言ID
    	if ( (flag == 1) && (pStr = strchr( pTmp, '=' )) )
        {
            *pStr = '\0';
            // 语言ID不允许为空!
        	if ( pTmp[0] != '\0' )
            {
            	pLangCfg->langID = atoi( pTmp );
            	pTmp = ++pStr;
            }
        	else
            {
            	flag = 0;
            }
        }
    	else
        {
        	flag = 0;
        }
        
        // 读取字体库名称
    	if ( (flag == 1) && (pStr = strchr( pTmp, ',' )) )
        {
            *pStr = '\0';
            // 字库名称不允许为空!
        	if( pTmp[0] != '\0' )
            {
            	pLangCfg->fontLib = StrCopy( pTmp );
            	if ( pLangCfg->fontLib ) pTmp = ++pStr;
            	else flag = 0;
            } 
        	else
            {
            	flag = 0;
            }
        }
    	else
        {
        	flag = 0;
        }

        // 读取字体最大宽度
    	if ( (flag == 1) && (pStr = strchr( pTmp, ',' )) )
        {
            *pStr = '\0';
        	pLangCfg->fontWidth = atoi( pTmp );
            // 字体最大宽度只允许为1或2
        	if ( 1 <= pLangCfg->fontWidth && pLangCfg->fontWidth <= 2 ) pTmp = ++pStr;
        	else flag = 0;
        }
    	else
        {
        	flag = 0;
        }
        
        // 读取字符集名称
    	if ( (flag == 1) && (pStr = strchr( pTmp, ',' )) )
        {
            *pStr = '\0';
        	pLangCfg->charSet = StrCopy( pTmp );
        	if ( pLangCfg->charSet ) pTmp = ++pStr;
        	else flag = 0;
        }
    	else
        {
        	flag = 0;
        }
        
        // 读取语言包名称
    	if ( (flag == 1) && (pStr = strchr( pTmp, ',' )) )
        {
            *pStr = '\0';

        	pLangCfg->langFile = StrCopy( pTmp );
        	if ( pLangCfg->langFile ) 
            {
                // 语言包名称不允许为空
            	if ( pLangCfg->langFile[0] != '\0' ) pTmp = ++pStr;
            	else flag = 0;
            }
        	else
            {
            	flag = 0;
            }
        }
    	else
        {
        	flag = 0;
        }    
        
        // 读取语言后缀名
    	if ( (flag == 1) && (pStr = strchr( pTmp, ',' )) )
        {
            *pStr = '\0';
        	pLangCfg->langSufix= StrCopy( pTmp );
        	if ( pLangCfg->langSufix ) pTmp = ++pStr;
        	else flag = 0;
        }
    	else
        {
        	flag = 0;
        }
        
        // 读取语言名称
    	if ( (flag == 1) && (pStr = strchr( pTmp, '\0' )) )
        {
            *pStr = '\0';
        	pLangCfg->langName = StrCopy( pTmp );
        	if ( pLangCfg->langName ) pTmp = ++pStr;
        	else flag = 0;
        }
    	else
        {
        	flag = 0;
        }

    	Free( pCfgTmp );
    }

	if ( flag == 0 )
    {
    	Free( pLangCfg->fontLib );
    	Free( pLangCfg->charSet );
    	Free( pLangCfg->langFile );    
    	Free( pLangCfg->langSufix );
    	Free( pLangCfg->langName );        
    	Free( pLangCfg ); 
    }

	return pLangCfg;
}

/* 字符串拷贝函数，动态分配内存保存拷贝的字符串 */
char *CFile::StrCopy( const char *pStr ) 
{
	int len = strlen( pStr );
	char *pBuf = ( char * )Malloc( sizeof( char ) * ( len + 1 ) );
	if ( pBuf != NULL ) strlcpy( pBuf, pStr, len + 1 );
	return pBuf;
}

/*
** 将文件内容读取到缓冲区，缓冲区大小则根据文件大小自动分配空间
*/
int CFile::ReadFile( char **ppBuf, char *pFileName )
{
	int nRet = 0;
	struct stat st;

	if ( pFileName != NULL && stat( pFileName, &st ) == 0 ) 
    {
    	int len = st.st_size;
    	if ( len > 0 )
        {
            *ppBuf = ( char * )Malloc( sizeof(char) * (len + 1) );
        	if ( *ppBuf != NULL )
            {    
            	memset( *ppBuf, 0, sizeof(char) * (len + 1) );
            	int fd = open( pFileName,  O_RDONLY );
            	if ( read( fd, *ppBuf, len ) == len )
                {
                	nRet = len;
                }
            	else
                {
                	Free( *ppBuf );
                    *ppBuf = NULL;
                	nRet = -1;
                }
            	close( fd );
            }    
        	else
            {    
            	nRet = -2;                
            }
        }
    }
    
	return nRet;
}

/*
** 从字符缓冲区中读取一行字符串并分配空间保存该行字符串，
** 同时修改字符串缓冲区指针，使其指向下一行字符串的开头。
*/
char *CFile::GetLine( char **ppBuf )
{
	int len = 0;

	if ( ppBuf == NULL || *ppBuf == NULL ) return NULL;

	char *pn = strchr( *ppBuf, '\n' );
	char *pLine = NULL;

	if ( pn == NULL )
    {
    	len = strlen( *ppBuf );
    	if ( len != 0 ) // 最后一行数据
        {
        	pLine = ( char * )Malloc( sizeof(char) * (len + 1) );
        	if ( pLine != NULL ) strlcpy( pLine, *ppBuf, len + 1 );
        }
        *ppBuf = NULL;
    }
	else
    {
    	len = pn - *ppBuf + 1;
    	pLine = ( char * )Malloc( sizeof(char) * len );
    	if ( pLine != NULL )
        {
        	memcpy( pLine, *ppBuf, sizeof(char) * len );
        	pLine[len-1] = '\0';
        	if ( pLine[len-2] == '\r' ) pLine[len-2] = '\0';
        }

        *ppBuf = pn + 1;  // 指向下一行数据
    }

	return pLine;
}

/*
** 从一行字符串中提取关键字和值
** 忽略关键字和值开头和结尾的空白符
** 忽略注释: 其中符号"//"后面的内容为注释
*/
void CFile::GetKeyVal( char **ppKey, char **ppVal, char * const pLine )
{
	char *pKey = NULL;
	char *pVal = NULL;

    // 去掉注释符"//"后面的内容
	char *pComment = strstr( pLine, "//" );
	if ( pComment != NULL ) *pComment = '\0';

    // 去掉内容结尾的空白符
	int len = strlen( pLine );
	while ( (len > 0) && (pLine[len-1] == ' ' || pLine[len-1] == '\t') )    
    {
    	pLine[len-1] = '\0';
    	len--;
    }

	char *pe = strchr( pLine, '=' );
	if ( pe != NULL )
    {
        *pe = '\0';

    	pKey = pLine;
        // 忽略关键字开头的空白符
    	while( *pKey == ' ' || *pKey == '\t' ) pKey++;  

    	char *pEnd = pe - 1;
        // 去掉关键字结尾的空白符
    	while( pKey <= pEnd && ( *pEnd == ' ' || *pEnd == '\t' ) )
        {
            *pEnd = '\0';
        	pEnd--;
        }
        
    	if ( pKey <= pEnd && *pKey != '\0' ) 
        {            
        	pVal = pe + 1; 
            // 忽略内容开头的空白符
        	while( *pVal == ' ' || *pVal == '\t' ) pVal++;
        	if ( *pVal == '\0' )
            {
            	pKey = NULL;
            	pVal = NULL;
            }
        }
    	else
        {
        	pKey = NULL;
        	pVal = NULL;
        }
    }

    *ppKey = pKey;
    *ppVal = pVal;
}

/* ------------------------------------------------------------------------- */

CLang s_SysLang;

int ReadLangCfg()
{
	char langCfg[256] = { 0 };
	snprintf( langCfg, sizeof(langCfg), "%s/lang.cfg", SYS_CONFIG_LANG_PATH );
	return s_SysLang.ReadCfg( langCfg );
}

char *GetLangText( char *pKey, char *pDefLang )
{    
	return s_SysLang.GetLangText( pKey );
}

char *GetLangText( int nKey, char *pDefLang )
{    
	char keyBuf[100] = { 0 };
	snprintf( keyBuf, sizeof(keyBuf), "%d", nKey );    
	return s_SysLang.GetLangText( keyBuf );
}

int LoadLangText( char *file )
{
	char path[256] = { 0 };
	snprintf( path, sizeof(path), "%s/%s", SYS_CONFIG_LANG_PATH, file );
	return s_SysLang.LoadLangFile( path );    
}

int SetLangID( int langID )
{
	int nRet = 0;
	int curID = s_SysLang.GetLangID();

	if ( curID != langID )
    {
    	nRet = s_SysLang.SetLangID( langID );
    	if ( nRet == 0 )
        {    
        	char path[256];
        	const char *pLangFile = s_SysLang.GetLangFile( langID );
        	snprintf( path, sizeof(path), "%s/%s", SYS_CONFIG_LANG_PATH, pLangFile );
        	nRet = s_SysLang.LoadLangFile( path );
        	if ( nRet != 0 ) s_SysLang.SetLangID( curID );
        }
    }
    
	return nRet;
}

int GetLangID()
{
	return s_SysLang.GetLangID();
}

const char *GetFontLib( int langID )
{
	return s_SysLang.GetFontLib( langID );
}

int GetFontWidth( int langID )
{
	return s_SysLang.GetFontWidth( langID );
}

const char *GetCharSet()
{
    return s_SysLang.GetCharSet( s_SysLang.GetLangID() );
}

const char *GetLangSufix( int langID )
{
	return s_SysLang.GetLangSufix( langID );
}

const char *GetLangName( int langID )
{
	return s_SysLang.GetLangName( langID );
}

int GetIDList( int **ppList )
{
	return s_SysLang.GetIDList( ppList );
}

