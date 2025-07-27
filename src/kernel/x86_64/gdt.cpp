#include "kernel/x86_64/cpu.hpp"
#include <arch/x86_64/gdt.hpp>

#include "kernel/generic/cpu.hpp"
#include "kernel/generic/kernel.hpp"
#include "libcore/alloc/alloc.hpp"
#include "libcore/fmt/log.hpp"
namespace arch::amd64
{

Gdtr *load_default_gdt()
{

    *CpuImpl::currentImpl()->gdt() = Gdt();
    *CpuImpl::currentImpl()->gdtr() = Gdtr{
        reinterpret_cast<uintptr_t>(CpuImpl::currentImpl()->gdt()),
        sizeof(*CpuImpl::currentImpl()->gdt()) - 1,
    };
    return CpuImpl::currentImpl()->gdtr();
}

extern "C" void gdtr_install(Gdtr *gdtr);
extern "C" void tss_update();
void gdt_use()
{
    gdtr_install(CpuImpl::currentImpl()->gdtr());
}

void setup_ist()
{
    auto cpu = arch::amd64::CpuImpl::currentImpl();
    if (!cpu) [[unlikely]]
    {
        log::err$("Failed to get current CPU implementation for IST setup");
        return;
    }

    *cpu->tss() = Tss();
    cpu->tss()->ist[0] = (uintptr_t)core::mem_alloc(kernel::kernel_stack_size).unwrap() + kernel::kernel_stack_size;
    cpu->tss()->ist[1] = (uintptr_t)core::mem_alloc(kernel::kernel_stack_size).unwrap() + kernel::kernel_stack_size;

    cpu->tss()->rsp[0] = (uintptr_t)core::mem_alloc(kernel::kernel_stack_size).unwrap() + kernel::kernel_stack_size;

    cpu->gdt()->tss = TssEntry(cpu->tss());
    tss_update();
}
} // namespace arch::amd64