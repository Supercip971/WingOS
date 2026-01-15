#pragma once
#include <libcore/bound.hpp>
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

    T _val;
    constexpr const T *as_ptr() const
    {
        return (T const *)(_data);
    }

    constexpr T *as_ptr()
    {
        return (T *)(_data);
    }


    constexpr Storage(Storage const &other bounded$)
        : _val(other.value())
    {
    }

    constexpr Storage() {}
    constexpr Storage(const T &value)
        : _val(value)
    {
    }
    constexpr Storage(T &&value bounded$)
        : _val(core::move(value))
    {
    }


    constexpr Storage &operator=(const Storage &other )
    {
        this->value() = other.value();
        return *this;
    }

    constexpr Storage &operator=(Storage &&other )
    {
        this->value() = core::move(other.value());
        return *this;
    }

    constexpr void destruct()
    {
        value().~T();
    }
    constexpr ~Storage()
    {
    }

    constexpr T &value() bounded$
    {
        return *as_ptr();
    }
    constexpr T const &value() const bounded$
    {
        return *as_ptr();
    }


    constexpr T take()
    {
        T val = core::move(_val);
        destruct();
        return val;
    }

    constexpr T & retreive() bounded$
    {
        return (*as_ptr());
    }

    constexpr void copy(const Storage &_other)
    {
        new (_data) T(_other.value());
    }

    // Returns a default-constructed T
    // Note: This requires T to be default-constructible
    static constexpr T empty()
        requires core::IsDefaultConstructible<T>
    {
        return T{};
    }

    // template<typename ...Args>
    // constexpr Storage(Args&&... args) : _value(core::forward(args)...) {}
};

class NoneValue
{
public:
};

template <class T>
class Optional
{
    using Type = Pure<T>;

public:
    Storage<Type> _value;
    bool _contain_value;

public:
    constexpr Optional() : _contain_value(false) {}
    constexpr Optional(const Type &value) : _value(value), _contain_value(true) {}
    constexpr Optional(Type &&value) : _value(core::move(value)), _contain_value(true) {}

    explicit constexpr Optional([[maybe_unused]] NoneValue v) : _contain_value(false) {};

    constexpr Optional(const Optional &other) : _contain_value(other._contain_value)
    {
        if (_contain_value)
        {
            new (&_value) Storage<Type>(other._value);
        }
    }
    constexpr Optional(Optional &&other) :
                                           _contain_value(other._contain_value)
    {
        if(other._contain_value)
        {
            _value = (Storage<Type>(core::move(other._value.value())));
            other._contain_value = false;
        }
    }

    constexpr Optional &operator=(const Optional &other)
    {
        if (this == &other)
        {
            return *this;
        }

        if (other.has_value() && _contain_value)
        {
            _value.value() = other._value.value();
        }
        else if (other.has_value() && !_contain_value)
        {
            new (&_value) Storage<Type>(other._value);
            _contain_value = true;
        }
        else if (!other.has_value() && _contain_value)
        {
            _contain_value = false;
            _value.destruct();
        }
        return *this;
    }

    constexpr Optional &operator=(const Type &value)
    {
        if (_contain_value)
        {
            _value.value() = value;
        }
        else
        {
            new (&_value) Storage<Type>(value);
            _contain_value = true;
        }
        return *this;
    }

    constexpr Optional &operator=(Type &&value)
    {
        if (_contain_value)
        {
            _value.value() = core::move(value);
        }
        else
        {
            new (&_value) Storage<Type>(core::move(value));
            _contain_value = true;
        }
        return *this;
    }

    constexpr Optional &operator=(Optional &&other)
    {
        if (this == &other)
        {
            return *this;
        }

        if (other.has_value() && _contain_value)
        {
            _value.value() = core::move(other._value.value());
            other._contain_value = false;
        }
        else if (other.has_value() && !_contain_value)
        {
            new (&_value) Storage<Type>(core::move(other._value.value()));
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

    constexpr Type &value()
    {
        return _value.value();
    }

    constexpr Type const &value() const
    {
        return _value.value();
    }

    constexpr Type &operator*()
    {
        return _value.value();
    }

    constexpr const Type &operator*() const
    {
        return _value.value();
    }

    constexpr Type *operator->()
    {
        return &_value.value();
    }

    constexpr const Type *operator->() const
    {
        return &_value.value();
    }
    constexpr Type take()
    {
        _contain_value = false;
        return core::move(_value.value());
    }
    constexpr Type& unwrap()
    {
        return (_value.value());
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
