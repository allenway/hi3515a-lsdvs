/*
*******************************************************************************
**  Copyright (c) 2013, 深圳市科技动车电气自动化有限公司
**  All rights reserved.
**    
**  description  : 此文件实现了操作licence文件所需要的相关函数
**  date           :  2011.04.06
**
**  version       :  1.0
**  author        :  sven
*******************************************************************************
*/
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "debug.h"
#include "mutex.h"
#include "strSafe.h"
#include "licence.h"

class CLicence 
{
public:
	CLicence();
    ~CLicence();
	int GetLicence( LICENCE *pLicence );

private:
	void Init();
	void InitDefaultLicence();
	void PrintLicence();
	int ReadLicence();    
	int WriteLicence();

private:
	CMutexLock m_Mutex;
	LICENCE m_Licence;
};

CLicence::CLicence()
{
	Init();
}

CLicence::~CLicence()
{
}

void CLicence::Init()
{
	InitDefaultLicence();
	ReadLicence();
	PrintLicence();

//	WriteLicence();
}

void CLicence::InitDefaultLicence()
{
	m_Mutex.Lock();
	m_Licence.flag	            = LICENCE_FLAG;
	m_Licence.oemVersion	    = OEM_FIRS;
	m_Licence.platform	        = PF_HI3512;
    
	for ( int channel = 0; channel < MAX_CHANNEL_NUM; ++channel )
    {
    	m_Licence.odType[channel]        = OD_TYPE_NULL;
    	m_Licence.odFlag[channel]        = OD_FLAG_CLOSE;
    	m_Licence.encodeFlag[channel]    = ENCODE_FLAG_OPEN;
    }
	for ( int channel = 0; channel < REAL_CHANNEL_NUM; ++channel )
    {
    	m_Licence.odType[channel]        = OD_TYPE_NULL;
    	m_Licence.odFlag[channel]        = OD_FLAG_CLOSE;
    	m_Licence.encodeFlag[channel]    = ENCODE_FLAG_OPEN;
    }
    
	m_Licence.serialType	    = SERIAL_RS232;
	m_Licence.protocolType	    = PROTOCOL_NORMAL;
	m_Licence.pcType	        = PC_TYPE_PC;
	m_Licence.dataUploadFlag	= DATA_UPLOAD_YES;
	m_Licence.fivRf433mFlag	    = 0;
	m_Licence.led	            = 0;
	m_Licence.mainSlave	        = 0;
    
	m_Licence.threeg	    = 1;                
	m_Licence.wifi	        = 1;            
	m_Licence.wlPlatform	= 0;                
	m_Licence.bsl             = 0;                
	m_Licence.synovate	    = 0;            

	m_Licence.czcf	        = 0;            
	m_Licence.shelterf	    = 0;                
	m_Licence.qirui	        = 0;                
	m_Licence.jyzht	        = 0;            
	m_Licence.rlweye	    = 0;                
	m_Licence.bjgq	        = 0;                
	m_Licence.reserved	    = 0;                

	m_Licence.devType	    = 0x06;
    
	m_Mutex.Unlock();    
}

