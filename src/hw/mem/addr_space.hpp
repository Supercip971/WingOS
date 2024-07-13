#pragma once

#include <math/range.hpp>
#include <stddef.h>
#include <stdint.h>

/* it's the implementation that knows how to cast virt address to physical ones */
/* for a user app it'll allocate a virtual range and map the physical address */
/* in a kernel it's just adding an offset */
struct VirtAddr
{
    uintptr_t _addr;

    constexpr VirtAddr() = default;
    constexpr VirtAddr(uintptr_t addr) : _addr(addr) {}

    operator void *() const { return (void *)_addr; }
    template <typename T>
    constexpr const T *as() const { return (T *)_addr; }

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

    template <typename T>
    void write(T value)
    {
        *as<T>() = value;
    }

    template <typename T>
    T read() const
    {
        return *as<T>();
    }

    VirtAddr offsetted(uintptr_t offset) const
    {
        return VirtAddr(_addr + offset);
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

static_assert(sizeof(VirtAddr) == sizeof(uintptr_t));

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

static_assert(sizeof(PhysAddr) == sizeof(uintptr_t));
using PhysRange = math::Range<PhysAddr>;

// unlike the kernel toVirt function, this one
// may allocate a new virtual range for the userspace

#if __ck_kernel__
#    if defined(__x86_64)
#        define MMAP_IO_BASE (0xffff800000000000)
#    elif defined(__riscv)
#        define MMAP_IO_BASE (0)
#    else
#        error "Unsupported architecture"
#    endif
#    if defined(__x86_64)
#        define MMAP_IO_BASE (0xffff800000000000)
#    elif defined(__riscv)
#        define MMAP_IO_BASE (0)
#    else
#        error "Unsupported architecture"
#    endif
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
#else

PhysAddr toPhys(VirtAddr addr);
VirtAddr toVirt(PhysAddr addr);
VirtRange toVirtRange(PhysRange range);

#endif
