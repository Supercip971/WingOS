#pragma once 


#include <stddef.h>

// malloc 
#ifdef __cplusplus
extern "C" {
#endif

void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);

#ifdef __cplusplus
}
#endif