void CLicence::PrintLicence()
{
	LICENCE licence = { 0 };
	if ( GetLicence( &licence ) != -1 )
    {        
    	char oemVersion[256] = { 0 };
    	switch( licence.oemVersion )
        {    
    	case OEM_FIRS:    	strlcpy( oemVersion, "FIRS", sizeof(oemVersion) );    	break;
    	case OEM_STD:    	strlcpy( oemVersion, "FIRS", sizeof(oemVersion) );    	break;            
    	case OEM_LTY:    	strlcpy( oemVersion, "LTY", sizeof(oemVersion) );    	break;            
    	case OEM_SD:    	strlcpy( oemVersion, "SangDa", sizeof(oemVersion) );	break;
    	case OEM_PNM:    	strlcpy( oemVersion, "PuNuoMa", sizeof(oemVersion) );	break;            
    	case OEM_FT:    	strlcpy( oemVersion, "FangTong", sizeof(oemVersion) );	break;            
    	case OEM_ZHUNDIAN:	strlcpy( oemVersion, "ZhunDian", sizeof(oemVersion) );	break;            
    	case OEM_YW:    	strlcpy( oemVersion, "YouWei", sizeof(oemVersion) );	break;            
    	case OEM_JSYD:    	strlcpy( oemVersion, "JiangSuYiDong", sizeof(oemVersion) );	break;
    	case OEM_WKP:    	strlcpy( oemVersion, "WeiKangPu", sizeof(oemVersion) );	break;
        }
        
    	char protocolType[256] = { 0 };
    	switch( licence.protocolType )
        {
    	case PROTOCOL_FIRS:
        	strlcpy( protocolType, "FIRS", sizeof(protocolType) );
        	break;            
    	case PROTOCOL_LTY:
        	strlcpy( protocolType, "LTY", sizeof(protocolType) );
        	break;            
    	case PROTOCOL_YW_GPS:
        	strlcpy( protocolType, "YW_GPS", sizeof(protocolType) );
        	break;
    	case PROTOCOL_JTX_GPS:
        	strlcpy( protocolType, "JTX_GPS", sizeof(protocolType) );
        	break;    
    	case PROTOCOL_FIRS_COM:
        	strlcpy( protocolType, "FIRS_COM", sizeof(protocolType) );
        	break;        
    	case PROTOCOL_FIRS_COUNTER:
        	strlcpy( protocolType, "FIRS_COUNTER", sizeof(protocolType) );
        	break;        
    	case PROTOCOL_FIRS_XS:
        	strlcpy( protocolType, "FIRS_XS", sizeof(protocolType) );
        	break;    
    	case PROTOCOL_FIRS_ERRISSOM:
        	strlcpy( protocolType, "FIRS_ERRISOM_10_PICS", sizeof(protocolType) );
        	break;
    	case PROTOCOL_JSYD_COM_GPS:
        	strlcpy( protocolType, "PROTOCOL_JSYD_COM_GPS", sizeof(protocolType) );
        	break;    
    	default:
        	strlcpy( protocolType, "NULL", sizeof(protocolType) );
        	break;                
        }
    
    	FiPrint( "####################### Licence Info #######################\r\n" );
    	FiPrint( "## Flag:            %08X\r\n", licence.flag );
    	FiPrint( "## OemVersion:      %s\r\n", oemVersion );
    	FiPrint( "## Platform:        %s\r\n", licence.platform ? "TI6446" : "HI3512" );

    	for ( int channel = 0; channel < REAL_CHANNEL_NUM; ++channel )
        {
        	char odType[256] = { 0 };
        	switch( licence.odType[channel] )
            {    
        	case OD_TYPE_NULL:    	strlcpy( odType, "NULL", sizeof(odType) );    	break;
        	case OD_TYPE_PC:    	strlcpy( odType, "PC", sizeof(odType) );    	break;
        	case OD_TYPE_FIV:    	strlcpy( odType, "FIV", sizeof(odType) );    	break;
        	case OD_TYPE_FOV:    	strlcpy( odType, "FOV", sizeof(odType) );    	break;
        	case OD_TYPE_FACE:    	strlcpy( odType, "FACE", sizeof(odType) );    	break;
        	case OD_TYPE_EYE:    	strlcpy( odType, "EYE", sizeof(odType) );    	break;
        	case OD_TYPE_FESMC:    	strlcpy( odType, "FESMC", sizeof(odType) );    	break;
        	case OD_TYPE_LP:    	strlcpy( odType, "LP", sizeof(odType) );    	break;
        	case OD_TYPE_ABDOOR:	strlcpy( odType, "ABDOOR", sizeof(odType) );	break;
            }
        	FiPrint( "## OdType %d:        %s\r\n", channel, odType );
        	FiPrint( "## OdFlag %d:        %d\r\n", channel, licence.odFlag[channel] );
        	FiPrint( "## EncodeFlag %d:    %d\r\n", channel, licence.encodeFlag[channel] );
        }
    	FiPrint( "## SerialType:      %s\r\n", licence.serialType ? "RS485" : "RS232" );
    	FiPrint( "## ProtocolType:    %s\r\n", protocolType );
    	FiPrint( "## PcType:          %s\r\n", licence.pcType ? "PC_BUS" : "PC" );
    	FiPrint( "## DataUpload:      %d\r\n", licence.dataUploadFlag );
    	FiPrint( "## FivRf433:        %d\r\n", licence.fivRf433mFlag);
    	FiPrint( "## LedFlag:         %d\r\n", licence.led);
    	FiPrint( "## MainSlaver:      %d\r\n", licence.mainSlave);
    	FiPrint( "## devType:           %d\r\n", licence.devType);        
    	FiPrint( "############################################################\r\n" );
    }
}

