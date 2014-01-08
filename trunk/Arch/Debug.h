
#ifndef  _XDEBUG_H_
#define  _XDEBUG_H_

#ifdef __cplusplus
extern "C"{
#endif	//#ifdef __cplusplus

#if XDEBUG
	#include "typedef.h"
	#include "printf.h"

	extern uint32 g_dwDebugLevel;
	void Debug_Init(void);
	void ShowWait(int printDotPerCount, int dotCountPerRow);
	void Debug_EnbaleLevel(uint32 level, Bool isEnable);

	enum	//Debug Level define
	{
		DL_MAIN 		= BIT_0
		, DL_ERROR		= BIT_1
		, DL_WARNING	= BIT_2
		, DL_ASSERT		= BIT_3
		, DL_LISTEX		= BIT_4
		, DL_TIMER		= BIT_5
		, DL_CHNL		= BIT_6
		
		, DL_SHOW_TICKS	= BIT_31
		, DL_DEBUG		= 0xFFFFFFFF
	};
		
	#define SHOW_IP(string,asUint32) 	Printf("%s = [%03d.%03d.%03d.%03d]\n"	\
									, string	\
									, MEM_BYTE(asUint32,0)	\
									, MEM_BYTE(asUint32,1)	\
									, MEM_BYTE(asUint32,2)	\
									, MEM_BYTE(asUint32,3))

	
	#define Assert(expr) if(!(expr))	\
			{                                   \
				Printf( "Assertion Failed! %s,%s,%s,line=%d\n", #expr,__FILE__,__FUNCTION__,_LINE_); 			\
				while(1){;}	\
			}
	
	#define AssertTrue(expr) if(!(expr))	\
			{                                   \
				Printf( "*Test Failed*: %s, (%s)\n",__FUNCTION__, #expr);	\
				while(1){;}	\
			}	\
			else	\
			{	\
				Printf( "Test Passed: %s, (%s)\n",__FUNCTION__, #expr);	\
			}
			
	#define AssertFalse(expr) if((expr)) {                                   \
			Printf( "Assertion Failed! %s,%s,%s,line=%d\n", #expr,__FILE__,__FUNCTION__,_LINE_); 			\
			while(1){;}	\
			}
	//#define Assert(x) if(!(x)){while(1){;}}
	#define Trace Printf
	#define PF(level, str) \
		{	\
			if((level) &  g_dwDebugLevel)	\
			{	\
				Printf str;	\
			}	\
		}	

	void Debug_DumpByteEx(uint32 level, const uint8* pData, uint16 len, uint8 cols, const char* str, int line) ;
	void Debug_DumpByte(const uint8* pData, uint16 len, uint8 cols);
	void Debug_DumpDword(const uint32* pData, uint16 len, uint8 cols);

	#define DUMP_BYTE(address,len)  Debug_DumpByte(address, len, 32)
	#define DUMP_DWORD(address,len) Debug_DumpDword(address, len, 8)

	#define DUMP_BYTE_LEVEL_EX(level, address, len, cols) {\
		if(((uint32)(level)) & g_dwDebugLevel) \
		{	\
    		Debug_DumpByte(address, len, cols);	\
		}	\
	}

	#define DUMP_BYTE_LEVEL(level, address, len){\
		if(((uint32)(level)) & g_dwDebugLevel) \
		{	\
    		DUMP_BYTE(address,len);	\
		}	\
	}
	#define DUMP_DWORD_LEVEL(level, address,len){\
		if(((uint32)(level)) & g_dwDebugLevel) \
		{	\
    		DUMP_DWORD(address,len);	\
		}	\
	}

	#define SHELL(x) Printf x
	
	#define Warning(x) PF(DL_WARNING, x);
	#define Error(x) PF(DL_ERROR, x);

	#define PF_FUN(level) PF(level, ("%s(),line=%d\n",_FUNC_, _LINE_));
	#define PF_LINE(level) PF(level, ("PF Line. %s,%s(),line=%d\n", __FILE__,__FUNCTION__,_LINE_))
	#define PF_VAR(level, V1) 		PF(level, ("%s(),line=%d,%s=[0x%x](%d)\n",_FUNC_, _LINE_, #V1, V1, V1))
	#define PF_FAILED() 		PF(DL_MAIN|DL_WARNING, ("%s() FAILED,line=%d.\n",_FUNC_, _LINE_))
	#define PF_FAILED_STR(str) 		PF(DL_MAIN|DL_WARNING, ("%s() FAILED,line=%d. %s\n",_FUNC_, _LINE_, str))
	#define PF_FAILED_V1(V1) 		PF(DL_MAIN|DL_WARNING, ("%s() FAILED,line=%d; %s=0x%x(%d).\n",_FUNC_, _LINE_, #V1, V1, V1))
	
//	#define PF_VAR(V1) 			Trace("%s(),line=%d,%s=[0x%02x](%d)\n", __FUNCTION__, _LINE_, #V1, V1, V1)
	#define PF_VAR2(level,V1,V2) 		PF((level), ("%s(),line=%d,%s=[0x%x](%d),%s=[0x%x](%d)\n", __FUNCTION__, _LINE_, #V1, V1, V1, #V2, V2, V2))
	#define PF_VAR3(level,V1,V2,V3) 	PF((level), ("%s(),line=%d,%s=[0x%x](%d),%s=[0x%x](%d),%s=[0x%x](%d)\n", __FUNCTION__, _LINE_, #V1, V1, V1, #V2, V2, V2, #V3, V3, V3))
//	#define PF_VAR4(V1,V2,V3,V4) Trace("%s(),line=%d,%s=[0x%02x](%d),%s=[0x%02x](%d),%s=[0x%02x](%d),%s=[0x%02x](%d)\n", __FUNCTION__, _LINE_, #V1, V1, V1, #V2, V2, V2, #V3, V3, V3, #V4, V4, V4)
	#define WAIT(maxMS, expr) {int ms = 0; while(!(expr) && ms++ < (maxMS)) {SLEEP(1);}}
	//#define PF_IP(level, asUint32)	PF(level, ("%s(),line=%d\n",_FUNC_, _LINE_));

#else	//#if XDEBUG
	#define Debug_Init(...)
	#define ShowWait(...)
	#define Debug_EnbaleLevel(...)
	#define Printf(...)
	#define Assert(...)
	#define AssertTrue(...)
	#define AssertFalse(...)
	#define Trace(...)
	#define Warning(...)
	#define Error(...)
	
	#define PF(...)
	#define PF_FUN(...)
	#define PF_LINE(...)
	#define PF_FAILED() (void)0
	#define PF_FAILED_V1(...)
	#define PF_VAR(...)
	#define PF_VAR2(...)
	#define PF_FAILED_STR(...)

	#define Debug_DumpByteEx(...)
	#define Debug_DumpByte(...)
	#define Debug_DumpDword(...)

	#define DUMP_BYTE(...)
	#define DUMP_DWORD(...)

	#define DUMP_BYTE_LEVEL(...)
	#define DUMP_DWORD_LEVEL(...)
	#define DUMP_BYTE_LEVEL_EX(...)
	#define Debug_Init(...)

	#define SHELL(...)
	#define PF_VAR(...)
	#define WAIT(...)
#endif	//#if XDEBUG

#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus

#endif //#ifndef  _DEBUG_H_


