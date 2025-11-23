#ifndef STRING_H
#define STRING_H
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


void *memcpy(void * __restrict dest, const void * __restrict src, size_t n);
void *memset(void *s, int c, size_t n);

#ifdef __cplusplus
}
#endif


#endif