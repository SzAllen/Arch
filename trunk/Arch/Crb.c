#include "Arch.h"
#include "Crb.h"
#include "Debug.h"
#include "Chnl.h"

////////////////////////////////////////////////////////////////////////////

void Crb_TimerStart(Crb* pCrb, uint8 timerId, uint32 value_ms)
{
	pCrb->m_TimerId = timerId;
	Arch_TimerStart(pCrb->m_TimerId, value_ms, (MsgIf*)pCrb);
}

void Crb_TimerStop(Crb* pCrb)
{
	Arch_TimerStop(pCrb->m_TimerId);
	pCrb->m_TimerId = INVALID_TIMER_ID;
}

void Crb_MsgTimerOut(Crb* pCrb, uint8 msgID, uint8 nTimeId, uint32 param2)
{
	Chnl* pChnl = pCrb->m_pChnl;
	
	if(CRB_WAIT_RSP == pCrb->m_State
		|| CRB_RX_REQ_SUCCESS == pCrb->m_State
		|| CRB_TX_REQ == pCrb->m_State
		|| CRB_TX_RSP == pCrb->m_State
		)
	{
		//Chnl_MsgHandler(pChnl, CHNL_TIMEOUT, (uint32)pCrb, 0);
		pChnl->PostMsg((MsgIf*)pChnl, CHNL_TIMEOUT, (uint32)pCrb, (uint32)0);	
	}

	Crb_TimerStop(pCrb);
}

uint32 Crb_GetWaitForRspTime(Crb* pCrb)
{
	CmdReq* pCmdReq = (CmdReq*)QUEUE_getHead(&pCrb->m_CmdPacketQueue);
	if(pCmdReq)
	{
		return pCmdReq->m_WaitForRspMs;
	}

	return 0;
}

void Crb_ConfigRsp(Crb* pCrb, DataPkt* pCmd)
{
	pCrb->m_MaxSendCount = 1;
	
	pCrb->m_State = CRB_READY;
	pCrb->m_ErrorCode = CRB_NO_ERROR;
}

static Bool Crb_SendCmdReq(Crb* pCrb, CmdReq* pCmdReq)
{
	const PktDesc* pPktDesc = Crb_GetPktDesc(pCrb);

	pCrb->m_State 		= CRB_READY;
	pCrb->m_ErrorCode 	= CRB_NO_ERROR;
	pCrb->m_bTxCount 	= 0;
	
	pCrb->m_RspCmd.m_isUsed = 0;

	pCrb->m_MaxSendCount 	= pCmdReq->m_reSendCounter;
	
	pCrb->m_ReqCmd.m_isUsed = 1;
	memcpy(pCrb->m_ReqCmd.m_pData, pCmdReq->m_pData, pCmdReq->m_Len);
	pCrb->m_ReqCmd.m_DataLen = pCmdReq->m_Len;
	
	return Chnl_SendCrb(pCrb->m_pChnl, pCrb);
}

static Bool Crb_PushReq(Crb* pCrb, uint8* pData, int nLen, uint8 reSendCount, uint32 waitForRspMs, CrbNotifyFun CrbDone, void* pRequester)
{
	Bool bFlag = False;
	Queue* pReqDataQueue = &pCrb->m_CmdPacketQueue;
	const PktDesc* pPktDesc = Crb_GetPktDesc(pCrb);
	CmdReq* pCmdReq = Null;

	LOCK();
	pCmdReq = (CmdReq*)QUEUE_getNew(&pCrb->m_CmdPacketQueue);
	if(pCmdReq)
	{
		
		pCmdReq->m_WaitForRspMs 	= waitForRspMs;
		pCmdReq->m_reSendCounter 	= reSendCount;
		pCmdReq->Done 				= CrbDone;
		pCmdReq->m_pRequester 		= pRequester;

		
		memcpy(pCmdReq->m_pData, pData, nLen);
		pCmdReq->m_Len = nLen;
				
		bFlag = True;
	}
	UNLOCK();

	return bFlag;
}

