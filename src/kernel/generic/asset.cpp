
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
        ptr.handle = space->alloc_uid++;

        if (space->assets.push(ptr).is_error())
        {
            log::err$("unable to add asset to space");
            delete asset;
            return core::Result<AssetPtr>::error("asset create: unable to add asset to space");
        }
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

        auto kernel_server = asset->ipc_connection->server_asset;
        
        if(!kernel_server->destroyed)
        {
            kernel_server->lock.lock();
            for(size_t i = 0; i < kernel_server->connections.len(); i++)
            {
                if(kernel_server->connections[i].asset == asset)
                {
                    kernel_server->connections.pop(i);
                    break;
                }
            }
            kernel_server->lock.release();
        }

        if(space->space_handle == asset->ipc_connection->server_space_handle)
        {
            asset->ipc_connection->server_mutex.mutex_release();
        }
        
        asset->ipc_connection->lock.release();
    }


    if(asset->kind == OBJECT_KIND_IPC_SERVER)
    {
        //log::log$("Releasing IPC server asset {}", asset->ipc_server->handle);
        asset->ipc_server->destroyed = true;
        asset->ipc_server->lock.lock();

        unregister_server(asset->ipc_server->handle, asset->ipc_server->parent_space);

        // Mark server as destroyed first, then close all connections
        core::Vec<AssetPtr> connections_copy = asset->ipc_server->connections;
        
        asset->ipc_server->connections.clear();
        asset->ipc_server->lock.release();

        // Now release each connection
        for( size_t i = 0; i < connections_copy.len(); i++)
        {
            // Mark the connection as closed from server side
            if(connections_copy[i].asset->kind == OBJECT_KIND_IPC_CONNECTION)
            {
                connections_copy[i].asset->ipc_connection->closed_status = IPC_CLOSED;
                asset->ref_count--;
            }
            asset_release(space, connections_copy[i].asset);
        }

        connections_copy.release();
       // Asset::dump_assets(space);


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

            auto kernel_server = asset->ipc_connection->server_asset;
            
            // Note: removal from server's connections list already happened above
            // before ref_count was decremented, to prevent dangling pointers
            
            // Check if we should auto-release the server when last connection is gone
            if(!kernel_server->destroyed && kernel_server->connections.len() == 0 && kernel_server->self->ref_count == 1)
            {
                kernel_server->destroyed = true;
                auto parent_space = Space::global_space_by_handle(kernel_server->parent_space).unwrap();
                asset_release(parent_space, kernel_server->self);                
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
    }
    asset->lock.release();

    asset_remove_from_space(space, asset);
    delete asset;
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
    for (size_t i = 0; i < from->assets.len(); i++)
    {
        if (from->assets[i].asset == asset.asset)
        {
            // Move the asset to the new space
            AssetPtr moved_asset = from->assets[i];
            moved_asset.handle = to->alloc_uid++;
            to->assets.push(moved_asset);
            from->assets.pop(i);
            return moved_asset;
        }
    }

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

    to->self->lock.lock();
    // Check if the asset exists in the from space
    AssetPtr copied_asset = {};
    copied_asset.asset = asset.asset;
    copied_asset.handle = to->alloc_uid++;
    asset.asset->ref_count++;

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

    auto query_res = query_server(params.server_handle);
    if (query_res.is_error())
    {
        log::err$("asset_create_ipc_connection: unable to query server: {} for {}", query_res.error(), params.server_handle);

        ptr.asset->lock.release();

        asset_release(space, ptr.asset);
        return core::Result<AssetPtr>::error("unable to query server");
    }

    auto server = query_res.unwrap();

    ptr.asset->ipc_connection = new IpcConnection();
    *ptr.asset->ipc_connection = {};
    ptr.asset->ipc_connection->accepted = false;
    ptr.asset->ipc_connection->server_handle = params.server_handle;
    ptr.asset->ipc_connection->server_space_handle = server->parent_space;
    ptr.asset->ipc_connection->client_space_handle = space->uid;
    ptr.asset->ipc_connection->server_asset = server;

    ptr.asset->lock.release();

    server->lock.lock();

    auto server_space = Space::global_space_by_handle(server->parent_space).unwrap();
    auto ptr_in_server = try$(asset_copy(space, server_space, ptr));



    server->connections.push(ptr_in_server);

    server->self->ref_count++;
    server->lock.release();

    return ptr;
}

core::Result<AssetIpcConnectionPipeCreateResult> asset_create_ipc_connections(
    Space *space_sender, Space* space_receiver, AssetIpcConnectionPipeCreateParams params)
{

    (void)params;
    AssetPtr send_ptr = try$(_asset_create(space_sender, OBJECT_KIND_IPC_CONNECTION));


    send_ptr.asset->ipc_connection = new IpcConnection();

    send_ptr.asset->ipc_connection->accepted = true;

    send_ptr.asset->ipc_connection->closed_status = IPC_STILL_OPEN;
    
    send_ptr.asset->ipc_connection->server_handle = 0;

    send_ptr.asset->ipc_connection->server_space_handle = space_receiver->uid;
    send_ptr.asset->ipc_connection->client_space_handle = space_sender->uid;

    send_ptr.asset->ipc_connection->server_asset = nullptr; // No server for pipe connections


    send_ptr.asset->lock.release();

    AssetPtr recv_ptr = try$(asset_copy(space_sender, space_receiver, send_ptr));

    return AssetIpcConnectionPipeCreateResult{
        .sender_connection = send_ptr,
        .receiver_connection = recv_ptr,
    };
}