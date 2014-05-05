
#include "Chnl.h"

#define SET_CRB_TO_TX_LIST(pChnl, pCrb) Chnl_SetCrbToTxList(pChnl, pCrb, True)
#define SET_CRB_TO_IDEL_LIST(pChnl, pCrb) Chnl_SetCrbToTxList(pChnl, pCrb, False)

void Chnl_PostMsg(Chnl* pChnl, uint8 msgID, uint32 param1, uint32 param2)
{
	pChnl->PostMsg((MsgIf*)pChnl, msgID, param1, param2);
}

void Chnl_SetCrbToTxList(Chnl* pChnl, Crb* pCrb, Bool isToPendingList)
{
	if(isToPendingList)
	{
		//Remove crb from m_pCrbList
		if(pChnl->m_pCrbList && List_isIn((List*)pChnl->m_pCrbList, (List*)pCrb))
			pChnl->m_pCrbList = (Crb*)List_Remove((List*)pCrb);
		
		//Put crb in m_pPendingCrbList
		pChnl->m_pPendingCrbList = (Crb*)List_AddTail((List*)pChnl->m_pPendingCrbList, (List*)pCrb);
	}
	else
	{
		pCrb->m_isPending = False;
		//Remove crb from m_pPendingCrbList
		if(pChnl->m_pPendingCrbList && List_isIn((List*)pChnl->m_pPendingCrbList, (List*)pCrb))
		{
			pChnl->m_pPendingCrbList = (Crb*)List_Remove((List*)pCrb);
		}
		
		//Put crb in m_pCrbList
		pChnl->m_pCrbList = (Crb*)List_AddTail((List*)pChnl->m_pCrbList, (List*)pCrb);
	}
}

void Chnl_AttachCrb(Chnl* pChnl, Crb* pCrb)
{
	if(Null == pChnl->m_pCrbList)
	{
		pChnl->m_pCrbList = pCrb;
		return;
	}
	else
	{
		List_AddTail((List*)pChnl->m_pCrbList, (List*)pCrb);
	}
}

void Chnl_UnAttachCrb(Chnl* pChnl, Crb* pCrb)
{
	pCrb->Reset(pCrb);
	pChnl->m_pCrbList = (Crb*)List_Remove((List*) pCrb);
}

void Chnl_AttachTransfer(Chnl* pChnl, Transfer* pTransfer)
{
	pChnl->m_pTransfer = pTransfer;
	pTransfer->m_pArg = pChnl;
}

static void Chnl_Ready(Chnl* pChnl)
{
	if(pChnl->m_pPendingCrbList && !pChnl->m_pPendingCrbList->m_isPending)
	{
		Chnl_PostMsg(pChnl, CHNL_TXDATA, (uint32)pChnl->m_pPendingCrbList, 0);
	}
}

static Crb* Chnl_GetCrb(Chnl* pChnl, uint8* pData)
{
	//Search the crb in m_pCrbList;
	Crb* pCrb = Crb_GetMatch(pChnl->m_pCrbList, pData);
	
	if(Null == pCrb)
	{
		//Search the crb in m_pPendingCrbList;
		pCrb = Crb_GetMatch(pChnl->m_pPendingCrbList, pData);
	}
	
	return pCrb;
}

