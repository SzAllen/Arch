
#include "Crb.h"
#include "Debug.h"
#include "Global.h"
#include "Chnl.h"

////////////////////////////////////////////////////////////////////////////

void Crb_TimerOut(SwTimer* pTimer, Crb* pCrb)
{
	Chnl* pChnl = pCrb->m_pChnl;
		
	if(CRB_WAIT_RSP == pCrb->m_State
		|| CRB_RX_REQ_SUCCESS == pCrb->m_State
		|| CRB_TX_REQ == pCrb->m_State
		|| CRB_TX_RSP == pCrb->m_State
		)
	{
		pChnl->PostMsg((MsgIf*)pChnl, CHNL_TIMEOUT, (uint32)pChnl, (uint32)pCrb);	
	}
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
	memcpy(&pCrb->m_ReqCmd.m_pData, pCmdReq->m_pData, pCmdReq->m_Len);
	pCrb->m_ReqCmd.m_DataLen = pCmdReq->m_Len;
	
	pCrb->CmdPacket(pCrb);
	return Chnl_SendCrb(pCrb->m_pChnl, pCrb);
}

static Bool Crb_PushQueue(Crb* pCrb, uint8* pData, int nLen, uint8 reSendCount, CrbNotifyFun CrbDone, void* pRequester)
{
	Queue* pReqDataQueue = &pCrb->m_CmdPacketQueue;
	const PktDesc* pPktDesc = Crb_GetPktDesc(pCrb);
	
	Assert(pCrb->m_pCmdReq->m_Len < pPktDesc->m_MaxLen);
	
	pCrb->m_pCmdReq->m_reSendCounter 	= reSendCount;
	pCrb->m_pCmdReq->Done 				= CrbDone;
	pCrb->m_pCmdReq->m_pRequester 		= pRequester;
	pCrb->m_pCmdReq->m_Len 				= nLen;
	memcpy(pCrb->m_pCmdReq->m_Data, pData, nLen);

	return QUEUE_add(pReqDataQueue, pCrb->m_pCmdReq, nLen);
}

