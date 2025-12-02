
#include <arch/x86/cpuid.hpp>

namespace arch::x86
{

    Cpuid::CpuidValue Cpuid::query(uint32_t leaf, uint32_t subleaf)
    {
        CpuidValue result = {};

        uint32_t eax, ebx, ecx, edx;
        asm volatile("cpuid"
                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                     : "a"(leaf), "c"(subleaf));

        result.eax = eax;
        result.ebx = ebx;
        result.ecx = ecx;
        result.edx = edx;

        return result;
    }

}  // namespace arch::x86