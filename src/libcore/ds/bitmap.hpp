
#pragma once
#include <libcore/mem/mem.hpp>
#include <libcore/result.hpp>
#include <stddef.h>
#include <stdint.h>
namespace core
{

struct Bitmap
{
public:
    MemAccess<uint8_t> _data;

    Bitmap() = default;
    Bitmap(MemAccess<uint8_t> &&data) : _data{core::move(data)} {};

    constexpr bool byte(size_t index) const
    {
        return _data[index / 8];
    }
    constexpr bool bit(size_t index) const
    {
        return (byte(index) & (1 << (index % 8))) != 0;
    }
    constexpr bool operator[](size_t index) const
    {
        return this->bit(index);
    }

    constexpr void bit(size_t index, bool value)
    {
        if (value)
        {
            _data[index / 8] |= (1 << (index % 8));
        }
        else
        {
            _data[index / 8] &= ~(1 << (index % 8));
        }
    }

    constexpr void fill(bool value)
    {
        for (size_t i = 0; i < len(); i++)
        {
            bit(i, value);
        }
    }

    constexpr size_t len() const
    {
        return _data.len() * 8;
    }

    constexpr operator bool() const
    {
        return _data;
    }

    constexpr bool operator==(const Bitmap &other) const
    {
        if (len() != other.len())
        {
            return false;
        }

        for (size_t i = 0; i < len(); i++)
        {
            if (bit(i) != other.bit(i))
            {
                return false;
            }
        }

        return true;
    }

    constexpr bool operator!=(const Bitmap &other) const
    {
        return !(*this == other);
    }

    constexpr const uint8_t *data() const
    {
        return _data.data();
    }

    constexpr Result<size_t> find(size_t continuous_len)
    {
        size_t found = 0;
        size_t start = 0;
        for (size_t i = 0; i < len(); i++)
        {
            if (bit(i))
            {
                found = 0;
                start = i + 1;
            }
            else
            {
                found++;
                if (found == continuous_len)
                {
                    return start;
                }
            }
        }
        return Result<size_t>("Bitmap: Not found");
    }
};
} // namespace core