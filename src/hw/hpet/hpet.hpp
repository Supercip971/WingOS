#pragma once

#include <hw/acpi/rsdt.hpp>

#include "hw/mem/addr_space.hpp"
namespace hw::hpet
{

enum HPETRegister
{
    HPET_CAPABILITIES = 0x0,
    HPET_CONFIGURATION = 0x10,
    HPET_INTERRUPT_STATUS = 0x20,
    HPET_MAIN_COUNTER = 0xf0,
    HPET_TIMER0_CONFIG = 0x100,
    HPET_TIMER0_COMPARATOR = 0x108,
    HPET_TIMER0_FSB = 0x110, // interrupt route
};

struct [[gnu::packed]] HPETAddress
{
    uint8_t is_system_io_or_memory;
    uint8_t register_bit_width;
    uint8_t register_bit_offset;
    uint8_t _reserved;
    PhysAddr address;
};

struct [[gnu::packed]] HPETEntry
{
    acpi::SdtHeader header;
    uint8_t hardware_rev_id;
    uint8_t comparator_count : 5;
    uint8_t counter_size : 1;
    uint8_t _reserved0 : 1;
    uint8_t legacy_replacement : 1;
    uint16_t pci_vendor_id;

    HPETAddress address;
    uint8_t hpet_number;
    uint16_t minimum_tick;
    uint8_t page_protection;

    static constexpr core::Str signature = "HPET";
};

struct [[gnu::packed]] HPETCaps
{
    uint8_t rev_id;
    uint8_t num_time_cap : 5;
    uint8_t count_size_cap : 1;
    uint8_t _reserved1 : 1;
    uint8_t legacy_replacement_cap : 1;
    uint16_t vendor_id;
    uint32_t clock_period;

    static HPETCaps from(uint64_t value)
    {
        return *(HPETCaps *)&value;
    }
};

enum HPETConfiguration
{
    HPET_DISABLE_COUNTER = 0,
    HPET_ENABLE_COUNTER = 1 << 0,
    HPET_LEGACY_REPLACEMENT = 1 << 1,
};

core::Result<void> hpet_initialize(hw::acpi::Rsdp *rsdp);

void hpet_sleep(uint64_t ms);

} // namespace hw::hpet