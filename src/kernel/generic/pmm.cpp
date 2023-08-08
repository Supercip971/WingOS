#include "pmm.hpp"
#include <libcore/fmt/log.hpp>

#include "kernel/generic/mem.hpp"
#include "mcx/mcx.hpp"
static Pmm instance;
Pmm &Pmm::the()
{
    return instance;
}
core::Result<void> Pmm::initialize(const mcx::MachineContext *context)
{
    instance = try$(Pmm::create(context));
    return {};
}

core::Result<Pmm> Pmm::_allocate(const mcx::MachineContext *context)
{
    size_t const size = pmm_size(context);
    Pmm pmm;
    mcx::MemoryRange pmm_memory_origin;
    bool found = false;
    mcx::MemoryMapIdx used_section = 0;

    // find a region to place the pmm structure
    for (int i = 0; i < context->_memory_map_count; i++)
    {
        auto aligned_range = context->_memory_map[i].range.shrinkAlign(page_size_byte);

        if (math::alignDown(context->_memory_map[i].range.len(), page_size_byte) == 0)
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

    pmm._sections = toVirt(pmm_memory_origin.start()).as<PmmSection>();
    pmm._sections_count = 0;
    pmm._range = pmm_memory_origin;
    pmm._section_location = used_section;

    log::log$("Pmm: allocated at {}", pmm_memory_origin | fmt::FMT_HEX);

    return pmm;
}

core::Result<void> Pmm::_fill(const mcx::MachineContext *context)
{
    PhysAddr bitmaps_start = _range.start() + sizeof(PmmSection) * pmm_section_count(context);

    size_t sec_id = 0;
    for (int i = 0; i < context->_memory_map_count; i++)
    {
        if (context->_memory_map[i].type != mcx::MemoryMap::Type::FREE)
        {
            continue;
        }

        log::log$("Pmm: section {} range: {}", sec_id, context->_memory_map[i].range | fmt::FMT_HEX);

        if (math::alignDown(context->_memory_map[i].range.len(), page_size_byte) == 0)
        {
            log::log$("skipped: too small");
            continue;
        }

        auto &section = _sections[sec_id];
        auto entry_range = context->_memory_map[i].range.shrinkAlign(page_size_byte);
        section.range = entry_range;

        core::MemAccess<uint8_t> bitmap_mem(
            toVirt(bitmaps_start).as<uint8_t>(),
            section.range.len() / page_size_byte);

        section.bitmap = core::Bitmap(core::move(bitmap_mem));
        section.bitmap.fill(false);

        if (_section_location == i)
        {
            // as the bitmap is represented using bytes and is offsetted by the start of the bitmap
            // we need to offset the range by the start of the bitmap and divide by the page size
            auto in_bitmap_range = _range.offsettedSub(entry_range.start())
                                       .div(page_size_bit);
            section.bitmap.fill(true, in_bitmap_range);
        }

        bitmaps_start += section.range.len() / page_size_byte;
        sec_id++;
    }

    _sections_count = sec_id;

    return {};
}

core::Result<Pmm> Pmm::create(const mcx::MachineContext *context)
{
    Pmm pmm = try$(_allocate(context));

    try$(pmm._fill(context));

    return pmm;
}