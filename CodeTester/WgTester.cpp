#include "StdAfx.h"
#include "WgTester.h"
#include "CodeTester.h"

#if 0
static Cmd g_cmd = {CMD_MASTER_VER, CMD_SUB_VER, 1, OC_SECTION_INIT_REQ, 1, DEV_BMJ_1};

extern "C"
{
extern int Call_Rpt(unsigned char nCallId,BcCallStatesReport *tReport);
}

int		g_nIpPktLen = 0;
uint8	g_IpBuff[1024];

int		g_nDstIpPktLen = 0;
uint8	g_DstIpBuff[1024];

void WirelessGate_RxIpDataEx(uint32 ipAddr, void* pData, int len)
{
	CmdPacket	cmdPkt;

	cmdPkt.m_FromIpAddress = ipAddr;
	cmdPkt.m_Len = len;
	cmdPkt.m_RxPort = IF2_PORT_RX;
	memcpy(cmdPkt.m_Data, pData, len);

	WirelessGate_RxIpData(&cmdPkt);
}


UINT Tester_WgStart(LPVOID pParam)
{
#if MFC_SOCKET
	g_WgTester.m_WasSocket.Create(IF2_PORT_RX, (RxDataFun)WirelessGate_RxIpDataEx, Null);
#endif
	WirelessGate_Task(Null);

	return 0;
}

UINT TimerRun(LPVOID pVoid)
{
	//uint32 test1,test2 = 1;
	DWORD g_Ticks = 1;
	uint32 ticks = 0;
	WirelessGate* pWirelessGate = (WirelessGate*)pVoid;

	Timermanager_ResetTimer(&g_pWg->m_TimerManager, GetTickCount());
	while(1)
	{
		//test1 =  GetTickCount();
		Sleep(g_Ticks);
		//ShowWait(1000, 32);
		//test2 =  GetTickCount();
			
		ticks = GetTickCount();
		//TRACE("time=%d\n", test2-test1);
		Timermanager_Run(&g_pWg->m_TimerManager, ticks);
	}
}

Bool Tester_WirelessGate_InitWas()
{
	WirelessGate lWirelessGate;
	Debug_Init();
	WirelessGate_Init(&lWirelessGate);
	return True;
}

Bool Tester_WirelessGate_InitDev()
{
	WirelessGate lWirelessGate;
	Debug_Init();
	WirelessGate_Init(&lWirelessGate);
	return True;
}

Bool Tester_Cmd_AddValueTlv()
{
	Cmd cmd = {0};
	Cmd_AddValueTlv(&cmd, 1, 0x12345678, 1);
	AssertTrue(cmd.m_Len == TLV_HEDER_SIZE + 1);
	AssertTrue(cmd.m_Data[cmd.m_Len - 1] == 0x78);

	cmd.m_Len = 0;
	Cmd_AddValueTlv(&cmd, 1, 0x12345678, 2);
	AssertTrue(cmd.m_Len == TLV_HEDER_SIZE + 2);
	AssertTrue(cmd.m_Data[cmd.m_Len - 2] == 0x56);
	AssertTrue(cmd.m_Data[cmd.m_Len - 1] == 0x78);

	cmd.m_Len = 0;
	Cmd_AddValueTlv(&cmd, 1, 0x12345678, 3);
	AssertTrue(cmd.m_Len == TLV_HEDER_SIZE + 3);
	AssertTrue(cmd.m_Data[cmd.m_Len - 3] == 0x34);
	AssertTrue(cmd.m_Data[cmd.m_Len - 2] == 0x56);
	AssertTrue(cmd.m_Data[cmd.m_Len - 1] == 0x78);

	cmd.m_Len = 0;
	Cmd_AddValueTlv(&cmd, 1, 0x12345678, 4);
	AssertTrue(cmd.m_Len == TLV_HEDER_SIZE + 4);
	AssertTrue(cmd.m_Data[cmd.m_Len - 4] == 0x12);
	AssertTrue(cmd.m_Data[cmd.m_Len - 3] == 0x34);
	AssertTrue(cmd.m_Data[cmd.m_Len - 2] == 0x56);
	AssertTrue(cmd.m_Data[cmd.m_Len - 1] == 0x78);


	return True;
}

Bool Tester_WirelessServer_DeviceStart()
{
	DWORD dwThreadId;
	HANDLE m_hWgThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Tester_WgStart,&g_WgTester,NULL,&dwThreadId);
	WAIT(5000, g_pWg && 1 == g_pWg->m_WgId);

	CHECK_STRING("WirelessGate init SUCCESS");
	CHECK_STRING("Device[DT_BMJ_413][1] Config Port[8]<->DcPort[DC_OPTO_IN_3]");
	CHECK_STRING("Device[DT_BMJ_413][1] Start Success");
	CHECK_STRING("Device_RemoveOutFifoHead(DT_BMJ_413), devId=1");
	CHECK_STRING("Device_RemoveOutFifoHead(DT_BMJ_413), devId=2");

	Sleep(1000);

	Device* pDev = WirelessGate_GetDevice(DEV_BMJ_1);
	pDev->Release(pDev);

	pDev = WirelessGate_GetDevice(DEV_BMJ_2);
	pDev->Release(pDev);

	TerminateThread(m_hWgThread, 0);

	return True;
}