Bool Crb_PushIn(Crb* pCrb, uint8* pData, int len)
{
	Chnl* pChnl = pCrb->m_pChnl;
	Bool isReq = Null;
	DataPkt* pDataPkt = Null;
	int nRet = CHNL_SUCCESS;

	Assert(pChnl);
	Assert(pChnl->m_pPktDesc);
	Assert(pChnl->m_pPktDesc->IsReq);
	
	isReq = pChnl->m_pPktDesc->IsReq(pData, len);
	
	PF(DL_CHNL,("Chnl[%d] Crb[@0x%x] Received %s, len=%d\n", pChnl->m_ChannelD, pData, isReq ? "REQ" : "RSP", len));
	
	if(isReq)
	{
		pDataPkt = &pCrb->m_ReqCmd;
		if(pDataPkt->m_isUsed)
		{
			PF_WARNING(("Busy.\n"));
			//IpmiHeader_Dump(&header, DL_WARNING, _FUNC_, _LINE_);
			nRet = CHNL_BUSY;
		}
		else if(pChnl->m_pPktDesc->m_MaxLen >= len)
		{
			memcpy(pDataPkt->m_pData, pData, len);
			pDataPkt->m_DataLen = len;
			
			pDataPkt->m_isUsed = True;
			//pChnl->PostMsg((MsgIf*)pChnl, CHNL_RX_SUCCESS, (uint32)pCrb, 0);
		}
		else
		{
			PF_WARNING(("%s,line=%d, Data len(=%d) is too long, discard.\n", _FUNC_, _LINE_, len));
			nRet = CHNL_VERIFY_FAILED;
		}
	}
	else
	{
		pDataPkt = &pCrb->m_RspCmd;
		if(pDataPkt->m_isUsed)
		{
			PF_WARNING(("%s,line=%d, Unexpected Response, discard.\n", _FUNC_, _LINE_));
			//IpmiHeader_Dump(&header, DL_WARNING, _FUNC_, _LINE_);
			nRet = CHNL_DISCARD;
		}
		else if(pChnl->m_pPktDesc->IsRspForReq(pCrb->m_ReqCmd.m_pData, pData))
		{
			if(pChnl->m_pPktDesc->m_MaxLen >= len)
			{
				memcpy(pDataPkt->m_pData, pData, len);
				pDataPkt->m_DataLen = len;
				pDataPkt->m_isUsed = True;
				//pChnl->PostMsg((MsgIf*)pChnl, CHNL_RX_SUCCESS, (uint32)pCrb, 0);
			}
			else
			{
				PF_WARNING(("%s,line=%d, Data len(=%d) is too long, discard.\n", _FUNC_, _LINE_, len));
				nRet = CHNL_VERIFY_FAILED;
			}
		}
		else
		{
			//IpmiHeader_Dump(&pCrb->m_pReqCmd->m_Header,DL_MAIN,  Null, 0);
			//IpmiHeader_Dump(&header, DL_MAIN, Null, 0);
			PF_WARNING(("%s,line=%d, Unexpected Response, discard.\n", _FUNC_, _LINE_));
			nRet = CHNL_DISCARD;
		}
	}

	return nRet;
}

Bool Crb_SendReq(Crb* pCrb, uint8* pData, int nLen, uint8 reSendCount, uint32 waitForRspMs, CrbNotifyFun CrbDone, void* pRequester)
{	
	if(!Crb_isIdle(pCrb)) return False;

	return Crb_PostReq(pCrb, pData, nLen, reSendCount, waitForRspMs, CrbDone, pRequester);
}

