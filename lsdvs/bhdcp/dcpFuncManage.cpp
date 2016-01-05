/********************************************************************************
**	Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**	author        :  sven
**	version       :  v1.0
**	date           :  2013.04.17
**	description  : 管理处理协议指令的函数
********************************************************************************/

#include "debug.h"
#include "dcpIns.h"
#include "dcpInsLocal.h"
#include "dcpFuncList.h"
#include "dcpFuncListLocal.h"
#include "dcpFuncManage.h"

/*****************************************************************************
* 对非法命令的统一处理
*******************************************************************************/
static int IavailableMsgType(DCP_HEAD_T *msgHead,CLIENT_COMMUNICATE_T *clientCom)
{    
	ERRORPRINT( "dvs recv inavailable msg:0x%X!\r\n", msgHead->msgType );        
	clientCom->writeBufSize = 0;
    
	return 0;    
}

// 客户端消息处理函数列表, 根据协议不断扩展.
static DCP_MSG_FUNC_T g_clientMsgFunction[] = 
{
    { SV_MSG_REQ_VARIFY,                        SVMsgVerifyReq	            },
    { SV_MSG_REQ_HEARTBEAT,                     SVMsgHeartbeatReq	        },
    { SV_MSG_REQ_STREAM_START,                  SVMsgStreamStartReq	        },
    { SV_MSG_REQ_GET_BASE_INFO,                 SVMsgGetBaseInfo	        }, 
    { SV_MSG_REQ_GET_NETWORK,                   SVMsgGetNetwork	            },
    
    { SV_MSG_REQ_SET_NETWORK,                   SVMsgSetNetwork	            },    
    { SV_MSG_REQ_GET_CLIENT_USER,               SVMsgGetClientUser	        },
    { SV_MSG_REQ_SET_CLIENT_USER,               SVMsgSetClientUser	        },
    { SV_MSG_REQ_GET_VIDEO_BASE_PARAM,          SVMsgGetVideoBaseParam	    },
    { SV_MSG_REQ_SET_VIDEO_BASE_PARAM,          SVMsgSetVideoBaseParam	    },
    
    { SV_MSG_REQ_GET_OSD_LOGO,            	    SVMsgGetOsdLogo             },
    { SV_MSG_REQ_SET_OSD_LOGO,            	    SVMsgSetOsdLogo              },
    { SV_MSG_REQ_GET_OSD_TIME,            	    SVMsgGetOsdTime             },
    { SV_MSG_REQ_SET_OSD_TIME,            	    SVMsgSetOsdTime             },
    { SV_MSG_REQ_GET_VIDEO_ENCODE_PUBLIC,	    SVMsgGetVideoEncodePublic	},    
    
    { SV_MSG_REQ_SET_VIDEO_ENCODE_PUBLIC,	    SVMsgSetVideoEncodePublic     },
    { SV_MSG_REQ_GET_AUDIO,                     SVMsgGetAudio	            },
    { SV_MSG_REQ_SET_AUDIO,                     SVMsgSetAudio	            },
    { SV_MSG_REQ_GET_AUTO_MAINTAIN,             SVMsgGetAutoMaintain	    },    
    { SV_MSG_REQ_SET_AUTO_MAINTAIN,             SVMsgSetAutoMaintain	    },
    
    { SV_MSG_REQ_GET_RECORD_PUBLIC,             SVMsgGetRecordPublic	    },     
    { SV_MSG_REQ_SET_RECORD_PUBLIC,             SVMsgSetRecordPublic	    },    
    { SV_MSG_REQ_GET_RECORD_PARAM,              SVMsgGetRecordParam	        }, 
    { SV_MSG_REQ_SET_RECORD_PARAM,              SVMsgSetRecordParam	        },
    { SV_MSG_REQ_GET_ALARM_IO,                  SVMsgGetAlarmIoParam	    },
    
    { SV_MSG_REQ_SET_ALARM_IO,                  SVMsgSetAlarmIoParam	    },
    { SV_MSG_REQ_GET_NTP,                       SVMsgGetNtp	                },
    { SV_MSG_REQ_SET_NTP,                       SVMsgSetNtp	                },
    { SV_MSG_REQ_GET_EMAIL,                     SVMsgGetEmail	            },
    { SV_MSG_REQ_SET_EMAIL,                     SVMsgSetEmail	            },
    
    { SV_MSG_REQ_GET_VIDEO_ENCODE_PARAM,     	SVMsgGetVideoEncodeParam	    },
    { SV_MSG_REQ_SET_VIDEO_ENCODE_PARAM,    	SVMsgSetVideoEncodeParam	    },
    { SV_MSG_REQ_GET_VIDEO_ENCODE_PARAM_SLAVE,  SVMsgGetVideoEncodeParamSlave	},
    { SV_MSG_REQ_SET_VIDEO_ENCODE_PARAM_SLAVE,	SVMsgSetVideoEncodeParamSlave	},
    { SV_MSG_REQ_GET_OSD,                    	SVMsgGetOsd	                    },
    
    { SV_MSG_REQ_SET_OSD,                       SVMsgSetOsd	                    },
    { SV_MSG_REQ_SEARCH_RECORD,                 SVMsgSearchRecord	            },
    { SV_MSG_REQ_RECORD,                        SVMsgRecordReq	                },
    { SV_MSG_REQ_STREAM_STOP,                   SVMsgStopStream	                },
    { SV_MSG_REQ_LOGOUT,                        SVMsgLogout	                    },

    { SV_MSG_REQ_UPDATE,                        SVMsgUpdateReq	                },
    { SV_MSG_REQ_GET_LOG,                       SVMsgGetLog	                    },
    { SV_MSG_REQ_GET_ALARM_VLOSS,               SVMsgGetAlarmVlossParam         },
    { SV_MSG_REQ_SET_ALARM_VLOSS,               SVMsgSetAlarmVlossParam         },
    { SV_MSG_REQ_GET_ALARM_SHELTER,             SVMsgGetAlarmVideoShelterParam  },

    { SV_MSG_REQ_SET_ALARM_SHELTER,             SVMsgSetAlarmVideoShelterParam  },
    { SV_MSG_REQ_GET_ALARM_MD,                  SVMsgGetAlarmMDParam            },
    { SV_MSG_REQ_SET_ALARM_MD,                  SVMsgSetAlarmMDParam            },
    { SV_MSG_REQ_STORAGE_INFO,                  SVMsgGetStorageInfo             },
    { SV_MSG_REQ_GET_TIME,                      SVMsgGetSysTime	                },

    { SV_MSG_REQ_SET_TIME,                      SVMsgSetSysTime	                },
    { SV_MSG_REQ_RESET_FACTORY,                 SVMsgResetFactory	            },
    { SV_MSG_REQ_RESET_DEFAULT_PARAM,           SVMsgResetDefaultParam	        },
    { SV_MSG_REQ_SYS_REBOOT,                    SVMsgSysReboot	                },
    { SV_MSG_REQ_GET_ALL_PARAM,                 SVMsgGetAllParam                },

    { SV_MSG_REQ_SET_ALL_PARAM,                 SVMsgSetAllParam                },
    
};

