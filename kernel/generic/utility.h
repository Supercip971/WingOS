#pragma once

#include <logging.h>
#include <stddef.h>
#include <stdint.h>

void kitoaT(char *buf, int base, size_t d);
inline void memzero(void const *s, const uint64_t n)
{
    for (uint64_t i = 0; i < n; i++)
    {
        ((uint8_t *)s)[i] = 0;
    }
}
constexpr inline uint8_t get_bit(uint8_t *bitmap, size_t bit_id)
{
    size_t bitmap_idx = bit_id / 8;
    size_t bit_idx = bit_id % 8;
    return bitmap[bitmap_idx] & (1 << bit_idx);
}
constexpr inline void set_bit(uint8_t *bitmap, size_t bit_id, uint8_t value)
{
    size_t bitmap_idx = bit_id / 8;
    size_t bit_idx = bit_id % 8;
    if (value)
    {

        bitmap[bitmap_idx] |= (1 << bit_idx);
    }
    else
    {
        bitmap[bitmap_idx] &= ~(1 << bit_idx);
    }
}
