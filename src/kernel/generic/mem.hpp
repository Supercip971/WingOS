#pragma once

#include <math/range.hpp>
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

    constexpr VirtAddr &operator+=(uintptr_t offset)
    {
        _addr += offset;
        return *this;
    }

    constexpr VirtAddr &operator-=(uintptr_t offset)
    {
        _addr -= offset;
        return *this;
    }

    constexpr VirtAddr operator+(uintptr_t offset) const { return VirtAddr(_addr + offset); }
    constexpr VirtAddr operator-(uintptr_t offset) const { return VirtAddr(_addr - offset); }
    constexpr size_t operator+(const VirtAddr &other) const { return (_addr + other._addr); }
    constexpr size_t operator-(const VirtAddr &other) const { return (_addr - other._addr); }

    constexpr bool operator<(const VirtAddr &other) const { return _addr < other._addr; }
    constexpr bool operator>(const VirtAddr &other) const { return _addr > other._addr; }
    constexpr bool operator<=(const VirtAddr &other) const { return _addr <= other._addr; }
    constexpr bool operator>=(const VirtAddr &other) const { return _addr >= other._addr; }
    constexpr bool operator==(const VirtAddr &other) const { return _addr == other._addr; }
    constexpr bool operator!=(const VirtAddr &other) const { return _addr != other._addr; }

    constexpr explicit operator uintptr_t(void) const { return _addr; }
};

using VirtRange = math::Range<VirtAddr>;

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

    constexpr size_t operator+(const PhysAddr &other) const { return (_addr + other._addr); }
    constexpr size_t operator-(const PhysAddr &other) const { return (_addr - other._addr); }

    constexpr bool operator<(const PhysAddr &other) const { return _addr < other._addr; }
    constexpr bool operator>(const PhysAddr &other) const { return _addr > other._addr; }
    constexpr bool operator<=(const PhysAddr &other) const { return _addr <= other._addr; }
    constexpr bool operator>=(const PhysAddr &other) const { return _addr >= other._addr; }
    constexpr bool operator==(const PhysAddr &other) const { return _addr == other._addr; }
    constexpr bool operator!=(const PhysAddr &other) const { return _addr != other._addr; }
};

using PhysRange = math::Range<PhysAddr>;

/* implemented in the bootloader entry */
/* TODO: move this to a different header */
extern "C" uintptr_t kernel_physical_base();
extern "C" uintptr_t kernel_virtual_base();

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
constexpr inline VirtRange toVirtRange(PhysRange range)
{
    return VirtRange(toVirt(range.start()), toVirt(range.end()));
}

inline VirtAddr toKernel(PhysAddr addr)
{
    return VirtAddr(addr._addr + kernel_virtual_base() - kernel_physical_base());
}

inline VirtRange toKernelRange(PhysRange range)
{
    return VirtRange::from_begin_len(toKernel(range.start()), range.len());
}
