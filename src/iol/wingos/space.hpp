#pragma once

#include <stdint.h>

#include "iol/wingos/asset.hpp"
#include "iol/wingos/ipc.hpp"
#include "libcore/fmt/log.hpp"

#define USERSPACE_VIRT_BASE 0x0000002000000000
namespace Wingos
{
struct Space
{
    uint64_t handle; // the handle of the space
    static Space self()
    {
        Space space;
        space.handle = 0; // self space handle is 0
        return space;
    }

    static Space from_uid(uint64_t uid)
    {
        Space space;
        space.handle = uid; // the handle of the space
        return space;
    }

    bool is_self() const
    {
        return handle == 0; // self space handle is 0
    }

    MemoryAsset allocate_physical_memory(uint64_t size, bool lower_half = false)
    {
        return MemoryAsset::allocate(handle, size, lower_half);
    }

    MemoryAsset own_memory_physical(uint64_t addr, uint64_t size)
    {

        return MemoryAsset::own(handle, addr, size);
    }

    VirtualMemoryAsset create_virtual_memory(uint64_t start, uint64_t end, uint64_t physical_mem_handle, uint64_t flags)
    {
        return VirtualMemoryAsset::create(handle, start, end, physical_mem_handle, flags);
    }

    VirtualMemoryAsset map_memory(MemoryAsset mem_asset, uint64_t flags)
    {
        return VirtualMemoryAsset::create(handle, mem_asset.memory.start() + USERSPACE_VIRT_BASE, mem_asset.memory.end() + USERSPACE_VIRT_BASE, mem_asset.handle, flags);
    }

    VirtualMemoryAsset map_memory(uint64_t start, uint64_t end, MemoryAsset mem_asset, uint64_t flags)
    {

        return VirtualMemoryAsset::create(handle, start, end, mem_asset.handle, flags);
    }

    VirtualMemoryAsset map_physical_memory(uint64_t start, uint64_t len, uint64_t flags)
    {
        auto phys = own_memory_physical(start, len);
        if (phys.handle == 0)
        {
            log::err$("failed to own physical memory: {}", phys.handle);
            return {};
        }
        return VirtualMemoryAsset::create(handle, start + USERSPACE_VIRT_BASE, start + phys.memory.len() + USERSPACE_VIRT_BASE, phys.handle, flags);
    }

    VirtualMemoryAsset allocate_memory(uint64_t size, bool lower_half = false)
    {
        auto mem_asset = allocate_physical_memory(size, lower_half);
        return VirtualMemoryAsset::create(handle, mem_asset.memory.start() + USERSPACE_VIRT_BASE, mem_asset.memory.end() + USERSPACE_VIRT_BASE, mem_asset.handle, ASSET_MAPPING_FLAG_WRITE | ASSET_MAPPING_FLAG_EXECUTE);
    }

    void release_memory(void *ptr, size_t size)
    {
        sys$asset_release_mem(ptr, (void*)(size + (uintptr_t)ptr));
    }

    Space create_space()
    {
        auto space_res = sys$space_create(handle, 0, 0);
        if (space_res.returned_handle == 0)
        {
            log::err$("failed to create space: {}", space_res.returned_handle);
            return Space::self();
        }
        return Space::from_uid(space_res.returned_handle);
    }

    TaskAsset create_task(uint64_t launch, uint64_t arg1 = 0, uint64_t arg2 = 0, uint64_t arg3 = 0, uint64_t arg4 = 0)
    {
        auto task_res = sys$task_create(handle, launch, arg1, arg2, arg3, arg4);
        if (task_res.returned_handle == 0)
        {
            log::err$("failed to create task: {}", task_res.returned_handle);
            return TaskAsset();
        }
        TaskAsset task_asset;
        task_asset.handle = task_res.returned_handle;
        task_asset.launch_addr = launch;

        task_asset.args[0] = arg1;
        task_asset.args[1] = arg2;
        task_asset.args[2] = arg3;
        task_asset.args[3] = arg4;

        return task_asset;
    }

    void launch_task(TaskAsset asset)
    {
        sys$task_launch(handle, asset.handle, asset.args[0], asset.args[1], asset.args[2], asset.args[3]);
    }

    UAsset _move_to(Space to, uint64_t moved_handle)
    {
        auto move_res = sys$asset_move(handle, to.handle, moved_handle);
        if (move_res.returned_handle_in_space == 0)
        {
            log::err$("failed to move asset: {}", move_res.returned_handle_in_space);
            return {};
        }
        UAsset moved_asset = {moved_handle};
        moved_asset.handle = move_res.returned_handle_in_space;
        return moved_asset;
    }

    template <typename T>
    T move_to(Space to, const T &asset)
    {
        auto moved_asset = _move_to(to, asset.handle);
        if (moved_asset.handle == 0)
        {
            log::err$("failed to move asset: {}", moved_asset.handle);
            return T();
        }

        T copy = asset;
        copy.handle = moved_asset.handle;
        return copy;
    }

    IpcServer create_ipc_server(bool is_root = false)
    {
        return IpcServer::create(handle, is_root);
    }

    IpcClient connect_to_ipc_server(uint64_t server_address, bool block = false, uint64_t flags = 0)
    {
        return IpcClient::connect(handle, server_address, block, flags);
    }
};
} // namespace Wingos