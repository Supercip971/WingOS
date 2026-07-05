#pragma once
#include <libcore/fmt/impl/integers.hpp>

#include "libcore/fmt/flags.hpp"
#include "libcore/io/writer.hpp"
#include "libcore/result.hpp"
#include "libcore/str.hpp"
#include "math/range.hpp"

namespace fmt
{
template <math::IntRangeable C, fc::Writable Targ>
constexpr fc::Result<int> format_v(Targ &target, C &&value)
{
    target.write(fc::Str("{ "));
    format_v(target, value.start());
    target.write(fc::Str(" - "));
    format_v(target, value.end());
    target.write(fc::Str(" }"));
    return 0;
}

template <math::IntRangeable C, fc::Writable Targ>
constexpr fc::Result<int> format_v(Targ &target, fmt::FormatFlags<C> range)
{
    target.write(fc::Str("{ "));
    format_v(target, range.forward_flags(range.value.start()));
    target.write(fc::Str(" - "));
    format_v(target, range.forward_flags(range.value.end()));
    target.write(fc::Str(" }"));
    return 0;
}

} // namespace fmt
