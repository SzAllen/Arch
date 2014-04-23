#ifndef  _TRANSFER_DRIVER_H_
#define  _TRANSFER_DRIVER_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"
//#include "Common_define.h"

typedef enum _TransferEvent
{
	 EVENT_TX_SUCCESS	= 0x01	
	,EVENT_TX_FAILED	= 0x02	
	,EVENT_RX_SUCCESS	= 0x10	
	,EVENT_RX_FAILED	= 0x12	
}TransferEvent;

//�������ݷ����붨��
typedef enum _TX_CODE
{
	TX_SUCCESS = 0	//���ݷ��ͳɹ�
	, TX_FAILED		//���ݷ���ʧ��
	, TX_PENDING	//�ȴ����ͽ��
}TX_CODE;

struct _tagTransfer;
typedef TX_CODE (*TransferFun)(struct _tagTransfer* pTransfer, uint8* pData, uint16 len);
typedef int (*RxCallBack)(void* pChnl, TransferEvent eventId, uint8* pData, uint16 len);

struct _tagChnl;
typedef struct _tagTransfer
{
    TransferFun 	 TxData; 
	
	void* 			m_pArg;
	RxCallBack	 	RxData;
}Transfer;

TX_CODE Transfer_TxData(Transfer* pTransfer, uint8* pData, uint16 len);

void Transfer_RxData(Transfer* pTransfer, uint8* pData, uint16 len);
void Transfer_Init(Transfer* pTransfer, RxCallBack rxFun, void* pArg);


#ifdef __cplusplus
}
#endif

#endif


