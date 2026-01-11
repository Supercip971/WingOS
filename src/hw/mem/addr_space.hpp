#pragma once

#include <math/range.hpp>
#include <stddef.h>
#include <stdint.h>


struct Pages
{
    uint64_t _count = 0;

    static constexpr uint64_t size = 4096;
    constexpr Pages() = default;
    constexpr Pages(uint64_t count) : _count(count) {}

    uint64_t count() const { return _count; }
    uint64_t bytes() const { return _count * Pages::size; }

    constexpr size_t operator+(const Pages &other) const { return (_count + other._count); }

    constexpr size_t operator-(const Pages &other) const { return (_count - other._count); }

    constexpr bool operator<(const Pages &other) const { return _count < other._count; }


    static constexpr Pages from_bytes_floored(size_t bytes)
    {
        return Pages(bytes / Pages::size);
    }
    static constexpr Pages from_bytes_ceil(size_t bytes)
    {
        return Pages((bytes + Pages::size - 1) / Pages::size);
    }



};
/* it's the implementation that knows how to cast virt address to physical ones */
/* for a user app it'll allocate a virtual range and map the physical address */
/* in a kernel it's just adding an offset */
struct VirtAddr
{
    uintptr_t _addr = 0;

    constexpr VirtAddr() = default;
    constexpr VirtAddr(uintptr_t addr) : _addr(addr) {}

    operator void *() const { return (void *)_addr; }

    template <typename T>
    constexpr T *as(void) const { return (T *)_addr; }

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

    // technically it's like a pointer, the pointer is constant
    // but not the value it points to
    template <typename T>
    void write(T value)
    {
        *((T*)_addr) = value;
    }

    template <typename T>
    T read() const
    {

        return *((T*)_addr);
    }

    template <typename T>
    void vwrite(T value)
    {
        (*(volatile T*)_addr) = value;
    }

    template <typename T>
    T vread() const
    {
        return (*(volatile T*)_addr);
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

static constexpr inline auto addrOf(auto *ptr)
{
    return VirtAddr(reinterpret_cast<uintptr_t>(ptr));
}

static_assert(sizeof(VirtAddr) == sizeof(uintptr_t));

using VirtRange = math::Range<VirtAddr>;

struct [[gnu::packed]] PhysAddr
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
#include <iol/wingos/space.hpp>
constexpr inline PhysAddr toPhys(VirtAddr addr)
{
    // in userspace we assume the mapping is direct for now
    return PhysAddr(addr._addr - USERSPACE_VIRT_BASE);
}
constexpr inline VirtAddr toVirt(PhysAddr addr)
{
    return VirtAddr(addr._addr + USERSPACE_VIRT_BASE);
}
constexpr inline VirtRange toVirtRange(PhysRange range)
{
    return VirtRange(toVirt(range.start()), toVirt(range.end()));
}

#endif

template <typename Fn>
concept MappCallbackFn = requires(uintptr_t addr, size_t size, Fn fn) {
    { fn(addr, size) } -> core::IsConvertibleTo<core::Result<uintptr_t>>;
};
