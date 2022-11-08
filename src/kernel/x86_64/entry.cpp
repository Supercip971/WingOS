
#include <mcx/mcx.hpp>
#include <kernel/kernel.hpp>
#include <libcore/fmt/log.hpp>

void arch_entry(const mcx::MachineContext* context)
{
    log::log$("started kernel arch");

    kernel_entry(context);
}