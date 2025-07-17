#pragma once

#include <stdint.h>

#include "hw/mem/addr_space.hpp"

#include "hw/acpi/madt.hpp"
#include "libcore/enum-op.hpp"
#include "libcore/result.hpp"
#include "libcore/type/trait.hpp"
namespace hw::acpi
{

// cf section 11.4 in chapter 3 of intel programmer manual
using LCpuId = uint64_t;

enum class LAPICReg : uint32_t
{

    ID = 0x20,
    VERSION = 0x30,
    TASK_PRIORITY = 0x80,
    ARBITRATION_PRIORITY = 0x90,
    PROCESSOR_PRIORITY = 0xA0,
    EOI = 0xB0,
    REMOTE_READ = 0xC0,
    LOGICAL_DESTINATION = 0xD0,
    Destination_FORMAT = 0xE0,
    SPURIOUS_INTERRUPT_VECTOR = 0xF0,
    IN_SERVICE_START = 0x100,
    TRIGGER_MODE_START = 0x180,
    INTERRUPT_REQUEST_START = 0x200,
    ERROR_STATUS = 0x280,
    LVT_CMCI = 0x2F0,
    INTERRUPT_COMMAND_START = 0x300,
    INTERRUPT_COMMAND_START_2 = 0x310,
    LVT_TIMER = 0x320,
    LVT_THERMAL_SENSOR = 0x330,
    LVT_PERFORMANCE_MONITORING_COUNTERS = 0x340,
    LVT_LINT0 = 0x350,
    LVT_LINT1 = 0x360,
    LVT_ERROR = 0x370,
    TIMER_INITIAL_COUNT = 0x380,
    TIMER_CURRENT_COUNT = 0x390,
    TIMER_DIVIDE_CONFIGURATION = 0x3E0,
};

enum LAPICSpuriousReg : uint32_t
{
    LAPIC_SPURIOUS_VECTOR = 0xFF,
    LAPIC_SPURIOUS_APIC_SOFT_ENABLE = 1 << 8,
    LAPIC_SPURIOUS_FOCUS_CORE_CHECK = 1 << 9,
};

// FIXME: seems weird, I feel like I'm mixing two definition in the same struct ?
// view: Interrupt Command Register # https://wiki.osdev.org/APIC
enum LAPICInterruptCommandDeliveryMode : uint8_t
{
    LAPIC_INTERRUPT_DELIVERY_FIXED = 0,
    LAPIC_INTERRUPT_DELIVERY_LOWEST_PRIORITY = 1,
    LAPIC_INTERRUPT_DELIVERY_SMI = 2,
    LAPIC_INTERRUPT_DELIVERY_NMI = 4,
    LAPIC_INTERRUPT_DELIVERY_INIT = 5,
    LAPIC_INTERRUPT_DELIVERY_SIPI = 6,
    LAPIC_INTERRUPT_DELIVERY_EXTINT = 7,
};


enum LAPICInterruptCommandBits : uint16_t
{
    LAPIC_INTERRUPT_CMD_LOGICAL = 1 << 11,
    LAPIC_INTERRUPT_CMD_PENDING = 1 << 12,
    LAPIC_INTERRUPT_CMD_SET_INIT_DEASSERT = 1 << 14,
};

union LAPICInterruptCommandRegister
{

    uint32_t raw;
    struct [[gnu::packed]]
    {
        uint8_t vector;
        uint8_t delivery_type : 3; // 0 = fixed, 1 = lowest priority, 2 = SMI, 4 = NMI, 5 = INIT, 6 SIPI
        // physical or logical
        uint8_t destination_mode : 1;
        uint8_t delivery_status : 1;
        uint8_t _reserved : 1;
        uint8_t clear_init_level_deassert : 1;
        uint8_t set_init_level_deassert : 1;
        uint8_t destination_type : 2;
        uint16_t reserved : 8;
    } val;
};

union LAPICLocalVectorTable
{
    uint32_t raw;
    struct [[gnu::packed]]
    {
        uint8_t vector;
        uint8_t delivery_mode : 3;
        uint8_t _reserved : 1;
        uint8_t delivery_status : 1;
        uint8_t pin_polarity : 1;
        uint8_t remote_irr : 1;
        uint8_t trigger_mode : 1; // 0 : edge | 1 : level
        uint8_t mask : 1;
        uint8_t _timer_mode : 2; // 00 : one shot | 01 : periodic | 10 : tsc deadline
        uint32_t _reserved2 : 14;
    } val;
};
// Intel volume 3A 12.5

enum LAPICTimerDivision : uint8_t
{
    LAPIC_TIMER_DIVIDE_BY_2 = 0,
    LAPIC_TIMER_DIVIDE_BY_4 = 1,
    LAPIC_TIMER_DIVIDE_BY_8 = 2,
    LAPIC_TIMER_DIVIDE_BY_16 = 3,
    LAPIC_TIMER_DIVIDE_BY_32 = 4,
    LAPIC_TIMER_DIVIDE_BY_64 = 5,
    LAPIC_TIMER_DIVIDE_BY_128 = 6,
    LAPIC_TIMER_DIVIDE_BY_1 = 7
};

class Lapic
{

    VirtAddr mapped_registers;
    Lapic(VirtAddr lapic_addr) : mapped_registers(lapic_addr) {}

    size_t _timer_tick_in_10ms();

public:
    void write(LAPICReg reg, uint32_t value)
    {
        mapped_registers.offsetted((uint32_t)reg).vwrite<uint32_t>(value);
    }

    uint32_t read(LAPICReg reg) const
    {
        return mapped_registers.offsetted((uint32_t)reg).vread<uint32_t>();
    }

    Lapic() = default;

    static Lapic &the();

    LCpuId id() const
    {
        return read(LAPICReg::ID) >> 24;
    }

    static core::Result<void> initialize(Madt *madt);

    void eoi()
    {
        write(LAPICReg::EOI, 0);
    }

    core::Result<void> enable();

    core::Result<void> init_cpu(LCpuId id);

    core::Result<void> send_sipi(LCpuId id, PhysAddr jump_addr);

    core::Result<void> timer_initialize();

    void send_interrupt(LCpuId cpu, uint8_t vector);

};
} // namespace hw::acpi
