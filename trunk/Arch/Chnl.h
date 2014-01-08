#ifndef  _IPMB_H_
#define  _IPMB_H_

#ifdef _cplusplus
extern "c"{
#endif

#include "ArchDef.h"
#include "Crb.h"
#include "Message.h"
#include "Queue.h"
#include "TransferDriver.h"
#include "CmdItem.h"

enum
{
	CHNL_TXDATA 		= BIT_0
	, CHNL_TX_SUCCESS	= BIT_1
	, CHNL_RX_SUCCESS	= BIT_2

	, CHNL_TX_FAILED	= BIT_3
	, CHNL_RX_FAILED	= BIT_4
	, CHNL_TIMEOUT		= BIT_5
};

enum	
{
	TIMER_TX_REQ	= 1
	, TIMER_RX_REQ
	, TIMER_TX_DATA
};

typedef enum _CHNL_RET
{
	  CHNL_SUCCESS = 0	//执行成功
	, CHNL_FAILED		//执行失败
	, CHNL_PENDING		//不知道结果，需要等待
	, CHNL_VERIFY_FAILED	//检验失败
}CHNL_RET;

struct _tagCrb;
struct _tagChnl;

//逻辑通道,实现在通道上的传输数据包流控制
typedef struct _tagChnl
{
	MsgIf	m_Base;

	const PacketDesc*	m_pPacketDesc;
	
	uint32	m_ChannelD;
	uint32	m_DelayMsForTxData;	//等待发送数据中断的延时时间
	
	Transfer* m_pTransfer;	//Transfer driver, be used to tx data, may be Iic, Uart...
	
	struct _tagCrb*    m_pCrbList;
	struct _tagCrb*    m_pTxCrbsList;
	struct _tagCrb*    m_pTxCrb;		//Point to TxData Crb;

	MsgPostFun		PostMsg;
}Chnl;

void Chnl_Init(Chnl* pChnl, uint8 ChnlID, const PacketDesc* pPacketDesc, MsgPostFun postMsg, Transfer* pTransfer);
Bool Chnl_SendCrb(Chnl* pChnl, struct _tagCrb* pCrb);
void Chnl_AttachTransfer(Chnl* pChnl, Transfer* pDriver);
int Chnl_RxData(Chnl* pChnl, TransferEvent eventId, uint8* pData, uint16 len);
void Chnl_AttachCrb(Chnl* pChnl, struct _tagCrb* pCrb);
void Chnl_UnAttachCrb(Chnl* pChnl, struct _tagCrb* pCrb);
void Chnl_CancelCrb(Chnl* pChnl, struct _tagCrb* pCrb);
void Chnl_Reset(Chnl* pChnl);
void Chnl_VerifyReset(Chnl* pChnl);
void Chnl_Release(Chnl* pChnl);
void Chnl_CmdDisptch(Chnl* pChnl, struct _tagCrb* pCrb);

#ifdef _cplusplus
}
#endif

#endif

