#include <string.h>
#include "hw/mem/addr_space.hpp"

#include "dev/pci/classes.hpp"
#include "dev/pci/pci.hpp"
#include "iol/wingos/space.hpp"
#include "json/json.hpp"
#include "libcore/fmt/flags.hpp"
#include "libcore/fmt/log.hpp"
#include "libelf/elf.hpp"
#include "mcx/mcx.hpp"
#include "execute.hpp"
#include "protocols/vfs/vfs.hpp"
#include "wingos-headers/asset.h"
#include "wingos-headers/startup.hpp"


core::Result<size_t> execute_program_from_mem(Wingos::Space &subspace, elf::ElfLoader loaded, StartupInfo const & args)
{
    auto startup_info_mem = Wingos::Space::self().allocate_physical_memory(sizeof(StartupInfo));

    auto startup_info_mapped = Wingos::Space::self().map_memory(startup_info_mem, ASSET_MAPPING_FLAG_WRITE | ASSET_MAPPING_FLAG_READ);

    StartupInfo *ptr = (StartupInfo *)startup_info_mapped.ptr();

    memcpy(ptr, &args, sizeof(StartupInfo));

    auto moved_startup_info = Wingos::Space::self().move_to(subspace, startup_info_mem);

    auto vasset = subspace.map_memory(moved_startup_info, ASSET_MAPPING_FLAG_WRITE | ASSET_MAPPING_FLAG_READ);
    auto task_asset = subspace.create_task((uintptr_t)loaded.entry_point(), (uintptr_t)vasset.ptr());

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
            log::warn$("skipping program header {}: type is not LOAD but {}", i, (uint32_t)ph.type);
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

core::Result<size_t> execute_program_from_path(Wingos::Space& subspace, const core::Str & path, StartupInfo const & args)
{
    auto file_asset = prot::VfsConnection::connect().unwrap().open_path(path).unwrap();

    auto file_size = file_asset.get_info().unwrap().size;
    auto mem_size = math::alignUp(file_size, 4096ul);

    auto data_asset = Wingos::Space::self().allocate_physical_memory(mem_size);
    size_t read_bytes = try$(file_asset.read(data_asset, 0, file_size)); 
    
    auto file_mapped = Wingos::Space::self().map_memory(data_asset, ASSET_MAPPING_FLAG_READ);

    auto range = VirtRange((uintptr_t)file_mapped.ptr(), (uintptr_t)file_mapped.ptr() + read_bytes);
    elf::ElfLoader prog = try$(elf::ElfLoader::load(range)); 

    return execute_program_from_mem(subspace, core::move(prog), args);
    
}
