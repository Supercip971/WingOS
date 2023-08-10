
#include "kernel/generic/paging.hpp"

#include "kernel/generic/mem.hpp"
#include "libcore/fmt/log.hpp"
#include "mcx/mcx.hpp"
core::Result<VmmSpace> VmmSpace::kernel_initialize(const mcx::MachineContext *ctx)
{

#define flags fmt::FMT_HEX | fmt::FMT_CYAN | fmt::FMT_PAD_8BYTES | fmt::FMT_PAD_ZERO

    log::log$("creating space");
    VmmSpace result = try$(VmmSpace::create(true));

    log::log$("mapping space: {}", ctx->_memory_map_count);
    for (int i = 0; i < ctx->_memory_map_count; i++)
    {

        auto const &entry = ctx->_memory_map[i];

        PhysRange entry_phys = entry.range.as<PhysAddr>();

        VirtRange entry_virt = toVirtRange(entry_phys);
        VirtRange entry_kernel = toKernelRange(entry_phys);

        log::log$("- mapping[{}]: ({}-{}) -> ({}-{})", i, entry_phys.start()._addr | flags, entry_phys.end()._addr | flags,
                  entry_virt.start()._addr | flags, entry_virt.end()._addr | flags);
        try$(result.map(entry_virt, entry_phys,
                        PageFlags().present(true).writeable(true).user(false)));

        log::log$("ok");
        if (entry.type == mcx::MemoryMap::Type::KERNEL_AND_MODULES)
        {
            log::log$("- mapping[{}]: ({}-{}) -> ({}-{})", i, entry_phys.start()._addr | flags, entry_phys.end()._addr | flags,
                      entry_kernel.start()._addr | flags, entry_kernel.end()._addr | flags);
            try$(result.map(entry_kernel, entry_phys,
                            PageFlags().present(true).writeable(true).user(false)));
        }
    }
#undef flags

    return result;
}