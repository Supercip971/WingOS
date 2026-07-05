#pragma once

#include "libcore/io/writer.hpp"
#include "libcore/result.hpp"
#include "libcore/str.hpp"

namespace fmt
{

struct Tabbed
{
    int tab_count;

    constexpr Tabbed(int _tab_count) : tab_count(_tab_count) {}
};

template <fc::Writable Targ>
constexpr fc::Result<int> format_v(Targ &target, Tabbed value)
{

    for (int i = 0; i < value.tab_count; i++)
    {
        target.write(fc::Str("\t"));
    }
    return {};
}
} // namespace fmt
