
#include "ArchDef.h"

void Uint32Set(uint32* pAddr, uint32 value, uint32 mask)
{
	int i = 0;
	for(i = 0; i < 32; i++)
	{
		if((mask >> i) & 0x1)
		{
			if((value >> i) & 0x1)
			{
				*pAddr |= (1 << i);
			}
			else
			{
				*pAddr &= ~(1 << i);
			}
		}
	}
}

#ifdef WIN32
CRITICAL_SECTION g_CriticalSection;
int Osa_Init()
{
	InitializeCriticalSection(&g_CriticalSection);

	return 0;
}

int Osa_ioctrl(uint32 hSocket, int value)
{
	int sock_argp = value;
	return ioctlsocket(hSocket, FIONBIO, &sock_argp);
}

int Osa_Recvfrom(uint32 hSocket, void* pBuff, int bufLen, int nFlags, struct sockaddr_in* pSocketAddr, int* pSocketAddrLen)
{
	Osa_ioctrl(hSocket, 1);
	return recvfrom(hSocket, (char*)pBuff, bufLen, nFlags, (struct sockaddr*)pSocketAddr, pSocketAddrLen);
}
int Osa_CreateTask(TaskFun task, void *p_arg, const char *pTaskName, uint32* pStkBuf, int stackSize)
{
	uint32 dwThreadId = 0;
		
	HANDLE m_hTimerThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)task, p_arg, NULL, &dwThreadId);
	return (int)m_hTimerThread;
}

#endif

#if UCOS
int Osa_CreateTask(TaskFun task, void *p_arg, const char *pTaskName, uint32* pStkBuf, int stackSize)
{
	int nRet = 0;
	static unsigned char prioLevel = 40;
	
	nRet = OSTaskCreate(task, p_arg, &pStkBuf[stackSize / 4 - 1], prioLevel++);
	if(nRet)
	{
		Printf("%s create failed, ret=%d.\n", pTaskName, nRet);
	}
	return nRet;
}

int Osa_ioctrl(uint32 hSocket)
{
	int sock_argp = 1;
	return ioctlsocket(hSocket, FIONBIO, &sock_argp);
}

#endif	

#if VXWORKS_VERSION_55
extern int gWgTskPriorityBase;
int Osa_CreateTask(TaskFun task, void *p_arg, const char *pTaskName, uint32* pStkBuf, int stackSize)
{
	int32 tid;
	int32 default_priority = gWgTskPriorityBase;
	int32 default_option= VX_FP_TASK;
	if(0 == stackSize)
	{
		stackSize = 20000;
	}

	tid = taskSpawn((char*)pTaskName, default_priority, default_option, stackSize, (FUNCPTR)task, (int)p_arg, 0,0,0,0,0,0,0,0,0);
	if(ERROR == tid)
	{
		Printf("%s create failed\n", pTaskName);
		return FALSE;
	}

	return True;
}

int Osa_ioctrl(uint32 hSocket)
{
	int sock_argp = 1;
	return ioctl((int)hSocket, FIONBIO, (int)&sock_argp);
}

int Osa_Recvfrom(uint32 hSocket, void* pBuff, int bufLen, int nFlags, struct sockaddr_in* pSocketAddr, int* pSocketAddrLen)
{
	Osa_ioctrl(hSocket);
	return recvfrom(hSocket, (char*)pBuff, bufLen, nFlags, (struct sockaddr*)pSocketAddr, pSocketAddrLen);
}

#elif defined (VXWORKS_VERSION_66)
int Osa_CreateTask(TaskFun task, void *p_arg, const char *pTaskName, uint32* pStkBuf, int stackSize)
{
	int32 tid;
	int32 default_priority = 100;
	int32 default_option= VX_FP_TASK;
	if(0 == stackSize)
	{
		stackSize = 20000;
	}

	tid = taskSpawn((char*)pTaskName, default_priority, default_option, stackSize, (FUNCPTR)task, (int)p_arg, 0,0,0,0,0,0,0,0,0);
	if(ERROR == tid)
	{
		Printf("%s create failed\n", pTaskName);
		return FALSE;
	}

	return True;
}

int Osa_ioctrl(uint32 hSocket)
{
	int sock_argp = 1;
	return ioctl(hSocket, FIONBIO, &sock_argp);
}
#endif

