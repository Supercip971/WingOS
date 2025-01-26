

#include <kernel/generic/kernel.hpp>
#include <libcore/fmt/log.hpp>
#include <stdlib.h>

#include "kernel/generic/pmm.hpp"
#include "libcore/fmt/flags.hpp"

void kernel_entry(const mcx::MachineContext *context)
{

    (void)context;
    
    asm volatile("sti");
    log::log$("started kernel");

    while (true)
    {
        asm volatile("hlt");
    }
}