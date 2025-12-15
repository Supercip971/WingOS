
#include "asset.hpp"

#include "arch/x86_64/paging.hpp"
#include "hw/mem/addr_space.hpp"
#include "iol/mem_flags.h"

#include "kernel/generic/ipc.hpp"
#include "kernel/generic/space.hpp"
#include "math/align.hpp"
#include "wingos-headers/asset.h"

core::Result<AssetPtr> _asset_create(Space *space, AssetKind kind)
{
    Asset *asset = new Asset();
    *asset = {};
    asset->kind = kind;

    asset->lock.lock();

    asset->ref_count = 1;

    AssetPtr ptr = {};
    ptr.asset = asset;
    ptr.handle = 0;
    if (space != nullptr)
    {
        space->self->lock.lock();
        ptr.handle = space->alloc_uid++;

        if (space->assets.push(ptr).is_error())
        {
            log::err$("unable to add asset to space");
            delete asset;

            space->self->lock.release();
            return core::Result<AssetPtr>::error("asset create: unable to add asset to space");
        }

        space->self->lock.release();
    }

    return ptr;
}

void asset_own(Asset *asset)
{
    asset->lock.lock();
    asset->ref_count++;
    asset->lock.release();
}

void asset_remove_from_space(Space *space, Asset *asset)
{
    if (space == nullptr || asset == nullptr)
    {
        return;
    }

    asset->lock.lock();
    for (size_t i = 0; i < space->assets.len(); i++)
    {
        if (space->assets[i].asset == asset)
        {
            space->assets.pop(i);
            break;
        }
    }
    asset->lock.release();
}

