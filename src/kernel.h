#pragma once
#include <com.h>
#include <int_value.h>
#include <stivale.h>
void _start(struct stivale_struct *bootloader_data);

__attribute__((optimize("O0"))) inline void memzero(void *s, uint64_t n)
{
    for (uint64_t i = 0; i < n; i++)
    {
        ((uint8_t *)s)[i] = 0;
    }
}

__attribute__((optimize("O0"))) inline void *memset(void *data, uint8_t value,
                                                    uint64_t lenght)
{
    uint8_t *d = (uint8_t *)data;
    for (uint64_t i = 0; i < lenght; i++)
    {
        d[i] = value;
    }
    return data;
}
