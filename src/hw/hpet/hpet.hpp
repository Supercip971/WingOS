#pragma once

#include <hw/acpi/rsdt.hpp>

#include "hw/mem/addr_space.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/time/time.hpp"
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

union [[gnu::packed]] HPETCaps
{
    struct [[gnu::packed]] {
    uint8_t rev_id;
    uint8_t num_time_cap : 5;
    uint8_t count_size_cap : 1;
    uint8_t _reserved1 : 1;
    uint8_t legacy_replacement_cap : 1;
    uint16_t vendor_id;
    uint32_t clock_period;
    };

    uint64_t value;

    static HPETCaps from(uint64_t value)
    {
        HPETCaps caps;
        caps.value = value;
        return caps;
    }
};

enum HPETConfiguration
{
    HPET_DISABLE_COUNTER = 0,
    HPET_ENABLE_COUNTER = 1 << 0,
    HPET_LEGACY_REPLACEMENT = 1 << 1,
};

core::Result<void> hpet_initialize(hw::acpi::Rsdp *rsdp);


template<MappCallbackFn Fn>
core::Result<void> hpet_prepare_mapping(uintptr_t rsdp_addr, Fn fn)
{
    auto hpet = hw::acpi::rsdt_find<hw::hpet::HPETEntry>(toVirt(rsdp_addr).as<hw::acpi::Rsdp>()).unwrap();

    auto hpet_base = hpet->address.address;
    try$(fn((uintptr_t)hpet_base, hpet->header.length));
    return {};

}
void hpet_sleep(core::Milliseconds ms);


core::Milliseconds hpet_clock_read();

} // namespace hw::hpet
