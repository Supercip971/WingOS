#pragma once
#include <stdint.h>
namespace arch
{
template <int V>
struct CpuCr
{
    static inline uint64_t read()
    {
        uint64_t value;
        switch (V)
        {
        case 0:
            asm volatile("mov %%cr0, %0" : "=r"(value));
            break;
        case 2:
            asm volatile("mov %%cr2, %0" : "=r"(value));
            break;
        case 3:
            asm volatile("mov %%cr3, %0" : "=r"(value));
            break;
        case 4:
            asm volatile("mov %%cr4, %0" : "=r"(value));
            break;
        default:
            value = 0;
            break;
        }
        return value;
    }
    static inline void write(uint64_t value)
    {
        switch (V)
        {
        case 0:
            asm volatile("mov %0, %%cr0" ::"r"(value));
            break;
        case 2:
            asm volatile("mov %0, %%cr2" ::"r"(value));
            break;
        case 3:
            asm volatile("mov %0, %%cr3" ::"r"(value));
            break;
        case 4:
            asm volatile("mov %0, %%cr4" ::"r"(value));
            break;
        default:
            break;
        }
    }
};

enum Cr0Bits
{
    CR0_PROTECTED_MODE_ENABLE = (1 << 0),
    CR0_MONITOR_CO_PROCESSOR = (1 << 1),
    CR0_EMULATION = (1 << 2),
    CR0_TASK_SWITCHED = (1 << 3),
    CR0_EXTENSION_TYPE = (1 << 4),
    CR0_NUMERIC_ERROR_ENABLE = (1 << 5),
    CR0_WRITE_PROTECT_ENABLE = (1 << 16),
    CR0_ALIGNMENT_MASK = (1 << 18),
    CR0_NOT_WRITE_THROUGH_ENABLE = (1 << 29),
    CR0_CACHE_DISABLE = (1 << 30),
    CR0_PAGING_ENABLE = (1 << 31),
};

enum Cr4Bits
{
    CR4_VIRTUAL_8086_MODE_EXT = (1 << 0),
    CR4_PROTECTED_MODE_VIRTUAL_INT = (1 << 1),
    CR4_TIME_STAMP_DISABLE = (1 << 2), // disable it only for ring != 0 for RDTSC instruction
    CR4_DEBUGGING_EXT = (1 << 3),      // enable debug register break on io space
    CR4_PAGE_SIZE_EXT = (1 << 4),
    CR4_PHYSICAL_ADDRESS_EXT = (1 << 5),
    CR4_MACHINE_CHECK_EXCEPTION = (1 << 6),
    CR4_PAGE_GLOBAL_ENABLE = (1 << 7),
    CR4_PERFORMANCE_COUNTER_ENABLE = (1 << 8),
    CR4_FXSR_ENABLE = (1 << 9),
    CR4_SIMD_EXCEPTION_SUPPORT = (1 << 10), // Operating System Support for Unmasked SIMD Floating-Point Exceptions
    CR4_USER_MODE_INSTRUCTION_PREVENTION = (1 << 11),
    CR4_5_LEVEL_PAGING_ENABLE = (1 << 12),
    CR4_VIRTUAL_MACHINE_EXT_ENABLE = (1 << 13),
    CR4_SAFER_MODE_EXT_ENABLE = (1 << 14),
    CR4_FS_GS_BASE_ENABLE = (1 << 16),
    CR4_PCID_ENABLE = (1 << 17),
    CR4_XSAVE_ENABLE = (1 << 18),
    CR4_SUPERVISOR_EXE_PROTECTION_ENABLE = (1 << 20),
    CR4_SUPERVISOR_ACCESS_PROTECTION_ENABLE = (1 << 21),
    CR4_KEY_PROTECTION_ENABLE = (1 << 22),
    CR4_CONTROL_FLOW_ENABLE = (1 << 23),
    CR4_SUPERVISOR_KEY_PROTECTION_ENABLE = (1 << 24),
};

enum XCR0Bits
{
    XCR0_XSAVE_SAVE_X87 = (1 << 0),
    XCR0_XSAVE_SAVE_SSE = (1 << 1),
    XCR0_AVX_ENABLE = (1 << 2),
    XCR0_BNDREG_ENABLE = (1 << 3),
    XCR0_BNDCSR_ENABLE = (1 << 4),
    XCR0_AVX512_ENABLE = (1 << 5),
    XCR0_ZMM0_15_ENABLE = (1 << 6),
    XCR0_ZMM16_32_ENABLE = (1 << 7),
    XCR0_PKRU_ENABLE = (1 << 9),
};

struct CpuXCR0
{
    static inline uint64_t read(uint32_t a)
    {
        uint32_t eax, edx;
        asm volatile("xgetbv"

                     : "=a"(eax), "=d"(edx)
                     : "c"(a)
                     : "memory");
        return ((uint64_t)edx << 32) | eax;
    }

    static inline void write(uint32_t a, uint64_t value)
    {
        uint32_t edx = value >> 32;
        uint32_t eax = (uint32_t)value;
        asm volatile("xsetbv"
                     :
                     : "a"(eax), "d"(edx), "c"(a)
                     : "memory");
    }
};

}; // namespace arch::x86