Bool Tester_WirelessServer_SectionInit_Ok()
{
	DWORD dwThreadId;
	HANDLE m_hWgThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Tester_WgStart,&g_WgTester,NULL,&dwThreadId);
	WAIT(5000, g_pWg && 1 == g_pWg->m_WgId);
	//HANDLE m_hTimerThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)TimerRun,g_pWg,NULL,&dwThreadId);

	CHECK_STRING("Device[DT_BMJ_413][1] Start Success");
	CHECK_STRING("Device[DT_BMJ_413][2] Start Success");
	CHECK_STRING("Wb[65535].Device[1] Send data");
	CHECK_STRING("Wb[65535].Device[2] Send data");

	CmdPacket* pCmdPacket = &g_WgTester.m_CmdPacket;
	memset(pCmdPacket, 0, sizeof(CmdPacket));

	Cmd* pCmd = (Cmd*)pCmdPacket->m_Data;
	memcpy(pCmd, &g_cmd, CMD_HEADER_SIZE);

	pCmd->m_OpCode = OC_SECTION_INIT_REQ;


	Cmd_AddValueTlv(pCmd, TAG_DEVICE_PORTSET, 0x12345678, 1);	//BMJ_PORTSET_1
	Cmd_AddValueTlv(pCmd, TAG_DEVICE_PORTSET, BMJ_PORTSET_1, 1);

	int nLength = pCmd->m_Len;

	Cmd_Swap(pCmd, H2N);

	Sleep(1000);
	WirelessGate_RxIpDataEx(0xC0A8004B, (uint8*)pCmd, nLength + CMD_HEADER_SIZE);
	CHECK_STRING("WirelessServer_CmdProc(REQ), OpCode=6,len=14, Seq=1");
	CHECK_STRING("Wb_New()");
	CHECK_STRING("Wb_CmdSectionInit(1)");
	CHECK_STRING("Device_BusinessInit(DT_BMJ_413), devId=1");
	CHECK_STRING("Device[DT_BMJ_413][1] state switch from [DS_READY] to [DS_ASSIGNED]");
	CHECK_STRING("Wb[1].Device[1] Send data [23]");

	WirelessGate_RxIpDataEx(0xC0A8004B, (uint8*)pCmd, nLength + CMD_HEADER_SIZE);
	//CHECK_STRING("Discard cmd:");

	Device* pDev = WirelessGate_GetDevice(DEV_BMJ_1);
	pDev->Release(pDev);

	pDev = WirelessGate_GetDevice(DEV_BMJ_2);
	pDev->Release(pDev);

	//TerminateThread(g_hCdmaThread, 0);
	TerminateThread(m_hWgThread, 0);
	//TerminateThread(m_hTimerThread, 0);

	return True;
}	

Bool Tester_WirelessServer_SectionInit_Failed()
{
	DWORD dwThreadId;
	HANDLE m_hWgThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Tester_WgStart,&g_WgTester,NULL,&dwThreadId);
	WAIT(5000, g_pWg && 1 == g_pWg->m_WgId);
	//HANDLE m_hTimerThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)TimerRun,g_pWg,NULL,&dwThreadId);

	CHECK_STRING("Device[DT_BMJ_413][1] Start Success");
	CHECK_STRING("Device[DT_BMJ_413][2] Start Success");
	CHECK_STRING("Wb[65535].Device[1] Send data");
	CHECK_STRING("Wb[65535].Device[2] Send data");

	CmdPacket* pCmdPacket = &g_WgTester.m_CmdPacket;
	memset(pCmdPacket, 0, sizeof(CmdPacket));

	Cmd* pCmd = (Cmd*)pCmdPacket->m_Data;
	memcpy(pCmd, &g_cmd, CMD_HEADER_SIZE);

	pCmd->m_OpCode = OC_SECTION_INIT_REQ;


	//Cmd_AddValueTlv(pCmd, TAG_DEVICE_PORTSET, 0x12345678, 1);	//BMJ_PORTSET_1
	//Cmd_AddValueTlv(pCmd, TAG_DEVICE_PORTSET, BMJ_PORTSET_1, 1);

	int nLength = pCmd->m_Len;

	Cmd_Swap(pCmd, H2N);

	Sleep(1000);
	WirelessGate_RxIpDataEx(0xC0A8004B, (uint8*)pCmd, nLength + CMD_HEADER_SIZE);
	CHECK_STRING("WirelessServer_CmdProc(REQ), OpCode=6");
	CHECK_STRING("Wb_New()");
	CHECK_STRING("Wb_CmdSectionInit(1) Param error, miss [PORT_SET]");
	CHECK_STRING("Wb[1].Device[1] Send data [23]");
	CHECK_STRING("Wb_Release");

	//WirelessGate_RxIpDataEx(0xC0A8004B, (uint8*)pCmd, nLength + CMD_HEADER_SIZE);
	//CHECK_STRING("Discard cmd:");

	Device* pDev = WirelessGate_GetDevice(DEV_BMJ_1);
	pDev->Release(pDev);

	pDev = WirelessGate_GetDevice(DEV_BMJ_2);
	pDev->Release(pDev);

	//TerminateThread(g_hCdmaThread, 0);
	TerminateThread(m_hWgThread, 0);
	//TerminateThread(m_hTimerThread, 0);

	return True;
}	

