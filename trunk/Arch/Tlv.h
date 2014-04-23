#ifndef  __TLV_H_
#define  __TLV_H_

#ifdef _cplusplus
extern "C"{
#endif

#include "typedef.h"

#define ADD_ACK_TLV(pTlvEr, var) {uint8 value = var;TlvEr_Add(pTlvEr, TAG_COMPLETE, &value, 1);}


#pragma pack(1)
typedef struct _Tlv
{
	uint32	m_Tag;
	uint16	m_Len;
	uint8	m_Data[1];
}Tlv;
#pragma pack()
#define TLV_HEDER_SIZE  (sizeof(Tlv)-1)

void Tlv_Dump(uint32 level, const uint8* pTlvData, uint16 len);
const Tlv* Tlv_Get(const uint8* pTlvData, uint16 len, uint32 tag);
const uint8* Tlv_GetValue(const uint8* pTlvData, uint16 len, uint32 tag);
const Tlv* Tlv_Find(const uint8* pTlvData, uint16* nIndex, uint16 len, uint32 tag);
uint16 Tlv_Add(uint8* pTagData, uint16 tagLen, uint16 maxLen, const Tlv* pTag);
uint16 Tlv_AddItem(uint8* pTlvData, uint16 maxLen, uint32 tag, const uint8* pData, uint16 len);
uint16 Tlv_AddEx(uint8* pTlvData, uint16 maxLen, uint32 tag, const void* pData, uint16 dataLen);
uint16 Tlv_AddValue(uint8* pTlvBuf, uint16 bufLen, uint32 tag, uint32 value, uint8 len);
void Tlv_Swap(const uint8* pTlvData, uint16 len, Bool isHtN);

Bool Tlv_GetUint8(const uint8* pTlvData, uint16 len, uint32 tag, uint8* result);
Bool Tlv_GetUint16(const uint8* pTlvData, uint16 len, uint32 tag, uint16* result);
Bool Tlv_GetUint32(const uint8* pTlvData, uint16 len, uint32 tag, uint32* result);
uint16 Tlv_GetByteArray(const uint8* pTlvData, uint16 len, uint32 tag, uint8* pBuf, uint16 nLen);

typedef struct _TlvEr
{
	uint16	m_MaxLen;
	uint16	m_Len;
	uint8*	m_pTlv;
}TlvEr;
void TlvEr_Init(TlvEr* pTlvEr, uint8* pData, uint16 nLen);
Bool TlvEr_Add(TlvEr* pTlvEr, uint32 tag, const void* pData, uint16 len);
Bool TlvEr_AddTlv(TlvEr* pTlvEr, const Tlv* pTlv);
const Tlv* TlvEr_Find(TlvEr* pTlvEr, uint32 tag, uint16* pStartIndex);
void TlvEr_Reset(TlvEr* pTlvEr);
const Tlv* TlvEr_Get(TlvEr* pTlvEr, uint32 tag);
void TlvEr_Swap(TlvEr* pTlvEr, Bool isHtN);
Bool TlvEr_AddValue(TlvEr* pTlvEr, uint32 tag, uint32 value, uint8 len);
Bool TlvEr_AddData(TlvEr* pTlvEr, uint32 tag, uint8* pData, uint8 len);

#ifdef _cplusplus
}
#endif

#endif



