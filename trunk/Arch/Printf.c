
#include "ArchDef.h"

#ifdef UCOS
	extern uint32_t UART_Send(LPC_USARTn_Type *UARTx, uint8_t *txbuf,	uint32_t buflen, TRANSFER_BLOCK_Type flag);
#endif

#if CONFIG_PFSOCKET
	#if DEFAULT_TO_STD
	PrintfFun g_printFn = __Printf;
	#else
	PrintfFun g_printFn = PfSocket_Out;
	#endif
#else
	PrintfFun g_printFn = __Printf;
#endif

#ifdef WIN32
PrintfFun TraceEx = Null;
#endif

int vsnprintfEx(char* buffer, int bufLen, const char* lpszFormat, va_list ptr)
{
	if(DL_SHOW_TICKS & g_dwDebugLevel)
	{
		int i = 0;
		int nLen = 0;
		static Bool isNewLine = True;
		
		if(isNewLine)
		{
			if('\n' == lpszFormat[0])
			{
				buffer[0] = '\n';
				i = 1;
			}
			nLen = sprintf(&buffer[i], "[%08x]:", GET_TICKS()) + i;
		}
		
		if(strlen(lpszFormat) >= 1)
		{
			isNewLine = (lpszFormat[strlen(lpszFormat) - 1] == '\n');
		}
		
		nLen = vsnprintf(&buffer[nLen], bufLen - nLen, &lpszFormat[i], ptr) + nLen;

		return nLen;
	}
	else
	{
		return vsnprintf(buffer, bufLen, lpszFormat, ptr);
	}
}

char g_Pfbuffer[MAX_PFBUFF_SIZE];
int __Printf(const char* lpszFormat, ...)
{
	int nLen = 0;
	va_list ptr;

	LOCK();

	memset(g_Pfbuffer, 0, sizeof(g_Pfbuffer));
	va_start(ptr, lpszFormat);
	nLen = vsnprintfEx(g_Pfbuffer, sizeof(g_Pfbuffer), lpszFormat, ptr);
	va_end(ptr);
	
	#ifdef UCOS
	UART_Send(LPC_USART0, (uint8_t *)g_Pfbuffer, nLen, BLOCKING);
	#elif __VXWORKS_OS__
	printf("%s", g_Pfbuffer);
	#elif WIN32
	if(TraceEx) TraceEx("%s", g_Pfbuffer);
	#endif
	
	UNLOCK();

	return nLen;
}

#if CONFIG_PFSOCKET
void Pf(uint8 isToPfSocket)
{
	if(isToPfSocket)
	{
		//printf("printf to PfSocket.\n");
		g_printFn = PfSocket_Out;
	}
	else
	{
		g_printFn = (PrintfFun)__Printf;
		//printf("printf to stdio.\n");
	}
}
#endif

/*---------------------------------------------------*/

