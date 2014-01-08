
#include "SwTimer.h"

void SwTimer_Init(SwTimer* pTimer, TimeoutFun timeout, void* pContext)
{
	memset(pTimer, 0, sizeof(SwTimer));

	pTimer->m_base.m_isUsed = 1;
	pTimer->m_context = pContext;
	pTimer->timeout = timeout;
	pTimer->m_bTimerId = 0;
}

void SwTimer_Start(SwTimer* pTimer, uint8 timerId, uint32 value_ms)
{
	Assert(value_ms);
	
	pTimer->m_dwTimeoutTicks = value_ms * MS_STICKS;
	if(pTimer->m_pTimerManager)
	{
		pTimer->m_dwInitTicks = pTimer->m_pTimerManager->m_ticks;
	}
	pTimer->m_bTimerId = timerId;
	pTimer->m_isStart = 1;

	PF(DL_TIMER,("Init Timer(ID=%d), Timerout ticks=%d, initTicks=0x%08x\n", pTimer->m_bTimerId, pTimer->m_dwTimeoutTicks, pTimer->m_dwInitTicks));
}

void SwTimer_ReStart(SwTimer* pTimer)
{
	pTimer->m_dwInitTicks = pTimer->m_pTimerManager->m_ticks;
	pTimer->m_isStart = 1;
}

void SwTimer_Stop(SwTimer* pTimer)
{
	pTimer->m_isStart = 0;
}

void SwTimer_Release(SwTimer* pTimer)
{
	memset(pTimer, 0, sizeof(SwTimer));
}


Bool SwTimer_isStart(SwTimer* pTimer)
{
	return pTimer->m_isStart;
}

static Bool SwTimer_isTimerOut(SwTimer* pTimer, uint32 ticks)
{
	uint32 totalTicks = 0;
	
	if(ticks < pTimer->m_dwInitTicks)
	{
		//tick counter overflow
		totalTicks = 0xFFFFFFFF - pTimer->m_dwInitTicks + ticks;
	}
	else
	{
		totalTicks = ticks - pTimer->m_dwInitTicks;
	}
	
	return (totalTicks >= pTimer->m_dwTimeoutTicks);
}

void SwTimer_TimerOut(SwTimer* pTimer)
{
	Assert(pTimer->timeout);
	pTimer->timeout(pTimer, pTimer->m_context);
}

void SwTimer_Reset(SwTimer* pTimer)
{
	if(pTimer->m_pTimerManager)
	{
		pTimer->m_dwInitTicks = pTimer->m_pTimerManager->m_ticks;
	}
}

uint8 SwTimer_GetId(SwTimer* pTimer)
{
	return pTimer->m_bTimerId;
}

////////////////////////////////////////////////////////////////////////////////

void TimerManager_Init(TimerManager* pTm)
{
	memset(pTm, 0, sizeof(TimerManager));
	#ifdef TIMER_TEST
	SwTimer_Test();
	#endif
}

void TimerManager_AddTimer(TimerManager* pTm, SwTimer* pTimer)
{
	pTimer->m_pTimerManager = pTm;
	if(Null == pTm->m_timerList)
	{
		pTm->m_timerList = pTimer;
	}
	else
	{
		List_AddTail((List*)pTm->m_timerList, (List*)pTimer);
	}
}	

void TimerManager_RemoveTimer(SwTimer* pTimer)
{
	TimerManager* pTm = pTimer->m_pTimerManager;
	
	if(Null == pTm || Null == pTm->m_timerList) return;
	
	if(List_isIn((List*)pTm->m_timerList, (List*)pTimer))
	{
		pTm->m_timerList = (SwTimer*)List_Remove((List*)pTimer);
	}
}

void TimerManager_ResetTimer(TimerManager* pTm, uint32 ticks)
{
	SwTimer* pTimer = (SwTimer*)pTm->m_timerList;

	pTm->m_ticks = ticks;
	for(;pTimer != Null; pTimer = (SwTimer*)(((List*)pTimer)->m_pNext))
	{
		if(SwTimer_isStart(pTimer))
		{
			SwTimer_Reset(pTimer);
		}
	}
}

void TimerManager_Run(TimerManager* pTm, uint32 ticks)
{
	//TimerManager* pTm = GetTimerManager();
	List* pListNode = (List*)pTm->m_timerList;
	SwTimer* pTimer = (SwTimer*)pListNode;

	pTm->m_ticks = ticks;
	
	if(Null == pListNode)
	{
		return;
	}
	
	do
	{
		if(SwTimer_isStart(pTimer))
		{
			if(SwTimer_isTimerOut(pTimer, ticks))
			{
				SwTimer_Stop(pTimer);
				SwTimer_TimerOut(pTimer);
			}
		}
		
		pListNode = pListNode->m_pNext;
		pTimer = (SwTimer*)pListNode;
		
	}while(pListNode);
	
}