//Chnl接收到数据
CHNL_RET Chnl_Event(Chnl* pChnl, TransferEvent eventId, uint8* pData, uint16 len)
{
	int nRet = CHNL_SUCCESS;
	Crb* pCrb = (pChnl->m_pPendingCrbList && pChnl->m_pPendingCrbList->m_isPending) ? pChnl->m_pPendingCrbList : Null;

	PF(DL_CHNL, ("Chnl[0x%x] trigger event[%d]\n", pChnl->m_ChannelD, eventId));	
	
	if(pCrb)
	{		
		if(EVENT_TX_SUCCESS & eventId)
		{
			Chnl_PostMsg(pChnl, CHNL_TX_SUCCESS, (uint32)pCrb, 0);
		}
		else if(EVENT_TX_FAILED & eventId)
		{
			Chnl_PostMsg(pChnl, CHNL_TX_FAILED, (uint32)pCrb, 0);
		}
		else
		{
			nRet = CHNL_FAILED;
			goto End;
		}
	}
	
	if(EVENT_RX_SUCCESS & eventId)
	{
		if(Null == pData || 0 == len)
		{
			nRet = CHNL_FAILED;
			goto End;
		}
		
		pCrb = Chnl_GetCrb(pChnl, pData);
		if(pCrb == Null)
		{
			PF_WARNING(("Don't find Crb, discard dataPkt!\n"));
			DUMP_BYTE(pData, len);
			nRet = CHNL_FAILED;
			goto End;
		}

		if(CHNL_SUCCESS == pCrb->PushIn(pCrb, pData, len))
		{
			Chnl_PostMsg(pChnl, CHNL_RX_SUCCESS, (uint32)pCrb, 0);
		}
		else
		{
			PF_WARNING(("Chnl[%d], Discard dataPkt, len=%d.\n", _FUNC_, len));
		}
	}
	else
	{
		nRet = CHNL_FAILED;
	}

End:	
    return nRet;
}

static CHNL_RET Chnl_MsgTimeOut(Chnl* pChnl, Crb* pCrb, uint32 param2)
{
	if(pCrb->m_IsForSendReq)
	{		
		if(CRB_WAIT_RSP == pCrb->m_State)	//Waiting for response time out
		{
			if(Crb_IsNeedReSend(pCrb))
			{
				Chnl_SendCrb(pChnl, pCrb);
				return CHNL_SUCCESS;
			}
			else
			{
				PF_WARNING(("CRB_RX_RSP_TIMEOUT\n"));
				pCrb->m_State = CRB_RX_RSP_FAILED;
				pCrb->m_ErrorCode = CRB_TIMEOUT;
				pCrb->CrbDone(pCrb);
			}
		}
		else if(CRB_WAIT_TX_RESULT == pCrb->m_State)
		{
			SET_CRB_TO_IDEL_LIST(pChnl, pCrb);
			
			if(Crb_IsNeedReSend(pCrb))
			{
				Chnl_SendCrb(pChnl, pCrb);
				return CHNL_SUCCESS;
			}
			else
			{
				PF_WARNING(("CRB_TX_REQ Timer out\n"));
				pCrb->m_State = CRB_TX_REQ_FAILED;
				pCrb->m_ErrorCode = CRB_TIMEOUT;
				pCrb->CrbDone(pCrb);
			}
		}
	}
	else
	{
		if(CRB_WAIT_TX_RESULT == pCrb->m_State)
		{
			SET_CRB_TO_IDEL_LIST(pChnl, pCrb);
			
			PF_WARNING(("CRB_TX_RSP timerOut\n"));
			pCrb->m_State = CRB_TX_RSP_FAILED;
			pCrb->m_ErrorCode = CRB_TIMEOUT;
			pCrb->CrbDone(pCrb);
		}
	}

	Chnl_Ready(pChnl);
	
	return CHNL_SUCCESS;
}

