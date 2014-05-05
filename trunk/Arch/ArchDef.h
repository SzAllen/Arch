#ifndef __ARCH_CFG_H_
#define __ARCH_CFG_H_

#ifdef __cplusplus
extern "C"{
#endif

#define MAX_MSG_COUNT	60	//Message queue deepth
#define MAX_TIMER_COUNT	16
#define MS_STICKS 		16	//Hot many ticks per ms, it is for timer
#define TIMER_TASK_INTERVAL 1

#define SEND_TO 	sendto
#define RECV_FROM 	Osa_Recvfrom

#ifdef WIN_32
	#include <windows.h>
	#include <stdio.h>

	#define SLEEP 		Sleep
	#define LOCK()		{if(g_CriticalSection.OwningThread) EnterCriticalSection(&g_CriticalSection)
	#define UNLOCK()	if(g_CriticalSection.OwningThread) LeaveCriticalSection(&g_CriticalSection);}
	#define GET_TICKS() GetTickCount()
	
	extern CRITICAL_SECTION g_CriticalSection;
#else
#define INVALID_SOCKET		0xFFFFFFFF
#endif

#include "typedef.h"	
#include "Debug.h"	
#include "String.h"
#include "MsgDef.h"
#include "RetCode.h"

#ifdef CONFIG_PFSOCKET
	#define PF_BUF_SIZE 4096
	#include "PfSocket.h"
	
	#define PF_TO_IP 	"172.18.17.27"
	#define PF_TX_PORT 	7500
	#define PF_RX_PORT 	7500
#else
	#define PfSocket_Start(...)
	#define PfSocket_Init(...)
	#define PfSocket_Out(...)
#endif

typedef struct _ArchDef
{
	uint8 id;
}ArchDef;

typedef void (*TaskFun)(void* pArg);

int Osa_CreateTask(TaskFun task, void *p_arg, const char *pTaskName, uint32* pStkBuf, int stackSize);
int Osa_ioctrl(uint32 hSocket, int value);
int Osa_Recvfrom(uint32 hSocket, void* pBuff, int bufLen, int nFlags, struct sockaddr_in* pSocketAddr, int* pSocketAddrLen);

int Osa_Init();
#ifdef __cplusplus
}
#endif

#endif




