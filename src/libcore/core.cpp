
#include <stdlib.h>
void *operator new(size_t size)
{
    return malloc(size);
}

void *operator new[](size_t size)
{
    return malloc(size);
}

void operator delete(void *p) noexcept
{
    free(p);
}

void operator delete[](void *p) noexcept
{
    free(p);
}

void operator delete(void *p, size_t) noexcept
{
    free(p);
}

void operator delete[](void *p, size_t) noexcept
{
    free(p);
}
