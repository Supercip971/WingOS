#pragma once 

#include <libcore/io/writer.hpp>
#include <libcore/result.hpp>
#include <libcore/str_writer.hpp>
#include <libcore/fmt/fmt.hpp>
namespace fmt 
{
    template<typename Fmt, typename... Args>
    constexpr core::Result<core::WStr> format_str( Fmt fmt, Args... args)
    {
        core::WStr writer;
        fmt::format(writer, core::Str(fmt), core::forward<Args>(args)...);
        return writer;
    }
    template<typename Fmt, typename... Args>
    constexpr core::Result<void> format_to_str(core::WStr& writer,  Fmt fmt, Args... args)
    {
        fmt::format(writer, core::Str(fmt), core::forward<Args>(args)...);
        return {};
    }
}