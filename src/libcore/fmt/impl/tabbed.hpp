#pragma once
#include <libcore/fmt/impl/integers.hpp>

#include "libcore/fmt/flags.hpp"

namespace fmt
{

struct Tabbed
{
    int tab_count;
    constexpr Tabbed(int tab_count) : tab_count(tab_count) {}
};

template <core::Writable Targ>
constexpr core::Result<int> format_v(Targ &target, Tabbed value)
{

    for (int i = 0; i < value.tab_count; i++)
    {
        target.write(core::Str("\t"));
    }
    return {};
}
} // namespace fmt