
#pragma once

#include <stddef.h>

#include "hw/acpi/madt.hpp"
#include "libcore/result.hpp"
namespace hw::acpi
{

enum IOApicRegs : uint32_t
{
    IOAPIC_REG_ID = 0x00,
    IOAPIC_REG_VERSION = 0x01,
    IOAPIC_REG_ARBITRATION = 0x02,
    IOAPIC_REG_REDIR_TABLE_BASE = 0x10,
};

class IOApic
{

    VirtAddr _base;
    MadtEntryIoapic _entry;

public:
    IOApic() = default;
    IOApic(VirtAddr base) : _base(base) {}

    template <typename T = uint32_t>
    T read(size_t reg)
    {
        T volatile *reg_ptr = _base.as<T volatile>();
        T volatile *value_ptr = (_base + 16).as<T volatile>();

        *reg_ptr = reg;
        return *value_ptr;
    }

    template <typename T = uint32_t>
    T write(size_t reg, T value)
    {
        T volatile *reg_ptr = _base.as<T volatile>();
        T volatile *value_ptr = (_base + 16).as<T volatile>();

        *reg_ptr = reg;
        *value_ptr = value;
    }

    size_t interrupt_base() const { return _entry.global_system_interrupt_base; }

    size_t max_redirect() { return (read(IOAPIC_REG_VERSION) & 0x00FF0000) >> 16; }

    static IOApic &get(size_t index);

    static core::Result<void> initialize(size_t index, MadtEntryIoapic const *entry);
};

}; // namespace hw::acpi
