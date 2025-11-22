
#pragma once
#include <libcore/mem/mem.hpp>
#include <libcore/result.hpp>
#include <math/range.hpp>
#include <stddef.h>
#include <stdint.h>
#include "libcore/fmt/log.hpp"

namespace core
{

struct Bitmap
{
    size_t _cache_latest_free = 0;

public:
    MemAccess<uint8_t> _data;

    Bitmap() = default;
    Bitmap(MemAccess<uint8_t> &&data) : _cache_latest_free(0), _data{core::move(data)} {};

    constexpr uint8_t byte(size_t index) const
    {
        return _data[index / 8];
    }
    constexpr bool bit(size_t index) const
    {
        return (byte(index) & (1 << (index % 8)));
    }
    constexpr bool operator[](size_t index) const
    {
        return this->bit(index);
    }

    constexpr void bit(size_t index, bool value)
    {

        auto prev = _data[index / 8];
        if (value)
        {
            _data[index / 8] = prev | ((uint8_t)1 << (index % 8));
        }
        else
        {
            _data[index / 8] = prev & ~((uint8_t)1 << (index % 8));
        }
    }

    constexpr void fill(bool value)
    {
        for (size_t i = 0; i < len(); i++)
        {
            bit(i, value);
        }
    }

    template <math::IntRangeable T>
    constexpr void fill(bool value, T range)
    {
        if (value == false)
        {
            _cache_latest_free = range.start();
        }
        for (size_t i = range.start(); i < range.end(); i++)
        {
            bit(i, value);
        }
    }

    /*
        Used to test if the bitmap is filled with the expected (inverse) value
        if you want to fill 0 and expect to replace 1 for example.
        This may be used to test if we are freeing already freed memory. For example
    */

    template <math::IntRangeable T>
    constexpr Result<void> fill_expected_inverse(bool value, T range)
    {
        for (size_t i = range.start(); i < range.end(); i++)
        {
            if (bit(i) == value) [[unlikely]]
            {
                return Result<void>("Bitmap: fill_expected_inverse: expected inverse");
            }
            bit(i, value);
        }
        return {};
    }

    constexpr size_t len() const
    {
        return _data.len() * 8;
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

    constexpr Result<size_t> alloc(size_t continuous_len)
    {
        size_t found = 0;
        size_t start = _cache_latest_free;
        for (size_t i = _cache_latest_free; i < len(); i++)
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
                    _cache_latest_free = i;
                    return start;
                }
            }
        }

        // we already searched the whole bitmap
        if (_cache_latest_free == 0)
        {

            return Result<size_t>("Bitmap: Not found");
        }

        // search from the beginning
        _cache_latest_free = 0;
        return alloc(continuous_len);
    }
};
} // namespace core