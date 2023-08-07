#pragma once

#include <libcore/ds/bitmap.hpp>
#include <libcore/fmt/log.hpp>
#include <mcx/mcx.hpp>
#include <stdint.h>

#include "libcore/fmt/impl/bitmap.hpp"
#include "libcore/mem/mem.hpp"
#include "libcore/type-utils.hpp"
#include "libcore/type/trait.hpp"
#include "math/align.hpp"
#include "math/range.hpp"
#include "mem.hpp"
struct PmmSection
{
    math::Range<uintptr_t> range;
    core::Bitmap bitmap;
};

struct Pmm
{
    static constexpr size_t page_size_bit = 4096;
    static constexpr size_t page_size_byte = page_size_bit * 8;

    mcx::MemoryRange range;
    PmmSection *sections;
    size_t sections_count;

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

    static core::Result<Pmm> create(const mcx::MachineContext *context)
    {
        size_t size = pmm_size(context);
        Pmm pmm;
        mcx::MemoryRange pmm_memory_origin;
        bool found = false;
        size_t used_section = 0;

        // find a region to place the pmm structure
        for (int i = 0; i < context->_memory_map_count; i++)
        {
            auto aligned_range = context->_memory_map[i].range.shrinkAlign(page_size_byte);
            if (math::alignDown(pmm.sections[i].range.len(), page_size_byte) == 0)
            {
                continue;
            }
            if (context->_memory_map[i].type == mcx::MemoryMap::Type::FREE && aligned_range.len() >= size)
            {
                used_section = i;
                pmm_memory_origin = aligned_range;
                pmm_memory_origin.len(size);
                found = true;
                break;
            }
        }

        if (!found)
        {
            return ("Could not find a suitable memory range for the pmm");
        }

        pmm.sections = toVirt(pmm_memory_origin.start()).as<PmmSection>();
        pmm.sections_count = 0;
        pmm.range = pmm_memory_origin;

        PhysAddr bitmaps_start = pmm_memory_origin.start() + sizeof(PmmSection) * pmm_section_count(context);

        size_t sec = 0;
        for (int i = 0; i < context->_memory_map_count; i++)
        {
            if (context->_memory_map[i].type == mcx::MemoryMap::Type::FREE)
            {

                log::log$("Pmm: section {} range: {}", sec, context->_memory_map[i].range | fmt::FMT_HEX);

                if (math::alignDown(context->_memory_map[i].range.len(), page_size_byte) == 0)
                {
                    log::log$("skipped: too small");
                    continue;
                }

                auto &section = pmm.sections[sec];
                auto range = context->_memory_map[i].range.shrinkAlign(page_size_byte);
                section.range = range;

                core::MemAccess<uint8_t> access(toVirt(bitmaps_start).as<uint8_t>(), section.range.len() / page_size_byte);

                section.bitmap = core::Bitmap(core::move(access));
                section.bitmap.fill(false);

                if (used_section == (size_t)i)
                {
                    // as the bitmap is represented using bytes and is offsetted by the start of the bitmap
                    // we need to offset the range by the start of the bitmap and divide by the page size
                    auto in_bitmap_range = pmm_memory_origin.offsettedSub(range.start())
                                               .div(page_size_bit);
                    section.bitmap.fill(true, in_bitmap_range);
                }

                bitmaps_start += section.range.len() / page_size_byte;
                sec++;
            }
        }

        pmm.sections_count = sec;

        return pmm;
    };
};

template <core::IsConvertibleTo<Pmm> T, core::Writable Targ>
constexpr core::Result<void> format_v(Targ &target, T &value)
{

    fmt::format(target, "Pmm [ \n");
    fmt::format(target, " - range: {} \n", value.range | fmt::FMT_HEX);
    fmt::format(target, " - sections_count: {} \n", value.sections_count);
    fmt::format(target, " - sections: \n");
    for (size_t i = 0; i < value.sections_count; i++)
    {
        fmt::format(target, " - section {} \n", i);
        fmt::format(target, "  - range: {} ({}) \n", value.sections[i].range | fmt::FMT_HEX, value.sections[i].range.len());
        fmt::format(target, "  - bitmap: \n");

        format_v(target, (core::Bitmap &)value.sections[i].bitmap);
    }
    return {};
}
