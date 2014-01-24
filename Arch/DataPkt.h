#ifndef  __DataPkt_H_
#define  __DataPkt_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "typedef.h"

//ִ������ķ�����
typedef enum _RSP_CODE
{
	RSP_SUCCESS = 0		//��Ӧ�ɹ�
	,RSP_FAILED		//��Ӧʧ��
	,RSP_PENDING	//��Ӧ�ȴ�
	,RSP_DISCARD	//��Ӧ����
}RSP_CODE;

typedef Bool (*PktDescIsFun)(void* pCmdBuff, int16 nLen);
typedef Bool (*PktDescIsRspForReq)(void* pReq, int16 nReqLen, void* pRsp, int16 nRspLen);
//�������ݰ���������һ���������ݰ����ܰ���3���� [����ͷ][��������][����β(У��)]
//����ֱ����������ֵĳ���
typedef struct _tagPktDesc
{
	uint16 	m_MaxLen;		//�������󳤶�      

	PktDescIsFun 		IsReq;
	PktDescIsFun 		Verify;
	PktDescIsRspForReq 	IsRspForReq;
}PktDesc;

void PktDesc_Init(PktDesc* pPktDesc
	, uint16 maxLen
	, PktDescIsFun IsReq
	, PktDescIsFun Verify
	, PktDescIsRspForReq IsRspForReq
	);

typedef struct  _tagDataPkt
{	
	uint8 	m_isUsed:1;  	//ָʾ��m_pCmd�Ƿ�ʹ�ã����ΪTrue,�������ռ��
	uint8 	m_Reserved:7;  
	
	uint16 	m_MaxLen;                
	uint16 	m_DataLen;                
	uint8* 	m_pData;  
}DataPkt;

void DataPkt_Init(DataPkt* pCmd, uint8* pCmdBuff, uint16 nBufLen);

void DataPkt_Reset(DataPkt* pCmd);

#ifdef __cplusplus
}
#endif

#endif



