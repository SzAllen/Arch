#ifndef  __CmdItem_H_
#define  __CmdItem_H_

#ifdef _cplusplus
extern "c"{
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


typedef Bool (*PacketDescIsFun)(void* pCmdBuff, int16 nLen);
typedef Bool (*PacketDescRspIsMatchWithReq)(void* pReq, int16 nReqLen, void* pRsp, int16 nRspLen);
//�������ݰ���������һ���������ݰ����ܰ���3���� [����ͷ][��������][����β(У��)]
//����ֱ����������ֵĳ���
typedef struct _tagPacketDesc
{
	uint16 	m_MaxLen;		//�������󳤶�      

	PacketDescIsFun 				IsReq;
	PacketDescIsFun 				Verify;
	PacketDescRspIsMatchWithReq 	RspIsMatchWithReq;
}PacketDesc;

void PacketDesc_Init(PacketDesc* pPacketDesc
	, uint16 maxLen
	, PacketDescIsFun IsReq
	, PacketDescIsFun Verify
	, PacketDescRspIsMatchWithReq RspIsMatchWithReq
	);

typedef struct  _tagCmdItem
{	
	uint8 	m_isUsed:1;  	//ָʾ��m_pCmd�Ƿ�ʹ�ã����ΪTrue,�������ռ��
	uint8 	m_Reserved:7;  
	
	uint16 	m_MaxLen;                
	uint16 	m_CmdLen;                
	uint8* 	m_pCmd;  
}CmdItem;

void CmdItem_Init(CmdItem* pCmd, uint8* pCmdBuff, uint16 nBufLen);

void CmdItem_Reset(CmdItem* pCmd);

#ifdef _cplusplus
}
#endif

#endif