Bool Tester_WirelessServer_SectionStart()
{
	DWORD dwThreadId;
	HANDLE m_hWgThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Tester_WgStart,&g_WgTester,NULL,&dwThreadId);
	WAIT(5000, g_pWg && 1 == g_pWg->m_WgId);
	//HANDLE m_hTimerThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)TimerRun,g_pWg,NULL,&dwThreadId);

	CHECK_STRING("Device[DT_BMJ_413][1] Start Success");
	CHECK_STRING("Device[DT_BMJ_413][2] Start Success");
	CHECK_STRING("Wb[65535].Device[1] Send data");
	CHECK_STRING("Wb[65535].Device[2] Send data");

	CmdPacket* pCmdPacket = &g_WgTester.m_CmdPacket;
	memset(pCmdPacket, 0, sizeof(CmdPacket));

	Cmd* pCmd = (Cmd*)pCmdPacket->m_Data;
	memcpy(pCmd, &g_cmd, CMD_HEADER_SIZE);

	pCmd->m_OpCode = OC_SECTION_INIT_REQ;


	Cmd_AddValueTlv(pCmd, TAG_DEVICE_PORTSET, 0x12345678, 1);	//BMJ_PORTSET_1
	Cmd_AddValueTlv(pCmd, TAG_DEVICE_PORTSET, BMJ_PORTSET_1, 1);

	int nLength = pCmd->m_Len;

	Cmd_Swap(pCmd, H2N);

	//预置设备DT_BMJ_413 1
	WirelessGate_RxIpDataEx(0xC0A8004B, (uint8*)pCmd, nLength + CMD_HEADER_SIZE);
	CHECK_STRING("WirelessServer_CmdProc(REQ), OpCode=6,len=14, Seq=1");
	CHECK_STRING("Wb_New()");
	CHECK_STRING("Wb_CmdSectionInit(1)");
	CHECK_STRING("Device_BusinessInit(DT_BMJ_413), devId=1");
	CHECK_STRING("Device[DT_BMJ_413][1] state switch from [DS_READY] to [DS_ASSIGNED]");
	CHECK_STRING("Wb[1].Device[1] Send data [23]");

	
	//预置设备DT_BMJ_413 2
	pCmd->m_DeviceId = H2N16(DEV_BMJ_2);
	pCmd->m_Seq = 2;
	pCmd->m_Seq = H2N32(pCmd->m_Seq);
	WirelessGate_RxIpDataEx(0xC0A8004B, (uint8*)pCmd, nLength + CMD_HEADER_SIZE);
	CHECK_STRING("Wb_CmdSectionInit(1) DeviceId[2], SectonId=1");
	CHECK_STRING("Device_BusinessInit(DT_BMJ_413), devId=2");
	CHECK_STRING("Device[DT_BMJ_413][2] state switch from [DS_READY] to [DS_ASSIGNED]");
	CHECK_STRING("Device_RemoveOutFifoHead(DT_BMJ_413), devId=2");
	

	//发送段启动命令 OC_SECTION_START_REQ
	memcpy(pCmd, &g_cmd, CMD_HEADER_SIZE);
	pCmd->m_DeviceId = DEV_BMJ_2;
	pCmd->m_Seq = 3;
	pCmd->m_OpCode = OC_SECTION_START_REQ;
	Cmd_AddValueTlv(pCmd, TAG_TAIL_WG_IP, 0xC0A8001B, 4);

	uint8 tel[16] = {'1','2','3','4','5','6','7','8','9'};;
	Cmd_AddTlv(pCmd, TAG_TAIL_WG_TELNUM, tel, sizeof(tel));

	nLength = pCmd->m_Len;
	Cmd_Swap(pCmd, H2N);
	WirelessGate_RxIpDataEx(0xC0A8004B, (uint8*)pCmd, nLength + CMD_HEADER_SIZE);
	CHECK_STRING("Device_BusinessStart(DT_BMJ_413), devId=2");
	CHECK_STRING("Device_BusinessStart(DT_BMJ_413), devId=1");
	CHECK_STRING("Wrb_TxData(RSP)");
	CHECK_STRING("Wrb_Reset(RSP)");

	//发送段启动命令 OC_SECTION_START_REQ
	memcpy(pCmd, &g_cmd, CMD_HEADER_SIZE);
	pCmd->m_DeviceId = DEV_BMJ_2;
	pCmd->m_Seq = 4;
	pCmd->m_OpCode = OC_SECTION_STOP_REQ;

	nLength = pCmd->m_Len;
	Cmd_Swap(pCmd, H2N);
	WirelessGate_RxIpDataEx(0xC0A8004B, (uint8*)pCmd, nLength + CMD_HEADER_SIZE);
	CHECK_STRING("Wb_Release");
	Sleep(2000);

	Device* pDev = WirelessGate_GetDevice(DEV_BMJ_1);
	pDev->Release(pDev);

	pDev = WirelessGate_GetDevice(DEV_BMJ_2);
	pDev->Release(pDev);

	//TerminateThread(g_hCdmaThread, 0);
	TerminateThread(m_hWgThread, 0);
	//TerminateThread(m_hTimerThread, 0);

	return True;
}	

extern "C"
{
extern uint32 m_Ptt;
};

Bool Tester_WirelessServer_StartD2o()
{
	DWORD dwThreadId;
	HANDLE m_hWgThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Tester_WgStart,&g_WgTester,NULL,&dwThreadId);
	WAIT(5000, g_pWg && 1 == g_pWg->m_WgId);
	//HANDLE m_hTimerThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)TimerRun,g_pWg,NULL,&dwThreadId);

	CHECK_STRING("Device[DT_BMJ_413][1] Start Success");
	CHECK_STRING("Device[DT_BMJ_413][2] Start Success");
	CHECK_STRING("Wb[65535].Device[1] Send data");
	CHECK_STRING("Wb[65535].Device[2] Send data");

	CmdPacket* pCmdPacket = &g_WgTester.m_CmdPacket;
	memset(pCmdPacket, 0, sizeof(CmdPacket));

	//发送段启动命令 OC_SECTION_INIT_REQ
	Cmd* pCmd = (Cmd*)pCmdPacket->m_Data;
	memcpy(pCmd, &g_cmd, CMD_HEADER_SIZE);

	pCmd->m_OpCode = OC_SECTION_INIT_REQ;


	Cmd_AddValueTlv(pCmd, TAG_DEVICE_PORTSET, BMJ_PORTSET_2, 1);

	int nLength = pCmd->m_Len;

	Cmd_Swap(pCmd, H2N);

	//预置设备DT_BMJ_413 1
	WirelessGate_RxIpDataEx(0xC0A8004B, (uint8*)pCmd, nLength + CMD_HEADER_SIZE);
	CHECK_STRING("WirelessServer_CmdProc(REQ), OpCode=6,len=7, Seq=1");
	CHECK_STRING("Wb_New()");
	CHECK_STRING("Wb_CmdSectionInit(1)");
	CHECK_STRING("Device_BusinessInit(DT_BMJ_413), devId=1");
	CHECK_STRING("Device[DT_BMJ_413][1] state switch from [DS_READY] to [DS_ASSIGNED]");
	CHECK_STRING("Wb[1].Device[1] Send data [23]");

	//发送段启动命令 OC_SECTION_START_REQ
	memcpy(pCmd, &g_cmd, CMD_HEADER_SIZE);
	pCmd->m_DeviceId = DEV_BMJ_1;
	pCmd->m_Seq = 3;
	pCmd->m_OpCode = OC_START_REQ;

	nLength = pCmd->m_Len;
	Cmd_Swap(pCmd, H2N);
	WirelessGate_RxIpDataEx(0xC0A8004B, (uint8*)pCmd, nLength + CMD_HEADER_SIZE);

	//发送段启动命令 OC_SECTION_START_REQ
	memcpy(pCmd, &g_cmd, CMD_HEADER_SIZE);
	pCmd->m_DeviceId = DEV_BMJ_1;
	pCmd->m_Seq = 4;
	pCmd->m_OpCode = OC_SECTION_STOP_REQ;

	nLength = pCmd->m_Len;
	Cmd_Swap(pCmd, H2N);
	WirelessGate_RxIpDataEx(0xC0A8004B, (uint8*)pCmd, nLength + CMD_HEADER_SIZE);
	CHECK_STRING("Device_Reset(DT_BMJ_413)");
	CHECK_STRING("Wb_Release");

	Sleep(1000);

	//Wb* pWb = WirelessServer_GetWb(1);
	//pWb->m_pDevice1->Release(pWb->m_pDevice1);
	//pWb->m_pDevice2->Release(pWb->m_pDevice2);

	Device* pDev = WirelessGate_GetDevice(DEV_BMJ_1);
	pDev->Release(pDev);

	pDev = WirelessGate_GetDevice(DEV_BMJ_2);
	pDev->Release(pDev);

	//TerminateThread(g_hCdmaThread, 0);
	TerminateThread(m_hWgThread, 0);
	//TerminateThread(m_hTimerThread, 0);

	return True;
}	

