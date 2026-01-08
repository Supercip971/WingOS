

#include "execute.hpp"
#include <string.h>

#include "hw/mem/addr_space.hpp"

#include "kernel/generic/asset.hpp"
#include "kernel/generic/kernel.hpp"
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
#include "mcx/mcx.hpp"
#include "wingos-headers/startup.hpp"

core::Result<void> start_module_execution(elf::ElfLoader loaded, mcx::MachineContext const *context)
{

    AssetRef<Space> ptr = (Space::create_root());
    Space *root_space = ptr.asset;
    void *context_mapped = nullptr;
    // copy machine context to task
    if (context)
    {
        auto mem_asset_res = root_space->create_memory({
            .size = math::alignUp(sizeof(StartupInfo), 4096ul),
        });
        if (mem_asset_res.is_error())
        {
            log::err$("unable to create memory asset for machine context: {}", mem_asset_res.error());

            return mem_asset_res.error();
        }

        auto mem_asset_ptr = mem_asset_res.unwrap();
        auto mem_asset = mem_asset_ptr.asset;
        void *copied_data = (void *)toVirt(mem_asset->addr);
        memset(copied_data, 0, sizeof(StartupInfo));
        memcpy(copied_data, context, sizeof(mcx::MachineContext));

        if (root_space->create_mapping((AssetMappingCreateParams){
                                           .start = mem_asset->addr,
                                           .end = math::alignUp(mem_asset->addr + sizeof(StartupInfo), 4096ul),
                                           .physical_mem = mem_asset_ptr,
                                           .writable = true,
                                       })
                .is_error())
        {
            log::err$("unable to create mapping for machine context: {}", mem_asset_res.error());
            return mem_asset_res.error();
        }
        context_mapped = (void *)mem_asset->addr;
    }

    auto task_asset_res = root_space->create_task(
        {
            .launch = {
                .entry = (void *)loaded.entry_point(),
                .args = {
                    (uintptr_t)context_mapped},
                .user = true,

            },
        });

    if (task_asset_res.is_error())
    {
        log::err$("unable to create task asset: {}", task_asset_res.error());
        return task_asset_res.error();
    }

    auto task_asset_ptr = task_asset_res.unwrap();
    auto task_asset = task_asset_ptr.asset;
    auto task = task_asset->task;

    for (size_t i = 0; i < loaded.program_count(); i++)
    {
        auto ph = try$(loaded.program_header(i));
        if (ph.type != core::underlying_value(ElfProgramHeaderType::HEADER_LOAD))
        {
            auto type_val = ph.type;
            log::warn$("skipping program header {}: type is not LOAD but {}", i, type_val);
            continue;
        }
        auto type_val = ph.type;
        auto flags_val = ph.flags;
        auto virt_addr_val = ph.virt_addr;
        auto file_offset_val = ph.file_offset;
        auto file_size_val = ph.file_size;
        log::log$("section[{}]: type: {}, flags: {}, virt_addr: {}, file_offset: {}, file_size: {}",
                  i, type_val, flags_val, virt_addr_val, file_offset_val, file_size_val);

        size_t page_count = math::alignUp(ph.mem_size, 4096ul) / 4096;
        size_t mem_size = page_count * 4096;
        if (page_count == 0)
        {
            log::warn$("skipping program header {}: page count is 0", i);
            continue;
        }
        auto mem_asset_res = root_space->create_memory({
            .size = mem_size,
        });

        if (mem_asset_res.is_error())
        {
            log::err$("unable to create memory asset for program header {}: {}", i, mem_asset_res.error());

            return mem_asset_res.error();
        }

        auto mem_asset_ptr = mem_asset_res.unwrap();
        auto mem_asset = mem_asset_ptr.asset;

        void *copied_data = (void *)toVirt(mem_asset->addr);

        memset(copied_data, 0, ph.mem_size);
        memcpy(copied_data,
               (void *)((uintptr_t)loaded.range().start() + ph.file_offset),
               ph.file_size);

        if (root_space->create_mapping((AssetMappingCreateParams){
                                           .start = ph.virt_addr,
                                           .end = ph.virt_addr + mem_size,
                                           .physical_mem = mem_asset_ptr,
                                           .writable = true,
                                           .executable = true,
                                       })
                .is_error())
        {
            log::err$("unable to create mapping for program header {}: {}", i, mem_asset_res.error());
            return mem_asset_res.error();
        }
    }

    log::log$("module loaded: task uid: {}", task->uid());

    try$(kernel::task_run(task->uid()));

    return {};
}
