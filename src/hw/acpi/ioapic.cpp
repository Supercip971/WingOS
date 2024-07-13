

#include <stddef.h>

#include "hw/mem/addr_space.hpp"

#include "hw/acpi/ioapic.hpp"
#include "hw/acpi/madt.hpp"
#include "libcore/ds/array.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/result.hpp"
namespace hw::acpi
{

constexpr int max_ioapic = 256;
core::Array<IOApic, max_ioapic> ioapics = {};

IOApic &IOApic::get(size_t index)
{
    if (index >= max_ioapic)
    {
        log::warn$("IOApic index {} is out of range", index);
        return ioapics[0];
    }
    return ioapics[index];
}

core::Result<void> IOApic::initialize(size_t index, MadtEntryIoapic const *entry)
{
    if (index >= max_ioapic)
    {
        return "error: out of range";
    }

    auto &ioapic = ioapics[entry->ioapic_id];

    ioapic = IOApic(*entry, toVirt(entry->ioapic_addr));

    log::log$("ioapic[{}]: ", entry->ioapic_id);
    log::log$("  id: {}", ioapic.read(IOAPIC_REG_ID));
    log::log$("  max redirect: {}", ioapic.max_redirect());
    log::log$("  interrupt base: {}", ioapic.interrupt_base());
    log::log$("  version: {}", ioapic.read(IOAPIC_REG_VERSION));
    log::log$("  arbitration: {}", ioapic.read(IOAPIC_REG_ARBITRATION));
    log::log$("  redir table base: {}", ioapic.read(IOAPIC_REG_REDIR_TABLE_BASE));

    return {};
}
}; // namespace hw::acpi
