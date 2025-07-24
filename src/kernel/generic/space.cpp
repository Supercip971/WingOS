#include "space.hpp"
#include "kernel/generic/asset.hpp"
#include "libcore/ds/vec.hpp"


size_t _space_handle; 

struct SpacePtr {
    Space *space;
    uint64_t handle; // the handle of the space in the space
};


core::Vec<SpacePtr> _spaces;

Asset * space_create(Space* parent, [[maybe_unused]] uint64_t flags, [[maybe_unused]] uint64_t rights)
{
    
    Asset *asset = _asset_create(parent, OBJECT_KIND_SPACE);
    
    Space* space = new Space();

    *space = {};
    asset->space = space;

    space->self = asset;

    space->parent_space_handle = parent; 
    auto vspace = VmmSpace::create(false);

    if (vspace.is_error())
    {
        log::err$("space_create: failed to create vmm space: {}", vspace.error());
        delete space;
        delete asset;
        return nullptr;
    }

    space->vmm_space = vspace.unwrap();

    SpacePtr space_ptr;
    space_ptr.space = space;
    space_ptr.handle = _space_handle++;
    _spaces.push(space_ptr);


    space->assets.push({.asset = asset, .handle = 0});

    asset->lock.release();


    return asset;
}
