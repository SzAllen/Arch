
#include "Chnl.h"

#define SET_CRB_TO_TX_LIST(pCrb) Chnl_SetCrbToTxList(pChnl, pCrb, True)
#define SET_CRB_TO_IDEL_LIST(pCrb) Chnl_SetCrbToTxList(pChnl, pCrb, False)

void Chnl_PostMsg(Chnl* pChnl, uint8 msgID, uint32 param1, uint32 param2)
{
	//PF(OBJECT_IPMB | DL_INTEG_TEST, ("%s, msId=%d\n", _FUNC_, msgID));
	pChnl->PostMsg((MsgIf*)pChnl, msgID, param1, param2);
}

void Chnl_SetCrbToTxList(Chnl* pChnl, Crb* pCrb, Bool isToPendingList)
{
	if(isToPendingList)
	{
		//Remove crb from m_pCrbList
		if(pChnl->m_pCrbList && List_isIn((List*)pChnl->m_pCrbList, (List*)pCrb))
			pChnl->m_pCrbList = (Crb*)List_Remove((List*)pCrb);
		
		//Put crb in m_pTxCrbsList
		pChnl->m_pTxCrbsList = (Crb*)List_AddTail((List*)pChnl->m_pTxCrbsList, (List*)pCrb);
	}
	else
	{
		//Remove crb from m_pTxCrbsList
		if(pChnl->m_pTxCrbsList && List_isIn((List*)pChnl->m_pTxCrbsList, (List*)pCrb))
			pChnl->m_pTxCrbsList = (Crb*)List_Remove((List*)pCrb);
		
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
	pTransfer->m_pChnl = pChnl;
}

static void Chnl_Ready(Chnl* pChnl)
{
	if(Null == pChnl->m_pTxCrb)
	{
		pChnl->m_pTxCrb = pChnl->m_pTxCrbsList;
		
		if(pChnl->m_pTxCrb)
		{
			Chnl_PostMsg(pChnl, CHNL_TXDATA, 0, 0);
		}
	}
}

static Crb* Chnl_GetCrb(Chnl* pChnl, uint8* pData)
{
	Crb* pCrb = pChnl->m_pTxCrb;
	List* pListNode = (List*)pChnl->m_pCrbList;
	
	if(pCrb && pCrb->IsMatch(pCrb, pData))
	{			
		return pCrb;
	}
	
	//Search the crb in m_pTxCrbsList;
	while(pListNode)
	{
		pCrb = (Crb*)pListNode;
		
		if(pCrb->IsMatch(pCrb, pData))
		{			
			return pCrb;
		}

		pListNode = pListNode->m_pNext;
	}

	//Search the crb in m_pTxCrbsList;
	pListNode = (List*)pChnl->m_pTxCrbsList;
	while(pListNode)
	{
		pCrb = (Crb*)pListNode;
		
		if(pCrb->IsMatch(pCrb, pData))
		{			
			return pCrb;
		}
		
		pListNode = pListNode->m_pNext;
	}

	return Null;
}

static CHNL_RET Chnl_CrbDone(Chnl* pChnl, Crb* pCrb)
{		
	pCrb->CrbDone(pCrb);
	
	return CHNL_SUCCESS;
}

static int Chnl_ReSendCrb(Chnl* pChnl, Crb* pCrb)
{
	Assert(pCrb);

	PF(DL_CHNL, ("%s(), TxCount=%d\n", _FUNC_, pCrb->m_bTxCount));
	
	if(pCrb->m_IsForSendReq)
	{
		pCrb->m_State = CRB_RETX_REQ;
	}
	else
	{
		pCrb->m_State = CRB_RETX_RSP;
	}

	SET_CRB_TO_TX_LIST(pCrb);
	
	if(Null == pChnl->m_pTxCrb || pChnl->m_pTxCrb == pCrb)
	{
		pChnl->m_pTxCrb = pCrb;
		Chnl_PostMsg(pChnl, CHNL_TXDATA, 0, 0);
	}
	
	return CHNL_SUCCESS;
}

//Chnl接收到数据
CHNL_RET Chnl_RxData(Chnl* pChnl, TransferEvent eventId, uint8* pData, uint16 len)
{
	int nRet = CHNL_SUCCESS;
	Crb* pCrb = pChnl->m_pTxCrb;
	Bool isReq = True;

	PF(DL_CHNL, ("Chnl_RxData[0x%x], eventId=%d\n", pCrb, eventId));	
	
	if(pCrb)
	{		
		if(EVENT_TX_SUCCESS & eventId)
		{
			Chnl_PostMsg(pChnl, CHNL_TX_SUCCESS, 0, (uint32)pCrb);
		}
		else if(EVENT_TX_FAILED & eventId)
		{
			Chnl_PostMsg(pChnl, CHNL_TX_FAILED, 0, (uint32)pCrb);
		}
		else
		{
			nRet = CHNL_FAILED;
			goto End;
		}
	}
	if(EVENT_RX_SUCCESS & eventId)
	{
		CmdItem* pCmdItem = Null;
		
		if(Null == pData)
		{
			nRet = CHNL_FAILED;
			goto End;
		}
		
		pCrb = Chnl_GetCrb(pChnl, pData);
		if(pCrb == Null)
		{
			PF(DL_WARNING, ("Warning: Don't find Crb, discard this cmd!\n"));
			DUMP_BYTE(pData, len);
			nRet = CHNL_FAILED;
			goto End;
		}
		isReq = pChnl->m_pPacketDesc->IsReq(pData, len);

		PF(DL_CHNL,("Chnl[%d] Received %s, len=%d: ", pChnl->m_ChannelD, isReq ? "REQ" : "RSP", len));
		
		if(isReq)
		{
			pCmdItem = &pCrb->m_ReqCmd;
			if(pCmdItem->m_isUsed)
			{
				PF(DL_WARNING, ("Warning: Busy.\n"));
				//IpmiHeader_Dump(&header, DL_WARNING, _FUNC_, _LINE_);
				nRet = CHNL_FAILED;
			}
			else if(pChnl->m_pPacketDesc->m_MaxLen >= len)
			{
				memcpy(pCmdItem->m_pCmd, pData, len);
				pCmdItem->m_CmdLen = len;
				
				pCmdItem->m_isUsed = True;
				Chnl_PostMsg(pChnl, CHNL_RX_SUCCESS, 0, (uint32)pCrb);
			}
			else
			{
				PF(DL_WARNING, ("Warning: %s,line=%d, Data len(=%d) is too long, discard.\n", _FUNC_, _LINE_, len));
				nRet = CHNL_FAILED;
			}
		}
		else
		{
			pCmdItem = &pCrb->m_RspCmd;
			if(pCmdItem->m_isUsed)
			{
				PF(DL_WARNING, ("Warning: %s,line=%d, Unexpected Response, discard.\n", _FUNC_, _LINE_));
				//IpmiHeader_Dump(&header, DL_WARNING, _FUNC_, _LINE_);
				nRet = CHNL_FAILED;
			}
			else if(pChnl->m_pPacketDesc->RspIsMatchWithReq(&pCrb->m_ReqCmd.m_pCmd, pCrb->m_ReqCmd.m_CmdLen, pData, len))
			{
				if(pChnl->m_pPacketDesc->m_MaxLen >= len)
				{
					memcpy(pCmdItem->m_pCmd, pData, len);
					pCmdItem->m_CmdLen = len;
					pCmdItem->m_isUsed = True;
					Chnl_PostMsg(pChnl, CHNL_RX_SUCCESS, 0, (uint32)pCrb);
				}
				else
				{
					PF(DL_WARNING, ("Warning: %s,line=%d, Data len(=%d) is too long, discard.\n", _FUNC_, _LINE_, len));
					nRet = CHNL_FAILED;
				}
			}
			else
			{
				//IpmiHeader_Dump(&pCrb->m_pReqCmd->m_Header,DL_MAIN,  Null, 0);
				//IpmiHeader_Dump(&header, DL_MAIN, Null, 0);
				PF(DL_WARNING, ("Warning: %s,line=%d, Unexpected Response, discard.\n", _FUNC_, _LINE_));
				nRet = CHNL_FAILED;
			}
		}
	}
	else
	{
		nRet = CHNL_FAILED;
	}

End:	
    return nRet;
}

static CHNL_RET Chnl_MsgTimeOut(Chnl* pChnl, uint32 param1, Crb* pCrb)
{
	if(pCrb->m_IsForSendReq)
	{		
		if(CRB_TX_REQ_SUCCESS == pCrb->m_State)	//Waiting for response time out
		{
			if(Crb_IsNeedReSend(pCrb))
			{
				Chnl_ReSendCrb(pChnl, pCrb);
				return CHNL_SUCCESS;
			}
			else
			{
				PF(DL_WARNING, ("Warning: CRB_RX_RSP_TIMEOUT\n"));
				pCrb->m_State = CRB_RX_RSP_FAILED;
				pCrb->m_ErrorCode = CRB_TIMEOUT;
			}
		}
		else if(CRB_TX_REQ == pCrb->m_State || CRB_RETX_REQ == pCrb->m_State)
		{
			SET_CRB_TO_IDEL_LIST(pCrb);
			//Chnl_SetCrbToTxList(pChnl, pCrb, False);
			
			if(Crb_IsNeedReSend(pCrb))
			{
				Chnl_ReSendCrb(pChnl, pCrb);
				return CHNL_SUCCESS;
			}
			else
			{
				PF(DL_WARNING, ("Warning: CRB_TX_REQ Timer out\n"));
				pCrb->m_State = CRB_TX_REQ_FAILED;
				pCrb->m_ErrorCode = CRB_TIMEOUT;
				pChnl->m_pTxCrb = Null;
			}
		}
		else
		{
			return CHNL_SUCCESS;
		}
	}
	else
	{
		if(CRB_RX_REQ_SUCCESS == pCrb->m_State)
		{
			pCrb->m_State = CRB_NO_RSP;
		}
		else if(CRB_TX_RSP == pCrb->m_State)
		{
			SET_CRB_TO_IDEL_LIST(pCrb);
			//Chnl_SetCrbToTxList(pChnl, pCrb, False);
			
			PF(DL_WARNING, ("Warning: CRB_TX_RSP timerOut\n"));
			pChnl->m_pTxCrb = Null;
			pCrb->m_State = CRB_TX_RSP_FAILED;
			pCrb->m_ErrorCode = CRB_TIMEOUT;
		}
		else
		{
			return CHNL_SUCCESS;
		}
	}

	Chnl_CrbDone(pChnl, pCrb);
	Chnl_Ready(pChnl);
	
	return CHNL_SUCCESS;
}

static CHNL_RET Chnl_MsgTransferFailed(Chnl* pChnl, uint32 param1, Crb* pCrb)
{	
	Assert(pCrb);

	//Tx Req Failed
	if(pCrb->m_IsForSendReq)
	{			
		if(CRB_TX_REQ == pCrb->m_State || CRB_RETX_REQ == pCrb->m_State)
		{
			SwTimer_Stop(&pCrb->m_Timer);

			SET_CRB_TO_IDEL_LIST(pCrb);
			//Chnl_SetCrbToTxList(pChnl, pCrb, False);
			
			if(Crb_IsNeedReSend(pCrb))
			{
				Chnl_ReSendCrb(pChnl, pCrb);
				return CHNL_SUCCESS;
			}
			else
			{
				PF(DL_WARNING, ("Warning: CRB_TX_REQ failed\n"));
				pChnl->m_pTxCrb = Null;
				pCrb->m_State = CRB_TX_REQ_FAILED;
			}
		}
	}
	else //Tx response data failed
	{
		SET_CRB_TO_IDEL_LIST(pCrb);
		//Chnl_SetCrbToTxList(pChnl, pCrb, False);
		PF(DL_WARNING, ("Warning: CRB_TX_RSP failed\n"));
		if(CRB_BUS_BUSY == pCrb->m_ErrorCode)
		{
			Chnl_ReSendCrb(pChnl, pCrb);
			return CHNL_SUCCESS;
		}
		pChnl->m_pTxCrb = Null;
		pCrb->m_State = CRB_TX_RSP_FAILED;
		
	}
	
	Chnl_CrbDone(pChnl, pCrb);
	Chnl_Ready(pChnl);
	return CHNL_SUCCESS;
}


static CHNL_RET Chnl_MsgTxDone(Chnl* pChnl, uint32 param1, uint32 param2)
{
	Crb* pCrb  = (Crb*)param2;

	if(Null == pCrb)
	{
		return CHNL_SUCCESS;
	}

	if(CRB_TX_REQ == pCrb->m_State || CRB_RETX_REQ == pCrb->m_State)
	{
		pCrb->m_State = CRB_TX_REQ_SUCCESS;

		//Start a timer to wait the response.
		if(pCrb->m_DelayMsForRsp)
		{
			SwTimer_Start(&pCrb->m_Timer, TIMER_TX_REQ, pCrb->m_DelayMsForRsp);
		}
		else
		{
			Chnl_CrbDone(pChnl, pCrb);
		}
	}
	else if(pCrb->m_State == CRB_TX_RSP)
	{
		pCrb->m_State = CRB_TX_RSP_SUCCESS;
		Chnl_CrbDone(pChnl, pCrb);
	}
	else
	{
		PF(DL_WARNING, ("Warning: Chnl_MsgTxDone() Error state =%d, \n", pCrb->m_State));
		return CHNL_SUCCESS;
	}

	SET_CRB_TO_IDEL_LIST(pCrb);
	//Chnl_SetCrbToTxList(pChnl, pCrb, False);
	
	pChnl->m_pTxCrb = Null;
	
	Chnl_Ready(pChnl);
	
	return CHNL_SUCCESS;
}

static CHNL_RET Chnl_MsgTxData(Chnl* pChnl, uint32 param1, uint32 param2)
{
	TX_CODE nRet = TX_SUCCESS;
	CmdItem* pCmdItem = Null;
	Crb* pCrb  = pChnl->m_pTxCrb;

	if(Null == pCrb)
	{
		goto END;
	}

	if(CRB_READY == pCrb->m_State
		|| CRB_TX_REQ == pCrb->m_State
		|| CRB_RETX_REQ == pCrb->m_State
		|| CRB_TX_RSP == pCrb->m_State
		|| CRB_RETX_RSP == pCrb->m_State
		|| CRB_TX_REQ_SUCCESS == pCrb->m_State
		|| CRB_RX_REQ_SUCCESS == pCrb->m_State)
	{
		//Start a timer to wait the response.
		SwTimer_Stop(&pCrb->m_Timer);
		if(pCrb->m_IsForSendReq)
		{			
			pCmdItem = &pCrb->m_ReqCmd;
			pCrb->m_State = CRB_TX_REQ;

			if(CRB_BUS_BUSY != pCrb->m_ErrorCode)
			{
				pCrb->m_bTxCount++;
			}
		}
		else
		{
			pCmdItem = &pCrb->m_RspCmd;
			pCrb->m_State = CRB_TX_RSP;
		}

		nRet = pChnl->m_pTransfer->TxData(pChnl->m_pTransfer, (uint8*)pCmdItem->m_pCmd, pCmdItem->m_CmdLen);
		if(TX_SUCCESS == nRet)
		{
			Chnl_MsgTxDone(pChnl, CHNL_TX_SUCCESS, (uint32)pCrb);
		}
		else if(TX_FAILED == nRet)
		{
			Chnl_MsgTxDone(pChnl, CHNL_TX_FAILED, (uint32)pCrb);
		}
		else if(pChnl->m_DelayMsForTxData)
		{
			SwTimer_Start(&pCrb->m_Timer, TIMER_TX_DATA, pChnl->m_DelayMsForTxData);
		}
		else
		{
			Assert(False);
		}
	}
	
END:		
	return CHNL_SUCCESS;
}

static CHNL_RET Chnl_MsgRxDone(Chnl* pChnl, uint32 param1, Crb* pCrb)
{
	int ret = CHNL_SUCCESS;
	CmdItem* pCmdItem = Null;

	if(pCrb->m_IsForSendReq)
	{
		pCmdItem = &pCrb->m_RspCmd;
	}
	else
	{
		pCmdItem = &pCrb->m_ReqCmd;
	}

	if(pChnl->m_pPacketDesc->Verify)
	{
		if(!pChnl->m_pPacketDesc->Verify(pCmdItem->m_pCmd, pCmdItem->m_CmdLen))
		{
			ret = CHNL_VERIFY_FAILED;
			PF(DL_WARNING, ("**Warning: Verify error!\n"));
			//CmdItem_Dump(pCmdItem);
			goto END;
		}
	}

	if(!pCrb->m_IsForSendReq)
	{
		if(pCrb->m_State == CRB_INIT)
		{
			//It is a new req, Crb is for response
			Crb_ConfigRsp(pCrb, pCmdItem);
			pCrb->m_State = CRB_RX_REQ_SUCCESS;
			Chnl_CmdDisptch(pChnl, pCrb);
		}
		else 
		{
			PF(DL_WARNING, ("%s: Crb busy: ", _FUNC_));
			goto END;
		}
	}
	else
	{
		if(CRB_TX_REQ_SUCCESS == pCrb->m_State)
		{
			//It is a response
			pCrb->m_State = CRB_RX_RSP_SUCCESS;
			Chnl_CrbDone(pChnl, pCrb);
		}
		else
		{
			PF(DL_WARNING, ("Chnl_MsgRxDone(), state[%d] error\n", pCrb->m_State));
		}
		ret = CHNL_SUCCESS;
		goto END;
	}
		
	Chnl_Ready(pChnl);
	return ret;

END:	
	CmdItem_Reset(pCmdItem);
	Chnl_Ready(pChnl);
	
	return ret;
}

void Chnl_CancelCrb(Chnl* pChnl, Crb* pCrb)
{
	Bool bFlag = False;
	List* node = Null;

	if(pCrb == pChnl->m_pTxCrb)
	{
		pChnl->m_pTxCrb = Null;
		bFlag = True;
	}
	else
	{
		for(node = (List*)pChnl->m_pTxCrbsList; node != Null; node = node->m_pNext)
		{
			if(node == (List*)pCrb)
			{		
				bFlag = True;
				break;
			}
		}
	}
	if(pCrb->m_State != CRB_INIT)
	{
		pCrb->m_State = CRB_CANCEL;
		Chnl_CrbDone(pChnl, pCrb);
	}
	
	if(bFlag)
	{
		PF(DL_WARNING, ("%s()\n", _FUNC_));

		Assert(List_isIn((List*)pChnl->m_pTxCrbsList, (List*)pCrb));
		SET_CRB_TO_IDEL_LIST(pCrb);
		//Chnl_SetCrbToTxList(pChnl, pCrb, False);
	}
	
	Chnl_Ready(pChnl);
}

Bool Chnl_SendCrb(Chnl* pChnl, Crb* pCrb)
{
	if(pChnl->m_pTxCrbsList)
	{
		if(List_isIn((List*)pChnl->m_pTxCrbsList, (List*)pCrb)) return False;
	}
		
	PF(DL_CHNL,("Chnl[%d] Send %s: ", pChnl->m_ChannelD, pCrb->m_IsForSendReq ? "REQ" : "RSP"));

	SET_CRB_TO_TX_LIST(pCrb);
	//Chnl_SetCrbToTxList(pChnl, pCrb, True);
	
	if(Null == pChnl->m_pTxCrb)
	{
		pChnl->m_pTxCrb = pCrb;
		Chnl_PostMsg(pChnl, CHNL_TXDATA, 0, 0);
	}
	
	return True;
}

void Chnl_CmdDisptch(Chnl* pChnl, Crb* pCrb)
{
	RSP_CODE nRet = RSP_SUCCESS;
	CmdItem* pReq = &pCrb->m_ReqCmd;
	uint16 nRspLen = pReq->m_MaxLen;
		
	if(pCrb->Disptch)
	{
		nRet = pCrb->Disptch(pCrb);
		
		if(RSP_SUCCESS == nRet)	//发送应答
		{
			Assert(pCrb->m_RspCmd.m_MaxLen >= pCrb->m_RspCmd.m_CmdLen);
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
}

void Chnl_MsgHandler(Chnl* pChnl, uint8 msgID, uint32 param1, uint32 param2)
{
	const static MsgMap g_MsgMapTbl[] = 
	{
		{CHNL_TX_SUCCESS	, (MsgMapFun)Chnl_MsgTxDone}
		, {CHNL_RX_SUCCESS	, (MsgMapFun)Chnl_MsgRxDone}
		, {CHNL_TXDATA		, (MsgMapFun)Chnl_MsgTxData}
		, {CHNL_TIMEOUT		, (MsgMapFun)Chnl_MsgTimeOut}
		
		, {CHNL_TX_FAILED 
			| CHNL_RX_FAILED 
							, (MsgMapFun)Chnl_MsgTransferFailed}
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
	List* node = Null;
	Crb* pCrb = pChnl->m_pTxCrb;
	
	if(pCrb)
	{
		pCrb->m_State = CRB_CANCEL;
		Chnl_CrbDone(pChnl, pCrb);
	}
	
	for(node = (List*)pChnl->m_pTxCrbsList; node != Null; )
	{
		pCrb = (Crb*)node;
		if(pCrb->m_State != CRB_INIT)
		{
			pCrb->m_State = CRB_CANCEL;
			Chnl_CrbDone(pChnl, pCrb);
		}
		node = node->m_pNext;
		SET_CRB_TO_IDEL_LIST(pCrb);
		//Chnl_SetCrbToTxList(pChnl, pCrb, False);
	}
}

void Chnl_Release(Chnl* pChnl)
{
	Chnl_Reset(pChnl);

	List_RemoveAll((List*)pChnl->m_pCrbList);
	List_RemoveAll((List*)pChnl->m_pTxCrbsList);
	List_RemoveAll((List*)pChnl->m_pTxCrb);
}

void Chnl_Init(Chnl* pChnl, uint8 ChnlID, const PacketDesc* pPacketDesc, MsgPostFun postMsg, Transfer* pTransfer)
{
	Assert(pPacketDesc);

	//PF(DL_MAIN, ("Chnl Init: channelNum=%d\n", channelNum));
    memset(pChnl, 0, sizeof(Chnl));

	MsgIf_Init((MsgIf*)pChnl, (MsgProcFun)Chnl_MsgHandler, Null);
	
	pChnl->m_ChannelD		= ChnlID;
	pChnl->m_pPacketDesc 	= pPacketDesc;
	
	pChnl->PostMsg 		= postMsg;
	pChnl->m_DelayMsForTxData = 10;

	pChnl->m_pTransfer = pTransfer;
	pTransfer->m_pChnl = pChnl;
}

void Chnl_VerifyReset(Chnl* pChnl)
{
	PF(DL_MAIN, ("%s: channelNum=%d, crbCount=%d\n", _FUNC_, pChnl->m_ChannelD, List_Count((List*)pChnl->m_pCrbList)));
	
	AssertTrue(pChnl);
	AssertTrue(pChnl->m_pTxCrb == Null);
	AssertTrue(pChnl->m_pTxCrbsList == Null);
	AssertTrue(pChnl->m_pCrbList);
}


