#include <string.h>

#include "module_startup.hpp"

#include "dev/pci/classes.hpp"
#include "dev/pci/pci.hpp"
#include "iol/wingos/space.hpp"
#include "json/json.hpp"
#include "libcore/fmt/flags.hpp"
#include "libcore/fmt/log.hpp"
#include "libelf/elf.hpp"
#include "mcx/mcx.hpp"
#include "wingos-headers/asset.h"

core::Result<size_t> execute_module(elf::ElfLoader loaded)
{

    auto subspace = Wingos::Space::self().create_space();

    auto task_asset = subspace.create_task((uintptr_t)loaded.entry_point());

    if (task_asset.handle == 0)
    {
        log::err$("failed to create task asset: {}", task_asset.handle);
        return core::Result<size_t>::error("failed to create task asset");
    }

    for (size_t i = 0; i < loaded.program_count(); i++)
    {

        auto ph = try$(loaded.program_header(i));
        if (ph.type != core::underlying_value(ElfProgramHeaderType::HEADER_LOAD))
        {
            log::warn$("skipping program header {}: type is not LOAD but {}", i, ph.type);
            continue;
        }

        auto memory = Wingos::Space::self().allocate_physical_memory(ph.mem_size, false);

        auto mapped_self = Wingos::Space::self().map_memory(memory, ASSET_MAPPING_FLAG_WRITE | ASSET_MAPPING_FLAG_EXECUTE);

        void *copied_data = (void *)((uintptr_t)mapped_self.ptr());
        memset(copied_data, 0, ph.mem_size);
        memcpy(copied_data,
               (void *)((uintptr_t)loaded.range().start() + ph.file_offset),
               ph.file_size);

        auto moved_memory = Wingos::Space::self().move_to(subspace, memory);

        subspace.map_memory(ph.virt_addr, ph.virt_addr + ph.mem_size, moved_memory, ASSET_MAPPING_FLAG_WRITE | ASSET_MAPPING_FLAG_EXECUTE);
    }

    subspace.launch_task(task_asset);

    return 0ul;
}