#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "const.h"
#include "debug.h"
#include "proconH264.h"
#include "rs_rtsp_commonLib.h"

struct sdp_session_level {
	int sdp_version;      /**< protocol version       :   (currently 0) */
	int id;               /**< session id */
	int version;          /**< session version       :   */
	int start_time;       /**< session start time (NTP time, in seconds),
            	or 0 in case of permanent session */
	int end_time;         /**< session end time (NTP time, in seconds),
            	or 0 if the session is not bounded */
	int ttl;              /**< TTL, in case of multicast stream */
	const char *user;     /**< username of the session's creator */
	const char *src_addr; /**< IP address of the machine from which the session was created */
	const char *dst_addr; /**< destination IP address (can be multicast) */
	const char *name;     /**< session name (can be an empty string) */
};

static int  sdp_write_header(char *buff, int size, struct sdp_session_level *s)
{
	int len;

	len = snprintf(buff, size, "v=%d\r\n"
            "o=- %d %d IN IPV4 %s\r\n"
            "t=%d %d\r\n"
            "s=%s\r\n"
            "a=control:*\r\n"
            "a=range:npt=0- \r\n"
            "c=IN IP4 %s\r\n",
        	s->sdp_version,
        	s->id, s->version, s->src_addr,
        	s->start_time, s->end_time,
        	s->name[0] ? s->name : "No Name",
        	s->dst_addr);

	return len;
}

//base64编码函数
/***************************************************
功能：base64编码函数
返回：void
***************************************************/
static void base64_encode(char *in, const int in_len, char *out, int out_len)
{
	static const char *codes ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int base64_len = 4 * ((in_len+2)/3);    
	char *p = out;
    int times = in_len / 3;
    int i;
    
	if(out_len < base64_len)
    {
    	SVPrint("out_len(%d) < base64_len(%d)!\n", out_len, base64_len);
    	return;
    }

    for(i=0; i<times; ++i)
       {
    *p++ = codes[(in[0] >> 2) & 0x3f];
            *p++ = codes[((in[0] << 4) & 0x30) + ((in[1] >> 4) & 0xf)];
            *p++ = codes[((in[1] << 2) & 0x3c) + ((in[2] >> 6) & 0x3)];
        *p++ = codes[in[2] & 0x3f];
            in += 3;
    }
    if(times * 3 + 1 == in_len) 
    {
        *p++ = codes[(in[0] >> 2) & 0x3f];
                *p++ = codes[((in[0] << 4) & 0x30) + ((in[1] >> 4) & 0xf)];
                *p++ = '=';
                *p++ = '=';
    }
	if(times * 3 + 2 == in_len) 
    {
               *p++ = codes[(in[0] >> 2) & 0x3f];
               *p++ = codes[((in[0] << 4) & 0x30) + ((in[1] >> 4) & 0xf)];
               *p++ = codes[((in[1] << 2) & 0x3c)];
               *p++ = '=';
       }
       *p = 0;
}        

static int sdp_write_videomedia(int ch, int ch_type, char *buf, int size)
{
	int len = 0;
	char *ptr;
	char profile_level_id[8]= {0};
	char h264_param_set[1024];
	char seq_paramBase64[512] = {0};
	char pic1_paramBase64[512] = {0};
	char pic2_paramBase64[512] = {0};
#if 1
	if (get_h264SeqParam(ch, ch_type, h264_param_set, &len, 0) == 0)
    {
    	sprintf(profile_level_id, "%02x%02x%02x", h264_param_set[1], h264_param_set[2], h264_param_set[3]);
    }
	else
    {
    	strcpy(profile_level_id, "42E00D");
    }
	base64_encode(h264_param_set, len, seq_paramBase64, 512);
	if (get_h264PicParam(ch, ch_type, h264_param_set, &len, 0) == 0)
    {
    	base64_encode(h264_param_set, len, pic1_paramBase64, 512);
    }
	if (get_h264PicParam(ch, ch_type, h264_param_set, &len, 1) == 0)
    {
    	base64_encode(h264_param_set, len, pic2_paramBase64, 512);
    }
	SVPrint("####pic1_paramBase64 = %s pic2_paramBase64 =%s \n", pic1_paramBase64, pic2_paramBase64);
#else
	strcpy(profile_level_id, "42C00B");
	strcpy(seq_paramBase64, "Z0LAC5JUFidCAAADAAIAAAMAIR4oIR4=");
	strcpy(pic1_paramBase64, "aM4yaj==");
	strcpy(pic2_paramBase64, "aM4yaj==");
#endif	
	ptr = buf;
	len = sprintf(ptr, "m=video 0 RTP/AVP 96\r\n");
	ptr += len;
	len = sprintf(ptr, "a=rtpmap:96 H264/90000\r\n");
	ptr += len;
//	len = sprintf(ptr, "a=fmtp:96 profile-level-id=%s; packetization-mode=1;sprop-parameter-sets=%s,%s,%s\r\n", 
//        	profile_level_id, seq_paramBase64, pic1_paramBase64, pic2_paramBase64);
	len = sprintf(ptr, "a=fmtp:96 profile-level-id=%s; packetization-mode=1;sprop-parameter-sets=%s,%s\r\n", 
        	profile_level_id, seq_paramBase64, pic1_paramBase64);
	ptr +=len;
	len = sprintf(ptr, "a=control:trackID=0\r\n");
	ptr += len;
    
    // for audio
	len = sprintf(ptr,"m=audio 0 RTP/AVP 8\r\n");
	ptr += len;
	len = sprintf(ptr,"a=control:trackID=1\r\n");
	ptr += len;
	len = sprintf(ptr,"a=rtpmap:8 PCMA/8000\r\n");
	ptr += len;
	len = strlen(buf);
    
	return len;
}

