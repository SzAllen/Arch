#if 1
/******************************************************************************
SysManager.c

����:
1. ʵ��ϵͳ����ӿں�������

����:
������   2013.09.3    �����ļ�.
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

//���ݱ�ŵõ����¼��
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

//�����кŵõ�һ����¼����
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

//�������������ҵ���Ӧ���к�
int16 cdm_getTableRecordLineNo(uint16 iTableID, uint8* pData, uint16 nLineNo)
{
	return 0;
}




//���������õ�һ����¼����
int32 cdm_getTableRecordByIdx(uint16 nTableID, uint8 nIdxId, uint8 nLen, uint8 *pValue, uint8* pData)
{
	return 0;
}

//���ݼ�ֵ��ȡ��¼����
int32 cdm_getTableRecordByKey(uint16 iTableID, uint8 nKeyLen, void* pKey, uint8* pData)
{
	static WgReco wgReco = {0x1234, {0x01,0x02}, 0, 0x12345678, 0x01};

	memcpy(pData, &wgReco, sizeof(WgReco));
	return 0;
}

#if 0
//ע�����ݱ��֪ͨ�Ļص�����
//typedef int32 (*CDM_DATA_CHANGE_CBF) (int32 iCfgType, int32 iOperType, void* pData);  
/*
iCfgType = tableid
IOperType = add/mod/del���ο�ö��CDM_DATA_CHANGE_TYPE
*/

int32 cdm_DynaChgCallback_reg(CDM_DATA_CHANGE_CBF pFun)
{
	return 0;
}


//ע��ϵͳ����ƽ̨�����·������Ļص�����
//typedef int (*SER_PROC_CBF) (uint16 nOprType, uint16 nOprID, uint16 nCndDataLen, uint8* pCndData, uint16 &nDataLen, uint8 *pData); 
//��ͨ��nOprIDȷ�������ѯʲô��Ϣ��Ŀǰֻ֧�ֲ�ѯ״̬��Ϣ
//pCndData�ǲ�ѯ����������
//pData�ǲ�ѯ��������

//�����豸״̬��ѯ��opTypeΪQuery��Ŀǰֻ��Ҫ֧��ϵͳ�������������ص�״̬��ѯ
int32 cdm_ServicePorcCallback_reg(uint8 nOpType, SER_PROC_CBF pFn)
{
	return 0;
}


//ע��ϵͳ����ƽ̨�����·��첽�����Ļص�����
//typedef int (*ASYN_SER_PROC_CBF) (uint16 nOprType, uint16 nOprID, uint32 nSeqNo, uint16 nCndDataLen, uint8* pCndData);
//��ͨ��nOprIDȷ�������ѯʲô��Ϣ��Ŀǰֻ֧�ֲ�ѯ״̬��Ϣ
//nSeqNo�ǲ�ѯ��ţ�����ʶ�𲢷��Ĳ�ѯ
//pCndData�ǲ�ѯ����������
//pData�ǲ�ѯ��������

//�����豸״̬��ѯ��opTypeΪQuery��Ŀǰֻ��Ҫ֧��ϵͳ�������������ص�״̬��ѯ������ѯ�����������ؽ��ʱ��ʹ�øýӿڡ�
int32 cdm_AsynServicePorcCallback_reg(uint8 nOpType, ASYN_SER_PROC_CBF pFn)
{
	return 0;
}
#endif


//�첽���������󣬵��øýӿڽ�����������ݸ�CDMA
int32 cdm_AsynServiceProcResult(uint16 nOprType, uint16 nOprID, uint32 nSeqNo, uint16 nDataLen, uint8 *pData)
{
	return 0;
}

//�ṩ��Ӧ�ò���ϱ��¼��Ľӿں���
int32 cdm_AddEvent(uint16 nEvtID, uint16 nEvtDataLen, uint8 *pEvtData)
{
	return 0;
}


//�ṩ��Ӧ�ò���ϱ��澯�Ľӿں���
int32 cdm_AlarmReport(uint32 nAlarmId, uint32 nAlarmType, uint32 nParamLen, uint8* pParam)
{
	return 0;
}


//�ṩ��Ӧ�ò���ϱ���־�Ľӿں���
int32 cdm_LogReport(uint8 nLevel, uint8 nLogInfoLen, uint8 *pLogData)
{
	return 0;
}

#endif

#endif
