#pragma once 


#include <stddef.h>

// malloc 
#ifdef __cplusplus
extern "C" {
#endif

void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);

[[noreturn]]
void exit(int code);

long strtol(const char *nptr, char **endptr, int base);
long long strtoll(const char *nptr, char **endptr, int base);
int atoi(const char* str);
#ifdef __cplusplus
}
#endif



