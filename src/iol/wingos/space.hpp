#pragma once

#include <stdint.h>

#include "iol/wingos/asset.hpp"
#include "iol/wingos/ipc.hpp"
#include "iol/wingos/syscalls.h"
#include "libcore/fmt/log.hpp"
#include "wingos-headers/asset.h"

#define USERSPACE_VIRT_BASE 0x0000002000000000

namespace Wingos
{
struct Space
{
    AssetHandle handle; // the handle of the space

    static Space self()
    {
        Space space = {};
        space.handle = AssetHandle::selfSpace();
        return space;
    }

    static Space from_uid(AssetHandle uid)
    {
        Space space = {};
        space.handle = uid; // the handle of the space
        return space;
    }

    bool is_self() const
    {
        return handle == AssetHandle::selfSpace(); // self space handle is 0
    }

    MemoryAsset allocate_physical_memory(uint64_t size, bool lower_half = false)
    {
        return MemoryAsset::allocate(handle, size, lower_half);
    }

    MemoryAsset own_memory_physical(uint64_t addr, uint64_t size)
    {

        return MemoryAsset::own(handle, addr, size);
    }

    VirtualMemoryAsset create_virtual_memory(uint64_t start, uint64_t end, AssetHandle physical_mem_handle, uint64_t flags)
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
        if (phys.handle == AssetHandle::invalid())
        {
            fmt::err$("failed to own physical memory: {}", phys.handle.id());
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
        sys$asset_release_mem(ptr, (void *)(size + (uintptr_t)ptr));
    }

    void release_asset(UAsset asset)
    {
        sys$asset_release(handle, asset.handle);
    }

    Space create_space()
    {
        auto space_res = sys$space_create(handle, 0, 0);
        if (space_res.returned_handle == AssetHandle::invalid())
        {
            fmt::err$("failed to create space: {}", space_res.returned_handle.id());
            return Space::self();
        }
        return Space::from_uid(space_res.returned_handle);
    }

    TaskAsset create_task(uint64_t launch, uint64_t arg1 = 0, uint64_t arg2 = 0, uint64_t arg3 = 0, uint64_t arg4 = 0)
    {
        auto task_res = sys$task_create(handle, launch, arg1, arg2, arg3, arg4);
        if (task_res.returned_handle == AssetHandle::invalid())
        {
            fmt::err$("failed to create task: {}", task_res.returned_handle.id());
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

    UAsset _move_to(Space to, AssetHandle moved_handle)
    {
        auto move_res = sys$asset_move(handle, to.handle, moved_handle);
        if (move_res.returned_handle_in_space == AssetHandle::invalid())
        {
            fmt::err$("failed to move asset: {}", move_res.returned_handle_in_space.id());
            return {};
        }

        UAsset moved_asset = {};
        moved_asset.handle = move_res.returned_handle_in_space;
        return moved_asset;
    }

    template <typename T>
    T move_to(Space to, const T &asset)
    {
        auto moved_asset = _move_to(to, asset.handle);
        if (moved_asset.handle == AssetHandle::invalid())
        {
            fmt::err$("failed to move asset: {}", moved_asset.handle.id());
            return T();
        }

        T copy = asset;
        copy.handle = moved_asset.handle;
        return copy;
    }

    RawIpcEndpoint create_public_ipc_server(bool is_root = false)
    {
        return RawIpcEndpoint::create(handle, true, is_root);
    }

    RawIpcEndpoint create_private_ipc_server()
    {
        return RawIpcEndpoint::create(handle, false);
    }

    IpcClient connect_by_addr(uint64_t endpoint_address)
    {
        return IpcClient::connect_by_addr(handle, endpoint_address);
    }

    IpcClient from_already_connected(AssetHandle endpoint_handle)
    {
        return IpcClient::already_connected(handle, endpoint_handle);
    }

    IpcClient connect_by_handle(AssetHandle endpoint_handle)
    {
        return IpcClient::connect_to_object(handle, endpoint_handle);
    }

    size_t iterate_through_assets(auto fn)
    {
        auto asset_count = sys$ipc_asset_info_by_handle(this->handle, 0).returned_info.space.element_count;

        for (size_t i = 0; i < asset_count; i++)
        {
            auto asset_info = sys$ipc_asset_info_by_index(this->handle, i);
            if (asset_info.returned_kind != 0)
            {
                fn(asset_info);
            }
        }
        return asset_count;
    }
};
} // namespace Wingos
