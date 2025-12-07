#include <string.h>
#include "arch/x86_64/paging.hpp"
#include "arch/x86_64/registers.hpp"
#include <arch/x86_64/simd.hpp>

#include "arch/x86/cpuid.hpp"
#include "kernel/generic/pmm.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/lock/lock.hpp"



static __attribute__((aligned(4096))) uint8_t
    initial_context_data[4096] = {0};

static core::Lock simd_init_lock = {};
namespace arch::x86_64
{

void SimdContext::save()
{
    if (_data == nullptr)
    {
        return;
    }

    if (_use_xsave)
    {
        asm volatile("xsave64 %0" :"+m"(*_data):  "a"(~(uintptr_t)0), "d"(~(uintptr_t)0)
                     : "memory");


    }
    else
    {
        asm volatile("fxsave %0" : "+m"(*_data) :
                     :  "memory");


    }
}

void SimdContext::load() const
{
    if (_data == nullptr)
    {
        return;
    }

    if (_use_xsave)
    {
        asm volatile("xrstor64 %0" : : "m"(*_data),
                     "a"(~(uintptr_t)0), "d"(~(uintptr_t)0)
                     : "memory");
    }
    else
    {
        asm volatile("fxrstor %0" : : "m"(*_data) : "memory");
    }
}

core::Result<SimdContext> SimdContext::create()
{

    SimdContext context = SimdContext();

    if (x86::Cpuid::has_xsave())
    {
        context._use_xsave = true;
        context._data_size = x86::Cpuid::xsave_size();
    }
    else
    {
        context._use_xsave = false;
        context._data_size = 512; // fxsave size
    }

    context._real_data = toVirt(try$(Pmm::the().allocate(math::alignUp<uint64_t>(context._data_size, amd64::PAGE_SIZE) / amd64::PAGE_SIZE)));
    
    // Align to 64-byte boundary (required for xsave, fxsave only needs 16)
    context._data = (uint8_t*)context._real_data;

    if (!context._data)
    {
        return core::Result<SimdContext>::error("failed to allocate simd context");
    }

    memcpy((void*)context._data, (void*)initial_context_data, context._data_size);

    return core::Result<SimdContext>::success(core::move(context));
}

core::Result<void> SimdContext::initialize_cpu()
{
    simd_init_lock.lock();
    arch::CpuCr<0>::write(arch::CpuCr<0>::read() & ~((uint64_t)CR0_EMULATION));
    arch::CpuCr<0>::write(arch::CpuCr<0>::read() | ((uint64_t)CR0_MONITOR_CO_PROCESSOR));
    arch::CpuCr<0>::write(arch::CpuCr<0>::read() | ((uint64_t)CR0_NUMERIC_ERROR_ENABLE));

    arch::CpuCr<4>::write(arch::CpuCr<4>::read() | ((uint64_t)CR4_FXSR_ENABLE));
    arch::CpuCr<4>::write(arch::CpuCr<4>::read() | ((uint64_t)CR4_SIMD_EXCEPTION_SUPPORT));

    if (x86::Cpuid::has_xsave())
    {

        log::log$("xsave supported, enabling it");
        arch::CpuCr<4>::write(arch::CpuCr<4>::read() | ((uint64_t)CR4_XSAVE_ENABLE));

        uint64_t xcr0 = 0;
        xcr0 |= XCR0_XSAVE_SAVE_X87;
        xcr0 |= XCR0_XSAVE_SAVE_SSE;

        if (x86::Cpuid::has_avx())
        {
            log::log$("avx supported, enabling it");
            xcr0 |= XCR0_AVX_ENABLE;
        }

        if (x86::Cpuid::has_avx512())
        {
            log::log$("avx512 supported, enabling it");
            xcr0 |= XCR0_AVX512_ENABLE;
            xcr0 |= XCR0_ZMM0_15_ENABLE;
            xcr0 |= XCR0_ZMM16_32_ENABLE;
        }

        arch::CpuXCR0::write(0, xcr0);
    }

    asm volatile("fninit" ::: "memory");
    if (x86::Cpuid::has_xsave())
    {
        log::log$("context size: {}", x86::Cpuid::xsave_size());
        asm volatile("xsave64 %0" : "+m"(*initial_context_data) : "a"(~(uintptr_t)0), "d"(~(uintptr_t)0)
                     : "memory");
    }
    else
    {
        log::log$("context size: 512 (fxsave)");
        asm volatile("fxsave %0" : "+m"(*(uint8_t(*)[512])initial_context_data)
                     : : "memory");

    }

    simd_init_lock.release();
    return {};
}

} // namespace arch::x86_64
