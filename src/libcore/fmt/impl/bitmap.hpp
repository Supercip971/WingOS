#pragma once
#include <libcore/ds/bitmap.hpp>
#include <libcore/fmt/fmt.hpp>
#include <libcore/fmt/impl/integers.hpp>

#include "libcore/fmt/flags.hpp"
#include "libcore/type/trait.hpp"

#define flags fmt::FMT_HEX | fmt::FMT_CYAN | fmt::FMT_PAD_8BYTES | fmt::FMT_PAD_ZERO
template <core::IsConvertibleTo<core::Bitmap> T, core::Writable Targ>
constexpr core::Result<void> format_v(Targ &target, T &value)
{

    bool last = value.bit(0);
    size_t begin = 0;
    size_t end = 0;

    fmt::format(target, "Bitmap[{}]:\n", value.len() | flags);

    for (size_t i = 0; i < value.len(); i++)
    {
        if (last != value.bit(i))
        {
            end = i;
            fmt::format(target, " - [{};{}] -> {}\n", begin | flags, end | flags, last ? "true" : "false");
            begin = i;
            last = value.bit(i);
        }
    }
    fmt::format(target, " - [{};{}] -> {}\n", begin | flags, value.len() | flags, last ? "true" : "false");

    return {};
} // namespace fmt
#undef flags