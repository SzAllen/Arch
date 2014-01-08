
#include "Crb.h"
#include "Debug.h"
#include "Global.h"

////////////////////////////////////////////////////////////////////////////

void Crb_TimerOut(SwTimer* pTimer, Crb* pCrb)
{
	Chnl* pChnl = pCrb->m_pChnl;
		
	if(CRB_TX_REQ_SUCCESS == pCrb->m_State
		|| CRB_RX_REQ_SUCCESS == pCrb->m_State
		|| CRB_TX_REQ == pCrb->m_State
		|| CRB_TX_RSP == pCrb->m_State
		)
	{
		pChnl->PostMsg((MsgIf*)pChnl, CHNL_TIMEOUT, (uint32)pChnl, (uint32)pCrb);	
	}
}

void Crb_ConfigRsp(Crb* pCrb, CmdItem* pCmd)
{
	pCrb->m_MaxSendCount = 1;
	
	pCrb->m_State = CRB_READY;
	pCrb->m_ErrorCode = CRB_NO_ERROR;
}

static Bool Crb_SendCmdReq(Crb* pCrb, CmdReq* pCmdReq)
{
	const PacketDesc* pPacketDesc = Crb_GetPacketDesc(pCrb);

	pCrb->m_State 		= CRB_READY;
	pCrb->m_ErrorCode 	= CRB_NO_ERROR;
	pCrb->m_bTxCount 	= 0;
	
	pCrb->m_RspCmd.m_isUsed = 0;

	pCrb->m_MaxSendCount 	= pCmdReq->m_reSendCounter;
	pCrb->m_DelayMsForRsp 	= pCmdReq->m_DelayMsForRsp;
	
	pCrb->m_ReqCmd.m_isUsed = 1;
	memcpy(&pCrb->m_ReqCmd.m_pCmd, pCmdReq->m_Data, pCmdReq->m_Len);
	pCrb->m_ReqCmd.m_CmdLen = pCmdReq->m_Len;
	
	return Chnl_SendCrb(pCrb->m_pChnl, pCrb);
}

static Bool Crb_PushQueue(Crb* pCrb, uint8* pData, int nLen, uint8 reSendCount, uint32 nDelayMsForRsp, CrbNotifyFun CrbDone, void* pRequester)
{
	Queue* pReqDataQueue = &pCrb->m_CmdPacketQueue;
	const PacketDesc* pPacketDesc = Crb_GetPacketDesc(pCrb);
	
	Assert(pCrb->m_pCmdReq->m_Len < pPacketDesc->m_MaxLen);
	
	pCrb->m_pCmdReq->m_reSendCounter 	= reSendCount;
	pCrb->m_pCmdReq->m_DelayMsForRsp	= nDelayMsForRsp;
	pCrb->m_pCmdReq->Done 				= CrbDone;
	pCrb->m_pCmdReq->m_pRequester 		= pRequester;
	pCrb->m_pCmdReq->m_Len 				= nLen;
	memcpy(pCrb->m_pCmdReq->m_Data, pData, nLen);

	return QUEUE_add(pReqDataQueue, pCrb->m_pCmdReq, nLen);
}

Bool Crb_SendReq(Crb* pCrb, uint8* pData, int nLen, uint8 reSendCount, uint32 nDelayMsForRsp, CrbNotifyFun CrbDone, void* pRequester)
{	
	if(!Crb_isIdle(pCrb)) return False;

	return Crb_PostReq(pCrb, pData, nLen, reSendCount, nDelayMsForRsp, CrbDone, pRequester);
}

