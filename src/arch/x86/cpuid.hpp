#pragma once
#include <stdint.h>
namespace arch::x86
{

// doc: https://www.amd.com/system/files/TechDocs/25481.pdf

struct Cpuid
{

    typedef struct
    {
        uint32_t eax;
        uint32_t ebx;
        uint32_t ecx;
        uint32_t edx;
    } CpuidValue;

    enum CpuidLeaf
    {
        CPUID_FEATURE_IDENTIFIER = 1,
        CPUID_EXTENDED_FEATURE_IDENTIFIER = 7,
        CPUID_PROC_EXTENDED_STATE_ENUMERATION = 13
    };

    enum CpuidFeatureBits
    {
        // ECX
        CPUID_SSSE3_SUPPORT = (1 << 9),
        CPUID_SSE41_SUPPORT = (1 << 19),
        CPUID_SSE42_SUPPORT = (1 << 20),
        CPUID_AES_SUPPORT = (1 << 25),
        CPUID_XSAVE_SUPPORT = (1 << 26),
        CPUID_XSAVE_ENABLED = (1 << 27),
        CPUID_AVX_SUPPORT = (1 << 28),
    };

    enum CpuidExtFeatureBits
    {
        // EBX
        CPUID_BIT_MANIPULATION_SUPPORT = (1 << 3),
        CPUID_AVX512_SUPPORT = (1 << 16),
    };

    static CpuidValue query(uint32_t leaf, uint32_t subleaf);

    static inline bool has_xsave(void)
    {
        return (query(CPUID_FEATURE_IDENTIFIER, 0).ecx & CPUID_XSAVE_SUPPORT) == CPUID_XSAVE_SUPPORT;
    }

    static inline bool has_avx(void)
    {
        return query(CPUID_FEATURE_IDENTIFIER, 0).ecx & CPUID_AVX_SUPPORT;
    }

    static inline bool has_avx512(void)
    {
        return query(CPUID_EXTENDED_FEATURE_IDENTIFIER, 0).ebx & CPUID_AVX512_SUPPORT;
    }

    static inline uint32_t xsave_size(void)
    {
        return query(CPUID_PROC_EXTENDED_STATE_ENUMERATION, 0).ecx;
    }
};
} // namespace arch::x86