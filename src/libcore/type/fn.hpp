#pragma once

#include <typeinfo>

#include "tuple.hpp"

namespace core
{
template <typename X, typename... Args>
core::TypeTuple<Args...> function_expand_utils(X(Args...));

// FIXME: just use https://en.cppreference.com/w/cpp/language/pack_indexing.html

template <size_t idx, auto T>
using FunctionArgType = typename decltype(function_expand_utils(T))::template TypeAt<idx>;

static inline float _function_test(bool T, int c)
{
    (void)T;
    (void)c;
    return 0;
}

static_assert(core::IsSame<FunctionArgType<0, _function_test>, bool>);
static_assert(core::IsSame<FunctionArgType<1, _function_test>, int>);

// function return type
template <typename X, typename... Args>
struct FunctionReturnType_I
{
    using Type = X;
};

template <typename X, typename... Args>
typename core::FunctionReturnType_I<X, Args...>::Type function_return_type_(X(Args...));

template <auto T>
using FunctionReturnType = decltype(function_return_type_(T));

static_assert(core::IsSame<FunctionReturnType<_function_test>, float>);
} // namespace core