static int prepare_sdp_description(RtspSession_t *pSess, uchar *pbuffer )
{
	struct sdp_session_level s;
	char *buf =pbuffer;
	int len;
	struct sockaddr_in my_addr, dest_addr;
	int channel = pSess->streamNo;
	int ch_type = pSess->channelNo;

#if 1
	memset(&s, 0, sizeof(struct sdp_session_level));
	s.user = "-";
	s.name = "szbeihai";
	s.src_addr = "127.0.0.1"; 

	len = sizeof(my_addr);
	getsockname(pSess->rtspSockFd, (struct sockaddr *)&my_addr, &len);
	len = sizeof(dest_addr);
	getpeername(pSess->rtspSockFd, (struct sockaddr *)&dest_addr, &len);
	s.src_addr = inet_ntoa(dest_addr.sin_addr);
	s.dst_addr = inet_ntoa(my_addr.sin_addr);
	SVPrint( "my_addr.sin_addr = %s, dest_addr.sin_addr = %s \n", s.dst_addr, s.src_addr);

	len = sdp_write_header(buf, 2048, &s);
	buf += len;

	len = sdp_write_videomedia(channel, ch_type, buf, 2048-len);
	buf += len;
#else
/*
v=0
o=- 1373974313479533 1 IN IP4 192.168.0.128
s=H.264 Program Stream, streamed by the LIVE555 Media Server
i=ch0_0.h264
t=0 0

a=DevVer:pusher2
a=GroupName:IPCAM
a=NickName:CIF
a=CfgSection:PROG_CHN0
a=tool:LIVE555 Streaming Media v2011.08.13

a=type:broadcast
a=control:*
a=range:npt=0-
a=x-qt-text-nam:H.264 Program Stream, streamed by the LIVE555 Media Server
a=x-qt-text-inf:ch0_0.h264

m=video 0 RTP/AVP 96
c=IN IP4 0.0.0.0
b=AS:4000
a=rtpmap:96 H264/90000
a=control:trackID=1

a=fmtp:96 packetization-mode=1;profile-level-id=42001F;sprop-parameter-sets=Z0IAH5WoFAFuQA==,aM48gA==
a=framesize:96 1280-720
a=cliprect:0,0,720,1280
a=framerate:25
m=audio 0 RTP/AVP 8

a=rtpmap:8 PCMA/8000
*/
	sprintf( buf, "%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n",
            "v=0",
            "o=- 0 1 IN IP4 192.168.1.165",
            "s=H.264 Program Stream, streamed by the LIVE555 Media Server",
            "i=0",
            "t=0 0" );
	sprintf( buf, "%s%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n", buf,
            "a=DevVer:pusher2",
            "a=GroupName:IPCAM",
            "a=NickName:CIF",
            "a=CfgSection:PROG_CHN0",
            "a=tool:LIVE555 Streaming Media v2011.08.13" );
            
	sprintf( buf, "%s%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n", buf,
            "a=type:broadcast",
            "a=control:*",
            "a=range:npt=0-",
            "a=x-qt-text-nam:H.264 Program Stream, streamed by the LIVE555 Media Server",
            "a=x-qt-text-inf:0" );
            
	sprintf( buf, "%s%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n", buf,
            "m=video 0 RTP/AVP 96",
            "c=IN IP4 0.0.0.0",
            "b=AS:4000",
            "a=rtpmap:96 H264/90000",
            "a=control:trackID=0" );
            
	sprintf( buf, "%s%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n", buf,
            "a=fmtp:96 packetization-mode=1;profile-level-id=42001F;sprop-parameter-sets=Z0IAH5WoFAFuQA==,aM48gA==",
            "a=framesize:96 704-576",
            "a=cliprect:0,0,576,704",
            "a=framerate:25",
            "m=audio 0 RTP/AVP 8",
            "a=rtpmap:8 PCMA/8000",
            "a=control:trackID=1");
    
#endif
	return strlen(pbuffer);
}

int rtsp_cmd_describe(RtspSession_t *pSess, char *pBuf )
{
	uchar content[2048];
	int content_length;

	content_length = prepare_sdp_description( pSess, content );
	if (content_length < 0) {
        //rtsp_reply_error(c, RTSP_STATUS_INTERNAL); // TODO
    	return 0;
    }
	memcpy( pBuf, content, content_length );
	SVPrint("write describe back ok\n");

	return content_length;
}

int Dessdp( RtspSession_t *pSess, char *pBuf )
{
	return rtsp_cmd_describe( pSess, pBuf );
}

