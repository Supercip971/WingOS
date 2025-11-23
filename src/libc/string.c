#include "string.h"

void *memcpy(void * __restrict dest, const void * __restrict src, size_t n)
{
    char *d = (char *)dest;
    const char *s = (const char *)src;
    while (n--)
    {
        *d++ = *s++;
    }
    return dest;
}
void *memset(void *s, int c, size_t n)
{
    char *p = (char *)s;
    while (n--)
    {
        *p++ = c;
    }
    return s;
}

void * memmove(void *dest, const void *src, size_t n)
{
    char *d = (char *)dest;
    const char *s = (const char *)src;
    if (d < s)
    {
        while (n--)
        {
            *d++ = *s++;
        }
    }
    else
    {
        d += n;
        s += n;
        while (n--)
        {
            *(--d) = *(--s);
        }
    }
    return dest;
}