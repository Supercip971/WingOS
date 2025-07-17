
#pragma once
#include <libcore/fmt/impl/integers.hpp>
#include <libcore/fmt/impl/ranges.hpp>
#include <libcore/io/writer.hpp>
#include <libcore/str.hpp>
#include <math/range.hpp>

#include "libcore/lock/lock.hpp"
#include "libcore/mem/view.hpp"
#include "libcore/result.hpp"
#include "libcore/type-utils.hpp"
#include "libcore/type/trait.hpp"

namespace fmt
{

template <core::IsConvertibleTo<core::Str> T, core::Writable Targ>
constexpr core::Result<void> format_v(Targ &target, T &&value)
{
    target.write(core::Str(value));
    return {};
}

// What I have done ?

template <core::Writable Targ>
constexpr core::Result<void> format_impl(Targ &target, core::Str fmt, int _c)
{
    size_t c = _c;

    while (c < fmt.len())
    {
        target.write(&fmt[c], 1);
        c++;
    }
    return {};
}

template <core::Writable Targ, typename Arg>
constexpr core::Result<void> format_impl(Targ &target, core::Str fmt, int _c, Arg &&a)
{
    size_t c = _c;
    while (c < fmt.len() && fmt[c] != '{')
    {
        target.write(&fmt[c], 1);
        c++;
    }
    if (c >= fmt.len())
    {
        return {};
    }
    if (c + 1 < fmt.len() && fmt[c] == '{' && fmt[c + 1] == '}')
    {

        format_v(target, a);
        return format_impl(target, fmt, c + 2);
    }
    else
    {
        c++;
        return format_impl(target, fmt, c, core::forward<Arg>(a));
    }
}

template <core::Writable Targ, typename Arg, typename... Args>
constexpr core::Result<void> format_impl(Targ &target, core::Str fmt, int _c, Arg &&a, Args &&...args)
{

    size_t c = _c;
    if (fmt.data() == nullptr)
    {
        return format_v(target, core::Str("{null}"));
    }
    while (c < fmt.len() && fmt[c] != '{')
    {
        target.write(fmt.begin() + c, 1);
        c++;
    }
    if (c >= fmt.len())
    {
        return {};
    }
    if (c + 1 < fmt.len() && fmt[c] == '{' && fmt[c + 1] == '}')
    {
        if constexpr (sizeof...(args) > 0)
        {
            format_v(target, a);
            return format_impl(target, fmt, c + 2, core::forward<Args>(args)...);
        }
        else
        {
            return {};
        }
    }
    else
    {
        c++;
        return format_impl(target, fmt, c, a, core::forward<Args>(args)...);
    }
}

template <core::Writable Targ, core::IsConvertibleTo<core::Str> Fmt, typename... Args>
constexpr core::Result<void> format(Targ &target, Fmt &&fmt, Args &&...args)
{
    return format_impl(target, core::Str(fmt), 0, core::forward<Args>(args)...);
}

} // namespace fmt