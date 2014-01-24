#include "IicCrb.h"
#include "IicCmd.h"
#include "CheckSum.h"
#include "Debug.h"

void IicCrb_CmdPacket(IicCrb* pIicCrb)
{
	Crb* pCrb = (Crb*)pIicCrb;
	IicMsg* pIicMsg = Null;
	uint8* pData = Null;
	
	if(pCrb->m_IsForSendReq)
	{
		pIicMsg = (IicMsg*)pCrb->m_ReqCmd.m_pData;
		
		PF(DL_CRB, ("\nSend Req[0x%02x->0x%02x]: cmd=0x%02x,NetFn=0x%02x,Seq=0x%02x\n"
			, pIicCrb->m_LocalAddr
			, pIicCrb->m_RemoteAddr
			, pIicMsg->cmd
			, pIicMsg->rsNetFn
			, pIicMsg->rqSeq
			));
	}
	else
	{
		pIicMsg = (IicMsg*)pCrb->m_RspCmd.m_pData;
	}
	
	IicMsg_Packet((IicMsg*)pData
		, (IpmiHeader*)pIpmiCmd
		, pIicCrb->m_LocalAddr
		, pIicCrb->m_LocalLun
		, pIicCrb->m_RemoteAddr
		);

	Assert(pIpmiCmd->m_CmdLen > 0);
	((IicMsg*)pData)->checkSum = CheckSum_Get(pData, 2);
	pData[pIpmiCmd->m_CmdLen - 1] = CheckSum_Get(&pData[3], pIpmiCmd->m_CmdLen - 4);
}

Bool IicCrb_IsMatch(IicCrb* pIicCrb, IicMsg* pIicCmd)
{
	Crb* pCrb = (Crb*)pIicCrb;
	uint8 isReq = IS_REQUEST_NETFUN(pIicCmd->rsNetFn);

	if(isReq == pCrb->m_IsForSendReq) return False;

	if(pIicCmd->rqSA == pIicCrb->m_RemoteAddr 
		&& pIicCmd->rsSA == pIicCrb->m_LocalAddr 
		&& pIicCmd->rsLun == pIicCrb->m_LocalLun
		)
	{
		if(pIicCmd->rqSeq > 0 && pIicCmd->rqSeq == pCrb->m_pRspIpmiCmd->m_Header.m_Seq) return False;
		if(isReq)
		{
			PF(TRACE_CRB, ("\nReceive %s[0x%02x->0x%02x]: cmd=0x%02x,NetFn=0x%02x,Seq=%d\n"
				, isReq ? "Req" : "Rsp"
				, pIicCmd->rqSA
				, pIicCmd->rsSA
				, pIicCmd->cmd
				, pIicCmd->rsNetFn
				, pIicCmd->rqSeq
				));
		}
		
		return True;
	}

	return False;
}

void IicCrb_Init(IicCrb* pIicCrb
	, uint16 maxReqCount
	, Chnl* pChnl
	, TimerManager* pTm
	, Bool isForSendReq
	, uint8 rA
	, uint8 lA
	, uint8 lLun
	)
{
	Crb* pCrb = (Crb*)pIicCrb;
	
	memset(pIicCrb, 0, sizeof(IicCrb));
	
	Crb_Init(pCrb, maxReqCount, pChnl, pTm, isForSendReq);

	pCrb->CmdPacket = (CrbFun)IicCrb_CmdPacket;
	pCrb->IsMatch 	= (CrbIsMatchFun)IicCrb_IsMatch;
	
	pIicCrb->m_RemoteAddr = rA;
	pIicCrb->m_LocalAddr = lA;
	pIicCrb->m_LocalLun = lLun;
}

