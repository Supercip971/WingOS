#include <kern/kernel_util.h>
#include <kern/mem_util.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
size_t strlen(const char *s)
{
    size_t string_length = 0;
    while (s[string_length] != 0)
    {
        string_length++;
    }
    return string_length;
}

size_t strnlen(const char *s, size_t maxlen)
{
    size_t string_length = 0;
    while (s[string_length] != 0 && string_length < maxlen)
    {
        string_length++;
    }
    return string_length;
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 == *s2 && (*s1))
    {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    while (n && *s1 && (*s1 == *s2))
    {
        ++s1;
        ++s2;
        --n;
    }
    if (n == 0)
    {
        return 0;
    }
    else
    {
        return (*(unsigned char *)s1 - *(unsigned char *)s2);
    }
}

char *strcpy(char *dest, const char *src)
{
    size_t i = 0;
    while (src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }
    return dest;
}

char *strncpy(char *dest, const char *src, size_t n)
{
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++)
    {
        dest[i] = src[i];
    }
    for (; i < n; i++)
    {
        dest[i] = '\0';
    }

    return dest;
}

char *strtok(char *s, const char delimeter)
{
    static char *buffer = s;
    if (buffer == NULL)
    {
        return NULL;
    }
    /* This dynamically alocated array should probably be changed
    to a vector once their implementation in this OS is more mature.
    */
    char *token = new char[strlen(buffer) + 1];
    int i = 0;
    for (; buffer[i] != '\0'; i++)
    {
        if (buffer[i] != delimeter)
        {
            token[i] = buffer[i];
        }
        else
        {
            token[i] = '\0';
            buffer = buffer + i + 1;
            return token;
        }
    }
    token[i] = '\0';
    buffer = NULL;
    return token;
}

#ifndef WOS_OPTIMIZATION
void *memcpy(void *dest, const void *src, size_t length)
{
    char *cdest = (char *)dest;
    const char *csrc = (const char *)src;
    for (size_t i = 0; i < length; i++)
    {
        cdest[i] = csrc[i];
    }
    return cdest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const char *ss1 = (const char *)s1;
    const char *ss2 = (const char *)s2;
    for (size_t i = 0; i < n; i++)
    {
        if (ss1[i] != ss2[i])
        {
            return false;
        }
    }
    return true;
}
void *memset(void *data, int value, size_t length)
{
    uint8_t *d = (uint8_t *)data;
    for (size_t i = 0; i < length; i++)
    {
        d[i] = value;
    }
    return data;
}

void *memmove(void *dest, const void *src, size_t n)
{
    char *new_dst = (char *)dest;
    const char *new_src = (const char *)src;
    char *temporary_data = (char *)malloc(n);

    for (size_t i = 0; i < n; i++)
    {
        temporary_data[i] = new_src[i];
    }
    for (size_t i = 0; i < n; i++)
    {
        new_dst[i] = temporary_data[i];
    }

    free(temporary_data);
    return dest;
}

void *memchr(const void *s, int c, size_t n)
{
    const unsigned char *copy = (const unsigned char *)s;
    for (size_t i = 0; i < n; i++)
    {
        if (copy[i] == c)
        {
            return (void *)(copy + i);
        }
    }

    return nullptr;
}
// for later optimization
#else

#endif
