
#include <kernel/generic/kernel.hpp>
#include <libcore/fmt/log.hpp>
#include <mcx/mcx.hpp>

#include "arch/x86_64/idt.hpp"
#include <arch/x86_64/gdt.hpp>

void arch_entry(const mcx::MachineContext *context)
{
    log::log$("started kernel arch");

    arch::amd64::gdt_use(arch::amd64::load_default_gdt());
    log::log$("loaded kernel gdt");

    arch::amd64::idt_use(arch::amd64::load_default_idt());
    log::log$("loaded kernel idt");

    kernel_entry(context);
}