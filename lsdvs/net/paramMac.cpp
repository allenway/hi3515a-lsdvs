/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  sven
**  version       :  v1.0
**  date           :  2013.06.18
**  description  : 建立本文件管理mac 参数的特殊性
********************************************************************************/

#include "linuxFile.h"
#include "debug.h"
#include "flash.h"
#include "paramManage.h"
#include "netMac.h"
#include "netCom.h"
#include "paramMac.h"

int ParamGetMac( char *pMac )
{
	int ret;
	char macUboot[100];
	char macParam[100];
	char *pMacUboot;
	int macUbootDefFlag = 0, macParamDefFlag = 0;
	PARAM_CONFIG_NETWORK param;

	if( NULL == pMac )
    {
    	SVPrint( "error:NULL == pMac!\r\n" );
    	return -1;
    }    

	pMacUboot = NULL;//GetUbootEnv( MARK_UBOOT_MAC );
	if( NULL != pMacUboot )
    {
    	Strcpy( macUboot, pMacUboot );        
    	if( 0 != Strcasecmp( macUboot, DEFAULT_UBOOT_MAC) )
        {
        	macUbootDefFlag = 1;            
        	Strcpy( pMac, macUboot );
        }
    }
    else
    {
        ERRORPRINT("GetUbootEnv ERROR\n");
    }
    
	ret = ParamGetNetwork( &param );
	if( 0 == ret )
    {
    	Strcpy( macParam, param.wired.mac );        
    	if( 1 == macUbootDefFlag )
        {
        	if( 0 != Strcasecmp( macUboot, macParam) )
            {            
            	Strcpy( param.wired.mac, macUboot );
            	ParamSetNetwork( &param );
            }
        }
    	else
        {
#if defined MCU_HI3515        
        	if( 0 != Strcasecmp( PARAM_CONFIG_DEFAULT_MAC, macParam) )
            {
            	macParamDefFlag = 1;                
            	Strcpy( pMac, macParam );
            	snprintf( macUboot, sizeof(macUboot)-1, "ethaddr=%s", macParam );
            	ret = SetUbootEnv( macUboot );  //DO NOT USE in 3515a  SVEN              	
            }  
#endif             
        }
    }

	if( (1 != macUbootDefFlag && 1 != macParamDefFlag)
       || (0 != CheckMacAddress(pMac)) )
    {        
    	NetGenRandMac( pMac );
    	ERRORPRINT( "failed: get wrong mac, generate rand mac(%s)!\r\n", pMac );

#if defined MCU_HI3515
    	snprintf( macUboot, sizeof(macUboot)-1, "ethaddr=%s", pMac );
    	SetUbootEnv( macUboot );// //DO NOT USE in 3515a SVEN   
#endif 
    	Strcpy( param.wired.mac, pMac );
    	ParamSetNetwork( &param );
    }

	return 0;
}