int CLicence::GetLicence( LICENCE *pLicence )
{
	if ( pLicence == NULL ) return -1;
	m_Mutex.Lock();
	memcpy( pLicence, &m_Licence, sizeof(LICENCE) );
	m_Mutex.Unlock();
	return 0;
}

int CLicence::ReadLicence()
{
	int nRet = -1;
	int hf = open( "./resource/licence", O_RDONLY );
	if ( hf != -1 )
    {
    	LICENCE licence = { 0 };
    	int size = read( hf, (char*)&licence, sizeof(LICENCE) );
    	close( hf );
    	if ( size != -1 && licence.flag == LICENCE_FLAG )
        {
        	if ( size > (int)sizeof(LICENCE) ) size = sizeof(LICENCE);
        	if ( size != (int)sizeof(LICENCE) )
            {
            	FiPrint( "INFO : Current licence file size %d bytes, "
                        "Licence struct size %d bytes.\r\n", size, sizeof(LICENCE) );
            }
        	m_Mutex.Lock();
        	memcpy( &m_Licence, &licence, size );        
        	m_Mutex.Unlock();
        	nRet = 0;
        }
    	else
        {
        	FiPrint( "ERROR: Read licence failed !\r\n" );
        }
    }
	return nRet;
}

int CLicence::WriteLicence()
{
	int nRet = -1;
	int hf = open( "./resource/licence", O_WRONLY );
	if ( hf != -1 )
    {
    	LICENCE licence = { 0 };
    	if ( GetLicence( &licence ) != -1 )
        {
        	if ( write(hf, &licence, sizeof(licence)) == sizeof(licence) ) nRet = 0;
        	else FiPrint( "ERROR: Write Licence failed !\r\n" );
        }
    	close( hf );
    }
	return nRet;
}

// -------------------------------------------------------------------------------------------------

//
// 接口函数
//

static CLicence s_SysLicence;

int SysGetLicence( LICENCE *pLicence )
{
	return s_SysLicence.GetLicence( pLicence );
}

int SysGetOemVersion()
{    
	LICENCE licence = { 0 };
	if ( s_SysLicence.GetLicence(&licence) != -1 );
    	return licence.oemVersion;
	return OEM_FIRS;
}

int SysGetPlatform()
{    
	LICENCE licence = { 0 };
	if ( s_SysLicence.GetLicence(&licence) != -1 );
    	return licence.platform;
	return PF_TI6446;
}

int SysGetOdType( int channel )
{
	LICENCE licence = { 0 };    
	if ( s_SysLicence.GetLicence(&licence) != -1 )
    	return licence.odType[channel];
	return OD_TYPE_NULL;
}

int SysGetOdFlag( int channel )
{
	LICENCE licence = { 0 };    
	if ( s_SysLicence.GetLicence(&licence) != -1 )
    	return licence.odFlag[channel];
	return OD_FLAG_CLOSE;
}

int SysGetEncodeFlag( int channel )
{
	LICENCE licence = { 0 };    
	if ( s_SysLicence.GetLicence(&licence) != -1 )
    	return licence.encodeFlag[channel];
	return ENCODE_FLAG_CLOSE;
}

int SysGetSerialType()
{
	LICENCE licence = { 0 };    
	if ( s_SysLicence.GetLicence(&licence) != -1 )
    	return licence.serialType;
	return SERIAL_NORMAL;
}

int SysGetProtocolType()
{
	LICENCE licence = { 0 };
	if ( s_SysLicence.GetLicence(&licence) != -1 )
    	return licence.protocolType;
	return -1;
}

