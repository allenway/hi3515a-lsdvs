#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__


#define CAMERAFLASHPORT 80 //HTTP
#define CAMERAFLASHIP   "192.168.100.1"

typedef enum
{
    RECORD_ON = 0,
    RECORD_OFF
}e_recordstatus;

typedef enum
{
    MUTE_OFF = 0,    //not mute 
    MUTE_ON          //mute 
}e_mutestatus;

typedef enum
{
    CF_QVGA = 0,
    CF_VGA
}e_cfresolution;

class httpclient
{

public:
    httpclient();
    ~httpclient();

    

private:
    char m_initstatus;
    char m_connectstatus;
    int  m_httpsock;
    int  m_port;
    char m_IP[16];
    char *m_sndbuf;
    
    
        
};

#endif

