#if CONFIG_CMDLINE

#include "CmdLine.h"
#include "Shell.h"

void Shell_Arg1(uint8 arg1)
{
	int i = 0;
	SHELL(("%s: arg1=%d\n", _FUNC_, arg1));
}
void Shell_Arg2(char arg1, uint16 arg2)
{
	SHELL(("%s: arg1=%d, arg2=%d\n", _FUNC_, arg1, arg2));
}
void Shell_Arg3(uint8 arg1, uint16 arg2, short arg3)
{
	SHELL(("%s: arg1=%d, arg2=%d, arg3=%d\n", _FUNC_, arg1, arg2, arg3));
}
void Shell_Arg4(uint8 arg1, uint16 arg2, short arg3, uint32 arg4)
{
	SHELL(("%s: arg1=%d, arg2=%d, arg3=%d, arg4=%d\n", _FUNC_, arg1, arg2, arg3, arg4));
}
void Shell_Arg5(char* arg1, uint16 arg2, char* arg3, uint32 arg4, char* arg5)
{
	SHELL(("%s: arg1=%s, arg2=%d, arg3=%s, arg4=0x%x, arg5=%s\n", _FUNC_, arg1, arg2, arg3, arg4, arg5));
}

void Shell_GetArgCount()
{
	AssertTrue(CmdLine_GetArgCount("Shell_Help()") == 0);
	AssertTrue(CmdLine_GetArgCount("Shell_Help(") == -1);
	AssertTrue(CmdLine_GetArgCount("Shell_Help(  )") == 0);
	AssertTrue(CmdLine_GetArgCount("Shell_Help(,,,)") == 4);
	AssertTrue(CmdLine_GetArgCount("Shell_Help(uint8 arg1)") == 1);
	AssertTrue(CmdLine_GetArgCount("Shell_Help(char arg1, uint16 arg2)") == 2);
	AssertTrue(CmdLine_GetArgCount("Shell_Help(uint8 arg1, uint16 arg2, short arg3)") == 3);
	AssertTrue(CmdLine_GetArgCount("Shell_Help(uint8 arg1, uint16 arg2, short arg3, uint32 arg4)") == 4);
	AssertTrue(CmdLine_GetArgCount("Shell_Help(char* arg1, uint16 arg2, char* arg3, uint32 arg4, char* arg5)") == 5);
}

///////////////////////////////////////////////////////

static CmdLine g_CmdLine;
void Shell_Help()
{
	int i = 0;
	const FnDef* pFnDef = g_CmdLine.m_FnArray;
	
	for(i = 0; i < g_CmdLine.m_FnCount; i++, pFnDef++)
	{		
		SHELL(("\t %s\n", pFnDef->m_Title));
	}
}


void Shell_Init()
{
	static FnDef g_CmdTable[] = 
	{
		  {"?"	 , Shell_Help, "Shell_Help()"}
		 ,{"Arg1", Shell_Arg1, "Shell_Arg1(uint8 arg1)"}
		 ,{"Arg2", Shell_Arg2, "Shell_Arg2(char arg1, uint16 arg2)"}
		 ,{"Arg3", Shell_Arg3, "Shell_Arg3(uint8 arg1, uint16 arg2, short arg3)"}
		 ,{"Arg4", Shell_Arg4, "Shell_Arg4(uint8 arg1, uint16 arg2, short arg3, uint32 arg4)"}
		 ,{"Arg5", Shell_Arg5, "Shell_Arg5(char* arg1, uint16 arg2, char* arg3, uint32 arg4, char* arg5)"}
		 
		 ,{"GetArgCount", Shell_GetArgCount, "Shell_GetArgCount()"}
	};
	
	CmdLine_Init(&g_CmdLine, g_CmdTable, sizeof(g_CmdTable) / sizeof(FnDef), True);
}

void Shell_Input(const char* pStr)
{
	int len = strlen(pStr);

	if(len)
	{
		CmdLine_AddStr(&g_CmdLine, pStr, len);
	}
}

#endif
