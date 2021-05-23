#include <bits/stdc++.h>
#include <stddef.h>
#include <stdlib.h>

// this is sketchy
void *operator new(size_t size, std::align_val_t align)
{
    return malloc(size);
}
void *operator new[](size_t size, std::align_val_t align)
{

    return malloc(size);
}
void *__attribute__((weak)) operator new(size_t size)
{
    return malloc(size);
}

void *__attribute__((weak)) operator new[](size_t size)
{
    return malloc(size);
}

void __attribute__((weak)) operator delete(void *p)
{
    free(p);
}

void __attribute__((weak)) operator delete[](void *p)
{
    free(p);
}

void operator delete(void *p, size_t size)
{

    free(p);
}
void operator delete[](void *p, size_t size)
{

    free(p);
}
