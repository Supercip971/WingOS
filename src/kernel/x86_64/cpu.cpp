
#include "cpu.hpp"
#include <libcore/fmt/log.hpp>

#include "arch/x86_64/gdt.hpp"
#include "arch/x86_64/msr.hpp"
#include <kernel/x86_64/cpu.hpp>

#include "hw/acpi/lapic.hpp"
#include "kernel/generic/cpu.hpp"
#include "libcore/ds/array.hpp"
#include "libcore/result.hpp"
core::Array<arch::amd64::CpuImpl, arch::amd64::max_cpu> cpus = {};

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


core::Lock cpu_sys_lock;

bool Cpu::enter_syscall_safe_mode()
{
    asm volatile(
        "cli\n"
    );

    cpu_sys_lock.lock();
    return true;
}

bool Cpu::exit_syscall_safe_mode()
{

    cpu_sys_lock.release();
    asm volatile(
        "sti\n"
    );

    return true;
}

bool Cpu::end_syscall()
{
    cpu_sys_lock.release();
    return true;
}
void Cpu::interrupt_release(bool re_enable_int)
{
    if(this->interrupt_depth == 0)
    {
        log::err$("cpu: {} interrupt_release called with depth 0", this->id());
        return;
    }
    this->interrupt_depth--;
    
    if (this->interrupt_depth == 0 && re_enable_int)
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

void setup_entry_gs()
{
    Msr::Write(MsrReg::GS_BASE, (uintptr_t)Cpu::current());
    Msr::Write(MsrReg::KERNEL_GS_BASE, (uintptr_t)Cpu::current());
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
static arch::amd64::Tss _tss[arch::amd64::max_cpu] = {};

arch::amd64::Gdt *arch::amd64::CpuImpl::gdt()
{
    return &_gdt[this->id()];
}

arch::amd64::Gdtr *arch::amd64::CpuImpl::gdtr()
{
    return &_gdtr[this->id()];
}

arch::amd64::Tss *arch::amd64::CpuImpl::tss()
{
    return &_tss[this->id()];
}

CoreId Cpu::currentId()
{
    if (hw::acpi::Lapic::the().is_loaded() == false)
    {
        return 0;
    }
    return hw::acpi::Lapic::the().id();
}

void reschedule_self()
{
    hw::acpi::Lapic::the().send_interrupt(Cpu::currentId(), 101);
}