#include <bits/stdc++.h>
#include <stddef.h>
#include <stdlib.h>
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
