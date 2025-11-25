#ifndef STRING_H
#define STRING_H
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


void *memcpy(void * __restrict dest, const void * __restrict src, size_t n);
void *memset(void *s, int c, size_t n) ;

char* strchr(const char* s, int c);

size_t strlen(const char* s);

char *strerror(int errnum);

#ifdef __cplusplus
}
#endif


#endif