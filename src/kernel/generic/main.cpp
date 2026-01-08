

#include <kernel/generic/kernel.hpp>
#include <libcore/fmt/log.hpp>
#include <libelf/elf.hpp>
#include <stdlib.h>

#include "hw/mem/addr_space.hpp"

#include "hw/acpi/lapic.hpp"
#include "kernel/generic/execute.hpp"
#include "kernel/generic/paging.hpp"
#include "kernel/generic/pmm.hpp"
#include "kernel/generic/scheduler.hpp"
#include "kernel/generic/space.hpp"
#include "kernel/generic/task.hpp"
#include "libcore/fmt/flags.hpp"
#include "libcore/lock/lock.hpp"
core::Lock kernel_lock;

void fun1()
{
    while (true)
    {
        lock_scope$(kernel_lock);
        log::log$("fun1 {}", Cpu::currentId());
        asm volatile("pause");
    }
}

void fun2()
{
    while (true)
    {
        lock_scope$(kernel_lock);
        log::log$("fun2 {}", Cpu::currentId());
        asm volatile("pause");
    }
}

void fun3()
{
    while (true)
    {
        lock_scope$(kernel_lock);
        log::log$("fun3 {}", Cpu::currentId());
        asm volatile("pause");
    }
}
void fun4()
{
    while (true)
    {
        lock_scope$(kernel_lock);
        log::log$("fun4 {}", Cpu::currentId());
        asm volatile("pause");
    }
}
void fun5()
{
    while (true)
    {
        lock_scope$(kernel_lock);
        log::log$("fun5 {}", Cpu::currentId());

        asm volatile("pause");
    }
}

void kernel_entry(const mcx::MachineContext *context)
{

    (void)context;

    log::log$("started kernel");

    kernel::scheduler_init(Cpu::count()).assert();

    //  kernel::task_run(task1->uid()).assert();
    //  kernel::task_run(task2->uid()).assert();
    //  kernel::task_run(task3->uid()).assert();
    //  kernel::task_run(task4->uid()).assert();
    //  kernel::task_run(task5->uid()).assert();

    for (int i = 0; i < context->_modules_count; i++)
    {
        auto mod = context->_modules[i];

        if (!core::Str(mod.path).start_with("/bin/init"))
        {
            log::log$("skipping module {}: {}", i, mod.path);
            continue;
        }
        log::log$("module {}: {}", i, mod.path);
        auto loader = (elf::ElfLoader::load(mod.range.as<VirtAddr>()));

        if (loader.is_error())
        {
            log::err$("unable to load module {}: {}", i, loader.error());
            continue;
        }

        log::log$("module {} loaded: {}", i, mod.path);

        start_module_execution(loader.unwrap(), context).unwrap();
    }


    auto v = Space::global_space_by_handle(0);

    if (v.is_error())
    {
        log::err$("unable to get global space: {}", v.error());
        while (true)
            ;
    }

    v.unwrap().asset->dump_assets();


    Cpu::current()->interrupt_release();


    while (true)
    {
        asm volatile("sti");
        asm volatile("hlt");
    }
}
