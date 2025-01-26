
#include <kernel/generic/pmm.hpp>

#include "hw/acpi/lapic.hpp"
#include "hw/acpi/rsdp.hpp"
#include "hw/acpi/rsdt.hpp"
#include "hw/hpet/hpet.hpp"
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
#include "kernel/x86_64/smp.hpp"
#include <arch/x86_64/gdt.hpp>
// ee

#include "cpu.hpp"

volatile size_t _running_cpu_count = 0;

static core::Result<void> cpu_detect(const hw::acpi::MadtEntryLapic *lapic)
{

    try$(arch::amd64::cpuContextInit(lapic->acpi_processor_id, lapic->apic_id));

    return {};
}
void arch_entry(const mcx::MachineContext *context)
{
    _running_cpu_count = 1;
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
    VmmSpace::use_kernel(kernel_space);

    log::log$("using vmm");

    hw::acpi::apic_initialize(context, cpu_detect).assert();

    log::log$("HPET initializing");
    hw::hpet::hpet_initialize(hw::acpi::rsdp()).assert();

    log::log$("HPET initialized");
    
    hw::acpi::Lapic::the().timer_initialize().assert();

    log::log$("cpu count: {}", hw::acpi::apic_cpu_count());
    arch::amd64::smp_initialize().assert();

    while (_running_cpu_count < arch::amd64::CpuImpl::count())
    {
        asm volatile("pause");
    }

    log::log$("all cpus are ready");

    kernel_entry(context);
}

void arch::amd64::other_cpu_entry()
{
    //  log::log$("other cpu entry");
    hw::acpi::Lapic::the().enable().assert();
    _running_cpu_count += 1;

    while (_running_cpu_count < CpuImpl::count())
    {
        asm volatile("pause");
    }

    while (true)
    {
        asm volatile("hlt");
    };
}