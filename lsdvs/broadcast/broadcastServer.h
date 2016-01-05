#ifndef __BROADCASTSERVER_H__
#define __BROADCASTSERVER_H__

void StartBroadcastServerThread();
void StopBroadcastServerThread();


typedef   struct   _DevFind_ 
{ 
    int             realChannelNum;            //本设备可用通道数
    int             serverType;                //设备类型 
    int             portNo;                    //监听端口 
    int             netType;                   //网络类型 
    char            hostName[32];              //设备名 
    char            macAddr[NET_ADDRSIZE];     //MAC地址 
    char            ipAddr[NET_ADDRSIZE];      //IP地址 
    char            netMask[NET_ADDRSIZE];     //网络掩码 
    char            gwAddr[NET_ADDRSIZE];      //网关地址 
    char            versionNo[32];             //版本号 
    unsigned int    portOffset;                //端口偏移 
    char            firstDns[NET_ADDRSIZE];    //主DNS 
    char            secondDns[NET_ADDRSIZE];   //备用DNS 
    unsigned short  dnsGetFlag;                //dns 自动获得标志,0: 自动获得, 1: 手动获得 
    unsigned short  webPort;                   //web 端口配置 
    char            dhcpFlag;
    char            unused[3];
    char            deviceType[32];            //产品型号 
}DEV_FIND_T;

#endif

