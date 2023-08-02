#include <kernel/generic/kernel.hpp>
#include <libcore/fmt/log.hpp>

void kernel_entry(const mcx::MachineContext *context)
{

    (void)context;
    log::log$("started kernel");

    while (true)
    {
        asm volatile("hlt");
    }
}