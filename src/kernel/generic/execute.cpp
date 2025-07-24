

#include "execute.hpp"
#include <string.h>

#include "hw/mem/addr_space.hpp"

#include "kernel/generic/asset.hpp"
#include "kernel/generic/pmm.hpp"
#include "kernel/generic/scheduler.hpp"
#include "kernel/generic/space.hpp"
#include "kernel/generic/task.hpp"
#include "libcore/alloc/alloc.hpp"
#include "libcore/enum-op.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/type/trait.hpp"
#include "libelf/elf.hpp"
#include "math/align.hpp"

core::Result<void> start_module_execution(elf::ElfLoader loaded)
{

    Asset* asset = space_create(nullptr, 0, 0);

    Space* root_space = asset->space;



    auto task_asset = asset_create_task(root_space, {
        .launch = {
            .entry = (void *)loaded.entry_point(),
            .user = true,
        },
    });

    auto task = task_asset->task;


    for (size_t i = 0; i < loaded.program_count(); i++)
    {
        auto ph = try$(loaded.program_header(i));
        if (ph.type != core::underlying_value(ElfProgramHeaderType::HEADER_LOAD))
        {
            log::warn$("skipping program header {}: type is not LOAD but {}", i, ph.type);
            continue;
        }
        log::log$("section[{}]: type: {}, flags: {}, virt_addr: {}, file_offset: {}, file_size: {}",
                  i, ph.type, ph.flags, ph.virt_addr, ph.file_offset, ph.file_size);
        

        size_t page_count = math::alignUp(ph.mem_size, 4096ul) / 4096;
        size_t mem_size = page_count * 4096;
        if (page_count == 0)
        {
            log::warn$("skipping program header {}: page count is 0", i);
            continue;
        }
        auto mem_asset = asset_create_memory(root_space, {
            .size = mem_size,
        });

        if (mem_asset == nullptr)
        {
            log::err$("unable to create memory asset for program header {}", i);
            return "unable to create memory asset";
        }

        auto mem = mem_asset->memory;

        void *copied_data = (void *)toVirt(mem.addr);

        memset(copied_data, 0, ph.mem_size);
        memcpy(copied_data,
               (void *)((uintptr_t)loaded.range().start() + ph.file_offset),
               ph.file_size);

        asset_create_mapping(root_space, (AssetMappingCreateParams){
            .start = ph.virt_addr,
            .end = ph.virt_addr + mem_size,
            .physical_mem = mem_asset,
            .writable = true,
            .executable = true,
        });
    }

    log::log$("module loaded: task uid: {}", task->uid());

    try$(kernel::task_run(task->uid()));


    

    return {};
}
