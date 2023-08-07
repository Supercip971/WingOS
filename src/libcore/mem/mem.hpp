#pragma once

#include <libcore/mem/view.hpp>
#include <libcore/type-utils.hpp>
#include <stddef.h>
#include <stdint.h>
namespace core
{

// NON owned memory access

template <class T = uint8_t>
class MemAccess : public NoCopy
{
protected:
    T *_data;
    size_t _len;

public:
    using Type = T;

    constexpr MemAccess() : _data(nullptr), _len(0) {}
    constexpr MemAccess(T *data, size_t len) : _data(data), _len(len) {}

    constexpr MemAccess(MemAccess &&other) : _data(other._data), _len(other._len)
    {
        other._data = nullptr;
        other._len = 0;
    }

    constexpr MemAccess &operator=(MemAccess &&other)
    {
        _data = other._data;
        _len = other._len;
        other._data = nullptr;
        other._len = 0;
        return *this;
    }

    constexpr T const &operator[](size_t index) const { return _data[index]; }
    constexpr T &operator[](size_t index) { return _data[index]; }

    constexpr T *data() { return _data; }
    constexpr const T *data() const { return _data; }

    __attribute__((always_inline)) inline constexpr size_t len() const { return _len; }

    constexpr const T *begin() const { return _data; }
    constexpr const T *end() const { return _data + _len; }
    constexpr T *begin() { return _data; }
    constexpr T *end() { return _data + _len; }

    constexpr operator bool() const { return _data != nullptr; }

    constexpr bool operator==(const MemView<T> &other) const
    {
        if (len() != other.len())
        {
            return false;
        }

        for (size_t i = 0; i < len(); i++)
        {
            if ((*this)[i] != other[i])
            {
                return false;
            }
        }

        return true;
    }

    constexpr bool operator!=(const MemView<T> &other) const
    {
        return !(*this == other);
    }

    constexpr operator MemView<T>() const
    {
        return MemView<T>(_data, _len);
    }

    constexpr MemView<T> view() const
    {
        return MemView<T>(_data, _len);
    }
};

static_assert(Viewable<MemAccess<char>>);

} // namespace core