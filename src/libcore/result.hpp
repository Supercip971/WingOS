#pragma once

#include <libcore/bound.hpp>
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
    Optional<ValueType> _value;
    Optional<ErrorType> _error;
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
    template <typename U>
    static constexpr Result<ValT, ErrT> success(U val bounded$)
        requires core::IsConvertibleTo<U, ValueType>
    {
        Result<ValT, ErrT> res;
        res._value = (core::forward<U>(val));
        return res;
    }
    
    template <typename E>
    static constexpr Result<ValT, ErrT> error(E err bounded$)
        requires core::IsConvertibleTo<E, ErrorType>
    {
        Result<ValT, ErrT> res;
        res._error = (core::forward<E>(err));
        return res;
    }
    void assert();
    ValT& unwrap() & bounded$ 
    {
        assert();
        return (_value.unwrap());
    }

    ValT&& unwrap() && 
    {
        assert();
        return core::move(_value.unwrap());
    }



    ValT take() && = delete;
    ValT take() & bounded$ 
    {
        assert();
        return core::move(_value.take());
    }

    explicit constexpr operator bool() const
    {
        return _value.has_value();
    }

    constexpr ErrT &error() const
    {
        return _error.value();
    }
    constexpr ErrT &error() bounded$
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

    constexpr Result(Result const &other) : _error(other._error) {}

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

    constexpr void take() const {};

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

#define try_v$(expr, vname) ({                                               \
    auto vname = (expr);                                                     \
    if ((vname._error.has_value())) [[unlikely]]                             \
    {                                                                        \
        core::debug_provide_info("error:    ", (const char *)vname.error()); \
        core::debug_provide_info("at:       ", #expr);                       \
        core::debug_provide_info("function: ", __FUNCTION__);                \
        core::debug_provide_info("file:     ", __FILE__);                    \
        core::debug_provide_info("line:     ", STRINGIZE(__LINE__));         \
        return vname.error();                                                \
    }                                                                        \
    vname.take();                                                          \
})

#define CONCAT_IMPL(x, y) x##y
#define MACRO_CONCAT(x, y) CONCAT_IMPL(x, y)

#define try$(expr) try_v$(expr, MACRO_CONCAT(_result, __COUNTER__))

} // namespace core