static CHNL_RET Chnl_MsgTxDone(Chnl* pChnl, Crb* pCrb, CHNL_EVENT event)
{
	if(Null == pCrb)
	{
		PF_FAILED_EXPR(("Arg error, pCrb=Null.\n"));
		return CHNL_SUCCESS;
	}
	
	if(CRB_WAIT_TX_RESULT != pCrb->m_State)
	{
		PF_FAILED_EXPR(("State[%d] error.\n", pCrb->m_State));
		return CHNL_SUCCESS;
	}

	SET_CRB_TO_IDEL_LIST(pChnl, pCrb);

	Crb_TimerStop(pCrb);

	if(event == CHNL_TX_FAILED)
	{
		if(pCrb->m_IsForSendReq)
		{
			if(Crb_IsNeedReSend(pCrb))
			{
				Chnl_SendCrb(pChnl, pCrb);
				return CHNL_SUCCESS;
			}
			else
			{
				pCrb->m_State = CRB_TX_REQ_FAILED;
			}
		}
		else //Tx response data failed
		{
			if(CRB_BUS_BUSY == pCrb->m_ErrorCode)
			{
				Chnl_SendCrb(pChnl, pCrb);
				return CHNL_SUCCESS;
			}
			pCrb->m_State = CRB_TX_RSP_FAILED;
			
		}
		
		pCrb->CrbDone(pCrb);
		Chnl_Ready(pChnl);
		return CHNL_SUCCESS;
	}
	
	if(pCrb->m_IsForSendReq)
	{
		pCrb->m_State = CRB_WAIT_RSP;

		//Start a timer to wait the response.
		if(pChnl->m_DelayMsForRsp)
		{
			Crb_TimerStart(pCrb, TIMER_TX_REQ, pChnl->m_DelayMsForRsp);
		}
		else
		{
			pCrb->CrbDone(pCrb);
		}
	}
	else
	{
		pCrb->m_State = CRB_TX_RSP_SUCCESS;
		pCrb->CrbDone(pCrb);
	}

	Chnl_Ready(pChnl);
	
	return CHNL_SUCCESS;
}

static CHNL_RET Chnl_MsgTxData(Chnl* pChnl, Crb* pCrb, uint32 param2)
{
	TX_CODE nRet = TX_SUCCESS;
	DataPkt* pDataPkt = Null;

	if(Null == pCrb)
	{
		goto END;
	}
	
	if(CRB_TX_REQ == pCrb->m_State)
	{
		pDataPkt = &pCrb->m_ReqCmd;
	}
	else if(CRB_TX_RSP == pCrb->m_State)
	{
		pDataPkt = &pCrb->m_RspCmd;
	}
	else
	{
		return CHNL_SUCCESS;
	}

	Crb_TimerStop(pCrb);

	pCrb->m_isPending = True;
	pCrb->m_bTxCount++;

	nRet = pChnl->m_pTransfer->TxData(pChnl->m_pTransfer, (uint8*)pDataPkt->m_pData, pDataPkt->m_DataLen);
	pCrb->m_State = CRB_WAIT_TX_RESULT;
	
	pChnl->m_DelayMsForRsp = Crb_GetWaitForRspTime(pCrb);
	if(TX_SUCCESS == nRet)
	{
		//Chnl_MsgTxDone(pChnl, pCrb, CHNL_TX_SUCCESS);
		Chnl_PostMsg(pChnl, CHNL_TX_SUCCESS, (uint32)pCrb, CHNL_TX_SUCCESS);
	}
	else if(TX_FAILED == nRet)
	{
		//Chnl_MsgTxDone(pChnl, pCrb, CHNL_TX_FAILED);
		Chnl_PostMsg(pChnl, CHNL_TX_SUCCESS, (uint32)pCrb, CHNL_TX_FAILED);
	}
	else if(pChnl->m_DelayMsForTxData)
	{
		Crb_TimerStart(pCrb, TIMER_TX_DATA, pChnl->m_DelayMsForTxData);
	}
	else
	{
		Assert(False);
	}
	
END:		
	return CHNL_SUCCESS;
}