Bool Crb_PushIn(Crb* pCrb, uint8* pData, int len)
{
	Chnl* pChnl = Null;
	Bool isReq = Null;
	DataPkt* pDataPkt = Null;
	int nRet = CHNL_SUCCESS;

	Assert(pCrb->m_pChnl);
	Assert(pChnl->m_pPktDesc);
	Assert(pChnl->m_pPktDesc->IsReq);
	
	pChnl = pCrb->m_pChnl;
	isReq = pChnl->m_pPktDesc->IsReq(pData, len);
	
	PF(DL_CHNL,("Chnl[%d] Crb[%d] Received %s, len=%d: ", pChnl->m_ChannelD, isReq ? "REQ" : "RSP", len));
	
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
			Chnl_PostMsg(pChnl, CHNL_RX_SUCCESS, (uint32)pCrb, 0);
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
		else if(pChnl->m_pPktDesc->IsRspForReq(&pCrb->m_ReqCmd.m_pData, pCrb->m_ReqCmd.m_DataLen, pData, len))
		{
			if(pChnl->m_pPktDesc->m_MaxLen >= len)
			{
				memcpy(pDataPkt->m_pData, pData, len);
				pDataPkt->m_DataLen = len;
				pDataPkt->m_isUsed = True;
				Chnl_PostMsg(pChnl, CHNL_RX_SUCCESS, (uint32)pCrb, 0);
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

Bool Crb_SendReq(Crb* pCrb, uint8* pData, int nLen, uint8 reSendCount, CrbNotifyFun CrbDone, void* pRequester)
{	
	if(!Crb_isIdle(pCrb)) return False;

	return Crb_PostReq(pCrb, pData, nLen, reSendCount, CrbDone, pRequester);
}

Bool Crb_PostReq(Crb* pCrb, uint8* pData, int nLen, uint8 reSendCount, CrbNotifyFun CrbDone, void* pRequester)
{
	Queue* pReqDataQueue = &pCrb->m_CmdPacketQueue;
	const PktDesc* pPktDesc = Crb_GetPktDesc(pCrb);
	
	if(pPktDesc->m_MaxLen < nLen) return False;
	if(QUEUE_isFull(pReqDataQueue)) return False;

	Crb_PushQueue(pCrb, pData, nLen, reSendCount, CrbDone, pRequester);

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
	pCrb->Reset(pCrb);
	Chnl_UnAttachCrb(pCrb->m_pChnl, pCrb);
	
	SwTimer_Release(&pCrb->m_Timer);

	free(pCrb->m_ReqCmd.m_pData);
	free(pCrb->m_RspCmd.m_pData);
	
	if(pCrb->m_pReqBuf)
	{
		free(pCrb->m_pReqBuf);
	}
	
	if(pCrb->m_CmdPacketQueue.m_nBufferSize)
	{
		free(pCrb->m_CmdPacketQueue.m_pBuffer);
	}
	
	memset(pCrb, 0, sizeof(Crb));	
}

void Crb_Init(Crb* pCrb, uint16 maxReqCount, Chnl* pChnl, TimerManager* pTm, Bool isForSendReq)
{
	int i = 0;
	DataPkt* pCmd = Null;
	uint16 nLen = 0;
	uint16	cmdSize = 0;
	uint8*	pBuf = Null;
	int nIndex = 0;
	CmdReq* pCmdReq = Null;

	memset(pCrb, 0, sizeof(Crb));

	Assert(pCrb->m_pChnl);
	Assert(pCrb->m_pChnl->m_pPktDesc);

	cmdSize = pCrb->m_pChnl->m_pPktDesc.m_MaxLen;
	Assert(cmdSize > 0);
	
	pCrb->m_State 	= CRB_INIT;
	pCrb->m_pChnl = pChnl;
	pCrb->m_IsForSendReq = isForSendReq;

	//初始化ReqCmd和RspCmd
	pCmd = &pCrb->m_ReqCmd;
	pBuf = (uint8*)malloc(cmdSize);
	Assert(pBuf);
	memset(pBuf, 0, cmdSize);
	DataPkt_Init(pCmd, pBuf, cmdSize);

	pCmd = &pCrb->m_RspCmd;
	pBuf = (uint8*)malloc(cmdSize);
	Assert(pBuf);
	memset(pBuf, 0, cmdSize);
	DataPkt_Init(pCmd, pBuf, cmdSize);

	if(maxReqCount > 0)
	{
		pCrb->m_pReqBuf = (uint8*)malloc(cmdSize * maxReqCount);
		Assert(pCrb->m_pReqBuf);
		memset(pCrb->m_pReqBuf, 0, cmdSize * maxReqCount);
		
		pCmdReq = (CmdReq*)malloc(sizeof(CmdReq) * maxReqCount);
		Assert(pCmdReq);
		memset(pCmdReq, 0, sizeof(CmdReq) * maxReqCount);
		
		for(i = 0; i < maxReqCount; i++)
		{
			pCmdReq[i].m_pData = &pCrb->m_pReqBuf[nIndex];
			nIndex += cmdSize;
		}
	}
	QUEUE_init(&pCrb->m_CmdPacketQueue, pCmdReq, sizeof(CmdReq), maxReqCount);

	Chnl_AttachCrb(pChnl, pCrb);
	
	//初始化定时器
	SwTimer_Init(&pCrb->m_Timer, (TimeoutFun)Crb_TimerOut, pCrb);
	TimerManager_AddTimer(pTm, &pCrb->m_Timer);
	
	pCrb->IsMatch 	= Crb_IsMatch;
	pCrb->CrbDone 	= Crb_Done;
	pCrb->Reset 	= Crb_Reset;
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

