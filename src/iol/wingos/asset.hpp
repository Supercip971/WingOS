#pragma once

#include <stdint.h>

#include "iol/wingos/syscalls.h"
#include "mcx/mcx.hpp"
#include "wingos-headers/syscalls.h"
namespace Wingos
{

struct UAsset
{
    uint64_t handle;
};

struct VirtualMemoryAsset : public UAsset
{

    mcx::MemoryRange memory; // the virtual memory range of the asset

    static VirtualMemoryAsset create(uint64_t space_handle, uint64_t start, uint64_t end, uint64_t physical_mem_handle, uint64_t flags)
    {
        VirtualMemoryAsset asset;
        asset.memory = mcx::MemoryRange(start, end).growAlign(4096);
        asset.handle = sys$map_create(space_handle, asset.memory.start(), asset.memory.end(), physical_mem_handle, flags).returned_handle;
        return asset;
    }

    void *ptr() const
    {
        return (void *)memory.start();
    }
};

struct MemoryAsset : public UAsset
{

    mcx::MemoryRange memory;

    bool allocated; // if true, the memory has been allocated by the kernel

    static MemoryAsset allocate(uint64_t space_handle, uint64_t size, [[maybe_unused]] bool lower_half = false)
    {
        MemoryAsset asset;

        size = math::alignUp(size, 4096ul); // align to page size
        auto phys_mem = sys$mem_own(space_handle, size, 0);

        asset.memory = mcx::MemoryRange::from_begin_len(phys_mem.addr, size);
        asset.handle = phys_mem.returned_handle;

        asset.allocated = true; // will be set to true when the memory is allocated
        return asset;
    }

    static MemoryAsset own(uint64_t space_handle, uint64_t addr, uint64_t size)
    {
        MemoryAsset asset;
        asset.memory = mcx::MemoryRange::from_begin_len(addr, size).growAlign(4096);

        asset.handle = sys$mem_own(space_handle, asset.memory.len(), asset.memory.start()).returned_handle;
        asset.allocated = false; // will be set to false when the memory is not allocated by the kernel
        return asset;
    }
};
struct TaskAsset : public UAsset
{
    uint64_t launch_addr;
    uint64_t args[4];

    static TaskAsset create(uint64_t space_handle, uint64_t launch_addr, uint64_t args[4])
    {
        TaskAsset asset;
        asset.launch_addr = launch_addr;

        asset.args[0] = args[0];
        asset.args[1] = args[1];
        asset.args[2] = args[2];
        asset.args[3] = args[3];

        asset.handle = sys$task_create(space_handle, launch_addr, args[0], args[1], args[2], args[3]).returned_handle;
        return asset;
    }
};

}; // namespace Wingos
