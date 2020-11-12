#pragma once
#include <com.h>
#include <int_value.h>
#include <stivale_struct.h>
void _start(struct stivale_struct *bootloader_data);
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

inline void *memcpy(void *dst, const void *src, int64_t len)
{
    char *dst8 = reinterpret_cast<char *>(dst);
    const char *src8 = reinterpret_cast<const char *>(src);
    for (uint64_t i = 0; i < len; i++)
    {
        dst8[i] = src8[i];
    }
    return dst;
}
