#pragma once 

#include <stdint.h>
#include "kernel/generic/paging.hpp"
#include "kernel/generic/task.hpp"
#include "libcore/ds/vec.hpp"
#include "libcore/lock/lock.hpp"

#include <wingos-headers/asset.h>

struct Asset;

struct AssetPtr {
    Asset *asset;
    uint64_t handle; // the handle of the asset in the space
    bool write;
    bool read; 
    bool share;
};

struct Space {

    size_t uid ;
    Asset* self;
    uint64_t space_handle; 
    Space* parent_space_handle; // the space that created this space
    VmmSpace vmm_space; // the virtual memory space of this space

    core::Vec<AssetPtr> assets;
};

struct Asset {
    core::Lock lock;

    size_t ref_count;
    AssetKind kind;
    union {
        struct {
            size_t size;
            size_t addr;
        } memory;

        struct {
            size_t start;
            size_t end;
            Asset *physical_mem; // the physical memory that this mapping is based on
            bool writable;
            bool executable;
        } mapping;


        Space* space;


        kernel::Task* task;
    };


    static core::Result<Asset*> by_handle(Space* space, uint64_t handle)
    {
        for (size_t i = 0; i < space->assets.len(); i++)
        {
            if (space->assets[i].handle == handle)
            {
                return space->assets[i].asset;
            }
        }
        return core::Result<Asset*>::error("asset not found");
    }
};


core::Result<AssetPtr> space_create(Space* parent, uint64_t flags, uint64_t rights);
