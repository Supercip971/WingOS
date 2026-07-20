#include "regions.hpp"
#include <string.h>

#include "hypervisor/vmx/vmx.hpp"
#include "kernel/generic/paging.hpp"
#include "kernel/generic/pmm.hpp"

VmcsRegion allocate_vmcs_region(VmxSystemState *state)
{
    VmcsRegion region;
    region.paddr = Pmm::the().allocate(2).unwrap();

    PhysRange pr = PhysRange(region.paddr, PhysAddr(region.paddr._addr + 0x1000));

    VmmSpace::kernel_page_table().map(toVirtRange(pr), pr, PageFlags().cache_disable(true).present(true).writeable(true).user(false));
    VmmSpace::kernel_page_table().invalidate();

    region.mapped = toVirt(region.paddr).as<void *>();
    memset(region.mapped, 0, 0x1000);

    *(volatile uint32_t *)region.mapped = state->vmx_version;

    asm volatile("vmptrld %0" : : "m"(region.paddr._addr) : "memory");

    return region;
}