void asset_release(Space *space, Asset *asset)
{

    asset->lock.lock();

    if (asset->ref_count == 0)
    {
        log::err$("asset_release: asset is already released");
        asset->lock.release();
        asset_remove_from_space(space, asset);
        return;
    }

    if (asset->kind == OBJECT_KIND_IPC_CONNECTION)
    {
        if (asset->ipc_connection->closed_status == IPC_STILL_OPEN)
        {
            asset->ipc_connection->closed_status = IPC_CLOSED;
        }

        if (space->uid == asset->ipc_connection->server_space_handle)
        {
            // CRITICAL: Use the server_handle to look up the server via query_server
            // instead of using the server_asset pointer directly.
            IpcServerHandle server_handle = asset->ipc_connection->server_handle;

            // Skip for pipe connections (server_handle = -1)
            if (server_handle != (IpcServerHandle)-1)
            {
                // This holds ipc_server_lock, ensuring the server can't be
                // unregistered while we're working with it
                auto server_result = query_server_locked(server_handle);

                if (!server_result.is_error())
                {
                    auto kernel_server = server_result.unwrap();

                    // Double-check this is really the server we connected to
                    // (same handle) and it's not destroyed
                    if (!kernel_server->destroyed)
                    {
                        // Release asset lock before acquiring server lock to avoid ABBA deadlock
                        // (server_accept_connection acquires server->self->lock then asset->lock)
                        asset->lock.release();

                        kernel_server->self->lock.lock();
                        for (size_t i = 0; i < kernel_server->connections.len(); i++)
                        {
                            if (kernel_server->connections[i].asset == asset)
                            {
                                kernel_server->connections.pop(i);
                                break;
                            }
                        }

                        kernel_server->self->ref_count--;
                        kernel_server->self->lock.release();

                        // Re-acquire asset lock to continue with release
                        asset->lock.lock();
                    }

                    release_server_lock();
                }
                // If query_server_locked failed, the server was already unregistered
                // and its connections were already cleaned up, so nothing to do
            }

            asset->ipc_connection->server_mutex.mutex_release();

        }
    }

    if (asset->kind == OBJECT_KIND_IPC_SERVER)
    {
        asset->ipc_server->destroyed = true;

        // Unregister the server first to prevent new connections
        unregister_server(asset->ipc_server->handle, asset->ipc_server->parent_space);

        core::Vec<AssetPtr> connections_copy = asset->ipc_server->connections;
        asset->ipc_server->connections.clear();

        for (size_t i = 0; i < connections_copy.len(); i++)
        {
            if (connections_copy[i].asset->kind == OBJECT_KIND_IPC_CONNECTION)
            {
                connections_copy[i].asset->ipc_connection->closed_status = IPC_CLOSED;
                asset->ref_count--;
            }
            asset_release(space, connections_copy[i].asset);
        }

        connections_copy.release();
    }

    asset->ref_count--;

    if (asset->ref_count == 0)
    {
        if (asset->kind == OBJECT_KIND_MEMORY)
        {
            if (asset->memory.allocated)
            {
                Pmm::the().release(PhysAddr{asset->memory.addr}, asset->memory.size);
            }
        }
        else if (asset->kind == OBJECT_KIND_MAPPING)
        {
            if (space != nullptr)
            {
                space->vmm_space.unmap(
                    VirtRange(asset->mapping.start,
                              asset->mapping.end),
                    true);
            }
        }
        else if (asset->kind == OBJECT_KIND_IPC_CONNECTION)
        {
            asset->ipc_connection->message_sent.release();

            if (asset->ipc_connection->server_mutex.mutex_value())
            {
                asset->ipc_connection->server_mutex.mutex_release();
                // kernel::resolve_blocked_tasks(); // release blocked tasks
            }
            if (asset->ipc_connection->client_mutex.mutex_value())
            {
                asset->ipc_connection->client_mutex.mutex_release();
                //  kernel::resolve_blocked_tasks(); // release blocked tasks
            }

            // Note: removal from server's connections list already happened above
            // before ref_count was decremented, to prevent dangling pointers

            // Check if we should auto-release the server when last connection is gone
            // Use query_server_locked to safely access the server (avoids use-after-free)
            IpcServerHandle server_handle = asset->ipc_connection->server_handle;
            if (server_handle != (IpcServerHandle)-1)
            {
                auto server_result = query_server_locked(server_handle);
                if (!server_result.is_error())
                {
                    auto kernel_server = server_result.unwrap();
                    if (!kernel_server->destroyed &&
                        kernel_server->connections.len() == 0 &&
                        kernel_server->self->ref_count == 1)
                    {
                        kernel_server->destroyed = true;
                        release_server_lock();
                        auto parent_space = Space::global_space_by_handle(kernel_server->parent_space).unwrap();
                        asset_release(parent_space, kernel_server->self);
                    }
                    else
                    {
                        release_server_lock();
                    }
                }
                // If query failed, server was already unregistered - nothing to do
            }

            delete asset->ipc_connection;
        }

        else if (asset->kind == OBJECT_KIND_IPC_SERVER)
        {
            // Connections list was already cleared and released above
            // Just delete the server structure
            delete asset->ipc_server;
        }
        else if (asset->kind == OBJECT_KIND_SPACE)
        {
            for (size_t i = 1; i < asset->space->assets.len(); i++)
            {
                asset_release(asset->space, asset->space->assets[i].asset);
            }

            delete asset->space; // delete the space if it exists
        }
        else if (asset->kind == OBJECT_KIND_TASK)
        {
            log::warn$("asset_release: task asset is not supported yet");
        }

        asset->lock.release();
        asset_remove_from_space(space, asset);
        delete asset;
    }
    else
    {
        asset->lock.release();
        asset_remove_from_space(space, asset);

    }

}

