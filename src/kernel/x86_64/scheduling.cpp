

#include <stdint.h>

#include "hw/acpi/lapic.hpp"
#include "kernel/generic/cpu.hpp"
#include "kernel/generic/scheduler.hpp"
#include "arch/x86_64/context.hpp"
#include "libcore/fmt/log.hpp"
uintptr_t _scheduler_impl(uintptr_t stack)
{


    kernel::scheduler_tick();
    auto res = kernel::schedule(Cpu::current()->currentTask(), (void *)stack, Cpu::currentId());

    if (res.is_error())
    {
        return stack;
    }

    return stack;
}

uintptr_t _scheduler_impl_soft(uintptr_t stack)
{

    auto *current = Cpu::current()->currentTask();

    auto res = kernel::schedule(current, (void *)stack, Cpu::currentId(), true);

    if (res.is_error())
    {
        auto err = res.error();
        log::err$("soft schedule: kernel::schedule returned error: {}", err);
        return stack;
    }

    return stack;
}



void trigger_reschedule(CoreId cpu)
{
    hw::acpi::Lapic::the().send_interrupt(cpu, 100);
}
void trigger_reschedule_unblocked(CoreId cpu)
{
    hw::acpi::Lapic::the().send_interrupt(cpu, 101);
}