Bool Crb_PostReq(Crb* pCrb, uint8* pData, int nLen, uint8 reSendCount, uint32 nDelayMsForRsp, CrbNotifyFun CrbDone, void* pRequester)
{
	Queue* pReqDataQueue = &pCrb->m_CmdPacketQueue;
	const PacketDesc* pPacketDesc = Crb_GetPacketDesc(pCrb);
	
	if(pPacketDesc->m_MaxLen < nLen) return False;
	if(QUEUE_isFull(pReqDataQueue)) return False;

	Crb_PushQueue(pCrb, pData, nLen, reSendCount, nDelayMsForRsp, CrbDone, pRequester);

	if(Crb_isIdle(pCrb))
	{
		Crb_SendCmdReq(pCrb, (CmdReq*)QUEUE_getHead(pReqDataQueue));
	}

	return True;
}

Bool Crb_IsNeedReSend(Crb* pCrb)
{
	return (pCrb->m_bTxCount < pCrb->m_MaxSendCount) 
		&& (pCrb->m_ErrorCode != CRB_BUS_NOACK)
		&& (pCrb->m_ErrorCode != CRB_BUS_ERROR);
}

Bool Crb_isIdle(Crb* pCrb)
{
	return pCrb->m_State == CRB_INIT;
}

const PacketDesc* Crb_GetPacketDesc(Crb* pCrb)
{
	return pCrb->m_pChnl->m_pPacketDesc;
}

RSP_CODE Crb_CmdDisptch(Crb* pCrb)
{
	return RSP_FAILED;
}

void Crb_Done(Crb* pCrb)
{
	Queue* pReqDataQueue = &pCrb->m_CmdPacketQueue;
	
	SwTimer_Stop(&pCrb->m_Timer);

	if(pCrb->m_IsForSendReq)
	{
		CmdReq* pCmdReq = (CmdReq*)QUEUE_getHead(pReqDataQueue);
		
		if(pCmdReq && pCmdReq->Done)
		{
			pCmdReq->Done(pCmdReq->m_pRequester, pCrb, pCrb->m_State);
		}

		QUEUE_removeHead(pReqDataQueue);

		pCmdReq = (CmdReq*)QUEUE_getHead(pReqDataQueue);
		if(CRB_CANCEL == pCrb->m_State)
		{
			//Remove all req in pCrb;
			while(pCmdReq)
			{
				pCmdReq->Done(pCmdReq->m_pRequester, pCrb, pCrb->m_State);
				QUEUE_removeHead(pReqDataQueue);
				pCmdReq = (CmdReq*)QUEUE_getHead(pReqDataQueue);
			}
		}
		if(pCmdReq)
		{
			Crb_SendCmdReq(pCrb, pCmdReq);
			return;
		}
	}
	
	pCrb->m_State = CRB_INIT;
	pCrb->m_ReqCmd.m_isUsed = 0;
	pCrb->m_RspCmd.m_isUsed = 0;
	pCrb->m_bTxCount = 0;
	pCrb->m_ErrorCode = CRB_NO_ERROR;
}

Bool Crb_IsMatch(Crb* pCrb, uint8* pCmdData)
{
	return True;
}

int Crb_Send(Crb* pCrb)
{
	return Chnl_SendCrb(pCrb->m_pChnl, pCrb);
}

void Crb_Reset(Crb* pCrb)
{	
	Queue* pReqDataQueue = &pCrb->m_CmdPacketQueue;
	CmdReq* pCmdReq = Null;
	
	if(pCrb->m_State != CRB_INIT)
	{
	}
	
	SwTimer_Stop(&pCrb->m_Timer);

	Chnl_CancelCrb(pCrb->m_pChnl, pCrb);
	
	//Remove all req in pCrb;
	pCmdReq = (CmdReq*)QUEUE_getHead(pReqDataQueue);
	while(pCmdReq)
	{
		pCrb->m_State = CRB_CANCEL;
		pCmdReq->Done(pCmdReq->m_pRequester, pCrb, pCrb->m_State);
		QUEUE_removeHead(pReqDataQueue);
		pCmdReq = (CmdReq*)QUEUE_getHead(pReqDataQueue);
	}
		
	pCrb->m_State = CRB_INIT;

	pCrb->m_ReqCmd.m_isUsed = 0;
	pCrb->m_RspCmd.m_isUsed = 0;
	pCrb->m_bTxCount = 0;
	pCrb->m_ErrorCode = CRB_NO_ERROR;

}

