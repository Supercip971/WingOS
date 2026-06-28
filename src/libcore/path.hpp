#pragma once

#include "libcore/str_writer.hpp"

namespace core
{

using Path = core::Str;

static inline core::WStr finalizePath(const Path &path)
{

    if (!path.start_with("/"))
    {
        return core::WStr::copy(path);
    }

#ifdef __ck_host__

    auto res = core::WStr::copy(".");
    res.append(path);
    return res;

#else
    auto res = core::WStr::copy(path);
    return res;
#endif
}
} // namespace core