Bool Tester_WirelessServer_StartD2rd()
{
	Device* pDev;
	DWORD dwThreadId;
	HANDLE m_hWgThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Tester_WgStart,&g_WgTester,NULL,&dwThreadId);
	WAIT(5000, g_pWg && 1 == g_pWg->m_WgId);
	//HANDLE m_hTimerThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)TimerRun,g_pWg,NULL,&dwThreadId);

	CHECK_STRING("Device[DT_BMJ_413][1] Start Success");
	CHECK_STRING("Device[DT_BMJ_413][2] Start Success");
	CHECK_STRING("Wb[65535].Device[1] Send data");
	CHECK_STRING("Wb[65535].Device[2] Send data");

	CmdPacket* pCmdPacket = &g_WgTester.m_CmdPacket;
	memset(pCmdPacket, 0, sizeof(CmdPacket));

	Cmd* pCmd = (Cmd*)pCmdPacket->m_Data;
	memcpy(pCmd, &g_cmd, CMD_HEADER_SIZE);

	pCmd->m_OpCode = OC_SECTION_INIT_REQ;


	Cmd_AddValueTlv(pCmd, TAG_DEVICE_PORTSET, BMJ_PORTSET_2, 1);

	int nLength = pCmd->m_Len;

	Cmd_Swap(pCmd, H2N);

	//预置设备DT_BMJ_413 1
	WirelessGate_RxIpDataEx(0xC0A8004B, (uint8*)pCmd, nLength + CMD_HEADER_SIZE);
	CHECK_STRING("WirelessServer_CmdProc(REQ), OpCode=6,len=7, Seq=1");
	CHECK_STRING("Wb_New()");
	CHECK_STRING("Wb_CmdSectionInit(1)");
	CHECK_STRING("Device_BusinessInit(DT_BMJ_413), devId=1");
	CHECK_STRING("Device[DT_BMJ_413][1] state switch from [DS_READY] to [DS_ASSIGNED]");
	CHECK_STRING("Wb[1].Device[1] Send data [23]");

	//发送段启动命令 OC_SECTION_START_REQ
	memcpy(pCmd, &g_cmd, CMD_HEADER_SIZE);
	pCmd->m_DeviceId = DEV_BMJ_1;
	pCmd->m_Seq = 3;
	pCmd->m_OpCode = OC_SECTION_START_REQ;
	Cmd_AddValueTlv(pCmd, TAG_TAIL_WG_IP, 0xC0A8004B, 4);

	uint8 tel[16] = {'1','2','3','4','5','6','7','8','9'};
	Cmd_AddTlv(pCmd, TAG_TAIL_WG_TELNUM, tel, sizeof(tel));
	Cmd_AddValueTlv(pCmd, TAG_TAIL_WG_IP, 0xC0A8004C, 4);

	nLength = pCmd->m_Len;
	Cmd_Swap(pCmd, H2N);
	WirelessGate_RxIpDataEx(0xC0A8004B, (uint8*)pCmd, nLength + CMD_HEADER_SIZE);

	CHECK_STRING("Device_BusinessStart(DT_REMOTE_DEVICE), devId=0");

	BcCallStatesReport report = {0};
	report.nCallId = 1;
	report.nCallState = CALL_STATE_CALL_ACTIVE;
	report.nResCause = 0;
	memcpy(report.szLocalNbr, tel, 16);
	memcpy(report.szLocalNbr, tel, 16);
	SectionDev* pSecDev = (SectionDev*)report.sdp;
	pSecDev->m_DevId = 5;
	pSecDev->m_SectionId = 1;
	report.nSdpLen = sizeof(SectionDev);

	Call_Rpt(1, &report);
	CHECK_STRING("received msg [CALL_STATE_CALL_ACTIVE]");
	CHECK_STRING("Timeslot Connect");

	PF(DL_MAIN, ("\nSet Ptt\n"));
	m_Ptt = 1;
	CHECK_STRING("DevRemote_SetPtt2()");
	CHECK_STRING("Wb[1].Device[5] Send data [16]");

	PF(DL_MAIN, ("\nSend OC_CTRL_REQ\n"));
	pDev = WirelessGate_GetDevice(DEV_BMJ_1);
	DevCmdPkt pkt;
	pCmd = (Cmd*)pkt.m_Data;
	memcpy(pCmd, &g_cmd, CMD_HEADER_SIZE);
	pCmd->m_DeviceId = DEV_BMJ_1;
	pkt.m_Seq = 3;
	pkt.m_OpCode = OC_CTRL_REQ;
	pkt.m_SectionId = 1;
	pkt.m_Len = CMD_HEADER_SIZE;
	Device_PushPktToOutFifo(pDev, &pkt);
	CHECK_STRING("DevRemote_RemoteCtrl2()");

	//发送段启动命令 OC_SECTION_START_REQ
	memcpy(pCmd, &g_cmd, CMD_HEADER_SIZE);
	pCmd->m_DeviceId = DEV_BMJ_1;
	pCmd->m_Seq = 4;
	pCmd->m_OpCode = OC_SECTION_STOP_REQ;

	nLength = pCmd->m_Len;
	Cmd_Swap(pCmd, H2N);
	WirelessGate_RxIpDataEx(0xC0A8004B, (uint8*)pCmd, nLength + CMD_HEADER_SIZE);
	CHECK_STRING("Device[5] send [IF7_OC_STOP_SECTION_REQ]");
	//CHECK_STRING("Resend[1] msg");
	//CHECK_STRING("Resend[2] msg");
	//CHECK_STRING("Resend[3] msg");
	//CHECK_STRING("Timeslot Disconnect");
	//CHECK_STRING("Device_Release(DT_REMOTE_DEVICE)");
	CHECK_STRING("Wb_Release");

	Sleep(1000);

	//Wb* pWb = WirelessServer_GetWb(1);
	//pWb->m_pDevice1->Release(pWb->m_pDevice1);
	//pWb->m_pDevice2->Release(pWb->m_pDevice2);

	pDev = WirelessGate_GetDevice(DEV_BMJ_1);
	pDev->Release(pDev);

	pDev = WirelessGate_GetDevice(DEV_BMJ_2);
	pDev->Release(pDev);

	//TerminateThread(g_hCdmaThread, 0);
	TerminateThread(m_hWgThread, 0);
	//TerminateThread(m_hTimerThread, 0);

	return True;
}	

