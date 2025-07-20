#include "kernel/x86_64/cpu.hpp"
#include <arch/x86_64/gdt.hpp>
#include "kernel/generic/cpu.hpp"
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
void gdt_use()
{
    gdtr_install(CpuImpl::currentImpl()->gdtr());
}
}