#include "StdAfx.h"
#include "ArchTester.h"
#include "CodeTester.h"

#if CONFIG_CMDLINE

Bool Tester_Shell_Arg1()
{
	Shell_Init();
	Shell_Input("Arg1(1)");
	return True;
}
Bool Tester_Shell_Arg2()
{
	Shell_Init();
	Shell_Input("Arg2(1,0xFFFF)");
	return True;
}
Bool Tester_Shell_Arg3()
{
	Shell_Init();
	Shell_Input("Arg3(1,0xFFFF,0xFFFF)");
	return True;
}
Bool Tester_Shell_Arg4()
{
	Shell_Init();
	Shell_Input("Arg4(0xFFF0,0xFFF1,0xFFF2,0xFFFFFFF3)");
	return True;
}
Bool Tester_Shell_Arg5()
{
	Shell_Init();
	Shell_Input("Arg5(\"1\",0xFFFFFFFF,\"192.168.0.1\",0xFFFFFFFF, \"test\")");
	return True;
}

Bool Tester_Shell_GetArgCount()
{
	Shell_GetArgCount();
	return True;
}

Bool Tester_Shell_Help()
{
	Shell_Init();
	Shell_Input("?()");
	return True;
}
#endif

TreeNodeData g_ArchUnitTestItem[] = 
{	
	{1, L"Shell模块单元测试"}
#if CONFIG_CMDLINE
		,{2, L"Shell_Help()",	(TesterFun)Tester_Shell_Help}
		,{2, L"Shell_Arg1()",	(TesterFun)Tester_Shell_Arg1}
		,{2, L"Shell_Arg2()",	(TesterFun)Tester_Shell_Arg2}
		,{2, L"Shell_Arg3()",	(TesterFun)Tester_Shell_Arg3}
		,{2, L"Shell_Arg4()",	(TesterFun)Tester_Shell_Arg4}
		,{2, L"Shell_Arg5()",	(TesterFun)Tester_Shell_Arg5}

		,{2, L"Shell_GetArgCount()",	(TesterFun)Tester_Shell_GetArgCount}
#endif
};

TreeNodeData g_ArchIntegTestItem[] = 
{
	{1, L"Arch_Main()",	(TesterFun)0}
};
CArchTester g_ArchTester;


CArchTester::CArchTester(void)
{
	//增加单元测试项///////////////////////////////////////////
	g_TestCaseTreeData.AddUnitTreeNodeData(g_ArchUnitTestItem, sizeof(g_ArchUnitTestItem)/sizeof(TreeNodeData));
	//g_TestCaseTreeData.AddIntegTreeNodeData(g_ArchIntegTestItem, sizeof(g_ArchIntegTestItem)/sizeof(TreeNodeData));

	//DWORD m_RxThreadId;
	//m_hRxThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Tester_SendToArch,this,NULL,&m_RxThreadId);
}


CArchTester::~CArchTester(void)
{
	//if(m_hRxThread)
	{
		//TerminateThread(m_hRxThread, 0);
		//m_hRxThread = 0;
	}
}