static CHNL_RET Chnl_MsgRxDone(Chnl* pChnl, Crb* pCrb, uint32 param2)
{
	int ret = CHNL_SUCCESS;
	DataPkt* pDataPkt = Null;

	if(pCrb->m_IsForSendReq)
	{
		pDataPkt = &pCrb->m_RspCmd;
	}
	else
	{
		pDataPkt = &pCrb->m_ReqCmd;
	}

	if(pChnl->m_pPktDesc->Verify)
	{
		if(!pChnl->m_pPktDesc->Verify(pDataPkt->m_pData, pDataPkt->m_DataLen))
		{
			ret = CHNL_VERIFY_FAILED;
			PF_WARNING(("Chnl[%d] Crb[%d] Verify data error!\n", pChnl->m_ChannelD, pCrb));
			goto END;
		}
	}

	if(!pCrb->m_IsForSendReq)
	{
		if(pCrb->m_State == CRB_INIT)
		{
			//It is a new req, Crb is for response
			Crb_ConfigRsp(pCrb, pDataPkt);
			pCrb->m_State = CRB_RX_REQ_SUCCESS;
			Chnl_CmdDisptch(pChnl, pCrb);
		}
		else 
		{
			PF_WARNING(("%s: Crb busy: ", _FUNC_));
			goto END;
		}
	}
	else
	{
		if(CRB_WAIT_RSP == pCrb->m_State)
		{
			//It is a response
			pCrb->m_State = CRB_RX_RSP_SUCCESS;
			pCrb->CrbDone(pCrb);
		}
		else
		{
			PF_WARNING(("Chnl_MsgRxDone(), state[%d] error\n", pCrb->m_State));
		}
		ret = CHNL_SUCCESS;
		goto END;
	}
		
	Chnl_Ready(pChnl);
	return ret;

END:	
	DataPkt_Reset(pDataPkt);
	Chnl_Ready(pChnl);
	
	return ret;
}

void Chnl_CancelCrb(Chnl* pChnl, Crb* pCrb)
{
	List* node = Null;
	
	if(pCrb->m_State != CRB_INIT)
	{
		pCrb->m_State = CRB_CANCEL;
		pCrb->CrbDone(pCrb);
	}
	
	if(pCrb->m_isPending)
	{
		PF_WARNING(("%s()\n", _FUNC_));

		Assert(List_isIn((List*)pChnl->m_pPendingCrbList, (List*)pCrb));
		SET_CRB_TO_IDEL_LIST(pChnl, pCrb);
	}

	Chnl_Ready(pChnl);
}

Bool Chnl_SendCrb(Chnl* pChnl, Crb* pCrb)
{
	if(pChnl->m_pPendingCrbList)
	{
		if(List_isIn((List*)pChnl->m_pPendingCrbList, (List*)pCrb)) return False;
	}
		
	PF(DL_CHNL,("Chnl[%d] Send %s, count=%d\n"
		, pChnl->m_ChannelD, pCrb->m_IsForSendReq ? "REQ" : "RSP", pCrb->m_bTxCount));

	SET_CRB_TO_TX_LIST(pChnl, pCrb);
	
	if(!pChnl->m_pPendingCrbList->m_isPending)
	{
		pCrb->m_State = pCrb->m_IsForSendReq ? CRB_TX_REQ : CRB_TX_RSP;
		Chnl_MsgTxData(pChnl, pCrb, 0);
	}
	
	return True;
}

void Chnl_CmdDisptch(Chnl* pChnl, Crb* pCrb)
{
	RSP_CODE nRet = RSP_SUCCESS;
	DataPkt* pReq = &pCrb->m_ReqCmd;
	DataPkt* pRsp = &pCrb->m_RspCmd;
	uint16 nRspLen = pReq->m_MaxLen;

	Assert(pChnl->ReqHandler);

	nRet = pChnl->ReqHandler(pChnl->m_pDisptchArg, pCrb, pReq->m_pData, pReq->m_DataLen);
	if(RSP_SUCCESS == nRet)	//发送应答
	{
		Assert(pCrb->m_RspCmd.m_MaxLen >= pCrb->m_RspCmd.m_DataLen);
		Chnl_SendCrb(pCrb->m_pChnl, pCrb);
	}
	else if(RSP_DISCARD == nRet)	//不响应，丢弃该命令
	{
		pCrb->Reset(pCrb);
	}
	else if(RSP_PENDING == nRet)
	{
		//Do nothing, waiting...
	}
	else
	{
		Assert(False);
	}
}

