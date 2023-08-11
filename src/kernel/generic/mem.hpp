#pragma once

#include <math/range.hpp>
#include <stdint.h>

#include <hw/mem/addr_space.hpp>
/* implemented in the bootloader entry */
/* TODO: move this to a different header */
extern "C" uintptr_t kernel_physical_base();
extern "C" uintptr_t kernel_virtual_base();

inline VirtAddr toKernel(PhysAddr addr)
{
    return VirtAddr(addr._addr + kernel_virtual_base() - kernel_physical_base());
}

inline VirtRange toKernelRange(PhysRange range)
{
    return VirtRange::from_begin_len(toKernel(range.start()), range.len());
}
