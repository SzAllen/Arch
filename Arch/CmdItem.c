
#include "CmdItem.h"
#include "Debug.h"

/////////////////////////////////////////////////////////////////////////////////
Bool PacketDesc_Verify(void* pCmdBuff, int16 nLen)
{
	return True;	
}

Bool PacketDescg_IsReq(void* pCmdBuff, int16 nLen)
{
	return True;
}

Bool PacketDesc_RspIsMatchWithReq(void* pReq, int16 nReqLen, void* pRsp, int16 nRspLen)
{
	return True;
}

void PacketDesc_Init(PacketDesc* pPacketDesc
	, uint16 maxLen
	, PacketDescIsFun IsReq
	, PacketDescIsFun Verify
	, PacketDescRspIsMatchWithReq RspIsMatchWithReq
	)
{
	memset(pPacketDesc, 0, sizeof(PacketDesc));
	
	pPacketDesc->m_MaxLen 	= maxLen;
	
	pPacketDesc->IsReq 		= IsReq;
	pPacketDesc->Verify 	= Verify;
	pPacketDesc->RspIsMatchWithReq = RspIsMatchWithReq;
}

void CmdItem_Reset(CmdItem* pCmd)
{
	memset(pCmd->m_pCmd, 0, pCmd->m_MaxLen);
	
	pCmd->m_CmdLen = 0;
	pCmd->m_isUsed = 0;
}

void CmdItem_Init(CmdItem* pCmd, uint8* pCmdBuff, uint16 nBufLen)
{
	memset(pCmd, 0, sizeof(CmdItem));
	
	pCmd->m_isUsed = 0;
	
	pCmd->m_MaxLen 	= nBufLen;
	pCmd->m_pCmd 	= pCmdBuff;
}


