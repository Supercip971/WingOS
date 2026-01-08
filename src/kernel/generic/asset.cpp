#include "asset.hpp"
#include <new>

#include "arch/x86_64/paging.hpp"
#include "hw/mem/addr_space.hpp"
#include "iol/mem_flags.h"

// Make sure IPC public types (e.g. IpcServerHandle) are visible before this TU
// pulls in headers that depend on them transitively.
#include "kernel/generic/ipc.hpp"
#include "kernel/generic/space.hpp"
#include "kernel/generic/task.hpp"
#include "math/align.hpp"
#include "mcx/mcx.hpp"
#include "wingos-headers/asset.h"
#include "wingos-headers/ipc.h"

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
    if (asset->kind == OBJECT_KIND_IPC_CONNECTION)
    {

        auto conn = asset->casted<AssetConnection>()->connection;
        if (conn->closed_status == IPC_STILL_OPEN)
        {
            conn->closed_status = IPC_CLOSED;
        }
    }

    if (asset->kind == OBJECT_KIND_IPC_SERVER)
    {
        auto server = asset->casted<AssetServer>();

        server->server->destroyed = true;

        // Unregister the server first to prevent new connections
        unregister_server(server->server->handle, server->server->parent_space);

        server->server->connections.release();
    }
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
        asset->ref_count.fetch_add(1, std::memory_order_relaxed);
        log::err$("asset_release: asset is already released");
        log::err$("double free detected");
        return;
    }

    if (old_count == 1)
    {
        // We decremented from 1 to 0, so we own destruction
        // Lock to ensure exclusive access during cleanup
        asset->lock.lock();

        //   log::log$("freeing asset");
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

//            log::log$("({}) destroying connection: {}", Cpu::currentId(), (uintptr_t)asset | fmt::FMT_HEX);


             auto conn = asset->casted<AssetConnection>();
            // conn->connection->message_sent.release();

            if (conn->connection->server_mutex.mutex_value())
            {
                conn->connection->server_mutex.mutex_release();
                // kernel::resolve_blocked_tasks(); // release blocked tasks
            }
            if (conn->connection->client_mutex.mutex_value())
            {
                conn->connection->client_mutex.mutex_release();
                // kernel::resolve_blocked_tasks(); // release blocked tasks
            }

            // Note: removal from server's connections list already happened above
            // before ref_count was decremented, to prevent dangling pointers

            // Check if we should auto-release the server when last connection is gone
            // Use query_server_locked to safely access the server (avoids use-after-free)

            // If query failed, server was already unregistered - nothing to do

            // Guard against accidental double-destruction if refcounts drifted.
            if (conn->connection != nullptr)
            {
                delete conn->connection;
                conn->connection = nullptr;
            }
        }

        else if (asset->kind == OBJECT_KIND_IPC_SERVER)
        {
            auto server = asset->casted<AssetServer>();
            // Connections list was already cleared and released above
            // Just delete the server structure

            for (auto &conn : server->server->connections)
            {
                // Lock each connection asset before modifying its state
                conn.asset->lock.lock();
                auto aconn = conn.asset->casted<AssetConnection>();
                aconn->connection->closed_status = IPC_CLOSED;
                conn.asset->lock.release();
            }

            server->server->connections.clear();
            delete server->server;
        }
        else if (asset->kind == OBJECT_KIND_SPACE)
        {
            // auto sp = asset->casted<Space>();

            log::warn$("asset_release: space asset is not supported yet");
            // Check if we should auto-release the space when last connection is gone

            // sp->vmm_space.release();
        }
        else if (asset->kind == OBJECT_KIND_TASK)
        {
            log::warn$("asset_release: task asset is not supported yet");
        }

        // Don't release the lock before deleting - the asset is about to be freed
        // and no other thread should access it. The lock will be destroyed as part of delete.
        delete asset;
    }
}

core::Result<AssetRef<AssetMemory>> Space::create_memory(AssetMemoryCreateParams params)
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
        core::Result<PhysAddr> res = params.lower_half
                                         ? Pmm::the().allocate(math::alignUp<size_t>(params.size, arch::amd64::PAGE_SIZE) / arch::amd64::PAGE_SIZE, IOL_ALLOC_MEMORY_FLAG_LOWER_SPACE)
                                         : Pmm::the().allocate(math::alignUp<size_t>(params.size, arch::amd64::PAGE_SIZE) / arch::amd64::PAGE_SIZE);

        if (res.is_error())
        {

            log::log$("asked size: {} (page count)", math::alignUp<size_t>(params.size, arch::amd64::PAGE_SIZE) / arch::amd64::PAGE_SIZE);
            log::err$("asset_create_memory: unable to allocate memory: {}", res.error());

            ptr.asset->lock.release();
            _asset_remove(ptr.handle);
            return ("unable to allocate memory");
        }

        ptr.asset->addr = res.unwrap()._addr;
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
                    log::err$("asset_create_memory: memory range {} is not free ({})", range, (int)map.type);
                }

                break;
            }
        }
        // res = PhysAddr{params.addr};

        ptr.asset->addr = params.addr;
    }

    ptr.asset->lock.release();
    return ptr;
}
core::Result<AssetRef<AssetMapping>> Space::create_mapping(AssetMappingCreateParams params)
{
    auto ptr = try$(allocate_asset<AssetMapping>(
        params.start,
        params.end,
        params.physical_mem,
        params.writable,
        params.executable));

    if (params.start >= params.end)
    {
        ptr.asset->lock.release();
        return ("asset_create_mapping: start must be less than end");
    }

    if (params.physical_mem.asset->kind != OBJECT_KIND_MEMORY)
    {
        ptr.asset->lock.release();
        return ("asset_create_mapping: physical_mem must be a memory asset");
    }

    if (params.start >= kernel_virtual_base())
    {
        ptr.asset->lock.release();
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
        return "unable to map physical memory";
    }

    ptr.asset->lock.release();

    return ptr;
}

