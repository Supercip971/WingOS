#pragma once

#include <stdint.h>

#include "libcore/enum-op.hpp"
#include "libcore/type/trait.hpp"

namespace arch::amd64
{

enum class MsrReg : uint32_t
{

    APIC = 0x1B,
    LAPIC_CPU_ID = 0x802,
    EFER = 0xC0000080,
    STAR = 0xC0000081,
    LSTAR = 0xC0000082,
    COMPAT_STAR = 0xC0000083,
    SYSCALL_FLAG_MASK = 0xC0000084,
    FS_BASE = 0xC0000100,
    GS_BASE = 0xC0000101,
    KERNEL_GS_BASE = 0xC0000102,

};

enum MsrApicBits : uint64_t
{
    MSR_BOOTSTRAP_CPU = 1 << 8,
    MSR_APIC_ENABLE = 1 << 11,
    MSR_X2APIC_ENABLE = 1 << 10,
};

ENUM_OP$(MsrApicBits);
struct Msr
{
    static inline uint64_t Read(MsrReg msr)
    {
        uint32_t low, high;
        asm volatile("rdmsr"
                     : "=a"(low), "=d"(high)
                     : "c"(static_cast<uint32_t>(msr)));
        return ((uint64_t)high << 32) | low;
    }

    template <typename T>
    static inline void Write(MsrReg msr, T value)
    {
        uint32_t low = static_cast<uint64_t>(value) & 0xFFFFFFFF;
        uint32_t high = static_cast<uint64_t>(value) >> 32;
        asm volatile("wrmsr"
                     :
                     : "a"(low), "d"(high), "c"(static_cast<uint32_t>(msr)));
    }
    static inline uint64_t Read(uint32_t msr)
    {
        uint32_t low, high;
        asm volatile("rdmsr"
                     : "=a"(low), "=d"(high)
                     : "c"(msr));
        return ((uint64_t)high << 32) | low;
    }

    static inline void Write(uint32_t msr, uint64_t value)
    {
        uint32_t low = value & 0xFFFFFFFF;
        uint32_t high = value >> 32;
        asm volatile("wrmsr"
                     :
                     : "a"(low), "d"(high), "c"(msr));
    }
};

} // namespace arch::amd64