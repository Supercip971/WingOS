
#include <stdint.h>
#include <hw/hpet/hpet.hpp>
#include "hw/mem/addr_space.hpp"

static VirtAddr hpet_base = 0;
static uint64_t hpet_tick = 0;


uint64_t _hpet_read(uintptr_t reg)
{
    return (hpet_base + reg).vread<uint64_t>();
}

void _hpet_write(uintptr_t reg, uint64_t value)
{
    (hpet_base + reg).vwrite<uint64_t>(value);
}

core::Result<void> hw::hpet::hpet_initialize(hw::acpi::Rsdp* rsdp)
{
    auto hpet = hw::acpi::rsdt_find<hw::hpet::HPETEntry>(rsdp).unwrap();

    hpet_base = toVirt(hpet->address.address);    

    if(hpet->address.is_system_io_or_memory == 1)
    {
        return "HPET is not in memory";
    }

    hpet_tick =HPETCaps::from(_hpet_read(HPETRegister::HPET_CAPABILITIES)).clock_period;
    
    _hpet_write(HPET_CONFIGURATION + 0, HPETConfiguration::HPET_DISABLE_COUNTER);
    _hpet_write(HPET_MAIN_COUNTER, 0);
    _hpet_write(HPET_CONFIGURATION + 0, HPETConfiguration::HPET_ENABLE_COUNTER);

    return {};
}


void hw::hpet::hpet_sleep(uint64_t ms)
{
    // 1 ms => 1000000000000 femtoseconds
    uint64_t target = _hpet_read(HPET_MAIN_COUNTER) + (ms * 1000000000000) / hpet_tick;

    while(_hpet_read(HPET_MAIN_COUNTER) <= target)
    {
        asm volatile("pause");
    }
}