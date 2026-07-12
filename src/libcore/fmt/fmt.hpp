#pragma once
#include <libcore/fmt/impl/integers.hpp>
#include <libcore/fmt/impl/ranges.hpp>
#include <libcore/io/writer.hpp>
#include <libcore/str.hpp>

#include "libcore/fmt/impl/asset_kind.hpp"

#include "libcore/mem/view.hpp"
#include "libcore/result.hpp"
#include "libcore/type-utils.hpp"
#include "libcore/type/trait.hpp"

namespace fmt
{

template <fc::IsConvertibleTo<fc::Str> T, fc::Writable Targ>
constexpr fc::Result<void> format_v(Targ &target, T &&value)
{
    target.write(fc::Str(value));
    return {};
}

// What I have done ?

template <fc::Writable Targ>
constexpr fc::Result<void> format_impl(Targ &target, fc::Str fmt, int _c)
{
    size_t c = _c;

    target.write(fmt.data() + c, fmt.len() - c);
    return {};
}

template <fc::Writable Targ, typename Arg>
constexpr fc::Result<void> format_impl(Targ &target, fc::Str fmt, int _c, Arg &&a)
{
    size_t c = _c;
    while (c < fmt.len() && fmt[c] != '{')
    {
        c++;
    }
    target.write(fmt.begin() + _c, c - _c);
    if (c >= fmt.len())
    {
        return {};
    }
    if (c + 1 < fmt.len() && fmt[c] == '{' && fmt[c + 1] == '}')
    {

        format_v(target, std::forward<Arg>(a));
        return format_impl(target, fmt, c + 2);
    }
    else
    {
        c++;
        return format_impl(target, fmt, c, std::forward<Arg>(a));
    }
}

template <fc::Writable Targ, typename Arg, typename... Args>
constexpr fc::Result<void> format_impl(Targ &target, fc::Str fmt, int _c, Arg &&a, Args &&...args)
{

    size_t c = _c;
    if (fmt.data() == nullptr)
    {
        return format_v(target, fc::Str("{null}"));
    }

    while (c < fmt.len() && fmt[c] != '{')
    {
        c++;
    }
    target.write(fmt.begin() + _c, c - _c);
    if (c >= fmt.len())
    {
        return {};
    }
    if (c + 1 < fmt.len() && fmt[c] == '{' && fmt[c + 1] == '}')
    {
        if constexpr (sizeof...(args) > 0)
        {
            format_v(target, std::forward<Arg>(a));
            return format_impl(target, fmt, c + 2, std::forward<Args>(args)...);
        }
        else
        {
            return {};
        }
    }
    else
    {
        c++;
        return format_impl(target, fmt, c, std::forward<Arg>(a), std::forward<Args>(args)...);
    }
}

template <fc::Writable Targ, fc::IsConvertibleTo<fc::Str> Fmt, typename... Args>
constexpr fc::Result<void> format(Targ &target, Fmt &&fmt, Args &&...args)
{
    return format_impl(target, fc::Str(fmt), 0, std::forward<Args>(args)...);
}

} // namespace fmt
