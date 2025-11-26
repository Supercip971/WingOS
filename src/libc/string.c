#include "string.h"
#include <signal.h>
#ifndef __clang__

__attribute__((optimize("no-tree-loop-distribute-patterns")))
#endif

void *memcpy(void *__restrict dest, const void *__restrict src, size_t n)
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

void *memmove(void *dest, const void *src, size_t n)
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

size_t strlen(const char *s)
{
    size_t len = 0;
    while (s[len] != '\0')
    {
        len++;
    }
    return len;
}

char *strchr(const char *s, int c)
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
char *strerror(int errnum)
{
    (void)errnum;
    return "Unknown error";
}

static char *save_ptr = NULL;
char *strtok(char *__restrict str, const char *__restrict delim)
{
    return strtok_r(str, delim, &save_ptr);
}

char *strtok_r(char *__restrict str, const char *__restrict delim,
               char **__restrict saveptr)
{
    char* str_b = str ? str : *saveptr;

    if(!str_b)
    {
        return NULL;
    }

    // Skip leading delimiters
    while (*str_b)
    {
        const char *d = delim;
        int is_delim = 0;
        while (*d)
        {
            if (*str_b == *d)
            {
                is_delim = 1;
                break;
            }
            d++;
        }
        if (!is_delim)
            break;
        str_b++;
    }

    if (*str_b == '\0')
    {
        *saveptr = NULL;
        return NULL;
    }

    // Find end of token
    char* bprk = strpbrk(str_b, delim);
    if(bprk)
    {
        *bprk = '\0';
        *saveptr = bprk + 1;
    }
    else
    {
        *saveptr = NULL;
    }

    return str_b;
}

char *strpbrk(const char *s, const char *accept)
{
    while(*s)
    {
        const char *a = accept;
        
        while(*a)
        {
            if(*s == *a)
            {
                return (char *)s;
            }
            a++;
        }
        s++;
    }

    return NULL;
}