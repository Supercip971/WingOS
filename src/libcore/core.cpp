#include <stdlib.h>

__attribute__((weak)) void *operator new(size_t size)
{
    return malloc(size);
}

__attribute__((weak)) void *operator new[](size_t size)
{
    return malloc(size);
}

__attribute__((weak)) void operator delete(void *p) noexcept
{
    free(p);
}

__attribute__((weak)) void operator delete[](void *p) noexcept
{
    free(p);
}

__attribute__((weak)) void operator delete(void *p, size_t) noexcept
{
    free(p);
}

__attribute__((weak)) void operator delete[](void *p, size_t) noexcept
{
    free(p);
}
