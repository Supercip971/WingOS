#pragma once

#include <libcore/optional.hpp>
#include <libcore/type-utils.hpp>

#include "libcore/type/trait.hpp"

namespace core
{

class Str;
template <typename ValT, typename ErrT = const char *>
struct Result : public NoCopy
{

private:
    Optional<ValT> _value;
    Optional<ErrT> _error;

public:
    using ErrorType = RemoveReference<ErrT>;
    using ValueType = RemoveReference<ValT>;
    constexpr Result() : _value(), _error() {}

    constexpr ~Result()
    {
        _value.~Optional();
        _error.~Optional();
    }

    constexpr Result(const ValT &value) : _value(value), _error() {}

    constexpr Result(ValT &&value) : _value(core::move(value)), _error() {}

    constexpr Result(const ErrT &error) : _value(), _error(error) {}

    constexpr Result(ErrT &&error) : _value(), _error(core::move(error)) {}

    constexpr Result(Result &&other) : _value(core::move(other._value)), _error(core::move(other._error)) {}

    constexpr Result &operator=(Result &&other)
    {
        _value = core::move(other._value);
        _error = core::move(other._error);
        return *this;
    }

    void assert();

    ValT &&unwrap()
    {
        assert();
        return core::move(_value.unwrap());
    }

    constexpr operator bool() const
    {
        return _value.has_value();
    }

    constexpr ErrT error() const
    {
        return *_error;
    }
};

template <typename ErrT>
struct Result<void, ErrT> : public NoCopy
{

private:
    Optional<ErrT> _error;

public:
    using ErrorType = RemoveReference<ErrT>;
    using ValueType = void;
    constexpr Result() : _error() {}

    constexpr Result(const ErrT &error) : _error(error) {}

    constexpr Result(ErrT &&error) : _error(core::move(error)) {}

    constexpr Result(Result &&other) : _error(core::move(other._error)) {}

    constexpr Result &operator=(Result &&other)
    {
        _error = core::move(other._error);
        return *this;
    }

    void assert();

    void unwrap()
    {
        assert();
    }

    constexpr operator bool() const
    {
        return !_error.has_value();
    }

    constexpr ErrT error() const
    {
        return *_error;
    }

    constexpr ~Result()
    {
        _error.~Optional();
    }
};

template <typename A, typename T>
concept IsConvertibleToResult =
    core::IsConvertibleTo<typename A::ValueType, T>;

template <typename T>
using EResult = Result<T, Str>;

#define try$(expr) ({           \
    auto _result = (expr);      \
    if (!_result)               \
    {                           \
        return _result.error(); \
    }                           \
    _result.unwrap();           \
})

} // namespace core