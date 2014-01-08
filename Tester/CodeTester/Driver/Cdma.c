#if 1
/******************************************************************************
SysManager.c

描述:
1. 实现系统管理接口函数功能

创建:
林良军   2013.09.3    建立文件.
******************************************************************************/

#include "Cdma.h"
#include "CdmaTester.h"

#ifdef REMOTE_WG_STUB
	#ifdef VXWORKS
		WasReco g_WasTbl[] 		= {{WAS_ID, 0xAC1211B0}};	//172.18.17.176
		WgReco g_WgTbl[] 		= {{WG_ID, {"1201"}, WT_RE, 0xAC1211B0, WAS_ID}};
		RemoteDevReco g_RemoteDevReco[2] = 
		{
			 {REMOTE_DEV_1, "1202", WG_ID}
			,{REMOTE_DEV_2, "1203", WG_ID}
		};
		DevReco g_DeviceTbl[] 	= 
		{
			 {1, 2, WG_ID}
			,{1, 2, WG_ID}
		};
		WGPortConnReco g_PortConnTbl[] = 
		{
			{1, 1, 1}
		};
	#else defined(WIN_32)
		WasReco g_WasTbl[] 		= {{WAS_ID, 0xAC12113B}};	//172.18.17.176
		WgReco g_WgTbl[] 		= {{WG_ID, {"1201"}, WT_RE, 0xAC12113B, WAS_ID}};
		RemoteDevReco g_RemoteDevReco[2] = 
		{
			 {REMOTE_DEV_1, "1202", WG_ID}
			,{REMOTE_DEV_2, "1203", WG_ID}
		};
		DevReco g_DeviceTbl[] 	= 
		{
			 {1, 2, WG_ID}
			,{1, 2, WG_ID}
		};
		WGPortConnReco g_PortConnTbl[] = 
		{
			{1, 1, 1}
		};
	#endif
#else
#include "Devdc.h"
#include "Dev413Bmj.h"

WgReco g_WgTbl[] 		= {WG_ID, {"1201"}, WT_RE, 0xAC1211B0, WAS_ID, 0xAC1211B0};
DevReco g_DeviceTbl[] 	= 
{
	 {DEV_BMJ_1, DT_BMJ_413, WG_ID}
	,{DEV_BMJ_2, DT_BMJ_413, WG_ID}
};

RemoteDevReco g_RemoteDevReco[] = 
{
	 {REMOTE_DEV_1, "1202", WG_ID}
	,{REMOTE_DEV_2, "1203", WG_ID}
};

WGPortConnReco g_PortConnTbl[] = 
{
	 {DC_AUDIO_OUT_1, DEV_BMJ_1, BMJ_AUDIO_IN}
	,{DC_AUDIO_IN_1	, DEV_BMJ_1, BMJ_AUDIO_OUT}
	
	,{DC_PTT_1		, DEV_BMJ_1, BMJ_PTT_IN}
	,{DC_OPTO_IN_1	, DEV_BMJ_1, BMJ_PTT_OUT}
	
	,{DC_OPTO_OUT_2	, DEV_BMJ_1, ENCRYDT_PORT}
	,{DC_OPTO_OUT_3	, DEV_BMJ_1, DECRYDT_PORT}
	
	,{DC_OPTO_IN_2	, DEV_BMJ_1, POWER_READY_PORT}
	,{DC_OPTO_IN_3	, DEV_BMJ_1, ENCRYDT_INDICATE_PORT}
	
	,{DC_AUDIO_OUT_2, DEV_BMJ_2, BMJ_AUDIO_IN}
	,{DC_AUDIO_IN_2	, DEV_BMJ_2, BMJ_AUDIO_OUT}
	
	,{DC_PTT_2		, DEV_BMJ_2, BMJ_PTT_IN}
	,{DC_OPTO_IN_4	, DEV_BMJ_2, BMJ_PTT_OUT}
	
	,{DC_OPTO_OUT_4	, DEV_BMJ_2, ENCRYDT_PORT}
	,{DC_OPTO_OUT_3	, DEV_BMJ_2, DECRYDT_PORT}
	
	,{DC_OPTO_IN_2	, DEV_BMJ_2, POWER_READY_PORT}
	,{DC_OPTO_IN_3	, DEV_BMJ_2, ENCRYDT_INDICATE_PORT}
};
#endif

#if CDMA_STUB
////////////////////////////////////////////////////////////////////////////
//////////////////////////////    ENUM  Definition      ///////////////////////////
////////////////////////////////////////////////////////////////////////////
void TinyCDMAInit(uint8 LocalDeviceType, uint8 LocalDeviceID, uint16 FrameID, uint8 SlotID, uint8 CPUID, char * CDMA_IP, char * CDMS_IP)
{
}

void TinyCDMA_TaskInit()
{
}

Bool cdm_IsDataReady()
{
	return True;
}

//根据表号得到其记录数
int32 cdm_getTableRowNum(uint16 iTableID)
{
	if(TBLID_WAS == iTableID)
	{
		return sizeof(g_WasTbl) / sizeof(WasReco);
	}
	else if(TBLID_WG == iTableID)
	{
		return sizeof(g_WgTbl) / sizeof(WgReco);
	}
	else if(TBLID_WD == iTableID)
	{
		return sizeof(g_DeviceTbl) / sizeof(DevReco);
	}
	else if(TBLID_WD_PORT_CONN_CFG == iTableID)
	{
		return sizeof(g_PortConnTbl) / sizeof(WGPortConnReco);
	}
	else if(TBLID_REMOTE_DEVICE == iTableID)
	{
		return sizeof(g_RemoteDevReco) / sizeof(RemoteDevReco);
	}
	
	return 0;
}

