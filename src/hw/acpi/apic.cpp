

#include <libcore/fmt/log.hpp>
#include <stdint.h>

#include "hw/mem/addr_space.hpp"
#include "kernel/x86_64/cpu.hpp"
#include "hw/pic/pic.hpp"

#include "apic.hpp"
#include "hw/acpi/ioapic.hpp"
#include "hw/acpi/lapic.hpp"
#include "hw/acpi/madt.hpp"
#include "hw/acpi/rsdp.hpp"
#include "hw/acpi/rsdt.hpp"
#include "kernel/generic/cpu.hpp"
#include "libcore/result.hpp"
#include "mcx/mcx.hpp"

size_t _cpu_count;
hw::acpi::Rsdp *_rsdp;
hw::acpi::Madt *_madt;

size_t current_iso = 0;

// CONFIGURABLE
constexpr int max_iso = 256;
core::Array<hw::acpi::MadtEntryIso, max_iso> iso = {};

namespace hw::acpi
{

size_t apic_cpu_count()
{
    return _cpu_count;
}

core::Result<void> update_interrupt_source_override(MadtEntryIso const *entry)
{

    if (current_iso >= max_iso - 1)
    {
        return "error: ISO out of range";
    }
    log::log$("iso: {}", current_iso);

    iso[current_iso] = *entry;
    current_iso++;

    log::log$("  bus source: {}", entry->bus_source);
    log::log$("  irq source: {}", entry->irq_source);
    log::log$("  global system interrupt: {}", entry->global_system_interrupt);
    log::log$("  flags: {}", entry->flags);
    return {};
}

core::Result<void> apic_enable()
{
    try$(Lapic::the().enable());
    hw::Pic::disable();
    return {};

}

hw::acpi::Rsdp *rsdp()
{
    return _rsdp;
}

core::Result<void> apic_initialize(mcx::MachineContext const *context, CpuDetectedFunc *cpu_callback)
{

    current_iso = 0;
    _rsdp = (hw::acpi::Rsdp *)context->_rsdp;

    log::log$("rsdp: {}", context->_rsdp | fmt::FMT_HEX);

    hw::acpi::Madt *madt;
    auto addr = _rsdp->rsdt_phys_addr();
    if (addr.type == hw::acpi::RsdtTypes::RSDT)
    {
        log::log$("kind: RSDT");

        auto rsdt = toVirt(addr.physical_addr).as<hw::acpi::Rsdt>();

        log::log$("rev: {}", rsdt->header.revision);

        hw::acpi::dump(rsdt);

        madt = try$((hw::acpi::SdtFind<hw::acpi::Rsdt, hw::acpi::Madt>(rsdt)));
    }
    else
    {
        log::log$("kind: XSDT");

        auto xsdt = toVirt(addr.physical_addr).as<hw::acpi::Xsdt>();

        log::log$("rev: {}", xsdt->header.revision);
        hw::acpi::dump(xsdt);

        madt = try$((hw::acpi::SdtFind<hw::acpi::Xsdt, hw::acpi::Madt>(xsdt)));
    }

    _madt = madt;

    madt->dump();
    madt->foreach_entry<MadtEntryLapic>([cpu_callback](MadtEntryLapic *entry)
                                        {
                                            log::log$("- cpu detected: {} {}", entry->acpi_processor_id, entry->apic_id);
                                            cpu_callback(entry).assert();
                                            _cpu_count++; });

    int count = 0;
    madt->foreach_entry<MadtEntryIoapic>([&count](auto *value)
                                         {
        IOApic::initialize(count, value);
        count++; });

    madt->foreach_entry<MadtEntryIso>([](auto *value)
                                      { update_interrupt_source_override(value); });

    // will disable PIC
    // will enable APIC
    Lapic::initialize(madt);
    
    return {};
}

core::Result<void> _raw_redirect_interrupt(LCpuId cpu, uint8_t vector, bool enabled, uint32_t target_gsi, uint16_t flags)
{
    auto ioapic_index = try$(IOApic::query_from_irq(target_gsi));

    auto &ioapic = IOApic::get(ioapic_index);

    IoapicRedirectionReg final = {0};

    final.val.vector = vector;
    final.val.mask = !enabled;

    if (flags & APIC_INTERRUPT_ACTIVE_LOW)
    {
        final.val.polarity = 1;
    }

    if (flags & APIC_INTERRUPT_LEVEL_TRIGGERED)
    {
        final.val.level_trigerred_received = 1;
    }

    auto arch_cpu = arch::amd64::CpuImpl::getImpl(cpu);

    if (arch_cpu == nullptr)
    {
        return "error: cpu not found";
    }

    final.val.destination = arch_cpu->lapic();

    ioapic.redirect(target_gsi - ioapic.interrupt_base(), final);

    return {};
}
core::Result<void> redirect_interrupt(LCpuId cpu, uint8_t irq, uint8_t vector, bool enabled)
{

    log::log$("- redirecting interrupt: cpu: {} irq: {} to vector: {} enabled: {}", cpu, irq, vector, enabled);

    for (size_t i = 0; i < current_iso; i++)
    {
        auto &entry = iso[i];
        if (entry.irq_source == irq)
        {
            log::log$("  found iso: {}", i);
            log::log$("    bus source: {}", entry.bus_source);
            log::log$("    irq source: {}", entry.irq_source);
            log::log$("    global system interrupt: {}", entry.global_system_interrupt);
            log::log$("    flags: {}", entry.flags);

            try$(_raw_redirect_interrupt(cpu, vector, enabled, iso[i].global_system_interrupt, entry.flags));
            return {};
        }
    }
    try$(_raw_redirect_interrupt(cpu, vector, enabled, irq, 0));

    return {};
}

} // namespace hw::acpi