core::Result<AssetPtr> asset_create_memory(Space *space, AssetMemoryCreateParams params)
{

    if (params.size == 0)
    {
        return core::Result<AssetPtr>::error("size must be greater than 0");
    }

    if (params.addr != 0 && params.addr + params.size > kernel_virtual_base())
    {
        return core::Result<AssetPtr>::error("addr must be lower than kernel virtual base");
    }

    AssetPtr ptr = try$(_asset_create(space, OBJECT_KIND_MEMORY));
    ptr.asset->memory.size = params.size;
    ptr.asset->memory.addr = params.addr;
    core::Result<PhysAddr> res(PhysAddr{0});
    if (params.addr == 0)
    {
        if (params.lower_half)
        {
            res = Pmm::the().allocate(math::alignUp<size_t>(params.size, arch::amd64::PAGE_SIZE) / arch::amd64::PAGE_SIZE, IOL_ALLOC_MEMORY_FLAG_LOWER_SPACE);
        }
        else
        {
            res = Pmm::the().allocate(math::alignUp<size_t>(params.size, arch::amd64::PAGE_SIZE) / arch::amd64::PAGE_SIZE);
        }

        if (res.is_error())
        {

            log::log$("asked size: {} (page count)", math::alignUp<size_t>(params.size, arch::amd64::PAGE_SIZE) / arch::amd64::PAGE_SIZE);
            log::err$("asset_create_memory: unable to allocate memory: {}", res.error());

            ptr.asset->lock.release();
            asset_release(space, ptr.asset);
            return core::Result<AssetPtr>::error("unable to allocate memory");
        }

        ptr.asset->memory.addr = res.unwrap()._addr;
    }
    else
    {
        //
    }

    if (res.is_error())
    {
        log::err$("asset_create_memory: unable to allocate memory: {}", res.error());

        ptr.asset->lock.release();
        asset_release(space, ptr.asset);
        return core::Result<AssetPtr>::error("unable to allocate memory");
    }

    ptr.asset->lock.release();
    return ptr;
}
core::Result<AssetPtr> asset_create_mapping(Space *space, AssetMappingCreateParams params)
{
    AssetPtr ptr = try$(_asset_create(space, OBJECT_KIND_MAPPING));

    if (params.start >= params.end)
    {
        return core::Result<AssetPtr>::error("asset_create_mapping: start must be less than end");
    }

    if (params.physical_mem->kind != OBJECT_KIND_MEMORY)
    {
        return core::Result<AssetPtr>::error("asset_create_mapping: physical_mem must be a memory asset");
    }

    if (params.start >= kernel_virtual_base())
    {
        return core::Result<AssetPtr>::error("asset_create_mapping: start must be less than kernel virtual base");
    }

    ptr.asset->mapping.start = params.start;
    ptr.asset->mapping.end = params.end;
    ptr.asset->mapping.physical_mem = params.physical_mem;
    ptr.asset->mapping.writable = params.writable;
    ptr.asset->mapping.executable = params.executable;

    if (space == nullptr)
    {
        log::warn$("asset_create_mapping: space is null");
        ptr.asset->lock.release();
        return ptr;
    }

    auto flags = PageFlags()
                     .user(true)
                     .executable(ptr.asset->mapping.executable)
                     .present(true)
                     .writeable(ptr.asset->mapping.writable);

    if (!space->vmm_space.map(
            {ptr.asset->mapping.start, ptr.asset->mapping.end},
            {ptr.asset->mapping.physical_mem->memory.addr,
             ptr.asset->mapping.physical_mem->memory.size + ptr.asset->mapping.physical_mem->memory.addr},
            flags))
    {
        ptr.asset->lock.release();
        asset_release(space, ptr.asset);
        return core::Result<AssetPtr>::error("unable to map memory");
    }

    ptr.asset->lock.release();

    return ptr;
}

core::Result<AssetPtr> asset_create_task(Space *space, AssetTaskCreateParams params)
{
    AssetPtr ptr = try$(_asset_create(space, OBJECT_KIND_TASK));
    ptr.asset->task = kernel::Task::task_create().unwrap();

    ptr.asset->task->_space_owner = space;

    if (ptr.asset->task->_initialize(params.launch, &space->vmm_space).is_error())
    {
        ptr.asset->lock.release();
        asset_release(space, ptr.asset);
        return core::Result<AssetPtr>::error("unable to initialize task asset");
    }

    ptr.asset->lock.release();
    return ptr;
}

core::Result<AssetPtr> asset_move(Space *from, Space *to, AssetPtr asset)
{
    if (from == nullptr || to == nullptr)
    {
        return core::Result<AssetPtr>::error("from or to space is null");
    }

    if (asset.asset == nullptr)
    {
        return core::Result<AssetPtr>::error("asset is null");
    }

    // Check if the asset exists in the from space
    from->self->lock.lock();
    for (size_t i = 0; i < from->assets.len(); i++)
    {
        if (from->assets[i].handle == asset.handle)
        {
            to->self->lock.lock();

            // Move the asset to the new space
            AssetPtr moved_asset = from->assets.pop(i);
            moved_asset.handle = to->alloc_uid++;
            to->assets.push(moved_asset);

            to->self->lock.release();
            from->self->lock.release();

            return moved_asset;
        }
    }

    from->self->lock.release();

    return core::Result<AssetPtr>::error("asset not found in from space");
}

// FIXME: rename this to asset_share
core::Result<AssetPtr> asset_copy(Space *from, Space *to, AssetPtr asset)
{
    if (from == nullptr || to == nullptr)
    {
        return core::Result<AssetPtr>::error("from or to space is null");
    }

    if (asset.asset == nullptr)
    {
        return core::Result<AssetPtr>::error("asset is null");
    }

    // Lock the asset to safely increment ref_count
    asset.asset->lock.lock();
    asset.asset->ref_count++;
    asset.asset->lock.release();

    // Now lock the destination space to add the asset
    to->self->lock.lock();
    AssetPtr copied_asset = {};
    copied_asset.asset = asset.asset;
    copied_asset.handle = to->alloc_uid++;
    to->assets.push(copied_asset);
    to->self->lock.release();

    return copied_asset;
}

