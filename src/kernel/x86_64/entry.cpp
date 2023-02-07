
#include <kernel/kernel.hpp>
#include <libcore/fmt/log.hpp>
#include <mcx/mcx.hpp>

#include <arch/x86_64/gdt.hpp>

void arch_entry(const mcx::MachineContext *context)
{
    log::log$("started kernel arch");

    arch::amd64::gdt_use(arch::amd64::load_default_gdt());
    log::log$("loaded kernel gdt");


    kernel_entry(context);
}