Bool Tester_WirelessServer_StartRd2d()
{
	SectionDev* pSecDev = Null;
	CmdPacket* pCmdPacket = Null;
	Cmd* pCmd = Null;
	Device* pDev = Null;
	DWORD dwThreadId;
	HANDLE m_hWgThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Tester_WgStart,&g_WgTester,NULL,&dwThreadId);
	WAIT(5000, g_pWg && 1 == g_pWg->m_WgId);
	//HANDLE m_hTimerThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)TimerRun,g_pWg,NULL,&dwThreadId);
	uint8 tel[16] = {'1','2','3','4','5','6','7','8','9'};

	CHECK_STRING("Device[DT_BMJ_413][1] Start Success");
	CHECK_STRING("Device[DT_BMJ_413][2] Start Success");
	CHECK_STRING("Wb[65535].Device[1] Send data");
	CHECK_STRING("Wb[65535].Device[2] Send data");

	PF(DL_MAIN, ("\nReceived CALL_STATE_IN_COMEING_CALL\n"));
	BcCallStatesReport report1 = {0};
	report1.nCallId = 1;
	report1.nCallState = CALL_STATE_IN_COMEING_CALL;
	report1.nResCause = 0;
	//memcpy(report1.szLocalNbr, tel, 16);
	//memcpy(report1.szLocalNbr, tel, 16);
	pSecDev = (SectionDev*)report1.sdp;
	pSecDev->m_DevId = 5;
	pSecDev->m_SectionId = 1;
	report1.nSdpLen = sizeof(SectionDev);
	Call_Rpt(1, &report1);
	CHECK_STRING("received msg [CALL_STATE_IN_COMEING_CALL]");
	CHECK_STRING("CallReject()");
	
	pCmdPacket = &g_WgTester.m_CmdPacket;
	memset(pCmdPacket, 0, sizeof(CmdPacket));

	pCmd = (Cmd*)pCmdPacket->m_Data;
	memcpy(pCmd, &g_cmd, CMD_HEADER_SIZE);

	pCmd->m_OpCode = OC_SECTION_INIT_REQ;

	Cmd_AddValueTlv(pCmd, TAG_DEVICE_PORTSET, BMJ_PORTSET_2, 1);

	int nLength = pCmd->m_Len;

	Cmd_Swap(pCmd, H2N);

	//预置设备DT_BMJ_413 1
	WirelessGate_RxIpDataEx(0xC0A8004B, (uint8*)pCmd, nLength + CMD_HEADER_SIZE);
	CHECK_STRING("WirelessServer_CmdProc(REQ), OpCode=6,len=7, Seq=1");
	CHECK_STRING("Wb_New()");
	CHECK_STRING("Wb_CmdSectionInit(1)");
	CHECK_STRING("Device_BusinessInit(DT_BMJ_413), devId=1");
	CHECK_STRING("Device[DT_BMJ_413][1] state switch from [DS_READY] to [DS_ASSIGNED]");
	CHECK_STRING("Wb[1].Device[1] Send data [23]");

	PF(DL_MAIN, ("\nReceived CALL_STATE_IN_COMEING_CALL\n"));
	report1.nCallId = 1;
	report1.nCallState = CALL_STATE_IN_COMEING_CALL;
	report1.nResCause = 0;
	//memcpy(report1.szLocalNbr, tel, 16);
	//memcpy(report1.szLocalNbr, tel, 16);
	pSecDev = (SectionDev*)report1.sdp;
	pSecDev->m_DevId = 5;
	pSecDev->m_SectionId = 1;
	report1.nSdpLen = sizeof(SectionDev);
	Call_Rpt(1, &report1);
	CHECK_STRING("received msg [CALL_STATE_IN_COMEING_CALL]");
	CHECK_STRING("Wb[1] state switch from [WBS_INIT] to [WBS_START]");
	CHECK_STRING("Timeslot Connect");

	//遥控命令响应
	pDev = WirelessGate_GetDevice(DEV_BMJ_1);
	DevCmdPkt pkt = {0};
	pCmd = (Cmd*)pkt.m_Data;
	memcpy(pCmd, &g_cmd, CMD_HEADER_SIZE);
	pCmd->m_OpCode = OC_CTRL_RSP;
	pkt.m_Seq 		= pCmd->m_Seq;
	pkt.m_OpCode 	= OC_CTRL_RSP;
	pkt.m_Len 		= pCmd->m_Len;
	pkt.m_SectionId = pCmd->m_SectionId;
		
	memcpy(pkt.m_Data, pCmd->m_Data, pCmd->m_Len);
	Device_PushPktToOutFifo(pDev, &pkt);
	CHECK_STRING("Wb[1].Device[5] Send data");
	
	//发送段启动命令 OC_SECTION_STOP_REQ
	memcpy(pCmd, &g_cmd, CMD_HEADER_SIZE);
	pCmd->m_DeviceId = DEV_BMJ_1;
	pCmd->m_Seq = 4;
	pCmd->m_OpCode = OC_SECTION_STOP_REQ;

	nLength = pCmd->m_Len;
	Cmd_Swap(pCmd, H2N);
	WirelessGate_RxIpDataEx(0xC0A8004B, (uint8*)pCmd, nLength + CMD_HEADER_SIZE);
	CHECK_STRING("Timeslot Disconnect");
	CHECK_STRING("Device_Release(DT_REMOTE_DEVICE)");
	CHECK_STRING("Wb_Release");

	Sleep(1000);

	//Wb* pWb = WirelessServer_GetWb(1);
	//pWb->m_pDevice1->Release(pWb->m_pDevice1);
	//pWb->m_pDevice2->Release(pWb->m_pDevice2);

	pDev = WirelessGate_GetDevice(DEV_BMJ_1);
	pDev->Release(pDev);

	pDev = WirelessGate_GetDevice(DEV_BMJ_2);
	pDev->Release(pDev);

	//TerminateThread(g_hCdmaThread, 0);
	TerminateThread(m_hWgThread, 0);
	//TerminateThread(m_hTimerThread, 0);

	return True;
}	

