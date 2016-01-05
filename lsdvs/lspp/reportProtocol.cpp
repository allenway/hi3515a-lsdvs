#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "debug.h"
#include "malloc.h"
#include "netSocket.h"
#include "packData.h"
#include "dataService.h"
#include "timeExchange.h"
#include "messageProtocol.h"
#include "reportProtocol.h"
#include "fitLspp.h"


typedef FIRS_PACK_HEAD REPORT_PACK_HEAD;
typedef FIRS_PACK_DATA REPORT_PACK_DATA;

//
// 获取数据包长度
//
static int GetReportPackLen( REPORT_PACK_DATA *pPackData )
{
	return FirsGetPackLen( pPackData );
}

//
// 生成搜索数据包
//
static void PackReportDataPack(	unsigned char	subType, 
    	unsigned char *	dataBuf, 
    	unsigned short	dataLen,
    	unsigned char *	packBuf,
    	int &        	packLen )
{
	REPORT_PACK_DATA *pReportPackData = ( REPORT_PACK_DATA * )packBuf;
	FirsGetDataHead( pReportPackData->head );

	unsigned int	msgFlag	    = pReportPackData->head.msgFlag;
	unsigned int	msgType	    = pReportPackData->head.msgType;
	unsigned short	packSn	    = pReportPackData->head.packSn;
	unsigned int	packType	= pReportPackData->head.packType;

	FirsPackDataPack( msgFlag, msgType, packSn, packType, subType,
        	dataBuf, dataLen, packBuf, packLen );
}

static void ChangeTimeToInt( char tBuf[19], time_t &t )
{
	char timeBuf[100] = { 0 };
	int year = 0, month = 0, day = 0;
	int hour = 0, minute = 0, second = 0;    
	memcpy( timeBuf, tBuf, 19 );
	sscanf( timeBuf, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second );    
	t = FiTimeHumanToUtc( year, month, day, hour, minute, second );
}


int DealPcReportDailyData( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	int	                	nRet	        = -1;
	REPORT_PACK_DATA *    	pReportPack	    = (REPORT_PACK_DATA *)dataBuf;
	REPORT_PC_DATA_REQUEST	stPcDataReq	    = { 0 };
	memcpy( &stPcDataReq, pReportPack->data, sizeof(stPcDataReq) );

	time_t startTime             = 0;
	time_t endTime              = 0;
	unsigned char *pReportData     = NULL;
	int reportLen                 = 0;
	ChangeTimeToInt( (char*)stPcDataReq.startTime, startTime );
	ChangeTimeToInt( (char*)stPcDataReq.endTime, 	endTime );

	if ( nRet == 0 && reportLen < 15*1024)
    {
        //PackReportDataPack( MSG_REPORT_RESPONSE, (unsigned char*)pReportData, reportLen, dataBuf, dataLen );
    	PackReportDataPack( MSG_REPORT_RESPONSE, pReportData, reportLen, dataBuf, dataLen );
    }
	else
    {
    	if ( reportLen >= 15*1024 )
        {
        	FiPrint( "DealPcReportDailyData report is too long, ERROR!\r\n" );
        }
    	else
        {
        	FiPrint( "DealPcReportDailyData GetReportData invoke failed!\r\n" );
        }
    	PackReportDataPack( MSG_REPORT_ERROR, NULL, 0, dataBuf, dataLen );
    	nRet = -2;
    }

	return nRet;
}

int DealPcReportMonthData( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	return DealPcReportDailyData( dataBuf, dataLen, bufSize );
}

//
// 处理函数列表, 根据协议不断扩展.
//
#define MESSAGE(id, memberFxn) {id, memberFxn}
static ID_CMD s_ReportIdCmd[] = 
{
	MESSAGE( MSG_REPORT_PC_GETDAILYDATA_REQUEST,    	DealPcReportDailyData	),
	MESSAGE( MSG_REPORT_PC_GETMONTHDATA_REQUEST,    	DealPcReportMonthData	),
};

int DealReportDataProcess( unsigned char *dataBuf, int &dataLen, const int bufSize )
{
	FiPrint( "\r\n#################DealReportDataProcess\r\n" );

	int nRet = -1;
	REPORT_PACK_DATA *pReportPack = (REPORT_PACK_DATA *)dataBuf;
	CIdCmd dealReport;
	dealReport.Init( s_ReportIdCmd, sizeof(s_ReportIdCmd)/sizeof(ID_CMD) );

	int cmdId = ntohl( pReportPack->head.packType );
	ID_CMD_ACTION DealReportCmd = dealReport.GetCmd( cmdId );
	if ( DealReportCmd != NULL )
    	nRet = DealReportCmd( dataBuf, dataLen, bufSize );

	if ( nRet < 0 ) FiPrint( "DealReportDataProcess - Iavailable cmdid(%d)!\r\n" );
	return nRet;
}

int CheckReportDataProcess( unsigned char *dataBuf, int dataLen, int bufSize, int &offset )
{
	int	            	nRet	        = -1;
	REPORT_PACK_DATA    *pReportPack	= (REPORT_PACK_DATA *)dataBuf;
	int	            	packLen	        = GetReportPackLen( pReportPack );

	if ( packLen > bufSize )
    {
    	offset = sizeof( REPORT_PACK_HEAD );  // 包长出错, 丢弃包头.
    	FiPrint( "Check Data Report Pack Length Error !\r\n" );
    }
	else
    	if ( packLen <= dataLen )
        {
        	if ( IsCheckSumOK(dataBuf, packLen) )
            {
            	if ( pReportPack->head.msgType == MSG_DATA_REPORT_TYPE )
                {
                	nRet = packLen;
                	offset = 0;
                }
            	else
                {
                	offset = packLen;
                	FiPrint( "Check Data Report Type Error !\r\n" );
                }
            }
        	else
            {
            	offset = packLen;
            	FiPrint( "Check Sum Error !\r\n" );
            }
        }
	return nRet;
}

