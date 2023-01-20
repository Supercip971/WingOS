#pragma once

#include <libcore/type-utils.hpp>
#include <stddef.h>
#include <stdint.h>
namespace core
{
template <class T>
class MemView
{
protected:
    const T *_data;
    size_t _len;

public:
    using Type = T;

    constexpr MemView() : _data(nullptr), _len(0) {}
    constexpr MemView(const T *data, size_t len) : _data(data), _len(len) {}

    

    constexpr T const &operator[](size_t index) const { return _data[index]; }

    constexpr const T *data() const { return _data; }
    __attribute__((always_inline)) inline constexpr size_t len() const { return _len; }

    constexpr const T *begin() const { return _data; }
    constexpr const T *end() const { return _data + _len; }

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
};

template <typename T>
concept Viewable = requires(T a)
{
    {
        a.data()
        } -> IsConvertibleTo<const typename T::Type *>;
    {
        a.len()
        } -> IsConvertibleTo<size_t>;
    {
        a.begin()
        } -> IsConvertibleTo<const typename T::Type *>;
    {
        a.end()
        } -> IsConvertibleTo<const typename T::Type *>;
};

static_assert(Viewable<MemView<char>>);
} // namespace core