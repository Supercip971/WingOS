#pragma once

#include <logging.h>
#include <stddef.h>
#include <stdint.h>
int isdigit(int c);
char *strtok(char *s, const char *delm);

int64_t strtoint(const char *nptr);

int strncmp(const char *s1, const char *s2, size_t n);
int strcmp(const char *s1, const char *s2);
size_t strlen(const char *s);

void kitoa(char *buf, int base, int d);
void kitoa64(char *buf, int base, int64_t d);
void kitoaT(char *buf, int base, size_t d);

inline void memzero(void const *s, const uint64_t n)
{
    for (uint64_t i = 0; i < n; i++)
    {
        ((uint8_t *)s)[i] = 0;
    }
}

inline void *memset(void *data, const uint8_t value,
                    const uint64_t lenght)
{
    uint8_t *d = reinterpret_cast<uint8_t *>(data);
    for (uint64_t i = 0; i < lenght; i++)
    {
        d[i] = value;
    }
    return data;
}

inline void *memcpy(void *dst, const void *src, uint64_t len)
{
    char *dst8 = reinterpret_cast<char *>(dst);
    const char *src8 = reinterpret_cast<const char *>(src);
    for (uint64_t i = 0; i < len; i++)
    {
        dst8[i] = src8[i];
    }
    return dst;
}