//查询所有的设备状态
Bool Tester_WirelessServer_QueryState_All()
{

	DWORD dwThreadId;
	HANDLE m_hWgThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Tester_WgStart,&g_WgTester,NULL,&dwThreadId);
	WAIT(5000, g_pWg && 1 == g_pWg->m_WgId);

	CHECK_STRING("Device[DT_BMJ_413][1] Config Port[8]<->DcPort[DC_OPTO_IN_3]");
	CHECK_STRING("WirelessGate init SUCCESS");
	CHECK_STRING("Device[DT_BMJ_413][1] Start Success");
	CHECK_STRING("Device_RemoveOutFifoHead(DT_BMJ_413), devId=1");
	CHECK_STRING("Device_RemoveOutFifoHead(DT_BMJ_413), devId=2");

	CmdPacket* pCmdPacket = &g_WgTester.m_CmdPacket;
	memset(pCmdPacket, 0, sizeof(CmdPacket));

	Cmd* pCmd = (Cmd*)pCmdPacket->m_Data;
	memcpy(pCmd, &g_cmd, CMD_HEADER_SIZE);

	pCmd->m_OpCode = OC_QUERY_REQ;

	int nLength = pCmd->m_Len;

	Cmd_Swap(pCmd, H2N);

	WirelessGate_RxIpDataEx(0xC0A8004B, (uint8*)pCmd, nLength + CMD_HEADER_SIZE);

	CHECK_STRING("Wb[1].Device[1] Send data [75]");

	Device* pDev = WirelessGate_GetDevice(DEV_BMJ_1);
	pDev->Release(pDev);

	pDev = WirelessGate_GetDevice(DEV_BMJ_2);
	pDev->Release(pDev);

	TerminateThread(m_hWgThread, 0);

	return True;

}

//查询设备状态
Bool Tester_WirelessServer_QueryState_Ok()
{
	////////////////////////////////////////////////////////////////
	DWORD dwThreadId;
	HANDLE m_hWgThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Tester_WgStart,&g_WgTester,NULL,&dwThreadId);
	WAIT(5000, g_pWg && 1 == g_pWg->m_WgId);

	CHECK_STRING("Device[DT_BMJ_413][1] Config Port[8]<->DcPort[DC_OPTO_IN_3]");
	CHECK_STRING("WirelessGate init SUCCESS");
	CHECK_STRING("Device[DT_BMJ_413][1] Start Success");
	CHECK_STRING("Device_RemoveOutFifoHead(DT_BMJ_413), devId=1");
	CHECK_STRING("Device_RemoveOutFifoHead(DT_BMJ_413), devId=2");

	CmdPacket* pCmdPacket = &g_WgTester.m_CmdPacket;
	memset(pCmdPacket, 0, sizeof(CmdPacket));

	Cmd* pCmd = (Cmd*)pCmdPacket->m_Data;
	memcpy(pCmd, &g_cmd, CMD_HEADER_SIZE);

	pCmd->m_OpCode = OC_QUERY_REQ;
	Cmd_AddValueTlv(pCmd, TAG_DEVICE_COUNT, 3, 1);
	uint16 devid[3] = {0x0100, 0x0200, 0x0300};
	Cmd_AddTlv(pCmd, TAG_DEVICE_ARRAY, devid, sizeof(devid));

	int nLength = pCmd->m_Len;

	Cmd_Swap(pCmd, H2N);

	WirelessGate_RxIpDataEx(0xC0A8004B, (uint8*)pCmd, nLength + CMD_HEADER_SIZE);

	CHECK_STRING("Wb[1].Device[1] Send data [75]");

	Device* pDev = WirelessGate_GetDevice(DEV_BMJ_1);
	pDev->Release(pDev);

	pDev = WirelessGate_GetDevice(DEV_BMJ_2);
	pDev->Release(pDev);

	TerminateThread(m_hWgThread, 0);

	return True;
}

