#include "space.hpp"
#include "kernel/generic/asset.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/result.hpp"


size_t _space_handle; 

struct SpacePtr {
    Space *space;
    uint64_t handle; // the handle of the space in the space
};


core::Vec<SpacePtr> _spaces;

core::Result<AssetPtr> space_create(Space* parent, [[maybe_unused]] uint64_t flags, [[maybe_unused]] uint64_t rights)
{
    
    AssetPtr ptr = try$(_asset_create(parent, OBJECT_KIND_SPACE));
    Asset * asset = ptr.asset;
    Space* space = new Space();

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


    space->assets.push({.asset = asset, .handle = 0});

    asset->lock.release();

    return ptr;
}