core::Result<AssetPtr> asset_create_ipc_server(Space *space, AssetIpcServerCreateParams params)
{
    AssetPtr ptr = try$(_asset_create(space, OBJECT_KIND_IPC_SERVER));

    if (params.is_root)
    {
        ptr.asset->ipc_server = register_server(0, space->uid);
    }
    else
    {
        ptr.asset->ipc_server = create_server(space->uid);
    }

    ptr.asset->ipc_server->self = ptr.asset;

    ptr.asset->lock.release();
    return ptr;
}

core::Result<AssetPtr> asset_create_ipc_connections(Space *space, AssetIpcConnectionCreateParams params)
{
    AssetPtr ptr = try$(_asset_create(space, OBJECT_KIND_IPC_CONNECTION));

    // This prevents another thread from unregistering (and deleting) the server while we use it.
    auto query_res = query_server_locked(params.server_handle);
    if (query_res.is_error())
    {
        log::err$("asset_create_ipc_connection: unable to query server: {} for {}", query_res.error(), params.server_handle);

        ptr.asset->lock.release();

        asset_release(space, ptr.asset);
        return core::Result<AssetPtr>::error("unable to query server");
    }

    auto server = query_res.unwrap();

    ptr.asset->ipc_connection = new IpcConnection();

    ptr.asset->ipc_connection->message_alloc_id = 0;
    ptr.asset->ipc_connection->accepted = false;
    ptr.asset->ipc_connection->closed_status = IPC_STILL_OPEN;
    ptr.asset->ipc_connection->server_handle = params.server_handle;
    ptr.asset->ipc_connection->server_space_handle = server->parent_space;
    ptr.asset->ipc_connection->client_space_handle = space->uid;
    ptr.asset->ipc_connection->server_asset = server;
    ptr.asset->ipc_connection->message_sent = {};

    ptr.asset->lock.release();

    // Get server space while holding ipc_server_lock (server can't be deleted)
    auto server_space_res = Space::global_space_by_handle(server->parent_space);
    if (server_space_res.is_error()) {
        log::err$("asset_create_ipc_connection: failed to get server space {}", server->parent_space);
        release_server_lock();
        ptr.asset->lock.lock();
        asset_release(space, ptr.asset);
        return core::Result<AssetPtr>::error("failed to get server space");
    }
    auto server_space = server_space_res.unwrap();

    // Copy asset to server space (this handles its own locking internally)
    auto copy_res = asset_copy(space, server_space, ptr);
    if (copy_res.is_error()) {
        log::err$("asset_create_ipc_connection: failed to copy asset to server space");
        release_server_lock();
        ptr.asset->lock.lock();
        asset_release(space, ptr.asset);
        return core::Result<AssetPtr>::error("failed to copy asset to server space");
    }
    auto ptr_in_server = copy_res.unwrap();

    // Now lock server->self to modify connections and ref_count
    server->self->lock.lock();
    server->connections.push(ptr_in_server);
    server->self->ref_count++;
    server->self->lock.release();

    release_server_lock();

    return ptr;
}

core::Result<AssetIpcConnectionPipeCreateResult> asset_create_ipc_connections(
    Space *space_sender, Space *space_receiver, AssetIpcConnectionPipeCreateParams params)
{

    (void)params;
    AssetPtr send_ptr = try$(_asset_create(space_sender, OBJECT_KIND_IPC_CONNECTION));

    send_ptr.asset->ipc_connection = new IpcConnection();
    send_ptr.asset->ipc_connection->message_alloc_id = 0;
    send_ptr.asset->ipc_connection->accepted = true;
    send_ptr.asset->ipc_connection->closed_status = IPC_STILL_OPEN;

    send_ptr.asset->ipc_connection->server_handle = -1;
    send_ptr.asset->ipc_connection->server_space_handle = space_receiver->uid;
    send_ptr.asset->ipc_connection->client_space_handle = space_sender->uid;

    send_ptr.asset->ipc_connection->message_sent = {};

    send_ptr.asset->lock.release();

    AssetPtr recv_ptr = try$(asset_copy(space_sender, space_receiver, send_ptr));

    return AssetIpcConnectionPipeCreateResult{
        .sender_connection = send_ptr,
        .receiver_connection = recv_ptr,
    };
}
