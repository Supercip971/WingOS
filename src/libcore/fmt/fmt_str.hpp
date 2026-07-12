#pragma once

#include <libcore/fmt/fmt.hpp>
#include <libcore/result.hpp>

#include <libcore/str_writer.hpp>

namespace fmt
{
template <typename Fmt, typename... Args>
constexpr fc::Result<fc::WStr> format_str(Fmt fmt, Args... args)
{
    fc::WStr writer;
    fmt::format(writer, fc::Str(fmt), std::forward<Args>(args)...);
    return writer;
}

template <typename Fmt, typename... Args>
constexpr fc::Result<void> format_to_str(fc::WStr &writer, Fmt fmt, Args... args)
{
    fmt::format(writer, fc::Str(fmt), std::forward<Args>(args)...);
    return {};
}
} // namespace fmt
