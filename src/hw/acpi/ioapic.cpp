

#include <stddef.h>

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
 core::Result<void> initialize(size_t index, MadtEntryIoapic const *entry)
{
    if (index >= max_ioapic)
    {
        return "error: out of range";
    }

    auto &ioapic = ioapics[index];
    (void)ioapic;

    (void)entry;
    return {};
}
}; // namespace hw::acpi