void Chnl_MsgHandler(Chnl* pChnl, uint8 msgID, uint32 param1, uint32 param2)
{
	const static MsgMap g_MsgMapTbl[] = 
	{
		{CHNL_TX_SUCCESS	, (MsgMapFun)Chnl_MsgTxDone}
		, {CHNL_RX_SUCCESS	, (MsgMapFun)Chnl_MsgRxDone}
		, {CHNL_TXDATA		, (MsgMapFun)Chnl_MsgTxData}
		, {CHNL_TIMEOUT		, (MsgMapFun)Chnl_MsgTimeOut}
	};
	int i = 0;
	int nCount = sizeof(g_MsgMapTbl) / sizeof(MsgMap);
	const MsgMap* pMsgHandler = g_MsgMapTbl;
	
	for(i = 0; i < nCount; i++, pMsgHandler++)
	{
		if(msgID & pMsgHandler->m_MsgID)
		{
			PF(DL_CHNL , ("Chnl_msgProc(%d, %d, 0x%x)\n", pChnl->m_ChannelD, msgID, param2));

			pMsgHandler->MsgHandler((MsgIf*)pChnl, param1, param2);
			break;
		}
	}
}

void Chnl_Reset(Chnl* pChnl)
{
	Crb* pCrb = Null;
	
	PF(DL_MAIN, ("%s: ChnlID=%d\n", _FUNC_, pChnl->m_ChannelD));
	
	for(pCrb = pChnl->m_pPendingCrbList; pCrb != Null; pCrb = (Crb*)(((List*)pCrb)->m_pNext))
	{
		if(pCrb->m_State != CRB_INIT)
		{
			pCrb->m_State = CRB_CANCEL;
			pCrb->CrbDone(pCrb);
		}
		SET_CRB_TO_IDEL_LIST(pChnl, pCrb);
	}
}

void Chnl_Release(Chnl* pChnl)
{
	Chnl_Reset(pChnl);
	
	PF(DL_MAIN, ("%s: ChnlID=%d\n", _FUNC_, pChnl->m_ChannelD));

	List_RemoveAll((List*)pChnl->m_pCrbList);
	List_RemoveAll((List*)pChnl->m_pPendingCrbList);
}

void Chnl_Init(Chnl* pChnl, uint8 ChnlID, const PktDesc* pPktDesc, MsgPostFun postMsg, Transfer* pTransfer, CrbReqFun ReqHandler, void*	pDisptchArg)
{
	Assert(pPktDesc);

	PF(DL_MAIN, ("%s: ChnlID=%d\n", _FUNC_, pChnl->m_ChannelD));
	
    memset(pChnl, 0, sizeof(Chnl));

	MsgIf_Init((MsgIf*)pChnl, (MsgProcFun)Chnl_MsgHandler, Null);
	
	pChnl->m_ChannelD		= ChnlID;
	pChnl->m_pPktDesc 	= pPktDesc;
	
	pChnl->PostMsg 		= postMsg;
	pChnl->m_DelayMsForTxData = 10;

	pChnl->ReqHandler = ReqHandler;
	pChnl->m_pDisptchArg = pDisptchArg;
	
	pChnl->m_pTransfer = pTransfer;
	pTransfer->m_pArg = pChnl;
}

void Chnl_VerifyReset(Chnl* pChnl)
{
	PF(DL_MAIN, ("%s: channelNum=%d, crbCount=%d\n", _FUNC_, pChnl->m_ChannelD, List_Count((List*)pChnl->m_pCrbList)));
	
	AssertTrue(pChnl);
	AssertTrue(pChnl->m_pPendingCrbList == Null);
	AssertTrue(pChnl->m_pCrbList);
}


