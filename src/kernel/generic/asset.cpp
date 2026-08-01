#include "asset.hpp"
#include <atomic>

#include "arch/x86_64/paging.hpp"
#include "hw/mem/addr_space.hpp"
#include "iol/mem_flags.h"
#include "kernel/generic/asset_types.hpp"
#include "kernel/generic/ipc_asset.hpp"
#include "kernel/generic/ipc_registry.hpp"
#include <arch/x86_64/barrier.hpp>

// Make sure IPC public types (e.g. IpcServerHandle) are visible before this TU
// pulls in headers that depend on them transitively.
#include "kernel/generic/mem.hpp"
#include "kernel/generic/space.hpp"
#include "kernel/generic/task.hpp"
#include "libcore/fmt/log.hpp"
#include "libcore/result.hpp"
#include "math/align.hpp"
#include "mcx/mcx.hpp"
#include "wingos-headers/asset.h"

void Asset::own(Asset *asset)
{
    if (asset != nullptr)
    {
        asset->ref_count.fetch_add(1, std::memory_order_relaxed);
    }
}

void Asset::release(Asset *asset)
{

    if (asset == nullptr)
    {
        return;
    }

    asset->lock.lock();

    AssetRef<> saved_self{};
    asset->lock.release();
    // `saved_self` destructor runs at end of scope
}

uintptr_t last_asset = 0;

void Asset::deref(Asset *asset)
{
    if (asset == nullptr)
    {
        return;
    }

    // Atomically decrement ref_count and get the previous value
    size_t old_count = asset->ref_count.fetch_sub(1, std::memory_order_acq_rel);

    if (old_count == 0)
    {
        // This means ref_count was already 0 before we decremented restore and bail
        //     asset->ref_count.fetch_add(1, std::memory_order_relaxed);
        fmt::err$("asset_release: asset is already released");
        fmt::err$("double free detected");
        return;
    }

    if (old_count == 1)
    {
        // We decremented from 1 to 0, so we own destruction
        // Lock to ensure exclusive access during cleanup
        asset->lock.lock();

        //   fmt::log$("freeing asset");
        if (asset->kind == OBJECT_KIND_MEMORY)
        {
            auto mem = asset->casted<AssetMemory>();
            if (mem->allocated)
            {
                Pmm::the().release(PhysAddr{mem->addr}, mem->size / 4096);
            }
        }
        else if (asset->kind == OBJECT_KIND_MAPPING)
        {
            auto map = asset->casted<AssetMapping>();
            map->physical_mem = AssetRef<AssetMemory>{}; // deref physical mem
        }

        else if (asset->kind == OBJECT_KIND_IPC_CONNECTION)
        {

            //            fmt::log$("({}) destroying connection: {}", Cpu::currentId(), (uintptr_t)asset | fmt::FMT_HEX);

            auto conn = asset->casted<kernel::IpcEndpointConnection>();
            conn->connection_to = {};

            // conn->connection->message_sent.release();
            // Note: removal from server's connections list already happened above
            // before ref_count was decremented, to prevent dangling pointers

            // Check if we should auto-release the server when last connection is gone
            // Use query_server_locked to safely access the server (avoids use-after-free)

            // If query failed, server was already unregistered - nothing to do
        }

        else if (asset->kind == OBJECT_KIND_IPC_ENDPOINT)
        {
            auto server = asset->casted<kernel::IpcEndpoint>();
            server->awaiting_server = {};
            server->target_message_space = {};
            server->~IpcEndpoint();
        }
        else if (asset->kind == OBJECT_KIND_SPACE)
        {
            // auto sp = asset->casted<Space>();

            fmt::warn$("asset_release: space asset is not supported yet");
            // Check if we should auto-release the space when last connection is gone

            // sp->vmm_space.release();
        }
        else if (asset->kind == OBJECT_KIND_TASK)
        {
            fmt::warn$("asset_release: task asset is not supported yet");
        }

        // Don't release the lock before deleting - the asset is about to be freed
        // and no other thread should access it. The lock will be destroyed as part of delete.
        delete asset;
    }
}

