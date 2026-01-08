#include "space.hpp"
#include "kernel/generic/asset_types.hpp"

#include <new>

#include "kernel/generic/asset.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/fmt/flags.hpp"
#include "libcore/lock/lock.hpp"
#include "libcore/result.hpp"
#include "wingos-headers/asset.h"

size_t _space_handle = 1;

struct SpacePtr
{
    Space *space;
    uint64_t handle; // the handle of the space in the space
};

core::Vec<SpacePtr> _spaces = {};
core::Lock _spaces_lock = {};


 AssetRef<Space> Space::create_root()
{
    Space * root_space = new Space();
    root_space->parent_space_handle = AssetRef<Space>(root_space, -1);
    auto vspace = VmmSpace::create(false);
    if (vspace.is_error())
    {
        log::err$("space_create: failed to create vmm space: {}", vspace.error());
        delete root_space;

    }
    root_space->vmm_space = vspace.unwrap();
    root_space->uid = 0;
    root_space->alloc_uid = 16;
    root_space->assets.push(
        AssetRef<Space>(root_space, 0).to_untyped());

    _spaces.push({root_space, 0});
    root_space->assets[0].asset->ref_count.store(99999, std::memory_order_relaxed);
    return *(AssetRef<Space>*) &root_space->assets[0];

}
// AssetRef is now defined (and fully implemented) in `asset_types.hpp`.
// Keep `space.cpp` focused on Space logic to avoid ODR violations from duplicate
// template out-of-line definitions.

void Space::dump_assets()
{
        lock.lock();
        log::log$("Assets in space {}:", uid);
        for (size_t i = 0; i < assets.len(); i++)
        {
            log::log$("  Asset[{}]: handle={}, kind={}", i, assets[i].handle, assetKind2Str(assets[i].asset->kind));
            if(assets[i].asset->kind == OBJECT_KIND_IPC_SERVER)
            {
                auto server = assets[i].asset->casted<AssetServer>();
                log::log$("    IPC Server parent space: {}", server->server->parent_space);
                log::log$("    IPC Server handle: {}", server->server->handle);
                log::log$("    IPC Server connections: {}", server->server->connections.len());
            }

            if(assets[i].asset->kind == OBJECT_KIND_IPC_CONNECTION)
            {
                auto conn = assets[i].asset->casted<AssetConnection>()->connection;
                log::log$("    IPC Connection accepted: {}", conn->accepted);
                 log::log$("    IPC Connection closed: {}", (int)conn->closed_status);
                 log::log$("    IPC Connected to server: {}", (int)conn->server_handle);

            }
            if(assets[i].asset->kind == OBJECT_KIND_MEMORY)
            {
                auto mem = assets[i].asset->casted<AssetMemory>();

                log::log$("    Memory addr: {}-{}", mem->addr | fmt::FMT_HEX,
                          (mem->addr + mem->size) | fmt::FMT_HEX);
                log::log$("    Memory allocated: {}", mem->allocated);
            }

            if(assets[i].asset->kind == OBJECT_KIND_MAPPING)
            {
                auto mapping = assets[i].asset->casted<AssetMapping>();

                log::log$("    Mapping: {}-{}", mapping->start | fmt::FMT_HEX, mapping->end | fmt::FMT_HEX);
                log::log$("    Mapping writable: {}", mapping->writable);
                log::log$("    Mapping executable: {}", mapping->executable);
            }



        }


       lock.release();
}

// asset_release is now a template function defined in space.hpp
core::Result<AssetRef<Space>> Space::create_space([[maybe_unused]] uint64_t flags, [[maybe_unused]] uint64_t rights)
{

    AssetRef<Space> ptr = try$(allocate_asset<Space>());
    auto asset = ptr.asset;

    asset->parent_space_handle = AssetRef<Space>(this, -1);

    auto vspace = VmmSpace::create(false);

    if (vspace.is_error())
    {
        log::err$("space_create: failed to create vmm space: {}", vspace.error());
        asset->lock.release();
        asset_release(ptr);
        return vspace.error();
    }

    asset->vmm_space = vspace.unwrap();

    SpacePtr space_ptr = {};
    space_ptr.space = asset;
    space_ptr.handle = _space_handle++;

    _spaces_lock.lock();
    _spaces.push(space_ptr);
    _spaces_lock.release();

    asset->uid = space_ptr.handle;
   // asset->ref_count++; // referencing by itself

    asset->alloc_uid = 16;

    asset->lock.release();

    asset->assets.push(
        AssetRef<Space>(asset, 0).to_untyped());

    return ptr;
}
// FIXME: this is not safe, because it does not check if the space exists in the parent space
core::Result<AssetRef<Space>> Space::global_space_by_handle(uint64_t handle)
{
    _spaces_lock.lock();
    for (auto &space_ptr : _spaces)
    {
        if (space_ptr.handle == handle)
        {
            auto sp = space_ptr;
            _spaces_lock.release();
            return AssetRef<Space>(sp.space, -1);
        }
    }
    _spaces_lock.release();


    return "Space not found";
}