//根据行号得到一条记录数据
int32 cdm_getTableRecord(uint16 iRow, uint16 iTableID, uint8* pData )
{
	Assert(iRow > 0);
	
	if(TBLID_WAS == iTableID)
	{
		if(iRow - 1 < sizeof(g_WasTbl) / sizeof(WasReco))
		{
			memcpy(pData, &g_WasTbl[iRow-1], sizeof(WasReco));
			return 0;
		}
	}
	else if(TBLID_WG == iTableID)
	{
		if(iRow - 1 < sizeof(g_WgTbl) / sizeof(WgReco))
		{
			memcpy(pData, &g_WgTbl[iRow-1], sizeof(WgReco));
			return 0;
		}
	}
	else if(TBLID_WD == iTableID)
	{
		if(iRow - 1 < sizeof(g_DeviceTbl) / sizeof(DevReco))
		{
			memcpy(pData, &g_DeviceTbl[iRow-1], sizeof(DevReco));
			return 0;
		}
	}
	else if(TBLID_WD_PORT_CONN_CFG == iTableID)
	{
		if(iRow - 1 < sizeof(g_PortConnTbl) / sizeof(WGPortConnReco))
		{
			memcpy(pData, &g_PortConnTbl[iRow-1], sizeof(WGPortConnReco));
			return 0;
		}
	}
	else if(TBLID_REMOTE_DEVICE == iTableID)
	{
		if(iRow - 1 < sizeof(g_RemoteDevReco) / sizeof(RemoteDevReco))
		{
			memcpy(pData, &g_RemoteDevReco[iRow-1], sizeof(RemoteDevReco));
			return 0;
		}
	}

	return -1;
}

//根据数据内容找到对应的行号
int16 cdm_getTableRecordLineNo(uint16 iTableID, uint8* pData, uint16 nLineNo)
{
	return 0;
}




//根据索引得到一条记录数据
int32 cdm_getTableRecordByIdx(uint16 nTableID, uint8 nIdxId, uint8 nLen, uint8 *pValue, uint8* pData)
{
	return 0;
}

//根据键值获取记录数据
int32 cdm_getTableRecordByKey(uint16 iTableID, uint8 nKeyLen, void* pKey, uint8* pData)
{
	static WgReco wgReco = {0x1234, {0x01,0x02}, 0, 0x12345678, 0x01};

	memcpy(pData, &wgReco, sizeof(WgReco));
	return 0;
}

#if 0
//注册数据变更通知的回调函数
//typedef int32 (*CDM_DATA_CHANGE_CBF) (int32 iCfgType, int32 iOperType, void* pData);  
/*
iCfgType = tableid
IOperType = add/mod/del，参考枚举CDM_DATA_CHANGE_TYPE
*/

int32 cdm_DynaChgCallback_reg(CDM_DATA_CHANGE_CBF pFun)
{
	return 0;
}


//注册系统管理平台主动下发操作的回调函数
//typedef int (*SER_PROC_CBF) (uint16 nOprType, uint16 nOprID, uint16 nCndDataLen, uint8* pCndData, uint16 &nDataLen, uint8 *pData); 
//可通过nOprID确定具体查询什么信息，目前只支持查询状态信息
//pCndData是查询的输入条件
//pData是查询的输出结果

//对于设备状态查询，opType为Query，目前只需要支持系统管理向无线网关的状态查询
int32 cdm_ServicePorcCallback_reg(uint8 nOpType, SER_PROC_CBF pFn)
{
	return 0;
}


//注册系统管理平台主动下发异步操作的回调函数
//typedef int (*ASYN_SER_PROC_CBF) (uint16 nOprType, uint16 nOprID, uint32 nSeqNo, uint16 nCndDataLen, uint8* pCndData);
//可通过nOprID确定具体查询什么信息，目前只支持查询状态信息
//nSeqNo是查询序号，用来识别并发的查询
//pCndData是查询的输入条件
//pData是查询的输出结果

//对于设备状态查询，opType为Query，目前只需要支持系统管理向无线网关的状态查询。当查询不能立即返回结果时，使用该接口。
int32 cdm_AsynServicePorcCallback_reg(uint8 nOpType, ASYN_SER_PROC_CBF pFn)
{
	return 0;
}
#endif


//异步操作结束后，调用该接口将操作结果传递给CDMA
int32 cdm_AsynServiceProcResult(uint16 nOprType, uint16 nOprID, uint32 nSeqNo, uint16 nDataLen, uint8 *pData)
{
	return 0;
}

//提供给应用层的上报事件的接口函数
int32 cdm_AddEvent(uint16 nEvtID, uint16 nEvtDataLen, uint8 *pEvtData)
{
	return 0;
}


//提供给应用层的上报告警的接口函数
int32 cdm_AlarmReport(uint32 nAlarmId, uint32 nAlarmType, uint32 nParamLen, uint8* pParam)
{
	return 0;
}


//提供给应用层的上报日志的接口函数
int32 cdm_LogReport(uint8 nLevel, uint8 nLogInfoLen, uint8 *pLogData)
{
	return 0;
}

#endif

#endif
