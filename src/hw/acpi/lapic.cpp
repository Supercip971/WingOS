
#include <stdint.h>

#include "arch/x86_64/msr.hpp"
#include "hw/mem/addr_space.hpp"

#include "lapic.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/result.hpp"

static hw::acpi::Lapic main_lapic;
namespace hw::acpi
{

Lapic &Lapic::current()
{
    return main_lapic;
}

using AMsr = arch::amd64::Msr;
using MsrReg = arch::amd64::MsrReg;
core::Result<void> Lapic::enable()
{

    log::log$("Enabling LAPIC");

    AMsr::Write(MsrReg::APIC,
                (AMsr::Read(MsrReg::APIC) | arch::amd64::MsrApicBits::MSR_APIC_ENABLE) & (~(arch::amd64::MsrApicBits::MSR_X2APIC_ENABLE)));

    this->write(LAPICReg::SPURIOUS_INTERRUPT_VECTOR,
                this->read(LAPICReg::SPURIOUS_INTERRUPT_VECTOR) | LAPIC_SPURIOUS_APIC_SOFT_ENABLE |
                    LAPIC_SPURIOUS_VECTOR);

    log::log$("LAPIC Enabled");
    return {};
}
core::Result<void> Lapic::initialize(PhysAddr lapic_addr)
{

    VirtAddr mapped_registers = toVirt(lapic_addr);

    main_lapic = Lapic(mapped_registers);

    try$(main_lapic.enable());
    return {};
}
}; // namespace hw::acpi