Bool Crb_PostReq(Crb* pCrb, uint8* pData, int nLen, uint8 reSendCount, uint32 waitForRspMs, CrbNotifyFun CrbDone, void* pRequester)
{
	Queue* pReqDataQueue = &pCrb->m_CmdPacketQueue;
	const PktDesc* pPktDesc = Crb_GetPktDesc(pCrb);
	
	if(pPktDesc->m_MaxLen < nLen) return False;
	if(QUEUE_isFull(pReqDataQueue)) return False;

	Crb_PushReq(pCrb, pData, nLen, reSendCount, waitForRspMs, CrbDone, pRequester);

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

const PktDesc* Crb_GetPktDesc(Crb* pCrb)
{
	return pCrb->m_pChnl->m_pPktDesc;
}

RSP_CODE Crb_CmdDisptch(Crb* pCrb)
{
	return RSP_FAILED;
}

void Crb_ReSendCurrentReq(Crb* pCrb)
{
	pCrb->m_isReSend = 1;
}

void Crb_Done(Crb* pCrb)
{
	Queue* pReqDataQueue = &pCrb->m_CmdPacketQueue;

	Crb_TimerStop(pCrb);

	if(pCrb->m_IsForSendReq)
	{
		CmdReq* pCmdReq = (CmdReq*)QUEUE_getHead(pReqDataQueue);

		pCrb->m_isReSend = 0;
		if(pCmdReq && pCmdReq->Done)
		{
			pCmdReq->Done(pCmdReq->m_pRequester, pCrb, pCrb->m_State);
		}

		if(0 == pCrb->m_isReSend)
		{
			QUEUE_removeHead(pReqDataQueue);
		}

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
	pCrb->m_isPending = False;
}

Bool Crb_IsMatch(Crb* pCrb, uint8* pCmdData)
{
	return True;
}

Crb* Crb_GetMatch(Crb* pCrbList, uint8* pData)
{
	//Search the crb in m_pCrbList;
	while(pCrbList)
	{		
		if(pCrbList->IsMatch(pCrbList, pData))
		{			
			return pCrbList;
		}

		pCrbList = (Crb*)(((List*)pCrbList)->m_pNext);
	}

	return Null;
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
	
	Crb_TimerStop(pCrb);

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
	pCrb->Reset(pCrb);
	Chnl_UnAttachCrb(pCrb->m_pChnl, pCrb);
	
	Arch_TimerRelease(pCrb->m_TimerId);

	memset(pCrb, 0, sizeof(Crb));	
}

int Crb_MsgProc(Crb* pCrb, uint32 msgID, uint32 param1, uint32 param2)
{
	const MsgMap msgTbl[] = 
	{
		{MSG_TIMEOUT, (MsgMapFun)Crb_MsgTimerOut}
	};

	return MsgIf_MsgProc((MsgIf*)pCrb, msgID, param1, param2, msgTbl, sizeof(msgTbl)/sizeof(MsgMap));
}

void Crb_Init(Crb* pCrb
	, uint8*	pReqBuf
	, uint8*	pRspBuf
	, uint16	bufLen
	
	, CmdReq*	pReqArray
	, int 		ReqArrayCount
	
	, uint8*	pReqQueueBuf
	, int 		queueBufSize
	
	, Chnl* 	pChnl
	, Bool 		isForSendReq
	)
{
	int i = 0;
	int nIndex = 0;
	DataPkt* pCmd = Null;
	uint16 nLen = 0;
	uint16 cmdSize = pChnl->m_pPktDesc->m_MaxLen;
	MsgIf* pMsgIf = (MsgIf*)pCrb;

	Assert(cmdSize > 0);
	Assert(bufLen == cmdSize);
	Assert(queueBufSize == ReqArrayCount * cmdSize);

	memset(pCrb, 0, sizeof(Crb));

	MsgIf_Init(pMsgIf, (MsgProcFun)Crb_MsgProc, Null);
	Arch_MsgIfAdd(pMsgIf);
	
	pCrb->m_State 	= CRB_INIT;
	pCrb->m_pChnl = pChnl;
	pCrb->m_IsForSendReq = isForSendReq;

	//³õÊ¼»¯ReqCmdºÍRspCmd
	pCmd = &pCrb->m_ReqCmd;
	DataPkt_Init(pCmd, pReqBuf, bufLen);

	pCmd = &pCrb->m_RspCmd;
	DataPkt_Init(pCmd, pRspBuf, bufLen);

	if(ReqArrayCount)
	{
		for(i = 0; i < ReqArrayCount; i++)
		{
			Assert(nIndex + cmdSize <= queueBufSize);
			pReqArray[i].m_pData = &pReqQueueBuf[nIndex];
			nIndex += cmdSize;
		}
		QUEUE_init(&pCrb->m_CmdPacketQueue, pReqArray, sizeof(CmdReq), ReqArrayCount);
	}
	else
	{
		QUEUE_init(&pCrb->m_CmdPacketQueue, Null, 0, 0);
	}

	Chnl_AttachCrb(pChnl, pCrb);
		
	pCrb->IsMatch 	= Crb_IsMatch;
	pCrb->CrbDone 	= Crb_Done;
	pCrb->Reset 	= Crb_Reset;
	pCrb->PushIn 	= (CrbDataHandleFun)Crb_PushIn;
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

