

#include <libcore/fmt/log.hpp>

#include "hw/mem/addr_space.hpp"

#include "apic.hpp"
#include "hw/acpi/lapic.hpp"
#include "hw/acpi/madt.hpp"
#include "hw/acpi/rsdp.hpp"
#include "hw/acpi/rsdt.hpp"
#include "libcore/result.hpp"
#include "mcx/mcx.hpp"

size_t _cpu_count;
hw::acpi::Rsdp *_rsdp;
hw::acpi::Madt *_madt;

namespace hw::acpi
{

size_t apic_cpu_count()
{
    return _cpu_count;
}

core::Result<void> apic_initialize(mcx::MachineContext const *context)
{

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

    madt->foreach_entry<MadtEntryLapic>([](MadtEntryLapic *entry)
                                        { log::log$("- cpu detected: {} {}", entry->acpi_processor_id, entry->apic_id);
                                        _cpu_count++; });

    Lapic::initialize(madt);
    return {};
}
} // namespace hw::acpi