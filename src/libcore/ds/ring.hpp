#pragma once

#include <stddef.h>
#include <string.h>

#include "libcore/alloc/alloc.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/unreachable.h"

namespace fc
{

template <typename T>
class Ring
{
    T *_data;
    long _head = 0;
    long _tail = 0;
    size_t _capacity = 0;
    size_t _len = 0;

public:
    using Type = T;

    Ring() = default;

    Ring(const Ring &rhs)
    {
        _data = mem_alloc<T>(rhs._capacity).take();
        _capacity = rhs._capacity;
        _head = rhs._head;
        _tail = rhs._tail;
        _len = rhs._len;

        for (size_t i = 0; i < _len; i++)
        {
            long final_idx = (i + _head) % _capacity;
            new (&_data[final_idx]) T(rhs._data[i]);
        }
    }

    Ring(Ring &&other) : _data((other._data)), _head(other._head), _tail(other._tail), _capacity(other._capacity), _len(other._len)
    {
        other._data = nullptr;
        other._head = 0;
        other._tail = 0;
        other._len = 0;
        other._capacity = 0;
    }

    size_t len() const
    {
        return _len;
    }

    void resize(size_t capacity)
    {
        if (capacity <= _capacity)
        {
            return;
        }

        if (_capacity == 0)
        {
            _data = fc::mem_alloc<T>(capacity).take();
            _capacity = capacity;
            return;
        }

        _data = fc::mem_realloc<T>(_data, capacity).take();
        // old data:
        // ..........
        //   ^
        // ..........#######
        // aims to get:
        // ...######........

        if (_head < _tail)
        {
            _capacity = capacity;
            return;
        }
        // make space in between so the offset from the end (i) stay constant
        // P  = orig_cap - i
        // P' = new_cap - i
        // P' = new_cap - orig_cap + P
        // P = P' - (new_cap - orig_cap)
        long delta_cap = capacity - _capacity;
        for (long i = capacity - 1; i >= _head + delta_cap; i--)
        {
            // _data[p'] = _data[p]
            _data[i] = std::move(_data[i - delta_cap]);
        }
        _head = capacity - _capacity + _head;
        _capacity = capacity;
    }

    void push(T &&value)
    {
        // i * 1.5
        if (len() + 1 >= _capacity)
        {
            resize((len() + 2) + (len() + 2) / 2);
        }

        new (&_data[_tail]) T(std::move(value));

        _tail = (_tail + 1) % _capacity;
        _len++;
    }

    void push(T const &value)
    {
        // i * 1.5
        if (len() + 1 >= _capacity)
        {
            resize((len() + 2) + (len() + 2) / 2);
        }

        new (&_data[_tail]) T(value);

        _tail = (_tail + 1) % _capacity;
        _len++;
    }

    T pop()
    {
        if (len() == 0) [[unlikely]]
        {
            unreachable$();
        }

        T value = std::move(_data[_head]);

        _data[_head].~T();

        _head = (_head + 1) % _capacity;

        _len--;
        return value;
    }

    auto &head()
    {
        return _data[_head];
    }

    auto &back()
    {
        return _data[(_tail - 1 + _capacity) % _capacity];
    }

    ~Ring()
    {
        for (size_t i = 0; i < _len; i++)
        {
            _data[(i + _head) % _capacity].~T();
        }
        fc::mem_free(_data);
        _capacity = 0;
        _len = 0;
        _head = 0;
        _tail = 0;
    }
};

static_assert(Viewable<Vec<int>>);
} // namespace fc
