
#include <kernel/generic/pmm.hpp>

#include "hw/acpi/rsdp.hpp"
#include "hw/acpi/rsdt.hpp"
#include "kernel/generic/mem.hpp"
#include "kernel/generic/paging.hpp"
#include "libcore/fmt/impl/bitmap.hpp"
// ee
#include <hw/acpi/apic.hpp>
#include <hw/acpi/madt.hpp>
#include <kernel/generic/kernel.hpp>
#include <libcore/fmt/log.hpp>
#include <mcx/mcx.hpp>

#include "arch/x86_64/idt.hpp"
#include "iol/mem_flags.h"
#include <arch/x86_64/gdt.hpp>
// ee

void arch_entry(const mcx::MachineContext *context)
{
    log::log$("started kernel arch");

    arch::amd64::gdt_use(arch::amd64::load_default_gdt());
    log::log$("loaded kernel gdt");

    arch::amd64::idt_use(arch::amd64::load_default_idt());
    log::log$("loaded kernel idt");

    Pmm::initialize(context).assert();

    log::log$("initialized kernel pmm");

    log::log$(" pmm: {}", Pmm::the());

    log::log$(" loading the vmm");

    auto kernel_space = VmmSpace::kernel_initialize(context).unwrap();
    log::log$("using vmm...");

    kernel_space.use();
    log::log$("using vmm");

    hw::acpi::apic_initialize(context);

    log::log$("cpu count: {}", hw::acpi::apic_cpu_count());
    kernel_entry(context);
}
