

#include <stddef.h>

#include "hw/mem/addr_space.hpp"

#include "hw/acpi/ioapic.hpp"
#include "hw/acpi/madt.hpp"
#include "libcore/ds/array.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/result.hpp"
namespace hw::acpi
{
// CONFIGURABLE
constexpr int max_ioapic = 256;
core::Array<IOApic, max_ioapic> ioapics = {};

core::Result<IOApic*> IOApic::get(IOApicIndex index)
{
    if (index >= max_ioapic)
    {
        return "error: IOApic index out of range";
    }
    return &ioapics[index];
}

core::Result<void> IOApic::initialize(IOApicIndex index, MadtEntryIoapic const *entry)
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

core::Result<IOApicIndex> IOApic::query_from_irq(size_t irq)
{
    for (size_t i = 0; i < max_ioapic; i++)
    {
        auto &ioapic = ioapics[i];
        if (ioapic.interrupt_base() <= irq && ioapic.interrupt_base() + ioapic.max_redirect() > irq)
        {
            return i;
        }
    }
    return "error: no ioapic found for irq";
}

}; // namespace hw::acpi
