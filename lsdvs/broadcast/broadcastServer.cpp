/********************************************************************************
**  Copyright (c) 2013, 深圳市动车电气自动化有限公司, All rights reserved.
**  author        :  SVEN
**  version       :  v1.0
**  date           :  2013.04.22
**  description  : 监听广播信息
********************************************************************************/

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <unistd.h>
#include "debug.h"
#include "linuxFile.h"
#include "hton.h"
#include "thread.h"
#include "message.h"
#include "message.h"
#include "netSocket.h"
#include "broadcastServer.h"
#include "paramConfig.h"
#include "paramManage.h"
#include "netDns.h"
#include "netGateway.h"
#include "netIp.h"
#include "netMask.h"
#include "netMac.h"

#define IFNAME          "eth0" 
#define PORT            8888               //端口号
#define IP_FOUND         "IP_FOUND"        //IP发现命令
#define IP_FOUND_ACK     "IP_FOUND_ACK"    //IP发现应答命令    
#define IP_FOUND_ACK_LEN 12

#define IP_FOUND_ID "UDP_SERVER_1"
#define IP_FOUND_ID_LEN 12

static THREAD_MAINTAIN_T g_broadcastServerTm;
static DEV_FIND_T g_devFind;


static int GetAllDevFindInfo(DEV_FIND_T *pDevFind)
{
    PARAM_CONFIG_BASE_INFO baseInfo, *pBaseInfo = &baseInfo;
    PARAM_CONFIG_NETWORK netWork, *pNetWork = &netWork;
    PARAM_CONFIG_DDNS Ddns;

    char ifname[] = IFNAME;

    ParamGetBaseInfo(pBaseInfo);
    ParamGetNetwork(pNetWork);
    ParamGetDdns(1, &Ddns);

    Memset( pDevFind, 0, sizeof(DEV_FIND_T) );

    pDevFind->realChannelNum =  REAL_CHANNEL_NUM;
    pDevFind->serverType = atoi(pBaseInfo->devModel);
    pDevFind->portNo = pNetWork->port.msgPort;
    pDevFind->netType = 0;
    Strncpy(pDevFind->hostName, pBaseInfo->devName, 32);

    NetGetMacAddr(ifname, pDevFind->macAddr, NET_ADDRSIZE);
    NetGetIpAddr(ifname, pDevFind->ipAddr, NET_ADDRSIZE);
    NetGetMask(ifname, pDevFind->netMask, NET_ADDRSIZE);
    NetRouteGetGateway(ifname, pDevFind->gwAddr, NET_ADDRSIZE);
    NetGetDns(pDevFind->firstDns, pDevFind->secondDns, NET_ADDRSIZE);

    Strncpy(pDevFind->versionNo, pBaseInfo->softwareVersion, 32);
    pDevFind->portOffset = 0;
    pDevFind->dnsGetFlag = Ddns.enable ? 0 : 1;
    pDevFind->webPort = pNetWork->port.webPort;
    pDevFind->dhcpFlag = pNetWork->wired.dhcpEnable;
    Strncpy(pDevFind->deviceType,pBaseInfo->hardwareVersion, 32);

    return 0;
};

/***********************************************************************
* 设置端口广播广播
************************************************************************/
static int SetBroadcast(int sock)
{
    int ret;
    int soBroadcast = 1;
    
    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &soBroadcast, sizeof(soBroadcast));
    if(-1 == ret)
    {
        SVPrint("set broadcast option err\n");
    }
    
    return ret;
}

int BrocastBuffer(char *buf, int num)
{
    int ret;
    int sock;
    char ipAddr[20];
    char ifname[] = IFNAME;
    char broadcastip[] = "255.255.255.255";
    
    NetGetIpAddr(ifname, ipAddr, NET_ADDRSIZE);
    SocketUdpListenAddr(&sock, ipAddr, PORT);
    if(0 > sock)
    {
        SVPrint("BrocastBuffer socket err");
        return -1;
    }

    ret = SetBroadcast(sock);
    if(0 == ret)
    {
        ret = UdpSendto(sock, broadcastip, PORT,  (unsigned char *)buf, num);
        if(0 > ret)
        {
            SVPrint("BrocastBuffer send err");
        }    
    }
    else
    {
        SVPrint("Set broadcast failed!\n");
    }

    Close(sock);
    return ret;
}


#if 1
//广播板子的信息
static void *ServerListenBroadcast(void *arg)
{
    int ret = -1;
    int sock;
    char bufRecv[20];
    struct sockaddr_in fromAddr;
    int sockLen;
    int reSocket = 1;
    char ipfound[] = IP_FOUND;

    sockLen = sizeof(struct sockaddr_in);    

	CORRECTPRINT( "@@@@@@@@thread:%s start@@@@@@@@\r\n", __FUNCTION__ );
    while(g_broadcastServerTm.runFlag)
    {
        if(reSocket)
        {
            ret = SocketUdpListen(&sock, PORT);
            reSocket = 0;
            if(ret < 0)
            {
                ERRORPRINT("ServerListenBroadcast listen fail!");
                return NULL;
            }
        }
        ret = SelectRead(sock, 2000);
        switch(ret)
        {
            case -1:
                SVPrint("select error!\n");
                g_broadcastServerTm.runFlag = 0;
                break;
            case 0:
                //ERRORPRINT("time out!\n");
                break;
            default:
                Memset(bufRecv, 0, sizeof(bufRecv));
                ret = recvfrom(sock, bufRecv, sizeof(bufRecv), 0, (struct sockaddr *)&fromAddr, (socklen_t *)&sockLen);
                if(0 > ret)
                {
                    ERRORPRINT("server recieve err");
                    g_broadcastServerTm.runFlag = 0;
                    break;
                }
                
                SVPrint("bufRecv : %s\n", bufRecv);
                
                //如果与IP_FOUND吻合
                if( Strstr(bufRecv, ipfound) )
                {
                    ColorPrint(COLORPURPLE,"ipfound client IP is %s\n", inet_ntoa(fromAddr.sin_addr));
                    Close(sock);
                    reSocket = 1;
                    GetAllDevFindInfo(&g_devFind);
                    BrocastBuffer((char *)&g_devFind, sizeof(g_devFind));
                }
                break;
        }
    }

	ERRORPRINT( "@@@@@@@@thread:%s stop@@@@@@@@\r\n", __FUNCTION__ );
    Close(sock);
    return NULL;
}
#endif
    
void StartBroadcastServerThread()
{
    int ret;
    g_broadcastServerTm.runFlag = 1;
    ret = ThreadCreate( &g_broadcastServerTm.id, ServerListenBroadcast, NULL );
    if( 0!= ret )
    {        
        g_broadcastServerTm.runFlag = 0;
        ERRORPRINT( "error:ThreadCreate:%s\r\n", STRERROR_ERRNO );
    }
}

void StopBroadcastServerThread()
{
    int ret;
    g_broadcastServerTm.runFlag = 0;
    ret = ThreadJoin( g_broadcastServerTm.id, NULL );
    if( 0 != ret )
    {
        SVPrint( "error:ThreadJoin:%s\r\n", STRERROR_ERRNO );
    }
}

