#ifndef  _CRB_H_
#define  _CRB_H_

#ifdef _cplusplus
extern "c"{
#endif

#include "typedef.h"
#include "CmdItem.h"
#include "SwTimer.h"
#include "Chnl.h"
#include "queue.h"

#include "List.h"

////////////////////////////////////////////////////////////////////////////


typedef enum _CrbState
{
	  CRB_INIT  = 0
	, CRB_READY
	, CRB_TX_REQ
	, CRB_RETX_REQ
	, CRB_TX_RSP
	, CRB_RETX_RSP	//5
	, CRB_TX_REQ_SUCCESS
	, CRB_TX_RSP_SUCCESS
	, CRB_RX_REQ_SUCCESS
	, CRB_RX_RSP_SUCCESS
	, CRB_TX_REQ_FAILED	//10
	, CRB_TX_RSP_FAILED
	, CRB_RX_RSP_FAILED
	, CRB_NO_RSP
	, CRB_CANCEL
	, CRB_STOP
}CrbState;

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
typedef int (*CrbNotifyFun)(void* pRequester, struct _tagCrb* pCrb, uint8 State);

//命令请求包定义
typedef struct _tagCmdReq
{
	uint8  	m_reSendCounter;		//失败重发次数
	uint32 	m_DelayMsForRsp;		//等待响应延时时间
	
	CrbNotifyFun	Done;			//Crb结束函数
	void* 			m_pRequester;	//请求者，用于调用函数Done
	
	uint16 	m_Len;					//数据长度
	uint8	m_Data[1];				//发送数据
}CmdReq;

typedef Bool (*CrbIsMatchFun)(struct _tagCrb* pCrb, uint8* pCmdData);
typedef void (*CrbFun)(struct _tagCrb* pCrb);
typedef RSP_CODE (*CmdDisptchFun)(struct _tagCrb* pCrb);

//Ipmi cmd request block
typedef struct _tagCrb
{
	List		m_Base;

	uint8		m_State;
	
	uint32 		m_ErrorCode;		//错误码
	uint32 		m_DelayMsForRsp;	//等待响应延时
	
	uint8		m_MaxSendCount:4;	//The max re-send count, the max value is 8
	uint8		m_bTxCount:3;		//Send data count
	uint8		m_IsForSendReq:1;	//1-used for REQ, 0-used for RSP
	
	CmdItem		m_ReqCmd;
	CmdItem		m_RspCmd;

	SwTimer		m_Timer;
	Queue		m_CmdPacketQueue;	//用于排队CmdPacket
	
	CmdReq*		m_pCmdReq;
	
	struct _tagChnl*	m_pChnl;
	

	CrbFun 			CrbDone;
	CrbFun 			Reset;
	CrbIsMatchFun 	IsMatch;
	
	CmdDisptchFun	Disptch;
	void*			m_pDisptchObj;	//仅用于当接收到请求时，调用函数Disptch时使用
}Crb;

void Crb_Init(Crb* pCrb
	, uint16 			maxCmdCount		//Crb可以最多排队命令个数
	, TimerManager* 	pTimerManager
	, struct _tagChnl* 	pChnl
	, Bool 				isForSendReq
	, void*				pDisptchObj
	);

void Crb_Reset(Crb* pCrb);
Bool Crb_isIdle(Crb* pCrb);

void Crb_ConfigRsp(Crb* pCrb, CmdItem* pCmd);
Bool Crb_SendReq(Crb* pCrb, uint8* pData, int nLen, uint8 reSendCount, uint32 nDelayMsForRsp, CrbNotifyFun CrbDone, void* pRequester);
Bool Crb_PostReq(Crb* pCrb, uint8* pData, int nLen, uint8 reSendCount, uint32 nDelayMsForRsp, CrbNotifyFun CrbDone, void* pRequester);
const PacketDesc* Crb_GetPacketDesc(Crb* pCrb);

void Crb_SetCmdData(Crb* pCrb, uint8* pData, uint8 len);
uint8* Crb_GetCmdData(Crb* pCrb);
void Crb_SetCmdDataLen(Crb* pCrb, uint8 len);
Bool Crb_IsNeedReSend(Crb* pCrb);
int Crb_Send(Crb* pCrb);
void Crb_Done(Crb* pCrb);
void Crb_VerifyReset(Crb* pCrb);
void Crb_Release(Crb* pCrb);
void Crb_ReSendCurrentReq(Crb* pCrb);

#ifdef _cplusplus
}
#endif

#endif             