void Crb_Release(Crb* pCrb)
{
	CmdItem* pCmd = Null;
	
	pCrb->Reset(pCrb);

	free(pCrb->m_ReqCmd.m_pCmd);
	free(pCrb->m_RspCmd.m_pCmd);
	free(pCrb->m_pCmdReq);
	free(pCrb->m_CmdPacketQueue.m_pBuffer);
	
	SwTimer_Release(&pCrb->m_Timer);
	
	Chnl_UnAttachCrb(pCrb->m_pChnl, pCrb);
	
	memset(pCrb, 0, sizeof(Crb));
	
}

void Crb_Init(Crb* pCrb
	, uint16 			maxCmdCount		//命令队列可以存储最大命令个数
	, TimerManager* 	pTimerManager
	, struct _tagChnl* 	pChnl
	, Bool 				isForSendReq
	, void*				pDisptchObj
	)
{
	uint8* pBuff = Null;
	CmdItem* pCmd = Null;
	uint16 nLen = 0;
	const PacketDesc* 	pPacketDesc = pChnl->m_pPacketDesc;

	Assert(pPacketDesc);
	Assert(maxCmdCount >= 2);
	
	pCrb->m_State 	= CRB_INIT;
	pCrb->m_pChnl = pChnl;
	pCrb->m_IsForSendReq = isForSendReq;
	pCrb->m_pDisptchObj = pDisptchObj;

	//初始化定时器
	SwTimer_Init(&pCrb->m_Timer, (TimeoutFun)Crb_TimerOut, pCrb);
	Timermanager_AddTimer(pTimerManager, &pCrb->m_Timer);
	
	Chnl_AttachCrb(pChnl, pCrb);
	
	//初始化ReqCmd和RspCmd
	{

		pCmd = &pCrb->m_ReqCmd;
		pBuff = (uint8*)malloc(pPacketDesc->m_MaxLen);
		Assert(pBuff);
		CmdItem_Init(pCmd, pBuff, pPacketDesc->m_MaxLen);

		pCmd = &pCrb->m_RspCmd;
		pBuff = (uint8*)malloc(pPacketDesc->m_MaxLen);
		Assert(pBuff);
		CmdItem_Init(pCmd, pBuff, pPacketDesc->m_MaxLen);
		
	}

	//初始化命令队列
	nLen = pPacketDesc->m_MaxLen + sizeof(CmdReq);
	pCrb->m_pCmdReq = (CmdReq*)malloc(nLen);
	Assert(pCrb->m_pCmdReq);
	
	if(maxCmdCount)
	{
		pBuff = (uint8*)malloc(nLen * maxCmdCount);
		Assert(pBuff);
	}

	else
	{
		pBuff = Null;
		nLen = 0;
	}
	QUEUE_init(&pCrb->m_CmdPacketQueue, pBuff, nLen, maxCmdCount);

	pCrb->IsMatch 	= Crb_IsMatch;
	pCrb->CrbDone 	= Crb_Done;
	pCrb->Reset 	= Crb_Reset;
	pCrb->Disptch 	= (CmdDisptchFun)Crb_CmdDisptch;
}

void Crb_VerifyReset(Crb* pCrb)
{
	AssertTrue(pCrb->m_State == CRB_INIT);
	AssertTrue(pCrb->m_ErrorCode == 0);
	AssertTrue(pCrb->m_bTxCount == 0);
	
	AssertTrue(&pCrb->m_ReqCmd);
	AssertTrue(&pCrb->m_RspCmd);
	AssertTrue(pCrb->m_ReqCmd.m_isUsed == False);
	AssertTrue(pCrb->m_RspCmd.m_isUsed == False);
	
	AssertTrue(pCrb->m_pChnl);
}

