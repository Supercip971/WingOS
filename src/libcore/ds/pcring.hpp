#pragma once

#include <stddef.h>
#include <string.h>
#include <utility>

#include "arch/generic/instruction.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/lock/lock.hpp"
#include "libcore/mem/mem.hpp"
#include "libcore/unreachable.h"

namespace fc
{

template <typename T>
class PCRing
{
    fc::Lock lock = {};
    long _head = 0;
    long _tail = 0;
    size_t _capacity = 0;
    size_t _len = 0;

    T _data[];

public:
    using Type = T;

    PCRing(size_t byte_length) : _capacity((byte_length - sizeof(PCRing)) / sizeof(T))
    {
    }

    static PCRing<T> *create_from_mem(void *mem, size_t byte_length)
    {
        return new (mem) PCRing<T>(byte_length);
    }

    static PCRing<T> *use_already_constructed_from_mem(void *mem)
    {
        return reinterpret_cast<PCRing<T> *>(mem);
    }

    size_t len() const
    {
        return _len;
    }

    template <typename U>
    void produce(U &&value)
    {
        while (true) [[unlikely]]
        {
            lock.lock();
            if (len() < _capacity)
            {
                break;
            }
            lock.release();
            arch::pause();
        }

        new (&_data[_tail]) T(std::move(value));

        _tail = (_tail + 1) % _capacity;
        _len++;

        lock.release();
    }

    T consume()
    {
        while (true) [[unlikely]]
        {
            lock.lock();
            if (len() != 0)
            {
                break;
            }
            lock.release();
            arch::pause();
        }

        T value = std::move(_data[_head]);

        _data[_head].~T();

        _head = (_head + 1) % _capacity;

        _len--;

        lock.release();
        return value;
    }

    fc::Result<void> try_consume(T *to)
    {
        lock.lock();
        if (len() == 0)
        {
            lock.release();
            return "no data";
        }

        *to = std::move(_data[_head]);

        _data[_head].~T();

        _head = (_head + 1) % _capacity;

        _len--;

        lock.release();
        return {};
    }

    auto &dirty_head()
    {
        return _data[_head];
    }

    auto &dirty_back()
    {
        return _data[(_tail - 1 + _capacity) % _capacity];
    }
};

static_assert(Viewable<Vec<int>>);
} // namespace fc
