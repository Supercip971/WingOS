#pragma once

#include <libcore/core.hpp>
#include <libcore/mem/view.hpp>

#include "libcore/alloc/alloc.hpp"
#include "libcore/ds/array.hpp"
#include <stdlib.h>
#include "libcore/logic.hpp"
namespace core
{
template <typename T>
class Vec
{

    T *_data = nullptr;
    size_t _count = 0;
    size_t _capacity = 0;

public:
    using Type = T;
    Vec() = default;
    Vec(Vec &&other)
    {
        core::swap(_data, other._data);
        core::swap(_count, other._count);
        core::swap(_capacity, other._capacity);
    }


    Vec(const Vec &other)
    {
        _data = nullptr;
        _count = 0;
        _capacity = 0;
        (reserve(other._capacity));
        for (size_t i = 0; i < other._count; i++)
        {
            (push(other._data[i]));
        }
    }

    Vec &operator=(Vec &&other)
    {
        core::swap(_data, other._data);
        core::swap(_count, other._count);
        core::swap(_capacity, other._capacity);
        return *this;
    }

    Vec &operator=(const Vec &other)
    {
        if (this == &other)
        {
            return *this;
        }
        release();
        (reserve(other._capacity));
        for (size_t i = 0; i < other._count; i++)
        {
            (push(other._data[i]));
        }
        return *this;
    }

    void release()
    {
        if (_data != nullptr)
        {
            clear();
            core::mem_free(_data);
            _data = nullptr;
            _capacity = 0;
        }
    }

    void clear()
    {
        for (size_t i = 0; i < _count; i++)
        {
            _data[i].~T();
        }
        _count = 0;
    }

    Result<void> reserve(size_t capacity)
    {
        size_t ncap = core::max(capacity, 16ul);
        if (ncap > _capacity)
        {

            T *new_data = try$(core::mem_realloc<T>(_data, ncap * 2));
            _data = new_data;
            _capacity = ncap;
        }

        return {};
    }

    Result<void> push(T &&value)
    {
        try$(reserve(_count + 1));
        new (&_data[_count]) T(core::move(value));
        _count++;
        return Result<void>();
    }

    Result<void> push(const T &value)
    {
        try$(reserve(_count + 1));
        new (&_data[_count]) T(value);
        _count++;
        return Result<void>();
    }

    Result<void> push(Vec<T> &&arr)
    {
        try$(reserve(_count + arr.len()));
        for (size_t i = 0; i < arr.len(); i++)
        {
            new (&_data[_count + i]) T(core::move(arr[i]));
        }
        _count += arr.len();
        return {};
    }

    T &operator[](size_t index)
    {
#ifdef ADVANCED_CHECK
        if (index > this->len()) [[unlikely]]
        {
            log::err$("error: out of range operator[] with vec {} > {}", index, this->len());
            abort();
        }
#endif
        return _data[index];
    }

    const T &operator[](size_t index) const
    {

#ifdef ADVANCED_CHECK
        if (index > this->len()) [[unlikely]]
        {
            log::err$("error: out of range operator[] with vec {} > {}", index, this->len());
            abort();
        }
#endif

        return _data[index];
    }

    size_t len() const
    {
        return _count;
    }

    T *data()
    {
        return _data;
    }

    const T *data() const
    {
        return _data;
    }

    T &&pop()
    {
        if (_count > 0)
        {
            _count--;
            return core::move(_data[_count]);
        }

        //abort();
        return core::move(_data[0]);
    }

    T &&pop(size_t id)
    {
        if (id >= _count)
        {
          //  abort();
            return core::move(_data[0]);
        }

        T &&value = core::move(_data[id]);
        for (size_t i = id; i < _count - 1; i++)
        {
            _data[i] = core::move(_data[i + 1]);
        }
        _count--;
        return value;
    }

    ~Vec()
    {
        release();
    }

    static Result<Vec> with_capacity(size_t capacity)
    {
        Vec vec;
        try$(vec.reserve(capacity));

        return core::move(vec);
    }

    constexpr T *begin()
    {
        return _data;
    }

    constexpr T *end()
    {
        return _data + _count;
    }
    constexpr const T *begin() const
    {
        return _data;
    }

    constexpr const T *end() const
    {
        return _data + _count;
    }

    constexpr bool contain(const T &value) const
    {
        for (size_t i = 0; i < _count; i++)
        {
            if (_data[i] == value)
            {
                return true;
            }
        }
        return false;
    }
};

static_assert(Viewable<Vec<int>>);
} // namespace core