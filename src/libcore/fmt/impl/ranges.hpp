#pragma once
#include <libcore/fmt/impl/integers.hpp>

#include "libcore/fmt/flags.hpp"

namespace fmt
{
template <math::IntRangeable C, core::Writable Targ>
constexpr core::Result<int> format_v(Targ &target, C &&value)
{
    target.write(core::Str("{ "));
    format_v(target, value.start());
    target.write(core::Str(" - "));
    format_v(target, value.end());
    target.write(core::Str(" }"));
    return {};
}
template <math::IntRangeable C, core::Writable Targ>
constexpr core::Result<int> format_v(Targ &target, fmt::FormatFlags<C> range)
{
    target.write(core::Str("{ "));
    format_v(target, range.forward_flags(range.value.start()));
    target.write(core::Str(" - "));
    format_v(target, range.forward_flags(range.value.end()));
    target.write(core::Str(" }"));
    return {};
}

} // namespace fmt