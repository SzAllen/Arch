
#include "ArchDef.h"
#include "DataPkt.h"
#include "Debug.h"

/////////////////////////////////////////////////////////////////////////////////
Bool PktDesc_Verify(void* pCmdBuff, int16 nLen)
{
	return True;	
}

Bool PktDescg_IsReq(void* pCmdBuff, int16 nLen)
{
	return True;
}

Bool PktDesc_IsRspForReq(void* pReq, int16 nReqLen, void* pRsp, int16 nRspLen)
{
	return True;
}

void PktDesc_Init(PktDesc* pPktDesc
	, uint16 maxLen
	, PktDescIsFun IsReq
	, PktDescIsFun Verify
	, PktDescIsRspForReq IsRspForReq
	)
{
	memset(pPktDesc, 0, sizeof(PktDesc));
	
	pPktDesc->m_MaxLen 	= maxLen;
	
	pPktDesc->IsReq 		= IsReq;
	pPktDesc->Verify 	= Verify;
	pPktDesc->IsRspForReq = IsRspForReq;
}

void DataPkt_Reset(DataPkt* pCmd)
{
	memset(pCmd->m_pData, 0, pCmd->m_MaxLen);
	
	pCmd->m_DataLen = 0;
	pCmd->m_isUsed = 0;
}

void DataPkt_Init(DataPkt* pCmd, uint8* pCmdBuff, uint16 nBufLen)
{
	memset(pCmd, 0, sizeof(DataPkt));
	
	pCmd->m_isUsed = 0;
	
	pCmd->m_MaxLen 	= nBufLen;
	pCmd->m_pData 	= pCmdBuff;
}


