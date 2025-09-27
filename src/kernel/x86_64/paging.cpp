#include <kernel/generic/paging.hpp>

#include "iol/mem_flags.h"
#include <arch/x86_64/paging.hpp>

#include "kernel/generic/mem.hpp"
#include "libcore/fmt/flags.hpp"

static VmmSpace kernel_space;
void VmmSpace::use_kernel(VmmSpace &space)
{
    kernel_space = space;
}

VmmSpace &VmmSpace::kernel_page_table()
{
    return kernel_space;
}
arch::amd64::Pml4 *as_root(VmmSpace &space)
{
    return reinterpret_cast<arch::amd64::Pml4 *>(space.self());
};

constexpr arch::amd64::Page page_create(PageFlags flags, PhysAddr addr)
{
    auto p = arch::amd64::Page{
        ._present = 1,
        ._writeable = flags._writeable,
        ._user = flags._user,
        ._write_through = flags._write_through,
        ._cache_disabled = flags._cache_disable,
        ._accessed = 0,
        ._dirty = 0,
        ._huge_page = 0,
        ._global = 0,
        ._available = 0,
        ._address = 0,
    };
    p.address(addr);
    return p;
}

void VmmSpace::use()
{

    asm volatile("mov %0, %%cr3" ::"r"(self_addr()));
}
core::Result<void> VmmSpace::map(VirtRange virt, PhysRange phys, PageFlags flags)
{
    auto root = as_root(*this);

    if (virt.len() != phys.len()) [[unlikely]]
    {
        log::err$("VmmSpace::map: virtual and physical ranges must be of equal length");
        log::err$("virt: {}, phys: {}", virt.start()._addr | fmt::FMT_HEX, phys.start()._addr | fmt::FMT_HEX);
        log::err$("virt len: {}, phys len: {}", virt.len() | fmt::FMT_HEX, phys.len() | fmt::FMT_HEX);
        return "Virtual and physical ranges must be of equal length";
    }

    if (virt.len() % arch::amd64::PAGE_SIZE != 0) [[unlikely]]
    {
        log::err$("VmmSpace::map: virtual and physical ranges must be page aligned");
        return "Virtual and physical ranges must be page aligned";
    }
    auto virt_begin = virt.start();
    auto phys_begin = phys.start();

    // TODO: map 1meg page when possible
    for (size_t i = 0; i < virt.len(); i += arch::amd64::PAGE_SIZE)
    {
        auto virt_addr = virt_begin + i;
        auto phys_addr = phys_begin + i;

        try$(root->map(virt_addr, page_create(flags, phys_addr)));
    }

    return {};
}
core::Result<void> VmmSpace::unmap(VirtRange virt)
{
    auto root = as_root(*this);

    if (virt.len() % arch::amd64::PAGE_SIZE != 0 || virt.start()._addr % arch::amd64::PAGE_SIZE != 0) [[unlikely]]
    {
        return "Virtual and physical ranges must be page aligned";
    }

    auto virt_begin = virt.start();
    for (size_t i = 0; i < virt.len(); i += arch::amd64::PAGE_SIZE)
    {
        auto virt_addr = virt_begin + i;

        try$(root->unmap(virt_addr));
    }

    return {};
}

core::Result<VmmSpace> VmmSpace::create(bool empty)
{
    VmmSpace result = {};
    // allocate to lower half because SMP need the pagetable address to be in the lower 4GB
    result._self = toVirt(try$(Pmm::the().allocate(1, IOL_ALLOC_MEMORY_FLAG_LOWER_SPACE))).as<void *>();

    log::log$("VmmSpace::create: new space at {}", (uintptr_t)result._self | fmt::FMT_HEX);
    auto root = as_root(result);
    auto kernel_root = as_root(kernel_space);
    for (size_t i = 0; i < arch::amd64::PAGE_TABLE_SIZE; i++)
    {
        // map the IO kernel space
        if (!empty && i >= 255)
        {
            root->page(i) = kernel_root->page(i);
        }
        else
        {
            root->page(i) = arch::amd64::Page{};
        }
    }

    return result;
}

core::Result<void> VmmSpace::verify(VirtAddr virt, size_t size)
{
    auto root = as_root(*this);

    return root->verify(virt, size);
}