DCP_CLIENT_COM_FUNC GetClientMsgFunction( unsigned short msg )
{
	int i;
	DCP_CLIENT_COM_FUNC func = IavailableMsgType;
    
	int loopNum = sizeof(g_clientMsgFunction)/sizeof(DCP_MSG_FUNC_T);
	for( i = 0; i < loopNum; i++ )
    {
    	if( msg == g_clientMsgFunction[i].msg )
        {
        	func =  g_clientMsgFunction[i].func;
        	break;
        }
    }

	return func;
}

// 以下是处理本地指令的函数管理
static DCP_FUNC_LOCAL_T g_dcpFuncLocal[] = 
{
    { DIL_FORWARD,                     DflForward           },
    { DIL_ALARM_UPLOAD,             DflAlarmUpload          },
    { DIL_HDD_STAT_ERR_REPORT,     	DflHddStatErrReport     },
    { DIL_PARAM_CHANGED_REPORT, 	DflParamChangedUpload   },
};

DCP_FUNC_LOCAL GetDcpFuncLocal( unsigned short msg )
{
	int i;
	DCP_FUNC_LOCAL func = DflIavailableMsgType;
    
	int loopNum = sizeof(g_dcpFuncLocal)/sizeof(DCP_FUNC_LOCAL_T);
	for( i = 0; i < loopNum; i++ )
    {
    	if( msg == g_dcpFuncLocal[i].msg )
        {
        	func =  g_dcpFuncLocal[i].func;
        }
    }

	return func;
}

