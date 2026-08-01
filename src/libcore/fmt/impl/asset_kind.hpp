#pragma once
#include <libcore/fmt/impl/integers.hpp>
#include <wingos-headers/asset.h>

#include "libcore/fmt/flags.hpp"
#include "libcore/io/writer.hpp"
#include "libcore/result.hpp"
#include "libcore/str.hpp"
#include "libcore/type/trait.hpp"

namespace fmt
{
template <fc::IsConvertibleTo<fc::Str> T, fc::Writable Targ>
constexpr fc::Result<void> format_v(Targ &target, T &&value);

template <typename C, fc::Writable Targ>
constexpr fc::Result<void> format_v(Targ &target, C &&value)
    requires(fc::IsSame<AssetKind, fc::RemoveReference<C>>)
{
    switch (value)
    {
    case OBJECT_KIND_UNKNOWN:
        return format_v(target, fc::Str("OBJECT_KIND_UNKNOWN"));
    case OBJECT_KIND_MEMORY:
        return format_v(target, fc::Str("OBJECT_KIND_MEMORY"));
    case OBJECT_KIND_MAPPING:
        return format_v(target, fc::Str("OBJECT_KIND_MAPPING"));
    case OBJECT_KIND_SPACE:
        return format_v(target, fc::Str("OBJECT_KIND_SPACE"));
    case OBJECT_KIND_TASK:
        return format_v(target, fc::Str("OBJECT_KIND_TASK"));
    case OBJECT_KIND_IPC_ENDPOINT:
        return format_v(target, fc::Str("OBJECT_KIND_IPC_ENDPOINT"));
    case OBJECT_KIND_IPC_CONNECTION:
        return format_v(target, fc::Str("OBJECT_KIND_IPC_CONNECTION"));
    default:
        return format_v(target, fc::Str("UNKNOWN_ASSET_KIND"));
    }
}

template <typename C, fc::Writable Targ>
constexpr fc::Result<void> format_v(Targ &target, fmt::FormatFlags<C> flagged)
    requires(fc::IsSame<AssetKind, fc::RemoveReference<C>>)
{
    switch (flagged.value)
    {
    case OBJECT_KIND_UNKNOWN:
        return format_v(target, flagged.forward_flags(fc::Str("OBJECT_KIND_UNKNOWN")));
    case OBJECT_KIND_MEMORY:
        return format_v(target, flagged.forward_flags(fc::Str("OBJECT_KIND_MEMORY")));
    case OBJECT_KIND_MAPPING:
        return format_v(target, flagged.forward_flags(fc::Str("OBJECT_KIND_MAPPING")));
    case OBJECT_KIND_SPACE:
        return format_v(target, flagged.forward_flags(fc::Str("OBJECT_KIND_SPACE")));
    case OBJECT_KIND_TASK:
        return format_v(target, flagged.forward_flags(fc::Str("OBJECT_KIND_TASK")));
    case OBJECT_KIND_IPC_ENDPOINT:
        return format_v(target, flagged.forward_flags(fc::Str("OBJECT_KIND_IPC_ENDPOINT")));
    case OBJECT_KIND_IPC_CONNECTION:
        return format_v(target, flagged.forward_flags(fc::Str("OBJECT_KIND_IPC_CONNECTION")));
    default:
        return format_v(target, flagged.forward_flags(fc::Str("UNKNOWN_ASSET_KIND")));
    }
}

} // namespace fmt
