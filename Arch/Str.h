#ifndef  _STR_H_
#define  _STR_H_

#ifdef __cplusplus
extern "C"{
#endif

int htoi(const char *s);

#ifdef CONFIG_STRING
const char* strchr(const char* s, char c);
#endif

#ifdef __cplusplus
}
#endif

#endif 


