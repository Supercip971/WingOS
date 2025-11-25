#include "string.h"
#include <signal.h>
#ifndef __clang__ 

__attribute__((optimize("no-tree-loop-distribute-patterns")))
#endif  



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



#ifndef __clang__ 

__attribute__((optimize("no-tree-loop-distribute-patterns")))
#endif  

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


size_t strlen(const char* s)
{
    size_t len = 0;
    while(s[len] != '\0') 
    {
        len++;
    }
    return len;
}

char* strchr(const char* s, int c)
{
    while (*s)
    {
        if (*s == (char)c)
        {
            return (char *)s;
        }
        s++;
    }
    if (c == '\0')
    {
        return (char *)s;
    }
    return NULL;
}
char * strerror(int errnum)
{
    (void)errnum;
    return "Unknown error";
}