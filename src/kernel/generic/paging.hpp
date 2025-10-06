#pragma once
#include <libcore/result.hpp>
#include <mcx/mcx.hpp>

#include "mem.hpp"

struct PageFlags
{
    bool _present : 1;
    bool _writeable : 1;
    bool _executable : 1;
    bool _user : 1;
    bool _write_through : 1;
    bool _cache_disable : 1;
    // if this is set, the page is not mapped to a physical page
    // but instead to a custom handler
    bool _custom_map_handler : 1;

    constexpr PageFlags() {};

    constexpr PageFlags present(bool present) const
    {
        PageFlags flags = *this;
        flags._present = present;
        return flags;
    }

    constexpr PageFlags writeable(bool writeable) const
    {
        PageFlags flags = *this;
        flags._writeable = writeable;
        return flags;
    }

    constexpr PageFlags executable(bool executable) const
    {
        PageFlags flags = *this;
        flags._executable = executable;
        return flags;
    }

    constexpr PageFlags user(bool user) const
    {
        PageFlags flags = *this;
        flags._user = user;
        return flags;
    }

    constexpr PageFlags write_through(bool write_through) const
    {
        PageFlags flags = *this;
        flags._write_through = write_through;
        return flags;
    }

    constexpr PageFlags cache_disable(bool cache_disable) const
    {
        PageFlags flags = *this;
        flags._cache_disable = cache_disable;
        return flags;
    }
};

class VmmSpace
{
    void *_self;

public:
    PhysAddr self_addr() const { return toPhys(VirtAddr((uintptr_t)self())); }
    void const *self() const { return _self; }

    void *self() { return _self; }
    static VmmSpace &kernel_page_table();

    static core::Result<VmmSpace> kernel_initialize(const mcx::MachineContext *ctx);
    // ------- external: implemented in kernel/{ARCH}  -------
    void use();

    core::Result<void> map(VirtRange virt, PhysRange phys, PageFlags flags);

    core::Result<void> unmap(VirtRange virt, bool user);

    core::Result<void> virtual_map(VirtAddr virt, size_t count, PageFlags flags, size_t object_rid);

    core::Result<PhysAddr> get_phys(VirtAddr virt);

    core::Result<void> verify(VirtAddr virt, size_t size);

    static void invalidate_address(VirtAddr virt)
    {
        asm volatile("invlpg (%0)" ::"r"(virt._addr) : "memory");
    }

    // empty: means that the higher half won't be copied from the kernel
    // this is used for the kernel page table

    static core::Result<VmmSpace> create(bool empty);

    static void invalidate()
    {
        asm volatile("mov %cr3, %rax; mov %rax, %cr3");
    }

    static void use_kernel(VmmSpace &space);
};