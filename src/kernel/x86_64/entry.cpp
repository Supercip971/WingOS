
#include <mcx/mcx.hpp>
#include <kernel/kernel.hpp>
#include <libcore/fmt/log.hpp>
#include <arch/x86_64/gdt.hpp>

void arch_entry(const mcx::MachineContext* context)
{
    log::log$("started kernel arch");

    {
        arch::amd64::load_gdt(arch::amd64::default_gdt());


    }

    kernel_entry(context);
}