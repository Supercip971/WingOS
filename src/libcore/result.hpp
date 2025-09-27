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

    using ErrorType = RemoveReference<ErrT>;
    using ValueType = RemoveReference<ValT>;

private:
    // FIXME: introduce either type

public:
    Optional<ValT> _value;
    Optional<ErrT> _error;
    constexpr Result() : _value(), _error() {}

    constexpr ~Result()
    {
        _value.~Optional();
        _error.~Optional();
    }

    constexpr Result(const ValueType &value) : _value(value), _error() {}

    constexpr Result(ValueType &&value) : _value(core::move(value)), _error() {}

    constexpr Result(const ErrorType &error) : _value(), _error(error) {}

    constexpr Result(ErrorType &&error) : _value(), _error(core::move(error)) {}

    constexpr Result(Result &&other) : _value(core::move(other._value)), _error(core::move(other._error)) {}

    constexpr Result &operator=(Result &&other)
    {
        _value = core::move(other._value);
        _error = core::move(other._error);
        return *this;
    }
    static constexpr auto csuccess(ValueType val)
    {
        auto res = Result<ValT, ErrT>();
        res._value = core::move(val);
        return core::move(res);
    }
    static constexpr auto success(ValueType &&val)
    {
        auto res = Result<ValT, ErrT>();
        res._value = core::move(val);
        return core::move(res);
    }
    static constexpr auto error(ErrorType &&error)
    {
        auto res = Result<ValT, ErrT>();
        res._error = core::move(error);
        return core::move(res);
    }

    void assert();

    ValT &&unwrap()
    {
        assert();
        return core::move(_value.unwrap());
    }

    explicit constexpr operator bool() const
    {
        return _value.has_value();
    }

    constexpr ErrT &error() const
    {
        return _error.value();
    }
    constexpr ErrT &error()
    {
        return _error.value();
    }

    constexpr bool is_error() const
    {
        return _error.has_value();
    }
};

template <typename ErrT>
struct Result<void, ErrT> : public NoCopy
{
private:
public:
    Optional<ErrT> _error;
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

    explicit constexpr operator bool() const
    {
        return !_error.has_value();
    }

    constexpr ErrT error() const
    {
        return _error.value();
    }

    constexpr ~Result()
    {
        _error.~Optional();
    }

    constexpr bool is_error() const
    {
        return _error.has_value();
    }
};

template <typename A, typename T>
concept IsConvertibleToResult =
    core::IsConvertibleTo<typename A::ValueType, T>;

template <typename T>
using EResult = Result<T, Str>;

extern void debug_provide_info(const char *info, const char *data);

#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)

#define try$(expr) ({                                                          \
    auto _result = (expr);                                                     \
    if ((_result._error.has_value())) [[unlikely]]                             \
    {                                                                          \
        core::debug_provide_info("error:    ", (const char *)_result.error()); \
        core::debug_provide_info("at:       ", #expr);                         \
        core::debug_provide_info("function: ", __FUNCTION__);                  \
        core::debug_provide_info("file:     ", __FILE__);                      \
        core::debug_provide_info("line:     ", STRINGIZE(__LINE__));           \
        return _result.error();                                                \
    }                                                                          \
    _result.unwrap();                                                          \
})

} // namespace core