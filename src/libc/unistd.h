#pragma once 

#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
char *getcwd(char* buf, size_t size);

int chdir(const char* path);

void _exit(int status);

#ifdef __cplusplus
}
#endif