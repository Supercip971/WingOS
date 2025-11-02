#pragma once
#include <libcore/fmt/impl/integers.hpp>


#include "libcore/fmt/flags.hpp"

#include <wingos-headers/asset.h>

namespace fmt
{
template <core::IsConvertibleTo<core::Str> T, core::Writable Targ>
constexpr core::Result<void> format_v(Targ &target, T &&value);


template <typename C, core::Writable Targ>
constexpr core::Result<void> format_v(Targ &target, C &&value) requires (core::IsSame<AssetKind, core::RemoveReference<C>>)
{
    switch(value)
    {
    case OBJECT_KIND_UNKNOWN:
        return format_v(target, core::Str("OBJECT_KIND_UNKNOWN"));
    case OBJECT_KIND_MEMORY:
        return format_v(target, core::Str("OBJECT_KIND_MEMORY"));
    case OBJECT_KIND_MAPPING:
        return format_v(target, core::Str("OBJECT_KIND_MAPPING"));
    case OBJECT_KIND_SPACE:
        return format_v(target, core::Str("OBJECT_KIND_SPACE"));
    case OBJECT_KIND_TASK:
        return format_v(target, core::Str("OBJECT_KIND_TASK"));
    case OBJECT_KIND_IPC_SERVER:
        return format_v(target, core::Str("OBJECT_KIND_IPC_SERVER"));
    case OBJECT_KIND_IPC_CONNECTION:
        return format_v(target, core::Str("OBJECT_KIND_IPC_CONNECTION"));
    default:
        return format_v(target, core::Str("UNKNOWN_ASSET_KIND"));
    }
}
template <typename C,  core::Writable Targ>
constexpr core::Result<void> format_v(Targ &target, fmt::FormatFlags<C> flagged) requires (core::IsSame<AssetKind, core::RemoveReference<C>>)
{  
    switch(flagged.value)
    {
    case OBJECT_KIND_UNKNOWN:
        return format_v(target, flagged.forward_flags(core::Str("OBJECT_KIND_UNKNOWN")));
    case OBJECT_KIND_MEMORY:
        return format_v(target, flagged.forward_flags(core::Str("OBJECT_KIND_MEMORY")));
    case OBJECT_KIND_MAPPING:
        return format_v(target, flagged.forward_flags(core::Str("OBJECT_KIND_MAPPING")));
    case OBJECT_KIND_SPACE:
        return format_v(target, flagged.forward_flags(core::Str("OBJECT_KIND_SPACE")));
    case OBJECT_KIND_TASK:
        return format_v(target, flagged.forward_flags(core::Str("OBJECT_KIND_TASK")));
    case OBJECT_KIND_IPC_SERVER:
        return format_v(target, flagged.forward_flags(core::Str("OBJECT_KIND_IPC_SERVER")));
    case OBJECT_KIND_IPC_CONNECTION:
        return format_v(target, flagged.forward_flags(core::Str("OBJECT_KIND_IPC_CONNECTION")));
    default:
        return format_v(target, flagged.forward_flags(core::Str("UNKNOWN_ASSET_KIND")));
    }
   
}

}