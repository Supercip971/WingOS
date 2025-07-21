

#include "execute.hpp"
#include <string.h>

#include "hw/mem/addr_space.hpp"

#include "kernel/generic/pmm.hpp"
#include "kernel/generic/scheduler.hpp"
#include "kernel/generic/task.hpp"
#include "libcore/alloc/alloc.hpp"
#include "libcore/enum-op.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/type/trait.hpp"
#include "libelf/elf.hpp"
#include "math/align.hpp"

core::Result<void> start_module_execution(elf::ElfLoader loaded)
{

    kernel::Task *task = kernel::Task::task_create().unwrap();

    try$(task->initialize({
        .entry = (void *)loaded.entry_point(),
        .user = true,
    }));

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

        auto flags = PageFlags()
                         .present(true)
                         .writeable(true)
                         .executable(true)
                         .user(true);

        size_t page_count = math::alignUp(ph.mem_size, 4096ul) / 4096;
        size_t mem_size = page_count * 4096;
        if (page_count == 0)
        {
            log::warn$("skipping program header {}: page count is 0", i);
            continue;
        }

        PhysAddr copied_data_paddr = try$(Pmm::the().allocate(page_count));
        void *copied_data = (void *)toVirt(copied_data_paddr);

        memset(copied_data, 0, ph.mem_size);
        memcpy(copied_data,
               (void *)((uintptr_t)loaded.range().start() + ph.file_offset),
               ph.file_size);

        auto virt_range = VirtRange{ph.virt_addr, ph.virt_addr + mem_size};
        auto phys_range = PhysRange{
            copied_data_paddr,
            copied_data_paddr + mem_size,
        };
        
        auto res = task->vmm_space().map(virt_range, phys_range, flags);
        if (res.is_error())
        {
            log::err$("unable to map program header {}: {}", i, res.error());
            return res;
        }
    }

    log::log$("module loaded: task uid: {}", task->uid());

    try$(kernel::task_run(task->uid()));


    

    return {};
}
