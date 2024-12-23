
#pragma once

#include <stddef.h>

#include "hw/mem/addr_space.hpp"

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

enum IoapicRegDeliveryType
{

    IOAPIC_REG_DELIVERY_NORMAL = 0,
    IOAPIC_REG_DELIVERY_LOWEST_PRIORITY = 1,
    IOAPIC_REG_DELIVERY_SMI = 2,
    IOAPIC_REG_DELIVERY_NMI = 4,
    IOAPIC_REG_DELIVERY_INIT = 5,
    IOAPIC_REG_DELIVERY_EXTINT = 7,
};

union IoapicRedirectionReg
{

    uint64_t raw;
    struct [[gnu::packed]]
    {
        uint8_t vector;

        uint8_t delivery_type : 3;
        // physical or logical
        uint8_t destination_mode : 1;

        uint8_t pending : 1;

        uint8_t polarity : 1;

        uint8_t level_trigerred_received : 1;
        // 0 = edge sensitive
        // 1 = Level sensitive
        uint8_t is_level_sensitive : 1;

        uint8_t mask : 1;

        uint64_t reserved : 39;

        uint8_t destination : 8;
    } val;
};
using IOApicIndex = size_t;
class IOApic
{

    VirtAddr _base;
    MadtEntryIoapic _entry;

public:
    IOApic() = default;
    IOApic(VirtAddr base) : _base(base) {}
    IOApic(MadtEntryIoapic entry, VirtAddr base) : _base(base), _entry(entry) {}
    template <typename T = uint32_t>
    T read(size_t reg)
    {
        T volatile *reg_ptr = _base.as<T volatile>();
        T volatile *value_ptr = (_base + 16).as<T volatile>();

        *reg_ptr = reg;
        return *value_ptr;
    }

    template <typename T = uint32_t>
    void write(size_t reg, T value)
    {
        T volatile *reg_ptr = _base.as<T volatile>();
        T volatile *value_ptr = (_base + 16).as<T volatile>();

        *reg_ptr = reg;
        *value_ptr = value;
    }

    void redirect(uint16_t local_irq, IoapicRedirectionReg reg)
    {
        write<uint32_t>(IOAPIC_REG_REDIR_TABLE_BASE + local_irq * 2, reg.raw & 0xFFFFFFFF);
        write<uint32_t>(IOAPIC_REG_REDIR_TABLE_BASE + local_irq * 2 + 1, reg.raw >> 32);
    }

    size_t interrupt_base() const { return _entry.global_system_interrupt_base; }

    size_t max_redirect() { return (read(IOAPIC_REG_VERSION) & 0x00FF0000) >> 16; }

    static IOApic &get(IOApicIndex index);

    // in the case of multiple ioapics, we need to select the one
    // that is responsible for the interrupt
    static core::Result<IOApicIndex> query_from_irq(size_t irq);

    static core::Result<void> initialize(IOApicIndex index, MadtEntryIoapic const *entry);
};

}; // namespace hw::acpi
