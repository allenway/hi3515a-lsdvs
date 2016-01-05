#include "hton.h"
#include <arpa/inet.h>

uint Htonl( uint hostlong )
{
	return htonl( hostlong );
}

ushort Htons( ushort hostshort )
{
	return htons( hostshort );
}

uint Ntohl( uint netlong )
{
	return ntohl( netlong );
}

ushort Ntohs( ushort netshort )
{
	return ntohs( netshort );
}

