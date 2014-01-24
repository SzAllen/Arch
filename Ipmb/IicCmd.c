#include "IicCmd.h"
#include "CheckSum.h"
#include "Debug.h"

void IicMsg_Packet(IicMsg* pIicMsg, IpmiHeader* pHeader, uint8 rqSA, uint8 rqLun, uint8 rsSA)
{
	pIicMsg->rqSA 	= rqSA;
	pIicMsg->rqLun 	= rqLun;
	pIicMsg->rsSA 	= rsSA;

	pIicMsg->rsLun 	= pHeader->m_Lun;
	pIicMsg->rsNetFn= pHeader->m_NetFn;
	pIicMsg->rqSeq	= pHeader->m_Seq;
	pIicMsg->cmd	= pHeader->m_Cmd;
}

Bool IicMsg_Verify(IicMsg* pIicMsg, int8 len)
{
	uint8* pData = (uint8*)pIicMsg;

	if(len < MIN_IPMICMD_LEN)
	{
		return False;
	}
	
    if (!CheckSum_Check(pData, 3) || !CheckSum_Check(&pData[3], len - 3))
    {
        return False;
    }
	
	return True;	
}

void IicMsg_GetHeader(IicMsg* pIicMsg, IpmiHeader* pIpmiHeader)
{
	pIpmiHeader->m_NetFn= pIicMsg->rsNetFn;
	pIpmiHeader->m_Lun 	= pIicMsg->rqLun;
	pIpmiHeader->m_Seq 	= pIicMsg->rqSeq;
	pIpmiHeader->m_Cmd 	= pIicMsg->cmd;
}

/////////////////////////////////////////////////////////////////////////////////////

uint8 IicMsg_GetRsp(const IicMsg* pReq, uint8* pRspBuff, uint8 rspLen, uint8* rspData, uint8 rspDataLen)
{
	IicMsg* pRsp = (IicMsg*)pRspBuff;

	Assert(rspLen >= rspDataLen + IICMSG_HEAD_SIZE + 1);
		
	memcpy(pRsp, pReq, IICMSG_HEAD_SIZE);
	pRsp->rsSA = pReq->rqSA;
	pRsp->rqSA = pReq->rsSA;
	pRsp->rsNetFn = pReq->rsNetFn + 1;
	memcpy(pRsp->smsgBuf, rspData, rspDataLen);
	
	rspLen = rspDataLen + IICMSG_HEAD_SIZE + 1;
	pRsp->checkSum = CheckSum_Get(pRspBuff, 2);
	pRspBuff[rspLen - 1] = CheckSum_Get(&pRspBuff[3], rspLen - 4);

	return rspLen;
}


void IicCmd_SetDataLen(IicCmd* pIicCmd, uint8 cmdDataLen)
{
	IpmiCmd* pCmd = (IpmiCmd*)pIicCmd;
	pCmd->m_CmdLen = pCmd->m_HeaderLen + cmdDataLen + 1;
}

uint8 IicCmd_GetDataLen(IicCmd* pIicCmd)
{
	IpmiCmd* pCmd = (IpmiCmd*)pIicCmd;
	return pCmd->m_CmdLen - pCmd->m_HeaderLen - 1/*Check sum*/;
}

void IicCmd_Init(IicCmd* pIicCmd)
{
	IpmiCmd* pCmd = (IpmiCmd*)pIicCmd;
	
	memset(pIicCmd, 0, sizeof(IicCmd));

	IpmiCmd_Init(pCmd
		, (uint8*)&pIicCmd->m_IicMsg
		, sizeof(IicMsg)
		, IICMSG_HEAD_SIZE
		, sizeof(IicMsg) - IICMSG_HEAD_SIZE - 1
		);
	
	pCmd->GetDataLen = (IpmiCmdGetDataLenFun)IicCmd_GetDataLen;
	pCmd->SetDataLen = (IpmiCmdSetDataLenFun)IicCmd_SetDataLen;
}


