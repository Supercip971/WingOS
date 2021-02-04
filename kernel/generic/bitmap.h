#ifndef BITMAP_H
#define BITMAP_H
#include <arch.h>
#include <lock.h>
#include <stddef.h>
#include <stdint.h>
#include <utility.h>

/*
 * the bitmap is a basic type
 * WARNING: the bitmap entry 0 is null and must not be used when returned by find_free() alloc() set_free() ...
 */
class bitmap
{
    size_t bitmap_size;
    uint8_t *buffer;
    size_t last_free = 0;

public:
    constexpr size_t get_size() const { return bitmap_size; };
    constexpr void set(size_t idx, bool value);
    constexpr bool get(size_t idx) const;
    bitmap(uint8_t *data, size_t size);
    bitmap() : bitmap_size(0)
    {
        buffer = nullptr;
    }

    size_t find_free(size_t length);

    size_t alloc(size_t length); // same as find free but set the bits

    size_t set_free(size_t idx, size_t length);
    size_t set_used(size_t idx, size_t length);

    void reset_last_free() { last_free = 0; };
};

#endif // BITMAP_H
