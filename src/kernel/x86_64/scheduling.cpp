

#include <stdint.h>

#include "kernel/generic/cpu.hpp"
#include "kernel/generic/scheduler.hpp"
uintptr_t _scheduler_impl(uintptr_t stack)
{

    kernel::scheduler_tick();
    auto res = kernel::schedule(Cpu::current()->currentTask(), (void *)stack, Cpu::currentId());

    if (res.error())
    {
        return stack;
    }

    return stack;
}