

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
#include "mcx/mcx.hpp"

core::Result<void> start_module_execution(elf::ElfLoader loaded, mcx::MachineContext const *context)
{

    AssetPtr ptr = try$(space_create(nullptr, 0, 0));
    Asset *asset = ptr.asset;
    Space *root_space = asset->space;
    void *context_mapped = nullptr;
    // copy machine context to task
    if (context)
    {
        auto mem_asset_res = asset_create_memory(root_space, {
                                                                 .size = math::alignUp(sizeof(mcx::MachineContext), 4096ul),
                                                             });
        if (mem_asset_res.is_error())
        {
            log::err$("unable to create memory asset for machine context: {}", mem_asset_res.error());
            asset_release(nullptr, asset);
            return mem_asset_res.error();
        }

        auto mem_asset_ptr = mem_asset_res.unwrap();
        auto mem_asset = mem_asset_ptr.asset;
        auto mem = mem_asset->memory;
        void *copied_data = (void *)toVirt(mem.addr);
        memcpy(copied_data, context, sizeof(mcx::MachineContext));
        if (asset_create_mapping(root_space, (AssetMappingCreateParams){
                                                 .start = mem.addr,
                                                 .end = math::alignUp(mem.addr + sizeof(mcx::MachineContext), 4096ul),
                                                 .physical_mem = mem_asset,
                                                 .writable = true,
                                             })
                .is_error())
        {
            log::err$("unable to create mapping for machine context: {}", mem_asset_res.error());
            asset_release(nullptr, asset);
            return mem_asset_res.error();
        }
        context_mapped = (void *)mem.addr;
    }

    auto task_asset_res = asset_create_task(root_space,
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
        asset_release(nullptr, asset);
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
        auto mem_asset_res = asset_create_memory(root_space, {
                                                                 .size = mem_size,
                                                             });

        if (mem_asset_res.is_error())
        {
            log::err$("unable to create memory asset for program header {}: {}", i, mem_asset_res.error());
            asset_release(nullptr, asset);

            return mem_asset_res.error();
        }

        auto mem_asset_ptr = mem_asset_res.unwrap();
        auto mem_asset = mem_asset_ptr.asset;
        auto mem = mem_asset->memory;

        void *copied_data = (void *)toVirt(mem.addr);

        memset(copied_data, 0, ph.mem_size);
        memcpy(copied_data,
               (void *)((uintptr_t)loaded.range().start() + ph.file_offset),
               ph.file_size);

        if (asset_create_mapping(root_space, (AssetMappingCreateParams){
                                                 .start = ph.virt_addr,
                                                 .end = ph.virt_addr + mem_size,
                                                 .physical_mem = mem_asset,
                                                 .writable = true,
                                                 .executable = true,
                                             })
                .is_error())
        {
            log::err$("unable to create mapping for program header {}: {}", i, mem_asset_res.error());
            asset_release(nullptr, asset);
            return mem_asset_res.error();
        }
    }

    log::log$("module loaded: task uid: {}", task->uid());

    try$(kernel::task_run(task->uid()));

    return {};
}
