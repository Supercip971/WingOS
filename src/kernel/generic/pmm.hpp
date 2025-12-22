#pragma once

#include <libcore/ds/bitmap.hpp>
#include <libcore/fmt/log.hpp>
#include <mcx/mcx.hpp>
#include <stdint.h>

#include "hw/mem/addr_space.hpp"
#include <iol/mem_flags.h>

#include "libcore/fmt/impl/bitmap.hpp"
#include "libcore/mem/mem.hpp"
#include "libcore/type-utils.hpp"
#include "libcore/type/trait.hpp"
#include "math/align.hpp"
#include "math/range.hpp"
#include "mem.hpp"
#include "libcore/lock/lock.hpp"
struct PmmSection
{
    math::Range<uintptr_t> range;
    core::Bitmap bitmap;
};

struct Pmm
{
    static constexpr size_t page_size_bit = 4096;
    static constexpr size_t page_size_byte = page_size_bit * 8;

    mcx::MemoryRange _range;
    PmmSection *_sections;
    core::Lock pmm_lock;
    mcx::MemoryMapIdx _section_location;
    size_t _sections_count;
    const mcx::MachineContext *_context;

    static size_t pmm_size(const mcx::MachineContext *context)
    {
        size_t size = 0;
        for (int i = 0; i < context->_memory_map_count; i++)
        {
            if (context->_memory_map[i].type == mcx::MemoryMap::Type::FREE)
            {
                size += math::alignUp(context->_memory_map[i].range.len(), page_size_byte) / (page_size_byte);
                size += sizeof(PmmSection);
            }
        }

        return math::alignUp(size, page_size_byte);
    }
    static size_t pmm_section_count(const mcx::MachineContext *context)
    {
        return core::count(context->_memory_map.sub(context->_memory_map_count),
                           [](auto v)
                           {
                               return v.type == mcx::MemoryMap::Type::FREE;
                           });
    }

    static Pmm &the();


    static core::Result<void> initialize(const mcx::MachineContext *context);

    static core::Result<Pmm> _allocate_structure(const mcx::MachineContext *context);

    core::Result<void> _fill(const mcx::MachineContext *context);

    static core::Result<Pmm> create(const mcx::MachineContext *context);

    core::Result<PhysAddr> allocate(size_t count, IolAllocMemoryFlag flags = IOL_ALLOC_MEMORY_FLAG_NONE);

    core::Result<void> own(PhysAddr addr, size_t count);
    core::Result<void> release(PhysAddr addr, size_t count);


    core::Result<bool> query_usage(PhysAddr addr);

};

template <core::IsConvertibleTo<Pmm> T, core::Writable Targ>
constexpr core::Result<void> format_v(Targ &target, T &value)
{

    fmt::format(target, "Pmm [ \n");
    fmt::format(target, " - range: {} \n", value._range | fmt::FMT_HEX);
    fmt::format(target, " - sections_count: {} \n", value._sections_count);
    fmt::format(target, " - sections: \n");
    for (size_t i = 0; i < value._sections_count; i++)
    {
        fmt::format(target, " - section {} \n", i);
        fmt::format(target, "  - range: {} ({}) \n", value._sections[i].range | fmt::FMT_HEX, value._sections[i].range.len());
        fmt::format(target, "  - bitmap: \n");

        format_v(target, (core::Bitmap &)value._sections[i].bitmap);
    }
    return {};
}
