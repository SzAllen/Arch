
#if CONFIG_PFSOCKET
#ifndef  _PF_SOCKET_H_
#define  _PF_SOCKET_H_

#ifdef __cplusplus
extern "C"{
#endif
	
#include "ArchDef.h"
#include "Queue.h"
	
typedef void (*CmdLineFn)(const char* string, int len);
typedef struct _PfSocket
{
	uint16 m_RxPort;
	uint16 m_TxPort;
	char m_IpAddr[16];
	
	uint8 m_Buff[PF_BUF_SIZE];
	Queue m_Queue;

	CmdLineFn	CmdLine;
}PfSocket;

int PfSocket_Out(const char* lpszFormat, ...);
void PfSocket_Init(const char* g_IpAddr, uint16 rxPort, uint16 txPort, CmdLineFn pFn);
void PfSocket_Start(void);

#ifdef __cplusplus
}
#endif

#endif             

#endif
 
