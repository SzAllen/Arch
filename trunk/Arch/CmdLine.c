#if CONFIG_CMDLINE

#include "CmdLine.h"
#include "Str.h"

ArgType CmdLine_GetArgType(const char* argStr)
{
	int nLen = strlen(argStr);

	if(Null == argStr) return AT_NONE;

	if(*argStr == '\"')
	{
		if('\"' == argStr[nLen-1])
		{
			return AT_STR;
		}
		else
		{
			return AT_ERROR;
		}
	}
	else if(argStr[0] == '0' && (argStr[1] == 'x' || argStr[1] == 'X'))
	{
		return AT_HEX;
	}
	else
	{
		return AT_DEC;
	}
}

Bool CmdLine_Parse(char* cmdLineStr, char** pFnName, char* pArgs[], int* argCount)
{
	int maxArgCount = *argCount;
	char *token;
	char fnNameseps[]   = " (\n";
	char argSeps[]   = ", )\n";

	//Find function name
	token = strtok(cmdLineStr, fnNameseps);
	if(Null == token) return False;
	*pFnName = token;

	*argCount= 0;
	token = strtok( NULL, argSeps);
	while( token != NULL )
	{
		pArgs[(*argCount)++] = token;
		if((*argCount) > maxArgCount)
		{
			Printf("Error: Arg count is too many\n");
			return False;
		}
		token = strtok( NULL, argSeps);
	}

	return True;
}

Bool CmdLine_ArgConvert(const char* pFnName, char* pArgs[], int argCount, uint32 arg[])
{
	int i = 0;
	ArgType at = AT_NONE;
	char* pChar = Null;

	for(i = 0; i < argCount; i++)
	{
		at = CmdLine_GetArgType(pArgs[i]);
		if(AT_DEC == at)
		{
			arg[i] = atoi(pArgs[i]);
		}
		else if(AT_HEX == at)
		{
			arg[i] = htoi(pArgs[i]);
		}
		else if(AT_STR == at)
		{
			pChar = pArgs[i];
			pChar[strlen(pChar) - 1] = 0;
			pChar++;
			arg[i] = (uint32)pChar;
		}
		else if(AT_DEC == at)
		{
			arg[i] = 0;
		}
		else
		{
			SHELL(("\tArg[%d](%s) error. \n", i+1, pArgs[i]));
			return False;
		}
	}
	return True;
}

void CmdLine_Exe(CmdLine* pCmdLine, const char* pFnName, uint32 arg[], int argCount)
{
	Bool isFind = False;
	int i = 0;
	const FnDef* pFnEntry = pCmdLine->m_FnArray;
	
	#define FUN(n, funType, args) if(n == pFnEntry->m_ArgCount)	\
		{	\
			((funType)pFnEntry->pFn) args;	\
			return;	\
		}

	for(i = 0; i < pCmdLine->m_FnCount; i++, pFnEntry++)
	{
		if(strcmp(pFnName, pFnEntry->m_FnName) == 0)
		{
			isFind = True;
			break;
		}
	}

	if(!isFind)
	{
		Printf("No function: %s\n", pFnName);
		return;
	}

	FUN(0, FnArg0, ());
	FUN(1, FnArg01, (arg[0]));
	FUN(2, FnArg02, (arg[0], arg[1]));
	FUN(3, FnArg03, (arg[0], arg[1], arg[2]));
	FUN(4, FnArg04, (arg[0], arg[1], arg[2], arg[3]));
	FUN(5, FnArg05, (arg[0], arg[1], arg[2], arg[3], arg[4]));
}

int CmdLine_GetArgCount(const char* str)
{
	Bool bFlag = False;
	int i = 0;
	int nArgCount = 0;
	
	str	= strchr(str, '(');

	if(Null == str)
	{
		return 0;
	}
	
	while(*(++str) != '\0')
	{
		if(')' == *str)
		{
			break;
		}
		else if(!bFlag)
		{
			if(' ' != *str)
			{
				bFlag = True;
				nArgCount++;

				if(',' == *str)
				{
					nArgCount++;
				}
			}
		}
		else if(',' == *str)
		{
			nArgCount++;
		}
	}
	
	return *str == ')' ? nArgCount : -1;
}

void CmdLine_Reset(CmdLine* pCmdLine)
{
	memset(pCmdLine->m_CmdLineStr, 0, sizeof(pCmdLine->m_CmdLineStr));
	pCmdLine->m_CmdLineStrLen = 0;
}

int CmdLine_AddStr(CmdLine* pCmdLine, const char* str, int len)
{
	int i = 0;

	if(len >= MAX_CMDLINE_LEN) return 0;

	for(i = 0; i < len; i++, str++)
	{
		if(pCmdLine->m_CmdLineStrLen >= MAX_CMDLINE_LEN)
		{
			CmdLine_Reset(pCmdLine);
		}
		
		if(pCmdLine->m_isEcho)
		{
			if(0 == pCmdLine->m_CmdLineStrLen)
			{
				Printf("->");
			}
			Printf("%c", *str);
		}
		
		pCmdLine->m_CmdLineStr[pCmdLine->m_CmdLineStrLen++] = *str;
		if(KEY_ENTER == *str || ')' == *str)
		{
			char* pFnName = Null;
			char* argStr[MAX_ARG_COUNT] = {0};
			int argCount = MAX_ARG_COUNT;
			
			Printf("\n");
			if(CmdLine_Parse(pCmdLine->m_CmdLineStr, &pFnName, argStr, &argCount))
			{
				uint32 arg[MAX_ARG_COUNT] = {0};
				
				if(CmdLine_ArgConvert(pFnName, argStr, argCount, arg))
				{
					CmdLine_Exe(pCmdLine, pFnName, arg, argCount);
				}
			}
			CmdLine_Reset(pCmdLine);
	   	}
	}

	return len;
}

void CmdLine_Init(CmdLine* pCmdLine, FnDef* pCmdTable, uint8 cmdTableCount, Bool isEcho)
{
	int i = 0;
	FnDef* pFnEntry = pCmdTable;
	
	memset(pCmdLine, 0, sizeof(CmdLine));
	
	pCmdLine->m_isEcho = isEcho;

	pCmdLine->m_FnArray = pCmdTable;
	pCmdLine->m_FnCount = cmdTableCount;

	for(i = 0; i < cmdTableCount; i++, pFnEntry++)
	{
		pFnEntry->m_ArgCount = CmdLine_GetArgCount(pFnEntry->m_Title);
		if(-1 == pFnEntry->m_ArgCount)
		{
			Printf("%s error, get arg count error.\n", pFnEntry->m_Title, pFnEntry->m_ArgCount);
		}
		
	}
}

#endif

