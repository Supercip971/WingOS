#ifndef STRING_H
#define STRING_H
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);

#ifdef __cplusplus
}
#endif


#endif