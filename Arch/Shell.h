#if CONFIG_CMDLINE

#ifndef  _SHELL_H_
#define  _SHELL_H_

#ifdef __cplusplus
extern "C"{
#endif

void Shell_Init(void);
void Shell_Input(const char* pStr);
void Shell_GetArgCount();

#ifdef __cplusplus
}
#endif

#endif 

#endif
