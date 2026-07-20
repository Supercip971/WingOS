#include <libcore/fmt/log.hpp>
#include <string.h>

#include "arch/x86_64/msr.hpp"
#include "arch/x86_64/registers.hpp"
#include "hw/mem/addr_space.hpp"
#include "kernel/x86_64/cpu.hpp"

#include "arch/x86/cpuid.hpp"
#include "hypervisor/hypervisor.hpp"
#include "hypervisor/vmx/vmx.hpp"
#include "kernel/generic/paging.hpp"
#include "kernel/generic/pmm.hpp"
#include "libcore/fmt/flags.hpp"

bool hyper::hypervisor_suppor()
{
    return arch::x86::Cpuid::has_vmx();
}

// https://github.com/HyperDbg/HyperDbg/issues/24
void hyper::hypervisor_init()
{
    auto vm_state = arch::amd64::CpuImpl::currentImpl()->vmx();

    fmt::log$("Enabling VMX for cpu {}...", arch::amd64::CpuImpl::currentId());
    uint64_t ft_control = arch::amd64::Msr::Read(arch::amd64::MsrReg::FEATURE_CONTROL);

    // it is locked, cannot modify, is it enabled?
    if (ft_control & arch::amd64::MsrFeatControlBits::MSR_LOCKED)
    {
        if (!(ft_control & arch::amd64::MsrFeatControlBits::MSR_VMX_ENABLE))
        {
            fmt::err$("VMX is not enabled and not able to be enabled.");
            return;
        }
    }
    else
    {
        ft_control |= arch::amd64::MsrFeatControlBits::MSR_LOCKED;
        ft_control |= arch::amd64::MsrFeatControlBits::MSR_VMX_ENABLE;

        arch::amd64::Msr::Write(arch::amd64::MsrReg::FEATURE_CONTROL, ft_control);
    }

    // now enable VMX

    auto cr4 = arch::CpuCr<4>::read();
    cr4 |= arch::CR4_VIRTUAL_MACHINE_EXT_ENABLE;
    arch::CpuCr<4>::write(cr4);

    vm_state->vmx_version = arch::amd64::Msr::Read(arch::amd64::MsrReg::VMX_BASIC) & 0xFFFFFFFF; // 32 first bits

    vm_state->cr0_fixed_to_0 = arch::amd64::Msr::Read(arch::amd64::MsrReg::VM_CR0_FIXED_TO_0);
    vm_state->cr0_fixed_to_1 = arch::amd64::Msr::Read(arch::amd64::MsrReg::VM_CR0_FIXED_TO_1);
    vm_state->cr4_fixed_to_0 = arch::amd64::Msr::Read(arch::amd64::MsrReg::VM_CR4_FIXED_TO_0);
    vm_state->cr4_fixed_to_1 = arch::amd64::Msr::Read(arch::amd64::MsrReg::VM_CR4_FIXED_TO_1);

    fmt::log$("cr0_fixed_to_0: {}", vm_state->cr0_fixed_to_0 | fmt::FMT_HEX);
    fmt::log$("cr0_fixed_to_1: {}", vm_state->cr0_fixed_to_1 | fmt::FMT_HEX);
    fmt::log$("cr4_fixed_to_0: {}", vm_state->cr4_fixed_to_0 | fmt::FMT_HEX);
    fmt::log$("cr4_fixed_to_1: {}", vm_state->cr4_fixed_to_1 | fmt::FMT_HEX);
    fmt::log$("vmx_version: {}", vm_state->vmx_version | fmt::FMT_HEX);

    { // 24.11.5 -- vmxon region
        vm_state->vmx_region = Pmm::the().allocate(2).unwrap();

        PhysRange pr = PhysRange(vm_state->vmx_region, PhysAddr(vm_state->vmx_region._addr + 0x1000 * 2));

        VmmSpace::kernel_page_table().map(toVirtRange(pr), pr, PageFlags().cache_disable(true).present(true).writeable(true).user(false));
        VmmSpace::kernel_page_table().invalidate();

        vm_state->vmx_region_mapped = toVirt(vm_state->vmx_region).as<void *>();
        auto raddr = vm_state->vmx_region._addr;
        fmt::log$("vmx region {} mapped at: {}", raddr | fmt::FMT_HEX, (uintptr_t)vm_state->vmx_region_mapped | fmt::FMT_HEX);

        *(volatile uint32_t *)vm_state->vmx_region_mapped = vm_state->vmx_version;
    }
    memset(vm_state->vmx_region_mapped, 0, 0x1000);

    asm volatile("vmxon %0" : : "m"(vm_state->vmx_region._addr) : "memory");

    fmt::log$("VMX enabled.");
}
