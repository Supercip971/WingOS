
#include "cpu.hpp"
#include "arch/x86_64/gdt.hpp"
#include <kernel/x86_64/cpu.hpp>
#include <libcore/fmt/log.hpp>

#include "hw/acpi/lapic.hpp"
#include "kernel/generic/cpu.hpp"
#include "libcore/ds/array.hpp"
#include "libcore/result.hpp"
core::Array<arch::amd64::CpuImpl, arch::amd64::max_cpu> cpus;

static int initialized_count = 0;
size_t Cpu::count()
{
    return initialized_count;
}

void Cpu::interrupt_hold()
{
    if (this->interrupt_depth == 0)
    {
        asm volatile("cli");
    }
    this->interrupt_depth++;
}
void Cpu::interrupt_release()
{
    this->interrupt_depth--;
    if (this->interrupt_depth == 0)
    {
        asm volatile("sti");
    }
}

namespace arch::amd64
{
core::Result<void> cpuContextInit(int id, int lapic)
{
    if (id > max_cpu) [[unlikely]]
    {
        return "error: out of range, max cpu exceeded";
    }

    cpus[id] = CpuImpl(id, lapic);

    log::log$("initialized cpu context: {} (lapic: {})", id, lapic);
    initialized_count++;
    return {};
}

CpuImpl *CpuImpl::getImpl(int id)
{
    if ((id > max_cpu || !cpus[id]._present) && id != 0) [[unlikely]]
    {
        log::log$("error: cpu id {} is not valid", id);
        return nullptr;
    }
    return &cpus[id];
}

size_t CpuImpl::max_processors()
{
    return max_cpu;
}

} // namespace arch::amd64
Cpu *Cpu::current()
{
    auto id = Cpu::currentId();
    return id == -1 ? nullptr : Cpu::get(id);
}
Cpu *Cpu::get(int id)
{
    return &cpus[id];
}

arch::amd64::CpuImpl *arch::amd64::CpuImpl::currentImpl()
{
    auto id = Cpu::currentId();
    if (id == -1)
    {
        return nullptr;
    }
    return CpuImpl::getImpl(id);
}

 static arch::amd64::Gdt _gdt[arch::amd64::max_cpu] = {};
 static arch::amd64::Gdtr _gdtr[arch::amd64::max_cpu] = {};


 arch::amd64::Gdt *arch::amd64::CpuImpl::gdt()
 {
        return &_gdt[this->id()];
 }

    arch::amd64::Gdtr *arch::amd64::CpuImpl::gdtr()
    {
            return &_gdtr[this->id()];
    }

CoreId Cpu::currentId()
{
    if(hw::acpi::Lapic::the().is_loaded() == false)
    {
        return 0;
    }
    return hw::acpi::Lapic::the().id();
}
