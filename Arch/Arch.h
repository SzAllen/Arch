#ifndef  _DEBUG_H_
#define  _DEBUG_H_

#ifdef __cplusplus
extern "C"{
#endif	//#ifdef __cplusplus

#include "ArchDef.h"
#include "Queue.h"
#include "SwTimer.h"
#include "Message.h"

typedef struct _Arch
{
	MsgIf m_Base;
	
	List* m_pMsgIfList;
	
	TimerManager	m_TimerManager;
	
	Queue	m_MsgQueue;
	Message	m_MessageBuff[MAX_MSG_COUNT];
}Arch;

int Arch_Init(void);
int Arch_Start(void);
void Arch_Run(void);
Bool Arch_PostMsg(void* pObj, uint32 msgID, uint32 param1, uint32 param2);
void Arch_AddMsgIf(MsgIf* pMsgIf);
void Arch_TimerStart(SwTimer* pTimer, uint8 timerId, uint32 value_ms, MsgIf* pMsgIf);
void Arch_RemoveMsgIf(MsgIf* pMsgIf);

#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus

#endif //#ifndef  _DEBUG_H_

