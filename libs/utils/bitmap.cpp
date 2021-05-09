#include "bitmap.h"
#include <stdio.h>
#include <string.h>
bitmap::bitmap(uint8_t *data, size_t size) : bitmap_size(size)
{
    memset(data, 0xff, size / 8);
    buffer = data;
    last_free = 0;
}

void bitmap::set(size_t idx, bool value)
{
    if (idx > bitmap_size)
    {
        printf("[error] bitmap: trying to set out of bound of the bitmap: %i> %i\n", idx, bitmap_size);
        return;
    }
    size_t bit = idx % 8;
    size_t byte = idx / 8;
    if (value)
    {
        buffer[byte] |= (1 << (bit));
    }
    else
    {
        buffer[byte] &= ~(1 << (bit));
    }
}

bool bitmap::get(size_t idx) const
{
    if (idx > bitmap_size)
    {
        printf("[error] bitmap: trying to get out of bound of the bitmap: %i> %i\n", idx, bitmap_size);

        return false;
    }
    size_t bit = idx % 8;
    size_t byte = idx / 8;
    return (buffer[byte] & (1 << (bit)));
}

size_t bitmap::find_free(size_t length)
{
    size_t current_founded_length = 0;
    size_t current_founded_idx = 0;

    for (size_t i = last_free; i < bitmap_size; i++)
    {
        if (i == 0)
        {
            continue;
        }

        if (!get(i))
        {
            if (current_founded_length == 0)
            {
                current_founded_idx = i;
            }
            current_founded_length++;
        }
        else
        {
            current_founded_length = 0;
            current_founded_idx = 0;
        }

        if (current_founded_length == length)
        {
            last_free = current_founded_idx + current_founded_length;
            return current_founded_idx;
        }
    }

    if (last_free == 0)
    {
        printf("[error] bitmap: no free bitmap entry\n");
        return 0;
    }
    else
    {
        last_free = 0;
        return find_free(length);
    }
}

size_t bitmap::alloc(size_t length)
{
    size_t v = find_free(length);
    if (v == 0)
    {
        printf("[error] bitmap: can't allocate block count %i\n", length);
        return 0;
    }

    if (set_used(v, length) == 0)
    {

        printf("[error] bitmap: can't allocate block count %i\n", length);
        return 0;
    }

    return v;
}
size_t bitmap::set_free(size_t idx, size_t length)
{

    for (size_t i = 0; i < length; i++)
    {
        if (get(idx + i) == false)
        {
            printf("freeing already free block: %i \n", idx + i);
            while (true)
            {
            };
        }
        set(idx + i, false);
    }
    last_free = idx;
    return 1;
}

size_t bitmap::set_used(size_t idx, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {

        if (get(idx + i) == true)
        {
            printf("setting already set block: %i", idx + i);
        }
        set(idx + i, true);
    }
    return 1;
}
