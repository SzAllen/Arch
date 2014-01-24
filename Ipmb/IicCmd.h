#ifndef  __IIC_CMD_H__
#define  __IIC_CMD_H__

#ifdef _cplusplus
extern "c"{
#endif

#include "typedef.h"
#include "IpmiCmd.h"

#define MIN_IPMICMD_LEN   		4 
#define MAX_IPMICMD_LEN 		32
#define IICMSG_HEAD_SIZE 		6
#define MAX_IPMICMD_DATA_LEN 	(MAX_IPMICMD_LEN - IICMSG_HEAD_SIZE - 1/*Check sum*/)

typedef struct  _tagIicMsg
{
	unsigned char  rsSA;  
	
	#ifdef BIG_EDIAN
	unsigned char  rsNetFn:6;	//MSB           
	unsigned char  rsLun:2;		//LSB
	#else
	unsigned char  rsLun:2;		//LSB
	unsigned char  rsNetFn:6;   //MSB             
	#endif
	
	unsigned char  checkSum;                 
	unsigned char  rqSA;              
	
	#ifdef BIG_EDIAN
	unsigned char  rqSeq:6;		//MSB
	unsigned char  rqLun:2;		//LSB
	#else
	unsigned char  rqLun:2;		//LSB
	unsigned char  rqSeq:6;		//MSB
	#endif  
	
	unsigned char  cmd;                      
	unsigned char  smsgBuf[MAX_IPMICMD_DATA_LEN + 1/*Check sum*/];     
}IicMsg;

void IicMsg_GetHeader(IicMsg* pIicMsg, IpmiHeader* pIpmiHeader);
Bool IicMsg_Verify(IicMsg* pIicMsg, int8 len);
void IicMsg_Packet(IicMsg* pIicMsg, IpmiHeader* pHeader, uint8 rqSA, uint8 rqLun, uint8 rsSA);
uint8 IicMsg_GetRsp(const IicMsg* pReq, uint8* pRspBuff, uint8 rspLen, uint8* rspData, uint8 rspDataLen);

/////////////////////////////////////////////////////////////////////////////////////

typedef struct _IicCmd
{
	IpmiCmd	m_Base;
	IicMsg	m_IicMsg;
}IicCmd;

void IicCmd_Init(IicCmd* pIicCmd);
void IicCmd_SetDataLen(IicCmd* pIicCmd, uint8 cmdLen);
uint8 IicCmd_GetDataLen(IicCmd* pIicCmd);

#ifdef _cplusplus
}
#endif

#endif 


