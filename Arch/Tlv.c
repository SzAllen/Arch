
#include "ArchDef.h"
#include "Tlv.h"

void Tlv_Dump(uint32 level, const uint8* pTlvData, uint16 len)
{
	int j = 0;
	int i = 0;
	const Tlv* pTlv = Null;

	PF(level,("Tlv Dump, len=%d: ", len));
	for(i = 0; i < len; )
	{
		pTlv = (Tlv*)&pTlvData[i];
		
		PF(level,("[%08x][%04x]", pTlv->m_Tag, pTlv->m_Len));
		for(j = 0; j < pTlv->m_Len; j++)
		{
			PF(level,("%02x ", pTlv->m_Data[j]));
		}

		i += pTlv->m_Len + TLV_HEDER_SIZE;
	}
	PF(level,("\n"));
}

const Tlv* Tlv_Get(const uint8* pTlvData, uint16 len, uint32 tag)
{
	uint16 nIndex = 0;
	return Tlv_Find(pTlvData, &nIndex, len, tag);
}

const uint8* Tlv_GetValue(const uint8* pTlvData, uint16 len, uint32 tag)
{
	uint16 nIndex = 0;
	const Tlv* pTlv = Tlv_Find(pTlvData, &nIndex, len, tag);

	if(pTlv)
	{
		return pTlv->m_Data;
	}

	return Null;
}

uint16 Tlv_GetByteArray(const uint8* pTlvData, uint16 len, uint32 tag, uint8* pBuf, uint16 nLen)
{
	uint16 nIndex = 0;
	const Tlv* pTlv = Tlv_Find(pTlvData, &nIndex, len, tag);

	if(pTlv && nLen <= pTlv->m_Len)
	{
		memcpy(pBuf, pTlv->m_Data, pTlv->m_Len);
		return pTlv->m_Len;
	}

	return 0;
}

Bool Tlv_GetUint32(const uint8* pTlvData, uint16 len, uint32 tag, uint32* result)
{
	uint16 nIndex = 0;
	const Tlv* pTlv = Tlv_Find(pTlvData, &nIndex, len, tag);

	if(pTlv)
	{
		if(4 == pTlv->m_Len)
		{
			*result = AS_UINT32(pTlv->m_Data[0], pTlv->m_Data[1], pTlv->m_Data[2], pTlv->m_Data[3]);
		}
		else if(2 == pTlv->m_Len)
		{
			*result = AS_UINT16(pTlv->m_Data[0], pTlv->m_Data[1]);
		}
		else if(1 == pTlv->m_Len)
		{
			*result = pTlv->m_Data[0];
		}
		else
		{
			return False;
		}
		
		return True;
	}

	return False;
}

Bool Tlv_GetUint16(const uint8* pTlvData, uint16 len, uint32 tag, uint16* result)
{
	uint16 nIndex = 0;
	const Tlv* pTlv = Tlv_Find(pTlvData, &nIndex, len, tag);

	if(pTlv)
	{
		if(2 == pTlv->m_Len)
		{
			*result = AS_UINT16(pTlv->m_Data[0], pTlv->m_Data[1]);
		}
		else if(1 == pTlv->m_Len)
		{
			*result = pTlv->m_Data[0];
		}
		else
		{
			return False;
		}
		
		return True;
	}

	return False;
}

Bool Tlv_GetUint8(const uint8* pTlvData, uint16 len, uint32 tag, uint8* result)
{
	uint16 nIndex = 0;
	const Tlv* pTlv = Tlv_Find(pTlvData, &nIndex, len, tag);

	if(pTlv && 1 == pTlv->m_Len)
	{
		*result = pTlv->m_Data[0];
		return True;
	}

	return False;
}

void Tlv_Swap(const uint8* pTlvData, uint16 len, Bool isHtN)
{
	int i = 0; 
	Tlv* pTlv = Null;
	uint16 length = 0;
	
	for(i = 0; i < len; )
	{
		pTlv = (Tlv*)&pTlvData[i];

		if(isHtN)
		{
			length = pTlv->m_Len;
		}
		else
		{
			length = H2N16(pTlv->m_Len);
		}

		pTlv->m_Tag = H2N32(pTlv->m_Tag);
		pTlv->m_Len = H2N16(pTlv->m_Len);

		i += TLV_HEDER_SIZE + length;
	}
}

const Tlv* Tlv_Find(const uint8* pTlvData, uint16* nIndex, uint16 len, uint32 tag)
{
	int nPos = * nIndex;
	int i = 0; 
	const uint8* pData = pTlvData;
	Tlv* pTlv = Null;

	if(nPos >= len) return Null;

	for(i = nPos; i < len; )
	{
		pTlv = (Tlv*)&pData[i];

		if(tag == pTlv->m_Tag)
		{
			*nIndex = i;
			
			return pTlv;
		}
		i += TLV_HEDER_SIZE + pTlv->m_Len;
	}

	return Null;
}
/* 调用举例
{
	uint16 nIndex = 0;
	while(pTlvData)
	{
		pTlvData = Tlv_Find(pTlvData, len, &nIndex, TAG);
	}
}
*/


//函数说明:增加Tag数据到已有的Tag数据的后面
//参数:
//	pTagData: TagData的Buffer数据指针
//	tagLen:	tag数据长度
//	maxLen:	tag数据最大长度
//返回值:
//	如果返回的值和tagLen相等，说明函数执行失败,Tag的Buffer已经满,否则返回增加只有的Tag长度
uint16 Tlv_Add(uint8* pTlvData, uint16 tlvLen, uint16 maxLen, const Tlv* pTag)
{
	uint16 len = tlvLen + TLV_HEDER_SIZE + pTag->m_Len;

	if(len <= maxLen)
	{
		memcpy(&pTlvData[tlvLen], pTag, TLV_HEDER_SIZE + pTag->m_Len);

		return len;
	}

	return tlvLen;
}

