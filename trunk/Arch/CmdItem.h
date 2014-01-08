#ifndef  __CmdItem_H_
#define  __CmdItem_H_

#ifdef _cplusplus
extern "c"{
#endif

#include "typedef.h"

//执行请求的返回码
typedef enum _RSP_CODE
{
	RSP_SUCCESS = 0		//响应成功
	,RSP_FAILED		//响应失败
	,RSP_PENDING	//响应等待
	,RSP_DISCARD	//响应丢弃
}RSP_CODE;


typedef Bool (*PacketDescIsFun)(void* pCmdBuff, int16 nLen);
typedef Bool (*PacketDescRspIsMatchWithReq)(void* pReq, int16 nReqLen, void* pRsp, int16 nRspLen);
//命令数据包描述符，一个命令数据包可能包括3部分 [命令头][命令数据][命令尾(校验)]
//下面分别描述三部分的长度
typedef struct _tagPacketDesc
{
	uint16 	m_MaxLen;		//命令的最大长度      

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
	uint8 	m_isUsed:1;  	//指示出m_pCmd是否被使用，如果为True,则表明被占用
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