int SysIsPcBus()
{    
	LICENCE licence = { 0 };
	if ( s_SysLicence.GetLicence(&licence) != -1 );
    	return licence.pcType == PC_TYPE_PC_BUS;
	return 0;
}

int SysGetPcType()
{
	LICENCE licence = { 0 };    
	if ( s_SysLicence.GetLicence(&licence) != -1 )
    	return licence.pcType;
	return PC_TYPE_PC;
}

int SysGetDataUploadFlag()
{
	LICENCE licence = { 0 };
	if ( s_SysLicence.GetLicence(&licence) != -1 )
    	return licence.dataUploadFlag;
	return 0;
}

int SysGetFivRf433mFlag()
{
	LICENCE licence = { 0 };
	if ( s_SysLicence.GetLicence(&licence) != -1 )
    	return licence.fivRf433mFlag;
	return 0;
}

int SysGetLedFlag()
{
	LICENCE licence = { 0 };
	if ( s_SysLicence.GetLicence(&licence) != -1 )
    	return licence.led > 0 ? 1 : 0;
	return 0;
}

int SysGetMasterSlaverFlag()
{
	LICENCE licence = { 0 };
	if ( s_SysLicence.GetLicence(&licence) != -1 )
    	return licence.mainSlave > 0 ? 1 : 0;
	return 0;
}

int SysGetWifiFlag()
{
	LICENCE licence = { 0 };
	if ( s_SysLicence.GetLicence(&licence) != -1 )
    	return licence.wifi > 0 ? 1 : 0;
	return 0;
}

int SysGetThreegFlag()
{
	LICENCE licence = { 0 };
	if ( s_SysLicence.GetLicence(&licence) != -1 )
    	return licence.threeg > 0 ? 1 : 0;
	return 0;
}

int SysGetOemType()
{
	LICENCE licence = { 0 };
	if ( s_SysLicence.GetLicence(&licence) != -1 )
    	return (int)licence.oemVersion;
	return 0;
}

// 城中村人脸抓拍项目
int SysGetCzcfFlag()
{
	LICENCE licence = { 0 };
	if ( s_SysLicence.GetLicence(&licence) != -1 )
    	return (int)licence.czcf;
	return 0;
}

int SysGetSynovateFlag()
{
	LICENCE licence = { 0 };
	if ( s_SysLicence.GetLicence(&licence) != -1 )
    	return licence.synovate > 0 ? 1 : 0;
	return 0;
}

int SysGetBslFlag()
{
	LICENCE licence = { 0 };
	if ( s_SysLicence.GetLicence(&licence) != -1 )
    	return licence.bsl > 0 ? 1 : 0;
	return 0;
}

// 是否遮挡的人脸抓拍
// Shf: shelter face 遮挡人脸抓拍
int SysGetShf()
{
	LICENCE licence = { 0 };
	if ( s_SysLicence.GetLicence(&licence) != -1 )
    	return licence.shelterf > 0 ? 1 : 0;
	return 0;
}

unsigned char SysGetDvsType()
{
	LICENCE licence = { 0 };
	if ( s_SysLicence.GetLicence(&licence) != -1 )
    	return licence.devType;
	return 0;
}

int SysGetQiruiFlag()
{
	LICENCE licence = { 0 };
	if ( s_SysLicence.GetLicence(&licence) != -1 )
    	return (int)licence.qirui;
	return 0;
}

int SysGetJyzhtFlag()
{
	LICENCE licence = { 0 };
	if ( s_SysLicence.GetLicence(&licence) != -1 )
    	return (int)licence.jyzht;
	return 0;
}

int SysGetRlweyeFlag()//列车疲劳检测,add at 2013.07.31
{
	LICENCE licence = { 0 };
	if ( s_SysLicence.GetLicence(&licence) != -1 )
    	return (int)licence.rlweye;
	return 0;
}
int SysGetBjgqFlag()
{
	LICENCE licence = { 0 };
	if ( s_SysLicence.GetLicence(&licence) != -1 )
    	return (int)licence.bjgq;
	return 0;
}