uint16 Tlv_AddEx(uint8* pTlvData, uint16 maxLen, uint32 tag, const void* pData, uint16 dataLen)
{
	int nIndex = 0;
	uint16 len = TLV_HEDER_SIZE + dataLen;

	if(len <= maxLen)
	{
		memcpy(&pTlvData[nIndex], &tag, sizeof(tag));
		nIndex += 4;

		memcpy(&pTlvData[nIndex], &dataLen, 2);
		nIndex += 2;

		if(dataLen > 0)
		{
			memcpy(&pTlvData[nIndex], pData, dataLen);
			nIndex += dataLen;
		}
	}

	return nIndex;
}

uint16 Tlv_AddValue(uint8* pTlvBuf, uint16 bufLen, uint32 tag, uint32 value, uint8 len)
{
	uint16 retLen = 0;
	uint8 data[4] = {0};

	Assert(len <= 4);
	Assert(len <= bufLen);

	data[0] = (uint8)value;
	
	if(len == 2)
	{
		data[1] = (uint8)value;
		data[0] = (uint8)(value >> 8);
	}
	else if(len == 3)
	{
		data[2] = (uint8)value;
		data[1] = (uint8)(value >> 8);
		data[0] = (uint8)(value >> 16);
	}
	else if(len == 4)
	{
		data[3] = (uint8)value;
		data[2] = (uint8)(value >> 8);
		data[1] = (uint8)(value >> 16);
		data[0] = (uint8)(value >> 24);
	}
	
	retLen = Tlv_AddEx(pTlvBuf, bufLen, tag, &data, len);

	return retLen;
}

void TlvEr_Swap(TlvEr* pTlvEr, Bool isHtN)
{
	Tlv_Swap(pTlvEr->m_pTlv, pTlvEr->m_Len, isHtN);
}

const Tlv* TlvEr_Get(TlvEr* pTlvEr, uint32 tag)
{
	uint16 nIndex = 0;
	return TlvEr_Find(pTlvEr, tag, &nIndex);
}

const Tlv* TlvEr_Find(TlvEr* pTlvEr, uint32 tag, uint16* pStartIndex)
{
	if(*pStartIndex >= pTlvEr->m_Len)
	{
		return Null;
	}
	else
	{
		return Tlv_Find(pTlvEr->m_pTlv, pStartIndex, pTlvEr->m_Len - *pStartIndex, tag);
	}
}

Bool TlvEr_AddTlv(TlvEr* pTlvEr, const Tlv* pTlv)
{
	if(pTlvEr->m_Len + pTlv->m_Len + TLV_HEDER_SIZE >= pTlvEr->m_MaxLen)
	{
		PF_FAILED();
		return False;
	}

	memcpy(&pTlvEr->m_pTlv[pTlvEr->m_Len], pTlv, pTlv->m_Len + TLV_HEDER_SIZE);

	pTlvEr->m_Len += pTlv->m_Len + TLV_HEDER_SIZE;

	return True;
}

Bool TlvEr_AddData(TlvEr* pTlvEr, uint32 tag, uint8* pData, uint8 len)
{
	int temp = pTlvEr->m_Len;

	pTlvEr->m_Len += Tlv_AddEx(&pTlvEr->m_pTlv[pTlvEr->m_Len]
		, pTlvEr->m_MaxLen - pTlvEr->m_Len
		, tag
		, pData
		, len);

	return (temp != pTlvEr->m_Len);
}

Bool TlvEr_AddValue(TlvEr* pTlvEr, uint32 tag, uint32 value, uint8 len)
{
	int temp = pTlvEr->m_Len;
	
	pTlvEr->m_Len += Tlv_AddValue(&pTlvEr->m_pTlv[pTlvEr->m_Len]
		, pTlvEr->m_MaxLen - pTlvEr->m_Len
		, tag
		, value
		, len);

	return (temp != pTlvEr->m_Len);
}

Bool TlvEr_Add(TlvEr* pTlvEr, uint32 tag, const void* pData, uint16 len)
{
	if(pTlvEr->m_Len + len + TLV_HEDER_SIZE >= pTlvEr->m_MaxLen)
	{
		PF_FAILED();
		return False;
	}

	memcpy(&pTlvEr->m_pTlv[pTlvEr->m_Len], &tag, sizeof(tag));
	pTlvEr->m_Len += sizeof(tag);
	
	memcpy(&pTlvEr->m_pTlv[pTlvEr->m_Len], &len, sizeof(len));
	pTlvEr->m_Len += sizeof(len);

	if(len > 0)
	{
		memcpy(&pTlvEr->m_pTlv[pTlvEr->m_Len], pData, len);
		pTlvEr->m_Len += len;
	}

	return True;
}

void TlvEr_Reset(TlvEr* pTlvEr)
{
	memset(pTlvEr->m_pTlv, 0, pTlvEr->m_MaxLen);
	pTlvEr->m_Len = 0;
}

void TlvEr_Init(TlvEr* pTlvEr, uint8* pData, uint16 nLen)
{
	memset(pTlvEr, 0, sizeof(TlvEr));

	pTlvEr->m_MaxLen = nLen;
	pTlvEr->m_pTlv = pData;
}

