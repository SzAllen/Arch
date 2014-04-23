#include "TransferDriver.h"
#include "Chnl.h"

//Return 
//	CHNL_SUCCESS	:ͬ��ģʽ�����ݷ��ͳɹ�
//	CHNL_FAILED	:ͬ��ģʽ�����ݷ���ʧ��
//	CHNL_PENDING	:�첽ģʽ����֪�����ݷ��ͳɹ�����ʧ�ܣ��ȴ����ݷ��ͽ����
TX_CODE Transfer_TxData(Transfer* pTransfer, uint8* pData, uint16 len)
{
    return TX_FAILED;
}

void Transfer_RxDataCallBack(Transfer* pTransfer, uint8* pData, uint16 len)
{
	Assert(pTransfer->RxData);
	pTransfer->RxData(pTransfer->m_pArg, EVENT_RX_SUCCESS, pData, len);
}

void Transfer_Init(Transfer* pTransfer, RxCallBack rxFun, void* pArg)
{
	memset(pTransfer, 0, sizeof(Transfer));
	
	pTransfer->TxData = Transfer_TxData;
	pTransfer->m_pArg = pArg;
	pTransfer->RxData = (RxCallBack)rxFun;
}

