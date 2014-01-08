#if CONFIG_CMDLINE

#ifndef  _CMDLINE_H_
#define  _CMDLINE_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "ArchDef.h"

#define	KEY_ENTER	0x0d
#define	KEY_BACK	0x08
#define KEY_BEEP	0x07
#define KEY_UP		0x41
#define KEY_DOWN	0x42
#define KEY_RIGHT	0x43
#define KEY_LEFT	0x44
#define KEY_HOME	0x48
#define KEY_END		0x4b
#define	KEY_ESC		0x1b
#define	KEY_EOF		0x1a
#define KEY_CANCEL  0x03
#define KEY_EOT     0x04

typedef void (*FnArg0)();
typedef void (*FnArg01)(uint32);
typedef void (*FnArg02)(uint32,uint32);
typedef void (*FnArg03)(uint32,uint32,uint32);
typedef void (*FnArg04)(uint32,uint32,uint32,uint32);
typedef void (*FnArg05)(uint32,uint32,uint32,uint32,uint32);
typedef void (*FnArg06)(uint32,uint32,uint32,uint32,uint32,uint32);
typedef void (*FnArg07)(uint32,uint32,uint32,uint32,uint32,uint32,uint32);
typedef void (*FnArg08)(uint32,uint32,uint32,uint32,uint32,uint32,uint32,uint32);
typedef void (*FnArg09)(uint32,uint32,uint32,uint32,uint32,uint32,uint32,uint32,uint32);

typedef enum _ArgType
{
	 AT_NONE = 0
	,AT_DEC 
	,AT_HEX
	,AT_STR
	,AT_ERROR
}ArgType;


#define MAX_ARG_COUNT  5
typedef struct _FnDef
{
	const char* 	m_FnName;
	void* 			pFn;
	const char* 	m_Title;
	uint8 			m_ArgCount;
}FnDef;

#define MAX_CMDLINE_LEN 64
typedef struct _CmdLine
{
	uint8 m_isEcho:1;
	uint8 m_Reserved:7;
	
	uint8 m_CmdLineStrLen;
	char  m_CmdLineStr[MAX_CMDLINE_LEN + 1];
	
	FnDef* m_FnArray;	//Point to a FnDef array
	uint8 m_FnCount;	//Nunber of FnDef array
}CmdLine;

void CmdLine_Init(CmdLine* pCmdLine, FnDef* pCmdTable, uint8 cmdTableCount, Bool isEcho);
int CmdLine_AddStr(CmdLine* pCmdLine, const char* str, int len);
int CmdLine_GetArgCount(const char* str);

#ifdef __cplusplus
}
#endif

#endif 

#endif
