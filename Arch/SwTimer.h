#ifndef  _SWTIMER_H_
#define  _SWTIMER_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "ArchDef.h"
#include "List.h"

struct _tagSwTimer;
typedef void (*TimeoutFun)(struct _tagSwTimer*, void* context);
struct _tagTimerManager;
typedef struct _tagSwTimer
{
	List	m_base;
    uint32 	m_dwTimeoutTicks;
    uint32 	m_dwInitTicks;
	
    uint8 	m_bTimerId;
    uint8 	m_isStart;
	
    void* 	m_context;
    TimeoutFun timeout;
	struct _tagTimerManager* m_pTimerManager;
}SwTimer;

typedef struct _tagTimerManager
{
    SwTimer* 	m_timerList;
	uint32		m_ticks;
}TimerManager;

void SwTimer_Init(SwTimer* pTimer, TimeoutFun timeout, void* pContext);
void SwTimer_Stop(SwTimer* pTimer);
void SwTimer_Start(SwTimer* pTimer, uint8 timerId, uint32 value_ms);
void SwTimer_ReStart(SwTimer* pTimer);
Bool SwTimer_isStart(SwTimer* pTimer);
void SwTimer_Reset(SwTimer* pTimer);
uint8 SwTimer_GetId(SwTimer* pTimer);
void SwTimer_Release(SwTimer* pTimer);

void TimerManager_Init(TimerManager* pTm);
void TimerManager_AddTimer(TimerManager* pTm, SwTimer* pTimer);
void TimerManager_Run(TimerManager* pTm, uint32 ticks);
void TimerManager_ResetTimer(TimerManager* pTm, uint32 ticks);
void TimerManager_RemoveTimer(SwTimer* pTimer);

#ifdef __cplusplus
}
#endif

#endif

