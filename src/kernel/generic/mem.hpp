#pragma once

#include <stdint.h>
struct VirtAddr
{
    uintptr_t _addr;

    constexpr VirtAddr() = default;
    constexpr VirtAddr(uintptr_t addr) : _addr(addr) {}

    operator void *(void) const { return (void *)_addr; }
    template <typename T>
    constexpr const T *as(void) const { return (T *)_addr; }

    template <typename T>
    constexpr T *as(void) { return (T *)_addr; }

    constexpr operator uintptr_t(void) const { return _addr; }
};

struct PhysAddr
{
    uintptr_t _addr;

    constexpr PhysAddr() = default;
    constexpr PhysAddr(uintptr_t addr) : _addr(addr) {}
    constexpr operator uintptr_t(void) const { return _addr; }

    constexpr PhysAddr &operator+=(uintptr_t offset)
    {
        _addr += offset;
        return *this;
    }
    constexpr PhysAddr &operator-=(uintptr_t offset)
    {
        _addr -= offset;
        return *this;
    }

    constexpr PhysAddr operator+(uintptr_t offset) const { return PhysAddr(_addr + offset); }
    constexpr PhysAddr operator-(uintptr_t offset) const { return PhysAddr(_addr - offset); }
};

#if defined(__x86_64)
#    define MMAP_IO_BASE (0xffff800000000000)
#    define MMAP_KERNEL_BASE (0xffffffff80000000)
#elif defined(__riscv)
#    define MMAP_IO_BASE (0)
#    define MMAP_KERNEL_BASE (0)
#else
#    error "Unsupported architecture"
#endif
constexpr inline PhysAddr toPhys(VirtAddr addr)
{
    return PhysAddr(addr._addr - MMAP_IO_BASE);
}
constexpr inline VirtAddr toVirt(PhysAddr addr)
{
    return VirtAddr(addr._addr + MMAP_IO_BASE);
}
