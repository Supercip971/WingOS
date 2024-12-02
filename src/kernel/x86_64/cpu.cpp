
#include "cpu.hpp"
#include <libcore/fmt/log.hpp>

#include "hw/acpi/lapic.hpp"
#include "kernel/generic/cpu.hpp"
#include "libcore/ds/array.hpp"
#include "libcore/result.hpp"
constexpr int max_cpu = 512;
core::Array<arch::amd64::CpuImpl, max_cpu> cpus;

static int initialized_count = 0;
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
    if (id > max_cpu || !cpus[id]._present) [[unlikely]]
    {
        log::log$("error: cpu id {} is not valid", id);
        return nullptr;
    }
    return &cpus[id];
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

int Cpu::currentId()
{
    return hw::acpi::Lapic::current_cpu().id();
}
