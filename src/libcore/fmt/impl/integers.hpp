
#pragma once
#include <libcore/fmt/flags.hpp>
#include <libcore/io/writer.hpp>
#include <libcore/str.hpp>
#include <math/range.hpp>

#include "libcore/buffer.hpp"
#include "libcore/result.hpp"
#include "libcore/type-utils.hpp"
#include "libcore/type/trait.hpp"
namespace fmt
{

template <core::IsIdentityIntegral T, core::Writable Targ>
constexpr core::Result<int> format_v(Targ &target, T &&v)
{

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
constexpr core::Result<int> format_v_hex(Targ &target, T &&v)
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

    for (int v = i - 1; v >= 0; v--)
    {
        target.write(&buffer[v], 1);
    }
    return {};
}

template <core::IsIdentityIntegral T, core::Writable Targ>
constexpr core::Result<int> format_v(Targ &target, FormatFlags<T> &&v)
{

    if (v.is_hex)
    {
        return format_v_hex(target, v.value);
    }
    else
    {
        return format_v(target, v.value);
    }
}

} // namespace fmt