Bool Tester_WirelessServer_Modify_Ming()
{
	////////////////////////////////////////////////////////////////
	DWORD dwThreadId;
	HANDLE m_hWgThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Tester_WgStart,&g_WgTester,NULL,&dwThreadId);
	WAIT(5000, g_pWg && 1 == g_pWg->m_WgId);

	CHECK_STRING("Device[DT_BMJ_413][1] Config Port[8]<->DcPort[DC_OPTO_IN_3]");
	CHECK_STRING("WirelessGate init SUCCESS");
	CHECK_STRING("Device[DT_BMJ_413][1] Start Success");
	CHECK_STRING("Device_RemoveOutFifoHead(DT_BMJ_413), devId=1");
	CHECK_STRING("Device_RemoveOutFifoHead(DT_BMJ_413), devId=2");

	CmdPacket* pCmdPacket = &g_WgTester.m_CmdPacket;
	memset(pCmdPacket, 0, sizeof(CmdPacket));

	Cmd* pCmd = (Cmd*)pCmdPacket->m_Data;
	memcpy(pCmd, &g_cmd, CMD_HEADER_SIZE);

	pCmd->m_OpCode = OC_SET_REQ;
	Cmd_AddValueTlv(pCmd, TAG_DEVID, 1, 2);
	Cmd_AddTlv(pCmd, TAG_BMJ_SET_MING, Null, 0);

	int nLength = pCmd->m_Len;

	Cmd_Swap(pCmd, H2N);

	WirelessGate_RxIpDataEx(0xC0A8004B, (uint8*)pCmd, nLength + CMD_HEADER_SIZE);

	CHECK_STRING("Wb_Init");
	CHECK_STRING("Dev413Bmj_CmdSetMing()");
	CHECK_STRING("Wb[1].Device[1] Send data [16]");
	CHECK_STRING("Wb_Release");

	Device* pDev = WirelessGate_GetDevice(DEV_BMJ_1);
	pDev->Release(pDev);

	pDev = WirelessGate_GetDevice(DEV_BMJ_2);
	pDev->Release(pDev);

	TerminateThread(m_hWgThread, 0);

#if 0
	DWORD dwThreadId;
	HANDLE m_hWgThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Tester_WgStart,&g_WgTester,NULL,&dwThreadId);
	WAIT(500000, g_pWg && 1 == g_pWg->m_WgId);
	//HANDLE m_hTimerThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)TimerRun,g_pWg,NULL,&dwThreadId);

	CmdPacket* pCmdPacket = &g_WgTester.m_CmdPacket;
	memset(pCmdPacket, 0, sizeof(CmdPacket));

	Cmd* pCmd = (Cmd*)pCmdPacket->m_Data;

	pCmd->m_MasterVer = CMD_MASTER_VER;
	pCmd->m_SubVer	= CMD_SUB_VER;

	pCmd->m_Seq = 1;

	pCmd->m_OpCode = OC_SET_REQ;
	pCmd->m_SectionId = 1;
	pCmd->m_DeviceId = 10;

	pCmd->m_Len = 0;
	pCmd->m_Data[0] = 0;

	TlvEr tlvEr;
	TlvEr_Init(&tlvEr, pCmd->m_Data, sizeof(pCmdPacket->m_Data) - CMD_HEADER_SIZE);

	uint8 devType = DT_BMJ_413;
	uint16 devid =  H2N16(11);
	TlvEr_Add(&tlvEr, TAG_DEVTYPE, &devType, 1);
	TlvEr_Add(&tlvEr, TAG_DEVID, &devid, 2);
	TlvEr_Add(&tlvEr, TAG_BMJ_SET_MING, Null, 0);

	pCmd->m_Len = tlvEr.m_Len;
	int nLength = pCmd->m_Len;
	Cmd_Swap(pCmd, H2N);

	Sleep(2000);
	WirelessGate_RxIpDataEx(0xC0A8004B, (uint8*)pCmd, nLength + CMD_HEADER_SIZE);
	WirelessGate_RxIpDataEx(0xC0A8004B, (uint8*)pCmd, nLength + CMD_HEADER_SIZE);
	////////////////////////////////////////////////
	Sleep(2000);
	pCmd->m_MasterVer = CMD_MASTER_VER;
	pCmd->m_SubVer	= CMD_SUB_VER;

	pCmd->m_Seq = 1;

	pCmd->m_OpCode = OC_SET_REQ;

	pCmd->m_SectionId = 1;

	pCmd->m_DeviceId = 10;

	pCmd->m_Len = 0;
	pCmd->m_Data[0] = 0;

	TlvEr_Init(&tlvEr, pCmd->m_Data, sizeof(pCmdPacket->m_Data) - CMD_HEADER_SIZE);

	devType = DT_BMJ_413;
	devid =  H2N16(11);
	TlvEr_Add(&tlvEr, TAG_DEVTYPE, &devType, 1);
	TlvEr_Add(&tlvEr, TAG_DEVID, &devid, 2);
	TlvEr_Add(&tlvEr, TAG_BMJ_SET_MI, Null, 0);

	pCmd->m_Len = tlvEr.m_Len;
	nLength = pCmd->m_Len;
	Cmd_Swap(pCmd, H2N);

	WirelessGate_RxIpDataEx(0xC0A8004B, (uint8*)pCmd, nLength + CMD_HEADER_SIZE);
	/////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////
	Sleep(500000);

	TerminateThread(m_hWgThread, 0);
	//TerminateThread(m_hTimerThread, 0);
#endif
	return True;
}

