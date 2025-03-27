#include "stdlib.h"
#include "liballoc/liballoc.h"


void *malloc(size_t size)
{
    return kmalloc(size);
}
void free(void *ptr)
{
    kfree(ptr);
}
void *realloc(void *ptr, size_t size)
{
    return krealloc(ptr, size);
}