fc::Result<AssetRef<AssetMemory>> Space::create_memory(AssetMemoryCreateParams params)
{

    if (params.size == 0)
    {
        return ("size must be greater than 0");
    }

    if (params.addr != 0 && params.addr + params.size > kernel_virtual_base())
    {
        return ("addr must be lower than kernel virtual base");
    }

    auto ptr = try$(allocate_asset<AssetMemory>(
        params.size,
        params.addr,
        params.addr == 0));
    if (params.addr == 0)
    {
        fc::Result<PhysAddr> res = params.lower_half
                                       ? Pmm::the().allocate(Pages::from_bytes_ceil(params.size), IOL_ALLOC_MEMORY_FLAG_LOWER_SPACE)
                                       : Pmm::the().allocate(Pages::from_bytes_ceil(params.size));

        if (res.is_error())
        {

            fmt::log$("asked size: {} (page count)", math::alignUp<size_t>(params.size, arch::amd64::PAGE_SIZE) / arch::amd64::PAGE_SIZE);
            fmt::err$("asset_create_memory: unable to allocate memory: {}", res.error());

            ptr.asset->lock.release();
            _asset_remove(ptr.handle);
            return ("unable to allocate memory");
        }

        ptr.asset->addr = res.unwrap()._addr;
        ptr.asset->size = params.size;
    }
    else
    {
        for (int i = 0; i < Pmm::the()._context->_memory_map_count; i++)
        {
            auto &map = Pmm::the()._context->_memory_map[i];
            auto range = map.range;
            if (range.start() <= params.addr && range.end() >= params.addr + params.size)
            {
                if (map.type == mcx::MemoryMap::Type::FREE)
                {
                    Pmm::the().own(PhysAddr{params.addr}, params.size);
                }
                else if (map.type != mcx::MemoryMap::Type::FREE)
                {
                    fmt::err$("asset_create_memory: memory range {} is not free ({})", range, (int)map.type);
                }

                break;
            }
        }
        // res = PhysAddr{params.addr};

        ptr.asset->addr = params.addr;
        ptr.asset->size = params.size;
    }

    ptr.asset->lock.release();
    return ptr;
}

fc::Result<AssetRef<AssetMapping>> Space::create_mapping(AssetMappingCreateParams params)
{
    params.physical_mem.lock();
    auto ptr = try$(allocate_asset<AssetMapping>(
        params.start,
        params.end,
        params.physical_mem,
        params.writable,
        params.executable));

    if (params.start >= params.end)
    {
        ptr.asset->lock.release();
        params.physical_mem.unlock();
        return ("asset_create_mapping: start must be less than end");
    }

    if (params.physical_mem.asset->kind != OBJECT_KIND_MEMORY)
    {
        ptr.asset->lock.release();
        params.physical_mem.unlock();
        return ("asset_create_mapping: physical_mem must be a memory asset");
    }

    if (params.start >= kernel_virtual_base())
    {
        ptr.asset->lock.release();
        params.physical_mem.unlock();
        return ("asset_create_mapping: start must be less than kernel virtual base");
    }

    auto flags = PageFlags()
                     .user(true)
                     .executable(ptr.asset->executable)
                     .present(true)
                     .writeable(ptr.asset->writable);

    if (vmm_space.map(
                     {ptr.asset->start, ptr.asset->end},
                     {ptr.asset->physical_mem.asset->addr,
                      ptr.asset->physical_mem.asset->size + ptr.asset->physical_mem.asset->addr},
                     flags)
            .is_error())
    {
        ptr.asset->lock.release();
        _asset_remove(ptr.handle);
        params.physical_mem.unlock();
        return "unable to map physical memory";
    }

    ptr.asset->lock.release();
    params.physical_mem.unlock();

    return ptr;
}

fc::Result<AssetRef<AssetTask>> Space::create_task(AssetTaskCreateParams params)
{
    auto ptr = try$(add_asset<AssetTask>(
        kernel::Task::task_create().unwrap()));

    ptr.asset->_space_owner = this;
    Asset::own(ptr.asset);

    if (ptr.asset->_initialize(params.launch, &vmm_space).is_error())
    {
        ptr.asset->lock.release();
        _asset_remove(ptr.handle);
        return ("unable to initialize task asset");
    }

    ptr.asset->lock.release();
    return ptr;
}

fc::Result<AssetRef<kernel::IpcMessageReturnTask>> Space::create_ipc_return_task(AssetIpcReturnTaskCreateParams const &params)
{
    auto ptr = try$(allocate_asset<kernel::IpcMessageReturnTask>(params.task));

    Asset::own(ptr.asset);

    ptr.asset->lock.release();
    return ptr;
}

// asset_move and asset_copy are now template functions defined in space.hpp

fc::Result<AssetRef<kernel::IpcEndpoint>> Space::create_ipc_endpoint(AssetIpcEndpointCreateParams const &params)
{
    auto ptr = try$(allocate_asset<kernel::IpcEndpoint>());

    ptr->target_message_space = AssetRef<Space>(this, 0);
    ptr->last_port = 16;

    if (params.publish)
    {
        ptr->uuid = kernel::publish_server(ptr, params.is_root);
    }
    else
    {
        ptr->uuid = kernel::get_next_ipc_server_handle();
    }

    Asset::own(ptr.asset);
    ptr.asset->lock.release();
    return ptr;
}

fc::Result<AssetRef<kernel::IpcEndpointConnection>> Space::create_ipc_connection(AssetIpcConnectionCreateParams const &params)
{
    auto send_ptr = try$(allocate_asset<kernel::IpcEndpointConnection>());

    Asset::own(send_ptr.asset);
    send_ptr->connection_to = params.endpoint;
    send_ptr->connection_to.lock();
    send_ptr->port = params.endpoint->last_port;
    send_ptr->connection_to->last_port++;
    send_ptr->connection_to.unlock();

    send_ptr->lock.release();
    return send_ptr;
}
