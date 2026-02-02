
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
#include "arch/x86_64/simd.hpp"
#include "iol/mem_flags.h"
#include "kernel/generic/cpu_tree.hpp"
#include "kernel/x86_64/numa.hpp"
#include "kernel/x86_64/smp.hpp"
#include "kernel/x86_64/syscall.hpp"
#include <arch/x86_64/gdt.hpp>
// ee

#include "cpu.hpp"

volatile size_t _running_cpu_count = 0;
using InitializerPtr = void (*)();

extern "C" InitializerPtr __init_array_start[] __attribute__((weak, visibility("hidden")));
extern "C" InitializerPtr __init_array_end[] __attribute__((weak, visibility("hidden")));

static core::Result<void> cpu_detect(const hw::acpi::MadtEntryLapic *lapic)
{

    try$(arch::amd64::cpuContextInit(lapic->acpi_processor_id, lapic->apic_id));

    return {};
}

void arch_entry(const mcx::MachineContext *context)
{
    _running_cpu_count = 1;
    log::log$("started kernel arch");
    {
        if (*__init_array_start != nullptr)
        {
            for (size_t i = 0; i < (size_t)(__init_array_end - __init_array_start); i++)
            {
                (__init_array_start[i])();
            }
        }
    }
    arch::amd64::load_default_gdt();
    arch::amd64::gdt_use();
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

    arch::x86_64::SimdContext::initialize_cpu().assert();

    log::log$("cpu count: {}", hw::acpi::apic_cpu_count());
    arch::amd64::smp_initialize().assert();

    arch::amd64::setup_ist();

    Cpu::current()->interrupt_hold();
    while (_running_cpu_count < arch::amd64::CpuImpl::count())
    {
        asm volatile("pause");
    }

    log::log$("all cpus are ready");

    log::log$("Initialize cpu tree:");
    auto root = initialize_cpu_tree().unwrap();
    CpuTreeNode::assign_root(root);

    log::log$("cpu tree initialized");

    arch::amd64::syscall_init_for_current_cpu().assert();
    arch::amd64::setup_entry_gs();
    kernel_entry(context);
}

void arch::amd64::other_cpu_entry(bool& ready)
{
    //  log::log$("other cpu entry");
    hw::acpi::Lapic::the().enable().assert();

    Cpu::current()->interrupt_hold();
    _running_cpu_count += 1;

    arch::amd64::load_default_gdt();
    arch::amd64::gdt_use();

    arch::amd64::setup_ist();

    arch::amd64::syscall_init_for_current_cpu();
    arch::amd64::setup_entry_gs();

    arch::x86_64::SimdContext::initialize_cpu().assert();


    ready = true;
    while (_running_cpu_count < CpuImpl::count())
       {
           asm volatile("pause");
       }

       Cpu::current()->interrupt_release();
       while (true)
       {
           asm volatile("sti");
           asm volatile("hlt");
       };

}
