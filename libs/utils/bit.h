#ifndef BIT_H
#define BIT_H
#include <stddef.h>
namespace utils
{
    template <typename T>
    constexpr bool get_bit(const T value, const size_t offset)
    {
        return (value & (1 << (offset)));
    }
    template <typename T>
    constexpr void set_bit(T &value, const size_t offset, const bool new_value)
    {
        if (new_value)
        {
            value |= (1 << (offset));
        }
        else
        {
            value &= ~(1 << (offset));
        }
    }

} // namespace utils

#endif // BIT_H
