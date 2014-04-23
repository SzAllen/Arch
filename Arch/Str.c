
#include "Arch.h"
#include "Str.h"

int htoi(const char *s)
{
	int n = 0;

	if( !s ) return 0;

	if( *s == '0' )
	{
		s++;
		if( *s == 'x' || *s == 'X' )s++;
	}

	while(*s != '\0')
	{
		n <<= 4;
		if( *s <= '9' )
			n |= ( *s & 0xf );
		else
			n |= ( (*s & 0xf) + 9 );
		s++;
	}
	return n;
}

char* inet_n2a(uint32 ipAddr)
{
	static char ipStr[16];
	
	memset(ipStr, 0, sizeof(ipStr));
	sprintf(ipStr, "%d.%d.%d.%d",  MEM_BYTE(ipAddr, 0), MEM_BYTE(ipAddr, 1), MEM_BYTE(ipAddr, 2), MEM_BYTE(ipAddr, 3));
	
	return ipStr;
}

int str_add(char* src, uint32 srcBufLen, const char* dst)
{
	if(srcBufLen > strlen(src) + strlen(dst))
	{
		memcpy(&src[strlen(src)], dst, strlen(dst));
		return strlen(src) + strlen(dst);
	}
	else
	{
		return 0;
	}
}

#ifdef CONFIG_STRING

const char* strchr(const char* s, char c)
{	
	while(*s != '\0' && *s != c)
	{
		++s;
	}

	return *s == c ? s : 0;
}

#endif


