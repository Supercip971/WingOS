#include "bitmap.h"
#include <logging.h>

bitmap::bitmap(uint8_t *data, size_t size) : bitmap_size(size)
{
    log("bitmap", LOG_INFO) << "creating bitmap " << (uint64_t)data << " of size " << size << "bit and " << size / 8 << "bytes";
    memset(data, 0xff, size / 8);
    buffer = data;
}
constexpr void bitmap::set(size_t idx, bool value)
{
    if (idx > bitmap_size)
    {
        log("bitmap", LOG_ERROR) << "trying to read out of bound of the bitmap" << idx << " > " << bitmap_size;
        return;
    }
    uint8_t bit = idx % 8;
    uint64_t byte = idx / 8;
    if (value)
    {
        buffer[byte] |= (1 << (bit));
    }
    else
    {
        buffer[byte] &= ~(1 << (bit));
    }
}
constexpr bool bitmap::get(size_t idx) const
{
    if (idx > bitmap_size)
    {
        log("bitmap", LOG_ERROR) << "trying to read out of bound of the bitmap" << idx << " > " << bitmap_size;
        return false;
    }
    return (buffer[idx / 8] & (1 << (idx % 8)));
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
        log("bitmap", LOG_WARNING) << "no free entry founded for the bitmap";
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
        log("bitmap", LOG_WARNING) << " can't allocate block count " << length;

        return 0;
    }
    if (set_used(v, length) == 0)
    {
        log("bitmap", LOG_WARNING) << " can't set used block count " << length;
        return 0;
    }

    return v;
}
size_t bitmap::set_free(size_t idx, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        set(idx + i, false);
    }
    return 1;
}
size_t bitmap::set_used(size_t idx, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        set(idx + i, true);
    }
    return 1;
}
