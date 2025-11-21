#include "space.hpp"

#include "kernel/generic/asset.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/result.hpp"

size_t _space_handle = 1;

struct SpacePtr
{
    Space *space;
    uint64_t handle; // the handle of the space in the space
};

core::Vec<SpacePtr> _spaces = {};

void Asset::dump_assets(Space *space)
{
        space->self->lock.lock();
        log::log$("Assets in space {}:", space->uid);
        for (size_t i = 0; i < space->assets.len(); i++)
        {
            log::log$("  Asset[{}]: handle={}, kind={}", i, space->assets[i].handle, assetKind2Str(space->assets[i].asset->kind));
            if(space->assets[i].asset->kind == OBJECT_KIND_IPC_SERVER)
            {
                log::log$("    IPC Server parent space: {}", space->assets[i].asset->ipc_server->parent_space);
                log::log$("    IPC Server handle: {}", space->assets[i].asset->ipc_server->handle);
                log::log$("    IPC Server connections: {}", space->assets[i].asset->ipc_server->connections.len());
            }

            if(space->assets[i].asset->kind == OBJECT_KIND_IPC_CONNECTION)
            {
                log::log$("    IPC Connection accepted: {}", space->assets[i].asset->ipc_connection->accepted);
                 log::log$("    IPC Connection closed: {}", (int)space->assets[i].asset->ipc_connection->closed_status);

                 log::log$("    IPC Connected: {}", (int)space->assets[i].asset->ipc_connection->server_asset->handle);
                
            }
        
        }
        space->self->lock.release();
    }
core::Result<AssetPtr> space_create(Space *parent, [[maybe_unused]] uint64_t flags, [[maybe_unused]] uint64_t rights)
{

    AssetPtr ptr = try$(_asset_create(parent, OBJECT_KIND_SPACE));
    Asset *asset = ptr.asset;
    Space *space = new Space();

    *space = {};
    asset->space = space;

    space->self = asset;

    space->parent_space_handle = parent;
    auto vspace = VmmSpace::create(false);

    if (vspace.is_error())
    {
        log::err$("space_create: failed to create vmm space: {}", vspace.error());
        asset->lock.release();
        asset_release(parent, asset);
        delete space;
        return vspace.error();
    }

    space->vmm_space = vspace.unwrap();

    SpacePtr space_ptr = {};
    space_ptr.space = space;
    space_ptr.handle = _space_handle++;
    _spaces.push(space_ptr);

    space->uid = space_ptr.handle;
    space->assets.push({.asset = asset, .handle = 0});
    space->alloc_uid = 16;
    asset->lock.release();

    return ptr;
}

// FIXME: this is not safe, because it does not check if the space exists in the parent space
core::Result<Space *> Space::space_by_handle(Space *parent, uint64_t handle)
{
    for (auto &space_ptr : parent->assets)
    {
        if (space_ptr.handle == handle && space_ptr.asset->kind == OBJECT_KIND_SPACE)
        {
            return core::Result<Space *>::success(space_ptr.asset->space);
        }
    }

    log::err$("Space::space_by_handle: space not found: {}", handle);

   return core::Result<Space *>::error("space not found");
}
// FIXME: this is not safe, because it does not check if the space exists in the parent space
core::Result<Space *> Space::global_space_by_handle(uint64_t handle)
{
    for (auto &space_ptr : _spaces)
    {
        if (space_ptr.handle == handle)
        {
            return core::Result<Space *>::success(space_ptr.space);
        }
    }

    return core::Result<Space *>::error("space not found");
}
