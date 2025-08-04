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

core::Vec<SpacePtr> _spaces;

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

    SpacePtr space_ptr;
    space_ptr.space = space;
    space_ptr.handle = _space_handle++;
    _spaces.push(space_ptr);

    asset->space->uid = space_ptr.handle;    
    space->assets.push({.asset = asset, .handle = 0});
    space->alloc_uid = 16;
    asset->lock.release();

    return ptr;
}

// FIXME: this is not safe, because it does not check if the space exists in the parent space
core::Result<Space *> Space::space_by_handle(Space* parent, uint64_t handle)
{
    for (auto &space_ptr : parent->assets)
    {
        if (space_ptr.handle == handle && space_ptr.asset->kind == OBJECT_KIND_SPACE)
        {
            return core::Result<Space *>::csuccess(space_ptr.asset->space);
        }   
     }

    return core::Result<Space *>::error("space not found");
}
// FIXME: this is not safe, because it does not check if the space exists in the parent space
core::Result<Space *> Space::global_space_by_handle(uint64_t handle)
{
    for (auto &space_ptr : _spaces)
    {
        if (space_ptr.handle == handle)
        {
            return core::Result<Space *>::csuccess(space_ptr.space);
        }
    }

    return core::Result<Space *>::error("space not found");
}
