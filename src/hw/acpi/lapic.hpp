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

class Lapic
{

    VirtAddr mapped_registers;
    Lapic(VirtAddr lapic_addr) : mapped_registers(lapic_addr) {}

public:
    void write(LAPICReg reg, uint32_t value)
    {
        mapped_registers.offsetted((uint32_t)reg).write<uint32_t>(value);
    }

    uint32_t read(LAPICReg reg) const
    {
        return mapped_registers.offsetted((uint32_t)reg).read<uint32_t>();
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
};
} // namespace hw::acpi