Bool Tester_WirelessServer_Ptt()
{
	DWORD dwThreadId;
	HANDLE m_hWgThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Tester_WgStart,&g_WgTester,NULL,&dwThreadId);
	WAIT(500000, g_pWg && 1 == g_pWg->m_WgId);
	//HANDLE m_hTimerThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)TimerRun,g_pWg,NULL,&dwThreadId);

	CmdPacket* pCmdPacket = &g_WgTester.m_CmdPacket;
	memset(pCmdPacket, 0, sizeof(CmdPacket));

	Cmd* pCmd = (Cmd*)pCmdPacket->m_Data;

	Sleep(2000);
	pCmd->m_MasterVer = CMD_MASTER_VER;
	pCmd->m_SubVer	= CMD_SUB_VER;

	pCmd->m_Seq = 1;
	pCmd->m_OpCode = IF7_OC_PTT_OPEN;
	pCmd->m_SectionId = 1;
	pCmd->m_DeviceId = 10;

	pCmd->m_Len = 0;
	pCmd->m_Data[0] = 0;

	TlvEr tlvEr;
	TlvEr_Init(&tlvEr, pCmd->m_Data, sizeof(pCmdPacket->m_Data) - CMD_HEADER_SIZE);

	uint8 devType = DT_BMJ_413;
	uint16 devid =  H2N16(11);
	TlvEr_Add(&tlvEr, TAG_DEVTYPE, &devType, 1);
	TlvEr_Add(&tlvEr, TAG_DEVID, &devid, 2);

	uint16 length = tlvEr.m_Len;
	pCmd->m_Len = tlvEr.m_Len;

	Cmd_Swap(pCmd, H2N);

	Sleep(1000);
	WirelessGate_RxIpDataEx(0xC0A8004B, (uint8*)pCmd, length + CMD_HEADER_SIZE);

	Sleep(1000);

	TerminateThread(m_hWgThread, 0);
	//TerminateThread(m_hTimerThread, 0);

	return True;
}


Bool Tester_Main()
{
#if MFC_SOCKET
	g_WgTester.m_WasSocket.Create(IF2_PORT_RX, (RxDataFun)WirelessGate_RxIpDataEx, Null);
#endif
	
	WirelessGate_Task(Null);

	return True;
}

TreeNodeData g_WgUnitTestItem[] = 
{
	
	{1, L"WirelessGate模块单元测试"}
		,{2, L"WirelessGate_Init()",	(TesterFun)Tester_WirelessGate_InitWas}
		,{2, L"WirelessGate_InitDev()",	(TesterFun)Tester_WirelessGate_InitDev}
		,{2, L"WirelessGate_GetDev()",	(TesterFun)Null}
		,{2, L"WirelessGate_TimerStart()",	(TesterFun)Null}
		,{2, L"WirelessGate_AddMsgIf()",	(TesterFun)Null}
		,{2, L"WirelessGate_GetDevice()",	(TesterFun)Null}
		,{2, L"WirelessGate_CreateDevice()",	(TesterFun)Null}
		,{2, L"WirelessGate_ParseDevRepo()",	(TesterFun)Null}
		,{2, L"WirelessGate_CheckDeviceOutFifo()",	(TesterFun)Null}
		,{2, L"WirelessGate_CmdQueryDevState()",	(TesterFun)Null}
		,{2, L"Cmd_AddValueTlv()",	(TesterFun)Tester_Cmd_AddValueTlv}
		
	 ,{1, L"WirelessServer模块单元测试"}
		,{2, L"WirelessServer_DeviceStart()",	(TesterFun)Tester_WirelessServer_DeviceStart}
		,{2, L"WirelessServer_SectionInitOk()",	(TesterFun)Tester_WirelessServer_SectionInit_Ok}
		,{2, L"WirelessServer_SectionInitFailed()",	(TesterFun)Tester_WirelessServer_SectionInit_Failed}
		,{2, L"WirelessServer_SectionStart()",	(TesterFun)Tester_WirelessServer_SectionStart}
		,{2, L"WirelessServer_StartD2o()",	(TesterFun)Tester_WirelessServer_StartD2o}
		,{2, L"WirelessServer_StartD2rd()",	(TesterFun)Tester_WirelessServer_StartD2rd}
		,{2, L"WirelessServer_StartRd2d()",	(TesterFun)Tester_WirelessServer_StartRd2d}
		,{2, L"WirelessServer_QueryState_All()",	(TesterFun)Tester_WirelessServer_QueryState_All}
		,{2, L"WirelessServer_QueryState()",	(TesterFun)Tester_WirelessServer_QueryState_Ok}
		,{2, L"WirelessServer_ModifyDev()",	(TesterFun)Tester_WirelessServer_Modify_Ming}
		,{2, L"WirelessServer_Ptt()",	(TesterFun)Tester_WirelessServer_Ptt}
};

TreeNodeData g_WgIntegTestItem[] = 
{
	{1, L"WirelessGate_Main()",	(TesterFun)Tester_Main}
};
CWgTester g_WgTester;

UINT Tester_SendToWg(LPVOID pParam)
{
	CmdPacket* pCmdPacket = &g_WgTester.m_CmdPacket;
	while(1)
	{
		if(1 == pCmdPacket->m_Reserved)
		{
			pCmdPacket->m_Reserved = 0;
			WirelessGate_RxIpDataEx(0x12345678, (uint8*)pCmdPacket, pCmdPacket->m_Len + CMDPACKET_HEAD_SIZE);
		}
		Sleep(100);
	}
}

CWgTester::CWgTester(void)
{
	//增加单元测试项///////////////////////////////////////////
	memset(&m_CmdPacket, 0, sizeof(m_CmdPacket));

	g_TestCaseTreeData.AddUnitTreeNodeData(g_WgUnitTestItem, sizeof(g_WgUnitTestItem)/sizeof(TreeNodeData));
	g_TestCaseTreeData.AddIntegTreeNodeData(g_WgIntegTestItem, sizeof(g_WgIntegTestItem)/sizeof(TreeNodeData));

	DWORD m_RxThreadId;
	m_hRxThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Tester_SendToWg,this,NULL,&m_RxThreadId);
}


CWgTester::~CWgTester(void)
{
	if(m_hRxThread)
	{
		TerminateThread(m_hRxThread, 0);
		m_hRxThread = 0;
	}
}

#if MFC_SOCKET
extern "C"{
int IpSendTo(uint32 ipAddr, uint16 port, const void* pData, uint16 nLen)
{
	if(g_WgTester.m_WasSocket.SendData(ipAddr, port, (BYTE*)pData, nLen))
	{
		return nLen;
	}
	else
	{
		return 0;
	}
}
}
#endif
#endif
