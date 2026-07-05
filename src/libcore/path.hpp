#pragma once

#include "libcore/str_writer.hpp"

#include "libcore/str.hpp"

namespace fc
{

using Path = fc::Str;

static inline fc::WStr finalizePath(const Path &path)
{

    if (!path.start_with("/"))
    {
        return fc::WStr::copy(path);
    }

#ifdef __ck_host__

    auto res = fc::WStr::copy(".");
    res.append(path);
    return res;

#else
    auto res = fc::WStr::copy(path);
    return res;
#endif
}
} // namespace fc
