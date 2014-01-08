#include "TransferDriver.h"
#include "Chnl.h"
#include "Debug.h"

//Return 
//	CHNL_SUCCESS	:同步模式，数据发送成功
//	CHNL_FAILED	:同步模式，数据发送失败
//	CHNL_PENDING	:异步模式，不知道数据发送成功或者失败，等待数据发送结果。
TX_CODE Transfer_TxData(Transfer* pTransfer, uint8* pData, uint16 len)
{
    return TX_FAILED;
}

void Transfer_RxDataCallBack(Transfer* pTransfer, uint8* pData, uint16 len)
{
	Assert(pTransfer->m_pChnl);
	Chnl_RxData(pTransfer->m_pChnl, EVENT_RX_SUCCESS, pData, len);
}

void Transfer_Init(Transfer* pTransfer, Chnl* pChnl)
{
	memset(pTransfer, 0, sizeof(Transfer));
	
	pTransfer->TxData = Transfer_TxData;
	pTransfer->m_pChnl = pChnl;
}



#endif

