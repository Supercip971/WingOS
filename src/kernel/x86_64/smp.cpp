#include "smp.hpp"
#include <string.h>

#include "hw/mem/addr_space.hpp"

#include "hw/acpi/lapic.hpp"
#include "kernel/generic/cpu.hpp"
#include "kernel/generic/kernel.hpp"
#include "kernel/generic/paging.hpp"
#include "kernel/generic/pmm.hpp"
#include "kernel/x86_64/cpu.hpp" // Ensure `CpuImpl` and `max_cpu` are available
#include "libcore/fmt/flags.hpp"
#include "libcore/fmt/log.hpp"

static bool _ready = false;
core::Result<void> arch::amd64::smp_initialize()
{
    log::log$("initializing smp (cpu count: {})", CpuImpl::count());

    if (CpuImpl::count() > arch::amd64::max_cpu)
    {
        log::warn$("cpu count ({}) exceeds max cpu count ({})", CpuImpl::count(), arch::amd64::max_cpu);
    }

    for (int cpu = 0; cpu < (int)CpuImpl::count(); cpu++)
    {

        CpuImpl *cpu_impl = CpuImpl::getImpl(cpu);

        cpu_impl->syscall_stack = toVirt(Pmm::the().allocate(kernel::kernel_stack_size).unwrap())._addr + kernel::kernel_stack_size;
        log::log$("cpu: {} (lapic: {})", cpu, cpu_impl->lapic());

        if (cpu_impl->lapic() != CpuImpl::currentId())
        {
            try$(smp_initialize_cpu(cpu_impl->lapic(), cpu));
        }
    }

    return {};
}

void smp_entry_other(void)
{
    log::log$("cpu booted!");
    arch::amd64::other_cpu_entry(_ready);

    while (true)
    {
    };
}

extern "C" uintptr_t start_cpu_entry;
extern "C" uintptr_t end_cpu_entry;
extern "C" uint64_t trampoline_start, trampoline_end;
core::Result<void> _setup_trampoline(hw::acpi::LCpuId cpu_id)
{
    arch::amd64::CpuImpl *cpu = arch::amd64::CpuImpl::getImpl(cpu_id);
    uint64_t trampoline_len = (uintptr_t)&trampoline_end - (uintptr_t)&trampoline_start;

    // setting up virtual memory
    try$(VmmSpace::kernel_page_table().map(
        VirtRange::from_begin_len(0, 0x1000),
        PhysRange::from_begin_len(0, 0x1000),
        PageFlags().executable(true).present(true).writeable(true)));

    try$(VmmSpace::kernel_page_table().map(
        VirtRange::from_begin_len(arch::amd64::SMP_TRAMPOLINE_START, trampoline_len),
        PhysRange::from_begin_len(arch::amd64::SMP_TRAMPOLINE_START, trampoline_len),
        PageFlags().executable(true).present(true).writeable(true)));

    VmmSpace::invalidate();
    VirtAddr(arch::amd64::SMP_PAGE_TABLE).write(VmmSpace::kernel_page_table().self_addr());


    // seting up code
    memcpy((void *)arch::amd64::SMP_TRAMPOLINE_START, (void *)(&trampoline_start), trampoline_len);

    // writing needed values
    VirtAddr(arch::amd64::SMP_START_ADDR).write(smp_entry_other);


    // setting up stack
    PhysAddr stack = try$(Pmm::the().allocate(kernel::kernel_stack_size, IOL_ALLOC_MEMORY_FLAG_LOWER_SPACE));
    uintptr_t stack_addr = (uintptr_t)stack;
    log::log$("stack addr: {}", stack_addr | fmt::FMT_HEX);
    cpu->trampoline_stack(stack);

    VirtAddr(arch::amd64::SMP_STACK).write(toVirt(stack) + kernel::kernel_stack_size);

    memset(toVirt(stack), 0, kernel::kernel_stack_size);

    asm volatile(
        "sgdt 0x530\n"
        "sidt 0x540\n");

    return {};
}

core::Result<void> arch::amd64::smp_initialize_cpu(int apic, int id)
{
    log::log$("initializing cpu: {} (lapic: {})...", id, apic);

    _ready = false;

    try$(hw::acpi::Lapic::the().init_cpu(id));

    try$(_setup_trampoline(id));

    try$(hw::acpi::Lapic::the().send_sipi(id, arch::amd64::SMP_TRAMPOLINE_START));

    while (!_ready)
    {
        // do nothing
        asm volatile("pause");
    }
    log::log$("initialized cpu");

    return {};
}
