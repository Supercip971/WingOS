
#pragma once
#include <libcore/fmt/flags.hpp>
#include <libcore/io/writer.hpp>
#include <libcore/str.hpp>
#include <math/range.hpp>

#include "libcore/mem/view.hpp"
#include "libcore/result.hpp"
#include "libcore/type-utils.hpp"
#include "libcore/type/trait.hpp"
namespace fmt
{

// FIXME: rewrite everything here, as it is really bad
typedef struct
{
    bool hex;
    int pad;
    char pad_char;
} FormatIntegerFlags;

template <core::IsIdentityIntegral T, core::Writable Targ>
constexpr core::Result<int> format_v(Targ &target, T &&v, FormatIntegerFlags flags)
{
    (void)flags;

    core::Pure<T> value = v;
    core::Pure<T> digit = value % 10;
    if (value == 0)
    {
        target.write("0", 1);
        return {};
    }

    // With an uint64_t, the max value contains 20 digits
    constexpr int base = 20;
    char buffer[base];
    int i = 0;

    while (value != 0)
    {
        char t = '0' + digit;
        buffer[i] = t;
        i++;
        value /= 10;
        digit = value % 10;
    }
    for (int v = i - 1; v >= 0; v--)
    {
        target.write(&buffer[v], 1);
    }

    return {};
}

template <core::IsIdentityIntegral T, core::Writable Targ>
constexpr core::Result<int> format_v(Targ &target, T &&v)
{
    return format_v(target, v, {false, 0, ' '});
}

template <core::IsIdentityIntegral T, core::Writable Targ>
constexpr core::Result<int> format_v_hex(Targ &target, T &&v, FormatIntegerFlags flags)
{

    core::Pure<T> value = v;
    core::Pure<T> digit = value % 16;

    // With an uint64_t, the max value contains 16 hex digits
    constexpr int base = 17;
    char buffer[base];
    int i = 0;

    while (value != 0)
    {
        if (digit < 10)
        {
            char t = '0' + digit;
            buffer[i] = t;
        }
        else
        {
            char t = 'a' + digit - 10;
            buffer[i] = t;
        }
        i++;
        value /= 16;
        digit = value % 16;
    }

    if (flags.pad > i)
    {
        for (int v = i; v < flags.pad; v++)
        {
            target.write(&flags.pad_char, 1);
        }
    }
    for (int v = i - 1; v >= 0; v--)
    {
        target.write(&buffer[v], 1);
    }
    return {};
}

template <core::IsIdentityIntegral T, core::Writable Targ>
constexpr core::Result<int> format_v_hex(Targ &target, T &&v)
{
    return format_v_hex(target, v, {true, 0, ' '});
}

template <core::IsIdentityIntegral T, core::Writable Targ>
constexpr core::Result<int> format_v(Targ &target, FormatFlags<T> &&v)
{
    if (v.color != Color::NONE)
    {
        target.write("\033[", 2);
        format_v(target, (int)v.color);
        target.write("m", 1);
    }
    if (v.is_hex)
    {
        auto r = format_v_hex(target, v.value, {v.is_hex, v.pad_size, v.pad_char});

        if (v.color != Color::NONE)
        {
            target.write("\033[0m", 4);
        }
        return r;
    }
    else
    {
        auto r = format_v(target, (int)v.value, {v.is_hex, v.pad_size, v.pad_char});
        if (v.color != Color::NONE)
        {
            target.write("\033[0m", 4);
        }
        return r;
    }
}
template <core::IsIdentityIntegral T, core::Writable Targ>
constexpr core::Result<int> format_v(Targ &target, const FormatFlags<T> &v)
{
    if (v.color != Color::NONE)
    {
        target.write("\033[", 2);
        format_v(target, (int)v.color);
        target.write("m", 1);
    }
    if (v.is_hex)
    {
        auto r = format_v_hex(target, v.value, {v.is_hex, v.pad_size, v.pad_char});

        if (v.color != Color::NONE)
        {
            target.write("\033[0m", 4);
        }
        return r;
    }
    else
    {
        auto r = format_v(target, (int)v.value, {v.is_hex, v.pad_size, v.pad_char});
        if (v.color != Color::NONE)
        {
            target.write("\033[0m", 4);
        }
        return r;
    }
}
} // namespace fmt