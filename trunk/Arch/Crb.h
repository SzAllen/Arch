#ifndef  _CRB_H_
#define  _CRB_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
#include "DataPkt.h"
#include "SwTimer.h"
#include "Chnl.h"
#include "queue.h"

#include "List.h"
#include "Message.h"

////////////////////////////////////////////////////////////////////////////


typedef enum _CrbState
{
	  CRB_INIT  = 0
	, CRB_READY
	, CRB_WAIT_TX_RESULT
	, CRB_WAIT_RSP
	, CRB_TX_RSP_SUCCESS
	, CRB_RX_REQ_SUCCESS
	, CRB_RX_RSP_SUCCESS
	, CRB_TX_REQ_FAILED	//10
	, CRB_TX_RSP_FAILED
	, CRB_RX_RSP_FAILED
	, CRB_NO_RSP
	, CRB_CANCEL
}CrbState;

typedef enum _CrbOperate
{
	  CRB_TX_REQ
	, CRB_TX_RSP
}CrbOperate;

typedef enum _CrbErrorCode
{
	  CRB_NO_ERROR = 0
	, CRB_BUS_ERROR
	, CRB_BUS_BUSY
	, CRB_BUS_NOACK
	, CRB_TIMEOUT
}CrbErrorCode;

typedef enum _tagCrbType
{
	 enRsp = 0x00
	, enReq = 0x01
	, enAll = 0x02
}CrbType;


struct _tagCrb;
typedef void (*CrbNotifyFun)(void* pRequester, struct _tagCrb* pCrb, uint8 State);

//命令请求包定义
typedef struct _tagCmdReq
{
	uint8  	m_reSendCounter;		//Re-send count
	uint32	m_WaitForRspMs;
	
	CrbNotifyFun	Done;			//Crb结束函数
	void* 			m_pRequester;	//请求者，用于调用函数Done
	
	uint16 	m_Len;					//数据长度
	uint8*	m_pData;				//发送数据
}CmdReq;

typedef Bool (*CrbIsMatchFun)(struct _tagCrb* pCrb, uint8* pCmdData);
typedef void (*CrbFun)(struct _tagCrb* pCrb);
typedef Bool (*CrbDataHandleFun)(struct _tagCrb* pCrb, void* pData, uint16 nLen);

//Cmd request block
typedef struct _tagCrb
{
	MsgIf		m_Base;
	
	uint8		m_isPending:1;		//Crb Is pending
	uint8		m_MaxSendCount:3;	//The max re-send count, the max value is 8
	uint8		m_bTxCount:3;		//Send data count
	uint8		m_IsForSendReq:1;	//1-used for REQ, 0-used for RSP

	uint8		m_State:7;
	uint8		m_isReSend:1;
	
	uint32 		m_ErrorCode;		//错误码
	
	DataPkt		m_ReqCmd;
	DataPkt		m_RspCmd;

	uint8		m_TimerId;
	
	Queue		m_CmdPacketQueue;	//用于排队CmdPacket

	struct _tagChnl*	m_pChnl;
	
	//Operation
	CrbFun 			CrbDone;
	CrbFun 			Reset;
	CrbIsMatchFun 	IsMatch;
	CrbDataHandleFun	PushIn;
	
}Crb;

void Crb_Init(Crb* pCrb
	, uint8*	pReqBuf
	, uint8*	pRspBuf
	, uint16	bufLen
	, CmdReq*	pReqArray
	, int 		ReqArrayCount
	, uint8*	pReqQueueBuf
	, int 		queueBufSize
	, struct _tagChnl* 	pChnl
	, Bool 		isForSendReq
	);

void Crb_Reset(Crb* pCrb);
Bool Crb_isIdle(Crb* pCrb);

void Crb_ConfigRsp(Crb* pCrb, DataPkt* pCmd);
Bool Crb_SendReq(Crb* pCrb, uint8* pData, int nLen, uint8 reSendCount, uint32 waitForRspMs, CrbNotifyFun CrbDone, void* pRequester);
Bool Crb_PostReq(Crb* pCrb, uint8* pData, int nLen, uint8 reSendCount, uint32 waitForRspMs, CrbNotifyFun CrbDone, void* pRequester);
const PktDesc* Crb_GetPktDesc(Crb* pCrb);

void Crb_SetCmdData(Crb* pCrb, uint8* pData, uint8 len);
uint8* Crb_GetCmdData(Crb* pCrb);
void Crb_SetCmdDataLen(Crb* pCrb, uint8 len);
Bool Crb_IsNeedReSend(Crb* pCrb);
int Crb_Send(Crb* pCrb);
void Crb_Done(Crb* pCrb);
void Crb_VerifyReset(Crb* pCrb);
void Crb_Release(Crb* pCrb);
void Crb_ReSendCurrentReq(Crb* pCrb);
Crb* Crb_GetMatch(Crb* pCrbList, uint8* pData);
uint32 Crb_GetWaitForRspTime(Crb* pCrb);

void Crb_TimerStop(Crb* pCrb);
void Crb_TimerStart(Crb* pCrb, uint8 timerId, uint32 value_ms);

#ifdef __cplusplus
}
#endif

#endif             