core::Result<AssetRef<AssetTask>> Space::create_task(AssetTaskCreateParams params)
{
    auto ptr = try$(allocate_asset<AssetTask>(
        kernel::Task::task_create().unwrap()));

    ptr.asset->task->_space_owner = this;
    Asset::own(ptr.asset);

    if (ptr.asset->task->_initialize(params.launch, &vmm_space).is_error())
    {
        ptr.asset->lock.release();
        _asset_remove(ptr.handle);
        return ("unable to initialize task asset");
    }

    ptr.asset->lock.release();
    return ptr;
}

// asset_move and asset_copy are now template functions defined in space.hpp

core::Result<AssetRef<AssetServer>> Space::create_ipc_server(AssetIpcServerCreateParams params)
{
    auto ptr = try$(allocate_asset<AssetServer>(nullptr));

    if (params.is_root)
    {
        ptr.asset->server = register_server(0, uid);
    }
    else
    {
        ptr.asset->server = create_server(uid);
    }

    ptr.asset->lock.release();
    ptr.asset->server->self = ptr.to_untyped();

    return ptr;
}

core::Result<AssetRef<AssetConnection>> Space::create_ipc_connection(AssetIpcConnectionCreateParams params)
{
    auto ptr = try$(allocate_asset<AssetConnection>(new IpcConnection{}));

    // This prevents another thread from unregistering (and deleting) the server while we use it.
    auto query_res = query_server_locked(params.server_handle);
    if (query_res.is_error())
    {
        log::err$("asset_create_ipc_connection: unable to query server: {} for {}", query_res.error(), params.server_handle);

        ptr.asset->lock.release();

        asset_release(ptr);
        return ("unable to query server");
    }

    auto server = query_res.unwrap();

    // Cache the server info we need while holding the lock
    auto server_parent_space = server->parent_space;
    auto server_self = server->self;

    // Release ipc_server_lock BEFORE acquiring any space locks to avoid deadlock
    release_server_lock();

    ptr.asset->connection->message_alloc_id = 0;
    ptr.asset->connection->accepted = false;
    ptr.asset->connection->closed_status = IPC_STILL_OPEN;
    ptr.asset->connection->server_handle = params.server_handle;
    ptr.asset->connection->server_space_handle = server_parent_space;
    ptr.asset->connection->client_space_handle = uid;
    ptr.asset->connection->server_asset = server_self;
    ptr.asset->connection->message_sent = {};

    ptr.asset->lock.release();

    // Get server space (ipc_server_lock already released)
    auto server_space_res = Space::global_space_by_handle(server_parent_space);
    if (server_space_res.is_error())
    {
        log::err$("asset_create_ipc_connection: failed to get server space {}", server_parent_space);
        _asset_remove(ptr.handle);

        return ("failed to get server space");
    }
    auto server_space = server_space_res.unwrap().asset;

    // Copy asset to server space (this handles its own locking internally)
    auto copy_res = asset_copy(this, server_space, ptr);
    if (copy_res.is_error())
    {
        log::err$("asset_create_ipc_connection: failed to copy asset to server space: {}", copy_res.error());
        _asset_remove(ptr.handle);

        return ("failed to copy asset to server space");
    }
    auto ptr_in_server = copy_res.unwrap();

    // Now lock server asset to modify connections
    server_self.asset->lock.lock();
    auto server_ptr = server_self.asset->casted<AssetServer>()->server;
    try$(server_ptr->connections.push(core::move(ptr_in_server)));
    server_self.asset->lock.release();

    return ptr;
}

core::Result<AssetIpcConnectionPipeCreateResult> Space::create_ipc_connections(
    Space *space_sender, Space *space_receiver, AssetIpcConnectionPipeCreateParams params)
{
    (void)params;

    AssetRef<AssetConnection> send_ptr = try$(space_sender->allocate_asset<AssetConnection>(
        new IpcConnection()));

    send_ptr.asset->connection->message_alloc_id = 0;
    send_ptr.asset->connection->accepted = true;
    send_ptr.asset->connection->closed_status = IPC_STILL_OPEN;

    send_ptr.asset->connection->server_handle = -1;
    send_ptr.asset->connection->server_space_handle = space_receiver->uid;
    send_ptr.asset->connection->client_space_handle = space_sender->uid;
    send_ptr.asset->connection->message_sent = {};

    send_ptr.asset->lock.release();

    auto recv_ptr = try$(asset_copy(space_sender, space_receiver, send_ptr));

    return AssetIpcConnectionPipeCreateResult{
        .sender_connection = send_ptr.to_untyped(),
        .receiver_connection = recv_ptr,
    };
}
