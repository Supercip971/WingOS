#pragma once

#include "libcore/result.hpp"
namespace arch::amd64
{

enum CpuSmpBaseAddress
{
    SMP_PAGE_TABLE = 0x500,
    SMP_START_ADDR = 0x510,
    SMP_STACK = 0x520,
    SMP_GDT = 0x530,
    SMP_IDT = 0x540,
    SMP_TRAMPOLINE_START = 0x1000
};

core::Result<void> smp_initialize();
core::Result<void> smp_initialize_cpu(int apic, int id);
} // namespace arch::amd64