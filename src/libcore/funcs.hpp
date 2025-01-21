#pragma once

#include <libcore/result.hpp>
#include <libcore/type-utils.hpp>
#include <stddef.h>

#include "libcore/type/trait.hpp"

void operator delete(void *);

namespace core
{

template <typename T>
concept Iterable = requires(T a) {
    {
        *a.begin()
    } -> IsConvertibleTo<const typename T::Type>;
    {
        *a.end()
    } -> IsConvertibleTo<const typename T::Type>;
};

template <Iterable T, typename V>
constexpr Optional<size_t> indexOf(V val)
{
    size_t idx = 0;
    for (auto it = val.begin(); it != val.end(); it++)
    {
        if (*it == val)
        {
            return idx;
        }
        idx++;
    }
    return Optional<size_t>();
}

template <Iterable T, typename F>
constexpr bool find(T view, F f)
{
    for (auto it = view.begin(); it != view.end(); it++)
    {
        bool do_continue = f(*it);
        if (!do_continue)
        {
            return true;
        }
    }
    return false;
}

template <Iterable T, typename F>
constexpr void forEach(T &view, F f)
{
    for (auto it = view.begin(); it != view.end(); ++it)
    {
        f(*it);
    }
}

template <Iterable T, typename F>
constexpr size_t forEachIdx(T &view, F f)
{
    size_t idx = 0;
    for (auto it = view.begin(); it != view.end(); ++it)
    {
        f(*it, idx);
        idx++;
    }
    return idx;
}

} // namespace core