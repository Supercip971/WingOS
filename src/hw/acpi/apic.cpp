

#include <libcore/fmt/log.hpp>

#include "hw/mem/addr_space.hpp"

#include "hw/acpi/lapic.hpp"
#include "hw/acpi/madt.hpp"
#include "hw/acpi/rsdp.hpp"
#include "hw/acpi/rsdt.hpp"
#include "libcore/result.hpp"
#include "mcx/mcx.hpp"

namespace hw::acpi
{

core::Result<void> apic_initialize(mcx::MachineContext const *context)
{
    hw::acpi::Rsdp *rsdp = (hw::acpi::Rsdp *)context->_rsdp;

    log::log$("rsdp: {}", context->_rsdp | fmt::FMT_HEX);

    hw::acpi::Madt *madt;
    auto addr = rsdp->rsdt_phys_addr();
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

        // madt  = try$(hw::acpi::SdtFind<hw::acpi::Xsdt, hw::acpi::Madt>(xsdt));

        madt = try$((hw::acpi::SdtFind<hw::acpi::Xsdt, hw::acpi::Madt>(xsdt)));
    }

    PhysAddr lapic_addr = madt->local_apic_addr;

    madt->foreach_entry<MadtEntryLapicOverride>([&](MadtEntryLapicOverride *entry)
                                                {
		log::log$("lapic override: {}", entry->local_apic_addr);
		lapic_addr = entry->local_apic_addr; });

    log::log$("lapic: {}", lapic_addr._addr | fmt::FMT_HEX);

    Lapic::initialize(lapic_addr);
    return {};
}
} // namespace hw::acpi
