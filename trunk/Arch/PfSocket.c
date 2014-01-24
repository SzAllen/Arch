#if CONFIG_PFSOCKET

#include "PfSocket.h"
#include "Printf.h"

static PfSocket __g_PfSocket;
PfSocket* g_pPfSocket=&__g_PfSocket;

void PfSocket_task(void * pArg)
{
	PfSocket* pPfSocket = (PfSocket*)pArg;
	uint32 hSocket;
	int i = 0;
	int nTxLen = 0;
    struct sockaddr_in  server, client;   
	char	revBuf[64] = {0};
	uint32 ipAddr = inet_addr(pPfSocket->m_IpAddr);
	int32 socketAddrLen = sizeof(struct sockaddr_in);
	uint16 nTxPort = htons(pPfSocket->m_TxPort);

	server.sin_family       = AF_INET;                                                           
	server.sin_addr.s_addr  = htonl( INADDR_ANY );   
    server.sin_port         = htons(pPfSocket->m_RxPort);              
   
    hSocket = socket( AF_INET, SOCK_DGRAM, 0 );                          
    if(INVALID_SOCKET == hSocket)
    {
		PF_WARNING(("PfSocket create failed, port=%d.\n", pPfSocket->m_RxPort));
		return;
    }
	else
	{
		i = Osa_ioctrl(hSocket, 1);

		if(0!= i)
		{
			printf("PfSocket Failed to set unblocking mode\n");
			return ;
		}
		
		i = bind(hSocket, (struct sockaddr *)&server, sizeof( server ));
	    if(0 != i)
	    {
			PF_WARNING(("PfSocket bind failed, port=%d, ret=%d.\n", pPfSocket->m_RxPort, i));
			return;
	    }
	}
	
	memset(&client, 0, sizeof(client));
	client.sin_family       = AF_INET;
	
	client.sin_port        = nTxPort;
	client.sin_addr.s_addr = ipAddr;
    while(1)
    {
		memset(revBuf, 0, sizeof(revBuf));
		nTxLen = RECV_FROM(hSocket, revBuf, sizeof(revBuf), 0, &client, &socketAddrLen);
		if(nTxLen > 0)
		{
			if(g_printFn != PfSocket_Out) g_printFn = PfSocket_Out;

			if(g_pPfSocket->CmdLine) g_pPfSocket->CmdLine(revBuf, nTxLen);
		}
		
		i = 0;
		while(!QUEUE_isEmpty(&pPfSocket->m_Queue))
		{
			LOCK();
			i = QUEUE_PushOutEleArray(&pPfSocket->m_Queue, revBuf, sizeof(revBuf) - 1);
			UNLOCK();
			
			nTxLen = SEND_TO(hSocket, revBuf, i, 0 , (struct sockaddr*)&client, sizeof(client));
		}
		SLEEP(1);
	}
}

void PfSocket_Init(const char* g_IpAddr, uint16 rxPort, uint16 txPort, CmdLineFn pFn)
{
	memset(g_pPfSocket, 0, sizeof(PfSocket));
	
	g_pPfSocket->m_RxPort = rxPort;
	g_pPfSocket->m_TxPort = txPort;
	strcpy(g_pPfSocket->m_IpAddr, g_IpAddr);

	g_pPfSocket->CmdLine = pFn;
		
	QUEUE_init(&g_pPfSocket->m_Queue, g_pPfSocket->m_Buff, 1, sizeof(g_pPfSocket->m_Buff));
}

void PfSocket_Start()
{
	uint32* pStackBuf = Null;
	int nSize = 0;
#if UCOS
	static OS_STK g_PfSocketStack[256];
	pStackBuf = (uint32*)&g_PfSocketStack;
	nSize = sizeof(g_PfSocketStack);
#endif

	Osa_CreateTask(PfSocket_task, (void *)g_pPfSocket, "PfSocket", pStackBuf, nSize);
}

int PfSocket_Out(const char* lpszFormat, ...)
{
	int nLen = 0;
	va_list ptr;
	va_start(ptr, lpszFormat);

	if(0 == g_pPfSocket->m_Queue.m_nBufferSize) return 0;

	LOCK();
	
	memset(g_Pfbuffer, 0, sizeof(g_Pfbuffer));
	nLen = vsnprintfEx(g_Pfbuffer, sizeof(g_Pfbuffer), lpszFormat, ptr);
	
	va_end(ptr);

	QUEUE_PushInEleArray(&g_pPfSocket->m_Queue, g_Pfbuffer, nLen);

	UNLOCK();

	return nLen;
}

#endif

