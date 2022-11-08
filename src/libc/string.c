#include <libc/string.h>
void *memcpy(void *dest, const void *src, size_t n)
{
    char *d = (char*)dest;
    const char *s = (const char*)src;
    while (n--)
        *d++ = *s++;
    return dest;
}
void *memset(void *s, int c, size_t n)
{
    char *p = (char*)s;
    while (n--)
        *p++ = c;
    return s;
}