#pragma once
#include <libcore/core.hpp>
#include <libcore/type-utils.hpp>
#include <stdint.h>

#include "libcore/type/trait.hpp"
namespace core
{

template <class _T>
union Storage
{

    using T = Pure<_T>;
    [[gnu::aligned(alignof(T))]] uint8_t _data[sizeof(T)];

    constexpr const T *as_ptr() const
    {
        return (T const *)(_data);
    }

    constexpr T *as_ptr()
    {
        return (T *)(_data);
    }

    constexpr Storage() {}
    constexpr Storage(const T &value)
    {
        new (_data) T((value));
    }
    constexpr Storage(T &&value)
    {
        new (_data) T(core::move(value));
    }

    constexpr void destruct()
    {
        value().~T();
    }
    constexpr ~Storage()
    {
        destruct();
    }

    constexpr T &value()
    {
        return *as_ptr();
    }
    constexpr T const &value() const
    {
        return *as_ptr();
    }

    constexpr T &&retreive()
    {
        return core::move(*as_ptr());
    }

    static constexpr T&& empty() {
        return core::move(Storage().retreive());
    }

    // template<typename ...Args>
    // constexpr Storage(Args&&... args) : _value(core::forward(args)...) {}
};

template <class T>
class Optional
{
    Storage<T> _value;
    bool _contain_value;

public:
    constexpr Optional() : _contain_value(false) {}
    constexpr Optional(const T &value) : _value(value), _contain_value(true) {}
    constexpr Optional(T &&value) : _value(core::move(value)), _contain_value(true) {}

    constexpr Optional(const Optional &other) : _contain_value(other._contain_value)
    {
        if (_contain_value)
        {
            _value.value() = other.value();
        }
    }
    constexpr Optional(Optional &&other) : _contain_value(other._contain_value)
    {

        if (other._contain_value)
        {
            _value.value() = core::move(other._value.retreive());
        }
    }

    constexpr Optional &operator=(const Optional &other)
    {

        if (other.has_value() && _contain_value)
        {
            _value.value() = (*other);
        }
        else if (other.has_value() && !_contain_value)
        {
            _value.value() = core::move(*(other));

            _contain_value = true;
        }
        else if (!other.has_value() && _contain_value)
        {
            _contain_value = false;
            _value.destruct();
        }
        return *this;
    }

    constexpr Optional &operator=(Optional &&other)
    {

        if (other.has_value() && _contain_value)
        {
            _value.value() = core::move(other._value.retreive());
        }
        else if (other.has_value() && !_contain_value)
        {
            _value.value() = core::move(other._value.retreive());
            other._contain_value = false;
            _contain_value = true;
        }
        else if (!other.has_value() && _contain_value)
        {
            _contain_value = false;
            _value.destruct();
        }
        return *this;
    }

    constexpr bool has_value() const
    {
        return _contain_value;
    }

    constexpr T &value()
    {
        return _value.value();
    }

    constexpr T const &value() const
    {
        return _value.value();
    }

    constexpr T &operator*()
    {
        return _value.value();
    }

    constexpr const T &operator*() const
    {
        return _value.value();
    }

    constexpr T *operator->()
    {
        return &_value.value();
    }

    constexpr const T *operator->() const
    {
        return &_value.value();
    }

    constexpr T &&unwrap()
    {
        _contain_value = false;
        return core::move(_value.retreive());
    }

    constexpr ~Optional()
    {
        if (_contain_value)
        {
            _value.destruct();
        }
        _contain_value = false;
    }
};

} // namespace core