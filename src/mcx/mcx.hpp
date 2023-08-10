#pragma once
#include <libcore/ds/array.hpp>
#include <libcore/fmt/fmt.hpp>
#include <math/range.hpp>
#include <stdint.h>

#include "libcore/fmt/flags.hpp"

namespace mcx
{

// for 32 bit systems, the high 32 bits are always 0
using MemoryRange = math::Range<uint64_t>;

struct MemoryMap
{
    MemoryRange range;

    enum class Type
    {
        FREE = 0,
        RESERVED = 1,
        ACPI_RECLAIMABLE = 2,
        ACPI_NVS = 3,
        BAD_MEMORY = 4,
        BOOTLOADER_RECLAIMABLE = 5,
        KERNEL_AND_MODULES = 6,
        FRAMEBUFFER = 7,
    };
    MemoryMap::Type type;
};

using MemoryMapIdx = int;

struct MachineFramebuffer
{
    void *address;
    int width;
    int height;
    int pitch;
    int bpp;

    enum class PixelFormat
    {
        RGB,
        BGR,
    };

    PixelFormat pixel_format;
};
struct MachineContext
{

public:
    constexpr static int max_memory_map = 64;
    core::Array<MemoryMap, max_memory_map> _memory_map;
    int _memory_map_count = 0;

    MachineFramebuffer _framebuffer;

    uintptr_t _rsdp;
};

template <core::IsSame<const MachineContext *> T, core::Writable Targ>
constexpr core::Result<void> format_v(Targ &target, T value)
{
    fmt::format(target, "\n MachineContext [ \n");
    fmt::format(target, "  _memory_map_count: {} \n", value->_memory_map_count);
    fmt::format(target, "  _memory_map: \n");
    for (int i = 0; i < value->_memory_map_count; i++)
    {
        fmt::format(target, "   - range: {} \n", value->_memory_map[i].range | fmt::FMT_HEX);
        fmt::format(target, "     type: {} \n", (int)value->_memory_map[i].type);
    }
    fmt::format(target, "  _rsdp: {} \n", value->_rsdp);
    fmt::format(target, "] \n");
    return {};
}

} // namespace mcx
