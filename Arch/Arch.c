
#include "Arch.h"

static Arch __g_Arch;
Arch* g_pArch = &__g_Arch;

void Arch_TimerTask(void * arg)
{
#if TIMER_TEST
	uint32 tick1;
	uint32 tick2;
#endif		
	Arch* pArch = (Arch*)arg;
	uint32 ticks = GET_TICKS();
	TimerManager_ResetTimer(&pArch->m_TimerManager, ticks);
	while(1)
	{
		ticks = GET_TICKS();
		TimerManager_Run(&pArch->m_TimerManager, ticks);

#if TIMER_TEST
		tick1 = GET_TICKS();
#endif		
		SLEEP(TIMER_TASK_INTERVAL);

#if TIMER_TEST
		tick2 = GET_TICKS();
		Printf("tick2=%d, tick1=%d, INTERVAL(%d) = tick2-tick1=%d\n", tick2, tick1, TIMER_TASK_INTERVAL, tick2-tick1);
#endif		
	}
}

void Arch_StartTimerTask()
{
	uint32* pStackBuf = Null;
	int nSize = 0;

	Osa_CreateTask((TaskFun)Arch_TimerTask, g_pArch, "ArchTimer", pStackBuf, nSize);
}

Bool Arch_PostMsg(void* pObj, uint32 msgID, uint32 param1, uint32 param2)
{
	Bool bRet = False;
	Message msg;

	LOCK();
	msg.m_pHandler 	= pObj;
	msg.m_MsgID 	= msgID;
	msg.m_Param1 	= param1;
	msg.m_Param2 	= param2;
	bRet = QUEUE_add(&g_pArch->m_MsgQueue, &msg, sizeof(Message));
	UNLOCK();
	
	return bRet;
}

void Arch_AddMsgIf(MsgIf* pMsgIf)
{
	if(Null == g_pArch->m_pMsgIfList)
	{
		g_pArch->m_pMsgIfList = (List *)pMsgIf;
	}
	else
	{
		List_AddTail(g_pArch->m_pMsgIfList, (List *)pMsgIf);
	}
}

void Arch_RemoteMsgIf(MsgIf* pMsgIf)
{
	g_pArch->m_pMsgIfList = List_Remove((List*)pMsgIf);
}

void Arch_TimerOut(SwTimer* pTimer, MsgIf* pMsgIf)
{
	if(pMsgIf->MsgHandler)
	{
		Arch_PostMsg(pMsgIf, MSG_TIMEOUT, pTimer->m_bTimerId, 0);
	}
}

void Arch_TimerStart(SwTimer* pTimer, uint8 timerId, uint32 value_ms, MsgIf* pMsgIf)
{
	if(Null == pTimer->m_pTimerManager)
	{
		SwTimer_Init(pTimer, (TimeoutFun)Arch_TimerOut, pMsgIf);
		TimerManager_AddTimer(&g_pArch->m_TimerManager, pTimer);
	}
	else
	{
		pTimer->m_context = pMsgIf;
		pTimer->timeout = (TimeoutFun)Arch_TimerOut;
	}
	
	SwTimer_Start(pTimer, timerId, value_ms);
}

void Arch_MsgProc(Arch* pArch, uint32 msgId, uint32 param1, uint32 param2)
{
	//MsgIf_MsgProc(pArch, msgId, param1, param2, msgTbl, sizeof(msgTbl) / sizeof(MsgMap));
}

void Arch_Run()
{
	Message* pMsg = Null;
	MsgIf* pMsgIf = Null;
	List* pNode = Null;
	
	PF_FUN(DL_MAIN);
	
    while(1)
    {			
		pMsg = (Message*)QUEUE_getHead(&g_pArch->m_MsgQueue);
		if(pMsg) 
		{
			pMsgIf = (MsgIf*)pMsg->m_pHandler;
			Assert(pMsgIf && pMsgIf->MsgHandler);
			if(pMsgIf && pMsgIf->MsgHandler)
			{
				pMsgIf->MsgHandler(pMsgIf, pMsg->m_MsgID, pMsg->m_Param1, pMsg->m_Param2);
			}
			
			LOCK();
            QUEUE_removeHead(&g_pArch->m_MsgQueue);
			UNLOCK();
		}

		for(pNode = (List*)&g_pArch->m_pMsgIfList; Null != pNode; pNode = pNode->m_pNext)
		{
			pMsgIf = (MsgIf*)pNode;
			if(pMsgIf->Run)
			{
				pMsgIf->Run(pMsgIf);
			}
		}

		SLEEP(1);
    }   
}

int Arch_Start()
{
	Arch_StartTimerTask();

	return RC_SUCCESS;
}

int Arch_Init()
{
	MsgIf* pMsgIf = (MsgIf*)g_pArch;
	
	memset(g_pArch, 0, sizeof(Arch));
	
	PfSocket_Init(PF_TO_IP, PF_TX_PORT, PF_RX_PORT, Null);
	PfSocket_Start();
	
	PF_FUN(DL_MAIN);
	
	Debug_Init();
	
	TimerManager_Init(&g_pArch->m_TimerManager);
	
	//Init Message Queue
	QUEUE_init(&g_pArch->m_MsgQueue
		, (uint8*)&g_pArch->m_MessageBuff
		, sizeof(Message)
		, MAX_MSG_COUNT
		);

	pMsgIf->MsgHandler = (MsgProcFun)Arch_MsgProc;

	Osa_Init();
	
	return RC_SUCCESS;
}

