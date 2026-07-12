#pragma once

#include <libcore/bound.hpp>
#include <libcore/type-utils.hpp>
// NOLINTBEGIN
#include <libcore/core.hpp>
// NOLINTEND

#include "libcore/unreachable.h"

void assert_dump(const char *error);

template <typename ErrT>
void assert_dump(ErrT error);

namespace fc
{

class Str;

template <typename ValT, typename ErrT = const char *>
struct Result : public NoCopy
{

private:
    // FIXME: introduce either type

public:
    using ErrorType = RemoveReference<ErrT>;
    using ValueType = RemoveReference<ValT>;

    union
    {
        ValueType _value;
        ErrorType _error;
    };

    bool _is_error = true;

    constexpr Result() : _error("empty result"), _is_error(true) {}

    constexpr ~Result()
    {
        if (_is_error)
        {
            _error.~ErrorType();
        }
        else
        {
            _value.~ValueType();
        }
    }

    constexpr Result(const ValueType &value) : _value(value), _is_error(false) {}

    constexpr Result(ValueType &&value) : _value(fc::move(value)), _is_error(false) {}

    constexpr Result(const ErrorType &error) : _error(error), _is_error(true) {}

    constexpr Result(ErrorType &&error) : _error(fc::move(error)), _is_error(true) {}

    constexpr Result(Result &&other) : _is_error(other._is_error)
    {
        if (_is_error)
        {
            new (&_error) ErrorType(fc::move(other._error));
        }
        else
        {
            new (&_value) ValueType(fc::move(other._value));
        }
    }

    constexpr Result &operator=(Result &&other)
    {
        if (this == &other)
            return *this;

        // Destroy current value
        if (_is_error)
        {
            _error.~ErrorType();
        }
        else
        {
            _value.~ValueType();
        }

        // Construct new value
        _is_error = other._is_error;
        if (_is_error)
        {
            new (&_error) ErrorType(fc::move(other._error));
        }
        else
        {
            new (&_value) ValueType(fc::move(other._value));
        }
        return *this;
    }

    template <typename U>
    static constexpr Result<ValT, ErrT> success(U val bounded$)
        requires fc::IsConvertibleTo<U, ValueType>
    {
        Result<ValT, ErrT> res{};
        res._is_error = false;
        res._value = (fc::forward<U>(val));
        return res;
    }

    template <typename E>
    static constexpr Result<ValT, ErrT> error(E err bounded$)
        requires fc::IsConvertibleTo<E, ErrorType>
    {
        Result<ValT, ErrT> res{};
        res._is_error = true;
        res._error = (fc::forward<E>(err));
        return res;
    }

    void assert() const
    {
        if (is_error()) [[unlikely]]
        {
            assert_dump(_error);

            unreachable$();
            while (true)
            {
            };
        }
    }

    ValT const &&unwrap() const &&
    {
        assert();
        return fc::move(_value);
    }

    ValT &&unwrap() &&
    {
        assert();
        return fc::move(_value);
    }

    ValT const &unwrap() const &bounded$
    {
        assert();
        return _value;
    }

    ValT &unwrap() & bounded$
    {
        assert();
        return _value;
    }

    ValT take()
    {
        assert();
        return fc::move(_value);
    }

    explicit constexpr operator bool() const
    {
        return !_is_error;
    }

    constexpr ErrT &error() const
    {
        return _error;
    }

    constexpr ErrT &error() bounded$
    {
        return _error;
    }

    constexpr bool is_error() const
    {
        return _is_error;
    }
};

template <typename ErrT>
struct Result<void, ErrT> : public NoCopy
{
public:
    union
    {

        ErrT _error;
    };

    bool _is_error;
    using ErrorType = RemoveReference<ErrT>;
    using ValueType = void;

    constexpr Result() : _error("empty result"), _is_error(false) {}

    constexpr Result(const ErrT &error) : _error(error), _is_error(true) {}

    constexpr Result(ErrT &&error) : _error(fc::move(error)), _is_error(true) {}

    constexpr Result(Result &&other) : _error(fc::move(other._error)), _is_error(other._is_error) {}

    constexpr Result &operator=(Result &&other)
    {
        _is_error = other._is_error;
        _error = fc::move(other._error);
        return *this;
    }

    void assert() const
    {
        if (is_error())
        {
            assert_dump(_error);

            unreachable$();
            while (true)
            {
            };
        }
    }

    void unwrap()
    {
        assert();
    }

    explicit constexpr operator bool() const
    {
        return !_is_error;
    }

    constexpr ErrT error() const
    {
        return _error;
    }

    constexpr void take() const {};

    constexpr ~Result()
    {
        if (_is_error)
        {
            _error.~ErrT();
        }
    }

    constexpr bool is_error() const
    {
        return _is_error;
    }
};

template <typename A, typename T>
concept IsConvertibleToResult =
    fc::IsConvertibleTo<typename A::ValueType, T>;

template <typename T>
using EResult = Result<T, Str>;

extern void debug_provide_info(const char *info, const char *data);

#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)

#define try_v$(expr, vname) ({                                             \
    auto vname = (expr);                                                   \
    if (vname.is_error()) [[unlikely]]                                     \
    {                                                                      \
        fc::debug_provide_info("error:    ", (const char *)vname.error()); \
        fc::debug_provide_info("at:       ", #expr);                       \
        fc::debug_provide_info("function: ", __FUNCTION__);                \
        fc::debug_provide_info("file:     ", __FILE__);                    \
        fc::debug_provide_info("line:     ", STRINGIZE(__LINE__));         \
        return vname.error();                                              \
    }                                                                      \
    vname.take();                                                          \
})

#define CONCAT_IMPL(x, y) x##y
#define MACRO_CONCAT(x, y) CONCAT_IMPL(x, y)

#define try$(expr) try_v$((expr), MACRO_CONCAT(_result, __COUNTER__))

} // namespace fc
